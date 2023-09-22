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
 * compresspdf.c
 *
 *    This program concatenates all pdfs in a directory by rendering them
 *    as images, optionally scaling the images, and generating an output pdf.
 *    The pdfs are taken in lexical order.  Pages are encoded with either
 *    tiffg4 or jpeg (DCT), or a mixture of them depending on input parameters
 *    and page color content.  For DCT encoding, the jpeg quality factor
 *    can be used to trade off the size of the resulting pdf against
 *    the image quality.
 *
 *    If the pages are monochrome (black and white), use of the %onebit
 *    flag will achieve better compression with less distortion.
 *    If most of the pages are black and white, but some have color that
 *    needs to be saved, input parameters %onebit and %savecolor should
 *    be both set to 1.  Then the pages with color are compressed with DCT
 *    and the monochrome pages are compressed with tiffg4.
 *
 *    The first step is to render the images as RGB, using Poppler's pdftoppm.
 *    Compare compresspdf with cleanpdf, which carries out several cleanup
 *    operations, such as deskewing and adaptive thresholding to clean
 *    noisy or dark backgrounds in grayscale or color images, resulting
 *    in high resolution, 1 bpp tiffg4 encoded images in the pdf.
 *
 *      Syntax:
 *       compresspdf basedir imres scalefactor onebit savecolor
 *                   quality title fileout
 *
 *    The %basedir is a directory where the input pdf files are located.
 *    The program will operate on every file in this directory with
 *    the ".pdf" extension.
 *
 *    The %imres is the desired resolution of the rasterization from the
 *    pdf page to a page image.  Two choices are allowed: 150 and 300 ppi.
 *    Use 0 for default (150 ppi).  The actual resolution used by the
 *    renderer depends on the page image size and is computed internally.
 *    We limit the maximum resolution to 300 ppi because these images are
 *    RGB uncompressed and are large: 6.3 MB for 150 ppi and 25 MB for 300 ppi.
 *
 *    The %scalefactor is typically used to downscale the image to
 *    reduce the size of the generated pdf.  It should not affect the
 *    pdf display otherwise.  For normal text on images scanned at 300 ppi,
 *    a 2x reduction (%scalefactor = 0.5) may be satisfactory.
 *    We compute an output resolution for that pdf that will cause it
 *    to print 11 inches high, based on the height in pixels of the
 *    first image in the set.
 *
 *    Images are saved in the ./image directory as RGB in ppm format.
 *    If the %onebit flag is 0, these will be encoded in the output pdf
 *    using DCT.  To force the images to be 1 bpp, with tiffg4 encoding, set
 *    the $onebit flag to 1.
 *
 *    The %savecolor flag is ignored unless %onebit is 1.  In that case,
 *    if %savecolor is 1, the image is tested for color content, and if
 *    even a relatively small amount is found, the image will be encoded
 *    with DCT instead of tiffg4.
 *
 *    The %quality is the jpeg output quality factor for images stored
 *    in the pdf.  Use 0 for the default value (50), which is satisfactory
 *    for many purposes.  Use 75 for standard jpeq quality; 85-95 are very
 *    high quality.  Allowed values are between 25 and 95.
 *
 *    The %title is the title given to the pdf.  Use %title == "none"
 *    to omit the title.
 *
 *    The pdf output is written to %fileout.  It is advisable (but not
 *    required) to have a '.pdf' extension.
 *
 *    The intent is to use pdftoppm to render the images at 150 pixels/inch
 *    for a full page, when scalefactor = 1.0.  The renderer uses the
 *    mediaboxes to decide how big to make the images.  If those boxes
 *    have values that are too large, the intermediate ppm images can
 *    be very large.  To prevent that, we compute the resolution to input
 *    to pdftoppm that results in RGB ppm images representing page images
 *    at about 150 ppi (when scalefactor = 1.0).  These images are about
 *    6MB, but are written quickly because there is no compression.
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
char       buf[256];
char      *basedir, *fname, *tail, *basename, *imagedir, *title, *fileout;
l_int32    imres, render_res, onebit, savecolor, quality, i, n, ret;
l_float32  scalefactor;
SARRAY    *sa;

    if (argc != 9)
        return ERROR_INT(
            "Syntax: compresspdf basedir imres scalefactor "
                     "onebit savecolor quality title fileout", __func__, 1);
    basedir = argv[1];
    imres = atoi(argv[2]);
    scalefactor = atof(argv[3]);
    onebit = atoi(argv[4]);  /* set to 1 to enforce 1 bpp tiffg4 encoding */
    savecolor = atoi(argv[5]);  /* if onebit == 1, set to 1 to save color */
    quality = atoi(argv[6]);  /* jpeg quality */
    title = argv[7];
    fileout = argv[8];
    setLeptDebugOK(1);
    if (imres == 0) imres = 150;  /* default value */
    if (imres != 150 && imres != 300) {
        L_WARNING("imres = %d must be 150 or 300; setting to 150\n",
                  __func__, imres);
        imres = 150;
    }
    if (quality <= 0) quality = 50;  /* default value */
    if (quality < 25) {
        L_WARNING("quality = %d is too low; setting to 25\n",
                  __func__, quality);
        quality = 25;
    }
    if (quality > 95) {
        L_WARNING("quality = %d is too high; setting to 95\n",
                  __func__, quality);
        quality = 95;
    }

        /* Set up a directory for temp images */
    imagedir = stringJoin(basedir, "/image");
