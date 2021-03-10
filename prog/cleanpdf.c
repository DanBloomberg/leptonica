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
 * cleanpdf.c
 *
 *    This program is intended to take as input pdf files that have
 *    been constructed from poorly compressed images -- typically images
 *    that have been scanned in grayscale or color but should be rendered
 *    in black and white (1 bpp).  It cleans and compresses them, and
 *    generates a pdf composed of tiff-g4 compressed images.
 *
 *    It will also take as input clean, orthographically-generated pdfs,
 *    and concatenate them into a single pdf file of images.
 *
 *     Syntax:  cleanpdf basedir threshold resolution outfile [rotation]
 *
 *    The %basedir is a directory where the input pdf files are located.
 *    The program will operate on every file in this directory with
 *    the ".pdf" extension.
 *
 *    The input binarization %threshold should be somewhere in the
 *    range [130 - 190].   The result is typically not very sensitive to
 *    the value, because internally we use a pixel mapping that is adapted
 *    to the local background before thresholding to binarize the image.
 *
 *    The output %resolution parameter can take on two values:
 *       300  (binarize at the same resolution as the gray or color input,
 *             which is typically 300 ppi)
 *       600  (binarize at twice the resolution of the gray or color input,
 *             by doing an interpolated 2x expansion on the grayscale
 *             image, followed by thresholding to 1 bpp)
 *    At 300 ppi, an 8.5 x 11 page would have 2550 x 3300 pixels.
 *    You can also input 0 for the default output resolution of 300 ppi.
 *
 *    The pdf output is written to %outfile.  It is advisable (but not
 *    required) to have a '.pdf' extension.
 *
 *    The optional %rotation is an integer:
 *       0      no rotation
 *       1      90 degrees cw
 *       1      180 degrees cw
 *       1      270 degrees cw
 *
 *    Whenever possible, the images will be deskewed.
 *
 *    Notes on using filenames with internal spaces.
 *    * The file-handling functions in leptonica do not support filenames
 *      that have spaces.  To use cleanpdf in linux with such input
 *      filenames, substitute an ascii character for the spaces; e.g., '^'.
 *         char *newstr = stringReplaceEachSubstr(str, " ", "^", NULL);
 *      Then run cleanpdf on the file(s).
 *    * To get an output filename with spaces, use single quotes; e.g.,
 *         cleanpdf dir thresh res 'filename with spaces'
 *
 *    N.B.  This requires the Poppler package of pdf utilities, such as
 *          pdfimages and pdftoppm.  For non-unix systems, this requires
 *          installation of the cygwin Poppler package:
 *       https://cygwin.com/cgi-bin2/package-cat.cgi?file=x86/poppler/
 *              poppler-0.26.5-1
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#ifdef _WIN32
# if defined(_MSC_VER) || defined(__MINGW32__)
#  include <direct.h>
# else
#  include <io.h>
# endif  /* _MSC_VER || __MINGW32__ */
#endif  /* _WIN32 */

    /* Set to 1 to use pdftoppm; 0 for pdfimages */
#define   USE_PDFTOPPM     1

#include "string.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "allheaders.h"

    /* Special version */
PIX *pixConvertTo8Special(PIX *pix);

l_int32 main(int    argc,
             char **argv)
{
char         buf[256];
char        *basedir, *fname, *tail, *basename, *imagedir, *outfile, *firstpath;
l_int32      thresh, res, rotation, i, n, ret;
PIX         *pixs, *pix1, *pix2, *pix3, *pix4, *pix5;
SARRAY      *sa;
static char  mainName[] = "cleanpdf";

    if (argc != 5 && argc != 6)
        return ERROR_INT(
            "Syntax: cleanpdf basedir threshold resolution outfile [rotation]",
            mainName, 1);
    basedir = argv[1];
    thresh = atoi(argv[2]);
    res = atoi(argv[3]);
    outfile = argv[4];
    if (argc == 6)
        rotation = atoi(argv[5]);
    else
        rotation = 0;
    if (rotation < 0 || rotation > 3) {
        L_ERROR("rotation not in valid set {0,1,2,3}; setting to 0", mainName);
        rotation = 0;
    }
    if (res == 0)
        res = 300;
    if (res != 300 && res != 600) {
        L_ERROR("invalid res = %d; res must be in {0, 300, 600}\n",
                mainName, res);
        return 1;
    }
    setLeptDebugOK(1);

#if 1
        /* Get the names of the pdf files */
    if ((sa = getSortedPathnamesInDirectory(basedir, "pdf", 0, 0)) == NULL)
        return ERROR_INT("files not found", mainName, 1);
    sarrayWriteStderr(sa);
    n = sarrayGetCount(sa);
#endif

        /* Rasterize: use either
         *     pdftoppm -r 300 fname outroot  (-r 300 renders output at 300 ppi)
         * or
         *     pdfimages -j fname outroot   (-j outputs jpeg if input is dct)
         * Use of pdftoppm:
         *    This works on all pdf pages, both wrapped images and pages that
         *    were made orthographically.  The default output resolution for
         *    pdftoppm is 150 ppi, but we use 300 ppi.  This makes large
         *    uncompressed files (e.g., a standard size RGB page image at 300
         *    ppi is 25 MB), but it is very fast.  This is now preferred over
         *    using pdfimages.
         * Use of pdfimages:
         *    This only works when all pages are pdf wrappers around images.
         *    In some cases, it scrambles the order of the output pages
         *    and inserts extra images. */
    imagedir = stringJoin(basedir, "/image");
#if 1
#ifndef _WIN32
    mkdir(imagedir, 0777);
#else
    _mkdir(imagedir);
#endif  /* _WIN32 */
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        splitPathAtDirectory(fname, NULL, &tail);
        splitPathAtExtension(tail, &basename, NULL);
  #if USE_PDFTOPPM
        snprintf(buf, sizeof(buf), "pdftoppm -r 300 %s %s/%s",
                 fname, imagedir, basename);
  #else
        snprintf(buf, sizeof(buf), "pdfimages -j %s %s/%s",
                 fname, imagedir, basename);
  #endif  /* USE_PDFTOPPM */
        lept_free(tail);
        lept_free(basename);
        lept_stderr("%s\n", buf);
        ret = system(buf);   /* pdfimages or pdftoppm */
    }
    sarrayDestroy(&sa);
