/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

/*
 *  seedfill.c:  Luc Vincent iterative raster algorithms
 *
 *      Binary seedfill:
 *               PIX      *pixSeedfillBinary()
 *
 *      Applications of binary seedfill to find and fill holes,
 *      and to remove c.c. touching the border:
 *               PIX      *pixHolesByFilling()
 *               PIX      *pixFillClosedBorders()
 *               PIX      *pixRemoveBorderConnComps()
 *
 *      Hole-filling of components to bounding rectangle
 *               PIX      *pixFillHolesToBoundingRect()
 *
 *      Gray seedfill:
 *               l_int32   pixSeedfillGray()
 *
 *      Distance function:
 *               PIX      *pixDistanceFunction()
 *
 *
 *
 *           ITERATIVE RASTER-ORDER SEEDFILL
 *
 *      The basic method in the Vincent seedfill (aka
 *      reconstruction) algorithm is simple.  We describe here the
 *      situation for binary seedfill.  Pixels are sampled
 *      in raster order in the seed image.  If they are 4-connected
 *      to ON pixels either directly above or to the left, and are
 *      not masked out by the mask image, they are turned on (or remain on).
 *      (Ditto for 8-connected, except you need to check 3 pixels
 *      on the previous line as well as the pixel to the left 
 *      on the current line.  This is extra computational work
 *      for relatively little gain, so it is preferable
 *      in most situations to use the 4-connected version.)
 *      The algorithm proceeds from UR to LL of the image, and
 *      then reverses and sweeps up from LL to UR.
 *      These double sweeps are iterated until there is no change.
 *      At this point, the seed has entirely filled the region it
 *      is allowed to, as delimited by the mask image. 
 *
 *      For some applications, the filled seed will later be OR'd
 *      with the negative of the mask.   This is used, for example,
 *      when you flood fill into a 4-connected region of OFF pixels
 *      and you want the result after those pixels are turned ON.
 *
 *      Note carefully that the mask we use delineates which pixels
 *      are allowed to be ON as the seed is filled.  We will call this
 *      a "filling mask".  As the seed expands, it is repeatedly
 *      ANDed with the filling mask: s & fm.  The process can equivalently
 *      be formulated using the inverse of the filling mask, which
 *      we will call a "blocking mask": bm = ~fm.   As the seed
 *      expands, the blocking mask is repeatedly used to prevent
 *      the seed from expanding into the blocking mask.  This is done
 *      by set subtracting the blocking mask from the expanded seed:
 *      s - bm.  Set subtraction of the blocking mask is equivalent
 *      to ANDing with the inverse of the blocking mask: s & (~bm).
 *      But from the inverse relation between blocking and filling
 *      masks, this is equal to s & fm, which proves the equivalence.
 *      
 *      For efficiency, the pixels can be taken in larger units
 *      for processing, but still in raster order.  It is natural
 *      to take them in 32-bit words, because the machine operations
 *      on the PIII work on 32-bit words.  The outline of the work
 *      to be done for 4-cc (not including special cases for boundary
 *      words, such as the first line or the last word in each line)
 *      is as follows.  Let the filling mask be m.  The
 *      seed is to fill "under" the mask; i.e., limited by an AND
 *      with the mask.  Let the current word be w, the word
 *      in the line above be wa, and the previous word in the
 *      current line be wp.   Let t be a temporary word that
 *      is used in computation.  Note that masking is performed by
 *      w & m.  (If we had instead used a "blocking" mask, we
 *      would perform masking by the set subtraction operation,
 *      w - m, which is defined to be w & ~m.)
 *
 *      The entire operation can be implemented with shifts,
 *      logical operations and tests.  For each word in the seed image
 *      there are two steps.  The first step is to OR the word with
 *      the word above and with the rightmost pixel in wp (call it "x").
 *      Because wp is shifted one pixel to its right, "x" is ORed
 *      to the leftmost pixel of w.  We then clip to the ON pixels in
 *      the mask.  The result is
 *               t  <--  (w | wa | x000... ) & m
 *      We've now finished taking data from above and to the left.
 *      The second step is to allow filling to propagate horizontally
 *      in t, always making sure that it is properly masked at each
 *      step.  So if filling can be done (i.e., t is neither all 0s
 *      nor all 1s), iteratively take:
 *           t  <--  (t | (t >> 1) | (t << 1)) & m
 *      until t stops changing.  Then write t back into w.
 *
 *      Finally, the boundary conditions require we note that in doing
 *      the above steps,
 *          (a) the words in the first row have no wa
 *          (b) the first word in each row has no wp in that row
 *          (c) the last word in each row must be masked so that
 *              pixels don't propagate beyond the right edge of the
 *              actual image.  (This is easily accomplished by
 *              setting the out-of-bound pixels in m to OFF.)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#ifndef  NO_CONSOLE_IO
#define   DEBUG_PRINT_ITERS    0
#endif  /* ~NO_CONSOLE_IO */

  /* two-way (UL --> LR, LR --> UL) sweep iterations; typically need only 4 */
