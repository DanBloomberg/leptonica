/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*
 * pngio_reg.c
 *
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *    This is the Leptonica regression test for lossless read/write
 *    I/O in png format.
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *    This tests reading and writing of images in png format for
 *    various depths, with and without colormaps.
 *
 *    This test is dependent on the following external libraries:
 *        libpng, libz
 */

#include "allheaders.h"

    /* Needed for checking libraries */
#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif /* HAVE_CONFIG_H */

#define   FILE_1BPP          "rabi.png"
#define   FILE_2BPP          "speckle2.png"
#define   FILE_2BPP_C        "weasel2.4g.png"
#define   FILE_4BPP          "speckle4.png"
#define   FILE_4BPP_C        "weasel4.16c.png"
#define   FILE_8BPP          "dreyfus8.png"
#define   FILE_8BPP_C        "weasel8.240c.png"
#define   FILE_16BPP         "test16.png"
#define   FILE_32BPP         "weasel32.png"
#define   FILE_32BPP_ALPHA   "test32-alpha.png"
#define   FILE_CMAP_ALPHA    "test-cmap-alpha.png"
#define   FILE_CMAP_ALPHA2   "test-cmap-alpha2.png"
#define   FILE_TRANS_ALPHA   "test-fulltrans-alpha.png"
#define   FILE_GRAY_ALPHA    "test-gray-alpha.png"

static l_int32 test_mem_png(const char *fname);
static l_int32 get_header_data(const char *filename);
static l_int32 test_1bpp_trans(L_REGPARAMS *rp);
static l_int32 test_1bpp_color(L_REGPARAMS *rp);
static l_int32 test_1bpp_gray(L_REGPARAMS *rp);
static l_int32 test_1bpp_bw1(L_REGPARAMS *rp);
static l_int32 test_1bpp_bw2(L_REGPARAMS *rp);
static l_int32 test_8bpp_trans(L_REGPARAMS  *rp);

LEPT_DLL extern const char *ImageFileFormatExtensions[];

