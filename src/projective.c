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
 *  projective.c
 *
 *      Projective (4-pt) image transformation using a sampled
 *      (to nearest integer) transform on each point
 *           PIX      *pixProjectiveSampled()
 *
 *      Projective (4-pt) image transformation using interpolation
 *      (or area mapping) for anti-aliasing images that are
 *      2, 4, or 8 bpp gray, or colormapped, or 32 bpp RGB
 *           PIX      *pixProjectiveInterpolated()
 *           PIX      *pixProjectiveInterpolatedColor()
 *           PIX      *pixProjectiveInterpolatedGray()
 *           void      projectiveInterpolatedColorLow()
 *           void      projectiveInterpolatedGrayLow()
 *
 *      Projective coordinate transformation
 *           l_int32   projectiveXformCoeffs()
 *           l_int32   projectiveXformSampled()
 *           l_int32   projectiveXformInterpolated()
 *
 *      A projective transform can be specified as a specific functional
 *      mapping between 4 points in the source and 4 points in the dest.
 *      It preserves straight lines, but is less stable than a bilinear
 *      transform, because it contains a division by a quantity that
 *      can get arbitrarily small.)
 *
 *      We give both a projective coordinate transformation and
 *      two projective image transformations.
 *
 *      For the former, we ask for the coordinate value (x',y')
 *      in the transformed space for any point (x,y) in the original
 *      space.  The coefficients of the transformation are found by
 *      solving 8 simultaneous equations for the 8 coordinates of
 *      the 4 points in src and dest.  The transformation can then
 *      be used to compute the associated image transform, by
 *      computing, for each dest pixel, the relevant pixel(s) in
 *      the source.  This can be done either by taking the closest
 *      src pixel to each transformed dest pixel ("sampling") or
 *      by doing an interpolation and averaging over 4 source
 *      pixels with appropriate weightings ("interpolated").
 *
 *      A typical application would be to remove keystoning
 *      due to a projective transform in the imaging system.
 *
 *      The projective transform is given by specifying two equations:
 *
 *          x' = (ax + by + c) / (gx + hy + 1)
 *          y' = (dx + ey + f) / (gx + hy + 1)
 *
 *      where the eight coefficients have been computed from four
 *      sets of these equations, each for two corresponding data pts.
 *      In practice, for each point (x,y) in the dest image, this
 *      equation is used to compute the corresponding point (x',y')
 *      in the src.  That computed point in the src is then used
 *      to determine the dest value in one of two ways:
 *
 *       - sampling: take the value of the src pixel in which this
 *                   point falls
 *       - interpolation: take appropriate linear combinations of the
 *                        four src pixels that this dest pixel would
 *                        overlap, with the coefficients proportional
 *                        to the amount of overlap
 *
 *      For small warp where there is little scale change, (e.g.,
 *      for rotation) area mapping is nearly equivalent to interpolation.
 *
 *      Typical relative timing of pointwise transforms (sampled = 1.0):
 *      8 bpp:   sampled        1.0
 *               interpolated   1.5
 *      32 bpp:  sampled        1.0
 *               interpolated   1.6
 *      Additionally, the computation time/pixel is nearly the same
 *      for 8 bpp and 32 bpp, for both sampled and interpolated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "allheaders.h"


/*-------------------------------------------------------------*
 *             Sampled projective image transformation           *
 *-------------------------------------------------------------*/
/*!
 *  pixProjectiveSampled()
 *
 *      Input:  pixs (all depths)
 *              ptad  (4 pts of final coordinate space)
 *              ptas  (4 pts of initial coordinate space)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Brings in either black or white pixels from the boundary.
 *      (2) Retains colormap, which you can do for a sampled transform..
 *      (3) No 3 of the 4 points may be collinear.
 *      (4) For 8 and 32 bpp pix, better quality is obtained by the
 *          somewhat slower pixProjectiveInterpolated().  See that
 *          function for relative timings between sampled and interpolated.
 */
