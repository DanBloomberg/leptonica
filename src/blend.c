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
 *  blend.c
 *
 *      Blending two images that are not colormapped
 *           PIX             *pixBlend()
 *           PIX             *pixBlendMask()
 *           PIX             *pixBlendGray()
 *           PIX             *pixBlendColor()
 *           PIX             *pixBlendColorByChannel()
 *           static l_uint8   blendComponentValues()
 *           PIX             *pixFadeWithGray()   ***
 *
 *      Blending two colormapped images
 *           l_int32          pixBlendCmap()
 *           
 *      Coloring "gray" pixels
 *           l_int32          pixColorGray()
 *
 *      Adjusting one or more colors to a target color
 *           PIX             *pixSnapColor()
 *           PIX             *pixSnapColorCmap()
 *           
 *      *** indicates implicit assumption about RGB component ordering
 *
 *  In blending operations a new pix is produced where typically
 *  a subset of pixels in src1 are changed by the set of pixels
 *  in src2, when src2 is located in a given position relative
 *  to src1.  This is similar to rasterop, except that the
 *  blending operations we allow are more complex, and typically
 *  result in dest pixels that are a linear combination of two
 *  pixels, such as src1 and its inverse.  I find it convenient
 *  to think of src2 as the "blender" (the one that takes the action)
 *  and src1 as the "blendee" (the one that changes).
 *
 *  Blending works best when src1 is 8 or 32 bpp.  We also allow
 *  src1 to be colormapped, but the colormap is removed before blending,
 *  so if src1 is colormapped, we can't allow in-place blending.
 *
 *  Because src2 is typically smaller than src1, we can implement by
 *  clipping src2 to src1 and then transforming some of the dest
 *  pixels that are under the support of src2.  In practice, we
 *  do the clipping in the inner pixel loop.  For grayscale and
 *  color src2, we also allow a simple form of transparency, where
 *  pixels of a particular value in src2 are transparent; for those pixels,
 *  no blending is done.
 *
 *  The blending functions are categorized by the depth of src2,
 *  the blender, and not that of src1, the blendee.
 *
 *   - If src2 is 1 bpp, we can do one of three things:
 *     (1) L_BLEND_WITH_INVERSE: Blend a given fraction of src1 with its
 *         inverse color for those pixels in src2 that are fg (ON),
 *         and leave the dest pixels unchanged for pixels in src2 that
 *         are bg (OFF).
 *     (2) L_BLEND_TO_WHITE: Fade the src1 pixels toward white by a
 *         given fraction for those pixels in src2 that are fg (ON),
 *         and leave the dest pixels unchanged for pixels in src2 that
 *         are bg (OFF).
 *     (3) L_BLEND_TO_BLACK: Fade the src1 pixels toward black by a
 *         given fraction for those pixels in src2 that are fg (ON),
 *         and leave the dest pixels unchanged for pixels in src2 that
 *         are bg (OFF).
 *     The blending function is pixBlendMask().
 *
 *   - If src2 is 8 bpp grayscale, we can do one of two things
 *     (but see pixFadeWithGray() below):
 *     (1) L_BLEND_GRAY: If src1 is 8 bpp, mix the two values, using
 *         a fraction of src2 and (1 - fraction) of src1.
 *         If src1 is 32 bpp (rgb), mix the fraction of src2 with
 *         each of the color components in src1.
 *     (2) L_BLEND_GRAY_WITH_INVERSE: Use the grayscale value in src2
 *         to determine how much of the inverse of a src1 pixel is
 *         to be combined with the pixel value.  The input fraction
 *         further acts to scale the change in the src1 pixel.
 *     The blending function is pixBlendGray().
 *
 *   - If src2 is color, we blend a given fraction of src2 with
 *     src1.  If src1 is 8 bpp, the resulting image is 32 bpp.
 *     The blending function is pixBlendColor().
 *
 *   - For all three blending functions -- pixBlendMask(), pixBlendGray()
 *     and pixBlendColor() -- you can apply the blender to the blendee
 *     either in-place or generating a new pix.  For the in-place
 *     operation, this requires that the depth of the resulting pix
 *     must equal that of the input pixs1.
 *
 *   - We remove colormaps from src1 and src2 before blending.
 *     Any quantization would have to be done after blending.
 *
 *  We include another function, pixFadeWithGray(), that blends
 *  a gray or color src1 with a gray src2.  It does one of these things:
 *     (1) L_BLEND_TO_WHITE: Fade the src1 pixels toward white by
 *         a number times the value in src2.
 *     (2) L_BLEND_TO_BLACK: Fade the src1 pixels toward black by
 *         a number times the value in src2.
 */


#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static l_uint8 blendComponentValues(l_uint8 a, l_uint8 b, l_float32 f);



/*-------------------------------------------------------------*
 *                         Pixel blending                      *
 *-------------------------------------------------------------*/
/*!
 *  pixBlend()
 *
 *      Input:  pixs1 (blendee)
 *              pixs2 (blender; typ. smaller)
 *              x,y  (origin (UL corner) of pixs2 relative to
 *                    the origin of pixs1; can be < 0)
 *              fract (blending fraction)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: simple top-level interface.  For more flexibility, call
 *        directly into pixBlendMask(), etc.
 */
PIX *
pixBlend(PIX       *pixs1,
         PIX       *pixs2,
         l_int32    x,
         l_int32    y,
         l_float32  fract)
{
l_int32    w1, h1, d1, d2;
BOX       *box;
PIX       *pixc, *pixt, *pixd;

    PROCNAME("pixBlend");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, NULL);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, NULL);

        /* check relative depths */
    d1 = pixGetDepth(pixs1);
    d2 = pixGetDepth(pixs2);
    if (d1 == 1 && d2 > 1)
        return (PIX *)ERROR_PTR("mixing gray or color with 1 bpp",
                                procName, NULL);

        /* remove colormap from pixs2 if necessary */
    pixt = pixRemoveColormap(pixs2, REMOVE_CMAP_BASED_ON_SRC);
    d2 = pixGetDepth(pixt);

        /* Check if pixs2 is clipped by its position with respect
         * to pixs1; if so, clip it and redefine x and y if necessary.
         * This actually isn't necessary, as the specific blending
         * functions do the clipping directly in the pixel loop
         * over pixs2, but it's included here to show how it can
         * easily be done on pixs2 first. */
    w1 = pixGetWidth(pixs1);
    h1 = pixGetHeight(pixs1);
    box = boxCreate(-x, -y, w1, h1);  /* box of pixs1 relative to pixs2 */
    pixc = pixClipRectangle(pixt, box, NULL);
    boxDestroy(&box);
    if (!pixc) {
        L_WARNING("box doesn't overlap pix", procName);
        return NULL;
    }
    x = L_MAX(0, x);
    y = L_MAX(0, y);

    if (d2 == 1)
        pixd = pixBlendMask(NULL, pixs1, pixc, x, y, fract,
                            L_BLEND_WITH_INVERSE);
    else if (d2 == 8)
        pixd = pixBlendGray(NULL, pixs1, pixc, x, y, fract,
                            L_BLEND_GRAY, 0, 0);
    else  /* d2 == 32 */
        pixd = pixBlendColor(NULL, pixs1, pixc, x, y, fract, 0, 0);

    pixDestroy(&pixc);
    pixDestroy(&pixt);
    return pixd;
}