int main(int    argc,
         char **argv)
{
l_int32       success, failure;
L_REGPARAMS  *rp;

#if !HAVE_LIBPNG || !HAVE_LIBZ
    fprintf(stderr, "libpng & libz are required for testing pngio_reg\n");
    return 1;
#endif  /* abort */

    if (regTestSetup(argc, argv, &rp))
        return 1;

    /* --------- Part 1: Test lossless r/w to file ---------*/

    failure = FALSE;
    success = TRUE;
    fprintf(stderr, "Test bmp 1 bpp file:\n");
    if (ioFormatTest(FILE_1BPP)) success = FALSE;
    fprintf(stderr, "\nTest 2 bpp file:\n");
    if (ioFormatTest(FILE_2BPP)) success = FALSE;
    fprintf(stderr, "\nTest 2 bpp file with cmap:\n");
    if (ioFormatTest(FILE_2BPP_C)) success = FALSE;
    fprintf(stderr, "\nTest 4 bpp file:\n");
    if (ioFormatTest(FILE_4BPP)) success = FALSE;
    fprintf(stderr, "\nTest 4 bpp file with cmap:\n");
    if (ioFormatTest(FILE_4BPP_C)) success = FALSE;
    fprintf(stderr, "\nTest 8 bpp grayscale file with cmap:\n");
    if (ioFormatTest(FILE_8BPP)) success = FALSE;
    fprintf(stderr, "\nTest 8 bpp color file with cmap:\n");
    if (ioFormatTest(FILE_8BPP_C)) success = FALSE;
    fprintf(stderr, "\nTest 16 bpp file:\n");
    if (ioFormatTest(FILE_16BPP)) success = FALSE;
    fprintf(stderr, "\nTest 32 bpp RGB file:\n");
    if (ioFormatTest(FILE_32BPP)) success = FALSE;
    fprintf(stderr, "\nTest 32 bpp RGBA file:\n");
    if (ioFormatTest(FILE_32BPP_ALPHA)) success = FALSE;
    fprintf(stderr, "\nTest spp = 1, cmap with alpha file:\n");
    if (ioFormatTest(FILE_CMAP_ALPHA)) success = FALSE;
    fprintf(stderr, "\nTest spp = 1, cmap with alpha (small alpha array):\n");
    if (ioFormatTest(FILE_CMAP_ALPHA2)) success = FALSE;
    fprintf(stderr, "\nTest spp = 1, fully transparent with alpha file:\n");
    if (ioFormatTest(FILE_TRANS_ALPHA)) success = FALSE;
    fprintf(stderr, "\nTest spp = 2, gray with alpha file:\n");
    if (ioFormatTest(FILE_GRAY_ALPHA)) success = FALSE;
    if (success) {
        fprintf(stderr,
            "\n  ********** Success on lossless r/w to file *********\n\n");
    } else {
        fprintf(stderr,
            "\n  ******* Failure on at least one r/w to file ******\n\n");
    }
    if (!success) failure = TRUE;

    /* ------------ Part 2: Test lossless r/w to memory ------------ */
    success = TRUE;
    if (test_mem_png(FILE_1BPP)) success = FALSE;
    if (test_mem_png(FILE_2BPP)) success = FALSE;
    if (test_mem_png(FILE_2BPP_C)) success = FALSE;
    if (test_mem_png(FILE_4BPP)) success = FALSE;
    if (test_mem_png(FILE_4BPP_C)) success = FALSE;
    if (test_mem_png(FILE_8BPP)) success = FALSE;
    if (test_mem_png(FILE_8BPP_C)) success = FALSE;
    if (test_mem_png(FILE_16BPP)) success = FALSE;
    if (test_mem_png(FILE_32BPP)) success = FALSE;
    if (test_mem_png(FILE_32BPP_ALPHA)) success = FALSE;
    if (test_mem_png(FILE_CMAP_ALPHA)) success = FALSE;
    if (test_mem_png(FILE_CMAP_ALPHA2)) success = FALSE;
    if (test_mem_png(FILE_TRANS_ALPHA)) success = FALSE;
    if (test_mem_png(FILE_GRAY_ALPHA)) success = FALSE;
    if (success) {
        fprintf(stderr,
            "\n  ****** Success on lossless r/w to memory *****\n");
    } else {
        fprintf(stderr,
            "\n  ******* Failure on at least one r/w to memory ******\n");
    }
    if (!success) failure = TRUE;

    /* ------------ Part 3: Test lossless 1 and 8 bpp r/w ------------ */
    fprintf(stderr, "\nTest lossless 1 and 8 bpp r/w\n");
    success = TRUE;
    if (test_1bpp_trans(rp) == 0) success = FALSE;
    if (test_1bpp_color(rp) == 0) success = FALSE;
    if (test_1bpp_gray(rp) == 0) success = FALSE;
    if (test_1bpp_bw1(rp) == 0) success = FALSE;
    if (test_1bpp_bw2(rp) == 0) success = FALSE;
    if (test_8bpp_trans(rp) == 0) success = FALSE;

    if (success) {
        fprintf(stderr,
            "\n  ******* Success on 1 and 8 bpp lossless *******\n\n");
    } else {
        fprintf(stderr,
            "\n  ******* Failure on 1 and 8 bpp lossless *******\n\n");
    }
    if (!success) failure = TRUE;

    /* -------------- Part 4: Read header information -------------- */
    success = TRUE;
    if (get_header_data(FILE_1BPP)) success = FALSE;
    if (get_header_data(FILE_2BPP)) success = FALSE;
    if (get_header_data(FILE_2BPP_C)) success = FALSE;
    if (get_header_data(FILE_4BPP)) success = FALSE;
    if (get_header_data(FILE_4BPP_C)) success = FALSE;
    if (get_header_data(FILE_8BPP)) success = FALSE;
    if (get_header_data(FILE_8BPP_C)) success = FALSE;
    if (get_header_data(FILE_16BPP)) success = FALSE;
    if (get_header_data(FILE_32BPP)) success = FALSE;
    if (get_header_data(FILE_32BPP_ALPHA)) success = FALSE;
    if (get_header_data(FILE_CMAP_ALPHA)) success = FALSE;
    if (get_header_data(FILE_CMAP_ALPHA2)) success = FALSE;
    if (get_header_data(FILE_TRANS_ALPHA)) success = FALSE;
    if (get_header_data(FILE_GRAY_ALPHA)) success = FALSE;

    if (success) {
        fprintf(stderr,
            "\n  ******* Success on reading headers *******\n\n");
    } else {
        fprintf(stderr,
            "\n  ******* Failure on reading headers *******\n\n");
    }
    if (!success) failure = TRUE;

    if (!failure) {
        fprintf(stderr,
            "  ******* Success on all tests *******\n\n");
    } else {
        fprintf(stderr,
            "  ******* Failure on at least one test *******\n\n");
    }

    if (failure) rp->success = FALSE;
    return regTestCleanup(rp);
}


    /* Returns 1 on error */
