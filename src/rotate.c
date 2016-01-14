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
 *  rotate.c
 *
 *     General rotation about image center
 *              PIX     *pixRotate()
 *              PIX     *pixEmbedForRotation()
 *
 *     Rotations are measured in radians; clockwise is positive.
 *
 *     The general rotation pixRotate() does the best job for
 *     rotating about the image center.  For 1 bpp, it uses shear;
 *     for others, it uses either shear or area mapping.
 *     If requested, it expands the output image so that no pixels are lost
 *     in the rotation, and this can be done on multiple successive shears
 *     without expanding beyond the maximum necessary size.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "allheaders.h"

static const l_float32  VERY_SMALL_ANGLE = 0.001;  /* radians; ~0.06 degrees */


/*------------------------------------------------------------------*
 *                  General rotation about the center               *
 *------------------------------------------------------------------*/
/*!
 *  pixRotate()
 *
 *      Input:  pixs (1, 2, 4, 8, 32 bpp rgb)
 *              angle (radians; clockwise is positive)
 *              type (L_ROTATE_AREA_MAP, L_ROTATE_SHEAR)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *              width (original width; use 0 to avoid embedding)
 *              height (original height; use 0 to avoid embedding)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Rotation is about the center of the image.
 *      (2) For very small rotations, just return a clone.
 *      (3) Rotation brings either white or black pixels in
 *          from outside the image.  For colormapped images where
 *          there is no white or black, a new color is added if
 *          possible for these pixels; otherwise, either the
 *          lightest or darkest color is used.
 *      (4) Binary images are always rotated by shear.
 *      (5) Colormaps are removed.
 *      (6) The dest can be expanded so that no image pixels
 *          are lost.  To invoke expansion, input the original
 *          width and height.  For repeated rotation, use of the
 *          original width and height allows the expansion to
 *          stop at the maximum required size, which is a square 
 *          with side = sqrt(w*w + h*h).
 *
 *  *** Warning: implicit assumption about RGB component ordering ***
 */
