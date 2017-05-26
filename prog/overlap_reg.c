/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*
 *  overlap_reg.c
 *
 *    Tests functions that combine boxes that overlap into
 *    their bounding regions.
 */

#include "allheaders.h"

    /* Determines maximum size of boxes.  Note the rapid change in
     * results as the maximum box dimension approaches a critical size. */
static const l_float32  maxsize[] = {5.0, 10.0, 15.0, 20.0, 25.0, 26.0, 27.0};

int main(int    argc,
         char **argv)
{
l_int32       i, n, k, x, y, w, h, result;
BOX          *box1;
BOXA         *boxa1, *boxa2, *boxa3, *boxa4;
PIX          *pix1, *pix2, *pix3;
PIXA         *pixa1;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
	return 1;

        /* Show the result as a kind of percolation problem */
    for (k = 0; k < 7; k++) {
        srand(45617);
        pixa1 = pixaCreate(2);
        boxa1 = boxaCreate(0);
        for (i = 0; i < 500; i++) {
            x = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
            y = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
            w = (l_int32)
              (1.0 + maxsize[k] * (l_float64)rand() / (l_float64)RAND_MAX);
            h = (l_int32)
              (1.0 + maxsize[k] * (l_float64)rand() / (l_float64)RAND_MAX);
            box1 = boxCreate(x, y, w, h);
            boxaAddBox(boxa1, box1, L_INSERT);
        }

        pix1 = pixCreate(660, 660, 1);
        pixRenderBoxa(pix1, boxa1, 2, L_SET_PIXELS);
        pixaAddPix(pixa1, pix1, L_INSERT);
        boxa2 = boxaCombineOverlaps(boxa1, NULL);
        pix2 = pixCreate(660, 660, 1);
        pixRenderBoxa(pix2, boxa2, 2, L_SET_PIXELS);
        pixaAddPix(pixa1, pix2, L_INSERT);

        pix3 = pixaDisplayTiledInRows(pixa1, 1, 1500, 1.0, 0, 50, 2);
        pixDisplayWithTitle(pix3, 100, 100 + 100 * k, NULL, rp->display);
        regTestWritePixAndCheck(rp, pix3, IFF_PNG);   /* 0 - 6 */
        fprintf(stderr, "%d: n_init = %d, n_final = %d\n",
                k, boxaGetCount(boxa1), boxaGetCount(boxa2));
        pixDestroy(&pix3);
        boxaDestroy(&boxa1);
        boxaDestroy(&boxa2);
        pixaDestroy(&pixa1);
    }

        /* Show for one case, with debugging */
    boxa1 = boxaCreate(0);
    pixa1 = pixaCreate(10);
    n = 80;
    for (i = 0; i < n; i++) {
        x = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
        y = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
        w = (l_int32)
          (10 + 48 * (l_float64)rand() / (l_float64)RAND_MAX);
        h = (l_int32)
          (10 + 53 * (l_float64)rand() / (l_float64)RAND_MAX);
        box1 = boxCreate(x, y, w, h);
        boxaAddBox(boxa1, box1, L_INSERT);
    }

    boxa2 = boxaCombineOverlaps(boxa1, pixa1);
    boxaContainedInBoxa(boxa2, boxa1, &result);  /* 7 */
    regTestCompareValues(rp, 1, result, 0);

    pix1 = pixaDisplayTiledInRows(pixa1, 32, 1500, 1.0, 0, 50, 2);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);   /* 8 */
    pixDisplayWithTitle(pix1, 600, 0, NULL, rp->display);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);
    pixaDestroy(&pixa1);
    pixDestroy(&pix1);

        /* Show for two boxa that are greedily munching each other */
    boxa1 = boxaCreate(0);
    boxa2 = boxaCreate(0);
    n = 80;
    for (i = 0; i < n; i++) {
        x = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
        y = (l_int32)(600.0 * (l_float64)rand() / (l_float64)RAND_MAX);
        w = (l_int32)
          (10 + 55 * (l_float64)rand() / (l_float64)RAND_MAX);
        h = (l_int32)
          (10 + 55 * (l_float64)rand() / (l_float64)RAND_MAX);
        box1 = boxCreate(x, y, w, h);
        if (i < n / 2)
            boxaAddBox(boxa1, box1, L_INSERT);
        else
            boxaAddBox(boxa2, box1, L_INSERT);
    }

    pixa1 = pixaCreate(0);
    boxaCombineOverlapsInPair(boxa1, boxa2, &boxa3, &boxa4, pixa1);
    pix1 = pixaDisplayTiledInRows(pixa1, 32, 1500, 1.0, 0, 50, 2);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);   /* 9 */
    pixDisplayWithTitle(pix1, 1200, 0, NULL, rp->display);
    pixDestroy(&pix1);
    pixaDestroy(&pixa1);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);
    boxaDestroy(&boxa3);
    boxaDestroy(&boxa4);

    return regTestCleanup(rp);
}