#endif

#if 1
        /* Clean, deskew and compress */
    sa = getSortedPathnamesInDirectory(imagedir, NULL, 0, 0);
    sarrayWriteStderr(sa);
    n = sarrayGetCount(sa);
    firstpath = NULL;
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        pixs = pixRead(fname);
        pix1 = pixConvertTo8Special(pixs);
        if (rotation > 0)
            pix2 = pixRotateOrth(pix1, rotation);
        else
            pix2 = pixClone(pix1);
        pix3 = pixFindSkewAndDeskew(pix2, 2, NULL, NULL);
        pix4 = pixBackgroundNormSimple(pix3, NULL, NULL);
        pixGammaTRC(pix4, pix4, 2.0, 50, 250);
        if (res == 300)
            pix5 = pixThresholdToBinary(pix4, thresh);
        else  /* res == 600 */
            pix5 = pixScaleGray2xLIThresh(pix4, thresh);
        splitPathAtDirectory(fname, NULL, &tail);
        splitPathAtExtension(tail, &basename, NULL);
        snprintf(buf, sizeof(buf), "%s/%s.tif", imagedir, basename);
        lept_stderr("%s\n", buf);
        pixWrite(buf, pix5, IFF_TIFF_G4);
        if (i == 0)  /* save full path to first image */
            firstpath = stringNew(buf);
        pixDestroy(&pixs);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pix3);
        pixDestroy(&pix4);
        pixDestroy(&pix5);
        lept_free(tail);
        lept_free(basename);
    }
    sarrayDestroy(&sa);
#endif

#if 1
        /* Generate the pdf.  Compute the actual input resolution from
         * the pixel dimensions of the first image.  This will cause each
         * page to be printed to cover an 8.5 x 11 inch sheet of paper.
         * We use flate encoding to avoid photometric reversal which
         * happens when encoded with G4 tiff.  */
    lept_stderr("Write output to %s\n", outfile);
    pix1 = pixRead(firstpath);
    pixInferResolution(pix1, 11.0, &res);
    pixDestroy(&pix1);
    lept_free(firstpath);
    convertFilesToPdf(imagedir, "tif", res, 1.0, L_G4_ENCODE,
                      0, NULL, outfile);
#endif

    return 0;
}


    /* A special version of pixConvertTo8() that returns an image without
     * a colormap and uses pixConvertRGBToGrayMinMax() to strongly
     * render color into black. */
PIX *
pixConvertTo8Special(PIX  *pixs)
{
    l_int32 d = pixGetDepth(pixs);
    if (d == 1) {
        return pixConvert1To8(NULL, pixs, 255, 0);
    } else if (d == 2) {
        return pixConvert2To8(pixs, 0, 85, 170, 255, FALSE);
    } else if (d == 4) {
        return pixConvert4To8(pixs, FALSE);
    } else if (d == 8) {
        if (pixGetColormap(pixs) != NULL)
            return pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
        else
            return pixCopy(NULL, pixs);
    } else if (d == 16) {
        return pixConvert16To8(pixs, L_MS_BYTE);
    } else if (d == 32) {
        return pixConvertRGBToGrayMinMax(pixs, L_CHOOSE_MIN);
    }

    L_ERROR("Invalid depth d = %d\n", "pixConvertSpecialTo8", d);
    return NULL;
}
