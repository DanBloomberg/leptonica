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
 *  convolve.c
 *
 *      Top level grayscale or color block convolution
 *          PIX      *pixBlockconv()
 *
 *      Color block convolution
 *          PIX      *pixBlockconvColor()
 *          
 *      Grayscale block convolution
 *          PIX      *pixBlockconvGray()
 *          PIX      *pixBlockconvAccum()
 *
 *      Binary block sum and rank filter
 *          PIX      *pixBlockrank()
 *          PIX      *pixBlocksum()
 *
 *      Woodfill transform
 *          PIX      *pixWoodfillTransform()
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


/*----------------------------------------------------------------------*
 *             Top-level grayscale or color block convolution           *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockconv()
 *
 *      Input:  pix (8 or 32 bpp; or 2, 4 or 8 bpp with colormap)
 *              wc, hc   (half width/height of convolution kernel)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1)
 *      (2) Returns a copy if both wc and hc are 0
 *      (3) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 */
PIX  *
pixBlockconv(PIX     *pix,
             l_int32  wc,
             l_int32  hc)
{
l_int32  w, h, d;
PIX     *pixs, *pixd;

    PROCNAME("pixBlockconv");

    if (!pix)
        return (PIX *)ERROR_PTR("pix not defined", procName, NULL);
    if (wc < 0) wc = 0;
    if (hc < 0) hc = 0;
    if (wc == 0 && hc == 0)   /* no-op */
        return pixCopy(NULL, pix);
    pixGetDimensions(pix, &w, &h, &d);
    if (w <= wc || h <= hc) {
        L_WARNING("conv kernel half-size >= image dimension!", procName);
        return pixCopy(NULL, pix);
    }

        /* Remove colormap if necessary */ 
    if ((d == 2 || d == 4 || d == 8) && pixGetColormap(pix)) {
        L_WARNING("pix has colormap; removing", procName);
        pixs = pixRemoveColormap(pix, REMOVE_CMAP_BASED_ON_SRC);
        d = pixGetDepth(pixs);
    }
    else
        pixs = pixClone(pix);

    if (d != 8 && d != 32) {
        pixDestroy(&pixs);
        return (PIX *)ERROR_PTR("depth not 8 or 32 bpp", procName, NULL);
    }

    if (d == 8)
        pixd = pixBlockconvGray(pixs, NULL, wc, hc);
    else  /* d == 32 */
        pixd = pixBlockconvColor(pixs, wc, hc);
    pixDestroy(&pixs);

    return pixd;
}


/*----------------------------------------------------------------------*
 *                        Color block convolution                       *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockconvColor()
 *
 *      Input:  pixs (32 bpp; 24 bpp RGB color)
 *              wc, hc   (half width/height of convolution kernel)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1)
 *      (2) Returns a copy if both wc and hc are 0.
 *      (3) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 */
PIX  *
pixBlockconvColor(PIX     *pixs,
                  l_int32  wc,
                  l_int32  hc)
{
l_int32  w, h, d;
PIX     *pixRed, *pixGreen, *pixBlue;
PIX     *pixRedConv, *pixGreenConv, *pixBlueConv;
PIX     *pixd;

    PROCNAME("pixBlockconvColor");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 32)
        return (PIX *)ERROR_PTR("pix not 32 bpp", procName, NULL);
    if (wc < 0) wc = 0;
    if (hc < 0) hc = 0;
    if (wc == 0 && hc == 0)   /* no-op */
        return pixCopy(NULL, pixs);
    if (w <= wc || h <= hc) {
        L_WARNING("conv kernel half-size >= image dimension!", procName);
        return pixCopy(NULL, pixs);
    }

    pixRed = pixGetRGBComponent(pixs, COLOR_RED);
    pixRedConv = pixBlockconvGray(pixRed, NULL, wc, hc);
    pixDestroy(&pixRed);
    pixGreen = pixGetRGBComponent(pixs, COLOR_GREEN);
    pixGreenConv = pixBlockconvGray(pixGreen, NULL, wc, hc);
    pixDestroy(&pixGreen);
    pixBlue = pixGetRGBComponent(pixs, COLOR_BLUE);
    pixBlueConv = pixBlockconvGray(pixBlue, NULL, wc, hc);
    pixDestroy(&pixBlue);

    if ((pixd = pixCreateRGBImage(pixRedConv, pixGreenConv, pixBlueConv))
            == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    pixDestroy(&pixRedConv);
    pixDestroy(&pixGreenConv);
    pixDestroy(&pixBlueConv);

    return pixd;
}



/*----------------------------------------------------------------------*
 *                     Grayscale block convolution                      *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockconvGray()
 *
 *      Input:  pix (8 bpp)
 *              accum pix (32 bpp; can be null)
 *              wc, hc   (half width/height of convolution kernel)
 *      Return: pix (8 bpp)
 *
 *  Notes:
 *      (1) If accum pix is null, make one and destroy it before
 *          returning; otherwise, just use the input accum pix.
 *      (2) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1).
 *      (3) Returns a copy if both wc and hc are 0.
 *      (4) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 */