static l_int32
test_mem_png(const char  *fname)
{
l_uint8  *data = NULL;
l_int32   same;
size_t    size = 0;
PIX      *pixs;
PIX      *pixd = NULL;

    if ((pixs = pixRead(fname)) == NULL) {
        fprintf(stderr, "Failure to read %s\n", fname);
        return 1;
    }
    if (pixWriteMem(&data, &size, pixs, IFF_PNG)) {
        fprintf(stderr, "Mem write fail for png\n");
        return 1;
    }
    if ((pixd = pixReadMem(data, size)) == NULL) {
        fprintf(stderr, "Mem read fail for png\n");
        lept_free(data);
        return 1;
    }

    pixEqual(pixs, pixd, &same);
    if (!same)
        fprintf(stderr, "Mem write/read fail for file %s\n", fname);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
    lept_free(data);
    return (!same);
}

    /* Retrieve header data from file and from array in memory */
static l_int32
get_header_data(const char  *filename)
{
l_uint8  *data;
l_int32   ret1, ret2, format1, format2;
l_int32   w1, w2, h1, h2, d1, d2, bps1, bps2, spp1, spp2, iscmap1, iscmap2;
size_t    nbytes1, nbytes2;

        /* Read header from file */
    nbytes1 = nbytesInFile(filename);
    ret1 = pixReadHeader(filename, &format1, &w1, &h1, &bps1, &spp1, &iscmap1);
    d1 = bps1 * spp1;
    if (d1 == 24) d1 = 32;
    if (ret1) {
        fprintf(stderr, "Error: couldn't read header data from file: %s\n",
                filename);
    } else {
        fprintf(stderr, "Format data for image %s with format %s:\n"
            "  nbytes = %lu, size (w, h, d) = (%d, %d, %d)\n"
            "  bps = %d, spp = %d, iscmap = %d\n",
            filename, ImageFileFormatExtensions[format1],
            (unsigned long)nbytes1, w1, h1, d1, bps1, spp1, iscmap1);
        if (format1 != IFF_PNG) {
            fprintf(stderr, "Error: format is %d; should be %d\n",
                    format1, IFF_PNG);
            ret1 = 1;
        }
    }

        /* Read header from array in memory */
    data = l_binaryRead(filename, &nbytes2);
    ret2 = pixReadHeaderMem(data, nbytes2, &format2, &w2, &h2, &bps2,
                            &spp2, &iscmap2);
    lept_free(data);
    d2 = bps2 * spp2;
    if (d2 == 24) d2 = 32;
    if (ret2) {
        fprintf(stderr, "Error: couldn't mem-read header data: %s\n", filename);
    } else {
        if (nbytes1 != nbytes2 || format1 != format2 || w1 != w2 ||
            h1 != h2 || d1 != d2 || bps1 != bps2 || spp1 != spp2 ||
            iscmap1 != iscmap2) {
            fprintf(stderr, "Incomsistency reading image %s with format %s\n",
                    filename, ImageFileFormatExtensions[IFF_PNG]);
            ret2 = 1;
        }
    }

    return ret1 || ret2;
}

