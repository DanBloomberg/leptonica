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

#define  BINARY_IMAGE             "feyn.tif"
#define  TWO_BPP_IMAGE_NO_CMAP    "weasel2.png"
#define  TWO_BPP_IMAGE_CMAP       "weasel2.4c.png"
#define  FOUR_BPP_IMAGE_NO_CMAP   "weasel4.png"
#define  FOUR_BPP_IMAGE_CMAP      "weasel4.16c.png"
#define  EIGHT_BPP_IMAGE_NO_CMAP  "weasel8.png"
#define  EIGHT_BPP_IMAGE_CMAP     "weasel8.240c.png"
#define  RGB_IMAGE                "marge.jpg"

static const l_int32 SPACE = 30;

void PixSave32(PIXA *pixa, PIX *pixc);


main(int    argc,
     char **argv)
{
PIX   *pixs, *pixc, *pixd;
PIXA  *pixa;
static char  mainName[] = "scale_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  scale_reg", mainName, 1));

    pixa = pixaCreate(0);

        /* Test 1 bpp */
    pixs = pixRead(BINARY_IMAGE);
    pixc = pixScale(pixs, 0.32, 0.32);
    pixSaveTiled(pixc, pixa, 1, 1, SPACE, 32);
    pixDestroy(&pixc);
    pixc = pixScaleToGray3(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray4(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray6(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray8(pixs);
    PixSave32(pixa, pixc);
    pixc = pixScaleToGray16(pixs);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 2 bpp without colormap */
    pixs = pixRead(TWO_BPP_IMAGE_NO_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 2.25, 2.25);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 2 bpp with colormap */
    pixs = pixRead(TWO_BPP_IMAGE_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 2.25, 2.25);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 4 bpp without colormap */
    pixs = pixRead(FOUR_BPP_IMAGE_NO_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.72, 1.72);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 4 bpp with colormap */
    pixs = pixRead(FOUR_BPP_IMAGE_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.72, 1.72);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 8 bpp without colormap */
    pixs = pixRead(EIGHT_BPP_IMAGE_NO_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.92, 1.92);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 8 bpp with colormap */
    pixs = pixRead(EIGHT_BPP_IMAGE_CMAP);
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixScale(pixs, 1.92, 1.92);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.85, 0.85);
    PixSave32(pixa, pixc);
    pixc = pixScale(pixs, 0.65, 0.65);
    PixSave32(pixa, pixc);
    pixDestroy(&pixs);

        /* Test 32 bpp */
    pixs = pixRead(RGB_IMAGE);
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


void PixSave32(PIXA *pixa, PIX *pixc) {
PIX  *pix32;
    pix32 = pixConvertTo32(pixc);
    pixSaveTiled(pix32, pixa, 1, 0, SPACE, 0);
    pixDestroy(&pixc);
    pixDestroy(&pix32);
}


