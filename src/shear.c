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
 *  shear.c
 *
 *    About arbitrary lines
 *           PIX      *pixHShear()
 *           PIX      *pixVShear()
 *
 *    About special 'points': UL corner and center
 *           PIX      *pixHShearCorner()
 *           PIX      *pixVShearCorner()
 *           PIX      *pixHShearCenter()
 *           PIX      *pixVShearCenter()
 *
 *    In place about arbitrary lines
 *           l_int32   pixHShearIP()
 *           l_int32   pixVShearIP()
 *
 *    Static helper
 *      static l_float32  normalizeAngleForShear()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "allheaders.h"

    /* Shear angle must not get too close to -pi/2 or pi/2 */
static const l_float32   MIN_DIFF_FROM_HALF_PI = 0.04;

static l_float32 normalizeAngleForShear(l_float32 radang, l_float32 mindist);


#ifndef  NO_CONSOLE_IO
#define  DEBUG     0
#endif  /* ~NO_CONSOLE_IO */


/*-------------------------------------------------------------*
 *                    About arbitrary lines                    *
 *-------------------------------------------------------------*/
/*!
 *  pixHShear()
 *
 *      Input:  pixd (<optional>, this can be null, equal to pixs,
 *                    or different from pixs)
 *              pixs (no restrictions on depth)
 *              liney  (location of horizontal line, measured from origin)
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, always
 *
 *  Notes:
 *      (1) There are 3 cases:
 *            (a) pixd == null (make a new pixd)
 *            (b) pixd == pixs (in-place)
 *            (c) pixd != pixs
 *      (2) For these three cases, use these patterns, respectively:
 *              pixd = pixHShear(NULL, pixs, ...);
 *              pixHShear(pixs, pixs, ...);
 *              pixHShear(pixd, pixs, ...);
 *      (3) This shear leaves the horizontal line of pixels at y = liney
 *          invariant.  For a positive shear angle, pixels above this
 *          line are shoved to the right, and pixels below this line
 *          move to the left.
 *      (4) With positive shear angle, this can be used, along with
 *          pixVShear(), to perform a cw rotation, either with 2 shears
 *          (for small angles) or in the general case with 3 shears.
 *      (5) Changing the value of liney is equivalent to translating
 *          the result horizontally.
 *      (6) This brings in 'incolor' pixels from outside the image.
 *      (7) For in-place operation, pixs cannot be colormapped,
 *          because the in-place operation only blits in 0 or 1 bits,
 *          not an arbitrary colormap index.
 *      (8) The angle is brought into the range [-pi, -pi].  It is
 *          not permitted to be within MIN_DIFF_FROM_HALF_PI radians
 *          from either -pi/2 or pi/2.
 */
PIX *
pixHShear(PIX       *pixd,
          PIX       *pixs,
          l_int32    liney,
          l_float32  radang,
          l_int32    incolor)
{
l_int32    sign, w, h;
l_int32    y, yincr, inityincr, hshift;
l_float32  tanangle, invangle;

    PROCNAME("pixHShear");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor value", procName, pixd);

    if (pixd == pixs) {  /* in place */
        if (pixGetColormap(pixs) != NULL)
            return (PIX *)ERROR_PTR("pixs is colormapped", procName, pixd);
        pixHShearIP(pixd, liney, radang, incolor);
        return pixd;
    }

        /* Make sure pixd exists and is same size as pixs */
    if (!pixd) {
        if ((pixd = pixCreateTemplate(pixs)) == NULL)
            return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }
    else  /* pixd != pixs */
        pixResizeImageData(pixd, pixs);

        /* Normalize angle.  If no rotation, return a copy */
    radang = normalizeAngleForShear(radang, MIN_DIFF_FROM_HALF_PI);
    if (radang == 0.0 || tan(radang) == 0.0)
        return pixCopy(pixd, pixs);

        /* Initialize to value of incoming pixels */
    pixSetBlackOrWhite(pixd, incolor);

    pixGetDimensions(pixs, &w, &h, NULL);
    sign = L_SIGN(radang);
    tanangle = tan(radang);
    invangle = L_ABS(1. / tanangle); 
    inityincr = (l_int32)(invangle / 2.);
    yincr = (l_int32)invangle;
    pixRasterop(pixd, 0, liney - inityincr, w, 2 * inityincr, PIX_SRC,
                pixs, 0, liney - inityincr);

    for (hshift = 1, y = liney + inityincr; y < h; hshift++) {
        yincr = (l_int32)(invangle * (hshift + 0.5) + 0.5) - (y - liney);
        if (h - y < yincr)  /* reduce for last one if req'd */
            yincr = h - y;
        pixRasterop(pixd, -sign*hshift, y, w, yincr, PIX_SRC, pixs, 0, y);
#if DEBUG
        fprintf(stderr, "y = %d, hshift = %d, yincr = %d\n", y, hshift, yincr);
#endif /* DEBUG */
        y += yincr;
    }

    for (hshift = -1, y = liney - inityincr; y > 0; hshift--) {
        yincr = (y - liney) - (l_int32)(invangle * (hshift - 0.5) + 0.5);
        if (y < yincr)  /* reduce for last one if req'd */
            yincr = y;
        pixRasterop(pixd, -sign*hshift, y - yincr, w, yincr, PIX_SRC,
            pixs, 0, y - yincr);
#if DEBUG
        fprintf(stderr, "y = %d, hshift = %d, yincr = %d\n",
                y - yincr, hshift, yincr);
#endif /* DEBUG */
        y -= yincr;
    }

    return pixd;
}
                        

