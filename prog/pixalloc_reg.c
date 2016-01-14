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
 * pixalloc_reg.c
 *
 *   Tests custom pix allocator.
 *
 *   The custom allocator is intended for situations where a number of large
 *   pix will be repeatedly allocated and freed over the lifetime of a program.
 *   If those pix are large, relying on malloc and free can result in
 *   fragmentation, even if there are no small memory leaks in the program.
 *
 *   Here we test the allocator in two situations:
 *     * a small number of relatively large pix
 *     * a large number of very small pix
 *
 *   For the second case, timing shows that the custom allocator does
 *   about as well as (malloc, free), even for thousands of very small pix.
 *   (Turn off logging to get a fair comparison).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32      i, j;
l_float32    factor;
BOX         *box;
BOXA        *boxa;
NUMA        *na;
PIX         *pix, *pixt, *pixs;
PIX         *pixt1, *pixt2, *pixt3, *pixt4;
PIXA        *pixa, *pixas;
PIXAA       *paa;
static char  mainName[] = "pixalloc_reg";

    setPixMemoryManager(pmsCustomAlloc, pmsCustomDealloc);

        /* Make a few large pix */
    na = numaCreate(4);
    numaAddNumber(na, 5);
    numaAddNumber(na, 4);
    numaAddNumber(na, 3);
    numaAddNumber(na, 3);
    pmsCreate(200000, 400000, na, "junklog1");
    pixs = pixRead("marge.jpg");
    box = boxCreate(130, 93, 263, 253);
    factor = sqrt(2.0);
    pixt1 = pixClipRectangle(pixs, box, NULL);  /* 266 KB */
    pixt2 = pixScale(pixt1, factor, factor);    /* 532 KB */
    pixt3 = pixScale(pixt2, factor, factor);    /* 1064 KB */
    pixt4 = pixScale(pixt3, factor, factor);    /* 2128 KB */
    pixas = pixaCreate(4);
    pixaAddPix(pixas, pixt1, L_INSERT);
    pixaAddPix(pixas, pixt2, L_INSERT);
    pixaAddPix(pixas, pixt3, L_INSERT);
    pixaAddPix(pixas, pixt4, L_INSERT);

    paa = pixaaCreate(0);
    for (i = 0; i < 4; i++) {
        pixa = pixaCreate(0);
        pixaaAddPixa(paa, pixa, L_INSERT);
        pix = pixaGetPix(pixas, i, L_CLONE);
        for (j = 0; j < 4; j++) {
            pixt = pixCopy(NULL, pix);
            pixaAddPix(pixa, pixt, L_INSERT);
        }
        pixDestroy(&pix);
    }
    pixaaDestroy(&paa);

    paa = pixaaCreate(0);
    for (i = 0; i < 4; i++) {
        pixa = pixaCreate(0);
        pixaaAddPixa(paa, pixa, L_INSERT);
        pix = pixaGetPix(pixas, i, L_CLONE);
        for (j = 0; j < 4; j++) {
            pixt = pixCopy(NULL, pix);
            pixaAddPix(pixa, pixt, L_INSERT);
        }
        pixDestroy(&pix);
    }
    pixaaDestroy(&paa);

    paa = pixaaCreate(0);
    for (i = 0; i < 4; i++) {
        pixa = pixaCreate(0);
        pixaaAddPixa(paa, pixa, L_INSERT);
        pix = pixaGetPix(pixas, i, L_CLONE);
        for (j = 0; j < 4; j++) {
            pixt = pixCopy(NULL, pix);
            pixaAddPix(pixa, pixt, L_INSERT);
        }
        pixDestroy(&pix);
    }
    pixaaDestroy(&paa);

    numaDestroy(&na);
    boxDestroy(&box);
    pixDestroy(&pixs);
    pixaDestroy(&pixas);
    pmsDestroy();

        /* Make many small pix */
    startTimer();
    na = numaCreate(10);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 500);
    numaAddNumber(na, 100);
    numaAddNumber(na, 100);
    numaAddNumber(na, 100);
    if (1)   /* 1 for logging; 0 for speed comparison */
        pmsCreate(20, 40, na, "junklog2");
    else
        pmsCreate(20, 40, na, NULL);
    pixs = pixRead("feyn.tif");

    for (i = 0; i < 5; i++) {
        boxa = pixConnComp(pixs, &pixa, 8);
        boxaDestroy(&boxa);
        pixaDestroy(&pixa);
    }

    numaDestroy(&na);
    pixDestroy(&pixs);
    pmsDestroy();
    fprintf(stderr, "Time (custom) = %7.3f sec\n", stopTimer());

        /* Use malloc and free for speed comparison */
    setPixMemoryManager(malloc, free);
    startTimer();
    na = numaCreate(10);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 2000);
    numaAddNumber(na, 500);
    numaAddNumber(na, 100);
    numaAddNumber(na, 100);
    numaAddNumber(na, 100);
    pixs = pixRead("feyn.tif");

    for (i = 0; i < 5; i++) {
        boxa = pixConnComp(pixs, &pixa, 8);
        boxaDestroy(&boxa);
        pixaDestroy(&pixa);
    }

    numaDestroy(&na);
    pixDestroy(&pixs);
    fprintf(stderr, "Time (default) = %7.3f sec\n", stopTimer());


}


