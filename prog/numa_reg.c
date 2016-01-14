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
 * numa_reg.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"


#define   DO_ALL     1


main(int    argc,
     char **argv)
{
l_int32      i, n, binsize, binstart, nbins;
l_float32    pi, val, angle, xval, yval, x0, y0, rank;
GPLOT       *gplot;
NUMA        *na, *nahisto, *nax, *nay, *nap, *nasx, *nasy;
NUMA        *nadx, *nady, *nafx, *nafy;
PIX         *pixs;
static char  mainName[] = "numa_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  numa_reg", mainName, 1));

#if  DO_ALL 
    pi = 3.1415926535;
    na = numaCreate(5000);
    for (i = 0; i < 500000; i++) {
        angle = 0.02293 * i * pi;
	val = (l_float32)(999. * sin(angle));
	numaAddNumber(na, val);
    }

    nahisto = numaMakeHistogramClipped(na, 6, 2000);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(0, 1, nbins);
    gplot = gplotCreate("junkroot1", GPLOT_X11, "example histo 1",
	    "i", "histo[i]");
    gplotAddPlot(gplot, nax, nahisto, GPLOT_LINES, "sine");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&nax);
    numaDestroy(&nahisto);

    nahisto = numaMakeHistogram(na, 1000, &binsize, &binstart);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(binstart, binsize, nbins);
    fprintf(stderr, " binsize = %d, binstart = %d\n", binsize, binstart);
    gplot = gplotCreate("junkroot2", GPLOT_X11, "example histo 2",
	    "i", "histo[i]");
    gplotAddPlot(gplot, nax, nahisto, GPLOT_LINES, "sine");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&nax);
    numaDestroy(&nahisto);

    nahisto = numaMakeHistogram(na, 1000, &binsize, NULL);
    nbins = numaGetCount(nahisto);
    nax = numaMakeSequence(0, binsize, nbins);
    fprintf(stderr, " binsize = %d, binstart = %d\n", binsize, 0);
    gplot = gplotCreate("junkroot3", GPLOT_X11, "example histo 3",
	    "i", "histo[i]");
    gplotAddPlot(gplot, nax, nahisto, GPLOT_LINES, "sine");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&nax);
    numaDestroy(&nahisto);
    numaDestroy(&na);
#endif

#if  DO_ALL
        /* Test numaInterpolateEqxInterval() */
    pixs = pixRead("test8.jpg");
    na = pixGetGrayHistogramMasked(pixs, NULL, 0, 0, 1);
/*    numaWriteStream(stderr, na); */
    nasy = numaGetPartialSums(na);
    gplotSimple1(nasy, GPLOT_X11, "junkroot4", "partial sums");
    gplotSimple1(na, GPLOT_X11, "junkroot5", "simple test");
    numaInterpolateEqxInterval(0.0, 1.0, na, L_LINEAR_INTERP,
		            0.0, 255.0, 15, &nax, &nay);
    gplot = gplotCreate("junkroot6", GPLOT_X11, "test interpolation",
		    "pix val", "num pix");
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "plot 1");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&na);
    numaDestroy(&nasy);
    numaDestroy(&nax);
    numaDestroy(&nay);
    pixDestroy(&pixs);
#endif

#if  DO_ALL
        /* Test numaInterpolateArbxInterval() */
    pixs = pixRead("test8.jpg");
    na = pixGetGrayHistogramMasked(pixs, NULL, 0, 0, 1);
    nasy = numaGetPartialSums(na);
    numaInsertNumber(nasy, 0, 0.0);
    nasx = numaMakeSequence(0.0, 1.0, 257);
/*    gplotSimple1(nasy, GPLOT_X11, "junknasy", "partial sums"); */
    numaInterpolateArbxInterval(nasx, nasy, L_LINEAR_INTERP,
		            10.0, 250.0, 23, &nax, &nay);
    gplot = gplotCreate("junkroot7", GPLOT_X11, "arbx interpolation",
		    "pix val", "cum num pix");
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "plot 1");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&na);
    numaDestroy(&nasx);
    numaDestroy(&nasy);
    numaDestroy(&nax);
    numaDestroy(&nay);
    pixDestroy(&pixs);
#endif

#if  DO_ALL
        /* Test numaInterpolateArbxVal() */
    pixs = pixRead("test8.jpg");
    na = pixGetGrayHistogramMasked(pixs, NULL, 0, 0, 1);
    nasy = numaGetPartialSums(na);
    numaInsertNumber(nasy, 0, 0.0);
    nasx = numaMakeSequence(0.0, 1.0, 257);
/*    gplotSimple1(nasy, GPLOT_X11, "junknasy", "partial sums"); */
    nax = numaMakeSequence(15.0, (250.0 - 15.0) / 23.0, 24);
    n = numaGetCount(nax);
    nay = numaCreate(n);
    for (i = 0; i < n; i++) {
        numaGetFValue(nax, i, &xval);
        numaInterpolateArbxVal(nasx, nasy, L_QUADRATIC_INTERP, xval, &yval);
	numaAddNumber(nay, yval);
    }
    gplot = gplotCreate("junkroot8", GPLOT_X11, "arbx interpolation",
		    "pix val", "cum num pix");
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "plot 1");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&na);
    numaDestroy(&nasx);
    numaDestroy(&nasy);
    numaDestroy(&nax);
    numaDestroy(&nay);
    pixDestroy(&pixs);
