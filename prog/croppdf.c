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
 * croppdf.c
 *
 *    This program concatenates all pdfs in a directory by rendering them
 *    as images, cropping each image to the foreground region with
 *    options for noise removal and margins, slightly thickens long
 *    horizontal lines (e.g., of a music staff), and to some extent
 *    scales the width to fill a printed page.  See documentation for
 *    pixCropImage() for the parameters.
 *
 *    The pdfs are concatenated in lexical order, and each image
 *    is encoded with tiffg4.
 *
 *    Syntax:
 *       croppdf basedir lrclear tbclear edgeclean lradd tbadd maxwiden
 *       title fileout
 *
 *    The %basedir is a directory where the input pdf files are located.
 *    The program will operate on every file in this directory with
 *    the ".pdf" extension.
 *
 *    The %lrclear and %tbclear parameters give the number of background
 *    pixels to be added to the foreground region.
 *
 *    The %edgeclean parameter is used to remove edge noise, going from
 *    0 (default, no removal) to 15 (maximally aggressive removal).
 *
 *    The suggested value for %lradd and %tbadd is 50.  Laser printers do not
 *    print foreground pixels very close to the page edges, and using a
 *    margin of 50 pixels (1/6" at 300 ppi) should allow all foregrounnd
 *    pixels to be printed.
 *
 *    The %maxwiden parameter allows the foreground to better fill an
 *    8.5 x 11 inch printed page.  It gives the maximum fractional horizontal
 *    stretching allowed.  Suggested values are between 1.0 and 1.15.
 *
 *    The %title is the title given to the pdf.  Use %title == "none"
 *    to omit the title.
 *
 *    The pdf output is written to %fileout.  It is advisable (but not
 *    required) to have a '.pdf' extension.
 *
 *    As the first step in processing, images are saved in the ./image
 *    directory as RGB at 300 ppi in ppm format.  Each image is about 26MB.
 *    Delete those images after use.
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
l_int32    lrclear, tbclear, edgeclean, lradd, tbadd;
l_int32    render_res, i, n, ret;
l_float32  maxwiden;
SARRAY    *sa;

    if (argc != 10)
        return ERROR_INT(
            "Syntax: croppdf basedir lrclear tbclear edgeclean "
            "lradd tbadd maxwiden title fileout", __func__, 1);
    basedir = argv[1];
    lrclear = atoi(argv[2]);
    tbclear = atoi(argv[3]);
    edgeclean = atoi(argv[4]);
    lradd = atoi(argv[5]);
    tbadd = atoi(argv[6]);
    maxwiden = atof(argv[7]);
    title = argv[8];
    fileout = argv[9];
    setLeptDebugOK(1);

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

        /* Figure out the resolution to use with the image renderer to
         * generate page images with a resolution of not more than 150 ppi.
         * These would have a maximum dimension of about 1650 pixels. 
         * Use the first pdf file in the directory.  */
    fname = sarrayGetString(sa, 0, L_NOCOPY);
    getPdfRendererResolution(fname, imagedir, &render_res);  /* for 300 ppi */

        /* Rasterize:
         *     pdftoppm -r 300 fname outroot
         * Use of pdftoppm:
         *    This works on all pdf pages, both wrapped images and pages that
         *    were made orthographically.  We generate images that are no
         *    larger than about 1650 pixels in the maximum direction. This
         *    makes uncompressed 6 MB files and is very fast.  If you want
         *    higher resolution 1 bpp output, use cleanpdf.c. */
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

        /* Process each image and collect all resulting 1 bpp images
         * in memory.  If n > 200, use pixacomp instead of pixa to
         * store the images before generating the pdf.  */
    sa = getSortedPathnamesInDirectory(imagedir, NULL, 0, 0);
    lept_free(imagedir);
    sarrayWriteStderr(sa);
    lept_stderr("cropping ...\n");
    cropFilesToPdf(sa, lrclear, tbclear, edgeclean, lradd, tbadd, maxwiden,
                   title, fileout);

    return 0;
}