PIX *
pixRotate(PIX       *pixs,
          l_float32  angle,
          l_int32    type,
          l_int32    incolor,
          l_int32    width,
          l_int32    height)
{
l_int32    d;
l_uint32   fillval;
PIX       *pixt1, *pixt2, *pixt3, *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixRotate");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (type != L_ROTATE_SHEAR && type != L_ROTATE_AREA_MAP)
        return (PIX *)ERROR_PTR("invalid type", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor", procName, NULL);

    if (L_ABS(angle) < VERY_SMALL_ANGLE)
        return pixClone(pixs);

        /* If there is a colormap, remove it. */
    if ((cmap = pixGetColormap(pixs)) != NULL)
        pixt1 = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    else
        pixt1 = pixClone(pixs);

        /* Request to embed in a larger image; do if necessary */
    pixt2 = pixEmbedForRotation(pixt1, angle, incolor, width, height);

        /* Area mapping requires 8 or 32 bpp.
         * If 1 bpp, always use shear. */
    d = pixGetDepth(pixt2);
    if (type == L_ROTATE_AREA_MAP && d > 1 && d < 8)
        pixt3 = pixConvertTo8(pixt2, FALSE);
    else
        pixt3 = pixClone(pixt2);

        /* Rotate by shear or area mapping */
    d = pixGetDepth(pixt3);
    if (d == 1 || type == L_ROTATE_SHEAR)
        pixd = pixRotateShearCenter(pixt3, angle, incolor);
    else {  /* rotate by area mapping */
        fillval = 0;
        if (incolor == L_BRING_IN_WHITE) {
            if (d == 8)
                fillval = 255;
            else  /* d == 32 */
                fillval = 0xffffff00;
        }
        if (d == 8)
            pixd = pixRotateAMGray(pixt3, angle, fillval);
        else   /* d == 32 */
            pixd = pixRotateAMColor(pixt3, angle, fillval);
    }

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    return pixd;
}


/*!
 *  pixEmbedForRotation()
 *
 *      Input:  pixs (1, 2, 4, 8, 32 bpp rgb)
 *              angle (radians; clockwise is positive)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *              width (original width; use 0 to avoid embedding)
 *              height (original height; use 0 to avoid embedding)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) For very small rotations, just return a clone.
 *      (2) Generate larger image to embed pixs if necessary, and
 *          place in the center.
 *      (3) Rotation brings either white or black pixels in
 *          from outside the image.  For colormapped images where
 *          there is no white or black, a new color is added if
 *          possible for these pixels; otherwise, either the
 *          lightest or darkest color is used.  In most cases,
 *          the colormap will be removed prior to rotation.
 *      (4) The dest is to be expanded so that no image pixels
 *          are lost after rotation.  Input of the original width
 *          and height allows the expansion to stop at the maximum
 *          required size, which is a square with side equal to
 *          sqrt(w*w + h*h).
 *      (5) Let theta be atan(w/h).  Then the height after rotation 
 *          cannot increase by a factor more than
 *               cos(theta - |angle|)
 *          whereas the width after rotation cannot increase by a
 *          factor more than 
 *               sin(theta + |angle|)
 *          These must be clipped to the maximal side, and additionally,
 *          we don't allow either the width or height to decrease.
 */
PIX *
pixEmbedForRotation(PIX       *pixs,
                    l_float32  angle,
                    l_int32    incolor,
                    l_int32    width,
                    l_int32    height)
{
l_int32    w, h, d, maxside, wnew, hnew, xoff, yoff, index;
l_float64  pi, theta, absangle, alpha, beta, diag;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixEmbedForRotation");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (L_ABS(angle) < VERY_SMALL_ANGLE)
        return pixClone(pixs);

        /* Test if big enough to hold any rotation */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    d = pixGetDepth(pixs);
    maxside = (l_int32)(sqrt((l_float64)(width * width) +
                             (l_float64)(height * height)) + 0.5);
    if (w >= maxside && h >= maxside)  /* big enough */
        return pixClone(pixs);
    
        /* Find the new sizes required to hold the image after rotation */
    pi = 3.1415926535;
    theta = atan((l_float64)w / (l_float64)h);
    absangle = (l_float64)(L_ABS(angle));
    alpha = theta - absangle;
    beta = theta + absangle;
    diag = sqrt((l_float64)(w * w) + (l_float64)(h * h));
    wnew = (l_int32)(diag * sin(beta) + 0.5);
    hnew = (l_int32)(diag * cos(alpha) + 0.5);
    wnew = L_MAX(w, wnew);  /* don't let it get smaller */
    hnew = L_MAX(h, hnew);  /* don't let it get smaller */
    if (wnew >= maxside)  /* clip */
        wnew = maxside;
    if (hnew >= maxside)  /* clip */
        hnew = maxside;

    if ((pixd = pixCreate(wnew, hnew, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);
    pixCopyText(pixd, pixs);
    xoff = (wnew - w) / 2;
    yoff = (hnew - h) / 2;

        /* Set background to color to be rotated in */
    cmap = pixGetColormap(pixd);
    if (!cmap) {
        if ((d == 1 && incolor == L_BRING_IN_BLACK) ||
            (d > 1 && incolor == L_BRING_IN_WHITE))
            pixSetAll(pixd);
    }
    else {  /* handle colormap */
        if (incolor == L_BRING_IN_BLACK)
            pixcmapAddBlackOrWhite(cmap, 0, &index);
        else  /* L_BRING_IN_WHITE */
            pixcmapAddBlackOrWhite(cmap, 1, &index);
        pixSetAllArbitrary(pixd, index);
    }

    pixRasterop(pixd, xoff, yoff, w, h, PIX_SRC, pixs, 0, 0);
    return pixd;
}