/*!
 *  pixBlendMask()
 *
 *      Input:  pixd (<optional>; either NULL or equal to pixs1 for in-place)
 *              pixs1 (blendee; depth > 1)
 *              pixs2 (blender; typ. smaller in size than pixs1)
 *              x,y  (origin (UL corner) of pixs2 relative to
 *                    the origin of pixs1; can be < 0)
 *              fract (blending fraction)
 *              type (L_BLEND_WITH_INVERSE, L_BLEND_TO_WHITE, L_BLEND_TO_BLACK)
 *      Return: pixd if OK; pixs1 on error
 *
 *  Notes:
 *      (1) pixs2 must be 1 bpp
 *      (2) Clipping of pixs2 to pixs1 is done in the inner pixel loop.
 *      (3) If pixs1 has a colormap, it is removed.
 *      (4) For inplace operation, call it this way:
 *            pixBlendMask(pixs1, pixs1, pixs2, ...)
 *      (5) For generating a new pixd:
 *            pixd = pixBlendMask(NULL, pixs1, pixs2, ...)
 *      (6) Only call in-place if pixs1 does not have a colormap.
 */
PIX *
pixBlendMask(PIX       *pixd,
             PIX       *pixs1,
             PIX       *pixs2,
             l_int32    x,
             l_int32    y,
             l_float32  fract,
             l_int32    type)
{
l_int32    i, j, d, wc, hc, w, h, wplc;
l_int32    val, rval, gval, bval;
l_uint32   pixval;
l_uint32  *linec, *datac;
PIX       *pixc, *pixt1, *pixt2;

    PROCNAME("pixBlendMask");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixs1);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixs1);
    if (pixGetDepth(pixs1) == 1)
        return (PIX *)ERROR_PTR("pixs1 is 1 bpp", procName, pixs1);
    if (pixGetDepth(pixs2) != 1)
        return (PIX *)ERROR_PTR("pixs2 not 1 bpp", procName, pixs1);
    if (pixd == pixs1 && pixGetColormap(pixs1))
        return (PIX *)ERROR_PTR("inplace; pixs1 has colormap", procName, pixs1);
    if (pixd && (pixd != pixs1))
        return (PIX *)ERROR_PTR("pixd must be NULL or pixs1", procName, pixs1);
    if (fract < 0.0 || fract > 1.0) {
        L_WARNING("fract must be in [0.0, 1.0]; setting to 0.5", procName);
        fract = 0.5;
    }
    if (type != L_BLEND_WITH_INVERSE && type != L_BLEND_TO_WHITE &&
        type != L_BLEND_TO_BLACK) {
        L_WARNING("invalid blend type; setting to L_BLEND_WITH_INVERSE",
                  procName);
        type = L_BLEND_WITH_INVERSE;
    }


        /* If pixd != NULL, we know that it is equal to pixs1 and
         * that pixs1 does not have a colormap, so that an in-place operation
         * can be done.  Otherwise, remove colormap from pixs1 if
         * it exists and unpack to at least 8 bpp if necessary,
         * to do the blending on a new pix. */
    if (!pixd) {
        pixt1 = pixRemoveColormap(pixs1, REMOVE_CMAP_BASED_ON_SRC);
        if (pixGetDepth(pixt1) < 8)
            pixt2 = pixConvertTo8(pixt1, FALSE);
        else
            pixt2 = pixClone(pixt1);
        pixd = pixCopy(NULL, pixt2);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    d = pixGetDepth(pixd);  /* must be either 8 or 32 bpp */
    pixc = pixClone(pixs2);
    wc = pixGetWidth(pixc);
    hc = pixGetHeight(pixc);
    datac = pixGetData(pixc);
    wplc = pixGetWpl(pixc);

        /* check limits for src1, in case clipping was not done */
    switch (type)
    {
    case L_BLEND_WITH_INVERSE:
        for (i = 0; i < hc; i++) {
            if (i + y < 0  || i + y >= h) continue; 
            linec = datac + i * wplc;
            for (j = 0; j < wc; j++) {
                if (j + x < 0  || j + x >= w) continue; 
                bval = GET_DATA_BIT(linec, j);
                if (bval) {
                    switch (d)
                    {
                    case 8:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = (l_uint8)((1. - fract) * pixval +
                                             fract * (255 - pixval));
                        pixSetPixel(pixd, x + j, y + i, val);
                        break;
                    case 32:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = GET_DATA_BYTE(&pixval, COLOR_RED);
                        rval = (l_uint8)((1. - fract) * val
                                          + fract * (255 - val));
                        val = GET_DATA_BYTE(&pixval, COLOR_GREEN);
                        gval = (l_uint8)((1. - fract) * val
                                          + fract * (255 - val));
                        val = GET_DATA_BYTE(&pixval, COLOR_BLUE);
                        bval = (l_uint8)((1. - fract) * val
                                          + fract * (255 - val));
                        composeRGBPixel(rval, gval, bval, &pixval);
                        pixSetPixel(pixd, x + j, y + i, pixval);
                        break;
                    default:
                        L_WARNING("d neither 8 nor 32 bpp; no blend", procName);
                    }
                }
            }
        }
        break;
    case L_BLEND_TO_WHITE:
        for (i = 0; i < hc; i++) {
            if (i + y < 0  || i + y >= h) continue; 
            linec = datac + i * wplc;
            for (j = 0; j < wc; j++) {
                if (j + x < 0  || j + x >= w) continue; 
                bval = GET_DATA_BIT(linec, j);
                if (bval) {
                    switch (d)
                    {
                    case 8:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = (l_uint8)(pixval + fract * (255 - pixval));
                        pixSetPixel(pixd, x + j, y + i, val);
                        break;
                    case 32:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = GET_DATA_BYTE(&pixval, COLOR_RED);
                        rval = (l_uint8)(val + fract * (255 - val));
                        val = GET_DATA_BYTE(&pixval, COLOR_GREEN);
                        gval = (l_uint8)(val + fract * (255 - val));
                        val = GET_DATA_BYTE(&pixval, COLOR_BLUE);
                        bval = (l_uint8)(val + fract * (255 - val));
                        composeRGBPixel(rval, gval, bval, &pixval);
                        pixSetPixel(pixd, x + j, y + i, pixval);
                        break;
                    default:
                        L_WARNING("d neither 8 nor 32 bpp; no blend", procName);
                    }
                }
            }
        }
        break;
    case L_BLEND_TO_BLACK:
        for (i = 0; i < hc; i++) {
            if (i + y < 0  || i + y >= h) continue; 
            linec = datac + i * wplc;
            for (j = 0; j < wc; j++) {
                if (j + x < 0  || j + x >= w) continue; 
                bval = GET_DATA_BIT(linec, j);
                if (bval) {
                    switch (d)
                    {
                    case 8:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = (l_uint8)((1. - fract) * pixval);
                        pixSetPixel(pixd, x + j, y + i, val);
                        break;
                    case 32:
                        pixGetPixel(pixd, x + j, y + i, &pixval);
                        val = GET_DATA_BYTE(&pixval, COLOR_RED);
                        rval = (l_uint8)((1. - fract) * val);
                        val = GET_DATA_BYTE(&pixval, COLOR_GREEN);
                        gval = (l_uint8)((1. - fract) * val);
                        val = GET_DATA_BYTE(&pixval, COLOR_BLUE);
                        bval = (l_uint8)((1. - fract) * val);
                        composeRGBPixel(rval, gval, bval, &pixval);
                        pixSetPixel(pixd, x + j, y + i, pixval);
                        break;
                    default:
                        L_WARNING("d neither 8 nor 32 bpp; no blend", procName);
                    }
                }
            }
        }
        break;
    default:
        L_WARNING("invalid binary mask blend type", procName);
        break;
    }
    
    pixDestroy(&pixc);
    return pixd;
}


