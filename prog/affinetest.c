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
 * affinetest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>


#include "allheaders.h"

#define   ADDED_BORDER_PIXELS       1000


#if 0   /* for symmetry test */
#define   X1       300
#define   Y1      1200
#define   X2      1200
#define   Y2      1100
#define   X3       200
#define   Y3       200

#define   X1P      500
#define   Y1P     1700
#define   X2P     1600
#define   Y2P     1800
#define   X3P      600
#define   Y3P      300
#endif

#if 0   /* for symmetry test */
#define   X1       300
#define   Y1      1200
#define   X2      1200
#define   Y2      1100
#define   X3       200
#define   Y3       200

#define   X1P      300
#define   Y1P     1400
#define   X2P     1400
#define   Y2P     1500
#define   X3P      200
#define   Y3P      300
#endif

#if 0   /* for symmetry test */
#define   X1       300
#define   Y1      1250
#define   X2      1125
#define   Y2      1100
#define   X3       200
#define   Y3       200

#define   X1P      350
#define   Y1P     1400
#define   X2P     1400
#define   Y2P     1500
#define   X3P      400
#define   Y3P      400
#endif

#if 0    /* comparison between sequential and sampling */
#define   X1        95
#define   Y1      2821
#define   X2      1432
#define   Y2      2682
#define   X3       232
#define   Y3       657

#define   X1P      117
#define   Y1P     2629
#define   X2P     1465
#define   Y2P     2432
#define   X3P      183
#define   Y3P      490
#endif

#if 1   /* for test with large distortion */
#define   X1        32
#define   Y1       934
#define   X2       487
#define   Y2       934
#define   X3        32 
#define   Y3        67 

#define   X1P       32 
#define   Y1P      934
#define   X2P      487
#define   Y2P      804 
#define   X3P       61
#define   Y3P       83
#endif



main(int    argc,
     char **argv)
{
l_int32      i, j, w, h, d, x, y, wpls;
l_uint32    *datas, *lines;
char        *filein, *fileout;
l_float32   *vc;
PIX         *pix, *pixs, *pixd, *pixt1, *pixt2, *pixc, *pixn;
PIX         *pixx, *pixg, *pixgx, *pixgi;
PTA         *ptas, *ptad;
static char  mainName[] = "affinetest";

    pixs = pixt1 = pixt2 = pixc = pixn = pixd = NULL;

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  affinetest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

#if 0   /* symmetry test: necessary but not sufficient */
    if ((pix = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));
    pixc = pixCopy(NULL, pix);
    pixs = pixAddBorder(pix, ADDED_BORDER_PIXELS, 0);

    ptas = ptaCreate(3);
    ptaAddPt(ptas, X1, Y1);
    ptaAddPt(ptas, X2, Y2);
    ptaAddPt(ptas, X3, Y3);
    ptad = ptaCreate(3);
    ptaAddPt(ptad, X1P, Y1P);
    ptaAddPt(ptad, X2P, Y2P);
    ptaAddPt(ptad, X3P, Y3P);

    pixt1 = pixAffineSequential(pixs, ptad, ptas, 0, 0);
    pixDisplay(pixt1, 20, 20);
    pixWrite("junkout1", pixt1, IFF_PNG);
    pixt2 = pixAffineSequential(pixt1, ptas, ptad, 0, 0);
    pixd = pixRemoveBorder(pixt2, ADDED_BORDER_PIXELS);

    pixDisplay(pixd, 50, 50);
    pixWrite(fileout, pixd, IFF_PNG);

    pixXor(pixd, pixd, pixc);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkout2", pixd, IFF_PNG);
    pixDestroy(&pixc);
    pixDestroy(&pixs);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixd);
    ptaDestroy(&ptas);
    ptaDestroy(&ptad);
#endif

#if 0    /* comparison between sequential and sampling */
    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
/*    pixs = pixAddBorder(pix, 150, 0); */

#if 1  /* use the rotated image and transform back */
    pixt1 = pixRotateShear(pixs, 450, 795, -0.033, L_BRING_IN_WHITE);
    pixVShearIP(pixt1, 680, -0.013, L_BRING_IN_WHITE);
    pixt2 = pixScale(pixt1, 1.012, 0.987);
    pixRasteropIP(pixt2, -47, -171, L_BRING_IN_WHITE);
    pixWrite("junkrot", pixt2, IFF_PNG);
#else   /* use the original image and compare quality of transforms */
    pixt2 = pixClone(pixs);