PIX *
pixProjectiveSampled(PIX     *pixs,
                     PTA     *ptad,
                     PTA     *ptas,
                     l_int32  incolor)
{
l_int32     i, j, w, h, d, x, y, wpls, wpld, color, cmapindex;
l_uint32    val;
l_float32  *vc;
l_uint32   *datas, *datad, *lines, *lined;
PIX        *pixd;
PIXCMAP    *cmap;

    PROCNAME("pixProjectiveSampled");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor", procName, NULL);
    if (ptaGetCount(ptas) != 4)
        return (PIX *)ERROR_PTR("ptas count not 4", procName, NULL);
    if (ptaGetCount(ptad) != 4)
        return (PIX *)ERROR_PTR("ptad count not 4", procName, NULL);

        /* Get backwards transform from dest to src */
    projectiveXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    d = pixGetDepth(pixs);

        /* Init all dest pixels to color to be brought in from outside */
    if ((cmap = pixGetColormap(pixs)) != NULL) {
        if (incolor == L_BRING_IN_WHITE)
            color = 1;
        else
            color = 0;
        pixcmapAddBlackOrWhite(cmap, color, &cmapindex);
        pixSetAllArbitrary(pixd, cmapindex);
    }
    else {
        if ((d == 1 && incolor == L_BRING_IN_WHITE) ||
            (d > 1 && incolor == L_BRING_IN_BLACK))
            pixClearAll(pixd);
        else
            pixSetAll(pixd);
    }

        /* Scan over dest pixels */
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            projectiveXformSampled(vc, j, i, &x, &y);
            if (x < 0 || y < 0 || x >=w || y >= h)
                continue;
            if (d == 1) {
                lines = datas + y * wpls;
                if (GET_DATA_BIT(lines, x))
                    SET_DATA_BIT(lined, j);
            }
            else if (d == 8) {
                lines = datas + y * wpls;
                val = GET_DATA_BYTE(lines, x);
                SET_DATA_BYTE(lined, j, val);
            }
            else if (d == 32) {
                lines = datas + y * wpls;
                lined[j] = lines[x];
            }
            else {  /* all other depths */
                pixGetPixel(pixs, x, y, &val);
                pixSetPixel(pixd, j, i, val);
            }
        }
    }

    FREE(vc);
    return pixd;
}


/*-------------------------------------------------------------*
 *         Interpolated projective image transformation        *
 *-------------------------------------------------------------*/
/*!
 *  pixProjectiveInterpolated()
 *
 *      Input:  pixs (8 bpp gray or colormapped or 32 bpp)
 *              ptad  (4 pts of final coordinate space)
 *              ptas  (4 pts of initial coordinate space)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Brings in either black or white pixels from the boundary
 *      (2) Removes any existing colormap, if necessary, before transforming
 */
PIX *
pixProjectiveInterpolated(PIX      *pixs,
                          PTA      *ptad,
                          PTA      *ptas,
                          l_uint32  incolor)
{
l_int32   d;
l_uint32  colorval;
PIX      *pixt1, *pixt2, *pixd;

    PROCNAME("pixProjectiveInterpolated");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) == 1)
        return (PIX *)ERROR_PTR("pixs is 1 bpp", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor", procName, NULL);
    if (ptaGetCount(ptas) != 4)
        return (PIX *)ERROR_PTR("ptas count not 4", procName, NULL);
    if (ptaGetCount(ptad) != 4)
        return (PIX *)ERROR_PTR("ptad count not 4", procName, NULL);

        /* Remove cmap if it exists, and unpack to 8 bpp if necessary */
    pixt1 = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    d = pixGetDepth(pixt1);
    if (d < 8)
        pixt2 = pixConvertTo8(pixt1, FALSE);
    else
        pixt2 = pixClone(pixt1);
    d = pixGetDepth(pixt2);

        /* Compute actual color to bring in from edges */
    colorval = 0;
    if (incolor == L_BRING_IN_WHITE) {
        if (d == 8)
            colorval = 255;
        else  /* d == 32 */
            colorval = 0xffffff00;
    }

    if (d == 8)
        pixd = pixProjectiveInterpolatedGray(pixt2, ptad, ptas, colorval);
    else  /* d == 32 */
        pixd = pixProjectiveInterpolatedColor(pixt2, ptad, ptas, colorval);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    return pixd;
}