PIX *
pixBlockconvGray(PIX     *pixs,
                 PIX     *pixacc,
                 l_int32  wc,
                 l_int32  hc)
{
l_int32    w, h, d, wpl, wpla;
l_uint32  *datad, *dataa;
PIX       *pixd, *pixt;

    PROCNAME("pixBlockconvGray");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (wc < 0) wc = 0;
    if (hc < 0) hc = 0;
    if (wc == 0 && hc == 0)   /* no-op */
        return pixCopy(NULL, pixs);
    if (w <= wc || h <= hc) {
        L_WARNING("conv kernel half-size >= image dimension!", procName);
        return pixCopy(NULL, pixs);
    }

    if (pixacc) {
        if (pixGetDepth(pixacc) == 32)
            pixt = pixClone(pixacc);
        else {
            L_WARNING("pixacc not 32 bpp; making new one", procName);
            if ((pixt = pixBlockconvAccum(pixs)) == NULL)
                return (PIX *)ERROR_PTR("pixt not made", procName, NULL);
        }
    }
    else {
        if ((pixt = pixBlockconvAccum(pixs)) == NULL)
            return (PIX *)ERROR_PTR("pixt not made", procName, NULL);
    }
        
    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    
    wpl = pixGetWpl(pixs);
    wpla = pixGetWpl(pixt);
    datad = pixGetData(pixd);
    dataa = pixGetData(pixt);
    blockconvLow(datad, w, h, wpl, dataa, wpla, wc, hc);

    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixBlockconvAccum()
 *
 *      Input:  pixs (1 or 8 bpp)
 *      Return: accum pix (32 bpp), or null on error.
 *
 *  Notes:
 *      (1) The general recursion relation is
 *            a(i,j) = v(i,j) + a(i-1, j) + a(i, j-1) - a(i-1, j-1)
 *          For the first line, this reduces to the special case
 *            a(i,j) = v(i,j) + a(i, j-1)
 *          For the first column, the special case is
 *            a(i,j) = v(i,j) + a(i-1, j)
 */
PIX *
pixBlockconvAccum(PIX  *pixs)
{
l_int32    w, h, d, wpls, wpld;
l_uint32  *datas, *datad;
PIX       *pixd;

    PROCNAME("pixBlockconvAccum");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 1 && d != 8)
        return (PIX *)ERROR_PTR("pixs not 1 or 8 bpp", procName, NULL);
    if ((pixd = pixCreate(w, h, 32)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    datas = pixGetData(pixs);
    datad = pixGetData(pixd);
    wpls = pixGetWpl(pixs);
    wpld = pixGetWpl(pixd);
    blockconvAccumLow(datad, w, h, wpld, datas, d, wpls);

    return pixd;
}


/*----------------------------------------------------------------------*
 *                        Binary block sum/rank                         *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockrank()
 *
 *      Input:  pix (1 bpp)
 *              accum pix (<optional> 32 bpp)
 *              wc, hc   (half width/height of block sum/rank kernel)
 *              rank   (between 0.0 and 1.0; 0.5 is median filter)
 *      Return: pix (1 bpp)
 *
 *  Notes:
 *      (1) If accum pix is null, make one, use it, and destroy it
 *          before returning; otherwise, just use the input accum pix
 *      (2) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1)
 *      (3) If both wc and hc are 0, returns a copy unless rank == 0.0,
 *          in which case this returns an all-set image.
 *      (4) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 */
PIX *
pixBlockrank(PIX       *pixs,
             PIX       *pixacc,
             l_int32    wc,
             l_int32    hc,
             l_float32  rank)
{
l_int32    i, j, w, h, d, wplbs, wpld;
l_uint8    thresh, val;
l_uint32  *databs, *datad, *linebs, *lined;
PIX       *pixbs, *pixd;

    PROCNAME("pixBlockrank");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (rank < 0.0 || rank > 1.0)
        return (PIX *)ERROR_PTR("rank must be in [0.0, 1.0]", procName, NULL);
    if (wc < 0) wc = 0;
    if (hc < 0) hc = 0;
    if (wc == 0 && hc == 0) {
        L_WARNING("block of unit size", procName);
        if (rank == 0.0) {
            pixd = pixCreateTemplate(pixs);
            pixSetAll(pixd);
        }
        else
            pixd = pixCopy(NULL, pixs);
        return pixd;
    }
    if (w <= wc || h <= hc) {
        L_WARNING("conv kernel half-size >= image dimension!", procName);
        return pixCopy(NULL, pixs);
    }

    if ((pixbs = pixBlocksum(pixs, pixacc, wc, hc)) == NULL)
        return (PIX *)ERROR_PTR("block sum pix not made", procName, NULL);

        /* 1 bpp block rank filter output */
    if ((pixd = pixCreateTemplate(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    databs = pixGetData(pixbs);
    wplbs = pixGetWpl(pixbs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    
    thresh = (l_int32)(rank * (l_float32)255);
    for (i = 0; i < h; i++) {
        linebs = databs + i * wplbs;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            val = GET_DATA_BYTE(linebs, j);
            if (val >= thresh)
                SET_DATA_BIT(lined, j);
        }
    }

    pixDestroy(&pixbs);
    return pixd;
}


/*!
 *  pixBlocksum()
 *
 *      Input:  pixs (1 bpp)
 *              accum pix (<optional> 32 bpp)
 *              wc, hc   (half width/height of block sum/rank kernel)
 *      Return: pix (8 bpp)
 *
 *  Notes:
 *      (1) If accum pix is null, make one and destroy it before
 *          returning; otherwise, just use the input accum pix
 *      (2) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1)
 *      (3) Use of wc = hc = 1, followed by pixInvert() on the
 *          8 bpp result, gives a nice anti-aliased, and somewhat
 *          darkened, result on text.
 *      (4) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 *      (5) Returns in each dest pixel the sum of all src pixels
 *          that are within a block of size of the kernel, centered
 *          on the dest pixel.  This sum is the number of src ON
 *          pixels in the block at each location, normalized to 255
 *          for a block containing all ON pixels.  For pixels near
 *          the boundary, where the block is not entirely contained
 *          within the image, we then multiply by a second normalization
 *          factor that is greater than one, so that all results
 *          are normalized by the number of participating pixels
 *          within the block.
 */
PIX *
pixBlocksum(PIX     *pixs,
            PIX     *pixacc,
            l_int32  wc,
            l_int32  hc)
{
l_int32    w, h, d, wplb, wpla;
l_uint32  *dataa, *datab;
PIX       *pixt, *pixb;

    PROCNAME("pixBlocksum");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (w <= wc || h <= hc)
        return (PIX *)ERROR_PTR("conv kernel half-size >= image dimension!",
                                procName, NULL);

    if (pixacc) {
        if (pixGetDepth(pixacc) != 32)
            return (PIX *)ERROR_PTR("pixacc not 32 bpp", procName, NULL);
        pixt = pixClone(pixacc);
    }
    else {
        if ((pixt = pixBlockconvAccum(pixs)) == NULL)
            return (PIX *)ERROR_PTR("pixt not made", procName, NULL);
    }
        
        /* 8 bpp block sum output */
    if ((pixb = pixCreate(w, h, 8)) == NULL)
        return (PIX *)ERROR_PTR("pixb not made", procName, NULL);
    pixCopyResolution(pixb, pixs);

    wplb = pixGetWpl(pixb);
    wpla = pixGetWpl(pixt);
    datab = pixGetData(pixb);
    dataa = pixGetData(pixt);
    blocksumLow(datab, w, h, wplb, dataa, wpla, wc, hc);

    pixDestroy(&pixt);
    return pixb;
}


/*----------------------------------------------------------------------*
 *                         Woodfill transform                           *
 *----------------------------------------------------------------------*/
/*!
 *  pixWoodfillTransform()
 *
 *      Input:  pixs (8 bpp)
 *              halfsize (of square over which neighbors are averaged)
 *              accum pix (<optional> 32 bpp)
 *      Return: pixd (1 bpp)
 *
 *  Notes:
 *      (1) The Woodfill transform compares each pixel against
 *          the average of its neighbors (in a square of odd
 *          dimension centered on the pixel).  If the pixel is
 *          greater than the average of its neighbors, the output
 *          pixel value is 1; otherwise it is 0.
 *      (2) This can be used as an encoding for an image that is
 *          fairly robust against slow illumination changes, with
 *          applications in image comparison and mosaicing.
 *      (3) The size of the convolution kernel is (2 * halfsize + 1)
 *          on a side.  The halfsize parameter must be >= 1.
 *      (4) If accum pix is null, make one, use it, and destroy it
 *          before returning; otherwise, just use the input accum pix
 */
PIX *
pixWoodfillTransform(PIX     *pixs,
                     l_int32  halfsize,
                     PIX     *pixacc)
{
l_int32    i, j, w, h, wpls, wplv, wpld;
l_int32    vals, valv;
l_uint32  *datas, *datav, *datad, *lines, *linev, *lined;
PIX       *pixav, *pixd;

    PROCNAME("pixWoodfillTransform");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (halfsize < 1)
        return (PIX *)ERROR_PTR("halfsize must be >= 1", procName, NULL);

        /* Get the average of each pixel with its neighbors */
    if ((pixav = pixBlockconvGray(pixs, pixacc, halfsize, halfsize))
          == NULL)
        return (PIX *)ERROR_PTR("pixav not made", procName, NULL);

        /* Subtract the pixel from the average, and then compare
         * the pixel value with the remaining average */
    pixGetDimensions(pixs, &w, &h, NULL);
    if ((pixd = pixCreate(w, h, 1)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    datas = pixGetData(pixs);
    datav = pixGetData(pixav);
    datad = pixGetData(pixd);
    wpls = pixGetWpl(pixs);
    wplv = pixGetWpl(pixav);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        linev = datav + i * wplv;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            vals = GET_DATA_BYTE(lines, j);
            valv = GET_DATA_BYTE(linev, j);
            if (vals > valv)
                SET_DATA_BIT(lined, j);
        }
    }

    pixDestroy(&pixav);
    return pixd;
}
        

