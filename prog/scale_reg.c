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
 * scale_reg.c
 *
 *      This tests a number of scaling operations, through the pixScale()
 *      interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static char *image[9] = {"feyn.tif",         /* 1 bpp */
                         "weasel2.png",      /* 2 bpp; no cmap */
                         "weasel2.4c.png",   /* 2 bpp; cmap */
                         "weasel4.png",      /* 4 bpp; no cmap */
                         "weasel4.16c.png",  /* 4 bpp; cmap */
                         "weasel8.png",      /* 8 bpp; no cmap */
                         "weasel8.240c.png", /* 8 bpp; cmap */
                         "marge.jpg",        /* 32 bpp rgb */
                         "test24.jpg"};      /* 32 bpp rgb */


static const l_int32 SPACE = 30;
static const l_int32 WIDTH = 300;
static const l_float32 FACTOR[5] = {2.3, 1.5, 1.1, 0.6, 0.3};

static void PixSave32(PIXA *pixa, PIX *pixc);
static void AddScaledImages(PIXA *pixa, char *fname, l_int32 width);


main(int    argc,
     char **argv)
{
l_int32  i;
PIX     *pixs, *pixc, *pixd;
PIXA    *pixa;
static char  mainName[] = "scale_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  scale_reg", mainName, 1));

    pixa = pixaCreate(0);

        /* Test 1 bpp */
    pixs = pixRead(image[0]);
    pixc = pixScale(pixs, 0.32, 0.32);
    pixSaveTiled(pixc, pixa, 1, 1, SPACE, 32);
    pixDestroy(&pixc);
    pixc = pixScaleToGray3(pixs);
    PixSave32(pixa, pixc);

    pixc = pixScaleToGray4(pixs);
    pixSaveTiled(pixc, pixa, 1, 1, SPACE, 32);
    pixDestroy(&pixc);
    pixc = pixScaleToGray6(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray8(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray16(pixs);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

    for (i = 1; i < 9; i++) {
/*        if (i != 2) continue; */
        AddScaledImages(pixa, image[i], WIDTH);
    }

        /* Test 2 bpp without colormap */
    pixs = pixRead(image[1]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 2.25, 2.25);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 2 bpp with colormap */
    pixs = pixRead(image[2]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 2.25, 2.25);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 4 bpp without colormap */
    pixs = pixRead(image[3]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.72, 1.72);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 4 bpp with colormap */
    pixs = pixRead(image[4]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.72, 1.72);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 8 bpp without colormap */
    pixs = pixRead(image[5]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.92, 1.92);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 8 bpp with colormap */
    pixs = pixRead(image[6]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.92, 1.92);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 32 bpp */
    pixs = pixRead(image[7]);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.42, 1.42);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkscale.jpg", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    return 0;
}

static void
AddScaledImages(PIXA    *pixa,
                char    *fname,
                l_int32  width)
{
l_int32    i, w;
l_float32  scalefactor;
PIX       *pixs, *pixt1, *pixt2, *pix32;

    pixs = pixRead(fname);
    w = pixGetWidth(pixs);
    for (i = 0; i < 5; i++) {
        scalefactor = (l_float32)width / (FACTOR[i] * (l_float32)w);
        pixt1 = pixScale(pixs, FACTOR[i], FACTOR[i]);
        pixt2 = pixScale(pixt1, scalefactor, scalefactor);
        pix32 = pixConvertTo32(pixt2);
        if (i == 0)
            pixSaveTiled(pix32, pixa, 1, 1, SPACE, 0);
        else
            pixSaveTiled(pix32, pixa, 1, 0, SPACE, 0);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixDestroy(&pix32);
    }
    pixDestroy(&pixs);

    return;
}


static void
PixSave32(PIXA *pixa, PIX *pixc)
{
PIX  *pix32;
    pix32 = pixConvertTo32(pixc);
    pixSaveTiled(pix32, pixa, 1, 0, SPACE, 0);
    pixDestroy(&pixc);
    pixDestroy(&pix32);
}

