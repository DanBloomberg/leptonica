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
 * projectivetest.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


#if 1   /* for test with large distortion */
#define   X1        32
#define   Y1       150 
#define   X2       520
#define   Y2       150
#define   X3        32 
#define   Y3       612 
#define   X4       520
#define   Y4       612 

#define   X1P       32 
#define   Y1P      150
#define   X2P      520
#define   Y2P       44
#define   X3P       32
#define   Y3P      612
#define   X4P      520
#define   Y4P      694
#endif



main(int    argc,
     char **argv)
{
l_int32      d, h;
char        *filein, *fileout;
l_float32    rat;
l_float32   *vc;
PIX         *pix, *pixs, *pixx, *pixg, *pixgx, *pixgi;
PTA         *ptas, *ptad;
static char  mainName[] = "projectivetest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  projectivetest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

#if 1   /* test with large distortion */
    if ((pix = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    h = pixGetHeight(pix);
    rat = 850. / h;
/*    pixs = pixScale(pix, rat, rat); */
    pixs = pixClone(pix);
    ptas = ptaCreate(4);
    ptaAddPt(ptas, X1, Y1);
    ptaAddPt(ptas, X2, Y2);
    ptaAddPt(ptas, X3, Y3);
    ptaAddPt(ptas, X4, Y4);
    ptad = ptaCreate(4);
    ptaAddPt(ptad, X1P, Y1P);
    ptaAddPt(ptad, X2P, Y2P);
    ptaAddPt(ptad, X3P, Y3P);
    ptaAddPt(ptad, X4P, Y4P);

    startTimer();
    pixx = pixProjectiveSampled(pixs, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixProjectiveSampled(): %6.2f sec\n",
           stopTimer());
    pixDisplay(pixx, 0, 0);
    d = pixGetDepth(pixs);
    if (d == 1)
        pixWrite("junkout1", pixx, IFF_PNG);
    else
        pixWrite("junkout1", pixx, IFF_JFIF_JPEG);

    if (d == 1)
	pixg = pixScaleToGray3(pixs);
    else
        pixg = pixClone(pixs);

    startTimer();
    pixgx = pixProjectiveSampled(pixg, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixProjectiveSampled(): %6.2f sec\n",
           stopTimer());
    pixDisplay(pixgx, 300, 0);
    pixWrite("junkout2", pixgx, IFF_JFIF_JPEG);

    startTimer();
    pixgi = pixProjectiveInterpolated(pixg, ptas, ptad, L_BRING_IN_WHITE);
    fprintf(stderr, " Time for pixProjectiveInterpolated(): %6.2f sec\n",
           stopTimer());
    pixDisplay(pixgi, 600, 0);
    pixWrite("junkout3", pixgi, IFF_JFIF_JPEG);

    pixDestroy(&pix);
    pixDestroy(&pixs);
    pixDestroy(&pixx);
    pixDestroy(&pixg);
    pixDestroy(&pixgx);
    pixDestroy(&pixgi);
    ptaDestroy(&ptas);
    ptaDestroy(&ptad);
#endif

    exit(0);
}

