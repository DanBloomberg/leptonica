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
 *      Un-normalized grayscale block convolution
 *          PIX      *pixBlockconvGrayUnnormalized()
 *
 *      Binary block sum and rank filter
 *          PIX      *pixBlockrank()
 *          PIX      *pixBlocksum()
 *
 *      Woodfill transform
 *          PIX      *pixWoodfillTransform()
 *
 *      Generic convolution (with Pix)
 *          PIX      *pixConvolve()
 *          PIX      *pixConvolveSep()
 *
 *      Generic convolution (with float arrays)
 *          FPIX     *fpixConvolve()
 *          FPIX     *fpixConvolveSep()
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
 *      Return: pix (8 bpp), or null on error
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
 *               Un-normalized grayscale block convolution              *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockconvGrayUnnormalized()
 *
 *      Input:  pixs (8 bpp)
 *              wc, hc   (half width/height of convolution kernel)
 *      Return: pix (32 bpp; containing the convolution without normalizing
 *                   for the window size), or null on error
 *
 *  Notes:
 *      (1) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1).
 *      (2) Returns an error if both wc and hc are 0.
 *      (3) Adds mirrored border to avoid treating the boundary pixels
 *          specially.  Note that we add wc + 1 pixels to the left
 *          and wc to the right.  The added width is 2 * wc + 1 pixels,
 *          and the particular choice simplifies the indexing in the loop.
 *          Likewise, add hc + 1 pixels to the top and hc to the bottom.
 *      (4) To get the normalized result, divide by the area of the
 *          convolution kernel: (2 * wc + 1) * (2 * hc + 1)
 *          Specifically, do this:
 *               pixc = pixBlockconvGrayUnnormalized(pixs, wc, hc);
 *               fract = 1. / ((2 * wc + 1) * (2 * hc + 1));
 *               pixMultConstantGray(pixc, fract);
 *               pixd = pixGetRGBComponent(pixc, L_ALPHA_CHANNEL);
 *      (5) Unlike pixBlockconvGray(), this always computes the accumulation
 *          pix because its size is tied to wc and hc.
 *      (6) Compare this implementation with pixBlockconvGray(), where
 *          most of the code in blockconvLow() is special casing for
 *          efficiently handling the boundary.  Here, the use of
 *          mirrored borders and destination indexing makes the
 *          implementation very simple.
 */
PIX *
pixBlockconvGrayUnnormalized(PIX     *pixs,
                             l_int32  wc,
                             l_int32  hc)
{
l_int32    i, j, w, h, d, wpla, wpld, jmax;
l_uint32  *linemina, *linemaxa, *lined, *dataa, *datad;
PIX       *pixsb, *pixacc, *pixd;

    PROCNAME("pixBlockconvGrayUnnormalized");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (wc < 0) wc = 0;
    if (hc < 0) hc = 0;
    if (wc == 0 && hc == 0)   /* no-op */
        return (PIX *)ERROR_PTR("both wc and hc are 0", procName, NULL);
    if (w <= wc || h <= hc)
        L_WARNING("conv kernel half-size >= image dimension!", procName);

    if ((pixsb = pixAddMirroredBorder(pixs, wc + 1, wc, hc + 1, hc)) == NULL)
        return (PIX *)ERROR_PTR("pixsb not made", procName, NULL);
    if ((pixacc = pixBlockconvAccum(pixsb)) == NULL)
        return (PIX *)ERROR_PTR("pixacc not made", procName, NULL);
    pixDestroy(&pixsb);
    if ((pixd = pixCreate(w, h, 32)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    
    wpla = pixGetWpl(pixacc);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);
    dataa = pixGetData(pixacc);
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        linemina = dataa + i * wpla;
        linemaxa = dataa + (i + 2 * hc + 1) * wpla;
        for (j = 0; j < w; j++) {
            jmax = j + 2 * wc + 1;
            lined[j] = linemaxa[jmax] - linemaxa[j] -
                       linemina[jmax] + linemina[j];
        }
    }

    pixDestroy(&pixacc);
    return pixd;
}


