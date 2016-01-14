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
 * plotprog.c
 *
 *	      plotprog  rootout  outputtype
 *
 *                rootout: root name of generated gnuplot data and cmd files
 *                outputtype: one of {PNG, PS, EPS, X11}
 *
 *     This is an example program that uses the gplot library
 *     calls to make plots programmatically using gnuplot.
 *     To use it for your application, you must choose a setting
 *     of PLOT_STYLE, change the function that generates the
 *     data, and recompile.
 * 
 *     To use data from a file as input, the plotfile program
 *     implements a subset of gnuplot and has a simple, single
 *     file as input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

    /* for PLOT_STYLE, use one of the following set:
       {GPLOT_LINES, GPLOT_POINTS, GPLOT_LINESPOINTS, GPLOT_DOTS,
        GPLOT_IMPULSE} */
#define  PLOT_STYLE   GPLOT_LINES

main(int    argc,
     char **argv)
{
char        *rootout, *outputtype;
l_int32      outtype, i;
l_float32    x, y1, y2, pi;
GPLOT       *gplot;
NUMA        *nax, *nay1, *nay2;
static char  mainName[] = "plotprog";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  plotprog rootout outputtype",
	     mainName, 1));

    rootout = argv[1];
    outputtype = argv[2];

	/* convert input strings in set {PNG, PS, EPS, X11}
	 * to internal integer flags used by gplot */
    outtype = UNDEF;
    for (i = 0; i < NUM_GPLOT_OUTPUTS; i++) {
	if (strcmp(outputtype, gplotfileoutputs[i]) == 0) {
	    outtype = i;
	    break;
	}
    }
    if (outtype == UNDEF) {   /* default to PNG */
	fprintf(stderr,
	    "outputtype not in set {PNG, PS, EPS, X11}; using PNG\n");
	outtype = GPLOT_PNG;
    }

	/* define data here and store in numas */
    nax = numaCreate(0);
    nay1 = numaCreate(0);
    nay2 = numaCreate(0);
    pi = 3.1415926535;
    for (i = 0; i < 180; i++) {
	x = (pi / 180.) * i;
	y1 = (l_float32)sin(2.4 * x);
	y2 = (l_float32)cos(2.4 * x);
	numaAddNumber(nax, x);
	numaAddNumber(nay1, y1);
	numaAddNumber(nay2, y2);
    }

	/* plot it */
    gplot = gplotCreate(nax, nay1, rootout, outtype, PLOT_STYLE, 
                         "Example plots", "sin (2.4 * theta)",
			 "theta", "f(theta)");
    gplotAddPlot(gplot, nax, nay2, PLOT_STYLE, "cos (2.4 * theta)");
    gplotMakeOutput(gplot);

    numaDestroy(&nax);
    numaDestroy(&nay1);
    numaDestroy(&nay2);
    gplotDestroy(&gplot);
    exit(0);
}

