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
 * binmazetest.c
 *
 *    Generates and traverses maze with breadth-first algorithms
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   WIDTH     200
#define   HEIGHT    200
#define   XINIT     20
#define   YINIT     20
#define   XEND      170
#define   YEND      170
#define   WALLPS    0.65
#define   RANIS     0.25
/*  #define   RANIS     0.35 */   /* no path found */


main(int    argc,
     char **argv)
{
char        *mazeout, *pathout;
PIX         *pixm, *pixex, *pixd;
PTA         *pta;
static char  mainName[] = "binmazetest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  binmazetest mazeout pathout", mainName, 1));

    mazeout = argv[1];
    pathout = argv[2];

    pixm = generateBinaryMaze(WIDTH, HEIGHT, XINIT, YINIT, WALLPS, RANIS);
    pixex = pixExpandBinaryPower2(pixm, 4);
    pixDisplay(pixex, 50, 50);
    pixDestroy(&pixex);
    pixWrite(mazeout, pixm, IFF_PNG);

    pta = searchBinaryMaze(pixm, XINIT, YINIT, XEND, YEND, NULL);
    pixd = pixDisplayPta(pixm, pta);
    pixex = pixScaleBySampling(pixd, 4., 4.);
    pixDisplay(pixex, 450, 50);
    pixDestroy(&pixex);
    pixWrite(pathout, pixd, IFF_PNG);

    pixDestroy(&pixm);
    pixDestroy(&pixd);
    ptaDestroy(&pta);
    exit(0);
}