/*----------------------------------------------------------------------*
 *                        Binary block sum/rank                         *
 *----------------------------------------------------------------------*/
/*!
 *  pixBlockrank()
 *
 *      Input:  pixs (1 bpp)
 *              accum pix (<optional> 32 bpp)
 *              wc, hc   (half width/height of block sum/rank kernel)
 *              rank   (between 0.0 and 1.0; 0.5 is median filter)
 *      Return: pixd (1 bpp)
 *
 *  Notes:
 *      (1) The full width and height of the convolution kernel
 *          are (2 * wc + 1) and (2 * hc + 1)
 *      (2) This returns a pixd where each pixel is a 1 if the
 *          neighborhood (2 * wc + 1) x (2 * hc + 1)) pixels
 *          contains the rank fraction of 1 pixels.  Otherwise,
 *          the returned pixel is 0.  Note that the special case
 *          of rank = 0.0 is always satisfied, so the returned
 *          pixd has all pixels with value 1.
 *      (3) If accum pix is null, make one, use it, and destroy it
 *          before returning; otherwise, just use the input accum pix
 *      (4) If both wc and hc are 0, returns a copy unless rank == 0.0,
 *          in which case this returns an all-ones image.
 *      (5) Require that wc < w and hc < h, where (w,h) are the dimensions
 *          of pixs.
 */