static l_int32
test_1bpp_trans(L_REGPARAMS  *rp)
{
l_int32   same, transp;
FILE     *fp;
PIX      *pix1, *pix2;
PIXCMAP  *cmap;

    pix1 = pixRead("feyn-fract2.tif");
    cmap = pixcmapCreate(1);
    pixSetColormap(pix1, cmap);
    pixcmapAddRGBA(cmap, 180, 130, 220, 0);  /* transparent */
    pixcmapAddRGBA(cmap, 20, 120, 0, 255);  /* opaque */
    pixWrite("/tmp/lept/regout/1bpp-trans.png", pix1, IFF_PNG);
    pix2 = pixRead("/tmp/lept/regout/1bpp-trans.png");
    pixEqual(pix1, pix2, &same);
    if (same)
        fprintf(stderr, "1bpp_trans: success\n");
    else
        fprintf(stderr, "1bpp_trans: bad output\n");
    pixDisplayWithTitle(pix2, 700, 0, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    fp = fopenReadStream("/tmp/lept/regout/1bpp-trans.png");
    fgetPngColormapInfo(fp, &cmap, &transp);
    if (fp) fclose(fp);
    if (transp)
        fprintf(stderr, "1bpp_trans: correct -- transparency found\n");
    else
        fprintf(stderr, "1bpp_trans: error -- no transparency found!\n");
    if (rp->display) pixcmapWriteStream(stderr, cmap);
    pixcmapDestroy(&cmap);
    return same;
}

static l_int32
test_1bpp_color(L_REGPARAMS  *rp)
{
l_int32   same, transp;
FILE     *fp;
PIX      *pix1, *pix2;
PIXCMAP  *cmap;

    pix1 = pixRead("feyn-fract2.tif");
    cmap = pixcmapCreate(1);
    pixSetColormap(pix1, cmap);
    pixcmapAddRGBA(cmap, 180, 130, 220, 255);  /* color, opaque */
    pixcmapAddRGBA(cmap, 20, 120, 0, 255);  /* color, opaque */
    pixWrite("/tmp/lept/regout/1bpp-color.png", pix1, IFF_PNG);
    pix2 = pixRead("/tmp/lept/regout/1bpp-color.png");
    pixEqual(pix1, pix2, &same);
    if (same)
        fprintf(stderr, "1bpp_color: success\n");
    else
        fprintf(stderr, "1bpp_color: bad output\n");
    pixDisplayWithTitle(pix2, 700, 100, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    fp = fopenReadStream("/tmp/lept/regout/1bpp-color.png");
    fgetPngColormapInfo(fp, &cmap, &transp);
    if (fp) fclose(fp);
    if (transp)
        fprintf(stderr, "1bpp_color: error -- transparency found!\n");
    else
        fprintf(stderr, "1bpp_color: correct -- no transparency found\n");
    if (rp->display) pixcmapWriteStream(stderr, cmap);
    pixcmapDestroy(&cmap);
    return same;
}

static l_int32
test_1bpp_gray(L_REGPARAMS  *rp)
{
l_int32   same;
PIX      *pix1, *pix2;
PIXCMAP  *cmap;

    pix1 = pixRead("feyn-fract2.tif");
    cmap = pixcmapCreate(1);
    pixSetColormap(pix1, cmap);
    pixcmapAddRGBA(cmap, 180, 180, 180, 255);  /* light, opaque */
    pixcmapAddRGBA(cmap, 60, 60, 60, 255);  /* dark, opaque */
    pixWrite("/tmp/lept/regout/1bpp-gray.png", pix1, IFF_PNG);
    pix2 = pixRead("/tmp/lept/regout/1bpp-gray.png");
    pixEqual(pix1, pix2, &same);
    if (same)
        fprintf(stderr, "1bpp_gray: success\n");
    else
        fprintf(stderr, "1bpp_gray: bad output\n");
    pixDisplayWithTitle(pix2, 700, 200, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    return same;
}

static l_int32
test_1bpp_bw1(L_REGPARAMS  *rp)
{
l_int32   same;
PIX      *pix1, *pix2;
PIXCMAP  *cmap;

    pix1 = pixRead("feyn-fract2.tif");
    cmap = pixcmapCreate(1);
    pixSetColormap(pix1, cmap);
    pixcmapAddRGBA(cmap, 0, 0, 0, 255);  /* black, opaque */
    pixcmapAddRGBA(cmap, 255, 255, 255, 255);  /* white, opaque */
    pixWrite("/tmp/lept/regout/1bpp-bw1.png", pix1, IFF_PNG);
    pix2 = pixRead("/tmp/lept/regout/1bpp-bw1.png");
    pixEqual(pix1, pix2, &same);
    if (same)
        fprintf(stderr, "1bpp_bw1: success\n");
    else
        fprintf(stderr, "1bpp_bw1: bad output\n");
    pixDisplayWithTitle(pix2, 700, 300, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    return same;
}

static l_int32
test_1bpp_bw2(L_REGPARAMS  *rp)
{
l_int32   same;
PIX      *pix1, *pix2;
PIXCMAP  *cmap;

    pix1 = pixRead("feyn-fract2.tif");
    cmap = pixcmapCreate(1);
    pixSetColormap(pix1, cmap);
    pixcmapAddRGBA(cmap, 255, 255, 255, 255);  /* white, opaque */
    pixcmapAddRGBA(cmap, 0, 0, 0, 255);  /* black, opaque */
    pixWrite("/tmp/lept/regout/1bpp-bw2.png", pix1, IFF_PNG);
    pix2 = pixRead("/tmp/lept/regout/1bpp-bw2.png");
    pixEqual(pix1, pix2, &same);
    if (same)
        fprintf(stderr, "1bpp_bw2: success\n");
    else
        fprintf(stderr, "1bpp_bw2: bad output\n");
    pixDisplayWithTitle(pix2, 700, 400, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    return same;
}

static l_int32
test_8bpp_trans(L_REGPARAMS  *rp)
{
l_int32   same, transp;
FILE     *fp;
PIX      *pix1, *pix2, *pix3;
PIXCMAP  *cmap;

    pix1 = pixRead("wyom.jpg");
    pix2 = pixColorSegment(pix1, 75, 10, 8, 7, 0);
    cmap = pixGetColormap(pix2);
    pixcmapSetAlpha(cmap, 0, 0);  /* set blueish sky color to transparent */
    pixWrite("/tmp/lept/regout/8bpp-trans.png", pix2, IFF_PNG);
    pix3 = pixRead("/tmp/lept/regout/8bpp-trans.png");
    pixEqual(pix2, pix3, &same);
    if (same)
        fprintf(stderr, "8bpp_trans: success\n");
    else
        fprintf(stderr, "8bpp_trans: bad output\n");
    pixDisplayWithTitle(pix3, 700, 0, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    fp = fopenReadStream("/tmp/lept/regout/8bpp-trans.png");
    fgetPngColormapInfo(fp, &cmap, &transp);
    if (fp) fclose(fp);
    if (transp)
        fprintf(stderr, "8bpp_trans: correct -- transparency found\n");
    else
        fprintf(stderr, "8bpp_trans: error -- no transparency found!\n");
    if (rp->display) pixcmapWriteStream(stderr, cmap);
    pixcmapDestroy(&cmap);
    return same;
}

