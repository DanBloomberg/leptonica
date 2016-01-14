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
 *  pix4.c
 *
 *    This file has these operations:
 *
 *      (1) Pixel histograms
 *      (2) Foreground/background estimation
 *      (3) Rectangle extraction
 *
 *    Pixel histogram, rank val, averaging and min/max
 *           NUMA       *pixGetGrayHistogram()
 *           NUMA       *pixGetGrayHistogramMasked()
 *           l_int32     pixGetColorHistogram()
 *           l_int32     pixGetColorHistogramMasked()
 *           l_int32     pixGetRankValueMaskedRGB()
 *           l_int32     pixGetRankValueMasked()
 *           l_int32     pixGetAverageMaskedRGB()
 *           l_int32     pixGetAverageMasked()
 *           l_int32     pixGetAverageTiledRGB()
 *           PIX        *pixGetAverageTiled()
 *           l_int32     pixGetExtremeValue()
 *           l_int32     pixGetMaxValueInRect()
 *           l_int32     pixGetRankColorArray()
 *           l_int32     pixGetBinnedColor()
 *
 *    Pixelwise aligned statistics
 *           PIX        *pixaGetAlignedStats()
 *           l_int32     pixaExtractColumnFromEachPix()
 *           l_int32     pixGetRowStats()
 *           l_int32     pixGetColumnStats()
 *           l_int32     pixSetPixelColumn()
 *
 *    Foreground/background estimation
 *           l_int32     pixThresholdForFgBg()
 *           l_int32     pixSplitDistributionFgBg()
 *
 *    Measurement of properties
 *           l_int32     pixaFindDimensions()
 *           NUMA        pixaFindAreaPerimRatio()
 *           l_int32     pixFindAreaPerimRatio()
 *           NUMA        pixaFindPerimSizeRatio()
 *           l_int32     pixFindPerimSizeRatio()
 *           NUMA        pixaFindAreaFraction()
 *           l_int32     pixFindAreaFraction()
 *           NUMA        pixaFindWidthHeightRatio()
 *           NUMA        pixaFindWidthHeightProduct()
 *
 *    Extract rectangle
 *           PIX        *pixClipRectangle()
 *           PIX        *pixClipMasked()
 *
 *    Clip to foreground
 *           PIX        *pixClipToForeground()
 *           l_int32     pixClipBoxToForeground()
 *           l_int32     pixScanForForeground()
 *           l_int32     pixClipBoxToEdges()
 *           l_int32     pixScanForEdge()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "allheaders.h"

static const l_uint32 rmask32[] = {0x0,
    0x00000001, 0x00000003, 0x00000007, 0x0000000f,
    0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
    0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
    0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff};

#ifndef  NO_CONSOLE_IO
#define  DEBUG_EDGES         0
#endif  /* ~NO_CONSOLE_IO */


/*------------------------------------------------------------------*
 *                  Pixel histogram and averaging                   *
 *------------------------------------------------------------------*/
/*!
 *  pixGetGrayHistogram()
 *
 *      Input:  pixs (1, 2, 4, 8, 16 bpp; can be colormapped)
 *              factor (subsampling factor; integer >= 1)
 *      Return: na (histogram), or null on error
 *
 *  Notes:
 *      (1) This generates a histogram of gray or cmapped pixels.
 *          The image must not be rgb.
 *      (2) If pixs does not have a colormap, the output histogram is
 *          of size 2^d, where d is the depth of pixs.
 *      (3) If pixs has has a colormap with color entries, the histogram
 *          generated is of the colormap indices, and is of size 2^d.
 *      (4) If pixs has a gray (r=g=b) colormap, a temporary 8 bpp image
 *          without the colormap is used to construct a histogram of
 *          size 256.
 *      (5) Set the subsampling factor > 1 to reduce the amount of computation.
 */
NUMA *
pixGetGrayHistogram(PIX     *pixs,
                    l_int32  factor)
{
l_int32     i, j, w, h, d, wpl, val, size, count, colorfound;
l_uint32   *data, *line;
l_float32  *array;
NUMA       *na;
PIX        *pixg;
PIXCMAP    *cmap;

    PROCNAME("pixGetGrayHistogram");

    if (!pixs)
        return (NUMA *)ERROR_PTR("pixs not defined", procName, NULL);
    d = pixGetDepth(pixs);
    if (d > 16)
        return (NUMA *)ERROR_PTR("depth not in {1,2,4,8,16}", procName, NULL);
    if (factor < 1)
        return (NUMA *)ERROR_PTR("sampling factor < 1", procName, NULL);

    if ((cmap = pixGetColormap(pixs)) != NULL)
        pixcmapHasColor(cmap, &colorfound);
    if (cmap && !colorfound)
        pixg = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    else
        pixg = pixClone(pixs);

    pixGetDimensions(pixg, &w, &h, &d);
    size = 1 << d;
    if ((na = numaCreate(size)) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    numaSetCount(na, size);  /* all initialized to 0.0 */
    array = numaGetFArray(na, L_NOCOPY);

    if (d == 1) {  /* special case */
        pixCountPixels(pixg, &count, NULL);
        array[0] = w * h - count;
        array[1] = count;
        pixDestroy(&pixg);
        return na;
    }

    wpl = pixGetWpl(pixg);
    data = pixGetData(pixg);
    for (i = 0; i < h; i += factor) {
        line = data + i * wpl;
        switch (d) 
        {
        case 2:
            for (j = 0; j < w; j += factor) {
                val = GET_DATA_DIBIT(line, j);
                array[val] += 1.0;
            }
            break;
        case 4:
            for (j = 0; j < w; j += factor) {
                val = GET_DATA_QBIT(line, j);
                array[val] += 1.0;
            }
            break;
        case 8:
            for (j = 0; j < w; j += factor) {
                val = GET_DATA_BYTE(line, j);
                array[val] += 1.0;
            }
            break;
        case 16:
            for (j = 0; j < w; j += factor) {
                val = GET_DATA_TWO_BYTES(line, j);
                array[val] += 1.0;
            }
            break;
        default:
            numaDestroy(&na);
            return (NUMA *)ERROR_PTR("illegal depth", procName, NULL);
        }
    }

    pixDestroy(&pixg);
    return na;
}


/*!
 *  pixGetGrayHistogramMasked()
 *
 *      Input:  pixs (8 bpp, or colormapped)
 *              pixm (<optional> 1 bpp mask over which histogram is
 *                    to be computed; use use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0; these values are ignored if pixm is null)
 *              factor (subsampling factor; integer >= 1)
 *      Return: na (histogram), or null on error
 *
 *  Notes:
 *      (1) If pixs is cmapped, it is converted to 8 bpp gray.
 *      (2) This always returns a 256-value histogram of pixel values.
 *      (3) Set the subsampling factor > 1 to reduce the amount of computation.
 *      (4) Clipping of pixm (if it exists) to pixs is done in the inner loop.
 *      (5) Input x,y are ignored unless pixm exists.
 */
NUMA *
pixGetGrayHistogramMasked(PIX        *pixs,
                          PIX        *pixm,
                          l_int32     x,
                          l_int32     y,
                          l_int32     factor)
{
l_int32     i, j, w, h, wm, hm, dm, wplg, wplm, val;
l_uint32   *datag, *datam, *lineg, *linem;
l_float32  *array;
NUMA       *na;
PIX        *pixg;

    PROCNAME("pixGetGrayHistogramMasked");

    if (!pixm)
        return pixGetGrayHistogram(pixs, factor);

    if (!pixs)
        return (NUMA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8 && !pixGetColormap(pixs))
        return (NUMA *)ERROR_PTR("pixs neither 8 bpp nor colormapped",
                                 procName, NULL);
    pixGetDimensions(pixm, &wm, &hm, &dm);
    if (dm != 1)
        return (NUMA *)ERROR_PTR("pixm not 1 bpp", procName, NULL);
    if (factor < 1)
        return (NUMA *)ERROR_PTR("sampling factor < 1", procName, NULL);

    if ((na = numaCreate(256)) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    numaSetCount(na, 256);  /* all initialized to 0.0 */
    array = numaGetFArray(na, L_NOCOPY);

    if (pixGetColormap(pixs))
        pixg = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    else
        pixg = pixClone(pixs);
    pixGetDimensions(pixg, &w, &h, NULL);
    datag = pixGetData(pixg);
    wplg = pixGetWpl(pixg);
    datam = pixGetData(pixm);
    wplm = pixGetWpl(pixm);

        /* Generate the histogram */
    for (i = 0; i < hm; i += factor) {
        if (y + i < 0 || y + i >= h) continue;
        lineg = datag + (y + i) * wplg;
        linem = datam + i * wplm;
        for (j = 0; j < wm; j += factor) {
            if (x + j < 0 || x + j >= w) continue;
            if (GET_DATA_BIT(linem, j)) {
                val = GET_DATA_BYTE(lineg, x + j);
                array[val] += 1.0;
            }
        }
    }

    pixDestroy(&pixg);
    return na;
}


/*!
 *  pixGetColorHistogram()
 *
 *      Input:  pixs (rgb or colormapped)
 *              factor (subsampling factor; integer >= 1)
 *              &nar (<return> red histogram)
 *              &nag (<return> green histogram)
 *              &nab (<return> blue histogram)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This generates a set of three 256 entry histograms,
 *          one for each color component (r,g,b).
 *      (2) Set the subsampling factor > 1 to reduce the amount of computation.
 */
l_int32
pixGetColorHistogram(PIX     *pixs,
                     l_int32  factor,
                     NUMA   **pnar,
                     NUMA   **pnag,
                     NUMA   **pnab)
{
l_int32     i, j, w, h, d, wpl, index, rval, gval, bval;
l_uint32   *data, *line;
l_float32  *rarray, *garray, *barray;
NUMA       *nar, *nag, *nab;
PIXCMAP    *cmap;

    PROCNAME("pixGetColorHistogram");

    if (!pnar || !pnag || !pnab)
        return ERROR_INT("&nar, &nag, &nab not all defined", procName, 1);
    *pnar = *pnag = *pnab = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    pixGetDimensions(pixs, &w, &h, &d);
    cmap = pixGetColormap(pixs);
    if (cmap && (d != 2 && d != 4 && d != 8))
        return ERROR_INT("colormap and not 2, 4, or 8 bpp", procName, 1);
    if (!cmap && d != 32)
        return ERROR_INT("no colormap and not rgb", procName, 1);
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);

        /* Set up the histogram arrays */
    nar = numaCreate(256);
    nag = numaCreate(256);
    nab = numaCreate(256);
    numaSetCount(nar, 256);
    numaSetCount(nag, 256);
    numaSetCount(nab, 256);
    rarray = numaGetFArray(nar, L_NOCOPY);
    garray = numaGetFArray(nag, L_NOCOPY);
    barray = numaGetFArray(nab, L_NOCOPY);
    *pnar = nar;
    *pnag = nag;
    *pnab = nab;

        /* Generate the color histograms */
    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    if (cmap) {
        for (i = 0; i < h; i += factor) {
            line = data + i * wpl;
            for (j = 0; j < w; j += factor) {
                if (d == 8)
                    index = GET_DATA_BYTE(line, j);
                else if (d == 4)
                    index = GET_DATA_QBIT(line, j);
                else   /* 2 bpp */
                    index = GET_DATA_DIBIT(line, j);
                pixcmapGetColor(cmap, index, &rval, &gval, &bval);
                rarray[rval] += 1.0;
                garray[gval] += 1.0;
                barray[bval] += 1.0;
            }
        }
    }
    else {  /* 32 bpp rgb */
        for (i = 0; i < h; i += factor) {
            line = data + i * wpl;
            for (j = 0; j < w; j += factor) {
                extractRGBValues(line[j], &rval, &gval, &bval);
                rarray[rval] += 1.0;
                garray[gval] += 1.0;
                barray[bval] += 1.0;
            }
        }
    }

    return 0;
}
            

/*!
 *  pixGetColorHistogramMasked()
 *
 *      Input:  pixs (32 bpp rgb, or colormapped)
 *              pixm (<optional> 1 bpp mask over which histogram is
 *                    to be computed; use use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0; these values are ignored if pixm is null)
 *              factor (subsampling factor; integer >= 1)
 *              &nar (<return> red histogram)
 *              &nag (<return> green histogram)
 *              &nab (<return> blue histogram)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This generates a set of three 256 entry histograms,
 *      (2) Set the subsampling factor > 1 to reduce the amount of computation.
 *      (3) Clipping of pixm (if it exists) to pixs is done in the inner loop.
 *      (4) Input x,y are ignored unless pixm exists.
 */
l_int32
pixGetColorHistogramMasked(PIX        *pixs,
                           PIX        *pixm,
                           l_int32     x,
                           l_int32     y,
                           l_int32     factor,
                           NUMA      **pnar,
                           NUMA      **pnag,
                           NUMA      **pnab)
{
l_int32     i, j, w, h, d, wm, hm, dm, wpls, wplm, index, rval, gval, bval;
l_uint32   *datas, *datam, *lines, *linem;
l_float32  *rarray, *garray, *barray;
NUMA       *nar, *nag, *nab;
PIXCMAP    *cmap;

    PROCNAME("pixGetColorHistogramMasked");

    if (!pixm)
        return pixGetColorHistogram(pixs, factor, pnar, pnag, pnab);

    if (!pnar || !pnag || !pnab)
        return ERROR_INT("&nar, &nag, &nab not all defined", procName, 1);
    *pnar = *pnag = *pnab = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    pixGetDimensions(pixs, &w, &h, &d);
    cmap = pixGetColormap(pixs);
    if (cmap && (d != 2 && d != 4 && d != 8))
        return ERROR_INT("colormap and not 2, 4, or 8 bpp", procName, 1);
    if (!cmap && d != 32)
        return ERROR_INT("no colormap and not rgb", procName, 1);
    pixGetDimensions(pixm, &wm, &hm, &dm);
    if (dm != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);

        /* Set up the histogram arrays */
    nar = numaCreate(256);
    nag = numaCreate(256);
    nab = numaCreate(256);
    numaSetCount(nar, 256);
    numaSetCount(nag, 256);
    numaSetCount(nab, 256);
    rarray = numaGetFArray(nar, L_NOCOPY);
    garray = numaGetFArray(nag, L_NOCOPY);
    barray = numaGetFArray(nab, L_NOCOPY);
    *pnar = nar;
    *pnag = nag;
    *pnab = nab;

        /* Generate the color histograms */
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    datam = pixGetData(pixm);
    wplm = pixGetWpl(pixm);
    if (cmap) {
        for (i = 0; i < hm; i += factor) {
            if (y + i < 0 || y + i >= h) continue;
            lines = datas + (y + i) * wpls;
            linem = datam + i * wplm;
            for (j = 0; j < wm; j += factor) {
                if (x + j < 0 || x + j >= w) continue;
                if (GET_DATA_BIT(linem, j)) {
                    if (d == 8)
                        index = GET_DATA_BYTE(lines, x + j);
                    else if (d == 4)
                        index = GET_DATA_QBIT(lines, x + j);
                    else   /* 2 bpp */
                        index = GET_DATA_DIBIT(lines, x + j);
                    pixcmapGetColor(cmap, index, &rval, &gval, &bval);
                    rarray[rval] += 1.0;
                    garray[gval] += 1.0;
                    barray[bval] += 1.0;
                }
            }
        }
    }
    else {  /* 32 bpp rgb */
        for (i = 0; i < hm; i += factor) {
            if (y + i < 0 || y + i >= h) continue;
            lines = datas + (y + i) * wpls;
            linem = datam + i * wplm;
            for (j = 0; j < wm; j += factor) {
                if (x + j < 0 || x + j >= w) continue;
                if (GET_DATA_BIT(linem, j)) {
                    extractRGBValues(lines[x + j], &rval, &gval, &bval);
                    rarray[rval] += 1.0;
                    garray[gval] += 1.0;
                    barray[bval] += 1.0;
                }
            }
        }
    }

    return 0;
}


/*!
 *  pixGetRankValueMaskedRGB()
 *
 *      Input:  pixs (32 bpp)
 *              pixm (<optional> 1 bpp mask over which rank val is to be taken;
 *                    use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0; these values are ignored if pixm is null)
 *              factor (subsampling factor; integer >= 1)
 *              rank (between 0.0 and 1.0; 1.0 is brightest, 0.0 is darkest)
 *              &rval (<optional return> red component val for to input rank)
 *              &gval (<optional return> green component val for to input rank)
 *              &bval (<optional return> blue component val for to input rank)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Computes the rank component values of pixels in pixs that
 *          are under the fg of the optional mask.  If the mask is null, it
 *          computes the average of the pixels in pixs.
 *      (2) Set the subsampling factor > 1 to reduce the amount of
 *          computation.
 *      (4) Input x,y are ignored unless pixm exists.
 *      (5) The rank must be in [0.0 ... 1.0], where the brightest pixel
 *          has rank 1.0.  For the median pixel value, use 0.5.
 */
l_int32
pixGetRankValueMaskedRGB(PIX        *pixs,
                         PIX        *pixm,
                         l_int32     x,
                         l_int32     y,
                         l_int32     factor,
                         l_float32   rank,
                         l_float32  *prval,
                         l_float32  *pgval,
                         l_float32  *pbval)
{
l_float32  scale;
PIX       *pixmt, *pixt;

    PROCNAME("pixGetRankValueMaskedRGB");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 32)
        return ERROR_INT("pixs not 32 bpp", procName, 1);
    if (pixm && pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);
    if (rank < 0.0 || rank > 1.0)
        return ERROR_INT("rank not in [0.0 ... 1.0]", procName, 1);
    if (!prval && !pgval && !pbval)
        return ERROR_INT("no results requested", procName, 1);

    pixmt = NULL;
    if (pixm) {
        scale = 1.0 / (l_float32)factor;
        pixmt = pixScale(pixm, scale, scale);
    }
    if (prval) {
        pixt = pixScaleRGBToGrayFast(pixs, factor, COLOR_RED);
        pixGetRankValueMasked(pixt, pixmt, x / factor, y / factor,
                              factor, rank, prval, NULL);
        pixDestroy(&pixt);
    }
    if (pgval) {
        pixt = pixScaleRGBToGrayFast(pixs, factor, COLOR_GREEN);
        pixGetRankValueMasked(pixt, pixmt, x / factor, y / factor,
                              factor, rank, pgval, NULL);
        pixDestroy(&pixt);
    }
    if (pbval) {
        pixt = pixScaleRGBToGrayFast(pixs, factor, COLOR_BLUE);
        pixGetRankValueMasked(pixt, pixmt, x / factor, y / factor,
                              factor, rank, pbval, NULL);
        pixDestroy(&pixt);
    }
    pixDestroy(&pixmt);
    return 0;
}


