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
 * numatest.c
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *fileout;
l_int32      i, binsize, binstart, nbins;
l_float32    pi, val, angle;
GPLOT       *gplot;
NUMA        *na, *nahisto, *nax;
static char  mainName[] = "numatest";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  numatest fileout", mainName, 1));

    fileout = argv[1];
    pi = 3.1415926535;

    na = numaCreate(5000);
    
    for (i = 0; i < 500000; i++) {
        angle = 0.02293 * i * pi;
	val = (l_float32)(999. * sin(angle));
	numaAddNumber(na, val);
    }

#if 1
    nahisto = numaMakeHistogramClipped(na, 6, 2000);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(0, 1, nbins);
#endif

#if 0
    nahisto = numaMakeHistogram(na, 1000, &binsize, &binstart);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(binstart, binsize, nbins);
    fprintf(stderr, " binsize = %d, binstart = %d\n", binsize, binstart);
#endif

#if 0
    nahisto = numaMakeHistogram(na, 1000, &binsize, NULL);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(0, binsize, nbins);
    fprintf(stderr, " binsize = %d, binstart = %d\n", binsize, 0);
#endif

    numaWrite(fileout, nahisto);

    gplot = gplotCreate(nax, nahisto, "junkroot", GPLOT_X11, GPLOT_LINES,
	    "example histo", "sine",
	    "i", "histo[i]");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);

    numaDestroy(&na);
    numaDestroy(&nax);
    numaDestroy(&nahisto);
    exit(0);
}