/*!
 *  pixVShear()
 *
 *      Input:  pixd (<optional>, this can be null, equal to pixs,
 *                    or different from pixs)
 *              pixs (no restrictions on depth)
 *              linex  (location of vertical line, measured from origin)
 *              angle (in radians; not too close to +-(pi / 2))
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) There are 3 cases:
 *            (a) pixd == null (make a new pixd)
 *            (b) pixd == pixs (in-place)
 *            (c) pixd != pixs
 *      (2) For these three cases, use these patterns, respectively:
 *              pixd = pixVShear(NULL, pixs, ...);
 *              pixVShear(pixs, pixs, ...);
 *              pixVShear(pixd, pixs, ...);
 *      (3) This shear leaves the vertical line of pixels at x = linex
 *          invariant.  For a positive shear angle, pixels to the right
 *          of this line are shoved downward, and pixels to the left
 *          of the line move upward.
 *      (4) With positive shear angle, this can be used, along with
 *          pixHShear(), to perform a cw rotation, either with 2 shears
 *          (for small angles) or in the general case with 3 shears.
 *      (5) Changing the value of linex is equivalent to translating
 *          the result vertically.
 *      (6) This brings in 'incolor' pixels from outside the image.
 *      (7) For in-place operation, pixs cannot be colormapped,
 *          because the in-place operation only blits in 0 or 1 bits,
 *          not an arbitrary colormap index.
 *      (8) The angle is brought into the range [-pi, -pi].  It is
 *          not permitted to be within MIN_DIFF_FROM_HALF_PI radians
 *          from either -pi/2 or pi/2.
 */
PIX *
pixVShear(PIX       *pixd,
          PIX       *pixs,
          l_int32    linex,
          l_float32  radang,
          l_int32    incolor)
{
l_int32    sign, w, h;
l_int32    x, xincr, initxincr, vshift;
l_float32  tanangle, invangle;

    PROCNAME("pixVShear");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor value", procName, NULL);

    if (pixd == pixs) {  /* in place */
        if (pixGetColormap(pixs) != NULL)
            return (PIX *)ERROR_PTR("pixs is colormapped", procName, pixd);
        pixVShearIP(pixd, linex, radang, incolor);
        return pixd;
    }

        /* Make sure pixd exists and is same size as pixs */
    if (!pixd) {
        if ((pixd = pixCreateTemplate(pixs)) == NULL)
            return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }
    else  /* pixd != pixs */
        pixResizeImageData(pixd, pixs);

        /* Normalize angle.  If no rotation, return a copy */
    radang = normalizeAngleForShear(radang, MIN_DIFF_FROM_HALF_PI);
    if (radang == 0.0 || tan(radang) == 0.0)
        return pixCopy(pixd, pixs);

        /* Initialize to value of incoming pixels */
    pixSetBlackOrWhite(pixd, incolor);

    pixGetDimensions(pixs, &w, &h, NULL);
    sign = L_SIGN(radang);
    tanangle = tan(radang);
    invangle = L_ABS(1. / tanangle); 
    initxincr = (l_int32)(invangle / 2.);
    xincr = (l_int32)invangle;
    pixRasterop(pixd, linex - initxincr, 0, 2 * initxincr, h, PIX_SRC,
                pixs, linex - initxincr, 0);

    for (vshift = 1, x = linex + initxincr; x < w; vshift++) {
        xincr = (l_int32)(invangle * (vshift + 0.5) + 0.5) - (x - linex);
        if (w - x < xincr)  /* reduce for last one if req'd */
            xincr = w - x;
        pixRasterop(pixd, x, sign*vshift, xincr, h, PIX_SRC, pixs, x, 0);
#if DEBUG
        fprintf(stderr, "x = %d, vshift = %d, xincr = %d\n", x, vshift, xincr);
#endif /* DEBUG */
        x += xincr;
    }

    for (vshift = -1, x = linex - initxincr; x > 0; vshift--) {
        xincr = (x - linex) - (l_int32)(invangle * (vshift - 0.5) + 0.5);
        if (x < xincr)  /* reduce for last one if req'd */
            xincr = x;
        pixRasterop(pixd, x - xincr, sign*vshift, xincr, h, PIX_SRC,
            pixs, x - xincr, 0);
#if DEBUG
        fprintf(stderr, "x = %d, vshift = %d, xincr = %d\n",
                x - xincr, vshift, xincr);
#endif /* DEBUG */
        x -= xincr;
    }

    return pixd;
}
                        


