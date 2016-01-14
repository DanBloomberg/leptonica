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
 *  warper.c
 *
 *      High-level captcha interface
 *          PIX               *pixSimpleCaptcha()
 *
 *      Random sinusoidal warping
 *          PIX               *pixRandomHarmonicWarp()
 *
 *      Helper functions
 *          static l_float64  *generateRandomNumberArray()
 *          static l_int32     applyWarpTransform()
 *
 *      Version using a LUT for sin
 *          PIX               *pixRandomHarmonicWarpLUT()
 *          static l_int32     applyWarpTransformLUT()
 *          static l_int32     makeSinLUT()
 *          static l_float32   getSinFromLUT()
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

static l_float64 *generateRandomNumberArray(l_int32 size);
static l_int32 applyWarpTransform(l_float32 xmag, l_float32 ymag,
                                l_float32 xfreq, l_float32 yfreq,
                                l_float64 *randa, l_int32 nx, l_int32 ny,
                                l_int32 xp, l_int32 yp,
                                l_float32 *px, l_float32 *py);

#define  USE_SIN_TABLE    0



/*----------------------------------------------------------------------*
 *                High-level example captcha interface                  *
 *----------------------------------------------------------------------*/
/*!
 *  pixSimpleCaptcha()
 *
 *      Input:  pixs (8 bpp; no colormap)
 *              border (added white pixels on each side)
 *              nterms (number of x and y harmonic terms)
 *              seed (of random number generator)
 *              color (for colorizing; in 0xrrggbb00 format; use 0 for black)
 *              cmapflag (1 for colormap output; 0 for rgb)
 *      Return: pixd (8 bpp cmap or 32 bpp rgb), or null on error
 *
 *  Notes:
 *      (1) This uses typical default values for generating captchas.
 *          The magnitudes of the harmonic warp are typically to be
 *          smaller when more terms are used, even though the phases
 *          are random.  See, for example, prog/warptest.c.
 */
PIX *
pixSimpleCaptcha(PIX      *pixs,
                 l_int32   border,
                 l_int32   nterms,
                 l_uint32  seed,
                 l_uint32  color,
                 l_int32   cmapflag)
{
l_int32    k;
l_float32  xmag[] = {7.0, 5.0, 4.0, 3.0};
l_float32  ymag[] = {10.0, 8.0, 6.0, 5.0};
l_float32  xfreq[] = {0.12, 0.10, 0.10, 0.11};
l_float32  yfreq[] = {0.15, 0.13, 0.13, 0.11};
PIX       *pixg, *pixgb, *pixw, *pixd;

    PROCNAME("pixSimpleCaptcha");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (nterms < 1 || nterms > 4)
        return (PIX *)ERROR_PTR("nterms must be in {1,2,3,4}", procName, NULL);

    k = nterms - 1;
    pixg = pixConvertTo8(pixs, 0);
    pixgb = pixAddBorder(pixg, border, 255);
    pixw = pixRandomHarmonicWarp(pixgb, xmag[k], ymag[k], xfreq[k], yfreq[k],
                                 nterms, nterms, seed, 255);
    pixd = pixColorizeGray(pixw, color, cmapflag);

    pixDestroy(&pixg);
    pixDestroy(&pixgb);
    pixDestroy(&pixw);
    return pixd;
}


/*----------------------------------------------------------------------*
 *                     Random sinusoidal warping                        *
 *----------------------------------------------------------------------*/
/*!
 *  pixRandomHarmonicWarp()
 *
 *      Input:  pixs (8 bpp; no colormap)
 *              xmag, ymag (maximum magnitude of x and y distortion)
 *              xfreq, yfreq (maximum magnitude of x and y frequency)
 *              nx, ny (number of x and y harmonic terms)
 *              seed (of random number generator)
 *              grayval (color brought in from the outside;
 *                       0 for black, 255 for white)
 *      Return: pixd (8 bpp; no colormap), or null on error
 *
 *  Notes:
 *      (1) To generate the warped image p(x',y'), set up the transforms
 *          that are in getWarpTransform().  For each (x',y') in the
 *          dest, the warp function computes the originating location
 *          (x, y) in the src.  The differences (x - x') and (y - y')
 *          are given as a sum of products of sinusoidal terms.  Each
 *          term is multiplied by a maximum amplitude (in pixels), and the
 *          angle is determined by a frequency and phase, and depends
 *          on the (x', y') value of the dest.  Random numbers with
 *          a variable input seed are used to allow the warping to be
 *          unpredictable.  A linear interpolation is used to find
 *          the value for the source at (x, y); this value is written
 *          into the dest.
 *      (2) This can be used to generate 'captcha's, which are somewhat
 *          randomly distorted images of text.  A typical set of parameters
 *          for a captcha are:
 *                    xmag = 4.0     ymag = 6.0
 *                    xfreq = 0.10   yfreq = 0.13
 *                    nx = 3         ny = 3
 *          Other examples can be found in prog/warptest.c.
 */
