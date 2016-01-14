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
 * rankhisto_reg.c
 *
 *   Tests grayscale rank functions:
 *      (1) pixGetRankColorArray()
 *      (2) numaDiscretizeRankAndIntensity()
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef  COMPILER_MSVC
#include <unistd.h>
#else
    /* Need declaration of Sleep() defined in WinBase.h, but must
     * include Windows.h to avoid errors  */
#include <Windows.h>
#endif  /* COMPILER_MSVC */
#include "allheaders.h"

static PIXA *PixSavePlots1(void);
static PIXA *PixSavePlots2(void);


main(int    argc,
     char **argv)
{
char       textstr[256], fname[256];
l_int32    i, w, h, nbins, factor, rval, gval, bval, success, display;
l_int32    spike;
l_uint32  *array;
FILE      *fp;
L_BMF     *bmf6;
NUMA      *na, *nan, *nai, *narbin;
PIX       *pixs, *pixt, *pixd;
PIXA      *pixa;

    if (regTestSetup(argc, argv, &fp, &display, &success, NULL))
        return 1;

    pixs = pixRead("map1.jpg");
    pixGetDimensions(pixs, &w, &h, NULL);
    factor = L_MAX(1, (l_int32)sqrt((l_float64)(w * h / 20000.0)));
    fprintf(stderr, "factor: %d\n", factor);
    nbins = 10;
    pixGetRankColorArray(pixs, nbins, factor, &array, 2);
    for (i = 0; i < nbins; i++)
        fprintf(stderr, "%d: %x\n", i, array[i]);
    bmf6 = bmfCreate("./fonts", 6);
    pixa = pixaCreate(nbins);
    for (i = 0; i < nbins; i++) {
        pixt = pixCreate(200, 200, 32);
        pixSetAllArbitrary(pixt, array[i]);
        extractRGBValues(array[i], &rval, &gval, &bval);
        snprintf(textstr, sizeof(textstr),
                 "%d: (%d %d %d)", i, rval, gval, bval);
        pixSaveTiledWithText(pixt, pixa, 200, (i % 5 == 0) ? 1 : 0,
                             20, 2, bmf6, textstr, 0xff000000, L_ADD_BELOW);
        pixDestroy(&pixt);
    }
    pixd = pixaDisplay(pixa, 0, 0);
    pixWrite("/tmp/junkrankhisto.0.png", pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/junkrankhisto.0.png", 0, &success);
    pixDisplayWithTitle(pixd, 100, 100, NULL, display);
    pixaDestroy(&pixa);
    pixDestroy(&pixd);

        /* Save the histogram plots */
#ifndef  COMPILER_MSVC
    sleep(2);  /* give gnuplot time to write out the files */
#else
    Sleep(2000);
#endif  /* COMPILER_MSVC */
    pixa = PixSavePlots1();
    pixd = pixaDisplay(pixa, 0, 0);
    pixWrite("/tmp/junkrankhisto.1.png", pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/junkrankhisto.1.png", 1, &success);
    pixDisplayWithTitle(pixd, 100, 600, NULL, display);
    pixaDestroy(&pixa);
    pixDestroy(&pixd);

        /* Map to the lightest bin; then do TRC adjustment */
    pixt = pixLinearMapToTargetColor(NULL, pixs, array[nbins - 1], 0xffffff00);
    pixd = pixGammaTRC(NULL, pixt, 1.0, 0, 240);
    pixWrite("/tmp/junkrankhisto.2.png", pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/junkrankhisto.2.png", 2, &success);
    pixDisplayWithTitle(pixd, 600, 100, NULL, display);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

        /* Now test the edge cases for the histogram and rank LUT,
         * where all the histo data is piled up at one place. 
         * We only require that the result be sensible. */
    for (i = 0; i < 3; i++) {
        if (i == 0)
            spike = 0;
        else if (i == 1)
            spike = 50;
        else
            spike = 99;
        na = numaMakeConstant(0, 100);
        numaReplaceNumber(na, spike, 200.0);
        nan = numaNormalizeHistogram(na, 1.0);
        numaDiscretizeRankAndIntensity(nan, 10, &narbin, &nai, NULL, NULL);
        snprintf(fname, sizeof(fname), "/tmp/junkrtnan%d", i + 1);
        gplotSimple1(nan, GPLOT_PNG, fname, "Normalized Histogram");
        snprintf(fname, sizeof(fname), "/tmp/junkrtnai%d", i + 1);
        gplotSimple1(nai, GPLOT_PNG, fname, "Intensity vs. rank bin");
        snprintf(fname, sizeof(fname), "/tmp/junkrtnarbin%d", i + 1);
        gplotSimple1(narbin, GPLOT_PNG, fname, "LUT: rank bin vs. Intensity");
        numaDestroy(&na);
        numaDestroy(&nan);
        numaDestroy(&narbin);
        numaDestroy(&nai);
    }
#ifndef  COMPILER_MSVC
    sleep(2);  /* give gnuplot time to write out the files */
#else
    Sleep(2000);
#endif  /* COMPILER_MSVC */
    pixa = PixSavePlots2();
    pixd = pixaDisplay(pixa, 0, 0);
    pixWrite("/tmp/junkrankhisto.3.png", pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/junkrankhisto.3.png", 3, &success);
    pixDisplayWithTitle(pixd, 500, 600, NULL, display);
    pixaDestroy(&pixa);
    pixDestroy(&pixd);

    pixDestroy(&pixs);
    bmfDestroy(&bmf6);
    FREE(array);
    regTestCleanup(argc, argv, fp, success, NULL);
    return 0;
}


static PIXA *
PixSavePlots1(void)
{
PIX    *pixt;
PIXA   *pixa;

    pixa = pixaCreate(8);
    pixt = pixRead("/tmp/junkrtnan.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnar.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnai.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnarbin.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnabb.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnared.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnagreen.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnablue.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    return pixa;
}


static PIXA *
PixSavePlots2(void)
{
PIX    *pixt;
PIXA   *pixa;

    pixa = pixaCreate(9);
    pixt = pixRead("/tmp/junkrtnan1.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnai1.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnarbin1.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnan2.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnai2.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnarbin2.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnan3.png");
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnai3.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = pixRead("/tmp/junkrtnarbin3.png");
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    return pixa;
}