/*!
 *  pixBlendGray()
 *
 *      Input:  pixd (<optional>; either NULL or equal to pixs1 for in-place)
 *              pixs1 (blendee; depth > 1)
 *              pixs2 (blender, 8 bpp; typ. smaller in size than pixs1)
 *              x,y  (origin (UL corner) of pixs2 relative to
 *                    the origin of pixs1; can be < 0)
 *              fract (blending fraction)
 *              type (L_BLEND_GRAY, L_BLEND_GRAY_WITH_INVERSE)
 *              transparent (1 to use transparency; 0 otherwise)
 *              transpix (pixel grayval in pixs2 that is to be transparent)
 *      Return: pixd if OK; pixs1 on error
 *
 *  Notes:
 *      (1) pixs2 must be 8 bpp, and have no colormap.
 *      (2) Clipping of pixs2 to pixs1 is done in the inner pixel loop.
 *      (3) If pixs1 has a colormap, it is removed.
 *      (4) If pixs1 has depth < 8, it is unpacked to generate a 8 bpp pix.
 *      (5) For inplace operation, call it this way:
 *            pixBlendGray(pixs1, pixs1, pixs2, ...)
 *      (6) For generating a new pixd:
 *            pixd = pixBlendGray(NULL, pixs1, pixs2, ...)
 *      (7) Only call in-place if pixs1 does not have a colormap;
 *          otherwise it is an error.
 *      (8) If transparent = 0, the blending fraction (fract) is
 *          applied equally to all pixels.
 *      (9) If transparent = 1, all pixels of value transpix (typically
 *          either 0 or 0xff) in pixs2 are transparent in the blend.
 *      (10) After processing pixs1, it is either 8 bpp or 32 bpp:
 *          - if 8 bpp, the fraction of pixs2 is mixed with pixs1.
 *          - if 32 bpp, each component of pixs1 is mixed with
 *            the same fraction of pixs2.
 *      (11) For L_BLEND_GRAY_WITH_INVERSE, the white values of the blendee
 *           (cval == 255 in the code below) result in a delta of 0.
 *           Thus, these pixels are intrinsically transparent!
 */