PIX *
pixRandomHarmonicWarp(PIX       *pixs,
                      l_float32  xmag,
                      l_float32  ymag,
                      l_float32  xfreq,
                      l_float32  yfreq,
                      l_int32    nx,
                      l_int32    ny,
                      l_uint32   seed,
                      l_int32    grayval)
{
l_int32     w, h, d, i, j, wpls, wpld, val;
l_uint32   *datas, *datad, *lined;
l_float32   x, y;
l_float64  *randa;
PIX        *pixd;

    PROCNAME("pixRandomHarmonicWarp");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);

        /* Compute filter output at each location.  We iterate over
         * the destination pixels.  For each dest pixel, use the
         * warp function to compute the four source pixels that
         * contribute, at the location (x, y).  Each source pixel
         * is divided into 16 x 16 subpixels to get an approximate value. */
    srand(seed);
    randa = generateRandomNumberArray(5 * (nx + ny));
    pixd = pixCreateTemplate(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            applyWarpTransform(xmag, ymag, xfreq, yfreq, randa, nx, ny,
                               j, i, &x, &y);
            linearInterpolatePixelGray(datas, wpls, w, h, x, y, grayval, &val);
            SET_DATA_BYTE(lined, j, val);
        }
    }

    FREE(randa);
    return pixd;
}


/*----------------------------------------------------------------------*
 *                         Static helper functions                      *
 *----------------------------------------------------------------------*/
static l_float64 *
generateRandomNumberArray(l_int32  size)
{
l_int32     i;
l_float64  *randa;

    PROCNAME("generateRandomNumberArray");

    if ((randa = (l_float64 *)CALLOC(size, sizeof(l_float64))) == NULL)
        return (l_float64 *)ERROR_PTR("calloc fail for randa", procName, NULL);

        /* Return random values between 0.5 and 1.0 */
    for (i = 0; i < size; i++)
        randa[i] = 0.5 * (1.0 + (l_float64)rand() / (l_float64)RAND_MAX);
    return randa;
}


/*!
 *  applyWarpTransform()
 *
 *  Notes:
 *      (1) Uses the internal sin function.
 */
static l_int32
applyWarpTransform(l_float32   xmag,
                   l_float32   ymag,
                   l_float32   xfreq,
                   l_float32   yfreq,
                   l_float64  *randa,
                   l_int32     nx,
                   l_int32     ny,
                   l_int32     xp,
                   l_int32     yp,
                   l_float32  *px,
                   l_float32  *py)
{
l_int32    i;
l_float64  twopi, x, y, anglex, angley;

    twopi = 6.283185;
    for (i = 0, x = xp; i < nx; i++) {
        anglex = xfreq * randa[3 * i + 1] * xp + twopi * randa[3 * i + 2];
        angley = yfreq * randa[3 * i + 3] * yp + twopi * randa[3 * i + 4];
        x += xmag * randa[3 * i] * sin(anglex) * sin(angley);
    }
    for (i = nx, y = yp; i < nx + ny; i++) {
        angley = yfreq * randa[3 * i + 1] * yp + twopi * randa[3 * i + 2];
        anglex = xfreq * randa[3 * i + 3] * xp + twopi * randa[3 * i + 4];
        y += ymag * randa[3 * i] * sin(angley) * sin(anglex);
    }

    *px = (l_float32)x;
    *py = (l_float32)y;
    return 0;
}


#if  USE_SIN_TABLE
/*----------------------------------------------------------------------*
 *                       Version using a LUT for sin                    *
 *----------------------------------------------------------------------*/
static l_int32 applyWarpTransformLUT(l_float32 xmag, l_float32 ymag,
                                l_float32 xfreq, l_float32 yfreq,
                                l_float64 *randa, l_int32 nx, l_int32 ny,
                                l_int32 xp, l_int32 yp, l_float32 *lut,
                                l_int32 npts, l_float32 *px, l_float32 *py);
static l_int32 makeSinLUT(l_int32 npts, NUMA **pna);
static l_float32 getSinFromLUT(l_float32 *tab, l_int32 npts,
                               l_float32 radang);

/*!
 *  pixRandomHarmonicWarpLUT()
 *
 *      Input:  pixs (8 bpp; no colormap)
 *              xmag, ymag (maximum magnitude of x and y distortion)
 *              xfreq, yfreq (maximum magnitude of x and y frequency)
 *              nx, ny (number of x and y harmonic terms)
 *              seed (of random number generator)
 *              grayval (color brought in from the outside;
 *                       0 for black, 255 for white)
 *      Return: pixd (8 bpp; no colormap), or null on error
 *
 *  Notes:
 *      (1) See notes and inline comments in pixRandomHarmonicWarp().
 *          This version uses a LUT for the sin function.  It is not
 *          appreciably faster than using the built-in sin function,
 *          and is here for comparison only.
 */