PIX *
pixBlockrank(PIX       *pixs,
             PIX       *pixacc,
             l_int32    wc,
             l_int32    hc,
             l_float32  rank)
{
l_int32  w, h, d, thresh;
PIX     *pixt, *pixd;

    PROCNAME("pixBlockrank");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (rank < 0.0 || rank > 1.0)
        return (PIX *)ERROR_PTR("rank must be in [0.0, 1.0]", procName, NULL);
    if (wc < 0 || hc < 0)
        return (PIX *)ERROR_PTR("wc and hc not both >= 0", procName, NULL);
    if (rank == 0.0) {
        pixd = pixCreateTemplate(pixs);
        pixSetAll(pixd);
    }

    if (wc == 0 && hc == 0) {
        L_WARNING("block of unit size", procName);
        return pixCopy(NULL, pixs);
    }
    if (w <= wc || h <= hc) {
        L_WARNING("conv kernel half-size >= image dimension!", procName);
        return pixCopy(NULL, pixs);
    }

    if ((pixt = pixBlocksum(pixs, pixacc, wc, hc)) == NULL)
        return (PIX *)ERROR_PTR("pixt not made", procName, NULL);

        /* 1 bpp block rank filter output.
         * Must invert because threshold gives 1 for values < thresh,
         * but we need a 1 if the value is >= thresh. */
    thresh = (l_int32)(255. * rank);
    pixd = pixThresholdToBinary(pixt, thresh);
    pixInvert(pixd, pixd);
    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixBlocksum()
 *
 *      Input:  pixs (1 bpp)
 *              accum pix (<optional> 32 bpp)
 *              wc, hc   (half width/height of block sum/rank kernel)
 *      Return: pixd (8 bpp)
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
l_int32    w, h, d, wplt, wpld;
l_uint32  *datat, *datad;
PIX       *pixt, *pixd;

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
    if ((pixd = pixCreate(w, h, 8)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);

    wpld = pixGetWpl(pixd);
    wplt = pixGetWpl(pixt);
    datad = pixGetData(pixd);
    datat = pixGetData(pixt);
    blocksumLow(datad, w, h, wpld, datat, wplt, wc, hc);

    pixDestroy(&pixt);
    return pixd;
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
        

/*----------------------------------------------------------------------*
 *                         Generic convolution                          *
 *----------------------------------------------------------------------*/
/*!
 *  pixConvolve()
 *
 *      Input:  pixs (8, 16, 32 bpp; no colormap)
 *              kernel
 *              outdepth (of pixd: 8, 16 or 32)
 *              normflag (1 to normalize kernel to unit sum; 0 otherwise)
 *      Return: pixd (8, 16 or 32 bpp)
 *
 *  Notes:
 *      (1) This gives a convolution with an arbitrary kernel.
 *      (2) The parameter @outdepth determines the depth of the result.
 *      (3) If normflag == 1, the result is normalized by scaling
 *          all kernel values for a unit sum.  Do not normalize if
 *          the kernel has null sum, such as a DoG.
 *      (4) If the kernel is normalized to unit sum, the output values
 *          can never exceed 255, so an output depth of 8 bpp is sufficient.
 *          If the kernel is not normalized, it may be necessary to use
 *          16 or 32 bpp output to avoid overflow.
 *      (5) The kernel values can be positive or negative, but the
 *          result for the convolution can only be stored as a positive
 *          number.  Consequently, if it goes negative, the choices are
 *          to clip to 0 or take the absolute value.  We're choosing
 *          the former for now.  Another possibility would be to output
 *          a second unsigned image for the negative values.
 *      (6) This uses a mirrored border to avoid special casing on
 *          the boundaries.
 *      (7) The function is slow, running at about 12 machine cycles for
 *          each pixel-op in the convolution.  For example, with a 3 GHz
 *          cpu, a 1 Mpixel grayscale image, and a kernel with
 *          (sx * sy) = 25 elements, the convolution takes about 100 msec.
 */
PIX *
pixConvolve(PIX       *pixs,
            L_KERNEL  *kel,
	    l_int32    outdepth,
	    l_int32    normflag)
{
l_int32    i, j, k, m, w, h, d, sx, sy, cx, cy, wplt, wpld;
l_int32    val;
l_uint32  *datat, *datad, *linet, *lined;
l_float32  sum;
L_KERNEL  *keli, *keln;
PIX       *pixt, *pixd;

    PROCNAME("pixConvolve");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetColormap(pixs))
        return (PIX *)ERROR_PTR("pixs has colormap", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8 && d != 16 && d != 32)
        return (PIX *)ERROR_PTR("pixs not 8, 16, or 32 bpp", procName, NULL);
    if (!kel)
        return (PIX *)ERROR_PTR("kel not defined", procName, NULL);

    keli = kernelInvert(kel);
    kernelGetParameters(keli, &sy, &sx, &cy, &cx);
    if (normflag)
        keln = kernelNormalize(keli, 1.0);
    else
        keln = kernelCopy(keli);

    if ((pixt = pixAddMirroredBorder(pixs, cx, sx - cx, cy, sy - cy)) == NULL)
        return (PIX *)ERROR_PTR("pixt not made", procName, NULL);

    pixd = pixCreate(w, h, outdepth);
    datat = pixGetData(pixt);
    datad = pixGetData(pixd);
    wplt = pixGetWpl(pixt);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            sum = 0.0;
            for (k = 0; k < sy; k++) {
                linet = datat + (i + k) * wplt;
                if (d == 8) {
                    for (m = 0; m < sx; m++) {
                        val = GET_DATA_BYTE(linet, j + m);
                        sum += val * keln->data[k][m];
                    }
                }
                else if (d == 16) {
                    for (m = 0; m < sx; m++) {
                        val = GET_DATA_TWO_BYTES(linet, j + m);
                        sum += val * keln->data[k][m];
                    }
                }
                else {  /* d == 32 */
                    for (m = 0; m < sx; m++) {
                        val = *(linet + j + m);
                        sum += val * keln->data[k][m];
                    }
                }
            }
#if 1
	    if (sum < 0.0) sum = -sum;  /* make it non-negative */
#endif
            if (outdepth == 8)
                SET_DATA_BYTE(lined, j, (l_int32)(sum + 0.5));
            else if (outdepth == 16)
                SET_DATA_TWO_BYTES(lined, j, (l_int32)(sum + 0.5));
            else  /* outdepth == 32 */
                *(lined + j) = (l_uint32)(sum + 0.5);
        }
    }

    kernelDestroy(&keli);
    kernelDestroy(&keln);
    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixConvolveSep()
 *
 *      Input:  pixs (8 bpp)
 *              kelx (x-dependent kernel)
 *              kely (y-dependent kernel)
 *              outdepth (of pixd: 8, 16 or 32)
 *              normflag (1 to normalize kernel to unit sum; 0 otherwise)
 *      Return: pixd (8, 16 or 32 bpp)
 *
 *  Notes:
 *      (1) This does a convolution with a separable kernel that is
 *          is a sequence of convolutions in x and y.  The two
 *          one-dimensional kernel components must be input separately;
 *          the full kernel is the product of these components.
 *          The support for the full kernel is thus a rectangular region.
 *      (2) The @outdepth and @normflag parameters are used as in
 *          pixConvolve().
 *      (3) If the kernel is normalized to unit sum, the output values
 *          can never exceed 255, so an output depth of 8 bpp is sufficient.
 *          If the kernel is not normalized, it may be necessary to use
 *          16 or 32 bpp output to avoid overflow.
 *      (4) The kernel values can be positive or negative, but the
 *          result for the convolution can only be stored as a positive
 *          number.  Consequently, if it goes negative, the choices are
 *          to clip to 0 or take the absolute value.  We're choosing
 *          the former for now.  Another possibility would be to output
 *          a second unsigned image for the negative values.
 *      (5) This uses mirrored borders to avoid special casing on
 *          the boundaries.
 */
