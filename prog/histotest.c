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
 * histotest.c
 *
 *    Makes histograms of grayscale and color pixels
 *    from a pix.  For RGB color, this uses
 *    rgb --> octcube indexing.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein;
l_int32      d, sigbits;
GPLOT       *gplot;
NUMA        *na;
PIX         *pixs, *pixd;
static char  mainName[] = "histotest";

    if (argc != 2 && argc != 3)
	exit(ERROR_INT(" Syntax:  histotest filein [sigbits]",
	                 mainName, 1));

    filein = argv[1];
    if (argc == 3) {
	sigbits = atoi(argv[2]);
	fprintf(stderr, "using %d sig bits for octcube\n", sigbits);
    }

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    d = pixGetDepth(pixs);
	    
    if (d == 32) {
	if (argc == 2)
	    exit(ERROR_INT("specify sig bits for color histo", mainName, 1));
	if ((na = pixOctcubeHistogram(pixs, sigbits)) == NULL)
	    exit(ERROR_INT("na not made", mainName, 1));
	gplot = gplotCreate(NULL, na, "junkroot", GPLOT_X11, GPLOT_LINES,
		"color histogram with octcube indexing", "input pix",
		"octcube index", "number of pixels in cube");
	gplotMakeOutput(gplot);
	gplotDestroy(&gplot);
    }
    else {
	if (argc == 3)
	    fprintf(stderr, "gray histogram; ignoring sigbits param\n");
	if ((na = pixGrayHistogram(pixs)) == NULL)
	    exit(ERROR_INT("na not made", mainName, 1));
	gplot = gplotCreate(NULL, na, "junkroot", GPLOT_X11, GPLOT_LINES,
		"grayscale histogram", "input pix", "gray value",
		"number of pixels");
	gplotMakeOutput(gplot);
	gplotDestroy(&gplot);
    }

    pixDestroy(&pixs);
    exit(0);
}