#ifndef _WIN32
    mkdir(imagedir, 0777);
#else
    _mkdir(imagedir);
#endif  /* _WIN32 */

        /* Get the names of the pdf files */
    if ((sa = getSortedPathnamesInDirectory(basedir, "pdf", 0, 0)) == NULL)
        return ERROR_INT("files not found", __func__, 1);
    sarrayWriteStderr(sa);
    n = sarrayGetCount(sa);

        /* Use the first pdf file in the directory to estimate the
         * resolution to use with the image renderer that will generate
         * page images with a resolution of either about 150 ppi
         * (which is the default) or about 300 ppi for special cases.
         * At 150 and 300 ppi, the page images have maximum dimensions
         * of about 1650 and 3300 pixels, respectively.  These are the
         * uncompressed images, written to file, from which the compressed
         * images will be generated.  */
    fname = sarrayGetString(sa, 0, L_NOCOPY);
    getPdfRendererResolution(fname, imagedir, &render_res);  /* for 300 ppi */
    if (imres == 150) render_res /= 2;

        /* Rasterize:
         *     pdftoppm -r 150 fname outroot   [max dimension about 1650 pixels]
         *     pdftoppm -r 300 fname outroot   [max dimension about 3300 pixels]
         * Use of pdftoppm:
         *    This works on all pdf pages, both wrapped images and pages that
         *    were made orthographically.  */
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        splitPathAtDirectory(fname, NULL, &tail);
        splitPathAtExtension(tail, &basename, NULL);
        snprintf(buf, sizeof(buf), "pdftoppm -r %d %s %s/%s",
                 render_res, fname, imagedir, basename);
        lept_free(tail);
        lept_free(basename);
        lept_stderr("%s\n", buf);
        ret = system(buf);
    }
    sarrayDestroy(&sa);

        /* Optionally binarize, then scale and collect all images in memory.
         * If n > 100, use pixacomp instead of pixa to store everything
         * before generating the pdf.
         * When using the onebit option, It is important to binarize
         * the images in leptonica.  Do not let 'pdftoppm -mono' do
         * the binarization, because it will apply error-diffusion
         * dithering to gray and color images. */
    sa = getSortedPathnamesInDirectory(imagedir, NULL, 0, 0);
    lept_free(imagedir);
    sarrayWriteStderr(sa);
    lept_stderr("compressing ...\n");
    compressFilesToPdf(sa, onebit, savecolor, scalefactor, quality,
                       title, fileout);

    return 0;
}