PIX *
pixConvolveSep(PIX       *pixs,
               L_KERNEL  *kelx,
               L_KERNEL  *kely,
               l_int32    outdepth,
               l_int32    normflag)
{
l_int32    w, h, d;
L_KERNEL  *kelxn, *kelyn;
PIX       *pixt, *pixd;

    PROCNAME("pixConvolveSep");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8 && d != 16 && d != 32)
        return (PIX *)ERROR_PTR("pixs not 8, 16, or 32 bpp", procName, NULL);
    if (!kelx)
        return (PIX *)ERROR_PTR("kelx not defined", procName, NULL);
    if (!kely)
        return (PIX *)ERROR_PTR("kely not defined", procName, NULL);

    if (normflag) {
        kelxn = kernelNormalize(kelx, 1000.0);
        kelyn = kernelNormalize(kely, 0.001);
        pixt = pixConvolve(pixs, kelxn, 32, 0);
        pixd = pixConvolve(pixt, kelyn, outdepth, 0);
        kernelDestroy(&kelxn);
        kernelDestroy(&kelyn);
    }
    else {  /* don't normalize */
        pixt = pixConvolve(pixs, kelx, 32, 0);
        pixd = pixConvolve(pixt, kely, outdepth, 0);
    }

    pixDestroy(&pixt);
    return pixd;
}


/*----------------------------------------------------------------------*
 *                  Generic convolution with float array                *
 *----------------------------------------------------------------------*/
/*!
 *  fpixConvolve()
 *
 *      Input:  fpixs (32 bit float array)
 *              kernel
 *              normflag (1 to normalize kernel to unit sum; 0 otherwise)
 *      Return: fpixd (32 bit float array)
 *
 *  Notes:
 *      (1) This gives a float convolution with an arbitrary kernel.
 *      (2) If normflag == 1, the result is normalized by scaling
 *          all kernel values for a unit sum.  Do not normalize if
 *          the kernel has null sum, such as a DoG.
 *      (3) With the FPix, there are no issues about negative
 *          array or kernel values.  The convolution is performed
 *          with single precision arithmetic.
 *      (4) This uses a mirrored border to avoid special casing on
 *          the boundaries.
 */