/*!
 *  pixProjectiveInterpolatedColor()
 *
 *      Input:  pixs (32 bpp)
 *              ptad  (4 pts of final coordinate space)
 *              ptas  (4 pts of initial coordinate space)
 *              colorval (e.g., 0 to bring in BLACK, 0xffffff00 for WHITE)
 *      Return: pixd, or null on error
 *
 *  *** Warning: implicit assumption about RGB component ordering ***
 */
PIX *
pixProjectiveInterpolatedColor(PIX      *pixs,
                               PTA      *ptad,
                               PTA      *ptas,
                               l_uint32  colorval)
{
l_int32     w, h, wpls, wpld;
l_float32  *vc;
l_uint32   *datas, *datad;
PIX        *pixd;

    PROCNAME("pixProjectiveInterpolatedColor");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs must be 32 bpp", procName, NULL);
    if (ptaGetCount(ptas) != 4)
        return (PIX *)ERROR_PTR("ptas count not 4", procName, NULL);
    if (ptaGetCount(ptad) != 4)
        return (PIX *)ERROR_PTR("ptad count not 4", procName, NULL);

        /* Get backwards transform from dest to src */
    projectiveXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    pixSetAllArbitrary(pixd, colorval);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    projectiveInterpolatedColorLow(datad, w, h, wpld, datas, wpls, vc);
    FREE(vc);

    return pixd;
}


/*!
 *  pixProjectiveInterpolatedGray()
 *
 *      Input:  pixs (8 bpp)
 *              ptad  (4 pts of final coordinate space)
 *              ptas  (4 pts of initial coordinate space)
 *              grayval (0 to bring in BLACK, 255 for WHITE)
 *      Return: pixd, or null on error
 */
PIX *
pixProjectiveInterpolatedGray(PIX     *pixs,
                              PTA     *ptad,
                              PTA     *ptas,
                              l_uint8  grayval)
{
l_int32     w, h, wpls, wpld;
l_float32  *vc;
l_uint32   *datas, *datad;
PIX        *pixd;

    PROCNAME("pixProjectiveInterpolatedGray");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
        return (PIX *)ERROR_PTR("pixs must be 8 bpp", procName, NULL);
    if (ptaGetCount(ptas) != 4)
        return (PIX *)ERROR_PTR("ptas count not 4", procName, NULL);
    if (ptaGetCount(ptad) != 4)
        return (PIX *)ERROR_PTR("ptad count not 4", procName, NULL);

        /* Get backwards transform from dest to src */
    projectiveXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    pixSetAllArbitrary(pixd, grayval);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    projectiveInterpolatedGrayLow(datad, w, h, wpld, datas, wpls, vc);
    FREE(vc);

    return pixd;
}


/*!
 *  projectiveInterpolatedColorLow()
 */