/*!
 *  pixGetRankValueMasked()
 *
 *      Input:  pixs (8 bpp, or colormapped)
 *              pixm (<optional> 1 bpp mask over which rank val is to be taken;
 *                    use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0; these values are ignored if pixm is null)
 *              factor (subsampling factor; integer >= 1)
 *              rank (between 0.0 and 1.0; 1.0 is brightest, 0.0 is darkest)
 *              &val (<return> pixel value corresponding to input rank)
 *              &na (<optional return> of histogram)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Computes the rank value of pixels in pixs that are under
 *          the fg of the optional mask.  If the mask is null, it
 *          computes the average of the pixels in pixs.
 *      (2) Set the subsampling factor > 1 to reduce the amount of
 *          computation.
 *      (3) Clipping of pixm (if it exists) to pixs is done in the inner loop.
 *      (4) Input x,y are ignored unless pixm exists.
 *      (5) The rank must be in [0.0 ... 1.0], where the brightest pixel
 *          has rank 1.0.  For the median pixel value, use 0.5.
 *      (6) The histogram can optionally be returned, so that other rank
 *          values can be extracted without recomputing the histogram.
 *          In that case, just use
 *              numaHistogramGetValFromRank(na, rank, &val);
 *          on the returned Numa for additional rank values.
 */
l_int32
pixGetRankValueMasked(PIX        *pixs,
                      PIX        *pixm,
                      l_int32     x,
                      l_int32     y,
                      l_int32     factor,
                      l_float32   rank,
                      l_float32  *pval,
                      NUMA      **pna)
{
NUMA  *na;

    PROCNAME("pixGetRankValueMasked");

    if (pna)
        *pna = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 8 && !pixGetColormap(pixs))
        return ERROR_INT("pixs neither 8 bpp nor colormapped", procName, 1);
    if (pixm && pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);
    if (rank < 0.0 || rank > 1.0)
        return ERROR_INT("rank not in [0.0 ... 1.0]", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0.0;  /* init */

    if ((na = pixGetGrayHistogramMasked(pixs, pixm, x, y, factor)) == NULL)
        return ERROR_INT("na not made", procName, 1);
    numaHistogramGetValFromRank(na, rank, pval);
    if (pna)
        *pna = na;
    else
        numaDestroy(&na);

    return 0;
}


/*!
 *  pixGetAverageMaskedRGB()
 *
 *      Input:  pixs (32 bpp, or colormapped)
 *              pixm (<optional> 1 bpp mask over which average is to be taken;
 *                    use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0)
 *              factor (subsampling factor; >= 1)
 *              type (L_MEAN_ABSVAL, L_ROOT_MEAN_SQUARE,
 *                    L_STANDARD_DEVIATION, L_VARIANCE)
 *              &rval (<return optional> measured red value of given 'type')
 *              &gval (<return optional> measured green value of given 'type')
 *              &bval (<return optional> measured blue value of given 'type')
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) For usage, see pixGetAverageMasked().
 *      (2) If there is a colormap, it is removed before the 8 bpp
 *          component images are extracted.
 */
l_int32
pixGetAverageMaskedRGB(PIX        *pixs,
                       PIX        *pixm,
                       l_int32     x,
                       l_int32     y,
                       l_int32     factor,
                       l_int32     type,
                       l_float32  *prval,
                       l_float32  *pgval,
                       l_float32  *pbval)
{
l_int32   color;
PIX      *pixt;
PIXCMAP  *cmap;

    PROCNAME("pixGetAverageMaskedRGB");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    color = 0;
    cmap = pixGetColormap(pixs);
    if (pixGetDepth(pixs) != 32 && !cmap)
        return ERROR_INT("pixs neither 32 bpp nor colormapped", procName, 1);
    if (pixm && pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("subsampling factor < 1", procName, 1);
    if (type != L_MEAN_ABSVAL && type != L_ROOT_MEAN_SQUARE &&
        type != L_STANDARD_DEVIATION && type != L_VARIANCE)
        return ERROR_INT("invalid measure type", procName, 1);
    if (!prval && !pgval && !pbval)
        return ERROR_INT("no values requested", procName, 1);

    if (prval) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_RED);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_RED);
        pixGetAverageMasked(pixt, pixm, x, y, factor, type, prval);
        pixDestroy(&pixt);
    }
    if (pgval) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_GREEN);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_GREEN);
        pixGetAverageMasked(pixt, pixm, x, y, factor, type, pgval);
        pixDestroy(&pixt);
    }
    if (pbval) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_BLUE);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_BLUE);
        pixGetAverageMasked(pixt, pixm, x, y, factor, type, pbval);
        pixDestroy(&pixt);
    }

    return 0;
}


/*!
 *  pixGetAverageMasked()
 *
 *      Input:  pixs (8 or 16 bpp, or colormapped)
 *              pixm (<optional> 1 bpp mask over which average is to be taken;
 *                    use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0)
 *              factor (subsampling factor; >= 1)
 *              type (L_MEAN_ABSVAL, L_ROOT_MEAN_SQUARE,
 *                    L_STANDARD_DEVIATION, L_VARIANCE)
 *              &val (<return> measured value of given 'type')
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Use L_MEAN_ABSVAL to get the average value of pixels in pixs
 *          that are under the fg of the optional mask.  If the mask
 *          is null, it finds the average of the pixels in pixs.
 *      (2) Likewise, use L_ROOT_MEAN_SQUARE to get the rms value of
 *          pixels in pixs, either masked or not; L_STANDARD_DEVIATION
 *          to get the standard deviation from the mean of the pixels;
 *          L_VARIANCE to get the average squared difference from the
 *          expected value.  The variance is the square of the stdev.
 *          For the standard deviation, we use
 *              sqrt(<(<x> - x)>^2) = sqrt(<x^2> - <x>^2)
 *      (3) Set the subsampling factor > 1 to reduce the amount of
 *          computation.
 *      (4) Clipping of pixm (if it exists) to pixs is done in the inner loop.
 *      (5) Input x,y are ignored unless pixm exists.
 */