/*-------------------------------------------------------------*
 *             Shears about UL corner and center               *
 *-------------------------------------------------------------*/
/*!
 *  pixHShearCorner()
 *
 *      Input:  pixd (<optional>, if not null, must be equal to pixs)
 *              pixs
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, or null on error.
 *
 *  Notes:
 *      (1) See pixHShear() for usage.
 *      (2) This does a horizontal shear about the UL corner, with (+) shear
 *          pushing increasingly leftward (-x) with increasing y. 
 */
PIX *
pixHShearCorner(PIX       *pixd,
                PIX       *pixs,
                l_float32  radang,
                l_int32    incolor)
{
    PROCNAME("pixHShearCorner");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);

    return pixHShear(pixd, pixs, 0, radang, incolor);
}


/*!
 *  pixVShearCorner()
 *
 *      Input:  pixd (<optional>, if not null, must be equal to pixs)
 *              pixs
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, or null on error.
 *
 *  Notes:
 *      (1) See pixVShear() for usage.
 *      (2) This does a vertical shear about the UL corner, with (+) shear
 *          pushing increasingly downward (+y) with increasing x. 
 */
PIX *
pixVShearCorner(PIX       *pixd,
                PIX       *pixs,
                l_float32  radang,
                l_int32    incolor)
{
    PROCNAME("pixVShearCorner");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);

    return pixVShear(pixd, pixs, 0, radang, incolor);
}
                        

/*!
 *  pixHShearCenter()
 *
 *      Input:  pixd (<optional>, if not null, must be equal to pixs)
 *              pixs
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, or null on error.
 *
 *  Notes:
 *      (1) See pixHShear() for usage.
 *      (2) This does a horizontal shear about the center, with (+) shear
 *          pushing increasingly leftward (-x) with increasing y. 
 */
PIX *
pixHShearCenter(PIX       *pixd,
                PIX       *pixs,
                l_float32  radang,
                l_int32    incolor)
{
    PROCNAME("pixHShearCenter");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);

    return pixHShear(pixd, pixs, pixGetHeight(pixs) / 2, radang, incolor);
}


/*!
 *  pixVShearCenter()
 *
 *      Input:  pixd (<optional>, if not null, must be equal to pixs)
 *              pixs
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: pixd, or null on error.
 *
 *  Notes:
 *      (1) See pixVShear() for usage.
 *      (2) This does a vertical shear about the center, with (+) shear
 *          pushing increasingly downward (+y) with increasing x. 
 */
PIX *
pixVShearCenter(PIX       *pixd,
                PIX       *pixs,
                l_float32  radang,
                l_int32    incolor)
{
    PROCNAME("pixVShearCenter");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);

    return pixVShear(pixd, pixs, pixGetWidth(pixs) / 2, radang, incolor);
}



/*--------------------------------------------------------------------------*
 *                       In place about arbitrary lines                     *
 *--------------------------------------------------------------------------*/
/*!
 *  pixHShearIP()
 *
 *      Input:  pixs
 *              liney  (location of horizontal line, measured from origin)
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This is an in-place version of pixHShear(); see comments there.
 *      (2) This brings in 'incolor' pixels from outside the image.
 *      (3) pixs cannot be colormapped, because the in-place operation
 *          only blits in 0 or 1 bits, not an arbitrary colormap index.
 *      (4) Does a horizontal full-band shear about the line with (+) shear
 *          pushing increasingly leftward (-x) with increasing y. 
 */
