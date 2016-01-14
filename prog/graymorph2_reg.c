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
 * graymorph2_reg.c
 *
 *   Compares graymorph results with special (3x1, 1x3, 3x3) cases
 *   against the general case.  Require exact equality.
 */

#include "allheaders.h"

main(int    argc,
     char **argv)
{
l_int32  success, display;
FILE    *fp;
PIX     *pixs, *pixt1, *pixt2, *pixd;
PIXA    *pixa;

    if (regTestSetup(argc, argv, &fp, &display, &success, NULL))
        return 1;

    pixs = pixRead("test8.jpg");

        /* Dilation */
    pixa = pixaCreate(0);
    pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
    pixt1 = pixDilateGray3(pixs, 3, 1);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixDilateGray(pixs, 3, 1);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 0, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixDilateGray3(pixs, 1, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixDilateGray(pixs, 1, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 1, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixDilateGray3(pixs, 3, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixDilateGray(pixs, 3, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 2, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplayWithTitle(pixd, 0, 100, "Dilation", display);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

        /* Erosion */
    pixa = pixaCreate(0);
    pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
    pixt1 = pixErodeGray3(pixs, 3, 1);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixErodeGray(pixs, 3, 1);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 3, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixErodeGray3(pixs, 1, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixErodeGray(pixs, 1, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 4, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixErodeGray3(pixs, 3, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixErodeGray(pixs, 3, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 5, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplayWithTitle(pixd, 250, 100, "Erosion", display);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

        /* Opening */
    pixa = pixaCreate(0);
    pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
    pixt1 = pixOpenGray3(pixs, 3, 1);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixOpenGray(pixs, 3, 1);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 6, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixOpenGray3(pixs, 1, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixOpenGray(pixs, 1, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 7, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixOpenGray3(pixs, 3, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixOpenGray(pixs, 3, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 8, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplayWithTitle(pixd, 500, 100, "Opening", display);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

        /* Closing */
    pixa = pixaCreate(0);
    pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
    pixt1 = pixCloseGray3(pixs, 3, 1);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixCloseGray(pixs, 3, 1);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 9, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixCloseGray3(pixs, 1, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixCloseGray(pixs, 1, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 10, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixt1 = pixCloseGray3(pixs, 3, 3);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixCloseGray(pixs, 3, 3);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);
    regTestComparePix(fp, argv, pixt1, pixt2, 11, &success);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplayWithTitle(pixd, 750, 100, "Closing", display);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    regTestCleanup(argc, argv, fp, success, NULL);
    pixDestroy(&pixs);
    return 0;
}

