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
 * blendcmaptest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  NX     3
#define  NY     4

#define  FADE_FRACTION    0.85

main(int    argc,
     char **argv)
{
l_uint8    rval, gval, bval;
l_int32    n, i, j, sindex, wm, hm, ws, hs, delx, dely, x, y, y0;
PIX       *pixs, *pixm, *pixb, *pixt0, *pixt1;
PIXCMAP   *cmap;
static char   mainName[] = "blendcmaptest";

    pixs = pixRead("rabi.png");
    pixb = pixRead("weasel4.11c.png");

        /* fade the blender */
    pixcmapShiftIntensity(pixGetColormap(pixb), 0.75);

        /* downscale the input */
    wm = pixGetWidth(pixm);
    hm = pixGetHeight(pixm);
    pixt0 = pixScaleToGray4(pixs);

        /* threshold to 5 levels, 4 bpp */
    ws = pixGetWidth(pixt0);
    hs = pixGetHeight(pixt0);
    pixWrite("junkt0", pixt0, IFF_JFIF_JPEG);
    pixt1 = pixThresholdTo4bpp(pixt0, 5, 1);
    pixWrite("junkt1", pixt1, IFF_PNG);
    cmap = pixGetColormap(pixt1);
/*    pixcmapWriteStream(stderr, cmap); */
    pixcmapGetIndex(cmap, 255, 255, 255, &sindex);  /* overwrite white pixels */
    delx = ws / NX;
    dely = hs / NY;
    for (i = 0; i < NY; i++) {
        y = 20 + i * dely;
        if (y >= hs + hm)
            continue;
        for (j = 0; j < NX; j++) {
            x = 30 + j * delx;
            y0 = y;
            if (j % 2 == 1) {
                y0 = y + dely / 2;
                if (y >= hs + hm)
                    continue;
            }
            if (x >= ws + wm)
                continue;
            pixBlendCmap(pixt1, pixb, x, y0, sindex);
        }
    }
    pixWrite("junkt2", pixt1, IFF_PNG);

    pixDestroy(&pixs);
    pixDestroy(&pixb);
    pixDestroy(&pixt0);
    pixDestroy(&pixt1);
    exit(0);
}

