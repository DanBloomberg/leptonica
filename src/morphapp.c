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
 *  morphapp.c
 *
 *      These are some useful and/or interesting composite
 *      image processing operations, of the type that are often
 *      useful in applications.  Most are morphological in
 *      nature.
 *
 *      Selective connected component closing
 *            PIX     *pixSelectiveConnCompClose()
 *
 *      Selective connected component filling
 *            PIX     *pixSelectiveConnCompFill()
 *
 *      Removal of matched patterns
 *            PIX     *pixRemoveMatchedPattern()
 *
 *      Display of matched patterns
 *            PIX     *pixDisplayMatchedPattern()
 *
 *      Iterative morphological seed filling (don't use for real work)
 *            PIX     *pixSeedfillMorph()
 *      
 *      Granulometry on binary images
 *            NUMA    *pixRunHistogramMorph()
 *
 *      Composite operations on grayscale images
 *            PIX     *pixTophat()
 *            PIX     *pixHDome()
 *            PIX     *pixMorphGradient()
 *
 *      Centroids of PIXA
 *            PTA     *pixaCentroids()
 */

#include <stdio.h>

#include "allheaders.h"


#define   SWAP(x, y)   {temp = (x); (x) = (y); (y) = temp;}



/*-----------------------------------------------------------------*
 *             Selective connected component closing               *
 *-----------------------------------------------------------------*/
/*!
 *  pixSelectiveConnCompClose()
 *
 *      Input:  pixs
 *              sel1
 *              sel2  (<optional>)
 *              connectivity (4 or 8)
 *              minw  (minimum width to consider; use 0 or 1 for any width)
 *              minh  (minimum height to consider; use 0 or 1 for any height)
 *      Return: pixd, or null on error
 *
 *  This closes each c.c in the pix that is larger
 *  than a certain minimum size with one or two sels.
 */
PIX *
pixSelectiveConnCompClose(PIX     *pixs,
                          SEL     *sel1,
                          SEL     *sel2,
         		  l_int32  connectivity,
                          l_int32  minw,
                          l_int32  minh)
{
l_int32  n, i, x, y, w, h;
BOXA    *boxa;
PIX     *pixt1, *pixt2, *pixd;
PIXA    *pixa;

    PROCNAME("pixSelectiveConnCompClose");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!sel1)
	return (PIX *)ERROR_PTR("sel1 not defined", procName, NULL);

    if (minw <= 0) minw = 1;
    if (minh <= 0) minh = 1;

    if ((pixd = pixCopy(NULL, pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    if ((boxa = pixConnComp(pixs, &pixa, connectivity)) == NULL)
	return (PIX *)ERROR_PTR("boxa not made", procName, NULL);
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
	if (w >= minw && h >= minh) {
	    if ((pixt1 = pixaGetPix(pixa, i, L_CLONE)) == NULL)
		return (PIX *)ERROR_PTR("pixt1 not found", procName, NULL);
	    if ((pixt2 = pixCloseSafe(NULL, pixt1, sel1)) == NULL)
		return (PIX *)ERROR_PTR("pixt2 not made", procName, NULL);
	    if (sel2)
		pixCloseSafe(pixt2, pixt2, sel2);
	    pixRasterop(pixd, x, y, w, h, PIX_PAINT, pixt2, 0, 0);
	    pixDestroy(&pixt1);
	    pixDestroy(&pixt2);
	}
    }
    pixaDestroy(&pixa);
    boxaDestroy(&boxa);

    return pixd;
}


/*-----------------------------------------------------------------*
 *             Selective connected component filling               *
 *-----------------------------------------------------------------*/
/*!
 *  pixSelectiveConnCompFill()
 *
 *      Input:  pixs (binary)
 *              connectivity (4 or 8)
 *              minw  (minimum width to consider; use 0 or 1 for any width)
 *              minh  (minimum height to consider; use 0 or 1 for any height)
 *      Return: pix with holes filled in selected c.c., or null on error
 */
PIX *
pixSelectiveConnCompFill(PIX     *pixs,
		         l_int32  connectivity,
                         l_int32  minw,
                         l_int32  minh)
{
l_int32  n, i, x, y, w, h;
BOXA    *boxa;
PIX     *pixt1, *pixt2, *pixd;
PIXA    *pixa;

    PROCNAME("pixSelectiveConnCompFill");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
	return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);

    if (minw <= 0) minw = 1;
    if (minh <= 0) minh = 1;

    if ((pixd = pixCopy(NULL, pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    if ((boxa = pixConnComp(pixs, &pixa, connectivity)) == NULL)
	return (PIX *)ERROR_PTR("boxa not made", procName, NULL);
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
	if (w >= minw && h >= minh) {
	    if ((pixt1 = pixaGetPix(pixa, i, L_CLONE)) == NULL)
		return (PIX *)ERROR_PTR("pixt1 not found", procName, NULL);
	    if ((pixt2 = pixHolesByFilling(pixt1, 12 - connectivity)) == NULL)
		return (PIX *)ERROR_PTR("pixt2 not made", procName, NULL);
	    pixRasterop(pixd, x, y, w, h, PIX_PAINT, pixt2, 0, 0);
	    pixDestroy(&pixt1);
	    pixDestroy(&pixt2);
	}
    }
    pixaDestroy(&pixa);
    boxaDestroy(&boxa);

    return pixd;
}


/*-----------------------------------------------------------------*
 *                    Removal of matched patterns                  *
 *-----------------------------------------------------------------*/
/*!
 *  pixRemoveMatchedPattern()
 *
 *      Input:  pixs (input image, 1 bpp)
 *              pixp (pattern to be removed from image, 1 bpp)
 *              pixe (image after erosion by Sel that approximates pixp, 1 bpp)
 *              x0, y0 (center of Sel)
 *              dsize (number of pixels on each side by which pixp is
 *                     dilated before being subtracted from pixs;
 *                     valid values are {0, 1, 2, 3, 4})
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *    (1) This is in-place.
 *    (2) You can use various functions in selgen to create a Sel
 *        that is used to generate pixe from pixs.
 *    (3) This function is applied after pixe has been computed.
 *        It finds the centroid of each c.c., and subtracts
 *        (the appropriately dilated version of) pixp, with the center
 *        of the Sel used to align pixp with pixs.
 */
l_int32
pixRemoveMatchedPattern(PIX     *pixs,
                        PIX     *pixp,
                        PIX     *pixe,
                        l_int32  x0,
                        l_int32  y0,
                        l_int32  dsize)
{
l_int32  i, nc, x, y, w, h, xb, yb;
BOXA    *boxa;
PIX     *pixt1, *pixt2;
PIXA    *pixa;
PTA     *pta;
SEL     *sel;

    PROCNAME("pixRemoveMatchedPattern");

    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);
    if (!pixp)
	return ERROR_INT("pixp not defined", procName, 1);
    if (!pixe)
	return ERROR_INT("pixe not defined", procName, 1);
    if (pixGetDepth(pixs) != 1 || pixGetDepth(pixp) != 1 ||
        pixGetDepth(pixe) != 1)
	return ERROR_INT("all input pix not 1 bpp", procName, 1);
    if (dsize < 0 || dsize > 4)
	return ERROR_INT("dsize not in {0,1,2,3,4}", procName, 1);

        /* Find the connected components and their centroids */
    boxa = pixConnComp(pixe, &pixa, 8);
    if ((nc = boxaGetCount(boxa)) == 0) {
        L_WARNING("no matched patterns", procName);
        boxaDestroy(&boxa);
        pixaDestroy(&pixa);
        return 0;
    }
    pta = pixaCentroids(pixa);

        /* Optionally dilate the pattern, first adding a border that
         * is large enough to accommodate the dilated pixels */
    sel = NULL;
    if (dsize > 0) {
        sel = selCreateBrick(2 * dsize + 1, 2 * dsize + 1, dsize, dsize,
                             SEL_HIT);
        pixt1 = pixAddBorder(pixp, dsize, 0);
        pixt2 = pixDilate(NULL, pixt1, sel);
        selDestroy(&sel);
        pixDestroy(&pixt1);
    }
    else
        pixt2 = pixClone(pixp);

        /* Subtract out each dilated pattern.  The centroid of each
         * component is located at:
         *       (box->x + x, box->y + y)
         * and the 'center' of the pattern used in making pixe is located at
         *       (x0 + dsize, (y0 + dsize)
         * relative to the UL corner of the pattern.  The center of the
         * pattern is placed at the center of the component. */
    w = pixGetWidth(pixt2);
    h = pixGetHeight(pixt2);
    for (i = 0; i < nc; i++) {
        ptaGetIPt(pta, i, &x, &y);
        boxaGetBoxGeometry(boxa, i, &xb, &yb, NULL, NULL);
        pixRasterop(pixs, xb + x - x0 - dsize, yb + y - y0 - dsize,
                    w, h, PIX_DST & PIX_NOT(PIX_SRC), pixt2, 0, 0);
    }

    boxaDestroy(&boxa);
    pixaDestroy(&pixa);
    ptaDestroy(&pta);
    pixDestroy(&pixt2);
    return 0;
}


/*-----------------------------------------------------------------*
 *                    Display of matched patterns                  *
 *-----------------------------------------------------------------*/
/*!
 *  pixDisplayMatchedPattern()
 *
 *      Input:  pixs (input image, 1 bpp)
 *              pixp (pattern to be removed from image, 1 bpp)
 *              pixe (image after erosion by Sel that approximates pixp, 1 bpp)
 *              x0, y0 (center of Sel)
 *              color (to paint the matched patterns; 0xrrggbb00)
 *              scale (reduction factor for output pixd)
 *              nlevels (if scale < 1.0, threshold to this number of levels)
 *      Return: pixd (8 bpp, colormapped), or null on error
 *
 *  Notes:
 *    (1) A 4 bpp colormapped image is generated.
 *    (2) If scale <= 1.0, do scale to gray for the output, and threshold
 *        to nlevels of gray.
 *    (3) You can use various functions in selgen to create a Sel
 *        that will generate pixe from pixs.
 *    (4) This function is applied after pixe has been computed.
 *        It finds the centroid of each c.c., and colors the output
 *        pixels using pixp (appropriately aligned) as a stencil.
 *        Alignment is done using the origin of the Sel and the
 *        centroid of the eroded image to place the stencil pixp.
 */
PIX *
pixDisplayMatchedPattern(PIX       *pixs,
                         PIX       *pixp,
                         PIX       *pixe,
                         l_int32    x0,
                         l_int32    y0,
                         l_uint32   color,
                         l_float32  scale,
                         l_int32    nlevels)
{
l_int32   i, nc, xb, yb, x, y, xi, yi, rval, gval, bval;
BOXA     *boxa;
PIX      *pixd, *pixt, *pixps;
PIXA     *pixa;
PTA      *pta;
PIXCMAP  *cmap;

    PROCNAME("pixDisplayMatchedPattern");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!pixp)
	return (PIX *)ERROR_PTR("pixp not defined", procName, NULL);
    if (!pixe)
	return (PIX *)ERROR_PTR("pixe not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1 || pixGetDepth(pixp) != 1 ||
        pixGetDepth(pixe) != 1)
	return (PIX *)ERROR_PTR("all input pix not 1 bpp", procName, NULL);
    if (scale > 1.0 || scale <= 0.0) {
        L_WARNING("scale > 1.0 or < 0.0; setting to 1.0", procName);
        scale = 1.0;
    }

        /* Find the connected components and their centroids */
    boxa = pixConnComp(pixe, &pixa, 8);
    if ((nc = boxaGetCount(boxa)) == 0) {
        L_WARNING("no matched patterns", procName);
        boxaDestroy(&boxa);
        pixaDestroy(&pixa);
        return 0;
    }
    pta = pixaCentroids(pixa);

    rval = GET_DATA_BYTE(&color, COLOR_RED);
    gval = GET_DATA_BYTE(&color, COLOR_GREEN);
    bval = GET_DATA_BYTE(&color, COLOR_BLUE);
    if (scale == 1.0) {  /* output 4 bpp at full resolution */
        pixd = pixConvert1To4(NULL, pixs, 0, 1);
        cmap = pixcmapCreate(4);
        pixcmapAddColor(cmap, 255, 255, 255);
        pixcmapAddColor(cmap, 0, 0, 0);
        pixSetColormap(pixd, cmap);

        /* Paint through pixp for each match location.  The centroid of each
         * component in pixe is located at:
         *       (box->x + x, box->y + y)
         * and the 'center' of the pattern used in making pixe is located at
         *       (x0, y0)
         * relative to the UL corner of the pattern.  The center of the
         * pattern is placed at the center of the component. */
        for (i = 0; i < nc; i++) {
            ptaGetIPt(pta, i, &x, &y);
            boxaGetBoxGeometry(boxa, i, &xb, &yb, NULL, NULL);
            pixSetMaskedCmap(pixd, pixp, xb + x - x0, yb + y - y0,
                             rval, gval, bval);
        }
    }
    else {  /* output 4 bpp downscaled */
        pixt = pixScaleToGray(pixs, scale);
        pixd = pixThresholdTo4bpp(pixt, nlevels, 1);
        pixps = pixScaleBySampling(pixp, scale, scale);

        for (i = 0; i < nc; i++) {
            ptaGetIPt(pta, i, &x, &y);
            boxaGetBoxGeometry(boxa, i, &xb, &yb, NULL, NULL);
            xi = (l_int32)(scale * (xb + x - x0));
            yi = (l_int32)(scale * (yb + y - y0));
            pixSetMaskedCmap(pixd, pixps, xi, yi, rval, gval, bval);
        }
        pixDestroy(&pixt);
        pixDestroy(&pixps);
    }

    boxaDestroy(&boxa);
    pixaDestroy(&pixa);
    ptaDestroy(&pta);
    return pixd;
}


/*-----------------------------------------------------------------*
 *             Iterative morphological seed filling                *
 *-----------------------------------------------------------------*/
/*!
 *  pixSeedfillMorph()
 *
 *      Input:  pixs (seed)
 *              pixm (mask)
 *              connectivity (4 or 8)
 *      Return: pix where seed has been grown to completion
 *              into the mask, or null on error
 *
 *  Notes:
 *    (1) This is in general a very inefficient method for filling
 *        from a seed into a mask.  I've included it here for
 *        pedagogical reasons, but it should NEVER be used if
 *        efficiency is any consideration -- use pixSeedfillBinary()!
 *    (2) We use a 3x3 brick SEL for 8-cc filling and a 3x3 plus SEL for 4-cc.
 */
PIX *
pixSeedfillMorph(PIX     *pixs,
                 PIX     *pixm,
                 l_int32  connectivity)
{
l_int32  same, iter;
PIX     *pixt1, *pixd, *temp;
SEL     *sel_3;

    PROCNAME("pixSeedfillMorph");

    if (!pixs)
	return (PIX *)ERROR_PTR("seed pix not defined", procName, NULL);
    if (!pixm)
	return (PIX *)ERROR_PTR("mask pix not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
	return (PIX *)ERROR_PTR("connectivity not in {4,8}", procName, NULL);

    if (pixSizesEqual(pixs, pixm) == 0)
	return (PIX *)ERROR_PTR("pix sizes unequal", procName, NULL);
    if (pixGetDepth(pixs) != 1)
	return (PIX *)ERROR_PTR("pix not binary", procName, NULL);

    if ((sel_3 = selCreateBrick(3, 3, 1, 1, 1)) == NULL)
	return (PIX *)ERROR_PTR("sel_3 not made", procName, NULL);
    if (connectivity == 4) {  /* remove corner hits to make a '+' */
	selSetElement(sel_3, 0, 0, SEL_DONT_CARE);
	selSetElement(sel_3, 2, 2, SEL_DONT_CARE);
	selSetElement(sel_3, 2, 0, SEL_DONT_CARE);
	selSetElement(sel_3, 0, 2, SEL_DONT_CARE);
    }

    if ((pixt1 = pixCopy(NULL, pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixt1 not made", procName, NULL);
    if ((pixd = pixCreateTemplate(pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    iter = 0;
    while (1) {
	iter++;
	pixDilate(pixd, pixt1, sel_3);
	pixAnd(pixd, pixd, pixm);
	pixEqual(pixd, pixt1, &same);
	if (same)
	    break;
	else
	    SWAP(pixt1, pixd);
    }
    fprintf(stderr, " Num iters in binary reconstruction = %d\n", iter);

    pixDestroy(&pixt1);
    selDestroy(&sel_3);

    return pixd;
}



/*-----------------------------------------------------------------*
 *                   Granulometry on binary images                 *
 *-----------------------------------------------------------------*/
/*!
 *  pixRunHistogramMorph()
 *
 *      Input:  pixs
 *              runtype (L_RUN_OFF, L_RUN_ON)
 *              direction (L_HORIZ, L_VERT)
 *              maxsize  (size of largest runlength counted)
 *      Return: numa of run-lengths
 */
NUMA *
pixRunHistogramMorph(PIX     *pixs,
		     l_int32  runtype,
		     l_int32  direction,
		     l_int32  maxsize)
{
l_int32    count, i;
l_float32  val;
NUMA      *na, *nah;
PIX       *pixt1, *pixt2, *pixt3;
SEL       *sel_2a;

    PROCNAME("pixRunHistogramMorph");

    if (!pixs)
	return (NUMA *)ERROR_PTR("seed pix not defined", procName, NULL);
    if (runtype != L_RUN_OFF && runtype != L_RUN_ON)
	return (NUMA *)ERROR_PTR("invalid run type", procName, NULL);
    if (direction != L_HORIZ && direction != L_VERT)
	return (NUMA *)ERROR_PTR("direction not in {L_HORIZ, L_VERT}",
	                         procName, NULL);

    if (pixGetDepth(pixs) != 1)
	return (NUMA *)ERROR_PTR("pixs must be binary", procName, NULL);

    if ((na = numaCreate(0)) == NULL)
	return (NUMA *)ERROR_PTR("na not made", procName, NULL);

    if (direction == L_HORIZ)
	sel_2a = selCreateBrick(1, 2, 0, 0, 1);
    else   /* direction == L_VERT */
	sel_2a = selCreateBrick(2, 1, 0, 0, 1);
    if (!sel_2a)
	return (NUMA *)ERROR_PTR("sel_2a not made", procName, NULL);

    if (runtype == L_RUN_OFF) {
	if ((pixt1 = pixCopy(NULL, pixs)) == NULL)
	    return (NUMA *)ERROR_PTR("pix1 not made", procName, NULL);
	pixInvert(pixt1, pixt1);
    }
    else  /* runtype == L_RUN_ON */
	pixt1 = pixClone(pixs);

    if ((pixt2 = pixCreateTemplate(pixs)) == NULL)
	return (NUMA *)ERROR_PTR("pix2 not made", procName, NULL);
    if ((pixt3 = pixCreateTemplate(pixs)) == NULL)
	return (NUMA *)ERROR_PTR("pix3 not made", procName, NULL);

	/* get pixel counts at different stages of erosion */
    pixCountPixels(pixt1, &count, NULL);
    numaAddNumber(na, count);
    pixErode(pixt2, pixt1, sel_2a);
    pixCountPixels(pixt2, &count, NULL);
    numaAddNumber(na, count);
    for (i = 0; i < maxsize / 2; i++) {
	pixErode(pixt3, pixt2, sel_2a);
	pixCountPixels(pixt3, &count, NULL);
	numaAddNumber(na, count);
	pixErode(pixt2, pixt3, sel_2a);
	pixCountPixels(pixt2, &count, NULL);
	numaAddNumber(na, count);
/*	if (i == 4) pixWrite("junkoutt", pixt2, IFF_PNG); */
    }

	/* compute length histogram */
    if ((nah = numaCreate(na->n)) == NULL)
	return (NUMA *)ERROR_PTR("nah not made", procName, NULL);
    numaAddNumber(nah, 0); /* number at length 0 */
    for (i = 1; i < na->n - 1; i++) {
	val = na->array[i+1] - 2 * na->array[i] + na->array[i-1];
	numaAddNumber(nah, val);
/*        fprintf(stderr, "i = %d, val = %f\n", i, nah->array[i]); */
    }

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    selDestroy(&sel_2a);
    numaDestroy(&na);

    return nah;
}


/*-----------------------------------------------------------------*
 *            Composite operations on grayscale images             *
 *-----------------------------------------------------------------*/
/*!
 *  pixTophat()
 *
 *      Input:  pixs
 *              hsize (of Sel; must be odd; origin implicitly in center)
 *              vsize (ditto)
 *              type   (TOPHAT_WHITE: image - opening
 *                      TOPHAT_BLACK: closing - image)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Sel is a brick with all elements being hits
 *      (2) If hsize = vsize = 1, returns an image with all 0 data.
 *      (3) The TOPHAT_WHITE flag emphasizes small bright regions,
 *          whereas the TOPHAT_BLACK flag emphasizes small dark regions.
 *          The TOPHAT_WHITE tophat can be accomplished by doing a
 *          TOPHAT_BLACK tophat on the inverse, or v.v.
 */
PIX *
pixTophat(PIX     *pixs,
	  l_int32  hsize,
	  l_int32  vsize,
	  l_int32  type)
{
PIX  *pixd;

    PROCNAME("pixTophat");

    if (!pixs)
	return (PIX *)ERROR_PTR("seed pix not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
	return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (hsize < 1 || vsize < 1)
	return (PIX *)ERROR_PTR("hsize or vsize < 1", procName, NULL);
    if ((hsize & 1) == 0 ) {
	L_WARNING("horiz sel size must be odd; increasing by 1", procName);
	hsize++;
    }
    if ((vsize & 1) == 0 ) {
	L_WARNING("vert sel size must be odd; increasing by 1", procName);
	vsize++;
    }
    if (type != TOPHAT_WHITE && type != TOPHAT_BLACK)
	return (PIX *)ERROR_PTR("type must be TOPHAT_BLACK or TOPHAT_WHITE",
                                procName, NULL);

    if (hsize == 1 && vsize == 1)
	return pixCreateTemplate(pixs);

    switch (type)
    {
    case TOPHAT_WHITE:
	if ((pixd = pixOpenGray(pixs, hsize, vsize)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
	pixSubtractGray(pixd, pixs, pixd);
	break;
    case TOPHAT_BLACK:
	if ((pixd = pixCloseGray(pixs, hsize, vsize)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
	pixSubtractGray(pixd, pixd, pixs);
	break;
    default:
        return (PIX *)ERROR_PTR("invalid type", procName, NULL);
    }

    return pixd;
}
	

/*!
 *  pixHDome()
 *
 *      Input:  pixs
 *              height (of hdome; should be > 0)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) We use connectivity of 4 for the fill.
 *      (2) pixd starts as a seed, with every pixel value set to be
 *          'height' less than the corresponding pixel in pixs.  Then
 *          do a grayscale seed fill, clipping pixd to the mask pixs.
 *          Finally, subtract the result from the original.
 */
PIX *
pixHDome(PIX     *pixs,
	 l_int32  height)
{
PIX  *pixd;

    PROCNAME("pixHDome");

    if (!pixs)
	return (PIX *)ERROR_PTR("src pix not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
	return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);

    if ((pixd = pixCopy(NULL, pixs)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixAddConstantGray(pixd, - L_ABS(height));
    pixSeedfillGray(pixd, pixs, 4);
    pixSubtractGray(pixd, pixs, pixd);

    return pixd;
}


/*!
 *  pixMorphGradient()
 *
 *      Input:  pixs
 *              hsize (of Sel; must be odd; origin implicitly in center)
 *              vsize (ditto)
 *              smoothing  (half-width of convolution smoothing filter.
 *                          The width is (2 * smoothing + 1), so 0 is no-op.
 *      Return: pixd, or null on error
 */
PIX *
pixMorphGradient(PIX     *pixs,
	         l_int32  hsize,
	         l_int32  vsize,
	         l_int32  smoothing)
{
PIX  *pixg, *pixd;

    PROCNAME("pixMorphGradient");

    if (!pixs)
	return (PIX *)ERROR_PTR("seed pix not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
	return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (hsize < 1 || vsize < 1)
	return (PIX *)ERROR_PTR("hsize or vsize < 1", procName, NULL);
    if ((hsize & 1) == 0 ) {
	L_WARNING("horiz sel size must be odd; increasing by 1", procName);
	hsize++;
    }
    if ((vsize & 1) == 0 ) {
	L_WARNING("vert sel size must be odd; increasing by 1", procName);
	vsize++;
    }

        /* optionally smooth first to remove noise.
	 * If smoothing is 0, just get a copy */
    pixg = pixBlockconvGray(pixs, NULL, smoothing, smoothing);

        /* this gives approximately the gradient of a transition */
    pixd = pixDilateGray(pixg, hsize, vsize);
    pixSubtractGray(pixd, pixd, pixg);
    pixDestroy(&pixg);
    return pixd;
}


/*-----------------------------------------------------------------*
 *                            Center of mass                       *
 *-----------------------------------------------------------------*/
/*!
 *  pixaCentroids()
 *
 *      Input:  pixa of components
 *      Return: pta of centroids relative to the UL corner of
 *              each pix, or null on error
 *
 *  Notes:
 *      (1) It is assumed that all pix are the same depth.
 *      (2) Only depths of 1 and 8 bpp are allowed
 */
PTA *
pixaCentroids(PIXA  *pixa)
{
l_int32    d, i, j, k, n, w, h, wpl, pixsum, val;
l_float32  xsum, ysum, xave, yave;
l_uint32  *data, *line;
PIX       *pix;
PTA       *pta;

    PROCNAME("pixaCentroids");

    if (!pixa)
	return (PTA *)ERROR_PTR("pixa not defined", procName, NULL);
    if ((n = pixaGetCount(pixa)) == 0)
	return (PTA *)ERROR_PTR("no pix in pixa", procName, NULL);
    pix = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pix);
    pixDestroy(&pix);
    if (d != 1 && d != 8)
	return (PTA *)ERROR_PTR("depth not 1 or 8 bpp", procName, NULL);

    if ((pta = ptaCreate(n)) == NULL)
	return (PTA *)ERROR_PTR("pta not defined", procName, NULL);

    for (k = 0; k < n; k++) {
	pix = pixaGetPix(pixa, k, L_CLONE);
	w = pixGetWidth(pix);
	h = pixGetHeight(pix);
	data = pixGetData(pix);
	wpl = pixGetWpl(pix);
	xsum = ysum = 0.0;
	pixsum = 0;
        if (d == 1) {
            for (i = 0; i < h; i++) {
                line = data + wpl * i;
                for (j = 0; j < w; j++) {
                    if (GET_DATA_BIT(line, j)) {
                        xsum += j;
                        ysum += i;
                        pixsum++;
                    }
                }
            }
            if (pixsum == 0) {
                L_WARNING("no ON pixels in pix", procName);
                ptaAddPt(pta, 0.0, 0.0);   /* this shouldn't happen */
            }
            else {
                xave = xsum / (l_float32)pixsum;
                yave = ysum / (l_float32)pixsum;
                ptaAddPt(pta, xave, yave);
            }
        }
        else {  /* d == 8 */
            for (i = 0; i < h; i++) {
                line = data + wpl * i;
                for (j = 0; j < w; j++) {
                    val = GET_DATA_BYTE(line, j);
                    xsum += val * j;
                    ysum += val * i;
                    pixsum += val;
                }
            }
            if (pixsum == 0) {
                L_WARNING("all pixels are 0", procName);
                ptaAddPt(pta, 0.0, 0.0);   /* this shouldn't happen */
            }
            else {
                xave = xsum / (l_float32)pixsum;
                yave = ysum / (l_float32)pixsum;
                ptaAddPt(pta, xave, yave);
            }
        }
	pixDestroy(&pix);
    }

    return pta;
}