PIX *
pixBlendGray(PIX       *pixd,
             PIX       *pixs1,
             PIX       *pixs2,
             l_int32    x,
             l_int32    y,
             l_float32  fract,
             l_int32    type,
             l_int32    transparent,
             l_uint32   transpix)
{
l_uint8    val8, cval, dval, rval, gval, bval;
l_int32    i, j, d, wc, hc, w, h, wplc, wpld, delta;
l_int32    ival, irval, igval, ibval;
l_uint32   val32;
l_uint32  *linec, *lined, *datac, *datad;
PIX       *pixc, *pixt1, *pixt2;

    PROCNAME("pixBlendGray");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixs1);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixs1);
    if (pixGetDepth(pixs1) == 1)
        return (PIX *)ERROR_PTR("pixs1 is 1 bpp", procName, pixs1);
    if (pixGetDepth(pixs2) != 8)
        return (PIX *)ERROR_PTR("pixs2 not 8 bpp", procName, pixs1);
    if (pixGetColormap(pixs2))
        return (PIX *)ERROR_PTR("pixs2 has a colormap", procName, pixs1);
    if (pixd == pixs1 && pixGetColormap(pixs1))
        return (PIX *)ERROR_PTR("can't do in-place with cmap", procName, pixs1);
    if (pixd && (pixd != pixs1))
        return (PIX *)ERROR_PTR("pixd must be NULL or pixs1", procName, pixs1);
    if (fract < 0.0 || fract > 1.0) {
        L_WARNING("fract must be in [0.0, 1.0]; setting to 0.5", procName);
        fract = 0.5;
    }
    if (type != L_BLEND_GRAY && type != L_BLEND_GRAY_WITH_INVERSE) {
        L_WARNING("invalid blend type; setting to L_BLEND_GRAY", procName);
        type = L_BLEND_GRAY;
    }

        /* If pixd != NULL, we know that it is equal to pixs1 and
         * that pixs1 does not have a colormap, so that an in-place operation
         * can be done.  Otherwise, remove colormap from pixs1 if
         * it exists and unpack to at least 8 bpp if necessary,
         * to do the blending on a new pix. */
    if (!pixd) {
        pixt1 = pixRemoveColormap(pixs1, REMOVE_CMAP_BASED_ON_SRC);
        if (pixGetDepth(pixt1) < 8)
            pixt2 = pixConvertTo8(pixt1, FALSE);
        else
            pixt2 = pixClone(pixt1);
        pixd = pixCopy(NULL, pixt2);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    d = pixGetDepth(pixd);  /* 8 or 32 bpp */
    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);
    pixc = pixClone(pixs2);
    wc = pixGetWidth(pixc);
    hc = pixGetHeight(pixc);
    datac = pixGetData(pixc);
    wplc = pixGetWpl(pixc);

        /* check limits for src1, in case clipping was not done */
    if (type == L_BLEND_GRAY) {
        for (i = 0; i < hc; i++) {
            if (i + y < 0  || i + y >= h) continue; 
            linec = datac + i * wplc;
            lined = datad + (i + y) * wpld;
            switch (d)
            {
            case 8:
                for (j = 0; j < wc; j++) {
                    if (j + x < 0  || j + x >= w) continue; 
                    cval = GET_DATA_BYTE(linec, j);
                    if (transparent == 0 ||
                        (transparent != 0 && cval != transpix)) {
                        dval = GET_DATA_BYTE(lined, j + x);
                        val8 = (l_uint8)((1. - fract) * dval + fract * cval);
                        SET_DATA_BYTE(lined, j + x, val8);
                    }
                }
                break;
            case 32:
                for (j = 0; j < wc; j++) {
                    if (j + x < 0  || j + x >= w) continue; 
                    cval = GET_DATA_BYTE(linec, j);
                    if (transparent == 0 ||
                        (transparent != 0 && cval != transpix)) {
                        val32 = *(lined + j + x);
                        rval = GET_DATA_BYTE(&val32, COLOR_RED);
                        rval = (l_uint8)((1. - fract) * rval + fract * cval);
                        gval = GET_DATA_BYTE(&val32, COLOR_GREEN);
                        gval = (l_uint8)((1. - fract) * gval + fract * cval);
                        bval = GET_DATA_BYTE(&val32, COLOR_BLUE);
                        bval = (l_uint8)((1. - fract) * bval + fract * cval);
                        composeRGBPixel(rval, gval, bval, &val32);
                        *(lined + j + x) = val32;
                    }
                }
                break;
            default:
                break;   /* shouldn't happen */
            }
        }
    }
    else {  /* L_BLEND_GRAY_WITH_INVERSE */
        for (i = 0; i < hc; i++) {
            if (i + y < 0  || i + y >= h) continue; 
            linec = datac + i * wplc;
            lined = datad + (i + y) * wpld;
            switch (d)
            {
            case 8:
                for (j = 0; j < wc; j++) {
                    if (j + x < 0  || j + x >= w) continue; 
                    cval = GET_DATA_BYTE(linec, j);
                    if (transparent == 0 ||
                        (transparent != 0 && cval != transpix)) {
                        ival = GET_DATA_BYTE(lined, j + x);
                        delta = (cval * ival + (255 - cval) * (255 - ival))
                                / 256 - ival;
                        ival = ival + (l_int32)(fract * delta);
                        SET_DATA_BYTE(lined, j + x, ival);
                    }
                }
                break;
            case 32:
                for (j = 0; j < wc; j++) {
                    if (j + x < 0  || j + x >= w) continue; 
                    cval = GET_DATA_BYTE(linec, j);
                    if (transparent == 0 ||
                        (transparent != 0 && cval != transpix)) {
                        val32 = *(lined + j + x);
                        irval = GET_DATA_BYTE(&val32, COLOR_RED);
                        delta = (cval * irval + (255 - cval) * (255 - irval))
                                / 256 - irval;
                        irval = irval + (l_int32)(fract * delta);
                        igval = GET_DATA_BYTE(&val32, COLOR_GREEN);
                        delta = (cval * igval + (255 - cval) * (255 - igval))
                                / 256 - igval;
                        igval = igval + (l_int32)(fract * delta);
                        ibval = GET_DATA_BYTE(&val32, COLOR_BLUE);
                        delta = (cval * ibval + (255 - cval) * (255 - ibval))
                                / 256 - ibval;
                        ibval = ibval + (l_int32)(fract * delta);
                        composeRGBPixel(irval, igval, ibval, &val32);
                        *(lined + j + x) = val32;
                    }
                }
                break;
            default:
                break;   /* shouldn't happen */
            }
        }
    }

    pixDestroy(&pixc);
    return pixd;
}


/*!
 *  pixBlendColor()
 *
 *      Input:  pixd (<optional>; either NULL or equal to pixs1 for in-place)
 *              pixs1 (blendee; depth > 1)
 *              pixs2 (blender, 32 bpp; typ. smaller in size than pixs1)
 *              x,y  (origin (UL corner) of pixs2 relative to
 *                    the origin of pixs1)
 *              fract (blending fraction)
 *              transparent (1 to use transparency; 0 otherwise)
 *              transpix (pixel color in pixs2 that is to be transparent)
 *      Return: pixd if OK; pixs1 on error
 *
 *  Notes:
 *      (1) pixs2 must be 32 bpp, and have no colormap.
 *      (2) Clipping of pixs2 to pixs1 is done in the inner pixel loop.
 *      (3) If pixs1 has a colormap, it is removed to generate a 32 bpp pix.
 *      (4) If pixs1 has depth < 32, it is unpacked to generate a 32 bpp pix.
 *      (5) For inplace operation, call it this way:
 *            pixBlendColor(pixs1, pixs1, pixs2, ...)
 *      (6) For generating a new pixd:
 *            pixd = pixBlendColor(NULL, pixs1, pixs2, ...)
 *      (7) Only call in-place if pixs1 is 32 bpp; otherwise it is an error.
 *      (8) If transparent = 0, the blending fraction (fract) is
 *          applied equally to all pixels.
 *      (9) If transparent = 1, all pixels of value transpix (typically
 *          either 0 or 0xffffff00) in pixs2 are transparent in the blend.
 */
PIX *
pixBlendColor(PIX       *pixd,
              PIX       *pixs1,
              PIX       *pixs2,
              l_int32    x,
              l_int32    y,
              l_float32  fract,
              l_int32    transparent,
              l_uint32   transpix)
{
l_uint8    rval, gval, bval, rcval, gcval, bcval;
l_int32    i, j, wc, hc, w, h, wplc, wpld;
l_uint32   cval32, val32;
l_uint32  *linec, *lined, *datac, *datad;
PIX       *pixc, *pixt1, *pixt2;

    PROCNAME("pixBlendColor");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixs1);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixs1);
    if (pixGetDepth(pixs1) == 1)
        return (PIX *)ERROR_PTR("pixs1 is 1 bpp", procName, pixs1);
    if (pixGetDepth(pixs2) != 32)
        return (PIX *)ERROR_PTR("pixs2 not 32 bpp", procName, pixs1);
    if (pixd == pixs1 && pixGetDepth(pixs1) != 32)
        return (PIX *)ERROR_PTR("inplace; pixs1 not 32 bpp", procName, pixs1);
    if (pixd && (pixd != pixs1))
        return (PIX *)ERROR_PTR("pixd must be NULL or pixs1", procName, pixs1);
    if (fract < 0.0 || fract > 1.0) {
        L_WARNING("fract must be in [0.0, 1.0]; setting to 0.5", procName);
        fract = 0.5;
    }

        /* If pixd != NULL, we know that it is equal to pixs1 and
         * that pixs1 is 32 bpp rgb, so that an in-place operation
         * can be done.  Otherwise, remove colormap from pixs1 if
         * it exists and unpack to 32 bpp if necessary, to do the
         * blending on a new 32 bpp Pix. */
    if (!pixd) {
        pixt1 = pixRemoveColormap(pixs1, REMOVE_CMAP_TO_FULL_COLOR);
        if (pixGetDepth(pixt1) < 32)
            pixt2 = pixConvertTo32(pixt1);
        else
            pixt2 = pixClone(pixt1);
        pixd = pixCopy(NULL, pixt2);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);
    pixc = pixClone(pixs2);
    wc = pixGetWidth(pixc);
    hc = pixGetHeight(pixc);
    datac = pixGetData(pixc);
    wplc = pixGetWpl(pixc);

        /* check limits for src1, in case clipping was not done */
    for (i = 0; i < hc; i++) {
        if (i + y < 0  || i + y >= h) continue; 
        linec = datac + i * wplc;
        lined = datad + (i + y) * wpld;
        for (j = 0; j < wc; j++) {
            if (j + x < 0  || j + x >= w) continue; 
            cval32 = *(linec + j);
            if (transparent == 0 ||
                (transparent != 0 &&
                     ((cval32 & 0xffffff00) != (transpix & 0xffffff00)))) { 
                val32 = *(lined + j + x);
                rcval = GET_DATA_BYTE(&cval32, COLOR_RED);
                rval = GET_DATA_BYTE(&val32, COLOR_RED);
                rval = (l_uint8)((1. - fract) * rval + fract * rcval);
                gcval = GET_DATA_BYTE(&cval32, COLOR_GREEN);
                gval = GET_DATA_BYTE(&val32, COLOR_GREEN);
                gval = (l_uint8)((1. - fract) * gval + fract * gcval);
                bcval = GET_DATA_BYTE(&cval32, COLOR_BLUE);
                bval = GET_DATA_BYTE(&val32, COLOR_BLUE);
                bval = (l_uint8)((1. - fract) * bval + fract * bcval);
                composeRGBPixel(rval, gval, bval, &val32);
                *(lined + j + x) = val32;
            }
        }
    }

    pixDestroy(&pixc);
    return pixd;
}


