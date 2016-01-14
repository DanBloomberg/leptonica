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
 *  binarize_reg.c
 *
 *     Tests Sauvola local binarization.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

PIX *PixTest1(PIX *pixs, l_int32 size, l_float32 factor,
              const char *dispname, const char *fname);
PIX *PixTest2(PIX *pixs, l_int32 size, l_float32 factor, l_int32 nx,
              l_int32 ny, const char *dispname, const char *fname);
void PixTest3(PIX *pixs, l_int32 size, l_float32 factor,
              l_int32 nx, l_int32 ny);

static l_int32  ok = TRUE;

main(int    argc,
     char **argv)
{
l_int32      same;
PIX         *pixs, *pixd1, *pixd2;
static char  mainName[] = "binarize_reg.c";

    pixs = pixRead("w91frag.jpg");

    PixTest3(pixs, 3, 0.20, 2, 3);
    PixTest3(pixs, 6, 0.20, 100, 100);
    PixTest3(pixs, 10, 0.40, 10, 10);
    PixTest3(pixs, 10, 0.40, 20, 20);
    PixTest3(pixs, 20, 0.34, 30, 30);

    pixd1 = PixTest1(pixs, 7, 0.34, "junkdisp1", "junkpixd1");
    pixd2 = PixTest2(pixs, 7, 0.34, 4, 4, "junkdisp2", "junkpixd2");
    pixEqual(pixd1, pixd2, &same);
    if (!same) {
        ok = FALSE;
        fprintf(stderr, "Failure for nx = %d, ny = %d\n", 4, 4);
    } else
        fprintf(stderr, "Success for nx = %d, ny = %d\n", 4, 4);
    pixDestroy(&pixd1);
    pixDestroy(&pixd2);

    if (ok)
        fprintf(stderr, "\nSuccess: no errors\n");
    else
        fprintf(stderr, "\nFailure: errors\n");
    pixDestroy(&pixs);
    return 0;
}


PIX *
PixTest1(PIX         *pixs,
         l_int32      size,
         l_float32    factor,
         const char  *dispname,
         const char  *fname)  /* for writing */
{
l_int32  w, h;
PIX     *pixm, *pixsd, *pixth, *pixd, *pixt;
PIXA    *pixa;

    pixm = pixsd = pixth = pixd = NULL;
    pixGetDimensions(pixs, &w, &h, NULL);

        /* Get speed */
    startTimer();
    pixSauvolaBinarize(pixs, size, factor, 1, NULL, NULL, NULL, &pixd);
    fprintf(stderr, "\nSpeed: 1 tile,  %7.3f Mpix/sec\n",
            (w * h / 1000000.) / stopTimer());
    pixDestroy(&pixd);

        /* Get results */
    pixSauvolaBinarize(pixs, size, factor, 1, &pixm, &pixsd, &pixth, &pixd);
    if (dispname) {
        pixa = pixaCreate(0);
        pixSaveTiled(pixm, pixa, 1, 1, 30, 8);
        pixSaveTiled(pixsd, pixa, 1, 0, 30, 8);
        pixSaveTiled(pixth, pixa, 1, 1, 30, 8);
        pixSaveTiled(pixd, pixa, 1, 0, 30, 8);
        pixt = pixaDisplay(pixa, 0, 0);
        pixWrite(dispname, pixt, IFF_JFIF_JPEG);
        pixDisplay(pixt, 100, 100);
        pixDestroy(&pixt);
        pixaDestroy(&pixa);
    }
    if (fname)
        pixWrite(fname, pixd, IFF_PNG);

    pixDestroy(&pixm);
    pixDestroy(&pixsd);
    pixDestroy(&pixth);
    return pixd;
}
   

PIX *
PixTest2(PIX         *pixs,
         l_int32      size,
         l_float32    factor,
         l_int32      nx,
         l_int32      ny,
         const char  *dispname,
         const char  *fname)  /* for writing */
{
l_int32  w, h;
PIX     *pixth, *pixd, *pixt;
PIXA    *pixa;

    pixth = pixd = NULL;
    pixGetDimensions(pixs, &w, &h, NULL);

        /* Get speed */
    startTimer();
    pixSauvolaBinarizeTiled(pixs, size, factor, nx, ny, NULL, &pixd);
    fprintf(stderr, "Speed: %d x %d tiles,  %7.3f Mpix/sec\n",
            nx, ny, (w * h / 1000000.) / stopTimer());
    pixDestroy(&pixd);

        /* Get results */
    pixSauvolaBinarizeTiled(pixs, size, factor, nx, ny, &pixth, &pixd);
    if (dispname) {
        pixa = pixaCreate(0);
        pixSaveTiled(pixth, pixa, 1, 1, 30, 8);
        pixSaveTiled(pixd, pixa, 1, 0, 30, 8);
        pixt = pixaDisplay(pixa, 0, 0);
        pixWrite(dispname, pixt, IFF_JFIF_JPEG);
        pixDisplay(pixt, 100, 100);
        pixDestroy(&pixt);
        pixaDestroy(&pixa);
    }
    if (fname)
        pixWrite(fname, pixd, IFF_PNG);

    pixDestroy(&pixth);
    return pixd;
}
 

void
PixTest3(PIX       *pixs,
         l_int32    size,
         l_float32  factor,
         l_int32    nx,
         l_int32    ny)
{
l_int32  same;
PIX     *pixd1, *pixd2;

    pixd1 = PixTest1(pixs, size, factor, NULL, NULL);
    pixd2 = PixTest2(pixs, size, factor, nx, ny, NULL, NULL);
    pixEqual(pixd1, pixd2, &same);
    pixDestroy(&pixd1);
    pixDestroy(&pixd2);
    if (!same) {
        ok = FALSE;
        fprintf(stderr, "Failure for nx = %d, ny = %d\n", nx, ny);
    }
    else
        fprintf(stderr, "Success for nx = %d, ny = %d\n", nx, ny);
    return;
}