l_int32
pixHShearIP(PIX       *pixs,
            l_int32    liney,
            l_float32  radang,
            l_int32    incolor)
{
l_int32    sign, w, h;
l_int32    y, yincr, inityincr, hshift;
l_float32  tanangle, invangle;

    PROCNAME("pixHShearIP");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return ERROR_INT("invalid incolor value", procName, 1);
    if (pixGetColormap(pixs) != NULL)
        return ERROR_INT("pixs is colormapped", procName, 1);

        /* Normalize angle */
    radang = normalizeAngleForShear(radang, MIN_DIFF_FROM_HALF_PI);
    if (radang == 0.0 || tan(radang) == 0.0)
        return 0;

    sign = L_SIGN(radang);
    pixGetDimensions(pixs, &w, &h, NULL);
    tanangle = tan(radang);
    invangle = L_ABS(1. / tanangle); 
    inityincr = (l_int32)(invangle / 2.);
    yincr = (l_int32)invangle;

    pixRasteropHip(pixs, liney - inityincr, 2 * inityincr, 0, incolor);

    for (hshift = 1, y = liney + inityincr; y < h; hshift++) {
        yincr = (l_int32)(invangle * (hshift + 0.5) + 0.5) - (y - liney);
        if (h - y < yincr)  /* reduce for last one if req'd */
            yincr = h - y;
        pixRasteropHip(pixs, y, yincr, -sign*hshift, incolor);
        y += yincr;
    }

    for (hshift = -1, y = liney - inityincr; y > 0; hshift--) {
        yincr = (y - liney) - (l_int32)(invangle * (hshift - 0.5) + 0.5);
        if (y < yincr)  /* reduce for last one if req'd */
            yincr = y;
        pixRasteropHip(pixs, y - yincr, yincr, -sign*hshift, incolor);
        y -= yincr;
    }

    return 0;
}
                        

/*!
 *  pixVShearIP()
 *
 *      Input:  pixs (all depths; not colormapped)
 *              linex  (location of vertical line, measured from origin)
 *              angle (in radians)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK);
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This is an in-place version of pixVShear(); see comments there.
 *      (2) This brings in 'incolor' pixels from outside the image.
 *      (3) pixs cannot be colormapped, because the in-place operation
 *          only blits in 0 or 1 bits, not an arbitrary colormap index.
 *      (4) Does a vertical full-band shear about the line with (+) shear
 *          pushing increasingly downward (+y) with increasing x. 
 */
l_int32
pixVShearIP(PIX       *pixs,
            l_int32    linex,
            l_float32  radang,
            l_int32    incolor)
{
l_int32    sign, w, h;
l_int32    x, xincr, initxincr, vshift;
l_float32  tanangle, invangle;

    PROCNAME("pixVShearIP");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return ERROR_INT("invalid incolor value", procName, 1);
    if (pixGetColormap(pixs) != NULL)
        return ERROR_INT("pixs is colormapped", procName, 1);

        /* Normalize angle */
    radang = normalizeAngleForShear(radang, MIN_DIFF_FROM_HALF_PI);
    if (radang == 0.0 || tan(radang) == 0.0)
        return 0;

    sign = L_SIGN(radang);
    pixGetDimensions(pixs, &w, &h, NULL);
    tanangle = tan(radang);
    invangle = L_ABS(1. / tanangle); 
    initxincr = (l_int32)(invangle / 2.);
    xincr = (l_int32)invangle;

    pixRasteropVip(pixs, linex - initxincr, 2 * initxincr, 0, incolor);

    for (vshift = 1, x = linex + initxincr; x < w; vshift++) {
        xincr = (l_int32)(invangle * (vshift + 0.5) + 0.5) - (x - linex);
        if (w - x < xincr)  /* reduce for last one if req'd */
            xincr = w - x;
        pixRasteropVip(pixs, x, xincr, sign*vshift, incolor);
        x += xincr;
    }

    for (vshift = -1, x = linex - initxincr; x > 0; vshift--) {
        xincr = (x - linex) - (l_int32)(invangle * (vshift - 0.5) + 0.5);
        if (x < xincr)  /* reduce for last one if req'd */
            xincr = x;
        pixRasteropVip(pixs, x - xincr, xincr, sign*vshift, incolor);
        x -= xincr;
    }

    return 0;
}


/*-------------------------------------------------------------------------*
 *                           Angle normalization                           *
 *-------------------------------------------------------------------------*/
static l_float32
normalizeAngleForShear(l_float32  radang,
                       l_float32  mindist)
{
l_float32 pi, diff90;

    PROCNAME("normalizeAngleForShear");

       /* Bring angle into range from [-pi, pi] */
    pi = 3.14159265;
    if (radang < -pi || radang > pi)
        radang = radang - (l_int32)(radang / pi) * pi;

       /* If angle is too close to pi/2 or -pi/2, move away and issue warning */
    diff90 = radang - pi / 2.0;
    if (L_ABS(diff90) < mindist)
        L_WARNING("angle close to pi/2; shifting away", procName);
    if (diff90 > -mindist && diff90 < 0.0)
        radang = pi / 2.0 - mindist;
    else if (diff90 >= 0.0 && diff90 < mindist)
        radang = pi / 2.0 + mindist;
    diff90 = radang + pi / 2.0;
    if (L_ABS(diff90) < mindist)
        L_WARNING("angle close to -pi/2; shifting away", procName);
    if (diff90 > -mindist && diff90 < 0.0)
        radang = -pi / 2.0 - mindist;
    else if (diff90 >= 0.0 && diff90 < mindist)
        radang = -pi / 2.0 + mindist;

    return radang;
}

