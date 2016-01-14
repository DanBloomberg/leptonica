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
 * enhance_reg.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const char *filein = "test24.jpg";
static const l_int32 WIDTH = 150;

main(int    argc,
     char **argv)
{
l_int32      w, h, d, i;
l_float32    scalefact, sat;
NUMA        *na;
PIX         *pix, *pixs, *pixt0, *pixt1, *pixd;
PIXA        *pixa, *pixaf;
static char  mainName[] = "enhance_reg";


    pix = pixRead(filein);
    pixGetDimensions(pix, &w, &h, &d);
    if (d != 32)
        return ERROR_INT("file not 32 bpp", mainName, 1);
    scalefact = (l_float32)WIDTH / (l_float32)w;
    pixs = pixScale(pix, scalefact, scalefact);
    w = pixGetWidth(pixs);
    pixaf = pixaCreate(5);

        /* TRC: vary gamma */
    pixa = pixaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixGammaTRC(NULL, pixs, 0.3 + 0.15 * i, 0, 255);
        pixaAddPix(pixa, pixt0, L_INSERT);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 32);
    pixWrite("junktrcgam.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 0, 100, "TRC Gamma", 1);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* TRC: vary black point */
    pixa = pixaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixGammaTRC(NULL, pixs, 1.0, 5 * i, 255);
        pixaAddPix(pixa, pixt0, L_INSERT);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 0);
    pixWrite("junktrc.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 300, 100, "TRC", 1);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* Vary hue */
    pixa = pixaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixModifyHue(NULL, pixs, 0.01 + 0.05 * i);
        pixaAddPix(pixa, pixt0, L_INSERT);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 0);
    pixWrite("junkhue.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 600, 100, "Hue", 1);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* Vary saturation */
    pixa = pixaCreate(20);
    na = numaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixModifySaturation(NULL, pixs, -0.9 + 0.1 * i);
        pixMeasureSaturation(pixt0, 1, &sat);
        pixaAddPix(pixa, pixt0, L_INSERT);
        numaAddNumber(na, sat);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 0);
    gplotSimple1(na, GPLOT_X11, "junkplot", "Average Saturation");
    pixWrite("junksat.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 900, 100, "Saturation", 1);
    numaDestroy(&na);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* Vary contrast */
    pixa = pixaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixContrastTRC(NULL, pixs, 0.1 * i);
        pixaAddPix(pixa, pixt0, L_INSERT);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 0);
    pixWrite("junkcontrast.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 0, 400, "Contrast", 1);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* Vary sharpening */
    pixa = pixaCreate(20);
    for (i = 0; i < 20; i++) {
        pixt0 = pixUnsharpMasking(pixs, 3, 0.01 + 0.15 * i);
        pixaAddPix(pixa, pixt0, L_INSERT);
    }
    pixt1 = pixaDisplayTiledAndScaled(pixa, 32, w, 5, 0, 10, 2);
    pixSaveTiled(pixt1, pixaf, 1, 1, 20, 0);
    pixWrite("junksharp.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 300, 400, "Sharp", 1);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

        /* Display results */
    pixd = pixaDisplay(pixaf, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkenhance.jpg", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixaf);

    pixDestroy(&pix);
    pixDestroy(&pixs);
    return 0;
}


