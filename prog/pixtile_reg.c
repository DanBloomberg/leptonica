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
 *   pixtile_reg.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static l_int32 TestTiling(PIX *pixd, PIX *pixs, l_int32 nx, l_int32 ny,
                          l_int32 w, l_int32 h, l_int32 overlap);


main(int    argc,
     char **argv)
{
PIX         *pixs, *pixd;

    pixs = pixRead("test24.jpg");
    pixd = pixCreateTemplateNoInit(pixs);

    TestTiling(pixd, pixs, 1, 1, 0, 0, 183);
    TestTiling(pixd, pixs, 0, 1, 60, 0, 30);
    TestTiling(pixd, pixs, 1, 0, 0, 60, 40);
    TestTiling(pixd, pixs, 0, 0, 27, 31, 27);
    TestTiling(pixd, pixs, 0, 0, 400, 400, 40);
    TestTiling(pixd, pixs, 7, 9, 0, 0, 35);

    pixDestroy(&pixs);
    pixDestroy(&pixd);
    return 0;
}


l_int32
TestTiling(PIX     *pixd,
           PIX     *pixs,
           l_int32  nx,
           l_int32  ny,
           l_int32  w,
           l_int32  h,
           l_int32  overlap)
{
l_int32     i, j, same;
PIX        *pixt;
PIXTILING  *pt;

    pixClearAll(pixd);
    pt = pixTilingCreate(pixs, nx, ny, w, h, overlap);
    pixTilingGetCount(pt, &nx, &ny);
    pixTilingGetSize(pt, &w, &h);
    if (pt)
        fprintf(stderr, "nx = %d, ny = %d, w = %d, h = %d, overlap = %d\n",
                nx, ny, w, h, pt->overlap);
    else
        return 1;

    for (i = 0; i < ny; i++) {
        for (j = 0; j < nx; j++) {
            pixt = pixTilingGetTile(pt, i, j);
            pixTilingPaintTile(pixd, i, j, pixt, pt);
            pixDestroy(&pixt);
        }
    }
    pixEqual(pixs, pixd, &same);
    if (same)
        fprintf(stderr, "Tiling OK\n");
    else
        fprintf(stderr, "Tiling ERROR !\n");

    pixTilingDestroy(&pt);
    return 0;
}


