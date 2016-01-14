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
 * psiotest.c
 *
 *   Tests writing of images in PS, with arbitrary scaling and
 *   translation, in the following formats:
 *
 *      - uncompressed
 *      - DCT compressed (jpeg for 8 bpp grayscale and RGB)
 *      - CCITT-G4 compressed (g4 fax compression for 1 bpp)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   PS_FACTOR        0.9

main(int    argc,
     char **argv)
{
l_int32      w, h;
l_float32    scale;
BOX         *box;
FILE        *fp;
PIX         *pix;
char        *filein, *fileout;
static char  mainName[] = "psiotest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  psiotest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pix = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

#if 0  /* uncompressed PS with scaling but centered on the page */
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    scale = L_MIN(PS_FACTOR * 2550 / w, PS_FACTOR * 3300 / h);
    fp = fopen(fileout, "wb+");
    pixWriteStreamPS(fp, pix, NULL, 300, scale);
    fclose(fp);
#endif

#if 0   /* uncompressed PS with scaling, with LL corner at (500, 500) mils */
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    scale = L_MIN(PS_FACTOR * 2550 / w, PS_FACTOR * 3300 / h);
    box = boxCreate(500, 500, (l_int32)(1000 * scale * w / 300),
                    (l_int32)(1000 * scale * h / 300));
    fp = fopen(fileout, "wb+");
    pixWriteStreamPS(fp, pix, box, 300, 1.0);
    fclose(fp);
#endif

#if 0   /* DCT compressed PS with LL corner at (300, 1000) pixels */
    pixWrite("junktemp", pix, IFF_JFIF_JPEG);
    convertJpegToPS("junktemp", fileout, "w", 300, 1000, 0, 1.0, 1, 1);
    boxDestroy(&box);
#endif

#if 0   /* CCITT-G4 compressed PS with LL corner at (800, 1500) pixels */
    pixWrite("junktemp", pix, IFF_TIFF_G4);
    convertTiffG4ToPS("junktemp", fileout, "w", 800, 1500, 0, 1.0, 1, 1, 0);
    boxDestroy(&box);
#endif

#if 0  /* g4 first as an image; then jpeg over it */
    convertTiffG4ToPS("feyn.tif", fileout, "w", 0, 0, 0, 1.0, 1, 1, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 500, 100, 300, 2.0, 1,  0);
    convertJpegToPS("marge.jpg", fileout, "a", 100, 800, 300, 2.0, 1, 1);

    convertTiffG4ToPS("feyn.tif", fileout, "a", 0, 0, 0, 1.0, 2, 1, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 1000, 700, 300, 2.0, 2, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 100, 200, 300, 2.0, 2, 1);

    convertTiffG4ToPS("feyn.tif", fileout, "a", 0, 0, 0, 1.0, 3, 1, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 200, 200, 300, 2.0, 3, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 200, 900, 300, 2.0, 3, 1);
#endif

#if 1  /* jpeg first; then paint through a g4 mask */
    convertJpegToPS("marge.jpg", fileout, "w", 500, 100, 300, 2.0, 1,  0);
    convertJpegToPS("marge.jpg", fileout, "a", 100, 800, 300, 2.0, 1, 0);
    convertTiffG4ToPS("feyn.tif", fileout, "a", 0, 0, 0, 1.0, 1, 1, 1);

    convertJpegToPS("marge.jpg", fileout, "a", 1000, 700, 300, 2.0, 2, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 100, 200, 300, 2.0, 2, 0);
    convertTiffG4ToPS("feyn.tif", fileout, "a", 0, 0, 0, 1.0, 2, 1, 1);

    convertJpegToPS("marge.jpg", fileout, "a", 200, 200, 300, 2.0, 3, 0);
    convertJpegToPS("marge.jpg", fileout, "a", 200, 900, 300, 2.0, 3, 0);
    convertTiffG4ToPS("feyn.tif", fileout, "a", 0, 0, 0, 1.0, 3, 1, 1);
#endif

    pixDestroy(&pix);
    exit(0);
}

