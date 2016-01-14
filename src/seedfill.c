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
 *  seedfill.c
 *
 *      Binary seedfill (source: Luc Vincent)
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
 *      Gray seedfill (source: Luc Vincent)
 *               l_int32   pixSeedfillGray()
 *
 *      Distance function (source: Luc Vincent)
 *               PIX      *pixDistanceFunction()
 *
 *      Seed spread (based on distance function)
 *               PIX      *pixSeedspread()
 *
 *      Local extrema:
 *               l_int32   pixLocalExtrema()
 *        static l_int32   pixQualifyLocalMinima()
 *               l_int32   pixSelectedLocalExtrema()
 *               PIX      *pixFindEqualValues()
 *
 *
 *
 *           ITERATIVE RASTER-ORDER SEEDFILL
 *
 *      The basic method in the Vincent seedfill (aka reconstruction)
 *      algorithm is simple.  We describe here the situation for
 *      binary seedfill.  Pixels are sampled in raster order in
 *      the seed image.  If they are 4-connected to ON pixels
 *      either directly above or to the left, and are not masked
 *      out by the mask image, they are turned on (or remain on).
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
 *      The grayscale seedfill is a straightforward generalization
 *      of the binary seedfill, and is described in seedfillLowGray().
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
 *      to take them in 32-bit words.  The outline of the work
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
 *      the above steps:
 *          (a) The words in the first row have no wa
 *          (b) The first word in each row has no wp in that row
 *          (c) The last word in each row must be masked so that
 *              pixels don't propagate beyond the right edge of the
 *              actual image.  (This is easily accomplished by
 *              setting the out-of-bound pixels in m to OFF.)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#ifndef  NO_CONSOLE_IO
#define   DEBUG_PRINT_ITERS    0
#endif  /* ~NO_CONSOLE_IO */

  /* Two-way (UL --> LR, LR --> UL) sweep iterations; typically need only 4 */
static const l_int32  MAX_ITERS = 40;

    /* Static function */
static l_int32 pixQualifyLocalMinima(PIX *pixs, PIX *pixm);


/*-----------------------------------------------------------------------*
 *              Vincent's Iterative Binary Seedfill method               *
 *-----------------------------------------------------------------------*/