/* 
 *  pixBlendColorByChannel()
 *
 *  This is an extended version of pixBlendColor.  All parameters have the
 *  same meaning except it takes one mixing fraction per channel, and the
 *  mixing fraction may be < 0 or > 1, in which case, the min or max of two
 *  images are taken.  More specifically,
 *  
 *   a = pixs1[i], b = pixs2[i]
 *   frac < 0.0 -> min(a, b)
 *   frac > 1.0 -> max(a, b)
 *   else -> (1-frac)*a + frac*b
 *   frac = 0 -> a
 *   frac = 1 -> b
 *
 * Notes:
 *   (1) See usage notes in pixBlendColor()
 *   (2) pixBlendColor() would be equivalent to 
 *         pixBlendColorChannel(..., fract, fract, fract, ...);
 *       at a small cost of efficiency.
 */
PIX *
pixBlendColorByChannel(PIX       *pixd,
                       PIX       *pixs1,
                           PIX       *pixs2,
                       l_int32    x,
                       l_int32    y,
                       l_float32  rfract,
                       l_float32  gfract,
                       l_float32  bfract,
                       l_int32    transparent,
                       l_uint32   transpix)
{
l_uint8    rval, gval, bval, rcval, gcval, bcval;
l_int32    i, j, wc, hc, w, h, wplc, wpld;
l_uint32   cval32, val32;
l_uint32  *linec, *lined, *datac, *datad;
PIX       *pixc, *pixt1, *pixt2;

    PROCNAME("pixBlendColorByChannel");

    if (!pixs1)
        return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixs1);
    if (!pixs2)
        return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixs1);
    if (pixGetDepth(pixs1) == 1)
        return (PIX *)ERROR_PTR("pixs1 is 1 bpp", procName, pixs1);
    if (pixGetDepth(pixs2) != 32)
        return (PIX *)ERROR_PTR("pixs2 not 32 bpp", procName, pixs1);
    if (pixd == pixs1 && pixGetDepth(pixs1) != 32)
        return (PIX *)ERROR_PTR("inplace; pixs1 not 32 bpp", procName, pixs1);
    if (pixd && (pixd != pixs1))
        return (PIX *)ERROR_PTR("pixd must be NULL or pixs1", procName, pixs1);

        /* If pixd != NULL, we know that it is equal to pixs1 and
         * that pixs1 is 32 bpp rgb, so that an in-place operation
         * can be done.  Otherwise, remove colormap from pixs1 if
         * it exists and unpack to 32 bpp if necessary, to do the
         * blending on a new 32 bpp Pix. */
    if (!pixd) {
        pixt1 = pixRemoveColormap(pixs1, REMOVE_CMAP_TO_FULL_COLOR);
        if (pixGetDepth(pixt1) < 32)
            pixt2 = pixConvertTo32(pixt1);
        else
            pixt2 = pixClone(pixt1);
        pixd = pixCopy(NULL, pixt2);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);
    pixc = pixClone(pixs2);
    wc = pixGetWidth(pixc);
    hc = pixGetHeight(pixc);
    datac = pixGetData(pixc);
    wplc = pixGetWpl(pixc);

        /* check limits for src1, in case clipping was not done */
    for (i = 0; i < hc; i++) {
        if (i + y < 0  || i + y >= h) continue; 
        linec = datac + i * wplc;
        lined = datad + (i + y) * wpld;
        for (j = 0; j < wc; j++) {
            if (j + x < 0  || j + x >= w) continue; 
            cval32 = *(linec + j);
            if (transparent == 0 ||
                (transparent != 0 &&
                     ((cval32 & 0xffffff00) != (transpix & 0xffffff00)))) { 
                val32 = *(lined + j + x);
                rcval = GET_DATA_BYTE(&cval32, COLOR_RED);
                rval = GET_DATA_BYTE(&val32, COLOR_RED);
                rval = blendComponentValues(rval, rcval, rfract);
                gcval = GET_DATA_BYTE(&cval32, COLOR_GREEN);
                gval = GET_DATA_BYTE(&val32, COLOR_GREEN);
                gval = blendComponentValues(gval, gcval, gfract);
                bcval = GET_DATA_BYTE(&cval32, COLOR_BLUE);
                bval = GET_DATA_BYTE(&val32, COLOR_BLUE);
                bval = blendComponentValues(bval, bcval, bfract);
                composeRGBPixel(rval, gval, bval, &val32);
                *(lined + j + x) = val32;
            }
        }
    }

    pixDestroy(&pixc);
    return pixd;
}


static l_uint8
blendComponentValues(l_uint8 a,
                     l_uint8 b,
                     l_float32 f)
{
    if (f < 0.)
        return (l_uint8)((a < b) ? a : b);
    if (f > 1.)
        return (l_uint8)((a > b) ? a : b);
    return (l_uint8)((1. - f) * a + f * b);
}