l_int32
pixGetAverageMasked(PIX        *pixs,
                    PIX        *pixm,
                    l_int32     x,
                    l_int32     y,
                    l_int32     factor,
                    l_int32     type,
                    l_float32  *pval)
{
l_int32    i, j, w, h, d, wm, hm, wplg, wplm, val, count;
l_uint32  *datag, *datam, *lineg, *linem;
l_float64  sumave, summs, ave, meansq, var;
PIX       *pixg;

    PROCNAME("pixGetAverageMasked");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    d = pixGetDepth(pixs);
    if (d != 8 && d != 16 && !pixGetColormap(pixs))
        return ERROR_INT("pixs not 8 or 16 bpp or colormapped", procName, 1);
    if (pixm && pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("subsampling factor < 1", procName, 1);
    if (type != L_MEAN_ABSVAL && type != L_ROOT_MEAN_SQUARE &&
        type != L_STANDARD_DEVIATION && type != L_VARIANCE)
        return ERROR_INT("invalid measure type", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0.0;  /* init */

    if (pixGetColormap(pixs))
        pixg = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    else
        pixg = pixClone(pixs);
    pixGetDimensions(pixg, &w, &h, &d);
    datag = pixGetData(pixg);
    wplg = pixGetWpl(pixg);

    sumave = summs = 0.0;
    count = 0;
    if (!pixm) {
        for (i = 0; i < h; i += factor) {
            lineg = datag + i * wplg;
            for (j = 0; j < w; j += factor) {
                if (d == 8)
                    val = GET_DATA_BYTE(lineg, j);
                else  /* d == 16 */
                    val = GET_DATA_TWO_BYTES(lineg, j);
                if (type != L_ROOT_MEAN_SQUARE)
                    sumave += val;
                if (type != L_MEAN_ABSVAL)
                    summs += val * val;
                count++;
            }
        }
    }
    else {
        pixGetDimensions(pixm, &wm, &hm, NULL);
        datam = pixGetData(pixm);
        wplm = pixGetWpl(pixm);
        for (i = 0; i < hm; i += factor) {
            if (y + i < 0 || y + i >= h) continue;
            lineg = datag + (y + i) * wplg;
            linem = datam + i * wplm;
            for (j = 0; j < wm; j += factor) {
                if (x + j < 0 || x + j >= w) continue;
                if (GET_DATA_BIT(linem, j)) {
                    if (d == 8)
                        val = GET_DATA_BYTE(lineg, x + j);
                    else  /* d == 16 */
                        val = GET_DATA_TWO_BYTES(lineg, x + j);
                    if (type != L_ROOT_MEAN_SQUARE)
                        sumave += val;
                    if (type != L_MEAN_ABSVAL)
                        summs += val * val;
                    count++;
                }
            }
        }
    }

    pixDestroy(&pixg);
    if (count == 0)
        return ERROR_INT("no pixels sampled", procName, 1);
    ave = sumave / (l_float64)count;
    meansq = summs / (l_float64)count;
    var = meansq - ave * ave;
    if (type == L_MEAN_ABSVAL)
        *pval = (l_float32)ave;
    else if (type == L_ROOT_MEAN_SQUARE)
        *pval = (l_float32)sqrt(meansq);
    else if (type == L_STANDARD_DEVIATION)
        *pval = (l_float32)sqrt(var);
    else  /* type == L_VARIANCE */
        *pval = (l_float32)var;

    return 0;
}


/*!
 *  pixGetAverageTiledRGB()
 *
 *      Input:  pixs (32 bpp, or colormapped)
 *              sx, sy (tile size; must be at least 2 x 2)
 *              type (L_MEAN_ABSVAL, L_ROOT_MEAN_SQUARE, L_STANDARD_DEVIATION)
 *              &pixr (<optional return> tiled 'average' of red component)
 *              &pixg (<optional return> tiled 'average' of green component)
 *              &pixb (<optional return> tiled 'average' of blue component)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) For usage, see pixGetAverageTiled().
 *      (2) If there is a colormap, it is removed before the 8 bpp
 *          component images are extracted.
 */
l_int32
pixGetAverageTiledRGB(PIX     *pixs,
                      l_int32  sx,
                      l_int32  sy,
                      l_int32  type,
                      PIX    **ppixr,
                      PIX    **ppixg,
                      PIX    **ppixb)
{
PIX      *pixt;
PIXCMAP  *cmap;

    PROCNAME("pixGetAverageTiledRGB");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    cmap = pixGetColormap(pixs);
    if (pixGetDepth(pixs) != 32 && !cmap)
        return ERROR_INT("pixs neither 32 bpp nor colormapped", procName, 1);
    if (sx < 2 || sy < 2)
        return ERROR_INT("sx and sy not both > 1", procName, 1);
    if (type != L_MEAN_ABSVAL && type != L_ROOT_MEAN_SQUARE &&
        type != L_STANDARD_DEVIATION)
        return ERROR_INT("invalid measure type", procName, 1);
    if (!ppixr && !ppixg && !ppixb)
        return ERROR_INT("no returned data requested", procName, 1);

    if (ppixr) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_RED);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_RED);
        *ppixr = pixGetAverageTiled(pixt, sx, sy, type);
        pixDestroy(&pixt);
    }
    if (ppixg) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_GREEN);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_GREEN);
        *ppixg = pixGetAverageTiled(pixt, sx, sy, type);
        pixDestroy(&pixt);
    }
    if (ppixb) {
        if (cmap)
            pixt = pixGetRGBComponentCmap(pixs, COLOR_BLUE);
        else
            pixt = pixGetRGBComponent(pixs, COLOR_BLUE);
        *ppixb = pixGetAverageTiled(pixt, sx, sy, type);
        pixDestroy(&pixt);
    }

    return 0;
}


/*!
 *  pixGetAverageTiled()
 *
 *      Input:  pixs (8 bpp, or colormapped)
 *              sx, sy (tile size; must be at least 2 x 2)
 *              type (L_MEAN_ABSVAL, L_ROOT_MEAN_SQUARE, L_STANDARD_DEVIATION)
 *      Return: pixd (average values in each tile), or null on error
 *
 *  Notes:
 *      (1) Only computes for tiles that are entirely contained in pixs.
 *      (2) Use L_MEAN_ABSVAL to get the average abs value within the tile;
 *          L_ROOT_MEAN_SQUARE to get the rms value within each tile;
 *          L_STANDARD_DEVIATION to get the standard dev. from the average
 *          within each tile.
 *      (3) If colormapped, converts to 8 bpp gray.
 */
PIX *
pixGetAverageTiled(PIX     *pixs,
                   l_int32  sx,
                   l_int32  sy,
                   l_int32  type)
{
l_int32    i, j, k, m, w, h, wd, hd, d, pos, wplt, wpld, valt;
l_uint32  *datat, *datad, *linet, *lined, *startt;
l_float64  sumave, summs, ave, meansq, normfact;
PIX       *pixt, *pixd;

    PROCNAME("pixGetAverageTiled");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8 && !pixGetColormap(pixs))
        return (PIX *)ERROR_PTR("pixs not 8 bpp or cmapped", procName, NULL);
    if (sx < 2 || sy < 2)
        return (PIX *)ERROR_PTR("sx and sy not both > 1", procName, NULL);
    wd = w / sx;
    hd = h / sy;
    if (wd < 1 || hd < 1)
        return (PIX *)ERROR_PTR("wd or hd == 0", procName, NULL);
    if (type != L_MEAN_ABSVAL && type != L_ROOT_MEAN_SQUARE &&
        type != L_STANDARD_DEVIATION)
        return (PIX *)ERROR_PTR("invalid measure type", procName, NULL);

    pixt = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    pixd = pixCreate(wd, hd, 8);
    datat = pixGetData(pixt);
    wplt = pixGetWpl(pixt);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    normfact = 1. / (l_float64)(sx * sy);
    for (i = 0; i < hd; i++) {
        lined = datad + i * wpld;
        linet = datat + i * sy * wplt;
        for (j = 0; j < wd; j++) {
            if (type == L_MEAN_ABSVAL || type == L_STANDARD_DEVIATION) {
                sumave = 0.0;
                for (k = 0; k < sy; k++) {
                    startt = linet + k * wplt;
                    for (m = 0; m < sx; m++) {
                        pos = j * sx + m;
                        valt = GET_DATA_BYTE(startt, pos);
                        sumave += valt;
                    }
                }
                ave = normfact * sumave;
            }
            if (type == L_ROOT_MEAN_SQUARE || type == L_STANDARD_DEVIATION) {
                summs = 0.0;
                for (k = 0; k < sy; k++) {
                    startt = linet + k * wplt;
                    for (m = 0; m < sx; m++) {
                        pos = j * sx + m;
                        valt = GET_DATA_BYTE(startt, pos);
                        summs += valt * valt;
                    }
                }
                meansq = normfact * summs;
            }
            if (type == L_MEAN_ABSVAL)
                valt = (l_int32)(ave + 0.5);
            else if (type == L_ROOT_MEAN_SQUARE)
                valt = (l_int32)(sqrt(meansq) + 0.5);
            else  /* type == L_STANDARD_DEVIATION */
                valt = (l_int32)(sqrt(meansq - ave * ave) + 0.5);
            SET_DATA_BYTE(lined, j, valt);
        }
    }

    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixGetExtremeValue()
 *
 *      Input:  pixs (8 bpp grayscale, 32 bpp rgb, or colormapped)
 *              factor (subsampling factor; >= 1; ignored if colormapped)
 *              type (L_CHOOSE_MIN or L_CHOOSE_MAX)
 *              &rval (<optional return> red component)
 *              &gval (<optional return> green component)
 *              &bval (<optional return> blue component)
 *              &grayval (<optional return> min gray value)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) If pixs is grayscale, the result is returned in &grayval.
 *          Otherwise, if there is a colormap or d == 32,
 *          each requested color component is returned.  At least
 *          one color component (address) must be input.
 */
l_int32
pixGetExtremeValue(PIX      *pixs,
                   l_int32   factor,
                   l_int32   type,
                   l_int32  *prval,
                   l_int32  *pgval,
                   l_int32  *pbval,
                   l_int32  *pgrayval)
{
l_int32    i, j, w, h, d, wpl;
l_int32    val, extval, rval, gval, bval, extrval, extgval, extbval;
l_uint32   pixel;
l_uint32  *data, *line;
PIXCMAP   *cmap;

    PROCNAME("pixGetExtremeValue");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    cmap = pixGetColormap(pixs);
    if (cmap)
        return pixcmapGetExtremeValue(cmap, type, prval, pgval, pbval);

    pixGetDimensions(pixs, &w, &h, &d);
    if (type != L_CHOOSE_MIN && type != L_CHOOSE_MAX)
        return ERROR_INT("invalid type", procName, 1);
    if (factor < 1)
        return ERROR_INT("subsampling factor < 1", procName, 1);
    if (d != 8 && d != 32)
        return ERROR_INT("pixs not 8 or 32 bpp", procName, 1);
    if (d == 8 && !pgrayval)
        return ERROR_INT("can't return result in grayval", procName, 1);
    if (d == 32 && !prval && !pgval && !pbval)
        return ERROR_INT("can't return result in r/g/b-val", procName, 1);

    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    if (d == 8) {
        if (type == L_CHOOSE_MIN)
            extval = 100000;
        else  /* get max */
            extval = 0;

        for (i = 0; i < h; i += factor) {
            line = data + i * wpl;
            for (j = 0; j < w; j += factor) {
                val = GET_DATA_BYTE(line, j);
                if ((type == L_CHOOSE_MIN && val < extval) ||
                    (type == L_CHOOSE_MAX && val > extval))
                    extval = val;
            }
        }
        *pgrayval = extval;
        return 0;
    }

        /* 32 bpp rgb */
    if (type == L_CHOOSE_MIN) {
        extrval = 100000;
        extgval = 100000;
        extbval = 100000;
    }
    else {
        extrval = 0;
        extgval = 0;
        extbval = 0;
    }
    for (i = 0; i < h; i += factor) {
        line = data + i * wpl;
        for (j = 0; j < w; j += factor) {
            pixel = line[j];
            if (prval) {
                rval = (pixel >> L_RED_SHIFT) & 0xff;
                if ((type == L_CHOOSE_MIN && rval < extrval) ||
                    (type == L_CHOOSE_MAX && rval > extrval))
                    extrval = rval;
            }
            if (pgval) {
                gval = (pixel >> L_GREEN_SHIFT) & 0xff;
                if ((type == L_CHOOSE_MIN && gval < extgval) ||
                    (type == L_CHOOSE_MAX && gval > extgval))
                    extgval = gval;
            }
            if (pbval) {
                bval = (pixel >> L_BLUE_SHIFT) & 0xff;
                if ((type == L_CHOOSE_MIN && bval < extbval) ||
                    (type == L_CHOOSE_MAX && bval > extbval))
                    extbval = bval;
            }
        }
    }
    if (prval) *prval = extrval;
    if (pgval) *pgval = extgval;
    if (pbval) *pbval = extbval;
    return 0;
}


