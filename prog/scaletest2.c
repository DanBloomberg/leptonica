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
 * scaletest2.c
 *
 *   Tests scale-to-gray and color scaling
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
PIX         *pixs, *pixt, *pixd;
l_int32      i;
l_float32    scale;
char        *filein, *fileout;
static char  mainName[] = "scaletest2";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  scaletest2 filein fileout",
	                 mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
        /* Integer scale-to-gray functions */
#if 0
    pixd = pixScaleToGray2(pixs);
    pixWrite("junkout2x", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixd = pixScaleToGray3(pixs);
    pixWrite("junkout3x", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixd = pixScaleToGray4(pixs);
    pixWrite("junkout4x", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixd = pixScaleToGray6(pixs);
    pixWrite("junkout6x", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixd = pixScaleToGray8(pixs);
    pixWrite("junkout8x", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixd = pixScaleToGray16(pixs);
    pixWrite("junkout16x", pixd, IFF_PNG);
    pixDestroy(&pixd);
#endif

        /* Various non-integer scale-to-gray, compared with
	 * with different ways of getting similar results */
#if 0
    pixd = pixScaleToGray8(pixs);
    pixWrite(fileout, pixd, IFF_PNG);
    pixDestroy(&pixd);

    pixd = pixScaleToGray(pixs, 0.124);
    pixWrite("junkout124", pixd, IFF_PNG);
    pixDestroy(&pixd);

    pixd = pixScaleToGray(pixs, 0.284);
    pixWrite("junkout284", pixd, IFF_PNG);
    pixDestroy(&pixd);

    pixt = pixScaleToGray4(pixs);
    pixd = pixScaleBySampling(pixt, 284./250., 284./250.);
    pixWrite("junkout284.2", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixt = pixScaleToGray4(pixs);
    pixd = pixScaleGrayLI(pixt, 284./250., 284./250.);
    pixWrite("junkout284.3", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixt = pixScaleBinary(pixs, 284./250., 284./250.);
    pixd = pixScaleToGray4(pixt);
    pixWrite("junkout284.4", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixt = pixScaleToGray4(pixs);
    pixd = pixScaleGrayLI(pixt, 0.49, 0.49);
    pixWrite("junkout42", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixt = pixScaleToGray4(pixs);
    pixd = pixScaleSmooth(pixt, 0.49, 0.49);
    pixWrite("junkout4sm", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixt = pixScaleBinary(pixs, .16/.125, .16/.125);
    pixd = pixScaleToGray8(pixt);
    pixWrite("junkout16", pixd, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixd);

    pixd = pixScaleToGray(pixs, .16);
    pixWrite("junkout16.2", pixd, IFF_PNG);
    pixDestroy(&pixd);
#endif

        /* Antialiased (smoothed) reduction, along with sharpening */
#if 0
{
PIX *pixt1, *pixt2;

    startTimer();
    pixt1 = pixScaleSmooth(pixs, 0.154, 0.154);
    fprintf(stderr, "fast scale: %5.3f sec\n", stopTimer());
    pixDisplay(pixt1, 0, 0);
    pixWrite("junkout1", pixt1, IFF_PNG);
    pixt2 = pixUnsharpMask(pixt1, 1, 0.3);
    pixWrite("junkout2", pixt2, IFF_PNG);
    pixDisplay(pixt2, 200, 0);
}
#endif


        /* Test a large range of scale-to-gray reductions */
#if 1
    for (i = 2; i < 15; i++) {
        scale = 1. / (l_float32)i;
	startTimer();
        pixd = pixScaleToGray(pixs, scale);
	fprintf(stderr, "Time for scale %7.3f: %7.3f sec\n",
	        scale, stopTimer());
        pixDisplay(pixd, 75 * i, 100);
        pixDestroy(&pixd);
    }
    for (i = 8; i < 14; i++) {
        scale = 1. / (l_float32)(2 * i);
	startTimer();
        pixd = pixScaleToGray(pixs, scale);
	fprintf(stderr, "Time for scale %7.3f: %7.3f sec\n",
	        scale, stopTimer());
        pixDisplay(pixd, 100 * i, 600);
        pixDestroy(&pixd);
    }
#endif

        /* Test the same range of scale-to-gray mipmap reductions */
#if 0
    for (i = 2; i < 15; i++) {
        scale = 1. / (l_float32)i;
	startTimer();
        pixd = pixScaleToGrayMipmap(pixs, scale);
	fprintf(stderr, "Time for scale %7.3f: %7.3f sec\n",
	        scale, stopTimer());
        pixDisplay(pixd, 75 * i, 100);
        pixDestroy(&pixd);
    }
    for (i = 8; i < 12; i++) {
        scale = 1. / (l_float32)(2 * i);
	startTimer();
        pixd = pixScaleToGrayMipmap(pixs, scale);
	fprintf(stderr, "Time for scale %7.3f: %7.3f sec\n",
	        scale, stopTimer());
        pixDisplay(pixd, 100 * i, 600);
        pixDestroy(&pixd);
    }
#endif

        /* Test several methods for antialiased reduction,
	 * along with sharpening */
#if 0
{
PIX *pixt1, *pixt2, *pixt3, *pixt4, *pixt5, *pixt6, *pixt7;
l_float32 SCALING = 0.27;
l_int32   SIZE = 7;
l_int32   smooth;
l_float32 FRACT = 1.0;

    smooth = SIZE / 2;

    startTimer();
    pixt1 = pixScaleSmooth(pixs, SCALING, SCALING);
    fprintf(stderr, "fast scale: %5.3f sec\n", stopTimer());
    pixDisplay(pixt1, 0, 0);
    pixWrite("junkout1", pixt1, IFF_PNG);
    pixt2 = pixUnsharpMask(pixt1, 1, 0.3);
    pixDisplay(pixt2, 150, 0);

    startTimer();
    pixt3 = pixBlockconv(pixs, smooth, smooth);
    pixt4 = pixScaleBySampling(pixt3, SCALING, SCALING);
    fprintf(stderr, "slow scale: %5.3f sec\n", stopTimer());
    pixDisplay(pixt4, 200, 200);
    pixWrite("junkout4", pixt4, IFF_PNG);

    startTimer();
    pixt5 = pixUnsharpMask(pixs, smooth, FRACT);
    pixt6 = pixBlockconv(pixt5, smooth, smooth);
    pixt7 = pixScaleBySampling(pixt6, SCALING, SCALING);
    fprintf(stderr, "very slow scale + sharp: %5.3f sec\n", stopTimer());
    pixDisplay(pixt7, 500, 200);

    pixWrite(fileout, pixt7, IFF_JFIF_JPEG);

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
    pixDestroy(&pixt5);
    pixDestroy(&pixt6);
    pixDestroy(&pixt7);
    pixDestroy(&pixs);
}
#endif

        /* Test the color scaling function, comparing the
	 * special case of scaling factor 2.0 with the 
	 * general case. */
#if 0
    {
    PIX    *pix1, *pix2, *pixr, *pixg, *pixb;
    NUMA   *nared, *nagreen, *nablue, *naseq;
    GPLOT  *gplot;

    startTimer();
    pix1 = pixScaleColorLI(pixs, 2.00001, 2.0);
    fprintf(stderr, " Time with regular LI: %7.3f\n", stopTimer());
    pixWrite("junkcolor1", pix1, IFF_JFIF_JPEG);
    startTimer();
    pix2 = pixScaleColorLI(pixs, 2.0, 2.0);
    fprintf(stderr, " Time with 2x LI: %7.3f\n", stopTimer());
    pixWrite("junkcolor2", pix2, IFF_JFIF_JPEG);

    pixd = pixAbsDifference(pix1, pix2);
    pixr = pixGetRGBComponent(pixd, COLOR_RED);
    nared = pixGrayHistogram(pixr);
    pixg = pixGetRGBComponent(pixd, COLOR_GREEN);
    nagreen = pixGrayHistogram(pixg);
    pixb = pixGetRGBComponent(pixd, COLOR_BLUE);
    nablue = pixGrayHistogram(pixb);
    naseq = numaMakeSequence(0., 1., 256);
    gplot = gplotCreate(naseq, nared, "absdiff",
    GPLOT_X11, GPLOT_POINTS, "Number vs diff",
    "red", "diff", "number");
    gplotAddPlot(gplot, naseq, nagreen, GPLOT_POINTS, "green");
    gplotAddPlot(gplot, naseq, nablue, GPLOT_POINTS, "blue");
    gplotMakeOutput(gplot);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pixr);
    pixDestroy(&pixg);
    pixDestroy(&pixb);
    pixDestroy(&pixd);
    numaDestroy(&naseq);
    numaDestroy(&nared);
    numaDestroy(&nagreen);
    numaDestroy(&nablue);
    gplotDestroy(&gplot);
    }
#endif

    pixDestroy(&pixs);
    exit(0);
}