/*!
 *  pixFadeWithGray()
 *
 *      Input:  pixs (colormapped or 8 bpp or 32 bpp)
 *              pixb (8 bpp blender)
 *              factor (multiplicative factor to apply to blender value)
 *              type (L_BLEND_TO_WHITE, L_BLEND_TO_BLACK)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) This function combines two pix aligned to the UL corner; they
 *          need not be the same size.
 *      (2) Each pixel in pixb is multiplied by 'factor' divided by 255, and
 *          clipped to the range [0 ... 1].  This gives the fade fraction
 *          to be appied to pixs.  Fade either to white (L_BLEND_TO_WHITE)
 *          or to black (L_BLEND_TO_BLACK).
 *      (3) Implicit assumption about RGB component ordering.
 */
PIX *
pixFadeWithGray(PIX       *pixs,
                PIX       *pixb,
                l_float32  factor,
                l_int32    type)
{
l_int32    i, j, w, h, d, wb, hb, db, wd, hd, wplb, wpld;
l_int32    valb, vald, nvald, rval, gval, bval, nrval, ngval, nbval;
l_float32  nfactor, fract;
l_uint32   val32, nval32;
l_uint32  *lined, *datad, *lineb, *datab;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixFadeWithGray");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!pixb)
        return (PIX *)ERROR_PTR("pixb not defined", procName, NULL);
    cmap = pixGetColormap(pixs);
    d = pixGetDepth(pixs);
    if (d < 8 && !cmap)
        return (PIX *)ERROR_PTR("pixs not cmapped and < 8bpp", procName, NULL);
    pixGetDimensions(pixb, &wb, &hb, &db);
    if (db != 8)
        return (PIX *)ERROR_PTR("pixb not 8bpp", procName, NULL);
    if (type != L_BLEND_TO_WHITE && type != L_BLEND_TO_BLACK)
        return (PIX *)ERROR_PTR("invalid fade type", procName, NULL);

    if (cmap)
        pixd = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    else
        pixd = pixCopy(NULL, pixs);
    pixGetDimensions(pixd, &wd, &hd, &d);
    w = L_MIN(wb, wd);
    h = L_MIN(hb, hd);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    datab = pixGetData(pixb);
    wplb = pixGetWpl(pixb);

    nfactor = factor / 255.;
    for (i = 0; i < h; i++) {
        lineb = datab + i * wplb;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            valb = GET_DATA_BYTE(lineb, j);
            fract = nfactor * (l_float32)valb;
            fract = L_MIN(fract, 1.0);
            if (d == 8) {
                vald = GET_DATA_BYTE(lined, j);
                if (type == L_BLEND_TO_WHITE)
                    nvald = vald + (l_int32)(fract * (255. - (l_float32)vald));
                else  /* L_BLEND_TO_BLACK */
                    nvald = vald - (l_int32)(fract * (l_float32)vald);
                SET_DATA_BYTE(lined, j, nvald);
            }
            else {  /* d == 32 */
                val32 = lined[j];
                rval = val32 >> 24;
                gval = (val32 >> 16) & 0xff;
                bval = (val32 >> 8) & 0xff;
                if (type == L_BLEND_TO_WHITE) {
                    nrval = rval + (l_int32)(fract * (255. - (l_float32)rval));
                    ngval = gval + (l_int32)(fract * (255. - (l_float32)gval));
                    nbval = bval + (l_int32)(fract * (255. - (l_float32)bval));
                }
                else {
                    nrval = rval - (l_int32)(fract * (l_float32)rval);
                    ngval = gval - (l_int32)(fract * (l_float32)gval);
                    nbval = bval - (l_int32)(fract * (l_float32)bval);
                }
                nval32 = (nrval << 24) | (ngval << 16) | (nbval << 8);
                lined[j] = nval32;
            }
        }
    }
    
    return pixd;
}


/*-------------------------------------------------------------*
 *               Blending two colormapped images               *
 *-------------------------------------------------------------*/
/*!
 *  pixBlendCmap()
 *
 *      Input:  pixs (2, 4 or 8 bpp, with colormap)
 *              pixb (colormapped blender)
 *              x, y (UL corner of blender relative to pixs)
 *              sindex (colormap index of pixels in pixs to be changed)
 *      Return: 0 if OK, 1 on error
 *
 *  Note:
 *      (1) This function combines two colormaps, and replaces the pixels
 *          in pixs that have a specified color value with those in pixb.
 *      (2) sindex must be in the existing colormap; otherwise an 
 *          error is returned.  In use, sindex will typically be the index
 *          for white (255, 255, 255).
 *      (3) Blender colors that already exist in the colormap are used;
 *          others are added.  If any blender colors cannot be
 *          stored in the colormap, an error is returned.
 *      (4) In the implementation, a mapping is generated from each
 *          original blender colormap index to the corresponding index
 *          in the expanded colormap for pixs.  Then for each pixel in
 *          pixs with value sindex, and which is covered by a blender pixel,
 *          the new index corresponding to the blender pixel is substituted
 *          for sindex.
 */
l_int32
pixBlendCmap(PIX     *pixs,
             PIX     *pixb,
             l_int32  x,
             l_int32  y,
             l_int32  sindex)
{
l_int32    rval, gval, bval;
l_int32    i, j, w, h, d, ncb, wb, hb, wpls;
l_int32    index, val, nadded;
l_int32    lut[256];
l_uint32   pval;
l_uint32  *lines, *datas;
PIXCMAP   *cmaps, *cmapb, *cmapsc;

    PROCNAME("pixBlendCmap");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pixb)
        return ERROR_INT("pixb not defined", procName, 1);
    if ((cmaps = pixGetColormap(pixs)) == NULL)
        return ERROR_INT("no colormap in pixs", procName, 1);
    if ((cmapb = pixGetColormap(pixb)) == NULL)
        return ERROR_INT("no colormap in pixb", procName, 1);
    ncb = pixcmapGetCount(cmapb);

        /* Make a copy of cmaps; we'll add to this if necessary
         * and substitute at the end if we found there was enough room
         * to hold all the new colors. */
    cmapsc = pixcmapCopy(cmaps);

    d = pixGetDepth(pixs);
    if (d != 2 && d != 4 && d != 8)
        return ERROR_INT("depth not in {2,4,8}", procName, 1);

        /* Add new colors if necessary; get mapping array between
         * cmaps and cmapb. */
    for (i = 0, nadded = 0; i < ncb; i++) {
        pixcmapGetColor(cmapb, i, &rval, &gval, &bval);
        if (pixcmapGetIndex(cmapsc, rval, gval, bval, &index)) { /* not found */
            if (pixcmapAddColor(cmapsc, rval, gval, bval)) {
                pixcmapDestroy(&cmapsc);
                return ERROR_INT("not enough room in cmaps", procName, 1);
            }
            lut[i] = pixcmapGetCount(cmapsc) - 1;
            nadded++;
        }
        else
            lut[i] = index;
    }

        /* Replace cmaps if colors have been added. */
    if (nadded == 0)
        pixcmapDestroy(&cmapsc);
    else
        pixSetColormap(pixs, cmapsc);

        /* Replace each pixel value sindex by mapped colormap index when 
         * a blender pixel in pixbc overlays it. */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    wb = pixGetWidth(pixb);
    hb = pixGetHeight(pixb);
    for (i = 0; i < hb; i++) {
        if (i + y < 0  || i + y >= h) continue; 
        lines = datas + (y + i) * wpls;
        for (j = 0; j < wb; j++) {
            if (j + x < 0  || j + x >= w) continue; 
            switch (d) {
            case 2:
                val = GET_DATA_DIBIT(lines, x + j);
                if (val == sindex) {
                    pixGetPixel(pixb, j, i, &pval);
                    SET_DATA_DIBIT(lines, x + j, lut[pval]);
                }
                break;
            case 4:
                val = GET_DATA_QBIT(lines, x + j);
                if (val == sindex) {
                    pixGetPixel(pixb, j, i, &pval);
                    SET_DATA_QBIT(lines, x + j, lut[pval]);
                }
                break;
            case 8:
                val = GET_DATA_BYTE(lines, x + j);
                if (val == sindex) {
                    pixGetPixel(pixb, j, i, &pval);
                    SET_DATA_BYTE(lines, x + j, lut[pval]);
                }
                break;
            default:
                return ERROR_INT("depth not in {2,4,8}", procName, 1);
            }
        }
    }

    return 0;
}