PIX *
pixRandomHarmonicWarpLUT(PIX       *pixs,
                         l_float32  xmag,
                         l_float32  ymag,
                         l_float32  xfreq,
                         l_float32  yfreq,
                         l_int32    nx,
                         l_int32    ny,
                         l_uint32   seed,
                         l_int32    grayval)
{
l_int32     w, h, d, i, j, wpls, wpld, val, npts;
l_uint32   *datas, *datad, *lined;
l_float32   x, y;
l_float32  *lut;
l_float64  *randa;
NUMA       *na;
PIX        *pixd;

    PROCNAME("pixRandomHarmonicWarp");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);

        /* Compute filter output at each location.  We iterate over
         * the destination pixels.  For each dest pixel, use the
         * warp function to compute the four source pixels that
         * contribute, at the location (x, y).  Each source pixel
         * is divided into 16 x 16 subpixels to get an approximate value. */
    srand(seed);
    randa = generateRandomNumberArray(5 * (nx + ny));
    pixd = pixCreateTemplate(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    npts = 100;
    makeSinLUT(npts, &na);
    lut = numaGetFArray(na, L_NOCOPY);
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            applyWarpTransformLUT(xmag, ymag, xfreq, yfreq, randa, nx, ny,
                                  j, i, lut, npts, &x, &y);
            linearInterpolatePixelGray(datas, wpls, w, h, x, y, grayval, &val);
            SET_DATA_BYTE(lined, j, val);
        }
    }

    numaDestroy(&na);
    FREE(randa);
    return pixd;
}


/*!
 *  applyWarpTransformLUT()
 *
 *  Notes:
 *      (1) Uses an LUT for computing sin(theta).  There is little speed
 *          advantage to using the LUT.
 */
static l_int32
applyWarpTransformLUT(l_float32   xmag,
                      l_float32   ymag,
                      l_float32   xfreq,
                      l_float32   yfreq,
                      l_float64  *randa,
                      l_int32     nx,
                      l_int32     ny,
                      l_int32     xp,
                      l_int32     yp,
                      l_float32  *lut,
                      l_int32     npts,
                      l_float32  *px,
                      l_float32  *py)
{
l_int32    i;
l_float64  twopi, x, y, anglex, angley, sanglex, sangley;

    twopi = 6.283185;
    for (i = 0, x = xp; i < nx; i++) {
        anglex = xfreq * randa[3 * i + 1] * xp + twopi * randa[3 * i + 2];
        angley = yfreq * randa[3 * i + 3] * yp + twopi * randa[3 * i + 4];
        sanglex = getSinFromLUT(lut, npts, anglex);
        sangley = getSinFromLUT(lut, npts, angley);
        x += xmag * randa[3 * i] * sanglex * sangley;
    }
    for (i = nx, y = yp; i < nx + ny; i++) {
        angley = yfreq * randa[3 * i + 1] * yp + twopi * randa[3 * i + 2];
        anglex = xfreq * randa[3 * i + 3] * xp + twopi * randa[3 * i + 4];
        sanglex = getSinFromLUT(lut, npts, anglex);
        sangley = getSinFromLUT(lut, npts, angley);
        y += ymag * randa[3 * i] * sangley * sanglex;
    }

    *px = (l_float32)x;
    *py = (l_float32)y;
    return 0;
}


static l_int32
makeSinLUT(l_int32  npts,
           NUMA   **pna)
{
l_int32    i, n;
l_float32  delx, fval;
NUMA      *na;

    PROCNAME("makeSinLUT");

    if (!pna)
        return ERROR_INT("&na not defined", procName, 1);
    *pna = NULL;
    if (npts < 2)
        return ERROR_INT("npts < 2", procName, 1);
    n = 2 * npts + 1;
    na = numaCreate(n);
    *pna = na;
    delx = 3.14159265 / (l_float32)npts;
    numaSetXParameters(na, 0.0, delx);
    for (i = 0; i < n / 2; i++)
         numaAddNumber(na, (l_float32)sin((l_float64)i * delx));
    for (i = 0; i < n / 2; i++) {
         numaGetFValue(na, i, &fval);
         numaAddNumber(na, -fval);
    }
    numaAddNumber(na, 0);

    return 0;
}


static l_float32
getSinFromLUT(l_float32  *tab,
              l_int32     npts,
              l_float32   radang)
{
l_int32    index;
l_float32  twopi, invtwopi, findex, diff;

        /* Restrict radang to [0, 2pi] */
    twopi = 6.283185;
    invtwopi = 0.1591549;
    if (radang < 0.0)
        radang += twopi * (1.0 - (l_int32)(-radang * invtwopi));
    else if (radang > 0.0)
        radang -= twopi * (l_int32)(radang * invtwopi);

        /* Interpolate */
    findex = (2.0 * (l_float32)npts) * (radang * invtwopi);
    index = (l_int32)findex;
    if (index == 2 * npts)
        return tab[index];
    diff = findex - index;
    return (1.0 - diff) * tab[index] + diff * tab[index + 1];
}
#endif  /* USE_SIN_TABLE */


