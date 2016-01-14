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
 *    The first part of the test works by doing a write/read and
 *    testing the result for equality.  We only test the lossless
 *    file formats, with pix of various depths, both with and
 *    without colormaps.  Because jpeg works fine on grayscale
 *    and rgb, there is no need for explicit tests on jpeg
 *    compression here.
 *
 *    The second part tests all different tiff compressions, for
 *    read/write that is backed both by file and by memory.
 *    For r/w to file, it is actually redundant with the first part.)
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
static l_int32 testcomp_mem(PIX *pixs, PIX **ppixt, l_int32 index,
                            l_int32 format);
static l_int32 test_writemem(PIX *pixs, l_int32 format, char *psfile);
static PIX *make_24_bpp_pix(PIX *pixs);


main(int    argc,
     char **argv)
{
char         psname[256];
l_uint8     *data;
l_int32      i, d, n, success, nbytes, same;
l_int32      w, h, bps, spp;
l_float32    diff;
size_t       size;
PIX         *pix1, *pix2, *pix4, *pix8, *pix16, *pix32;
PIX         *pix, *pixt, *pixd;
PIXA        *pixa;
static char  mainName[] = "ioformats_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  ioformats_reg", mainName, 1));

    /* --------- Part 1: Test all lossless formats for r/w to file ---------*/

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

    /* ------------------ Part 2: Test tiff r/w to file ------------------- */

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
            "\n  ********** Success on tiff r/w to file *********\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on at least one tiff r/w to file ******\n");

    /* ------------------ Part 3: Test tiff r/w to memory ----------------- */

    success = TRUE;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, L_CLONE);
	d = pixGetDepth(pix);
        fprintf(stderr, "%d bpp\n", d);
	if (i == 0) {   /* 1 bpp */
            pixWriteMemTiff(&data, &size, pix, IFF_TIFF_G3);
            nbytes = nbytesInFile("junkg3.tif");
            fprintf(stderr, "nbytes = %d, size = %d\n", nbytes, size);
            pixt = pixReadMemTiff(data, size, 0);
            if (testcomp_mem(pix, &pixt, i, IFF_TIFF_G3)) success = FALSE;
	    FREE(data);
            pixWriteMemTiff(&data, &size, pix, IFF_TIFF_G4);
            nbytes = nbytesInFile("junkg4.tif");
            fprintf(stderr, "nbytes = %d, size = %d\n", nbytes, size);
            pixt = pixReadMemTiff(data, size, 0);
            if (testcomp_mem(pix, &pixt, i, IFF_TIFF_G4)) success = FALSE;
            readHeaderMemTiff(data, size, 0, &w, &h, &bps, &spp, NULL, NULL);
            fprintf(stderr, "(w,h,bps,spp) = (%d,%d,%d,%d)\n", w, h, bps, spp);
	    FREE(data);
            pixWriteMemTiff(&data, &size, pix, IFF_TIFF_RLE);
            nbytes = nbytesInFile("junkrle.tif");
            fprintf(stderr, "nbytes = %d, size = %d\n", nbytes, size);
            pixt = pixReadMemTiff(data, size, 0);
            if (testcomp_mem(pix, &pixt, i, IFF_TIFF_RLE)) success = FALSE;
	    FREE(data);
            pixWriteMemTiff(&data, &size, pix, IFF_TIFF_PACKBITS);
            nbytes = nbytesInFile("junkpb.tif");
            fprintf(stderr, "nbytes = %d, size = %d\n", nbytes, size);
            pixt = pixReadMemTiff(data, size, 0);
            if (testcomp_mem(pix, &pixt, i, IFF_TIFF_PACKBITS)) success = FALSE;
	    FREE(data);
	}
        pixWriteMemTiff(&data, &size, pix, IFF_TIFF_LZW);
        pixt = pixReadMemTiff(data, size, 0);
        if (testcomp_mem(pix, &pixt, i, IFF_TIFF_LZW)) success = FALSE;
        FREE(data);
        pixWriteMemTiff(&data, &size, pix, IFF_TIFF_ZIP);
        pixt = pixReadMemTiff(data, size, 0);
        if (testcomp_mem(pix, &pixt, i, IFF_TIFF_ZIP)) success = FALSE;
        readHeaderMemTiff(data, size, 0, &w, &h, &bps, &spp, NULL, NULL);
        fprintf(stderr, "(w,h,bps,spp) = (%d,%d,%d,%d)\n", w, h, bps, spp);
        FREE(data);
        pixWriteMemTiff(&data, &size, pix, IFF_TIFF);
        pixt = pixReadMemTiff(data, size, 0);
        if (testcomp_mem(pix, &pixt, i, IFF_TIFF)) success = FALSE;
        FREE(data);
        pixDestroy(&pix);
    }
    if (success)
        fprintf(stderr,
            "\n  ********** Success on tiff r/w to memory *********\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on at least one tiff r/w to memory ******\n");


    /* ---------------- Part 4: Test non-tiff r/w to memory ---------------- */

