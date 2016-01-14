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
 *  colorquant_reg.c
 *
 *    Regression test for various color quantizers
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  FILE1    "marge.jpg"
#define  FILE2    "test24.jpg"
#define  NFILES   2


main(int    argc,
     char **argv)
{
l_int32      i;
PIX         *pixs, *pixt;
PIXA        *pixa;
static char  mainName[] = "colorquant_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  colorquant_reg", mainName, 1));

    pixa = pixaCreate(2);
    pixs = pixRead(FILE1);
    pixaAddPix(pixa, pixs, L_INSERT);
    pixs = pixRead(FILE2);
    pixaAddPix(pixa, pixs, L_INSERT);

    pixDisplayWrite(NULL, -1);

    for (i = 0; i < NFILES; i++) { 
        pixs = pixaGetPix(pixa, i, L_CLONE);

	    /* Conversion with either fixed octcubes or dithered */
        pixt = pixConvertRGBToColormap(pixs, 4, NULL);
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

            /* Simple 1-pass octree quantizer */
        pixt = pixColorQuant1Pass(pixs, 0);  /* no dither */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixColorQuant1Pass(pixs, 1);  /* dither */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

            /* 2-pass octree quantizer */
        pixt = pixOctreeColorQuant(pixs, 220, 0);  /* no dither */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixOctreeColorQuant(pixs, 220, 1);  /* dither */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

            /* 2-depth octree quantizer */
        pixt = pixOctreeQuant(pixs, 64, 1);  /* max 64 colors */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixOctreeQuant(pixs, 220, 1);  /* max 220 colors */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

            /* Quantize to fully populated level 4 octree (as RGB) */
        pixt = pixFixedOctcubeQuantRGB(pixs, 4);
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

            /* Mixed color/gray octree quantizer */
        pixt = pixOctcubeQuantMixed(pixs, 8, 64, 10);  /* max delta = 10 */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixOctcubeQuantMixed(pixs, 8, 64, 30);  /* max delta = 30 */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixOctcubeQuantMixed(pixs, 8, 64, 50);  /* max delta = 50 */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

#if 0   /* not yet ready for prime time */
            /* Median cut color quantizer */
        pixt = pixMedianCutQuant(pixs, 0, 256, 5, 1);  /* 5 sigbits */
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
#endif

        pixDestroy(&pixs);
    }

    system("gthumb junk_write_display* &");

    pixaDestroy(&pixa);
    return 0;
}