void
projectiveInterpolatedColorLow(l_uint32   *datad,
                               l_int32     w,
                               l_int32     h,
                               l_int32     wpld,
                               l_uint32   *datas,
                               l_int32     wpls,
                               l_float32  *vc)
{
l_int32    i, j, x, y, xf, yf, wm2, hm2;
l_int32    rval, gval, bval;
l_uint32   word00, word01, word10, word11, val;
l_uint32  *lines, *lined;

        /* Iterate over destination pixels */
    wm2 = w - 2;
    hm2 = h - 2;
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
                /* Compute src pixel and fraction corresponding to (i,j) */
            projectiveXformInterpolated(vc, j, i, &x, &y, &xf, &yf);

                /* Skip if off the edge; omit x = 0 and y = 0 because
                 * xf and yf can be < 0, in which case overflow is
                 * possible for val, and black pixels can be rendered
                 * on pixels at the src boundaries. */
            if (x < 1 || y < 1 || x > wm2 || y > hm2)
                continue;

                /* Do area weighting (eqiv. to linear interpolation) */
            lines = datas + y * wpls;
            word00 = *(lines + x);
            word10 = *(lines + x + 1);
            word01 = *(lines + wpls + x);
            word11 = *(lines + wpls + x + 1);
            rval = ((16 - xf) * (16 - yf) * ((word00 >> L_RED_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_RED_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_RED_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_RED_SHIFT) & 0xff) + 128) / 256;
            gval = ((16 - xf) * (16 - yf) * ((word00 >> L_GREEN_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_GREEN_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_GREEN_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_GREEN_SHIFT) & 0xff) + 128) / 256;
            bval = ((16 - xf) * (16 - yf) * ((word00 >> L_BLUE_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_BLUE_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_BLUE_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_BLUE_SHIFT) & 0xff) + 128) / 256;
            val = (rval << L_RED_SHIFT) | (gval << L_GREEN_SHIFT) |
                  (bval << L_BLUE_SHIFT);
            *(lined + j) = val;
        }
    }

    return;
}


/*!
 *  projectiveInterpolatedGrayLow()
 */
void
projectiveInterpolatedGrayLow(l_uint32   *datad,
                              l_int32     w,
                              l_int32     h,
                              l_int32     wpld,
                              l_uint32   *datas,
                              l_int32     wpls,
                              l_float32  *vc)
{
l_int32     i, j, x, y, xf, yf, wm2, hm2;
l_int32     v00, v01, v10, v11;
l_uint8     val;
l_uint32   *lines, *lined;

        /* Iterate over destination pixels */
    wm2 = w - 2;
    hm2 = h - 2;
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
                /* Compute src pixel and fraction corresponding to (i,j) */
            projectiveXformInterpolated(vc, j, i, &x, &y, &xf, &yf);

                /* Skip if off the edge; omit x = 0 and y = 0 because
                 * xf and yf can be < 0, in which case overflow is
                 * possible for val, and black pixels can be rendered
                 * on pixels at the src boundaries. */
            if (x < 1 || y < 1 || x > wm2 || y > hm2)
                continue;

                /* Do area weighting (eqiv. to linear interpolation) */
            lines = datas + y * wpls;
            v00 = (16 - xf) * (16 - yf) * GET_DATA_BYTE(lines, x);
            v10 = xf * (16 - yf) * GET_DATA_BYTE(lines, x + 1);
            v01 = (16 - xf) * yf * GET_DATA_BYTE(lines + wpls, x);
            v11 = xf * yf * GET_DATA_BYTE(lines + wpls, x + 1);
            val = (l_uint8)((v00 + v01 + v10 + v11 + 128) / 256);
            SET_DATA_BYTE(lined, j, val);
        }
    }

    return;
}


/*-------------------------------------------------------------*
 *                Projective coordinate transformation         *
 *-------------------------------------------------------------*/
