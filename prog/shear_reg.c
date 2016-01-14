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
 *   shear_reg.c
 *
 *    Regression test for shear, both IP and to new pix.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   BINARY_IMAGE              "test1.png"
#define   TWO_BPP_IMAGE             "weasel2.4c.png"
#define   FOUR_BPP_IMAGE1           "weasel4.11c.png"
#define   FOUR_BPP_IMAGE2           "weasel4.16g.png"
#define   EIGHT_BPP_IMAGE           "test8.jpg"
#define   EIGHT_BPP_CMAP_IMAGE1     "dreyfus8.png"
#define   EIGHT_BPP_CMAP_IMAGE2     "test24.jpg"
#define   RGB_IMAGE                 "marge.jpg"

void static shearTest(PIX *pixs, const char *filename, l_int32 reduction);

static const l_float32  ANGLE1 = 3.14159265 / 12.;


l_int32 main(int    argc,
             char **argv)
{
l_int32      index;
PIX         *pixs, *pixd;
PIXCMAP     *cmap;
static char  mainName[] = "shear_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  shear_reg", mainName, 1));

    fprintf(stderr, "Test binary image:\n");
    pixs = pixRead(BINARY_IMAGE);
    shearTest(pixs, "/tmp/junk1bpp.png", 1);
    pixDestroy(&pixs);

        /* We change the black to dark red so that we can see
         * that the IP shear does brings in that color.  It
         * can't bring in black because the cmap is filled. */
    fprintf(stderr, "Test 2 bpp cmapped image with filled cmap:\n");
    pixs = pixRead(TWO_BPP_IMAGE);
    cmap = pixGetColormap(pixs);
    pixcmapGetIndex(cmap, 40, 44, 40, &index);
    pixcmapResetColor(cmap, index, 100, 0, 0);
    shearTest(pixs, "/tmp/junk2bpp.png", 1);
    pixDestroy(&pixs);

    fprintf(stderr, "Test 4 bpp cmapped image with unfilled cmap:\n");
    pixs = pixRead(FOUR_BPP_IMAGE1);
    shearTest(pixs, "/tmp/junk4bpp1.png", 1);
    pixDestroy(&pixs);

    fprintf(stderr, "Test 4 bpp cmapped image with filled cmap:\n");
    pixs = pixRead(FOUR_BPP_IMAGE2);
    shearTest(pixs, "/tmp/junk4bpp2.png", 1);
    pixDestroy(&pixs);

    fprintf(stderr, "Test 8 bpp grayscale image:\n");
    pixs = pixRead(EIGHT_BPP_IMAGE);
    shearTest(pixs, "/tmp/junk8bpp1.png", 1);
    pixDestroy(&pixs);

    fprintf(stderr, "Test 8 bpp grayscale cmap image:\n");
    pixs = pixRead(EIGHT_BPP_CMAP_IMAGE1);
    shearTest(pixs, "/tmp/junk8bpp2.png", 1);
    pixDestroy(&pixs);

    fprintf(stderr, "Test 8 bpp color cmap image:\n");
    pixs = pixRead(EIGHT_BPP_CMAP_IMAGE2);
    pixd = pixOctreeColorQuant(pixs, 200, 0);
    shearTest(pixd, "/tmp/junk8bpp3.png", 4);
    pixDestroy(&pixs);
    pixDestroy(&pixd);

    fprintf(stderr, "Test rgb image:\n");
    pixs = pixRead(RGB_IMAGE);
    shearTest(pixs, "/tmp/junkrgb.png", 1);
    pixDestroy(&pixs);

    return 0;
}


void static
shearTest(PIX          *pixs,
          const char  *filename,
          l_int32      reduction)
{
l_int32   w, h, d;
PIX      *pixt1, *pixt2, *pixd;
PIXA     *pixa;

    PROCNAME("shearTest");

    pixa = pixaCreate(0);
    pixGetDimensions(pixs, &w, &h, &d);

    pixt1 = pixHShear(NULL, pixs, 0, ANGLE1, L_BRING_IN_WHITE);
    pixSaveTiled(pixt1, pixa, reduction, 1, 20, 32);
    pixt2 = pixHShear(NULL, pixs, h / 2, ANGLE1, L_BRING_IN_WHITE);
    pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixt1 = pixHShear(NULL, pixs, 0, ANGLE1, L_BRING_IN_BLACK);
    pixSaveTiled(pixt1, pixa, reduction, 0, 20, 0);
    pixt2 = pixHShear(NULL, pixs, h / 2, ANGLE1, L_BRING_IN_BLACK);
    pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    if (!pixGetColormap(pixs)) {
        pixt1 = pixCopy(NULL, pixs);
        pixHShearIP(pixt1, 0, ANGLE1, L_BRING_IN_WHITE);
        pixSaveTiled(pixt1, pixa, reduction, 1, 20, 0);
        pixt2 = pixCopy(NULL, pixs);
        pixHShearIP(pixt2, h / 2, ANGLE1, L_BRING_IN_WHITE);
        pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixt1 = pixCopy(NULL, pixs);
        pixHShearIP(pixt1, 0, ANGLE1, L_BRING_IN_BLACK);
        pixSaveTiled(pixt1, pixa, reduction, 0, 20, 0);
        pixt2 = pixCopy(NULL, pixs);
        pixHShearIP(pixt2, h / 2, ANGLE1, L_BRING_IN_BLACK);
        pixSaveTiled(pixt2, pixa, reduction, 0, 20, 32);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    pixt1 = pixVShear(NULL, pixs, 0, ANGLE1, L_BRING_IN_WHITE);
    pixSaveTiled(pixt1, pixa, reduction, 1, 20, 0);
    pixt2 = pixVShear(NULL, pixs, w / 2, ANGLE1, L_BRING_IN_WHITE);
    pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixt1 = pixVShear(NULL, pixs, 0, ANGLE1, L_BRING_IN_BLACK);
    pixSaveTiled(pixt1, pixa, reduction, 0, 20, 0);
    pixt2 = pixVShear(NULL, pixs, w / 2, ANGLE1, L_BRING_IN_BLACK);
    pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    if (!pixGetColormap(pixs)) {
        pixt1 = pixCopy(NULL, pixs);
        pixVShearIP(pixt1, 0, ANGLE1, L_BRING_IN_WHITE);
        pixSaveTiled(pixt1, pixa, reduction, 1, 20, 0);
        pixt2 = pixCopy(NULL, pixs);
        pixVShearIP(pixt2, w / 2, ANGLE1, L_BRING_IN_WHITE);
        pixSaveTiled(pixt2, pixa, reduction, 0, 20, 0);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixt1 = pixCopy(NULL, pixs);
        pixVShearIP(pixt1, 0, ANGLE1, L_BRING_IN_BLACK);
        pixSaveTiled(pixt1, pixa, reduction, 0, 20, 0);
        pixt2 = pixCopy(NULL, pixs);
        pixVShearIP(pixt2, w / 2, ANGLE1, L_BRING_IN_BLACK);
        pixSaveTiled(pixt2, pixa, reduction, 0, 20, 32);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite(filename, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    return;
}


