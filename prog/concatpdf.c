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
 * concatpdf.c
 *
 *    This program concatenates all pdfs in a directory by rendering them
 *    as images, optionally scaling the images, and generating an output pdf.
 *    The pdfs are taken in lexical order.
 *
 *    It makes no other changes to the images, which are rendered
 *    by Poppler's pdftoppm.  Compare with cleanpdf.c, which carries
 *    out several operations to make high resolution, 1 bpp g4-tiff
 *    encoded images in the pdf.
 *
 *     Syntax:  cconcatpdf basedir scalefactor outfile
 *
 *    The %basedir is a directory where the input pdf files are located.
 *    The program will operate on every file in this directory with
 *    the ".pdf" extension.
 *
 *    The %scalefactor is typically used to downscale the image to
 *    reduce the size of the generated pdf.  It should not affect the
 *    pdf display otherwise.  For normal text on images scanned at 300 ppi,
 *    a 2x reduction (%scalefactor = 0.5) may be satisfactory.
 *    We compute an output resolution for that pdf that will cause it
 *    to print 11 inches high, based on the height in pixels of the
 *    first image in the set.
 *
 *    The pdf encoding for each page is chosen by the default mechanism.
 *    See selectDefaultPdfEncoding() for details.
 *    If DCT encoding (jpeg) is used, the quality factor is set to 50.
 *    This makes smaller files with (usually) decent image quality.
 *
 *    The pdf output is written to %outfile.  It is advisable (but not
 *    required) to have a '.pdf' extension.
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

#include "string.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "allheaders.h"

l_int32 main(int    argc,
             char **argv)
{
char         buf[256];
char        *basedir, *fname, *tail, *basename, *imagedir, *outfile;
l_int32      res, i, n, ret;
l_float32    scalefactor;
PIX         *pixs, *pix1;
PIXA        *pixa1;
SARRAY      *sa;
static char  mainName[] = "concatpdf";

    if (argc != 4)
        return ERROR_INT(
            "Syntax: concatpdf basedir scalefactor outfile",
            mainName, 1);
    basedir = argv[1];
    scalefactor = atof(argv[2]);
    outfile = argv[3];
    setLeptDebugOK(1);

#if 1
        /* Get the names of the pdf files */
    if ((sa = getSortedPathnamesInDirectory(basedir, "pdf", 0, 0)) == NULL)
        return ERROR_INT("files not found", mainName, 1);
    sarrayWriteStderr(sa);
    n = sarrayGetCount(sa);
#endif

        /* Rasterize:
         *     pdftoppm -r 150 fname outroot
         * Use of pdftoppm:
         *    This works on all pdf pages, both wrapped images and pages that
         *    were made orthographically.  We use the default output resolution
         *    of 150 ppi for pdftoppm, which makes uncompressed 6 MB files
         *    and is very fast.  If you want higher resolution 1 bpp output,
         *    use cleanpdf.c. */
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
        snprintf(buf, sizeof(buf), "pdftoppm -r 150 %s %s/%s",
                 fname, imagedir, basename);
        lept_free(tail);
        lept_free(basename);
        lept_stderr("%s\n", buf);
        ret = system(buf);
    }
    sarrayDestroy(&sa);
#endif

#if 1
        /* Scale and collect */
    sa = getSortedPathnamesInDirectory(imagedir, NULL, 0, 0);
    sarrayWriteStderr(sa);
    n = sarrayGetCount(sa);
    pixa1 = pixaCreate(n);
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        pixs = pixRead(fname);
        if (scalefactor == 1.0)
            pix1 = pixClone(pixs);
        else
            pix1 = pixScale(pixs, scalefactor, scalefactor);
        pixaAddPix(pixa1, pix1, L_INSERT);
        pixDestroy(&pixs);
    }
    sarrayDestroy(&sa);
#endif

#if 1
        /* Generate the pdf.  Compute the actual input resolution from
         * the pixel dimensions of the first image.  This will cause each
         * page to be printed to cover an 8.5 x 11 inch sheet of paper. */
    lept_stderr("Write output to %s\n", outfile);
    pix1 = pixaGetPix(pixa1, 0, L_CLONE);
    pixInferResolution(pix1, 11.0, &res);
    pixDestroy(&pix1);
    pixaConvertToPdf(pixa1, res, 1.0, L_DEFAULT_ENCODE, 50, NULL, outfile);
    pixaDestroy(&pixa1);
#endif

    return 0;
}