/*!
 *  pixGetMaxValueInRect()
 *
 *      Input:  pixs (8 bpp or 32 bpp grayscale; no color space components)
 *              box (<optional> region; set box = NULL to use entire pixs)
 *              &maxval (<optional return> max value in region)
 *              &xmax (<optional return> x location of max value)
 *              &ymax (<optional return> y location of max value)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This can be used to find the maximum and its location
 *          in a 2-dimensional histogram, where the x and y directions
 *          represent two color components (e.g., saturation and hue).
 *      (2) Note that here a 32 bpp pixs has pixel values that are simply
 *          numbers.  They are not 8 bpp components in a colorspace.
 */
l_int32
pixGetMaxValueInRect(PIX       *pixs,
                     BOX       *box,
                     l_uint32  *pmaxval,
                     l_int32   *pxmax,
                     l_int32   *pymax)
{
l_int32    i, j, w, h, d, wpl, bw, bh;
l_int32    xstart, ystart, xend, yend, xmax, ymax;
l_uint32   val, maxval;
l_uint32  *data, *line;

    PROCNAME("pixGetMaxValueInRect");

    if (!pmaxval && !pxmax && !pymax)
        return ERROR_INT("nothing to do", procName, 1);
    if (pmaxval) *pmaxval = 0;
    if (pxmax) *pxmax = 0;
    if (pymax) *pymax = 0;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetColormap(pixs) != NULL)
        return ERROR_INT("pixs has colormap", procName, 1);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8 && d != 32)
        return ERROR_INT("pixs not 8 or 32 bpp", procName, 1);

    xstart = ystart = 0;
    xend = w - 1;
    yend = h - 1;
    if (box) {
        boxGetGeometry(box, &xstart, &ystart, &bw, &bh);
        xend = xstart + bw - 1;
        yend = ystart + bh - 1;
    }

    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    maxval = 0;
    xmax = ymax = 0;
    for (i = ystart; i <= yend; i++) {
        line = data + i * wpl;
        for (j = xstart; j <= xend; j++) {
            if (d == 8)
                val = GET_DATA_BYTE(line, j);
            else  /* d == 32 */
                val = line[j];
            if (val > maxval) {
                maxval = val;
                xmax = j;
                ymax = i;
            }
        }
    }
    if (maxval == 0) {  /* no counts; pick the center of the rectangle */
        xmax = (xstart + xend) / 2;
        ymax = (ystart + yend) / 2;
    }

    if (pmaxval) *pmaxval = maxval;
    if (pxmax) *pxmax = xmax;
    if (pymax) *pymax = ymax;
    return 0;
}


/*!
 *  pixGetRankColorArray()
 *
 *      Input:  pixs (32 bpp or cmapped)
 *              nbins (number of equal population bins; must be > 1)
 *              factor (subsampling factor; integer >= 1)
 *              &carray (<return> array of colors, ranked by intensity)
 *              debugflag (1 to display output debug plots of color
 *                         components; 2 to write them as png to file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Computes the histogram of the minimum component in each RGB
 *          pixel.  Then for each of the @nbins sets of pixels,
 *          ordered by this min component value, find the average color,
 *          and return this as a "rank color" array.  The output array
 *          has @nbins colors.
 *      (2) Set the subsampling factor > 1 to reduce the amount of
 *          computation.  Typically you want at least 10,000 pixels
 *          for reasonable statistics.
 *      (3) The rank color as a function of rank can then be found from
 *             rankint = (l_int32)(rank * (nbins - 1) + 0.5);
 *             extractRGBValues(array[rankint], &rval, &gval, &bval);
 *          where the rank is in [0.0 ... 1.0].
 *          This function is meant to be simple and approximate.
 */
l_int32
pixGetRankColorArray(PIX        *pixs,
                     l_int32     nbins,
                     l_int32     factor,
                     l_uint32  **pcarray,
                     l_int32     debugflag)
{
NUMA      *na, *nan, *narbin;
PIX       *pixt, *pixc, *pixg;
PIXCMAP   *cmap;

    PROCNAME("pixGetRankColorArray");

    if (!pcarray)
        return ERROR_INT("&carray not defined", procName, 1);
    *pcarray = NULL;
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);
    if (nbins < 2)
        return ERROR_INT("nbins must be at least 2", procName, 1);
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    cmap = pixGetColormap(pixs);
    if (pixGetDepth(pixs) != 32 && !cmap)
        return ERROR_INT("pixs neither 32 bpp nor cmapped", procName, 1);

        /* Downscale by factor and remove colormap if it exists */
    pixt = pixScaleByIntSubsampling(pixs, factor);
    if (cmap)
        pixc = pixRemoveColormap(pixt, REMOVE_CMAP_TO_FULL_COLOR);
    else
        pixc = pixClone(pixt);
    pixDestroy(&pixt);

        /* Get normalized histogram of the minimum component */
    pixg = pixConvertRGBToGrayMinMax(pixc, L_CHOOSE_MIN);
    if ((na = pixGetGrayHistogram(pixg, 1)) == NULL)
        return ERROR_INT("na not made", procName, 1);
    nan = numaNormalizeHistogram(na, 1.0);

        /* Get the following arrays:
         * (1) nar: cumulative normalized histogram (rank vs intensity value).
         *     With 256 intensity values, we have 257 rank values.
         * (2) nai: "average" intensity as function of rank, within
         *     @nbins equally spaced in rank between 0.0 and 1.0.
         * (3) narbin: bin number of discretized rank as a function of
         *     intensity.  This is the 'inverse' of nai.
         * (4) nabb: intensity value of the right bin boundary, for each
         *     of the @nbins descretized rank bins. */
    if (!debugflag)
        numaDiscretizeRankAndIntensity(nan, nbins, &narbin, NULL, NULL, NULL);
    else {
        l_int32  type;
        NUMA    *nai, *nar, *nabb;
        numaDiscretizeRankAndIntensity(nan, nbins, &narbin, &nai, &nar, &nabb);
        type = (debugflag == 1) ? GPLOT_X11 : GPLOT_PNG;
        gplotSimple1(nan, type, "/tmp/junkrtnan", "Normalized Histogram");
        gplotSimple1(nar, type, "/tmp/junkrtnar", "Cumulative Histogram");
        gplotSimple1(nai, type, "/tmp/junkrtnai", "Intensity vs. rank bin");
        gplotSimple1(narbin, type, "/tmp/junkrtnarbin",
                     "LUT: rank bin vs. Intensity");
        gplotSimple1(nabb, type, "/tmp/junkrtnabb",
                     "Intensity of right edge vs. rank bin");
        numaDestroy(&nai);
        numaDestroy(&nar);
        numaDestroy(&nabb);
    }

        /* Get the average color in each bin for pixels whose grayscale
         * values fall in the bin range.  @narbin is the LUT that
         * determines the bin number from the grayscale version of
         * the image. */
    pixGetBinnedColor(pixc, pixg, 1, nbins, narbin, pcarray, debugflag);

    pixDestroy(&pixc);
    pixDestroy(&pixg);
    numaDestroy(&na);
    numaDestroy(&nan);
    numaDestroy(&narbin);
    return 0;
}


/*!
 *  pixGetBinnedColor()
 *
 *      Input:  pixs (32 bpp)
 *              pixg (8 bpp grayscale version of pixs)
 *              factor (sampling factor along pixel counting direction)
 *              nbins (number of intensity bins)
 *              nalut (LUT for mapping from intensity to bin number)
 *              &carray (<return> array of average color values in each bin)
 *              debugflag (1 to display output debug plots of color
 *                         components; 2 to write them as png to file)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This takes a color image, a grayscale (intensity) version,
 *          a LUT from intensity to bin number, and the number of bins.
 *          It computes the average color for pixels whose intensity
 *          is in each bin.  This is returned as an array of l_uint32
 *          colors in our standard RGBA ordering.
 */
l_int32
pixGetBinnedColor(PIX        *pixs,
                  PIX        *pixg,
                  l_int32     factor,
                  l_int32     nbins,
                  NUMA       *nalut,
                  l_uint32  **pcarray,
                  l_int32     debugflag)
{
l_int32     i, j, w, h, wpls, wplg, grayval, bin, rval, gval, bval;
l_uint32   *datas, *datag, *lines, *lineg, *carray;
l_float64   norm;
l_float64  *rarray, *garray, *barray, *narray;

    PROCNAME("pixGetBinnedColor");

    if (!pcarray)
        return ERROR_INT("&carray not defined", procName, 1);
    *pcarray = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pixg)
        return ERROR_INT("pixg not defined", procName, 1);
    if (!nalut)
        return ERROR_INT("nalut not defined", procName, 1);
    if (factor < 1) {
        L_WARNING("sampling factor less than 1; setting to 1", procName);
        factor = 1;
    }

        /* Find the color for each rank bin */
    pixGetDimensions(pixs, &w, &h, NULL);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    datag = pixGetData(pixg);
    wplg = pixGetWpl(pixg);
    rarray = (l_float64 *)CALLOC(nbins, sizeof(l_float64));
    garray = (l_float64 *)CALLOC(nbins, sizeof(l_float64));
    barray = (l_float64 *)CALLOC(nbins, sizeof(l_float64));
    narray = (l_float64 *)CALLOC(nbins, sizeof(l_float64));
    for (i = 0; i < h; i += factor) {
        lines = datas + i * wpls;
        lineg = datag + i * wplg;
        for (j = 0; j < w; j += factor) {
            grayval = GET_DATA_BYTE(lineg, j);
            numaGetIValue(nalut, grayval, &bin);
            extractRGBValues(lines[j], &rval, &gval, &bval);
            rarray[bin] += rval;
            garray[bin] += gval;
            barray[bin] += bval;
            narray[bin] += 1.0;  /* count samples in each bin */
        }
    }

    for (i = 0; i < nbins; i++) {
        norm = 1. / narray[i];
        rarray[i] *= norm;
        garray[i] *= norm;
        barray[i] *= norm;
    }

    if (debugflag) {
        l_int32  type;
        NUMA *nared, *nagreen, *nablue;
        nared = numaCreate(nbins);
        nagreen = numaCreate(nbins);
        nablue = numaCreate(nbins);
        for (i = 0; i < nbins; i++) {
            numaAddNumber(nared, rarray[i]);
            numaAddNumber(nagreen, garray[i]);
            numaAddNumber(nablue, barray[i]);
        }
        type = (debugflag == 1) ? GPLOT_X11 : GPLOT_PNG;
        gplotSimple1(nared, type, "/tmp/junkrtnared",
                     "Average red val vs. rank bin");
        gplotSimple1(nagreen, type, "/tmp/junkrtnagreen",
                     "Average green val vs. rank bin");
        gplotSimple1(nablue, type, "/tmp/junkrtnablue",
                     "Average blue val vs. rank bin");
        numaDestroy(&nared);
        numaDestroy(&nagreen);
        numaDestroy(&nablue);
    }

        /* Save colors for all bins  in a single array */
    if ((carray = (l_uint32 *)CALLOC(nbins, sizeof(l_uint32))) == NULL)
        return ERROR_INT("rankcolor not made", procName, 1);
    *pcarray = carray;
    for (i = 0; i < nbins; i++) {
        rval = (l_int32)(rarray[i] + 0.5);
        gval = (l_int32)(garray[i] + 0.5);
        bval = (l_int32)(barray[i] + 0.5);
        composeRGBPixel(rval, gval, bval, carray + i);
    }

    FREE(rarray);
    FREE(garray);
    FREE(barray);
    FREE(narray);
    return 0;
}


/*-------------------------------------------------------------*
 *                 Pixelwise aligned statistics                *
 *-------------------------------------------------------------*/
/*!
 *  pixaGetAlignedStats()
 *
 *      Input:  pixa (of identically sized, 8 bpp pix; not cmapped)
 *              type (L_MEAN_ABSVAL, L_MEDIAN_VAL, L_MODE_VAL, L_MODE_COUNT)
 *              nbins (of histogram for median and mode; ignored for mean)
 *              thresh (on histogram for mode val; ignored for all other types)
 *      Return: pix (with pixelwise aligned stats), or null on error.
 *
 *  Notes:
 *      (1) Each pixel in the returned pix represents an average
 *          (or median, or mode) over the corresponding pixels in each
 *          pix in the pixa.
 *      (2) The @thresh parameter works with L_MODE_VAL only, and
 *          sets a minimum occupancy of the mode bin.
 *          If the occupancy of the mode bin is less than @thresh, the
 *          mode value is returned as 0.  To always return the actual
 *          mode value, set @thresh = 0.  See pixGetRowStats().
 */
