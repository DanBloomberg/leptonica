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
 * colorquant_reg.c
 *
 *    Regression test for various color quantizers
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32 SPACE = 30;
static const l_int32 MAX_WIDTH = 350;
static const char *image[4] = {"marge.jpg",
                               "test24.jpg",
                               "juditharismax.jpg",
                               "hardlight2_2.jpg"};

static l_int32 TestImage(const char *filename, l_int32 i);
static void PixSave32(PIXA *pixa, PIX *pixc);

main(int    argc,
     char **argv)
{
l_int32  i;
static char  mainName[] = "colorquant_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax: colorquant_reg", mainName, 1));

    for (i = 0; i < 4; i++) {
/*        if (i != 2) continue; */
        TestImage(image[i], i);
    }
    return 0;
}


static l_int32
TestImage(const char *filename,
          l_int32     i)
{
char       buf[256];
l_int32    w, h, nerrors;
l_float32  factor;
PIX       *pix, *pixs, *pixc, *pix32, *pixt, *pixd;
PIXA      *pixa;
char      *fileout;

    PROCNAME("TestImage");

    if ((pix = pixRead(filename)) == NULL)
        return ERROR_INT("pix not made", procName, 1);
    pixGetDimensions(pix, &w, &h, NULL);
    if (w > MAX_WIDTH) {
        factor = (l_float32)MAX_WIDTH / (l_float32)w;
        pixs = pixScale(pix, factor, factor);
    }
    else
        pixs = pixClone(pix);
    pixDestroy(&pix);

    pixa = pixaCreate(0);

        /* Median cut quantizer (no dither; 5 sigbits) */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 16, 5, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 128, 5, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 256, 5, 1);
    PixSave32(pixa, pixc);

        /* Median cut quantizer (with dither; 5 sigbits) */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 16, 5, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 128, 5, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 256, 5, 1);
    PixSave32(pixa, pixc);

        /* Median cut quantizer (no dither; 6 sigbits) */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 32);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 16, 6, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 128, 6, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 256, 6, 1);
    PixSave32(pixa, pixc);

        /* Median cut quantizer (with dither; 6 sigbits) */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 16, 6, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 128, 6, 1);
    PixSave32(pixa, pixc);
    pixc = pixMedianCutQuantGeneral(pixs, 1, 0, 256, 6, 10);
    PixSave32(pixa, pixc);

        /* Simple 256 cube octcube quantizer */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixFixedOctcubeQuant256(pixs, 0);  /* no dither */
    PixSave32(pixa, pixc);
    pixc = pixFixedOctcubeQuant256(pixs, 1);  /* dither */
    PixSave32(pixa, pixc);

        /* 2-pass octree quantizer */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixOctreeColorQuant(pixs, 128, 0);  /* no dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeColorQuant(pixs, 240, 0);  /* no dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeColorQuant(pixs, 128, 1);  /* dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeColorQuant(pixs, 240, 1);  /* dither */
    PixSave32(pixa, pixc);

        /* Simple adaptive quantization to 4 or 8 bpp, specifying ncolors */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixOctreeQuantNumColors(pixs, 8, 0);    /* fixed: 8 colors */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantNumColors(pixs, 16, 0);   /* fixed: 16 colors */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantNumColors(pixs, 64, 0);   /* fixed: 64 colors */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantNumColors(pixs, 256, 0);   /* fixed: 256 colors */
    PixSave32(pixa, pixc);

        /* Quantize to fully populated octree (RGB) at given level */
    pixSaveTiled(pixs, pixa, 1, 1, SPACE, 0);
    pixc = pixFixedOctcubeQuantGenRGB(pixs, 2);  /* level 2 */
    PixSave32(pixa, pixc);
    pixc = pixFixedOctcubeQuantGenRGB(pixs, 3);  /* level 3 */
    PixSave32(pixa, pixc);
    pixc = pixFixedOctcubeQuantGenRGB(pixs, 4);  /* level 4 */
    PixSave32(pixa, pixc);
    pixc = pixFixedOctcubeQuantGenRGB(pixs, 5);  /* level 5 */
    PixSave32(pixa, pixc);

        /* Generate 32 bpp RGB image with num colors <= 256 */
    pixt = pixOctreeQuantNumColors(pixs, 256, 0);   /* cmapped version */
    pix32 = pixRemoveColormap(pixt, REMOVE_CMAP_BASED_ON_SRC);

        /* Quantize image with few colors at fixed octree leaf level */
    pixSaveTiled(pixt, pixa, 1, 1, SPACE, 0);
    pixc = pixFewColorsOctcubeQuant1(pix32, 2);   /* level 2 */
    PixSave32(pixa, pixc);
    pixc = pixFewColorsOctcubeQuant1(pix32, 3);   /* level 3 */
    PixSave32(pixa, pixc);
    pixc = pixFewColorsOctcubeQuant1(pix32, 4);   /* level 4 */
    PixSave32(pixa, pixc);
    pixc = pixFewColorsOctcubeQuant1(pix32, 5);   /* level 5 */
    PixSave32(pixa, pixc);

        /* Quantize image by population */
    pixSaveTiled(pixt, pixa, 1, 1, SPACE, 0);
    pixc = pixOctreeQuantByPopulation(pixs, 3, 0);  /* level 3, no dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantByPopulation(pixs, 3, 1);  /* level 3, dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantByPopulation(pixs, 4, 0);  /* level 4, no dither */
    PixSave32(pixa, pixc);
    pixc = pixOctreeQuantByPopulation(pixs, 4, 1);  /* level 4, dither */
    PixSave32(pixa, pixc);

        /* Mixed color/gray octree quantizer */
    pixSaveTiled(pixt, pixa, 1, 1, SPACE, 0);
    pixc = pixOctcubeQuantMixedWithGray(pix32, 8, 64, 10);  /* max delta = 10 */
    PixSave32(pixa, pixc);
    pixc = pixOctcubeQuantMixedWithGray(pix32, 8, 64, 30);  /* max delta = 30 */
    PixSave32(pixa, pixc);
    pixc = pixOctcubeQuantMixedWithGray(pix32, 8, 64, 50);  /* max delta = 50 */
    PixSave32(pixa, pixc);
    
        /* Run the high-level converter */
    pixSaveTiled(pixt, pixa, 1, 1, SPACE, 0);
    pixc = pixConvertRGBToColormap(pix32, 1);
    PixSave32(pixa, pixc);

    pixDestroy(&pix32);
    pixDestroy(&pixt);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplay(pixd, 100, 100);
    sprintf(buf, "junkdisp.%d.jpg", i);
    pixWrite(buf, pixd, IFF_JFIF_JPEG);

    pixDestroy(&pixs);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    return 0;
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