static const l_int32  MAX_ITERS = 40;


/*-----------------------------------------------------------------------*
 *              Vincent's Iterative Binary Seedfill method               *
 *-----------------------------------------------------------------------*/
/*!
 *  pixSeedfillBinary()
 *
 *      Input:  pixd  (<optional> destination: this can be null,
 *                     equal to pixs, or different from pixs)
 *              pixs  (seed)
 *              pixm  (filling mask)
 *              connectivity  (4 or 8)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) This is for binary seedfill
 *      (2) If pixs == pixd, the fill is in-place
 *      (3) The returned pixd is the filled seed.  For some 
 *          applications you want to OR it with the inverse of
 *          the filling mask.
 *      (4) The seed and mask images can be different sizes, but
 *          in typical use the difference, if any, would be only
 *          a few pixels in each direction.  If the sizes differ,
 *          the clipping is handled by the low-level function
 *          seedfillBinaryLow().
 */
PIX *
pixSeedfillBinary(PIX     *pixd,
                  PIX     *pixs,
	          PIX     *pixm,
	          l_int32  connectivity)
{
l_int32    i, boolval;
l_int32    hd, hm, wpld, wplm;
l_uint32  *datad, *datam;
PIX       *pixt;

    PROCNAME("pixSeedfillBinary");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (!pixm)
        return (PIX *)ERROR_PTR("pixm not defined", procName, pixd);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not in {4,8}", procName, pixd);
    if (pixGetDepth(pixs) != 1 || pixGetDepth(pixm) != 1)
        return (PIX *)ERROR_PTR("pixs must be binary", procName, pixd);

	/* pixd starts out as a copy or identity with pixs */
    if (pixd != pixs) {
	if ((pixd = pixCopy(pixd, pixs)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, pixd);
    }

	/* pixt is used to test for completion */
    if ((pixt = pixCreateTemplate(pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixt not made", procName, pixd);

    hd = pixGetHeight(pixd);
    hm = pixGetHeight(pixm);  /* included so seedfillBinaryLow() can clip */
    datad = pixGetData(pixd);
    datam = pixGetData(pixm);
    wpld = pixGetWpl(pixd);
    wplm = pixGetWpl(pixm);

    pixSetPadBits(pixm, 0);

    for (i = 0; i < MAX_ITERS; i++) {
	pixCopy(pixt, pixd);
	seedfillBinaryLow(datad, hd, wpld, datam, hm, wplm, connectivity);
	pixEqual(pixd, pixt, &boolval);
	if (boolval == 1) {
#if DEBUG_PRINT_ITERS
	    fprintf(stderr, "Binary seed fill converged: %d iters\n", i + 1);
#endif  /* DEBUG_PRINT_ITERS */
	    break;
	}
    }

    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixHolesByFilling()
 *
 *      Input:  pixs, connectivity (4 or 8)
 *      Return: pixd  (inverted image of all holes), or null on error
 *
 * Action:
 *     (1) start with 1-pixel black border on otherwise white pixd
 *     (2) use the inverted pixs as the filling mask to fill in
 *         all the pixels from the border to the pixs foreground
 *     (3) OR the result with pixs to have an image with all
 *         ON pixels except for the holes.
 *     (4) invert the result to get the holes as foreground
 *
 * Note: To get 4-c.c. holes of the 8-c.c. as foreground, use
 *       4-connected filling; to get 8-c.c. holes of the 4-c.c.
 *       as foreground, use 8-connected filling.
 */
PIX *
pixHolesByFilling(PIX     *pixs,
                  l_int32  connectivity)
{
PIX  *pixsi, *pixd;

    PROCNAME("pixHolesByFilling");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    if ((pixsi = pixInvert(NULL, pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixsi not made", procName, NULL);

    pixSetOrClearBorder(pixd, 1, 1, 1, 1, PIX_SET);
    pixSeedfillBinary(pixd, pixd, pixsi, connectivity);
    pixOr(pixd, pixd, pixs);
    pixInvert(pixd, pixd);
    pixDestroy(&pixsi);

    return pixd;
}


/*!
 *  pixFillClosedBorders()
 *
 *      Input:  pixs
 *              filling connectivity (4 or 8)
 *      Return: pixd  (all topologically outer closed borders are filled
 *                     as connected comonents), or null on error
 *
 *  Action:
 *      (1) start with 1-pixel black border on otherwise white pixd
 *      (2) subtract input pixs to remove border pixels that were
 *          also on the closed border
 *      (3) use the inverted pixs as the filling mask to fill in
 *          all the pixels from the outer border to the closed border
 *          on pixs
 *      (4) invert the result to get the filled component, including
 *          the input border
 *
 *  Note: (1) if the borders are 4-c.c., use 8-c.c. filling, and v.v.
 *        (2) closed borders within c.c. that represent holes, etc., are filled.
 */
PIX *
pixFillClosedBorders(PIX     *pixs,
                     l_int32  connectivity)
{
PIX  *pixsi, *pixd;

    PROCNAME("pixFillClosedBorders");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixSetOrClearBorder(pixd, 1, 1, 1, 1, PIX_SET);
    pixSubtract(pixd, pixd, pixs);
    if ((pixsi = pixInvert(NULL, pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixsi not made", procName, NULL);

    pixSeedfillBinary(pixd, pixd, pixsi, connectivity);
    pixInvert(pixd, pixd);
    pixDestroy(&pixsi);

    return pixd;
}


/*!
 *  pixRemoveBorderConnComps()
 *
 *      Input:  pixs
 *              filling connectivity (4 or 8)
 *      Return: pixd  (all pixels in the src that are not touching the
 *                     border) or null on error
 *
 *  Note: This is a very simple application of seedfill, where
 *        we find all components that are touching the borders
 *        and remove them.
 */
PIX *
pixRemoveBorderConnComps(PIX     *pixs,
                         l_int32  connectivity)
{
PIX  *pixd;

    PROCNAME("pixRemoveBorderConnComps");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

        /* pixd is the seed; start with 1 pixel wide black border */
    pixSetOrClearBorder(pixd, 1, 1, 1, 1, PIX_SET);

       /* fill from the seed, using pixs as the filling mask,
        * to fill in all components that are touching the border */
    pixSeedfillBinary(pixd, pixd, pixs, connectivity);

       /* get components in filling mask but not in seed */
    pixXor(pixd, pixd, pixs);

    return pixd;
}


/*-----------------------------------------------------------------------*
 *            Hole-filling of components to bounding rectangle           *
 *-----------------------------------------------------------------------*/
/*!
 *  pixFillHolesToBoundingRect()
 *
 *      Input:  pixs (1 bpp)
 *              minsize (min number of pixels in the hole)
 *              maxhfract (max hole area as fraction of fg pixels in the cc)
 *              minfgfract (min fg area as fraction of bounding rectangle)
 *      Return: pixd (pixs, with some holes possibly filled and some c.c.
 *                    possibly expanded to their bounding rects),
 *                    or null on error
 *
 *  Notes:
 *      (1) This does not fill holes that are smaller in area than 'minsize'.
 *      (2) This does not fill holes with an area larger than
 *          'maxhfract' times the fg area of the c.c.
 *      (3) This does not expand the fg of the c.c. to bounding rect if
 *          the fg area is less than 'minfgfract' times the area of the
 *          bounding rect.
 *      (4) The decisions are made as follows:
 *           - Decide if we are filling the holes; if so, when using
 *             the fg area, include the filled holes.
 *           - Decide based on the fg area if we are filling to a bounding rect.
 *             If so, do it.
 *             If not, fill the holes if the condition is satisfied.
 *      (5) The choice of minsize depends on the resolution.
 *      (6) For solidifying image mask regions on printed materials,
 *          which tend to be rectangular, values for maxhfract
 *          and minfgfract around 0.5 are reasonable.
 */
PIX *
pixFillHolesToBoundingRect(PIX       *pixs,
                           l_int32    minsize,
                           l_float32  maxhfract,
                           l_float32  minfgfract)
{
l_int32    i, x, y, w, h, n, nfg, nh, ntot, area;
l_int32   *tab;
l_float32  hfract;  /* measured hole fraction */
l_float32  fgfract;  /* measured fg fraction */
BOXA      *boxa;
PIX       *pixd, *pixfg, *pixh;
PIXA      *pixa;

    PROCNAME("pixFillHolesToBoundingRect");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);

    pixd = pixCopy(NULL, pixs);
    boxa = pixConnComp(pixd, &pixa, 8);
    n = boxaGetCount(boxa);
    tab = makePixelSumTab8();
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
        area = w * h;
        if (area < minsize)
            continue;
        pixfg = pixaGetPix(pixa, i, L_COPY);
        pixh = pixHolesByFilling(pixfg, 4);  /* holes only */
        pixCountPixels(pixfg, &nfg, tab);
        pixCountPixels(pixh, &nh, tab);
        hfract = (l_float32)nh / (l_float32)nfg;
        ntot = nfg;
        if (hfract <= maxhfract)  /* we will fill the holes (at least) */
            ntot = nfg + nh;
        fgfract = (l_float32)ntot / (l_float32)area;
        if (fgfract >= minfgfract) {  /* fill to bounding rect */
            pixSetAll(pixfg);
            pixRasterop(pixd, x, y, w, h, PIX_SRC, pixfg, 0, 0);
        }
        else if (hfract <= maxhfract) {  /* fill just the holes */
            pixRasterop(pixd, x, y, w, h, PIX_DST | PIX_SRC , pixh, 0, 0);
        }
        pixDestroy(&pixfg);
        pixDestroy(&pixh);
    }
    boxaDestroy(&boxa);
    pixaDestroy(&pixa);
    FREE(tab);

    return pixd;
}


/*-----------------------------------------------------------------------*
 *             Vincent's Iterative Grayscale Seedfill method             *
 *-----------------------------------------------------------------------*/
/*!
 *  pixSeedfillGray()
 *
 *      Input:  pixs  (seed; filled in place)
 *              pixm  (filling mask)
 *              connectivity  (4 or 8)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixSeedfillGray(PIX     *pixs,
	        PIX     *pixm,
	        l_int32  connectivity)
{
l_int32    i, h, w, wpls, wplm, boolval;
l_uint32  *datas, *datam;
PIX       *pixt;

    PROCNAME("pixSeedfillGray");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pixm)
        return ERROR_INT("pixm not defined", procName, 1);
    if (connectivity != 4 && connectivity != 8)
        return ERROR_INT("connectivity not in {4,8}", procName, 1);
    if (pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs must be 8 bpp", procName, 1);
    
	/* make sure the sizes of seed and mask images are the same */
    if (pixSizesEqual(pixs, pixm) == 0)
        return ERROR_INT("pixs and pixm sizes differ", procName, 1);
    h = pixGetHeight(pixs);
    w = pixGetWidth(pixs);

	/* pixt is used to test for completion */
    if ((pixt = pixCreateTemplate(pixs)) == NULL)
	return ERROR_INT("pixt not made", procName, 1);

    datas = pixGetData(pixs);
    datam = pixGetData(pixm);
    wpls = pixGetWpl(pixs);
    wplm = pixGetWpl(pixm);
    for (i = 0; i < MAX_ITERS; i++) {
	pixCopy(pixt, pixs);
	seedfillGrayLow(datas, w, h, wpls, datam, wplm, connectivity);
	pixEqual(pixs, pixt, &boolval);
	if (boolval == 1) {
#if DEBUG_PRINT_ITERS
	    fprintf(stderr, "Gray seed fill converged: %d iters\n", i + 1);
#endif  /* DEBUG_PRINT_ITERS */
	    break;
	}
    }

    pixDestroy(&pixt);
    return 0;
}



/*-----------------------------------------------------------------------*
 *                   Vincent's Distance Function method                  *
 *-----------------------------------------------------------------------*/
/*!
 *  pixDistanceFunction()
 *
 *      Input:  pixs  (1 bpp source)
 *              connectivity  (4 or 8)
 *              depth (8 or 16 bits for pixd)
 *      Return: pixd, or null on error
 *
 *  This computes the distance of each pixel from the nearest
 *  background pixel.  All bg pixels therefore have a distance of 0,
 *  and the fg pixel distances increase linearly from 1 at the
 *  boundary.  It can also be used to compute the distance of
 *  each pixel from the nearest fg pixel, by inverting the input
 *  image before calling this function.  Then all fg pixels have
 *  a distance 0 and the bg pixel distances increase linearly
 *  from 1 at the boundary.
 *
 *  The algorithm, described at Leptonica on the page on seed
 *  filling and connected components, is due to Luc Vincent.
 *  In brief, we generate an 8 or 16 bpp image, initialized to 1 for
 *  the fg pixels of the input pix, subject to the constraint that
 *  the boundary pixels are initialized to 0.  We then do 2 sweeps,
 *  in raster followed by anti-raster order, where the value of each
 *  new pixel is taken to be 1 more than the minimum of the
 *  previously-seen connected pixels (using either 4 or 8
 *  connectivity).  
 */
PIX *
pixDistanceFunction(PIX     *pixs,
	            l_int32  connectivity,
		    l_int32  depth)
{
l_int32    w, h, wpld;
l_uint32  *datad;
PIX       *pixd;

    PROCNAME("pixDistanceFunction");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);
    if (depth != 8 && depth != 16)
        return (PIX *)ERROR_PTR("depth not 8 or 16 bpp", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs must be binary", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if ((pixd = pixCreate(w, h, depth)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    pixSetMasked(pixd, pixs, 1);
    distanceFunctionLow(datad, w, h, depth, wpld, connectivity);

    return pixd;
}