PIX *
pixaGetAlignedStats(PIXA     *pixa,
                    l_int32   type,
                    l_int32   nbins,
                    l_int32   thresh)
{
l_int32     j, n, w, h, d;
l_float32  *colvect;
PIX        *pixt, *pixd;

    PROCNAME("pixaGetAlignedStats");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    if (type != L_MEAN_ABSVAL && type != L_MEDIAN_VAL &&
        type != L_MODE_VAL && type != L_MODE_COUNT)
        return (PIX *)ERROR_PTR("invalid type", procName, NULL);
    n = pixaGetCount(pixa);
    if (n == 0)
        return (PIX *)ERROR_PTR("no pix in pixa", procName, NULL);
    pixaGetPixDimensions(pixa, 0, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pix not 8 bpp", procName, NULL);

    pixd = pixCreate(w, h, 8);
    pixt = pixCreate(n, h, 8);
    colvect = (l_float32 *)CALLOC(h, sizeof(l_float32));
    for (j = 0; j < w; j++) {
        pixaExtractColumnFromEachPix(pixa, j, pixt);
        pixGetRowStats(pixt, type, nbins, thresh, colvect);
        pixSetPixelColumn(pixd, j, colvect);
    }

    FREE(colvect);
    pixDestroy(&pixt);
    return pixd;
}
    

/*!
 *  pixaExtractColumnFromEachPix()
 *
 *      Input:  pixa (of identically sized, 8 bpp; not cmapped)
 *              col (column index)
 *              pixd (pix into which each column is inserted)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaExtractColumnFromEachPix(PIXA    *pixa,
                             l_int32  col,
                             PIX     *pixd)
{
l_int32    i, k, n, w, h, ht, val, wplt, wpld;
l_uint32  *datad, *datat;
PIX       *pixt;

    PROCNAME("pixaExtractColumnFromEachPix");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (!pixd || pixGetDepth(pixd) != 8)
        return ERROR_INT("pixa not defined or not 8 bpp", procName, 1);
    n = pixaGetCount(pixa);
    pixGetDimensions(pixd, &w, &h, NULL);
    if (n != w)
        return ERROR_INT("pix width != n", procName, 1);
    pixt = pixaGetPix(pixa, 0, L_CLONE);
    wplt = pixGetWpl(pixt);
    pixGetDimensions(pixt, NULL, &ht, NULL);
    pixDestroy(&pixt);
    if (h != ht)
        return ERROR_INT("pixd height != column height", procName, 1);

    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    for (k = 0; k < n; k++) {
        pixt = pixaGetPix(pixa, k, L_CLONE);
        datat = pixGetData(pixt);
        for (i = 0; i < h; i++) {
            val = GET_DATA_BYTE(datat, col);
            SET_DATA_BYTE(datad + i * wpld, k, val);
            datat += wplt;
        }
        pixDestroy(&pixt);
    }

    return 0;
}


/*!
 *  pixGetRowStats()
 *
 *      Input:  pixs (8 bpp; not cmapped)
 *              type (L_MEAN_ABSVAL, L_MEDIAN_VAL, L_MODE_VAL, L_MODE_COUNT)
 *              nbins (of histogram for median and mode; ignored for mean)
 *              thresh (on histogram for mode; ignored for mean and median)
 *              colvect (vector of results gathered across the rows of pixs)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This computes a column vector of statistics using each
 *          row of a Pix.  The result is put in @colvect.
 *      (2) The @thresh parameter works with L_MODE_VAL only, and
 *          sets a minimum occupancy of the mode bin.
 *          If the occupancy of the mode bin is less than @thresh, the
 *          mode value is returned as 0.  To always return the actual
 *          mode value, set @thresh = 0.
 *      (3) What is the meaning of this @thresh parameter?
 *          For each row, the total count in the histogram is w, the
 *          image width.  So @thresh, relative to w, gives a measure
 *          of the ratio of the bin width to the width of the distribution.
 *          The larger @thresh, the narrower the distribution must be
 *          for the mode value to be returned (instead of returning 0).
 *      (4) If the Pix consists of a set of corresponding columns,
 *          one for each Pix in a Pixa, the width of the Pix is the
 *          number of Pix in the Pixa and the column vector can
 *          be stored as a column in a Pix of the same size as
 *          each Pix in the Pixa.
 */
l_int32
pixGetRowStats(PIX        *pixs,
               l_int32     type,
               l_int32     nbins,
               l_int32     thresh,
               l_float32  *colvect)
{
l_int32    i, j, k, w, h, val, wpls, sum, target, max, modeval;
l_int32   *histo, *gray2bin, *bin2gray;
l_uint32  *lines, *datas;

    PROCNAME("pixGetRowStats");

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!colvect)
        return ERROR_INT("colvect not defined", procName, 1);
    if (type != L_MEAN_ABSVAL && type != L_MEDIAN_VAL &&
        type != L_MODE_VAL && type != L_MODE_COUNT)
        return ERROR_INT("invalid type", procName, 1);
    if (type != L_MEAN_ABSVAL && (nbins < 1 || nbins > 256))
        return ERROR_INT("invalid nbins", procName, 1);
    pixGetDimensions(pixs, &w, &h, NULL);

    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    if (type == L_MEAN_ABSVAL) {
        for (i = 0; i < h; i++) {
            sum = 0;
            lines = datas + i * wpls;
            for (j = 0; j < w; j++)
                sum += GET_DATA_BYTE(lines, j);
            colvect[i] = (l_float32)sum / (l_float32)w;
        }
        return 0;
    }
            
        /* We need a histogram; binwidth ~ 256 / nbins */
    histo = (l_int32 *)CALLOC(nbins, sizeof(l_int32));
    gray2bin = (l_int32 *)CALLOC(256, sizeof(l_int32));
    bin2gray = (l_int32 *)CALLOC(nbins, sizeof(l_int32));
    for (i = 0; i < 256; i++)  /* gray value --> histo bin */
        gray2bin[i] = (i * nbins) / 256;
    for (i = 0; i < nbins; i++)  /* histo bin --> gray value */
        bin2gray[i] = (i * 256 + 128) / nbins;

    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        for (k = 0; k < nbins; k++)
            histo[k] = 0;
        for (j = 0; j < w; j++) {
            val = GET_DATA_BYTE(lines, j);
            histo[gray2bin[val]]++;
        }

        if (type == L_MEDIAN_VAL) {
            sum = 0;
            target = (w + 1) / 2;
            for (k = 0; k < nbins; k++) {
                sum += histo[k];
                if (sum >= target) {
                    colvect[i] = bin2gray[k];
                    break;
                }
            }
        }
        else if (type == L_MODE_VAL) {
            max = 0;
            modeval = 0;
            for (k = 0; k < nbins; k++) {
                if (histo[k] > max) {
                    max = histo[k];
                    modeval = k;
                }
            }
            if (max < thresh)
                colvect[i] = 0;
            else
                colvect[i] = bin2gray[modeval];
        }
        else {  /* type == L_MODE_COUNT */
            max = 0;
            modeval = 0;
            for (k = 0; k < nbins; k++) {
                if (histo[k] > max) {
                    max = histo[k];
                    modeval = k;
                }
            }
            colvect[i] = max;
        }
    }

    FREE(histo);
    FREE(gray2bin);
    FREE(bin2gray);
    return 0;
}


/*!
 *  pixGetColumnStats()
 *
 *      Input:  pixs (8 bpp; not cmapped)
 *              type (L_MEAN_ABSVAL, L_MEDIAN_VAL, L_MODE_VAL, L_MODE_COUNT)
 *              nbins (of histogram for median and mode; ignored for mean)
 *              thresh (on histogram for mode val; ignored for all other types)
 *              rowvect (vector of results gathered down the columns of pixs)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This computes a row vector of statistics using each
 *          column of a Pix.  The result is put in @rowvect.
 *      (2) The @thresh parameter works with L_MODE_VAL only, and
 *          sets a minimum occupancy of the mode bin.
 *          If the occupancy of the mode bin is less than @thresh, the
 *          mode value is returned as 0.  To always return the actual
 *          mode value, set @thresh = 0.
 *      (3) What is the meaning of this @thresh parameter?
 *          For each column, the total count in the histogram is h, the
 *          image height.  So @thresh, relative to h, gives a measure
 *          of the ratio of the bin width to the width of the distribution.
 *          The larger @thresh, the narrower the distribution must be
 *          for the mode value to be returned (instead of returning 0).
 */
l_int32
pixGetColumnStats(PIX        *pixs,
                  l_int32     type,
                  l_int32     nbins,
                  l_int32     thresh,
                  l_float32  *rowvect)
{
l_int32    i, j, k, w, h, val, wpls, sum, target, max, modeval;
l_int32   *histo, *gray2bin, *bin2gray;
l_uint32  *datas;

    PROCNAME("pixGetColumnStats");

    if (!pixs || pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not defined or not 8 bpp", procName, 1);
    if (!rowvect)
        return ERROR_INT("rowvect not defined", procName, 1);
    if (type != L_MEAN_ABSVAL && type != L_MEDIAN_VAL &&
        type != L_MODE_VAL && type != L_MODE_COUNT)
        return ERROR_INT("invalid type", procName, 1);
    if (type != L_MEAN_ABSVAL && (nbins < 1 || nbins > 256))
        return ERROR_INT("invalid nbins", procName, 1);
    pixGetDimensions(pixs, &w, &h, NULL);

    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    if (type == L_MEAN_ABSVAL) {
        for (j = 0; j < w; j++) {
            sum = 0;
            for (i = 0; i < h; i++)
                sum += GET_DATA_BYTE(datas + i * wpls, j);
            rowvect[j] = (l_float32)sum / (l_float32)h;
        }
        return 0;
    }
            
        /* We need a histogram; binwidth ~ 256 / nbins */
    histo = (l_int32 *)CALLOC(nbins, sizeof(l_int32));
    gray2bin = (l_int32 *)CALLOC(256, sizeof(l_int32));
    bin2gray = (l_int32 *)CALLOC(nbins, sizeof(l_int32));
    for (i = 0; i < 256; i++)  /* gray value --> histo bin */
        gray2bin[i] = (i * nbins) / 256;
    for (i = 0; i < nbins; i++)  /* histo bin --> gray value */
        bin2gray[i] = (i * 256 + 128) / nbins;

    for (j = 0; j < w; j++) {
        for (i = 0; i < h; i++) {
            val = GET_DATA_BYTE(datas + i * wpls, j);
            histo[gray2bin[val]]++;
        }

        if (type == L_MEDIAN_VAL) {
            sum = 0;
            target = (h + 1) / 2;
            for (k = 0; k < nbins; k++) {
                sum += histo[k];
                if (sum >= target) {
                    rowvect[j] = bin2gray[k];
                    break;
                }
            }
        }
        else if (type == L_MODE_VAL) {
            max = 0;
            modeval = 0;
            for (k = 0; k < nbins; k++) {
                if (histo[k] > max) {
                    max = histo[k];
                    modeval = k;
                }
            }
            if (max < thresh)
                rowvect[j] = 0;
            else
                rowvect[j] = bin2gray[modeval];
        }
        else {  /* type == L_MODE_COUNT */
            max = 0;
            modeval = 0;
            for (k = 0; k < nbins; k++) {
                if (histo[k] > max) {
                    max = histo[k];
                    modeval = k;
                }
            }
            rowvect[j] = max;
        }
        for (k = 0; k < nbins; k++)
            histo[k] = 0;
    }

    FREE(histo);
    FREE(gray2bin);
    FREE(bin2gray);
    return 0;
}


/*!
 *  pixSetPixelColumn()
 *
 *      Input:  pix (8 bpp; not cmapped)
 *              col (column index)
 *              colvect (vector of floats)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixSetPixelColumn(PIX        *pix,
                  l_int32     col,
                  l_float32  *colvect)
{
l_int32    i, w, h, wpl;
l_uint32  *data;

    PROCNAME("pixSetCPixelColumn");

    if (!pix || pixGetDepth(pix) != 8)
        return ERROR_INT("pix not defined or not 8 bpp", procName, 1);
    if (!colvect)
        return ERROR_INT("colvect not defined", procName, 1);
    pixGetDimensions(pix, &w, &h, NULL);
    if (col < 0 || col > w)
        return ERROR_INT("invalid col", procName, 1);

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    for (i = 0; i < h; i++)
        SET_DATA_BYTE(data + i * wpl, col, (l_int32)colvect[i]);

    return 0;
}


/*-------------------------------------------------------------*
 *              Foreground/background estimation               *
 *-------------------------------------------------------------*/
/*!
 *  pixThresholdForFgBg()
 *
 *      Input:  pixs (any depth; cmapped ok)
 *              factor (subsampling factor; integer >= 1)
 *              thresh (threshold for generating foreground mask)
 *              &fgval (<optional return> average foreground value)
 *              &bgval (<optional return> average background value)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixThresholdForFgBg(PIX      *pixs,
                    l_int32   factor,
                    l_int32   thresh,
                    l_int32  *pfgval,
                    l_int32  *pbgval)
{
l_float32  fval;
PIX       *pixg, *pixm;

    PROCNAME("pixThresholdForFgBg");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

        /* Generate a subsampled 8 bpp version and a mask over the fg */
    pixg = pixConvertTo8BySampling(pixs, factor, 0);
    pixm = pixThresholdToBinary(pixg, thresh);

    if (pfgval) {
        pixGetAverageMasked(pixg, pixm, 0, 0, 1, L_MEAN_ABSVAL, &fval);
        *pfgval = (l_int32)(fval + 0.5);
    }

    if (pbgval) {
        pixInvert(pixm, pixm);
        pixGetAverageMasked(pixg, pixm, 0, 0, 1, L_MEAN_ABSVAL, &fval);
        *pbgval = (l_int32)(fval + 0.5);
    }

    pixDestroy(&pixg);
    pixDestroy(&pixm);
    return 0;
}


