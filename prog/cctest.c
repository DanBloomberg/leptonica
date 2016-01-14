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
 * cctest.c
 *
 *    This is a test of the following function:
 *
 *        BOXA   *pixConnComp(PIX *pixs, PIXA  **ppixa, l_int32 connectivity)
 *
 *                    pixs:  input pix
 *                    ppixa: &pixa (<optional> pixa of each c.c.)
 *                    connectivity (4 or 8)
 *                    boxa: returned array of boxes of c.c.
 *
 *        Use NULL for &pixa if you don't want the pixa array.
 *
 *        We test this for both modes, without and with
 *        the generation of the pixa.  When the pixa is
 *        generated, we compare it pixelwise with the source
 *        to verify that it is correct and complete.  We do
 *        these tests for both 4- and 8-connected components.
 *        The extraction of the components from the source image,
 *        and the regeneration of the dest image from components,
 *        are also a good test of the rasterop function.
 *        This has been tested with valgrind on a set of
 *        images and their bit inverses for memory correctness.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  NTIMES             10


main(int    argc,
     char **argv)
{
char        *filein;
l_int32      i, n, np, same;
FILE        *fp;
BOX         *box;
BOXA        *boxa, *boxa2;
PIX         *pixs, *pixd;
PIXA        *pixa;
PIXCMAP     *cmap;
static char  mainName[] = "cctest";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  cctest filein", mainName, 1));

    filein = argv[1];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
#if 0
	/* test speed of pixConnComp(), with only boxa output  */
    startTimer();
    for (i = 0; i < NTIMES; i++) {
	boxa = pixConnComp(pixs, NULL, 4);
	boxaDestroy(&boxa);
    }
    fprintf(stderr, "Time to compute 4-cc: %6.3f sec\n", stopTimer()/NTIMES);
    startTimer();
    for (i = 0; i < NTIMES; i++) {
	boxa = pixConnComp(pixs, NULL, 8);
	boxaDestroy(&boxa);
    }
    fprintf(stderr, "Time to compute 8-cc: %6.3f sec\n", stopTimer()/NTIMES);
#endif

#if 0
	/* test pixConnComp() with output to both boxa and pixa */
	/* first, test with 4-cc */
    boxa = pixConnComp(pixs, &pixa, 4);
    n = boxaGetCount(boxa);
    fprintf(stderr, "Number of 4 c.c. b.b: %d\n", n);
    np = pixaGetCount(pixa);
    fprintf(stderr, "Number of 4 c.c. pix: %d\n", np);
    pixd = pixaDisplay(pixa, pixGetWidth(pixs), pixGetHeight(pixs));
    pixWrite("junkout1", pixd, IFF_PNG);
    pixEqual(pixs, pixd, &same);
    if (same == 1)
	fprintf(stderr, "Source and reconstructed pix are the same.\n");
    else
	fprintf(stderr, "Error: source and reconstructed pix differ!\n");
    pixaDestroy(&pixa);
    boxaDestroy(&boxa);
    pixDestroy(&pixd);

	/* then, test with 8-cc */
    boxa = pixConnComp(pixs, &pixa, 8);
    n = boxaGetCount(boxa);
    fprintf(stderr, "Number of 8 c.c. b.b: %d\n", n);
    np = pixaGetCount(pixa);
    fprintf(stderr, "Number of 8 c.c. pix: %d\n", np);
    pixd = pixaDisplay(pixa, pixGetWidth(pixs), pixGetHeight(pixs));
    pixWrite("junkout2", pixd, IFF_PNG);
    pixEqual(pixs, pixd, &same);
    if (same == 1)
	fprintf(stderr, "Source and reconstructed pix are the same.\n");
    else
	fprintf(stderr, "Error: source and reconstructed pix differ!\n");
    pixaDestroy(&pixa);
    boxaDestroy(&boxa);
    pixDestroy(&pixd);
#endif 

#if 1
        /* Display each component as a random color in cmapped 8 bpp.
         * Background is color 0; it is set to white. */
    boxa = pixConnComp(pixs, &pixa, 4);
    pixd = pixaDisplayRandomCmap(pixa, pixGetWidth(pixs), pixGetHeight(pixs));
    cmap = pixGetColormap(pixd);
    pixcmapResetColor(cmap, 0, 255, 255, 255);  /* reset background to white */
    pixDisplay(pixd, 100, 100);
    pixWrite("junkout4", pixd, IFF_PNG);
    boxaDestroy(&boxa);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);
#endif

#if 0
	/* draw outline of each c.c. box */
    boxa = pixConnComp(pixs, NULL, 4);
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
	box = boxaGetBox(boxa, i, L_CLONE);
	pixRenderBox(pixs, box, 3, L_FLIP_PIXELS);
	boxDestroy(&box);   /* remember, clones need to be destroyed */
    }
    pixWrite("junkout3", pixs, IFF_PNG);
    boxaDestroy(&boxa);
#endif

#if 0
	/* test I/O */
    boxa = pixConnComp(pixs, NULL, 4);
    fp = fopen("junkboxa", "wb+");
    boxaWriteStream(fp, boxa);
    fclose(fp);
    fp = fopen("junkboxa", "r");
    boxa2 = boxaReadStream(fp);
    fclose(fp);
    fp = fopen("junkboxa2", "wb+");
    boxaWriteStream(fp, boxa2);
    fclose(fp);
    boxaDestroy(&boxa);
    boxaDestroy(&boxa2);
#endif

    pixDestroy(&pixs);

    exit(0);
}


