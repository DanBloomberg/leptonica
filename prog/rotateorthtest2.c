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
 * rotateorthtest2.c
 *
 *    Regression test for all rotateorth functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   BINARY_IMAGE        "test1.png"
#define   GRAYSCALE_IMAGE     "test8.jpg"
#define   FOUR_BPP_IMAGE      "weasel4.8g.png"
#define   COLORMAP_IMAGE      "dreyfus8.png"
#define   RGB_IMAGE           "marge.jpg"

void rotateOrthTest(char *fname);


main(int    argc,
     char **argv)
{
static char  mainName[] = "rotateorthtest2";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  rotateorthtest2", mainName, 1));

    fprintf(stderr, "Test binary image:\n");
    rotateOrthTest(BINARY_IMAGE);
    fprintf(stderr, "Test 4 bpp colormapped image:\n");
    rotateOrthTest(FOUR_BPP_IMAGE);
    fprintf(stderr, "Test grayscale image:\n");
    rotateOrthTest(GRAYSCALE_IMAGE);
    fprintf(stderr, "Test colormap image:\n");
    rotateOrthTest(COLORMAP_IMAGE);
    fprintf(stderr, "Test rgb image:\n");
    rotateOrthTest(RGB_IMAGE);
    return 0;
}


void
rotateOrthTest(char *fname)
{
l_int32   zero, count;
PIX      *pixs, *pixt, *pixd;

    PROCNAME("rotateOrthTest");

    if ((pixs = pixRead(fname)) == NULL)
	return ERROR_VOID("pixs not read", procName);

	/* Test 4 successive 90 degree rotations */
    pixt = pixRotate90(pixs, 1);
    pixd = pixRotate90(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixRotate90(pixd, 1);
    pixDestroy(&pixd);
    pixd = pixRotate90(pixt, 1);
    pixDestroy(&pixt);
    pixXor(pixd, pixd, pixs);
    pixZero(pixd, &zero);
    if (zero)
	fprintf(stderr, "OK.  Four 90-degree rotations gives I\n");
    else {
	pixCountPixels(pixd, &count, NULL);
	fprintf(stderr, "Failure for four 90-degree rotations; count = %d\n",
		count);
    }
    pixDestroy(&pixd);

	/* Test 2 successive 180 degree rotations */
    pixt = pixRotate180(NULL, pixs);
    pixRotate180(pixt, pixt);
    pixXor(pixt, pixt, pixs);
    pixZero(pixt, &zero);
    if (zero)
	fprintf(stderr, "OK.  Two 180-degree rotations gives I\n");
    else {
	pixCountPixels(pixt, &count, NULL);
	fprintf(stderr, "Failure for two 180-degree rotations; count = %d\n",
		count);
    }
    pixDestroy(&pixt);

	/* Test 2 successive LR flips */
    pixt = pixFlipLR(NULL, pixs);
    pixFlipLR(pixt, pixt);
    pixXor(pixt, pixt, pixs);
    pixZero(pixt, &zero);
    if (zero)
	fprintf(stderr, "OK.  Two LR flips gives I\n");
    else {
	pixCountPixels(pixt, &count, NULL);
	fprintf(stderr, "Failure for two LR flips; count = %d\n",
		count);
    }
    pixDestroy(&pixt);

	/* Test 2 successive TB flips */
    pixt = pixFlipTB(NULL, pixs);
    pixFlipTB(pixt, pixt);
    pixXor(pixt, pixt, pixs);
    pixZero(pixt, &zero);
    if (zero)
	fprintf(stderr, "OK.  Two TB flips gives I\n");
    else {
	pixCountPixels(pixt, &count, NULL);
	fprintf(stderr, "Failure for two TB flips; count = %d\n",
		count);
    }
    pixDestroy(&pixt);

    pixDestroy(&pixs);
    return;
}