/*!
 *  pixSplitDistributionFgBg()
 *
 *      Input:  pixs (any depth; cmapped ok)
 *              scorefract (fraction of the max score, used to determine
 *                          the range over which the histogram min is searched)
 *              factor (subsampling factor; integer >= 1)
 *              &thresh (<optional return> best threshold for separating)
 *              &fgval (<optional return> average foreground value)
 *              &bgval (<optional return> average background value)
 *              debugflag (1 for plotting of distribution and split point)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See numaSplitDistribution() for details on the underlying
 *          method of choosing a threshold.
 */
l_int32
pixSplitDistributionFgBg(PIX       *pixs,
                         l_float32  scorefract,
                         l_int32    factor,
                         l_int32   *pthresh,
                         l_int32   *pfgval,
                         l_int32   *pbgval,
                         l_int32    debugflag)
{
l_int32    thresh;
l_float32  avefg, avebg, maxnum;
GPLOT     *gplot;
NUMA      *na, *nascore, *nax, *nay;
PIX       *pixg;

    PROCNAME("pixSplitDistributionFgBg");

    if (pthresh) *pthresh = 0;
    if (pfgval) *pfgval = 0;
    if (pbgval) *pbgval = 0;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

        /* Generate a subsampled 8 bpp version */
    pixg = pixConvertTo8BySampling(pixs, factor, 0);

        /* Make the fg/bg estimates */
    na = pixGetGrayHistogram(pixg, 1);
    if (debugflag) {
        numaSplitDistribution(na, scorefract, &thresh, &avefg, &avebg,
                              NULL, NULL, &nascore);
        numaDestroy(&nascore);
    }
    else
        numaSplitDistribution(na, scorefract, &thresh, &avefg, &avebg,
                              NULL, NULL, NULL);

    if (pthresh) *pthresh = thresh;
    if (pfgval) *pfgval = (l_int32)(avefg + 0.5);
    if (pbgval) *pbgval = (l_int32)(avebg + 0.5);

    if (debugflag) {
        gplot = gplotCreate("junk_histplot", GPLOT_X11, "Histogram",
                            "Grayscale value", "Number of pixels");
        gplotAddPlot(gplot, NULL, na, GPLOT_LINES, NULL);
        nax = numaMakeConstant(thresh, 2);
        numaGetMax(na, &maxnum, NULL);
        nay = numaMakeConstant(0, 2);
        numaReplaceNumber(nay, 1, (l_int32)(0.5 * maxnum));
        gplotAddPlot(gplot, nax, nay, GPLOT_LINES, NULL);
        gplotMakeOutput(gplot);
        gplotDestroy(&gplot);
        numaDestroy(&nax);
        numaDestroy(&nay);
    }

    pixDestroy(&pixg);
    numaDestroy(&na);
    return 0;
}


/*-------------------------------------------------------------*
 *                 Measurement of properties                   *
 *-------------------------------------------------------------*/
/*!
 *  pixaFindDimensions()
 *
 *      Input:  pixa
 *              &naw (<optional return> numa of pix widths)
 *              &nah (<optional return> numa of pix heights)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaFindDimensions(PIXA   *pixa,
                   NUMA  **pnaw,
                   NUMA  **pnah)
{
l_int32  i, n, w, h;
PIX     *pixt;

    PROCNAME("pixaFindDimensions");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (!pnaw && !pnah) 
        return 0;

    n = pixaGetCount(pixa);
    if (pnaw) *pnaw = numaCreate(n);
    if (pnah) *pnah = numaCreate(n);
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixGetDimensions(pixt, &w, &h, NULL);
        if (pnaw)
            numaAddNumber(*pnaw, w);
        if (pnah)
            numaAddNumber(*pnah, h);
        pixDestroy(&pixt);
    }
    return 0;
}


/*!
 *  pixaFindAreaPerimRatio()
 *
 *      Input:  pixa (of 1 bpp pix)
 *      Return: na (of area/perimeter ratio for each pix), or null on error
 *
 *  Notes:
 *      (1) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
NUMA *
pixaFindAreaPerimRatio(PIXA  *pixa)
{
l_int32    i, n;
l_int32   *tab;
l_float32  fract;
NUMA      *na;
PIX       *pixt;

    PROCNAME("pixaFindAreaPerimRatio");

    if (!pixa)
        return (NUMA *)ERROR_PTR("pixa not defined", procName, NULL);

    n = pixaGetCount(pixa);
    na = numaCreate(n);
    tab = makePixelSumTab8();
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixFindAreaPerimRatio(pixt, tab, &fract);
        numaAddNumber(na, fract);
        pixDestroy(&pixt);
    }
    FREE(tab);
    return na;
}


/*!
 *  pixFindAreaPerimRatio()
 *
 *      Input:  pixs (1 bpp)
 *              tab (<optional> pixel sum table, can be NULL)
 *              &fract (<return> area/perimeter ratio)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The area is the number of fg pixels that are not on the
 *          boundary (i.e., not 8-connected to a bg pixel), and the
 *          perimeter is the number of boundary fg pixels.
 *      (2) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
l_int32
pixFindAreaPerimRatio(PIX        *pixs,
                      l_int32    *tab,
                      l_float32  *pfract)
{
l_int32  *tab8;
l_int32   nin, nbound;
PIX      *pixt;

    PROCNAME("pixFindAreaPerimRatio");

    if (!pfract)
        return ERROR_INT("&fract not defined", procName, 1);
    *pfract = 0.0;
    if (!pixs || pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

    if (!tab)
        tab8 = makePixelSumTab8();
    else
        tab8 = tab;

    pixt = pixErodeBrick(NULL, pixs, 3, 3);
    pixCountPixels(pixt, &nin, tab8);
    pixXor(pixt, pixt, pixs);
    pixCountPixels(pixt, &nbound, tab8);
    *pfract = (l_float32)nin / (l_float32)nbound;

    if (!tab)
        FREE(tab8);
    pixDestroy(&pixt);
    return 0;
}


/*!
 *  pixaFindPerimSizeRatio()
 *
 *      Input:  pixa (of 1 bpp pix)
 *      Return: na (of fg perimeter/(w*h) ratio for each pix), or null on error
 *
 *  Notes:
 *      (1) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
NUMA *
pixaFindPerimSizeRatio(PIXA  *pixa)
{
l_int32    i, n;
l_int32   *tab;
l_float32  ratio;
NUMA      *na;
PIX       *pixt;

    PROCNAME("pixaFindPerimSizeRatio");

    if (!pixa)
        return (NUMA *)ERROR_PTR("pixa not defined", procName, NULL);

    n = pixaGetCount(pixa);
    na = numaCreate(n);
    tab = makePixelSumTab8();
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixFindPerimSizeRatio(pixt, tab, &ratio);
        numaAddNumber(na, ratio);
        pixDestroy(&pixt);
    }
    FREE(tab);
    return na;
}


/*!
 *  pixFindPerimSizeRatio()
 *
 *      Input:  pixs (1 bpp)
 *              tab (<optional> pixel sum table, can be NULL)
 *              &ratio (<return> perimeter/size ratio)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The size is the sum of the width and height of the pix,
 *          and the perimeter is the number of boundary fg pixels.
 *      (2) This has a large value for dendritic, fractal-like components
 *          with highly irregular boundaries.
 *      (3) This is typically used for a single connected component.
 */