/*---------------------------------------------------------------------*
 *                        Coloring "gray" pixels                       *
 *---------------------------------------------------------------------*/
/*!
 *  pixColorGray()
 *
 *      Input:  pixs (rgb or colormapped image)
 *              box (<optional> region in which to apply color; can be NULL)
 *              type (L_PAINT_LIGHT, L_PAINT_DARK)
 *              thresh (average value below/above which pixel is unchanged)
 *              rval, gval, bval (new color to paint)
 *      Return: 0 if OK; 1 on error
 *
 *  Note:
 *      (1) This is an in-place operation.
 *      (2) If type == L_PAINT_LIGHT, it colorizes non-black pixels,
 *          preserving antialiasing.
 *          If type == L_PAINT_DARK, it colorizes non-white pixels,
 *          preserving antialiasing.
 *      (3) If box is NULL, applies function to the entire image; otherwise,
 *          clips the operation to the intersection of the box and pix.
 *      (4) If colormapped, calls pixColorGrayCmap(), which applies the
 *          coloring algorithm only to pixels that are strictly gray.
 *      (5) For RGB, determines a "gray" value by averaging; then uses this
 *          value, plus the input rgb target, to generate the output
 *          pixel values.
 *      (6) thresh is only used for rgb; it is ignored for colormapped pix.
 *          If type == L_PAINT_LIGHT, use thresh = 0 if all pixels are to
 *          be colored (black pixels will be unaltered).
 *          In situations where there are a lot of black pixels,
 *          setting thresh > 0 will make the function considerably
 *          more efficient without affecting the final result.
 *          If type == L_PAINT_DARK, use thresh = 255 if all pixels
 *          are to be colored (white pixels will be unaltered).
 *          In situations where there are a lot of white pixels,
 *          setting thresh < 255 will make the function considerably
 *          more efficient without affecting the final result.
 */
l_int32
pixColorGray(PIX     *pixs,
             BOX     *box,
             l_int32  type,
             l_int32  thresh,
             l_int32  rval,
             l_int32  gval,
             l_int32  bval)
{
l_int32     i, j, w, h, d, wpl, x1, x2, y1, y2;
l_int32     nrval, ngval, nbval, aveval;
l_float32   factor;
l_uint32    val32;
l_uint32   *line, *data;
PIXCMAP    *cmap;

    PROCNAME("pixColorGray");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (type != L_PAINT_LIGHT && type != L_PAINT_DARK)
        return ERROR_INT("invalid type", procName, 1);

    cmap = pixGetColormap(pixs);
    d = pixGetDepth(pixs);
    if (!cmap && d != 32)
        return ERROR_INT("pixs not cmapped or rgb", procName, 1);
    if (cmap)
        return pixColorGrayCmap(pixs, box, type, rval, gval, bval);

        /* rgb image; check the thresh */
    if (type == L_PAINT_LIGHT) {  /* thresh should be low */
        if (thresh >= 255)
            return ERROR_INT("thresh must be < 255; else this is a no-op",
                             procName, 1);
        if (thresh > 127)
            L_WARNING("threshold set very high", procName);
    }
    else {  /* type == L_PAINT_DARK; thresh should be high */
        if (thresh <= 0)
            return ERROR_INT("thresh must be > 0; else this is a no-op",
                             procName, 1);
        if (thresh < 128)
            L_WARNING("threshold set very low", procName);
    }

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if (!box) {
        x1 = y1 = 0;
        x2 = w;
        y2 = h;
    }
    else {
        x1 = box->x;
        y1 = box->y;
        x2 = x1 + box->w - 1;
        y2 = y1 + box->h - 1;
    }
    
    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    factor = 1. / 255.;
    for (i = y1; i <= y2; i++) {
        if (i < 0 || i >= h)
            continue;
        line = data + i * wpl;
        for (j = x1; j <= x2; j++) {
            if (j < 0 || j >= w)
                continue;
            val32 = *(line + j);
            aveval = ((val32 >> 24) + ((val32 >> 16) & 0xff) +
                      ((val32 >> 8) & 0xff)) / 3;
            if (type == L_PAINT_LIGHT) {
                if (aveval < thresh)  /* skip sufficiently dark pixels */
                    continue;
                nrval = (l_int32)(rval * aveval * factor);
                ngval = (l_int32)(gval * aveval * factor);
                nbval = (l_int32)(bval * aveval * factor);
            }
            else {  /* type == L_PAINT_DARK */
                if (aveval > thresh)  /* skip sufficiently light pixels */
                    continue;
                nrval = rval + (l_int32)((255. - rval) * aveval * factor);
                ngval = gval + (l_int32)((255. - gval) * aveval * factor);
                nbval = bval + (l_int32)((255. - bval) * aveval * factor);
            }
            composeRGBPixel(nrval, ngval, nbval, &val32);
            *(line + j) = val32;
        }
    }

    return 0;
}



/*------------------------------------------------------------------*
 *            Adjusting one or more colors to a target color        *
 *------------------------------------------------------------------*/