/*!
 *  pixSeedfillBinary()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs; 1 bpp)
 *              pixs  (1 bpp seed)
 *              pixm  (1 bpp filling mask)
 *              connectivity  (4 or 8)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) This is for binary seedfill (aka "binary reconstruction").
 *      (2) There are 3 cases:
 *            (a) pixd == null (make a new pixd)
 *            (b) pixd == pixs (in-place)
 *            (c) pixd != pixs
 *      (3) If you know the case, use these patterns for clarity:
 *            (a) pixd = pixSeedfillBinary(NULL, pixs, ...);
 *            (b) pixSeedfillBinary(pixs, pixs, ...);
 *            (c) pixSeedfillBinary(pixd, pixs, ...);
 *      (4) The resulting pixd contains the filled seed.  For some 
 *          applications you want to OR it with the inverse of
 *          the filling mask.
 *      (5) The input seed and mask images can be different sizes, but
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

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (!pixm || pixGetDepth(pixm) != 1)
        return (PIX *)ERROR_PTR("pixm undefined or not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not in {4,8}", procName, NULL);

        /* Prepare pixd as a copy of pixs if not identical */
    if ((pixd = pixCopy(pixd, pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

        /* pixt is used to test for completion */
    if ((pixt = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixt not made", procName, NULL);

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
 *      Input:  pixs (1 bpp)
 *              connectivity (4 or 8)
 *      Return: pixd  (inverted image of all holes), or null on error
 *
 * Action:
 *     (1) Start with 1-pixel black border on otherwise white pixd
 *     (2) Use the inverted pixs as the filling mask to fill in
 *         all the pixels from the border to the pixs foreground
 *     (3) OR the result with pixs to have an image with all
 *         ON pixels except for the holes.
 *     (4) Invert the result to get the holes as foreground
 *
 * Notes:
 *     (1) To get 4-c.c. holes of the 8-c.c. as foreground, use
 *         4-connected filling; to get 8-c.c. holes of the 4-c.c.
 *         as foreground, use 8-connected filling.
 */
PIX *
pixHolesByFilling(PIX     *pixs,
                  l_int32  connectivity)
{
PIX  *pixsi, *pixd;

    PROCNAME("pixHolesByFilling");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
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
 *      Input:  pixs (1 bpp)
 *              filling connectivity (4 or 8)
 *      Return: pixd  (all topologically outer closed borders are filled
 *                     as connected comonents), or null on error
 *
 *  Notes:
 *      (1) Start with 1-pixel black border on otherwise white pixd
 *      (2) Subtract input pixs to remove border pixels that were
 *          also on the closed border
 *      (3) Use the inverted pixs as the filling mask to fill in
 *          all the pixels from the outer border to the closed border
 *          on pixs
 *      (4) Invert the result to get the filled component, including
 *          the input border
 *      (5) If the borders are 4-c.c., use 8-c.c. filling, and v.v.
 *      (6) Closed borders within c.c. that represent holes, etc., are filled.
 */
PIX *
pixFillClosedBorders(PIX     *pixs,
                     l_int32  connectivity)
{
PIX  *pixsi, *pixd;

    PROCNAME("pixFillClosedBorders");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
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
 *      Input:  pixs (1 bpp)
 *              filling connectivity (4 or 8)
 *      Return: pixd  (all pixels in the src that are not touching the
 *                     border) or null on error
 *
 *  Notes:
 *      (1) This is a very simple application of seedfill, where
 *          we find all components that are touching the borders
 *          and remove them.
 */
PIX *
pixRemoveBorderConnComps(PIX     *pixs,
                         l_int32  connectivity)
{
PIX  *pixd;

    PROCNAME("pixRemoveBorderConnComps");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

        /* Start with 1 pixel wide black border as seed. */
    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixSetOrClearBorder(pixd, 1, 1, 1, 1, PIX_SET);

       /* Fill from the seed, using pixs as the filling mask,
        * to fill in all components that are touching the border */
    pixSeedfillBinary(pixd, pixd, pixs, connectivity);

       /* Get components in filling mask but not in seed */
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

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);

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
 *      Input:  pixs  (8 bpp seed; filled in place)
 *              pixm  (8 bpp filling mask)
 *              connectivity  (4 or 8)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) For details of the operation, see the description in
 *          seedfillGrayLow() and the code there.
 *      (2) For use of the operation, see the description in pixHDome().
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

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!pixm || pixGetDepth(pixm) != 8)
        return ERROR_INT("pixm not defined or not 8 bpp", procName, 1);
    if (connectivity != 4 && connectivity != 8)
        return ERROR_INT("connectivity not in {4,8}", procName, 1);
    
        /* Make sure the sizes of seed and mask images are the same */
    if (pixSizesEqual(pixs, pixm) == 0)
        return ERROR_INT("pixs and pixm sizes differ", procName, 1);

        /* This is used to test for completion */
    if ((pixt = pixCreateTemplate(pixs)) == NULL)
        return ERROR_INT("pixt not made", procName, 1);

    datas = pixGetData(pixs);
    datam = pixGetData(pixm);
    wpls = pixGetWpl(pixs);
    wplm = pixGetWpl(pixm);
    pixGetDimensions(pixs, &w, &h, NULL);
    for (i = 0; i < MAX_ITERS; i++) {
        pixCopy(pixt, pixs);
        seedfillGrayLow(datas, w, h, wpls, datam, wplm, connectivity);
        pixEqual(pixs, pixt, &boolval);
        if (boolval == 1) {
#if DEBUG_PRINT_ITERS
            L_INFO_INT("Gray seed fill converged: %d iters", procName, i + 1);
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
 *              outdepth (8 or 16 bits for pixd)
 *              boundcond (L_BOUNDARY_BG, L_BOUNDARY_FG)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) This computes the distance of each pixel from the nearest
 *          background pixel.  All bg pixels therefore have a distance of 0,
 *          and the fg pixel distances increase linearly from 1 at the
 *          boundary.  It can also be used to compute the distance of
 *          each pixel from the nearest fg pixel, by inverting the input
 *          image before calling this function.  Then all fg pixels have
 *          a distance 0 and the bg pixel distances increase linearly
 *          from 1 at the boundary.
 *      (2) The algorithm, described in Leptonica on the page on seed
 *          filling and connected components, is due to Luc Vincent.
 *          In brief, we generate an 8 or 16 bpp image, initialized
 *          with the fg pixels of the input pix set to 1 and the
 *          1-boundary pixels (i.e., the boundary pixels of width 1 on
 *          the four sides set as either:
 *            * L_BOUNDARY_BG: 0
 *            * L_BOUNDARY_FG:  max
 *          where max = 0xff for 8 bpp and 0xffff for 16 bpp.
 *          Then do raster/anti-raster sweeps over all pixels interior
 *          to the 1-boundary, where the value of each new pixel is
 *          taken to be 1 more than the minimum of the previously-seen
 *          connected pixels (using either 4 or 8 connectivity).
 *          Finally, set the 1-boundary pixels using the mirrored method;
 *          this removes the max values there.
 *      (3) Using L_BOUNDARY_BG clamps the distance to 0 at the
 *          boundary.  Using L_BOUNDARY_FG allows the distance
 *          at the image boundary to "float".
 *      (4) For 4-connected, one could initialize only the left and top
 *          1-boundary pixels, and go all the way to the right
 *          and bottom; then coming back reset left and top.  But we
 *          instead use a method that works for both 4- and 8-connected.
 */
PIX *
pixDistanceFunction(PIX     *pixs,
                    l_int32  connectivity,
                    l_int32  outdepth,
                    l_int32  boundcond)
{
l_int32    w, h, wpld;
l_uint32  *datad;
PIX       *pixd;

    PROCNAME("pixDistanceFunction");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("!pixs or pixs not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);
    if (outdepth != 8 && outdepth != 16)
        return (PIX *)ERROR_PTR("outdepth not 8 or 16 bpp", procName, NULL);
    if (boundcond != L_BOUNDARY_BG && boundcond != L_BOUNDARY_FG)
        return (PIX *)ERROR_PTR("invalid boundcond", procName, NULL);

    pixGetDimensions(pixs, &w, &h, NULL);
    if ((pixd = pixCreate(w, h, outdepth)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

        /* Initialize the fg pixels to 1 and the bg pixels to 0 */
    pixSetMasked(pixd, pixs, 1);

    if (boundcond == L_BOUNDARY_BG)
        distanceFunctionLow(datad, w, h, outdepth, wpld, connectivity);
    else {  /* L_BOUNDARY_FG: set boundary pixels to max val */
        pixRasterop(pixd, 0, 0, w, 1, PIX_SET, NULL, 0, 0);   /* top */
        pixRasterop(pixd, 0, h - 1, w, 1, PIX_SET, NULL, 0, 0);   /* bot */
        pixRasterop(pixd, 0, 0, 1, h, PIX_SET, NULL, 0, 0);   /* left */
        pixRasterop(pixd, w - 1, 0, 1, h, PIX_SET, NULL, 0, 0);   /* right */

        distanceFunctionLow(datad, w, h, outdepth, wpld, connectivity);

            /* Set each boundary pixel equal to the pixel next to it */
        pixSetMirroredBorder(pixd, 1, 1, 1, 1);
    }

    return pixd;
}

 
/*-----------------------------------------------------------------------*
 *                Seed spread (based on distance function)               *
 *-----------------------------------------------------------------------*/
/*!
 *  pixSeedspread()
 *
 *      Input:  pixs  (8 bpp source)
 *              connectivity  (4 or 8)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) The raster/anti-raster method for implementing this filling
 *          operation was suggested by Ray Smith.
 *      (2) This takes an arbitrary set of nonzero pixels in pixs, which
 *          can be sparse, and spreads (extrapolates) the values to
 *          fill all the pixels in pixd with the nonzero value it is
 *          closest to in pixs.  This is similar (though not completely
 *          equivalent) to doing a Voronoi tiling of the image, with a
 *          tile surrounding each pixel that has a nonzero value.
 *          All pixels within a tile are then closer to its "central"
 *          pixel than to any others.  Then assign the value of the
 *          "central" pixel to each pixel in the tile.
 *      (3) This is implemented by computing a distance function in parallel
 *          with the fill.  The distance function uses free boundary
 *          conditions (assumed maxval outside), and it controls the
 *          propagation of the pixels in pixd away from the nonzero
 *          (seed) values.  This is done in 2 traversals (raster/antiraster).
 *          In the raster direction, whenever the distance function
 *          is nonzero, the spread pixel takes on the value of its
 *          predecessor that has the minimum distance value.  In the
 *          antiraster direction, whenever the distance function is nonzero
 *          and its value is replaced by a smaller value, the spread
 *          pixel takes the value of the predecessor with the minimum
 *          distance value.
 *      (4) At boundaries where a pixel is equidistant from two
 *          nearest nonzero (seed) pixels, the decision of which value
 *          to use is arbitrary (greedy in search for minimum distance).
 *          This can give rise to strange-looking results, particularly
 *          for 4-connectivity where the L1 distance is computed from
 *          steps in N,S,E and W directions (no diagonals).
 */
PIX *
pixSeedspread(PIX     *pixs,
              l_int32  connectivity)
{
l_int32    w, h, wplt, wplg;
l_uint32  *datat, *datag;
PIX       *pixm, *pixt, *pixg, *pixd;

    PROCNAME("pixSeedspread");

    if (!pixs || pixGetDepth(pixs) != 8)
        return (PIX *)ERROR_PTR("!pixs or pixs not 8 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

        /* Add a 4 byte border to pixs.  This simplifies the computation. */
    pixg = pixAddBorder(pixs, 4, 0);
    pixGetDimensions(pixg, &w, &h, NULL);

        /* Initialize distance function pixt.  Threshold pixs to get
         * a 0 at the seed points where the pixs pixel is nonzero, and
         * a 1 at all points that need to be filled.  Use this as a
         * mask to set a 1 in pixt at all non-seed points.  Also, set all
         * pixt pixels in an interior boundary of width 1 to the
         * maximum value.   For debugging, to view the distance function,
         * use pixConvert16To8(pixt, 0) on small images.  */
    pixm = pixThresholdToBinary(pixg, 1);
    pixt = pixCreate(w, h, 16);
    pixSetMasked(pixt, pixm, 1);
    pixRasterop(pixt, 0, 0, w, 1, PIX_SET, NULL, 0, 0);   /* top */
    pixRasterop(pixt, 0, h - 1, w, 1, PIX_SET, NULL, 0, 0);   /* bot */
    pixRasterop(pixt, 0, 0, 1, h, PIX_SET, NULL, 0, 0);   /* left */
    pixRasterop(pixt, w - 1, 0, 1, h, PIX_SET, NULL, 0, 0);   /* right */
    datat = pixGetData(pixt);
    wplt = pixGetWpl(pixt);

        /* Do the interpolation and remove the border. */
    datag = pixGetData(pixg);
    wplg = pixGetWpl(pixg);
    seedspreadLow(datag, w, h, wplg, datat, wplt, connectivity);
    pixd = pixRemoveBorder(pixg, 4);

    pixDestroy(&pixm);
    pixDestroy(&pixg);
    pixDestroy(&pixt);
    return pixd;
}



/*-----------------------------------------------------------------------*
 *                              Local extrema                            *
 *-----------------------------------------------------------------------*/
/*!
 *  pixLocalExtrema()
 *
 *      Input:  pixs  (8 bpp)
 *              &ppixmin (<optional return> mask of local minima)
 *              &ppixmax (<optional return> mask of local maxima)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This gives the actual local minima and maxima.
 *          A local minimum is a pixel whose surrounding pixels all
 *          have values at least as large, and likewise for a local
 *          maximum.
 *      (2) The minima are found by starting with the erosion-and-equality
 *          approach of pixSelectedLocalExtrema.  This is followed
 *          by a qualification step, where each c.c. in the resulting
 *          minimum mask is extracted, the pixels bordering it are
 *          located, and they are queried.  If all of those pixels
 *          are larger than the value of that minimum, it is a true
 *          minimum and its c.c. is saved; otherwise the c.c. is
 *          rejected.  Note that if a bordering pixel has the
 *          same value as the minimum, it must then have a 
 *          neighbor that is smaller, so the component is not a
 *          true minimum.
 *      (3) The maxima are found by inverting the image and looking
 *          for the minima there.
 *      (4) The generated masks can be used as markers for
 *          further operations.
 */
l_int32
pixLocalExtrema(PIX     *pixs,
                PIX    **ppixmin,
                PIX    **ppixmax)
{
PIX  *pixmin, *pixmax, *pixt1, *pixt2;

    PROCNAME("pixLocalExtrema");

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!ppixmin && !ppixmax)
        return ERROR_INT("neither &pixmin, &pixmax are defined", procName, 1);

    if (ppixmin) {
        pixt1 = pixErodeGray(pixs, 3, 3);
        pixmin = pixFindEqualValues(pixs, pixt1);
        pixDestroy(&pixt1);
        pixQualifyLocalMinima(pixs, pixmin);
        *ppixmin = pixmin;
    }

    if (ppixmax) {
        pixt1 = pixInvert(NULL, pixs);
        pixt2 = pixErodeGray(pixt1, 3, 3);
        pixmax = pixFindEqualValues(pixt1, pixt2);
        pixDestroy(&pixt2);
        pixQualifyLocalMinima(pixt1, pixmax);
        *ppixmax = pixmax;
        pixDestroy(&pixt1);
    }

    return 0;
}


/*!
 *  pixQualifyLocalMinima()
 *
 *      Input:  pixs  (8 bpp)
 *              pixm  (1 bpp mask of values equal to min in 3x3 neighborhood)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This function acts in-place to remove all c.c. in pixm
 *          that are not true local minima.  See notes in pixLocalExtrema().
 */
static l_int32
pixQualifyLocalMinima(PIX  *pixs,
                      PIX  *pixm)
{
l_int32    n, i, j, k, x, y, w, h, xc, yc, wc, hc, xon, yon;
l_int32    vals, wpls, wplc, ismin;
l_uint32   val;
l_uint32  *datas, *datac, *lines, *linec;
BOXA      *boxa;
PIX       *pixt1, *pixt2, *pixc;
PIXA      *pixa;

    PROCNAME("pixQualifyLocalMinima");

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!pixm || pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not defined or not 1 bpp", procName, 1);

    pixGetDimensions(pixs, &w, &h, NULL);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    boxa = pixConnComp(pixm, &pixa, 8);
    n = pixaGetCount(pixa);
    for (k = 0; k < n; k++) {
        boxaGetBoxGeometry(boxa, k, &xc, &yc, &wc, &hc);
        pixt1 = pixaGetPix(pixa, k, L_COPY);
        pixt2 = pixAddBorder(pixt1, 1, 0);
        pixc = pixDilateBrick(NULL, pixt2, 3, 3);
        pixXor(pixc, pixc, pixt2);  /* exterior boundary pixels */
        datac = pixGetData(pixc);
        wplc = pixGetWpl(pixc);
        nextOnPixelInRaster(pixt1, 0, 0, &xon, &yon);
        pixGetPixel(pixs, xc + xon, yc + yon, &val);
        ismin = TRUE;
        for (i = 0, y = yc - 1; i < hc + 2 && y >= 0 && y < h; i++, y++) {
            lines = datas + y * wpls;
            linec = datac + i * wplc;
            for (j = 0, x = xc - 1; j < wc + 2 && x >= 0 && x < w; j++, x++) {
                if (GET_DATA_BIT(linec, j)) {
                    vals = GET_DATA_BYTE(lines, x);
                    if (vals <= val) {  /* not a minimum! */
                        ismin = FALSE;
                        break;
                    }
                }
            }
            if (!ismin)
                break;
        }
        if (!ismin)  /* erase it */
            pixRasterop(pixm, xc, yc, wc, hc, PIX_XOR, pixt1, 0, 0);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixDestroy(&pixc);
    }

    boxaDestroy(&boxa);
    pixaDestroy(&pixa);
    return 0;
}


/*!
 *  pixSelectedLocalExtrema()
 *
 *      Input:  pixs  (8 bpp)
 *              mindist (-1 for keeping all pixels; >= 0 specifies distance)
 *              &ppixmin (<return> mask of local minima)
 *              &ppixmax (<return> mask of local maxima)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This selects those local 3x3 minima that are at least a
 *          specified distance from the nearest local 3x3 maxima, and v.v.
 *          for the selected set of local 3x3 maxima.
 *          The local 3x3 minima is the set of pixels whose value equals
 *          the value after a 3x3 brick erosion, and the local 3x3 maxima
 *          is the set of pixels whose value equals the value after
 *          a 3x3 brick dilation.
 *      (2) mindist is the minimum distance allowed between
 *          local 3x3 minima and local 3x3 maxima, in an 8-connected sense.
 *          mindist == 1 keeps all pixels found in step 1.
 *          mindist == 0 removes all pixels from each mask that are
 *          both a local 3x3 minimum and a local 3x3 maximum.
 *          mindist == 1 removes any local 3x3 minimum pixel that touches a
 *          local 3x3 maximum pixel, and likewise for the local maxima.
 *          To make the decision, visualize each local 3x3 minimum pixel
 *          as being surrounded by a square of size (2 * mindist + 1)
 *          on each side, such that no local 3x3 maximum pixel is within
 *          that square; and v.v.
 *      (3) The generated masks can be used as markers for further operations.
 */
l_int32
pixSelectedLocalExtrema(PIX     *pixs,
                        l_int32  mindist,
                        PIX    **ppixmin,
                        PIX    **ppixmax)
{
PIX  *pixmin, *pixmax, *pixt, *pixtmin, *pixtmax;

    PROCNAME("pixSelectedLocalExtrema");

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!ppixmin || !ppixmax)
        return ERROR_INT("&pixmin and &pixmax not both defined", procName, 1);

    pixt = pixErodeGray(pixs, 3, 3);
    pixmin = pixFindEqualValues(pixs, pixt);
    pixDestroy(&pixt);
    pixt = pixDilateGray(pixs, 3, 3);
    pixmax = pixFindEqualValues(pixs, pixt);
    pixDestroy(&pixt);

        /* Remove all points that are within the prescribed distance
         * from each other. */
    if (mindist < 0) {  /* remove no points */
        *ppixmin = pixmin;
        *ppixmax = pixmax;
    } else if (mindist == 0) {  /* remove points belonging to both sets */
        pixt = pixAnd(NULL, pixmin, pixmax);
        *ppixmin = pixSubtract(pixmin, pixmin, pixt);
        *ppixmax = pixSubtract(pixmax, pixmax, pixt);
        pixDestroy(&pixt);
    } else {
        pixtmin = pixDilateBrick(NULL, pixmin,
                                 2 * mindist + 1, 2 * mindist + 1);
        pixtmax = pixDilateBrick(NULL, pixmax,
                                 2 * mindist + 1, 2 * mindist + 1);
        *ppixmin = pixSubtract(pixmin, pixmin, pixtmax);
        *ppixmax = pixSubtract(pixmax, pixmax, pixtmin);
        pixDestroy(&pixtmin);
        pixDestroy(&pixtmax);
    }
    return 0;
}


/*!
 *  pixFindEqualValues()
 *
 *      Input:  pixs1 (8 bpp)
 *              pixs2 (8 bpp)
 *      Return: pixd (1 bpp mask), or null on error
 *
 *  Notes:
 *      (1) The two images are aligned at the UL corner, and the returned
 *          image has ON pixels where the pixels in pixs1 and pixs2
 *          have equal values.
 */
PIX *
pixFindEqualValues(PIX  *pixs1,
                   PIX  *pixs2)
{
l_int32    w1, h1, w2, h2, w, h;
l_int32    i, j, val1, val2, wpls1, wpls2, wpld;
l_uint32  *datas1, *datas2, *datad, *lines1, *lines2, *lined;
PIX       *pixd;

    PROCNAME("pixFindEqualValues");

    if (!pixs1 || pixGetDepth(pixs1) != 8)
        return (PIX *)ERROR_PTR("pixs1 undefined or not 8 bpp", procName, NULL);
    if (!pixs2 || pixGetDepth(pixs2) != 8)
        return (PIX *)ERROR_PTR("pixs2 undefined or not 8 bpp", procName, NULL);
    pixGetDimensions(pixs1, &w1, &h1, NULL);
    pixGetDimensions(pixs2, &w2, &h2, NULL);
    w = L_MIN(w1, w2);
    h = L_MIN(h1, h2);
    pixd = pixCreate(w, h, 1);
    datas1 = pixGetData(pixs1);
    datas2 = pixGetData(pixs2);
    datad = pixGetData(pixd);
    wpls1 = pixGetWpl(pixs1);
    wpls2 = pixGetWpl(pixs2);
    wpld = pixGetWpl(pixd);

    for (i = 0; i < h; i++) {
        lines1 = datas1 + i * wpls1;
        lines2 = datas2 + i * wpls2;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            val1 = GET_DATA_BYTE(lines1, j);
            val2 = GET_DATA_BYTE(lines2, j);
            if (val1 == val2)
                SET_DATA_BIT(lined, j);
        }
    }

    return pixd;
}