/*!
 *  projectiveXformCoeffs()
 *
 *      Input:  ptas  (source 4 points; unprimed)
 *              ptad  (transformed 4 points; primed)
 *              &vc   (<return> vector of coefficients of transform)
 *      Return: 0 if OK; 1 on error
 *
 *  We have a set of 8 equations, describing the projective
 *  transformation that takes 4 points (ptas) into 4 other
 *  points (ptad).  These equations are:
 *
 *          x1' = (c[0]*x1 + c[1]*y1 + c[2]) / (c[6]*x1 + c[7]*y1 + 1)
 *          y1' = (c[3]*x1 + c[4]*y1 + c[5]) / (c[6]*x1 + c[7]*y1 + 1)
 *          x2' = (c[0]*x2 + c[1]*y2 + c[2]) / (c[6]*x2 + c[7]*y2 + 1)
 *          y2' = (c[3]*x2 + c[4]*y2 + c[5]) / (c[6]*x2 + c[7]*y2 + 1)
 *          x3' = (c[0]*x3 + c[1]*y3 + c[2]) / (c[6]*x3 + c[7]*y3 + 1)
 *          y3' = (c[3]*x3 + c[4]*y3 + c[5]) / (c[6]*x3 + c[7]*y3 + 1)
 *          x4' = (c[0]*x4 + c[1]*y4 + c[2]) / (c[6]*x4 + c[7]*y4 + 1)
 *          y4' = (c[3]*x4 + c[4]*y4 + c[5]) / (c[6]*x4 + c[7]*y4 + 1)
 *    
 *  Multiplying both sides of each eqn by the denominator, we get
 *
 *           AC = B
 *
 *  where B and C are column vectors
 *    
 *         B = [ x1' y1' x2' y2' x3' y3' x4' y4' ]
 *         C = [ c[0] c[1] c[2] c[3] c[4] c[5] c[6] c[7] ]
 *    
 *  and A is the 8x8 matrix
 *
 *             x1   y1     1     0   0    0   -x1*x1'  -y1*x1'
 *              0    0     0    x1   y1   1   -x1*y1'  -y1*y1'
 *             x2   y2     1     0   0    0   -x2*x2'  -y2*x2'
 *              0    0     0    x2   y2   1   -x2*y2'  -y2*y2'
 *             x3   y3     1     0   0    0   -x3*x3'  -y3*x3'
 *              0    0     0    x3   y3   1   -x3*y3'  -y3*y3'
 *             x4   y4     1     0   0    0   -x4*x4'  -y4*x4'
 *              0    0     0    x4   y4   1   -x4*y4'  -y4*y4'
 *
 *  These eight equations are solved here for the coefficients C.
 *
 *  These eight coefficients can then be used to find the mapping
 *  (x,y) --> (x',y'):
 *
 *           x' = (c[0]x + c[1]y + c[2]) / (c[6]x + c[7]y + 1)
 *           y' = (c[3]x + c[4]y + c[5]) / (c[6]x + c[7]y + 1)
 *
 *  that is implemented in projectiveXformSampled() and
 *  projectiveXFormInterpolated().
 */