/*!
 *  pixSnapColor()
 *
 *      Input:  pixd (<optional>; either NULL or equal to pixs for in-place)
 *              pixs (colormapped or 8 bpp gray or 32 bpp rgb)
 *              srcval (color center to be selected for change: 0xrrggbb00)
 *              dstval (target color for pixels: 0xrrggbb00)
 *              diff (max absolute difference, applied to all components)
 *      Return: pixd (with all pixels within diff of pixval set to pixval),
 *              or pixs on error
 *
 *  Notes:
 *      (1) For inplace operation, call it this way:
 *           pixSnapColor(pixs, pixs, ... )
 *      (2) For generating a new pixd:
 *           pixd = pixSnapColor(NULL, pixs, ...)
 *      (3) If pixs has a colormap, it is handled by pixSnapColorCmap().
 *      (4) All pixels within 'diff' of 'srcval', componentwise,
 *          will be changed to 'dstval'.
 */
PIX *
pixSnapColor(PIX      *pixd,
             PIX      *pixs,
             l_uint32  srcval,
             l_uint32  dstval,
             l_int32   diff)
{
l_int32    val, sval, dval;
l_int32    rval, gval, bval, rsval, gsval, bsval;
l_int32    i, j, w, h, d, wpl;
l_uint32   pixel;
l_uint32  *line, *data;

    PROCNAME("pixSnapColor");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixs);
    if (pixd && (pixd != pixs))
        return (PIX *)ERROR_PTR("pixd not null or == pixs", procName, pixs);

    if (pixGetColormap(pixs))
        return pixSnapColorCmap(pixd, pixs, srcval, dstval, diff);

        /* pixs does not have a colormap; it must be 8 bpp gray or
         * 32 bpp rgb. */
    if (pixGetDepth(pixs) < 8)
        return (PIX *)ERROR_PTR("pixs is < 8 bpp", procName, pixs);

        /* Do the work on pixd */
    if (!pixd)
        pixd = pixCopy(NULL, pixs);

    d = pixGetDepth(pixd);
    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    data = pixGetData(pixd);
    wpl = pixGetWpl(pixd);
    if (d == 8) {
        sval = srcval & 0xff;
        dval = dstval & 0xff;
        for (i = 0; i < h; i++) {
            line = data + i * wpl;
            for (j = 0; j < w; j++) {
                val = GET_DATA_BYTE(line, j);
                if (L_ABS(val - sval) <= diff)
                    SET_DATA_BYTE(line, j, dval);
            }
        }
    }
    else {  /* d == 32 */
        rsval = GET_DATA_BYTE(&srcval, COLOR_RED);
        gsval = GET_DATA_BYTE(&srcval, COLOR_GREEN);
        bsval = GET_DATA_BYTE(&srcval, COLOR_BLUE);
        for (i = 0; i < h; i++) {
            line = data + i * wpl;
            for (j = 0; j < w; j++) {
                pixel = *(line + j);
                rval = GET_DATA_BYTE(&pixel, COLOR_RED);
                gval = GET_DATA_BYTE(&pixel, COLOR_GREEN);
                bval = GET_DATA_BYTE(&pixel, COLOR_BLUE);
                if ((L_ABS(rval - rsval) <= diff) &&
                    (L_ABS(gval - gsval) <= diff) &&
                    (L_ABS(bval - bsval) <= diff)) 
                    *(line + j) = dstval;  /* replace */
            }
        }
    }

    return pixd;
}


/*!
 *  pixSnapColorCmap()
 *
 *      Input:  pixd (<optional>; either NULL or equal to pixs for in-place)
 *              pixs (colormapped)
 *              srcval (color center to be selected for change: 0xrrggbb00)
 *              dstval (target color for pixels: 0xrrggbb00)
 *              diff (max absolute difference, applied to all components)
 *      Return: pixd (with all pixels within diff of srcval set to dstval),
 *              or pixs on error
 *
 *  Notes:
 *      (1) For inplace operation, call it this way:
 *           pixSnapCcmap(pixs, pixs, ... )
 *      (2) For generating a new pixd:
 *           pixd = pixSnapCmap(NULL, pixs, ...)
 *      (3) pixs must have a colormap.
 *      (4) All colors within 'diff' of 'srcval', componentwise,
 *          will be changed to 'dstval'.
 */
PIX *
pixSnapColorCmap(PIX      *pixd,
                 PIX      *pixs,
                 l_uint32  srcval,
                 l_uint32  dstval,
                 l_int32   diff)
{
l_int32    i, w, h, ncolors, index, found;
l_int32    rval, gval, bval, rsval, gsval, bsval, rdval, gdval, bdval;
PIX       *pixm, *pixt;
PIXCMAP   *cmap;

    PROCNAME("pixSnapColorCmap");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixs);
    if (!pixGetColormap(pixs))
        return (PIX *)ERROR_PTR("cmap not found", procName, pixs);
    if (pixd && (pixd != pixs))
        return (PIX *)ERROR_PTR("pixd not null or == pixs", procName, pixs);

    if (!pixd)
        pixd = pixCopy(NULL, pixs);

        /* If no free colors, look for one close to the target
         * that can be commandeered. */
    cmap = pixGetColormap(pixd);
    ncolors = pixcmapGetCount(cmap);
    rsval = GET_DATA_BYTE(&srcval, COLOR_RED);
    gsval = GET_DATA_BYTE(&srcval, COLOR_GREEN);
    bsval = GET_DATA_BYTE(&srcval, COLOR_BLUE);
    rdval = GET_DATA_BYTE(&dstval, COLOR_RED);
    gdval = GET_DATA_BYTE(&dstval, COLOR_GREEN);
    bdval = GET_DATA_BYTE(&dstval, COLOR_BLUE);
    found = FALSE;
    if (pixcmapGetFreeCount(cmap) == 0) {
        for (i = 0; i < ncolors; i++) {
            pixcmapGetColor(cmap, i, &rval, &gval, &bval);
            if ((L_ABS(rval - rsval) <= diff) &&
                (L_ABS(gval - gsval) <= diff) &&
                (L_ABS(bval - bsval) <= diff)) {
                index = i;
                pixcmapResetColor(cmap, index, rdval, gdval, bdval);
                found = TRUE;
                break;
            }
        }
    }
    else {  /* just add the new color */
        pixcmapAddColor(cmap, rdval, gdval, bdval);
        index = ncolors;  /* index of new destination color */
        found = TRUE;
    }

    if (!found) {
        L_INFO("nothing to do", procName);
        return pixd;
    }

        /* For each color in cmap that is close enough to srcval,
         * add that pixel to a binary mask. */
    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    pixm = pixCreate(w, h, 1);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        if ((L_ABS(rval - rsval) <= diff) &&
            (L_ABS(gval - gsval) <= diff) &&
            (L_ABS(bval - bsval) <= diff)) {
            pixt = pixGenerateMaskByValue(pixd, i);
            pixOr(pixm, pixm, pixt);
            pixDestroy(&pixt);
        }
    }

        /* Use the binary mask to set all selected pixels to
         * the dest color index. */
    pixSetMasked(pixd, pixm, index);
    pixDestroy(&pixm);

        /* Remove all unused colors from the colormap. */
    pixRemoveUnusedColors(pixd);

    return pixd;
}