l_int32
pixFindPerimSizeRatio(PIX        *pixs,
                      l_int32    *tab,
                      l_float32  *pratio)
{
l_int32  *tab8;
l_int32   w, h, nbound;
PIX      *pixt;

    PROCNAME("pixFindPerimSizeRatio");

    if (!pratio)
        return ERROR_INT("&ratio not defined", procName, 1);
    *pratio = 0.0;
    if (!pixs || pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

    if (!tab)
        tab8 = makePixelSumTab8();
    else
        tab8 = tab;

    pixt = pixErodeBrick(NULL, pixs, 3, 3);
    pixXor(pixt, pixt, pixs);
    pixCountPixels(pixt, &nbound, tab8);
    pixGetDimensions(pixs, &w, &h, NULL);
    *pratio = (l_float32)nbound / (l_float32)(w + h);

    if (!tab)
        FREE(tab8);
    pixDestroy(&pixt);
    return 0;
}


/*!
 *  pixaFindAreaFraction()
 *
 *      Input:  pixa (of 1 bpp pix)
 *      Return: na (of area fractions for each pix), or null on error
 *
 *  Notes:
 *      (1) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
NUMA *
pixaFindAreaFraction(PIXA  *pixa)
{
l_int32    i, n;
l_int32   *tab;
l_float32  fract;
NUMA      *na;
PIX       *pixt;

    PROCNAME("pixaFindAreaFraction");

    if (!pixa)
        return (NUMA *)ERROR_PTR("pixa not defined", procName, NULL);

    n = pixaGetCount(pixa);
    na = numaCreate(n);
    tab = makePixelSumTab8();
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixFindAreaFraction(pixt, tab, &fract);
        numaAddNumber(na, fract);
        pixDestroy(&pixt);
    }
    FREE(tab);
    return na;
}


/*!
 *  pixFindAreaFraction()
 *
 *      Input:  pixs (1 bpp)
 *              tab (<optional> pixel sum table, can be NULL)
 *              &fract (<return> fg area/size ratio)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This finds the ratio of the number of fg pixels to the
 *          size of the pix (w * h).  It is typically used for a
 *          single connected component.
 */
l_int32
pixFindAreaFraction(PIX        *pixs,
                    l_int32    *tab,
                    l_float32  *pfract)
{
l_int32   w, h, d, sum;
l_int32  *tab8;

    PROCNAME("pixFindAreaFraction");

    if (!pfract)
        return ERROR_INT("&fract not defined", procName, 1);
    *pfract = 0.0;
    pixGetDimensions(pixs, &w, &h, &d);
    if (!pixs || d != 1)
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

    if (!tab)
        tab8 = makePixelSumTab8();
    else
        tab8 = tab;

    pixCountPixels(pixs, &sum, tab8);
    *pfract = (l_float32)sum / (l_float32)(w * h);

    if (!tab)
        FREE(tab8);
    return 0;
}


/*!
 *  pixaFindWidthHeightRatio()
 *
 *      Input:  pixa (of 1 bpp pix)
 *      Return: na (of width/height ratios for each pix), or null on error
 *
 *  Notes:
 *      (1) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
NUMA *
pixaFindWidthHeightRatio(PIXA  *pixa)
{
l_int32  i, n, w, h;
NUMA    *na;
PIX     *pixt;

    PROCNAME("pixaFindWidthHeightRatio");

    if (!pixa)
        return (NUMA *)ERROR_PTR("pixa not defined", procName, NULL);

    n = pixaGetCount(pixa);
    na = numaCreate(n);
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixGetDimensions(pixt, &w, &h, NULL);
        numaAddNumber(na, (l_float32)w / (l_float32)h);
        pixDestroy(&pixt);
    }
    return na;
}


/*!
 *  pixaFindWidthHeightProduct()
 *
 *      Input:  pixa (of 1 bpp pix)
 *      Return: na (of width*height products for each pix), or null on error
 *
 *  Notes:
 *      (1) This is typically used for a pixa consisting of
 *          1 bpp connected components.
 */
NUMA *
pixaFindWidthHeightProduct(PIXA  *pixa)
{
l_int32  i, n, w, h;
NUMA    *na;
PIX     *pixt;

    PROCNAME("pixaFindWidthHeightProduct");

    if (!pixa)
        return (NUMA *)ERROR_PTR("pixa not defined", procName, NULL);

    n = pixaGetCount(pixa);
    na = numaCreate(n);
    for (i = 0; i < n; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixGetDimensions(pixt, &w, &h, NULL);
        numaAddNumber(na, w * h);
        pixDestroy(&pixt);
    }
    return na;
}


/*-------------------------------------------------------------*
 *                Extract rectangular region                   *
 *-------------------------------------------------------------*/
/*!
 *  pixClipRectangle()
 *
 *      Input:  pixs
 *              box  (requested clipping region; const)
 *              &boxc (<optional return> actual box of clipped region)
 *      Return: clipped pix, or null on error or if rectangle
 *              doesn't intersect pixs
 *
 *  Notes:
 *
 *  This should be simple, but there are choices to be made.
 *  The box is defined relative to the pix coordinates.  However,
 *  if the box is not contained within the pix, we have two choices:
 *
 *      (1) clip the box to the pix
 *      (2) make a new pix equal to the full box dimensions,
 *          but let rasterop do the clipping and positioning
 *          of the src with respect to the dest
 *
 *  Choice (2) immediately brings up the problem of what pixel values
 *  to use that were not taken from the src.  For example, on a grayscale
 *  image, do you want the pixels not taken from the src to be black
 *  or white or something else?  To implement choice 2, one needs to
 *  specify the color of these extra pixels.
 *
 *  So we adopt (1), and clip the box first, if necessary,
 *  before making the dest pix and doing the rasterop.  But there
 *  is another issue to consider.  If you want to paste the
 *  clipped pix back into pixs, it must be properly aligned, and
 *  it is necessary to use the clipped box for alignment.
 *  Accordingly, this function has a third (optional) argument, which is
 *  the input box clipped to the src pix.
 */
PIX *
pixClipRectangle(PIX   *pixs,
                 BOX   *box,
                 BOX  **pboxc)
{
l_int32  w, h, d, bx, by, bw, bh;
BOX     *boxc;
PIX     *pixd;

    PROCNAME("pixClipRectangle");

    if (pboxc)
        *pboxc = NULL;
    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!box)
        return (PIX *)ERROR_PTR("box not defined", procName, NULL);

        /* Clip the input box to the pix */
    pixGetDimensions(pixs, &w, &h, &d);
    if ((boxc = boxClipToRectangle(box, w, h)) == NULL) {
        L_WARNING("box doesn't overlap pix", procName);
        return NULL;
    }
    boxGetGeometry(boxc, &bx, &by, &bw, &bh);

        /* Extract the block */
    if ((pixd = pixCreate(bw, bh, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);
    pixRasterop(pixd, 0, 0, bw, bh, PIX_SRC, pixs, bx, by);

    if (pboxc)
        *pboxc = boxc;
    else
        boxDestroy(&boxc);

    return pixd;
}


/*!
 *  pixClipMasked()
 *
 *      Input:  pixs (1, 2, 4, 8, 16, 32 bpp; colormap ok)
 *              pixm  (clipping mask, 1 bpp)
 *              x, y (origin of clipping mask relative to pixs)
 *              outval (val to use for pixels that are outside the mask)
 *      Return: pixd, (clipped pix) or null on error or if pixm doesn't
 *              intersect pixs
 *
 *  Notes:
 *      (1) If pixs has a colormap, it is preserved in pixd.
 *      (2) The depth of pixd is the same as that of pixs.
 *      (3) If the depth of pixs is 1, use @outval = 0 for white background
 *          and 1 for black; otherwise, use the max value for white
 *          and 0 for black.  If pixs has a colormap, the max value for
 *          @outval is 0xffffffff; otherwise, it is 2^d - 1.
 *      (4) When using 1 bpp pixs, this is a simple clip and
 *          blend operation.  For example, if both pix1 and pix2 are
 *          black text on white background, and you want to OR the
 *          fg on the two images, let pixm be the inverse of pix2.
 *          Then the operation takes all of pix1 that's in the bg of
 *          pix2, and for the remainder (which are the pixels
 *          corresponding to the fg of the pix2), paint them black
 *          (1) in pix1.  The function call looks like
 *             pixClipMasked(pix2, pixInvert(pix1, pix1), x, y, 1);
 */
PIX *
pixClipMasked(PIX      *pixs,
              PIX      *pixm,
              l_int32   x,
              l_int32   y,
              l_uint32  outval)
{
l_int32   wm, hm, d, index, rval, gval, bval;
l_uint32  pixel;
BOX      *box;
PIX      *pixmi, *pixd;
PIXCMAP  *cmap;

    PROCNAME("pixClipMasked");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!pixm || pixGetDepth(pixm) != 1)
        return (PIX *)ERROR_PTR("pixm undefined or not 1 bpp", procName, NULL);

        /* Clip out the region specified by pixm and (x,y) */
    pixGetDimensions(pixm, &wm, &hm, NULL);
    box = boxCreate(x, y, wm, hm);
    pixd = pixClipRectangle(pixs, box, NULL);

        /* Paint 'outval' (or something close to it if cmapped) through
         * the pixels not masked by pixm */
    cmap = pixGetColormap(pixd);
    pixmi = pixInvert(NULL, pixm);
    d = pixGetDepth(pixd);
    if (cmap) {
        extractRGBValues(outval, &rval, &gval, &bval);
        pixcmapGetNearestIndex(cmap, rval, gval, bval, &index);
        pixcmapGetColor(cmap, index, &rval, &gval, &bval);
        composeRGBPixel(rval, gval, bval, &pixel);
        pixPaintThroughMask(pixd, pixmi, 0, 0, pixel);
    }
    else
        pixPaintThroughMask(pixd, pixmi, 0, 0, outval);

    boxDestroy(&box);
    pixDestroy(&pixmi);
    return pixd;
}


/*---------------------------------------------------------------------*
 *                          Clipping to Foreground                     *
 *---------------------------------------------------------------------*/
/*!
 *  pixClipToForeground()
 *
 *      Input:  pixs (1 bpp)
 *              &pixd  (<optional return> clipped pix returned)
 *              &box   (<optional return> bounding box)
 *      Return: 0 if OK; 1 on error or if there are no fg pixels
 *
 *  Notes:
 *      (1) At least one of {&pixd, &box} must be specified.
 *      (2) If there are no fg pixels, the returned ptrs are null.
 */
l_int32
pixClipToForeground(PIX   *pixs,
                    PIX  **ppixd,
                    BOX  **pbox)
{
l_int32    w, h, wpl, nfullwords, extra, i, j;
l_int32    minx, miny, maxx, maxy;
l_uint32   result, mask;
l_uint32  *data, *line;
BOX       *box;

    PROCNAME("pixClipToForeground");

    if (!ppixd && !pbox)
        return ERROR_INT("neither &pixd nor &box defined", procName, 1);
    if (ppixd)
        *ppixd = NULL;
    if (pbox)
        *pbox = NULL;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

    pixGetDimensions(pixs, &w, &h, NULL);
    nfullwords = w / 32;
    extra = w & 31;
    mask = ~rmask32[32 - extra];
    wpl = pixGetWpl(pixs);
    data = pixGetData(pixs);

    result = 0;
    for (i = 0, miny = 0; i < h; i++, miny++) {
        line = data + i * wpl;
        for (j = 0; j < nfullwords; j++)
            result |= line[j];
        if (extra)
            result |= (line[j] & mask);
        if (result)
            break;
    }
    if (miny == h)  /* no ON pixels */
        return 1;

    result = 0;
    for (i = h - 1, maxy = h - 1; i >= 0; i--, maxy--) {
        line = data + i * wpl;
        for (j = 0; j < nfullwords; j++)
            result |= line[j];
        if (extra)
            result |= (line[j] & mask);
        if (result)
            break;
    }

    minx = 0;
    for (j = 0, minx = 0; j < w; j++, minx++) {
        for (i = 0; i < h; i++) {
            line = data + i * wpl;
            if (GET_DATA_BIT(line, j))
                goto minx_found;
        }
    }

minx_found:
    for (j = w - 1, maxx = w - 1; j >= 0; j--, maxx--) {
        for (i = 0; i < h; i++) {
            line = data + i * wpl;
            if (GET_DATA_BIT(line, j))
                goto maxx_found;
        }
    }

maxx_found:
    box = boxCreate(minx, miny, maxx - minx + 1, maxy - miny + 1);

    if (ppixd)
        *ppixd = pixClipRectangle(pixs, box, NULL);
    if (pbox)
        *pbox = box;
    else
        boxDestroy(&box);

    return 0;
}


/*!
 *  pixClipBoxToForeground()
 *
 *      Input:  pixs (1 bpp)
 *              boxs  (<optional> ; use full image if null)
 *              &pixd  (<optional return> clipped pix returned)
 *              &boxd  (<optional return> bounding box)
 *      Return: 0 if OK; 1 on error or if there are no fg pixels
 *
 *  Notes:
 *      (1) At least one of {&pixd, &boxd} must be specified.
 *      (2) If there are no fg pixels, the returned ptrs are null.
 *      (3) Do not use &pixs for the 3rd arg or &boxs for the 4th arg;
 *          this will leak memory.
 */
l_int32
pixClipBoxToForeground(PIX   *pixs,
                       BOX   *boxs,
                       PIX  **ppixd,
                       BOX  **pboxd)
{
l_int32  w, h, bx, by, bw, bh, cbw, cbh, left, right, top, bottom;
BOX     *boxt, *boxd;

    PROCNAME("pixClipBoxToForeground");

    if (!ppixd && !pboxd)
        return ERROR_INT("neither &pixd nor &boxd defined", procName, 1);
    if (ppixd) *ppixd = NULL;
    if (pboxd) *pboxd = NULL;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

    if (!boxs)
        return pixClipToForeground(pixs, ppixd, pboxd);

    pixGetDimensions(pixs, &w, &h, NULL);
    boxGetGeometry(boxs, &bx, &by, &bw, &bh);
    cbw = L_MIN(bw, w - bx);
    cbh = L_MIN(bh, h - by);
    if (cbw < 0 || cbh < 0)
        return ERROR_INT("box not within image", procName, 1);
    boxt = boxCreate(bx, by, cbw, cbh);

    if (pixScanForForeground(pixs, boxt, L_FROM_LEFT, &left)) {
        boxDestroy(&boxt);
        return 1;
    }
    pixScanForForeground(pixs, boxt, L_FROM_RIGHT, &right);
    pixScanForForeground(pixs, boxt, L_FROM_TOP, &top);
    pixScanForForeground(pixs, boxt, L_FROM_BOTTOM, &bottom);

    boxd = boxCreate(left, top, right - left + 1, bottom - top + 1);
    if (ppixd)
        *ppixd = pixClipRectangle(pixs, boxd, NULL);
    if (pboxd)
        *pboxd = boxd;
    else
        boxDestroy(&boxd);

    boxDestroy(&boxt);
    return 0;
}


/*!
 *  pixScanForForeground()
 *
 *      Input:  pixs (1 bpp)
 *              box  (<optional> within which the search is conducted)
 *              scanflag (direction of scan; e.g., L_FROM_LEFT)
 *              &loc (location in scan direction of first black pixel)
 *      Return: 0 if OK; 1 on error or if no fg pixels are found
 *
 *  Notes:
 *      (1) If there are no fg pixels, the position is set to 0.
 *          Caller must check the return value!
 *      (2) Use @box == NULL to scan from edge of pixs
 */
l_int32
pixScanForForeground(PIX      *pixs,
                     BOX      *box,
                     l_int32   scanflag,
                     l_int32  *ploc)
{
l_int32    bx, by, bw, bh, x, xstart, xend, y, ystart, yend, wpl;
l_uint32  *data, *line;
BOX       *boxt;

    PROCNAME("pixScanForForeground");

    if (!ploc)
        return ERROR_INT("&ploc not defined", procName, 1);
    *ploc = 0;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);

        /* Clip box to pixs if it exists */
    pixGetDimensions(pixs, &bw, &bh, NULL);
    if (box) {
        if ((boxt = boxClipToRectangle(box, bw, bh)) == NULL)
            return ERROR_INT("invalid box", procName, 1);
        boxGetGeometry(boxt, &bx, &by, &bw, &bh);
        boxDestroy(&boxt);
    }
    else
        bx = by = 0;
    xstart = bx;
    ystart = by;
    xend = bx + bw - 1;
    yend = by + bh - 1;

    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    if (scanflag == L_FROM_LEFT) {
        for (x = xstart; x <= xend; x++) {
            for (y = ystart; y <= yend; y++) {
                line = data + y * wpl;
                if (GET_DATA_BIT(line, x)) {
                    *ploc = x;
                    return 0;
                }
            }
        }
    }
    else if (scanflag == L_FROM_RIGHT) {
        for (x = xend; x >= xstart; x--) {
            for (y = ystart; y <= yend; y++) {
                line = data + y * wpl;
                if (GET_DATA_BIT(line, x)) {
                    *ploc = x;
                    return 0;
                }
            }
        }
    }
    else if (scanflag == L_FROM_TOP) {
        for (y = ystart; y <= yend; y++) {
            line = data + y * wpl;
            for (x = xstart; x <= xend; x++) {
                if (GET_DATA_BIT(line, x)) {
                    *ploc = y;
                    return 0;
                }
            }
        }
    }
    else if (scanflag == L_FROM_BOTTOM) {
        for (y = yend; y >= ystart; y--) {
            line = data + y * wpl;
            for (x = xstart; x <= xend; x++) {
                if (GET_DATA_BIT(line, x)) {
                    *ploc = y;
                    return 0;
                }
            }
        }
    }
    else
        return ERROR_INT("invalid scanflag", procName, 1);

    return 1;  /* no fg found */
}