#endif

    ptas = ptaCreate(3);
    ptaAddPt(ptas, X1, Y1);
    ptaAddPt(ptas, X2, Y2);
    ptaAddPt(ptas, X3, Y3);
    ptad = ptaCreate(3);
    ptaAddPt(ptad, X1P, Y1P);
    ptaAddPt(ptad, X2P, Y2P);
    ptaAddPt(ptad, X3P, Y3P);

	/* do it by sequential transforms */
    startTimer();
    pixd = pixAffineSequential(pixt2, ptas, ptad,
                     ADDED_BORDER_PIXELS, ADDED_BORDER_PIXELS);
    fprintf(stderr,
        " Time for pixAffineSequential(): %6.2f sec\n", stopTimer());
    pixDisplay(pixd, 20, 20);
    pixWrite(fileout, pixd, IFF_PNG);

    pixXor(pixd, pixd, pixs);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkout1", pixd, IFF_PNG);

	/* do it by sampled transforms */
    startTimer();
    pixn = pixAffineSampled(pixt2, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixAffineSampled(): %6.2f sec\n", stopTimer());

    pixDisplay(pixn, 150, 150);
    pixWrite("junkout2", pixn, IFF_PNG);

    pixXor(pixn, pixn, pixs);
    pixDisplay(pixn, 200, 200);
    pixWrite("junkout3", pixn, IFF_PNG);
#endif

#if 1   /* test with large distortion */
    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    ptas = ptaCreate(3);
    ptaAddPt(ptas, X1, Y1);
    ptaAddPt(ptas, X2, Y2);
    ptaAddPt(ptas, X3, Y3);
    ptad = ptaCreate(3);
    ptaAddPt(ptad, X1P, Y1P);
    ptaAddPt(ptad, X2P, Y2P);
    ptaAddPt(ptad, X3P, Y3P);

    startTimer();
    pixx = pixAffineSampled(pixs, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixAffineSampled(): %6.2f sec\n", stopTimer());
    pixDisplay(pixx, 0, 0);
    d = pixGetDepth(pixs);
    if (d == 1)
        pixWrite("junkout1", pixx, IFF_PNG);
    else
        pixWrite("junkout1", pixx, IFF_JFIF_JPEG);

    d = pixGetDepth(pixs);
    if (d == 1)
	pixg = pixScaleToGray3(pixs);
    else
        pixg = pixClone(pixs);
    startTimer();
    pixgx = pixAffineSampled(pixg, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixAffineSampled(): %6.2f sec\n", stopTimer());
    pixDisplay(pixgx, 300, 0);
    pixWrite("junkout2", pixgx, IFF_PNG);

    startTimer();
    pixgi = pixAffineInterpolated(pixg, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixAffineInterpolated(): %6.2f sec\n",
           stopTimer());
    pixDisplay(pixgi, 600, 0);
    pixWrite("junkout3", pixgi, IFF_PNG);
    pixDestroy(&pixs);
    pixDestroy(&pixx);
    pixDestroy(&pixg);
    pixDestroy(&pixgx);
    pixDestroy(&pixgi);
    ptaDestroy(&ptas);
    ptaDestroy(&ptad);
#endif

	/* do it by sequential transforms */
#if 0  /* going the other way; sampling errors */
    pixn = pixCreateTemplate(pixs);
    w = pixGetWidth(pixn);
    h = pixGetHeight(pixn);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    affineXformCoeffs(ptas, ptad, &vc);
    fprintf(stderr, "c[0] = %f, c[1] = %f, c[2] = %f\n", vc[0], vc[1], vc[2]);
    fprintf(stderr, "c[3] = %f, c[4] = %f, c[5] = %f\n", vc[3], vc[4], vc[5]);
    affineXform(vc, X1, Y1, &x, &y);
    fprintf(stderr, "x = %d, X1P = %d, y = %d, Y1P = %d\n", x, X1P, y, Y1P);
    affineXform(vc, X2, Y2, &x, &y);
    fprintf(stderr, "x = %d, X2P = %d, y = %d, Y2P = %d\n", x, X2P, y, Y2P);
    affineXform(vc, X3, Y3, &x, &y);
    fprintf(stderr, "x = %d, X3P = %d, y = %d, Y3P = %d\n", x, X3P, y, Y3P);
    for (i = 0; i < h; i++) {
	lines = datas + i * wpls;
	for (j = 0; j < w; j++) {
	    if (GET_DATA_BIT(lines, j)) {
		affineXform(vc, j, i, &x, &y);
		if (x < 0 || y < 0 || x > w || y > h)
		    continue;
		pixSetPixel(pixn, x, y, 1);
	    }
	}
    }
#endif

#if 0
    pixDestroy(&pixs);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixd);
    pixDestroy(&pixn);
    ptaDestroy(&ptas);
    ptaDestroy(&ptad);
#endif

    exit(0);
}