#endif

#if  DO_ALL 
        /* Test interpolation */
    nasx = numaRead("testangle.numa");
    nasy = numaRead("testscore.numa");
    gplot = gplotCreate("junkroot9", GPLOT_X11, "arbx interpolation",
		    "angle", "score");
    numaInterpolateArbxInterval(nasx, nasy, L_LINEAR_INTERP,
		                -2.00, 0.0, 50, &nax, &nay);
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "linear");
    numaDestroy(&nax);
    numaDestroy(&nay);
    numaInterpolateArbxInterval(nasx, nasy, L_QUADRATIC_INTERP,
		                -2.00, 0.0, 50, &nax, &nay);
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "quadratic");
    numaDestroy(&nax);
    numaDestroy(&nay);
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    gplot = gplotCreate("junkroot10", GPLOT_X11, "arbx interpolation",
		    "angle", "score");
    numaInterpolateArbxInterval(nasx, nasy, L_LINEAR_INTERP,
		                -1.2, -0.8, 50, &nax, &nay);
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "quadratic");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaFitMax(nay, &yval, nax, &xval);
    fprintf(stderr, "max = %f at loc = %f\n", yval, xval);
    numaDestroy(&nasx);
    numaDestroy(&nasy);
    numaDestroy(&nax);
    numaDestroy(&nay);
#endif

#if  DO_ALL 
        /* Test integration and differentiation */
    nasx = numaRead("testangle.numa");
    nasy = numaRead("testscore.numa");
        /* ---------- Plot the derivative ---------- */
    numaDifferentiateInterval(nasx, nasy, -2.0, 0.0, 50, &nadx, &nady);
    gplot = gplotCreate("junkroot11", GPLOT_X11, "derivative",
		    "angle", "slope");
    gplotAddPlot(gplot, nadx, nady, GPLOT_LINES, "derivative");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
        /*  ---------- Plot the original function ----------- */
        /*  and the integral of the derivative; the two       */
        /*  should be approximately the same.                 */
    gplot = gplotCreate("junkroot12", GPLOT_X11, "integ-diff",
		        "angle", "val");
    numaInterpolateArbxInterval(nasx, nasy, L_LINEAR_INTERP,
		                -2.00, 0.0, 50, &nafx, &nafy);
    gplotAddPlot(gplot, nafx, nafy, GPLOT_LINES, "function");
    n = numaGetCount(nadx);
    numaGetFValue(nafx, 0, &x0);
    numaGetFValue(nafy, 0, &y0);
    nay = numaCreate(n);
        /* (Note: this tests robustness of the integrator: we go from
         * i = 0, and choose to have only 1 point in the interpolation
         * there, which is too small and causes the function to bomb out.) */
    for (i = 0; i < n; i++) {
        numaGetFValue(nadx, i, &xval);
        numaIntegrateInterval(nadx, nady, x0, xval, 2 * i + 1, &yval);
        numaAddNumber(nay, y0 + yval);
    }
    fprintf(stderr, "It's required to get a 'npts < 2' error here!\n");
    gplotAddPlot(gplot, nafx, nay, GPLOT_LINES, "anti-derivative");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&nasx);
    numaDestroy(&nasy);
    numaDestroy(&nafx);
    numaDestroy(&nafy);
    numaDestroy(&nadx);
    numaDestroy(&nady);
    numaDestroy(&nay);
#endif

#if  DO_ALL 
        /* Rank extraction with interpolation */
    pixs = pixRead("test8.jpg");
    nasy= pixGetGrayHistogramMasked(pixs, NULL, 0, 0, 1);
    numaMakeRankFromHistogram(0.0, 1.0, nasy, 350, &nax, &nay);
    gplot = gplotCreate("junkroot13", GPLOT_X11, "test rank extractor",
                        "pix val", "rank val");
    gplotAddPlot(gplot, nax, nay, GPLOT_LINES, "plot 1");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    numaDestroy(&nasy);
    numaDestroy(&nax);
    numaDestroy(&nay);
    pixDestroy(&pixs);
#endif

#if  DO_ALL 
        /* Rank extraction, point by point */
    pixs = pixRead("test8.jpg");
    nap = numaCreate(200);
    pixGetRankValueMasked(pixs, NULL, 0, 0, 2, 0.0, &val, &na);
    for (i = 0; i < 101; i++) {
      rank = 0.01 * i;
      numaHistogramGetValFromRank(na, 0, 1, rank, &val);
      numaAddNumber(nap, val);
    }
    gplotSimple1(nap, GPLOT_X11, "junkroot14", "rank value");
    numaDestroy(&na);
    numaDestroy(&nap);
    pixDestroy(&pixs);
#endif

    return 0;
}