/*!
 *  pixClipBoxToEdges()
 *
 *      Input:  pixs (1 bpp)
 *              boxs  (<optional> ; use full image if null)
 *              lowthresh (threshold to choose clipping location)
 *              highthresh (threshold required to find an edge)
 *              maxwidth (max allowed width between low and high thresh locs)
 *              factor (sampling factor along pixel counting direction)
 *              &pixd  (<optional return> clipped pix returned)
 *              &boxd  (<optional return> bounding box)
 *      Return: 0 if OK; 1 on error or if a fg edge is not found from
 *              all four sides.
 *
 *  Notes:
 *      (1) At least one of {&pixd, &boxd} must be specified.
 *      (2) If there are no fg pixels, the returned ptrs are null.
 *      (3) This function attempts to locate rectangular "image" regions
 *          of high-density fg pixels, that have well-defined edges
 *          on the four sides.
 *      (4) Edges are searched for on each side, iterating in order
 *          from left, right, top and bottom.  As each new edge is
 *          found, the search box is resized to use that location.
 *          Once an edge is found, it is held.  If no more edges
 *          are found in one iteration, the search fails.
 *      (5) See pixScanForEdge() for usage of the thresholds and @maxwidth.
 *      (6) The thresholds must be at least 1, and the low threshold
 *          cannot be larger than the high threshold.
 *      (7) If the low and high thresholds are both 1, this is equivalent
 *          to pixClipBoxToForeground().
 */
l_int32
pixClipBoxToEdges(PIX     *pixs,
                  BOX     *boxs,
                  l_int32  lowthresh,
                  l_int32  highthresh,
                  l_int32  maxwidth,
                  l_int32  factor,
                  PIX    **ppixd,
                  BOX    **pboxd)
{
l_int32  w, h, bx, by, bw, bh, cbw, cbh, left, right, top, bottom;
l_int32  lfound, rfound, tfound, bfound, change;
BOX     *boxt, *boxd;

    PROCNAME("pixClipBoxToEdges");

    if (!ppixd && !pboxd)
        return ERROR_INT("neither &pixd nor &boxd defined", procName, 1);
    if (ppixd) *ppixd = NULL;
    if (pboxd) *pboxd = NULL;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);
    if (lowthresh < 1 || highthresh < 1 ||
        lowthresh > highthresh || maxwidth < 1)
        return ERROR_INT("invalid thresholds", procName, 1);
    factor = L_MIN(1, factor);

    if (lowthresh == 1 && highthresh == 1)
        return pixClipBoxToForeground(pixs, boxs, ppixd, pboxd);

    pixGetDimensions(pixs, &w, &h, NULL);
    if (boxs) {
        boxGetGeometry(boxs, &bx, &by, &bw, &bh);
        cbw = L_MIN(bw, w - bx);
        cbh = L_MIN(bh, h - by);
        if (cbw < 0 || cbh < 0)
            return ERROR_INT("box not within image", procName, 1);
        boxt = boxCreate(bx, by, cbw, cbh);
    }
    else
        boxt = boxCreate(0, 0, w, h);

    lfound = rfound = tfound = bfound = 0;
    while (!lfound || !rfound || !tfound || !bfound) { 
        change = 0;
        if (!lfound) {
            if (!pixScanForEdge(pixs, boxt, lowthresh, highthresh, maxwidth,
                                factor, L_FROM_LEFT, &left)) {
                lfound = 1;
                change = 1;
                boxRelocateOneSide(boxt, boxt, left, L_FROM_LEFT);
            }
        }
        if (!rfound) {
            if (!pixScanForEdge(pixs, boxt, lowthresh, highthresh, maxwidth,
                                factor, L_FROM_RIGHT, &right)) {
                rfound = 1;
                change = 1;
                boxRelocateOneSide(boxt, boxt, right, L_FROM_RIGHT);
            }
        }
        if (!tfound) {
            if (!pixScanForEdge(pixs, boxt, lowthresh, highthresh, maxwidth,
                                factor, L_FROM_TOP, &top)) {
                tfound = 1;
                change = 1;
                boxRelocateOneSide(boxt, boxt, top, L_FROM_TOP);
            }
        }
        if (!bfound) {
            if (!pixScanForEdge(pixs, boxt, lowthresh, highthresh, maxwidth,
                                factor, L_FROM_BOTTOM, &bottom)) {
                bfound = 1;
                change = 1;
                boxRelocateOneSide(boxt, boxt, bottom, L_FROM_BOTTOM);
            }
        }

#if DEBUG_EDGES
        fprintf(stderr, "iter: %d %d %d %d\n", lfound, rfound, tfound, bfound);
#endif  /* DEBUG_EDGES */

        if (change == 0) break;
    }
    boxDestroy(&boxt);

    if (change == 0)
        return ERROR_INT("not all edges found", procName, 1);

    boxd = boxCreate(left, top, right - left + 1, bottom - top + 1);
    if (ppixd)
        *ppixd = pixClipRectangle(pixs, boxd, NULL);
    if (pboxd)
        *pboxd = boxd;
    else
        boxDestroy(&boxd);

    return 0;
}


/*!
 *  pixScanForEdge()
 *
 *      Input:  pixs (1 bpp)
 *              box  (<optional> within which the search is conducted)
 *              lowthresh (threshold to choose clipping location)
 *              highthresh (threshold required to find an edge)
 *              maxwidth (max allowed width between low and high thresh locs)
 *              factor (sampling factor along pixel counting direction)
 *              scanflag (direction of scan; e.g., L_FROM_LEFT)
 *              &loc (location in scan direction of first black pixel)
 *      Return: 0 if OK; 1 on error or if the edge is not found
 *
 *  Notes:
 *      (1) If there are no fg pixels, the position is set to 0.
 *          Caller must check the return value!
 *      (2) Use @box == NULL to scan from edge of pixs
 *      (3) As the scan progresses, the location where the sum of
 *          pixels equals or excees @lowthresh is noted (loc).  The
 *          scan is stopped when the sum of pixels equals or exceeds
 *          @highthresh.  If the scan distance between loc and that
 *          point does not exceed @maxwidth, an edge is found and
 *          its position is taken to be loc.  @maxwidth implicitly
 *          sets a minimum on the required gradient of the edge.
 *      (4) The thresholds must be at least 1, and the low threshold
 *          cannot be larger than the high threshold.
 */
l_int32
pixScanForEdge(PIX      *pixs,
               BOX      *box,
               l_int32   lowthresh,
               l_int32   highthresh,
               l_int32   maxwidth,
               l_int32   factor,
               l_int32   scanflag,
               l_int32  *ploc)
{
l_int32    bx, by, bw, bh, foundmin, loc, sum, wpl;
l_int32    x, xstart, xend, y, ystart, yend;
l_uint32  *data, *line;
BOX       *boxt;

    PROCNAME("pixScanForEdge");

    if (!ploc)
        return ERROR_INT("&ploc not defined", procName, 1);
    *ploc = 0;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return ERROR_INT("pixs not defined or not 1 bpp", procName, 1);
    if (lowthresh < 1 || highthresh < 1 ||
        lowthresh > highthresh || maxwidth < 1)
        return ERROR_INT("invalid thresholds", procName, 1);
    factor = L_MIN(1, factor);

        /* Clip box to pixs if it exists */
    pixGetDimensions(pixs, &bw, &bh, NULL);
    if (box) {
        if ((boxt = boxClipToRectangle(box, bw, bh)) == NULL)
            return ERROR_INT("invalid box", procName, 1);
        boxGetGeometry(boxt, &bx, &by, &bw, &bh);
        boxDestroy(&boxt);
    }
    else
        bx = by = 0;
    xstart = bx;
    ystart = by;
    xend = bx + bw - 1;
    yend = by + bh - 1;

    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    foundmin = 0;
    if (scanflag == L_FROM_LEFT) {
        for (x = xstart; x <= xend; x++) {
            sum = 0;
            for (y = ystart; y <= yend; y += factor) {
                line = data + y * wpl;
                if (GET_DATA_BIT(line, x))
                    sum++;
            }
            if (!foundmin && sum < lowthresh)
                continue;
            if (!foundmin) {  /* save the loc of the beginning of the edge */
                foundmin = 1;
                loc = x;
            }
            if (sum >= highthresh) {
#if DEBUG_EDGES
                fprintf(stderr, "Left: x = %d, loc = %d\n", x, loc);
#endif  /* DEBUG_EDGES */
                if (x - loc < maxwidth) {
                    *ploc = loc;
                    return 0;
                }
                else return 1;
            }
        }
    }
    else if (scanflag == L_FROM_RIGHT) {
        for (x = xend; x >= xstart; x--) {
            sum = 0;
            for (y = ystart; y <= yend; y += factor) {
                line = data + y * wpl;
                if (GET_DATA_BIT(line, x))
                    sum++;
            }
            if (!foundmin && sum < lowthresh)
                continue;
            if (!foundmin) {
                foundmin = 1;
                loc = x;
            }
            if (sum >= highthresh) {
#if DEBUG_EDGES
                fprintf(stderr, "Right: x = %d, loc = %d\n", x, loc);
#endif  /* DEBUG_EDGES */
                if (loc - x < maxwidth) {
                    *ploc = loc;
                    return 0;
                }
                else return 1;
            }
        }
    }
    else if (scanflag == L_FROM_TOP) {
        for (y = ystart; y <= yend; y++) {
            sum = 0;
            line = data + y * wpl;
            for (x = xstart; x <= xend; x += factor) {
                if (GET_DATA_BIT(line, x))
                    sum++;
            }
            if (!foundmin && sum < lowthresh)
                continue;
            if (!foundmin) {
                foundmin = 1;
                loc = y;
            }
            if (sum >= highthresh) {
#if DEBUG_EDGES
                fprintf(stderr, "Top: y = %d, loc = %d\n", y, loc);
#endif  /* DEBUG_EDGES */
                if (y - loc < maxwidth) {
                    *ploc = loc;
                    return 0;
                }
                else return 1;
            }
        }
    }
    else if (scanflag == L_FROM_BOTTOM) {
        for (y = yend; y >= ystart; y--) {
            sum = 0;
            line = data + y * wpl;
            for (x = xstart; x <= xend; x += factor) {
                if (GET_DATA_BIT(line, x))
                    sum++;
            }
            if (!foundmin && sum < lowthresh)
                continue;
            if (!foundmin) {
                foundmin = 1;
                loc = y;
            }
            if (sum >= highthresh) {
#if DEBUG_EDGES
                fprintf(stderr, "Bottom: y = %d, loc = %d\n", y, loc);
#endif  /* DEBUG_EDGES */
                if (loc - y < maxwidth) {
                    *ploc = loc;
                    return 0;
                }
                else return 1;
            }
        }
    }
    else
        return ERROR_INT("invalid scanflag", procName, 1);

    return 1;  /* edge not found */
}