FPIX *
fpixConvolve(FPIX      *fpixs,
             L_KERNEL  *kel,
	     l_int32    normflag)
{
l_int32     i, j, k, m, w, h, sx, sy, cx, cy, wplt, wpld;
l_float32   val;
l_float32  *datat, *datad, *linet, *lined;
l_float32   sum;
L_KERNEL   *keli, *keln;
FPIX       *fpixt, *fpixd;

    PROCNAME("fpixConvolve");

    if (!fpixs)
        return (FPIX *)ERROR_PTR("fpixs not defined", procName, NULL);
    if (!kel)
        return (FPIX *)ERROR_PTR("kel not defined", procName, NULL);

    keli = kernelInvert(kel);
    kernelGetParameters(keli, &sy, &sx, &cy, &cx);
    if (normflag)
        keln = kernelNormalize(keli, 1.0);
    else
        keln = kernelCopy(keli);

    fpixGetDimensions(fpixs, &w, &h);
    fpixt = fpixAddMirroredBorder(fpixs, cx, sx - cx, cy, sy - cy);
    if (!fpixt)
        return (FPIX *)ERROR_PTR("fpixt not made", procName, NULL);

    fpixd = fpixCreate(w, h);
    datat = fpixGetData(fpixt);
    datad = fpixGetData(fpixd);
    wplt = fpixGetWpl(fpixt);
    wpld = fpixGetWpl(fpixd);
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            sum = 0.0;
            for (k = 0; k < sy; k++) {
                linet = datat + (i + k) * wplt;
                for (m = 0; m < sx; m++) {
                    val = *(linet + j + m);
                    sum += val * keln->data[k][m];
                }
            }
            *(lined + j) = sum;
        }
    }

    kernelDestroy(&keli);
    kernelDestroy(&keln);
    fpixDestroy(&fpixt);
    return fpixd;
}


/*!
 *  fpixConvolveSep()
 *
 *      Input:  fpixs (32 bit float array)
 *              kelx (x-dependent kernel)
 *              kely (y-dependent kernel)
 *              normflag (1 to normalize kernel to unit sum; 0 otherwise)
 *      Return: fpixd (32 bit float array)
 *
 *  Notes:
 *      (1) This does a convolution with a separable kernel that is
 *          is a sequence of convolutions in x and y.  The two
 *          one-dimensional kernel components must be input separately;
 *          the full kernel is the product of these components.
 *          The support for the full kernel is thus a rectangular region.
 *      (2) The normflag parameter is used as in fpixConvolve().
 *      (3) This uses mirrored borders to avoid special casing on
 *          the boundaries.
 */
FPIX *
fpixConvolveSep(FPIX      *fpixs,
                L_KERNEL  *kelx,
                L_KERNEL  *kely,
                l_int32    normflag)
{
L_KERNEL  *kelxn, *kelyn;
FPIX      *fpixt, *fpixd;

    PROCNAME("fpixConvolveSep");

    if (!fpixs)
        return (FPIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!kelx)
        return (FPIX *)ERROR_PTR("kelx not defined", procName, NULL);
    if (!kely)
        return (FPIX *)ERROR_PTR("kely not defined", procName, NULL);

    if (normflag) {
        kelxn = kernelNormalize(kelx, 1.0);
        kelyn = kernelNormalize(kely, 1.0);
        fpixt = fpixConvolve(fpixs, kelxn, 0);
        fpixd = fpixConvolve(fpixt, kelyn, 0);
        kernelDestroy(&kelxn);
        kernelDestroy(&kelyn);
    }
    else {  /* don't normalize */
        fpixt = fpixConvolve(fpixs, kelx, 0);
        fpixd = fpixConvolve(fpixt, kely, 0);
    }

    fpixDestroy(&fpixt);
    return fpixd;
}