#if HAVE_FMEMOPEN
    pixDisplayWrite(NULL, -1);
    success = TRUE;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, L_CLONE);
	d = pixGetDepth(pix);
        sprintf(psname, "junkps.%d", d);
        fprintf(stderr, "%d bpp\n", d);
        if (d != 16) {
            if (test_writemem(pix, IFF_PNG, NULL)) success = FALSE;
            if (test_writemem(pix, IFF_BMP, NULL)) success = FALSE;
        }
        if (test_writemem(pix, IFF_PNM, NULL)) success = FALSE;
        if (test_writemem(pix, IFF_PS, psname)) success = FALSE;
	if (d == 8 || d == 32)
            if (test_writemem(pix, IFF_JFIF_JPEG, NULL)) success = FALSE;
        pixDestroy(&pix);
    }
    if (success)
        fprintf(stderr,
            "\n  ********** Success on non-tiff r/w to memory *********\n");
    else
        fprintf(stderr,
            "\n  ***** Failure on at least one non-tiff r/w to memory *****\n");
#else
        fprintf(stderr,
            "\n  ***** Non-tiff r/w to memory not enabled *****\n");
#endif  /*  HAVE_FMEMOPEN  */

    pixaDestroy(&pixa);

    /* ------------ Part 5: Test multipage tiff r/w to memory ------------ */

        /* Make a multipage tiff file, and read it back into memory */
    success = TRUE;
    pix = pixRead("feyn.tif");
    pixa = pixaSplitPix(pix, 3, 3, 0, 0);
    for (i = 0; i < 9; i++) {
        pixt = pixaGetPix(pixa, i, L_CLONE);
        if (i == 0)
            pixWriteTiff("junktiffmpage.tif", pixt, IFF_TIFF_G4, "w");
        else
            pixWriteTiff("junktiffmpage.tif", pixt, IFF_TIFF_G4, "a");
        pixDestroy(&pixt);
    }
    data = arrayRead("junktiffmpage.tif", &nbytes);
    pixaDestroy(&pixa);

        /* Read the individual pages from memory to a pix */
    pixa = pixaCreate(9);
    for (i = 0; i < 9; i++) {
        pixt = pixReadMemTiff(data, nbytes, i);
        pixaAddPix(pixa, pixt, L_INSERT);
    }
    FREE(data);

        /* Un-tile the pix in the pixa back to the original image */
    pixt = pixaDisplayUnsplit(pixa, 3, 3, 0, 0);
    pixaDestroy(&pixa);

        /* Clip to foreground to remove any extra rows or columns */
    pixClipToForeground(pix, &pix1, NULL);
    pixClipToForeground(pixt, &pix2, NULL);
    pixEqual(pix1, pix2, &same); 
    if (same)
        fprintf(stderr,
            "\n  ******* Success on tiff multipage read from memory *******\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on tiff multipage read from memory *******\n");

    pixDestroy(&pix);
    pixDestroy(&pixt);
    pixDestroy(&pix1);
    pixDestroy(&pix2);

    /* ------------ Part 6: Test 24 bpp writing ------------ */

        /* Generate a 24 bpp (not 32 bpp !!) rgb pix and write it out */
    success = TRUE;
    pix = pixRead("marge.jpg");
    pixt = make_24_bpp_pix(pix);
    pixWrite("junk24.png", pixt, IFF_PNG);
    pixWrite("junk24.jpg", pixt, IFF_JFIF_JPEG);
    pixWrite("junk24.tif", pixt, IFF_TIFF);
    pixd = pixRead("junk24.png");
    pixEqual(pix, pixd, &same);
    if (!same) success = FALSE;
    pixDestroy(&pixd);
    pixd = pixRead("junk24.jpg");
    pixCompareRGB(pix, pixd, L_COMPARE_ABS_DIFF, 0, NULL, &diff, NULL, NULL);
    if (diff > 0.1) success = FALSE;
    pixDestroy(&pixd);
    pixd = pixRead("junk24.tif");
    pixEqual(pix, pixd, &same);
    if (!same) success = FALSE;
    pixDestroy(&pixd);
    if (success)
        fprintf(stderr,
            "\n  ******* Success on 24 bpp rgb writing *******\n");
    else
        fprintf(stderr,
            "\n  ******* Failure on 24 bpp rgb writing *******\n");
    pixDestroy(&pix);
    pixDestroy(&pixt);

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


    /* Returns 1 on error */
