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
 * ioformats_reg.c
 *
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *    This is the primary Leptonica regression test for lossless
 *    read/write I/O to standard image files (png, tiff, bmp, etc.)
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *    This tests reading and writing of images in different formats
 *    It should work properly on input images of any depth, with
 *    and without colormaps.
 *
 *    This works by doing a write/read and testing the result
 *    for equality.  For that reason, we only test the lossless
 *    file formats.  jpeg works fine on grayscale and rgb, so
 *    there's no need for explicit tests here.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   BMP_FILE      "test1.bmp"
#define   FILE_1BPP     "feyn.tif"
#define   FILE_2BPP     "weasel2.4g.png"
#define   FILE_4BPP     "weasel4.16c.png"
#define   FILE_8BPP_1   "dreyfus8.png"
#define   FILE_8BPP_2   "weasel8.240c.png"
#define   FILE_8BPP_3   "test8.jpg"
#define   FILE_16BPP    "test16.tif"
#define   FILE_32BPP    "marge.jpg"

static l_int32 testcomp(const char *filename, PIX *pix, l_int32 comptype);


main(int    argc,
     char **argv)
{
l_int32      i, d, n, success;
PIX         *pix1, *pix2, *pix4, *pix8, *pix16, *pix32, *pix;
PIXA        *pixa;
static char  mainName[] = "ioformats_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  ioformats_reg", mainName, 1));

    success = TRUE;
    fprintf(stderr, "Test bmp 1 bpp file:\n");
    if (ioFormatTest(BMP_FILE)) success = FALSE;
    fprintf(stderr, "\nTest other 1 bpp file:\n");
    if (ioFormatTest(FILE_1BPP)) success = FALSE;
    fprintf(stderr, "\nTest 2 bpp file:\n");
    if (ioFormatTest(FILE_2BPP)) success = FALSE;
    fprintf(stderr, "\nTest 4 bpp file:\n");
    if (ioFormatTest(FILE_4BPP)) success = FALSE;
    fprintf(stderr, "\nTest 8 bpp grayscale file with cmap:\n");
    if (ioFormatTest(FILE_8BPP_1)) success = FALSE;
    fprintf(stderr, "\nTest 8 bpp color file with cmap:\n");
    if (ioFormatTest(FILE_8BPP_2)) success = FALSE;
    fprintf(stderr, "\nTest 8 bpp file without cmap:\n");
    if (ioFormatTest(FILE_8BPP_3)) success = FALSE;
    fprintf(stderr, "\nTest 16 bpp file:\n");
    if (ioFormatTest(FILE_16BPP)) success = FALSE;
    fprintf(stderr, "\nTest 32 bpp file:\n");
    if (ioFormatTest(FILE_32BPP)) success = FALSE;
    if (success)
        fprintf(stderr,
            "\n  ********** Success on all i/o format tests *********\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on at least one i/o format test ******\n");

        /* Test tiff r/w */
    fprintf(stderr, "\n\nTest tiff r/w and format extraction\n");
    pixa = pixaCreate(6);
    pix1 = pixRead(BMP_FILE);
    pix2 = pixConvert1To2(NULL, pix1, 3, 0);
    pix4 = pixConvert1To4(NULL, pix1, 15, 0);
    pix16 = pixRead(FILE_16BPP);
    fprintf(stderr, "Input format: %d\n", pixGetInputFormat(pix16));
    pix8 = pixConvert16To8(pix16, 1);
    pix32 = pixRead(FILE_32BPP);
    pixaAddPix(pixa, pix1, L_INSERT);
    pixaAddPix(pixa, pix2, L_INSERT);
    pixaAddPix(pixa, pix4, L_INSERT);
    pixaAddPix(pixa, pix8, L_INSERT);
    pixaAddPix(pixa, pix16, L_INSERT);
    pixaAddPix(pixa, pix32, L_INSERT);
    n = pixaGetCount(pixa);

    success = TRUE;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, L_CLONE);
	d = pixGetDepth(pix);
        fprintf(stderr, "%d bpp\n", d);
	if (i == 0) {   /* 1 bpp */
            pixWrite("junkg3.tif", pix, IFF_TIFF_G3);
            pixWrite("junkg4.tif", pix, IFF_TIFF_G4);
            pixWrite("junkrle.tif", pix, IFF_TIFF_RLE);
            pixWrite("junkpb.tif", pix, IFF_TIFF_PACKBITS);
	    if (testcomp("junkg3.tif", pix, IFF_TIFF_G3)) success = FALSE;
	    if (testcomp("junkg4.tif", pix, IFF_TIFF_G4)) success = FALSE;
	    if (testcomp("junkrle.tif", pix, IFF_TIFF_RLE)) success = FALSE;
	    if (testcomp("junkpb.tif", pix, IFF_TIFF_PACKBITS)) success = FALSE;
	}
        pixWrite("junklzw.tif", pix, IFF_TIFF_LZW);
        pixWrite("junkzip.tif", pix, IFF_TIFF_ZIP);
        pixWrite("junknon.tif", pix, IFF_TIFF);
        if (testcomp("junklzw.tif", pix, IFF_TIFF_LZW)) success = FALSE;
        if (testcomp("junkzip.tif", pix, IFF_TIFF_ZIP)) success = FALSE;
        if (testcomp("junknon.tif", pix, IFF_TIFF)) success = FALSE;
	pixDestroy(&pix);
    }
    if (success)
        fprintf(stderr,
            "\n  ********** Success on tiff r/w tests *********\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on at least one tiff r/w test ******\n");

    pixaDestroy(&pixa);
    exit(0);
}


    /* Returns 1 on error */
static l_int32
testcomp(const char  *filename,
         PIX         *pix,
         l_int32      comptype)
{
l_int32  format, sameformat, sameimage;
FILE    *fp;
PIX     *pixt;

    fp = fopen(filename, "r");
    format = findFileFormat(fp);
    sameformat = TRUE;
    if (format != comptype) {
        fprintf(stderr, "File %s has format %d, not comptype %d\n",
                filename, format, comptype);
        sameformat = FALSE;
    }
    fclose(fp);
    pixt = pixRead(filename);
    pixEqual(pix, pixt, &sameimage);
    pixDestroy(&pixt);
    if (!sameimage)
        fprintf(stderr, "Write/read fail for file %s with format %d\n",
                filename, format);
    return (!sameformat || !sameimage);
}