l_int32
projectiveXformCoeffs(PTA         *ptas,
                      PTA         *ptad,
                      l_float32  **pvc)
{
l_int32     i;
l_float32   x1, y1, x2, y2, x3, y3, x4, y4;
l_float32  *b;   /* rhs vector of primed coords X'; coeffs returned in *pvc */
l_float32  *a[8];  /* 8x8 matrix A  */

    PROCNAME("projectiveXformCoeffs");

    if (!ptas)
        return ERROR_INT("ptas not defined", procName, 1);
    if (!ptad)
        return ERROR_INT("ptad not defined", procName, 1);
    if (!pvc)
        return ERROR_INT("&vc not defined", procName, 1);
        
    if ((b = (l_float32 *)CALLOC(8, sizeof(l_float32))) == NULL)
        return ERROR_INT("b not made", procName, 1);
    *pvc = b;

    ptaGetPt(ptas, 0, &x1, &y1);
    ptaGetPt(ptas, 1, &x2, &y2);
    ptaGetPt(ptas, 2, &x3, &y3);
    ptaGetPt(ptas, 3, &x4, &y4);
    ptaGetPt(ptad, 0, &b[0], &b[1]);
    ptaGetPt(ptad, 1, &b[2], &b[3]);
    ptaGetPt(ptad, 2, &b[4], &b[5]);
    ptaGetPt(ptad, 3, &b[6], &b[7]);

    for (i = 0; i < 8; i++) {
        if ((a[i] = (l_float32 *)CALLOC(8, sizeof(l_float32))) == NULL)
            return ERROR_INT("a[i] not made", procName, 1);
    }

    a[0][0] = x1;
    a[0][1] = y1;
    a[0][2] = 1.;
    a[0][6] = -x1 * b[0];
    a[0][7] = -y1 * b[0];
    a[1][3] = x1;
    a[1][4] = y1;
    a[1][5] = 1;
    a[1][6] = -x1 * b[1];
    a[1][7] = -y1 * b[1];
    a[2][0] = x2;
    a[2][1] = y2;
    a[2][2] = 1.;
    a[2][6] = -x2 * b[2];
    a[2][7] = -y2 * b[2];
    a[3][3] = x2;
    a[3][4] = y2;
    a[3][5] = 1;
    a[3][6] = -x2 * b[3];
    a[3][7] = -y2 * b[3];
    a[4][0] = x3;
    a[4][1] = y3;
    a[4][2] = 1.;
    a[4][6] = -x3 * b[4];
    a[4][7] = -y3 * b[4];
    a[5][3] = x3;
    a[5][4] = y3;
    a[5][5] = 1;
    a[5][6] = -x3 * b[5];
    a[5][7] = -y3 * b[5];
    a[6][0] = x4;
    a[6][1] = y4;
    a[6][2] = 1.;
    a[6][6] = -x4 * b[6];
    a[6][7] = -y4 * b[6];
    a[7][3] = x4;
    a[7][4] = y4;
    a[7][5] = 1;
    a[7][6] = -x4 * b[7];
    a[7][7] = -y4 * b[7];

    gaussjordan(a, b, 8);

    for (i = 0; i < 8; i++)
        FREE(a[i]);

    return 0;
}


/*!
 *  projectiveXformSampled()
 *
 *      Input:  vc (vector of 8 coefficients)
 *              (x, y)  (initial point)
 *              (&xp, &yp)   (<return> transformed point)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: this does not check ptrs for returned data!
 */
l_int32
projectiveXformSampled(l_float32  *vc,
                       l_int32     x,
                       l_int32     y,
                       l_int32    *pxp,
                       l_int32    *pyp)
{
l_float32  factor;

    PROCNAME("projectiveXformSampled");

    if (!vc)
        return ERROR_INT("vc not defined", procName, 1);

    factor = 1. / (vc[6] * x + vc[7] * y + 1.);
    *pxp = (l_int32)(factor * (vc[0] * x + vc[1] * y + vc[2]) + 0.5);
    *pyp = (l_int32)(factor * (vc[3] * x + vc[4] * y + vc[5]) + 0.5);

    return 0;
}


/*!
 *  projectiveXformInterpolated()
 *
 *      Input:  vc (vector of 8 coefficients)
 *              (x, y)  (initial point)
 *              (&xp, &yp)   (<return> transformed point)
 *              (&fxp, &fyp)   (<return> fractional transformed point)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: this does not check ptrs for returned data!
 */
l_int32
projectiveXformInterpolated(l_float32  *vc,
                            l_int32     x,
                            l_int32     y,
                            l_int32    *pxp,
                            l_int32    *pyp,
                            l_int32    *pfxp,
                            l_int32    *pfyp)
{
l_float32  xp, yp, factor;

    PROCNAME("projectiveXformInterpolated");

    if (!vc)
        return ERROR_INT("vc not defined", procName, 1);

    factor = 1. / (vc[6] * x + vc[7] * y + 1.);
    xp = factor * (vc[0] * x + vc[1] * y + vc[2]);
    yp = factor * (vc[3] * x + vc[4] * y + vc[5]);
    *pxp = (l_int32)xp;
    *pyp = (l_int32)yp;
    *pfxp = (l_int32)(16. * (xp - *pxp));
    *pfyp = (l_int32)(16. * (yp - *pyp));

    return 0;
}