static l_int32
testcomp_mem(PIX     *pixs,
             PIX    **ppixt,
             l_int32  index,
             l_int32  format)
{
l_int32  sameimage;
PIX     *pixt;

    pixt = *ppixt;
    pixEqual(pixs, pixt, &sameimage);
    if (!sameimage)
        fprintf(stderr, "Mem Write/read fail for file %s with format %d\n",
                index, format);
    pixDestroy(&pixt);
    *ppixt = NULL;
    return (!sameimage);
}


    /* Returns 1 on error */
static l_int32
test_writemem(PIX      *pixs,
              l_int32   format,
              char     *psfile)
{
l_uint8  *data = NULL;
l_int32   same;
size_t    size = 0;
PIX      *pixd = NULL;

    if (format == IFF_PS) {
       pixWriteMemPS(&data, &size, pixs, NULL, 0, 1.0);
       arrayWrite(psfile, "w", data, size);
       FREE(data);
       return 0;
    }

    if (pixWriteMem(&data, &size, pixs, format)) {
        fprintf(stderr, "Mem write fail for format %d\n", format);
        return 1;
    }
    if ((pixd = pixReadMem(data, size)) == NULL) {
        fprintf(stderr, "Mem read fail for format %d\n", format);
        FREE(data);
        return 1;
    }
    if (format == IFF_JFIF_JPEG) {
        fprintf(stderr, "jpeg size = %d\n", size);
        pixDisplayWrite(pixd, 1);
        same = TRUE;
    }
    else {
        pixEqual(pixs, pixd, &same);
        if (!same)
           fprintf(stderr, "Mem write/read fail for format %d\n", format);
    }
    pixDestroy(&pixd);
    FREE(data);
    return (!same);
}


    /* Composes 24 bpp rgb pix */
static PIX *
make_24_bpp_pix(PIX  *pixs)
{
l_int32    i, j, w, h, wpls, wpld, rval, gval, bval;
l_uint32  *lines, *lined, *datas, *datad;
PIX       *pixd;

    pixGetDimensions(pixs, &w, &h, NULL);
    pixd = pixCreate(w, h, 24);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            extractRGBValues(lines[j], &rval, &gval, &bval);
            *((l_uint8 *)lined + 3 * j) = rval;
            *((l_uint8 *)lined + 3 * j + 1) = gval;
            *((l_uint8 *)lined + 3 * j + 2) = bval;
        }
    }

    return pixd;
}


