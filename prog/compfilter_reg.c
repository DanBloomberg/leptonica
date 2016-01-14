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
 *  compfilter_reg.c
 *
 *     Tests filters that select components based on size, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static void count_pieces(PIX  *pix, l_int32 nexp);
static void count_pieces2(BOXA *boxa, l_int32 nexp);


main(int    argc,
     char **argv)
{
BOX         *box1, *box2, *box3, *box4;
BOXA        *boxa, *boxat;
PIX         *pixs, *pixt, *pixd;
static char  mainName[] = "compfilter_reg";

        /* Draw 4 filled boxes of different sizes */
    pixs = pixCreate(200, 200, 1);
    box1 = boxCreate(10, 10, 20, 30);
    box2 = boxCreate(50, 10, 40, 20);
    box3 = boxCreate(110, 10, 35, 5);
    box4 = boxCreate(160, 10, 5, 15);
    boxa = boxaCreate(4);
    boxaAddBox(boxa, box1, L_INSERT);
    boxaAddBox(boxa, box2, L_INSERT);
    boxaAddBox(boxa, box3, L_INSERT);
    boxaAddBox(boxa, box4, L_INSERT);
    pixRenderBox(pixs, box1, 1, L_SET_PIXELS);
    pixRenderBox(pixs, box2, 1, L_SET_PIXELS);
    pixRenderBox(pixs, box3, 1, L_SET_PIXELS);
    pixRenderBox(pixs, box4, 1, L_SET_PIXELS);
    pixt = pixFillClosedBorders(pixs, 4);
    pixDisplayWrite(pixt, 1);

        /* Exercise the parameters */
    pixd = pixSelectBySize(pixt, 0, 22, 8, L_SELECT_HEIGHT,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 1);
    pixd = pixSelectBySize(pixt, 0, 30, 8, L_SELECT_HEIGHT,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 3);
    pixd = pixSelectBySize(pixt, 0, 5, 8, L_SELECT_HEIGHT,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 3);
    pixd = pixSelectBySize(pixt, 0, 6, 8, L_SELECT_HEIGHT,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 1);
    pixd = pixSelectBySize(pixt, 20, 0, 8, L_SELECT_WIDTH,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectBySize(pixt, 31, 0, 8, L_SELECT_WIDTH,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectBySize(pixt, 21, 10, 8, L_SELECT_IF_EITHER,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 3);
    pixd = pixSelectBySize(pixt, 20, 30, 8, L_SELECT_IF_EITHER,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectBySize(pixt, 22, 32, 8, L_SELECT_IF_BOTH,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectBySize(pixt, 6, 32, 8, L_SELECT_IF_BOTH,
                           L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 1);
    pixd = pixSelectBySize(pixt, 5, 25, 8, L_SELECT_IF_BOTH,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 1);
    pixd = pixSelectBySize(pixt, 25, 5, 8, L_SELECT_IF_BOTH,
                           L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 1);
    pixd = pixSelectByAreaPerimRatio(pixt, 1.7, 8, L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectByAreaPerimRatio(pixt, 5.5, 8, L_SELECT_IF_LT, NULL);
    count_pieces(pixd, 3);
    pixd = pixSelectByAreaPerimRatio(pixt, 1.5, 8, L_SELECT_IF_GTE, NULL);
    count_pieces(pixd, 2);
    pixd = pixSelectByAreaPerimRatio(pixt, 13.0/12.0, 8, L_SELECT_IF_GT, NULL);
    count_pieces(pixd, 3);

    boxat = boxaSelectBySize(boxa, 21, 10, L_SELECT_IF_EITHER,
                             L_SELECT_IF_LT, NULL);
    count_pieces2(boxat, 3);
    boxat = boxaSelectBySize(boxa, 22, 32, L_SELECT_IF_BOTH,
                             L_SELECT_IF_LT, NULL);
    count_pieces2(boxat, 2);

    system("/usr/bin/gthumb junk_write_display* &");

    boxaDestroy(&boxa);
    pixDestroy(&pixt);
    pixDestroy(&pixs);
    return 0;
}


void count_pieces(PIX  *pix, l_int32 nexp)
{
l_int32  n;
BOXA    *boxa;

    pixDisplayWrite(pix, 1);
    boxa = pixConnComp(pix, NULL, 8);
    n = boxaGetCount(boxa);
    if (n == nexp)
        fprintf(stderr, "Correct: Num. comps: %d\n", n);
    else
        fprintf(stderr, "WRONG!: Num. comps: %d\n", n);
    boxaDestroy(&boxa);
    pixDestroy(&pix);
}

void count_pieces2(BOXA  *boxa, l_int32 nexp)
{
l_int32  n;

    n = boxaGetCount(boxa);
    if (n == nexp)
        fprintf(stderr, "Correct: Num. boxes: %d\n", n);
    else
        fprintf(stderr, "WRONG!: Num. boxes: %d\n", n);
    boxaDestroy(&boxa);
}


