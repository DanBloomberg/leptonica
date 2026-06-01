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
 * rotateorthpdf.c
 *
 *    This program rotates individual page images in a pdf of images.
 *    It takes a rotation string that has three modes:
 *    (1) a list of integers in [0,1,2,3] for 90 degree cw rotations; only
 *        need to list up to last non-zero rotation
 *    (2) a 4, followed by one integer, to rotate all images by the same amount
 *    (3) a 5, followed by parenthesized comma-separated pairs of numbers:
 *    (page, rotation).
 *
 *    Syntax:
 *       rotateorthpdf filein imres rotstring scalefactor quality
 *                     title fileout
 *
 *    The %rotstring flag determines which, if any, images are rotated cw by
 *    multiples of 90 degrees.  There are 3 modes, and here are examples:
 *    Mode 1: "00201".  The third image is rotated by 180 degrees and
 *            the fifth by 90 degrees.  All others are not rotated.
 *    Mode 2: "41".  All images are rotated 90 degrees cw.
 *    Mode 3: "5(12,3)(19,1)".  Image 12 is rotated cw by 270 degrees and image
 *            19 is rotated by 90 degrees.  All others are not rotated.
 *    Note that images are numbered from 0, not from 1.
 *
 *    The %scalefactor is the scaling to be applied to each image.  You
 *    can use any positive value not exceeding 2.0.
 *
 *    The %quality is the jpeg output quality factor for images stored
 *    with DCT encoding in the pdf.  Use 0 for the default value (50),
 *    which is satisfactory for many purposes.  Use 75 for standard
 *    jpeq quality; 85-95 is very high quality.  Allowed values are
 *    between 25 and 95.
 *
 *    The %title is the title given to the pdf.  Use %title == "none"
 *    to omit the title.
 *
 *    The pdf output is written to %fileout.  It is advisable (but not
 *    required) to have a '.pdf' extension.
 *
 *    A typical application is to generate a pdf from a set of images in
 *    a directory.  As a preprocessing step, put the images into a pdf with
 *        converttopdf dir fileout
 *    and check the output file to see which images need to be rotated.
 *    Then run rotateorthpdf with a rotation string.
 *
 *    As the first step in processing, extracted images are saved, in
 *    their original compressed pdf encodings, in the directory
 *    /tmp/lept/renderpdf/.  We use pdfimages to do the rendering
 *    without scaling or transcoding.
 *
 *    N.B.  This requires running pdfimages from the Poppler package
 *          of pdf utilities  For non-unix systems, this requires
 *          installation of the cygwin Poppler package:
 *       https://cygwin.com/cgi-bin2/package-cat.cgi?file=x86/poppler/
 *              poppler-0.26.5-1
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#include "allheaders.h"

l_int32 main(int    argc,
             char **argv)
{
char       buf[256];
char      *rotstring, *filein, *title, *fileout;
l_int32    quality;
l_float32  scalefactor;
SARRAY    *safiles;

    if (argc != 7)
        return ERROR_INT(
            "Syntax: rotateorthpdf filein rotstring"
                    " scalefactor quality title fileout", __func__, 1);
    filein = argv[1];
    rotstring = argv[2];
    scalefactor = atof(argv[3]);
    quality = atoi(argv[4]);  /* jpeg quality */
    title = argv[5];
    fileout = argv[6];
    if (scalefactor <= 0.0) scalefactor = 1.0;
    if (scalefactor > 2.0) {
        L_WARNING("scalefactor %f too big; setting to 2.0\n", __func__, 
                  scalefactor);
        scalefactor = 2.0;
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
    setLeptDebugOK(1);

        /* Render all images from the pdf file.
         * We could call:
         *      l_pdfRenderFile(filein, 300, &safiles);
         * This renders the images in ppm format at approximately (but
         * not exactly) the same pixel dimensions as in their encoded
         * form within the pdf.  However, by calling
         *      l_pdfRenderUnscaledFile(filein, &safiles);
         * the images are extracted from the pdf in their originally
         * encoded form.  */
    if (l_pdfRenderUnscaledFile(filein, &safiles))
        return ERROR_INT_1("rendering failed from filein", filein,
                           __func__, 1);

        /* Optionally rotate, then scale and collect all images in memory.
         * If n > 100, use pixacomp instead of pixa to store everything
         * before generating the pdf. */
    lept_stderr("rotating ...\n");
    rotateorthFilesToPdf(safiles, rotstring, scalefactor, quality,
                         title, fileout);
    sarrayDestroy(&safiles);
    return 0;
}

