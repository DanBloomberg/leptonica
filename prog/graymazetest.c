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
 *  graymazetest.c
 *
 *    Finds the least-cost path using a breadth-first algorithm
 *    between two points on a grayscale image.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  NPATHS     6
static const l_int32 xinit[NPATHS] = {42, 73, 73, 42, 324, 471};
static const l_int32 yinit[NPATHS] = {117, 319, 319, 117, 170, 201};
static const l_int32 xend[NPATHS] = {419, 419, 233, 326, 418, 128};
static const l_int32 yend[NPATHS] = {383, 383, 112, 168, 371, 341};

static const l_int32  XINIT = 42;
static const l_int32  YINIT = 117;
static const l_int32  XEND  = 419;
static const l_int32  YEND  = 383;


main(int    argc,
     char **argv)
{
char        *mazein, *pathout;
l_int32      i, w, h;
PIX         *pixex, *pix, *pixs, *pixd;
PTA         *pta;
PTAA        *ptaa;
static char  mainName[] = "graymazetest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  graymazetest mazein pathout", mainName, 1));

    mazein = argv[1];
    pathout = argv[2];

    if ((pix = pixRead(mazein)) == NULL)
	exit(ERROR_INT("pixs not read", mainName, 1));
    if (pixGetDepth(pix) != 8)
        pixs = pixConvertTo8(pix, FALSE);
    else
        pixs = pixClone(pix);
    pixGetDimensions(pixs, &w, &h, NULL);

#if 1  /* multiple paths */ 

    ptaa = ptaaCreate(NPATHS);
    for (i = 0; i < NPATHS; i++) {
        if (xinit[i] >= w || xend[i] >= w || yinit[i] >= h || yend[i] >= h) {
            fprintf(stderr, "path %d extends beyond image; skipping\n", i);
            continue;
        }
        pta = searchGrayMaze(pixs, xinit[i], yinit[i], xend[i], yend[i],
	                     NULL);
        ptaaAddPta(ptaa, pta, L_INSERT);
    }

    pixd = pixDisplayPtaa(pixs, ptaa);
    pixex = pixScaleBySampling(pixd, 4., 4.);
    pixDisplay(pixex, 450, 50);
    pixWrite(pathout, pixd, IFF_PNG);
    ptaaDestroy(&ptaa);

#else  /* one path */

    pta = searchGrayMaze(pixs, XINIT, YINIT, XEND, YEND, &pixd);
    pixd = pixDisplayPta(pixs, pta);
    pixex = pixScaleBySampling(pixd, 4., 4.);
    pixDisplay(pixex, 450, 50);
    pixWrite(pathout, pixd, IFF_PNG);
    ptaDestroy(&pta);

#endif

    pixDestroy(&pixs);
    pixDestroy(&pixd);
    pixDestroy(&pixex);
    exit(0);
}

