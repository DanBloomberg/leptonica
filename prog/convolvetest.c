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
 * convolvetest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  NTIMES   100

main(int    argc,
     char **argv)
{
l_int32      i, wc, hc;
PIX         *pixs, *pixacc, *pixd;
char        *filein, *fileout;
static char  mainName[] = "convolvetest";

    if (argc != 5)
	exit(ERROR_INT(" Syntax:  convolvetest filein wc hc fileout", mainName, 1));

    filein = argv[1];
    wc = atoi(argv[2]);
    hc = atoi(argv[3]);
    fileout = argv[4];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

#if 0  /* measure speed */
    pixacc = pixBlockconvAccum(pixs);
    for (i = 0; i < NTIMES; i++) {
	pixd = pixBlockconvGray(pixs, pixacc, wc, hc);
	if ((i+1) % 10 == 0)
	    fprintf(stderr, "%d iters\n", i + 1);
	pixDestroy(&pixd);
    }
    pixd = pixBlockconvGray(pixs, pixacc, wc, hc);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixacc);
#endif

#if 0  /* test pixBlockconvGray() */
    pixacc = pixBlockconvAccum(pixs);
    pixd = pixBlockconvGray(pixs, pixacc, wc, hc);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixacc);
#endif

#if 0  /* test pixBlockconv() */
    pixd = pixBlockconv(pixs, wc, hc);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
#endif

#if 0  /* test pixBlockrank() */
    pixacc = pixBlockconvAccum(pixs);
    pixd = pixBlockrank(pixs, pixacc, wc, hc, 0.5);
    pixWrite(fileout, pixd, IFF_TIFF_G4);
    pixDestroy(&pixacc);
#endif

#if 1  /* test pixBlocksum() */
    pixacc = pixBlockconvAccum(pixs);
    pixd = pixBlocksum(pixs, pixacc, wc, hc);
    pixInvert(pixd, pixd);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixacc);
#endif

    pixDestroy(&pixs);
    pixDestroy(&pixd);

    exit(0);
}

