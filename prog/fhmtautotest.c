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
 * fhmtautotest.c
 *
 *    Tests hmt from all hmt structuring elements
 *    by comparing the full image rasterop results with the
 *    automatically generated dwa results.
 *
 *    Results are identical for all operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32      i, nsels, same, xorcount;
char        *filein, *selname;
PIX         *pixs, *pixs1, *pixt1, *pixt2, *pixt3, *pixt4;
SEL         *sel;
SELA        *sela;
static char  mainName[] = "fhmtautotest";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  fhmtautotest filein", mainName, 1));

    filein = argv[1];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

    sela = selaAddHitMiss(NULL);
    nsels = selaGetCount(sela);

    for (i = 0; i < nsels; i++)
    {
	sel = selaGetSel(sela, i);
	selname = selGetName(sel);

	pixt1 = pixHMT(NULL, pixs, sel);

	pixs1 = pixAddBorder(pixs, 32, 0);
	pixt2 = pixFHMTGen_1(NULL, pixs1, selname);
	pixt3 = pixRemoveBorder(pixt2, 32);

	pixt4 = pixXor(NULL, pixt1, pixt3);
	pixZero(pixt4, &same);

	if (same == 1) {
	    fprintf(stderr, "hmt are identical for sel %d (%s)\n", i, selname);
	}
	else {
	    fprintf(stderr, "hmt differ for sel %d (%s)\n", i, selname);
	    pixCountPixels(pixt4, &xorcount, NULL);
	    fprintf(stderr, "Number of pixels in XOR: %d\n", xorcount);
	}

	pixDestroy(&pixt1);
	pixDestroy(&pixt2);
	pixDestroy(&pixt3);
	pixDestroy(&pixt4);
	pixDestroy(&pixs1);
    }

    pixDestroy(&pixs);
    selaDestroy(&sela);

    exit(0);
}

