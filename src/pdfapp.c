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

/*!
 * \file pdfapp.c
 * <pre>
 *
 *    Image processing operations on multiple images followed by wrapping
 *    them into a pdf.
 *
 *    |=============================================================|
 *    |                        Important notes                      |
 *    |=============================================================|
 *    | Some of these functions require I/O libraries such as       |
 *    | libtiff, libjpeg, libpng and libz.  If you do not have      |
 *    | these libraries, some calls will fail.  For example,        |
 *    | if you do not have libtiff, you cannot write a pdf that     |
 *    | uses libtiff to encode bilevel images in tiffg4.            |
 *    |                                                             |
 *    | You can manually deactivate all pdf writing by setting      |
 *    | this in environ.h:                                          |
 *    | \code                                                       |
 *    |      #define  USE_PDFIO     0                               |
 *    | \endcode                                                    |
 *    | This will link the stub file pdfappstub.c.                  |
 *    |=============================================================|
 *
 *     The images in the pdf file can be rendered using a pdf viewer,
 *     such as evince, gv, xpdf or acroread.
 *
 *     Compression of images for prog/compresspdf
 *          l_int32          compressFilesToPdf()
 *
 *     Crop images for prog/croppdf
 *          l_int32          cropFilesToPdf()
 *
 *     Cleanup and binarization of images for prog/cleanpdf
 *          l_int32          cleanTo1bppFilesToPdf()
 *
 *     Static helpers:
 *       Version of pixConvertTo8() that removes cmap and pushes color to black
 *          static PIX      *pixConvertTo8Special()
 *
 *       Adaptive binarization with darkening helper function
 *          static PIX      *pixAdaptTo1bppAndDarken()
 *
 * </pre>
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include "allheaders.h"

    /* Static helper */
static PIX *pixConvertTo8Special(PIX *pixs);
static PIX *pixAdaptTo1bppAndDarken(PIX *pixs, l_int32 darken, l_int32 res);

/* --------------------------------------------*/
#if  USE_PDFIO   /* defined in environ.h */
 /* --------------------------------------------*/

/*---------------------------------------------------------------------*
 *              Compression of images for prog/compresspdf             *
 *---------------------------------------------------------------------*/
/*!
 * \brief   compressFilesToPdf()
 *
 * \param[in]    sa            sorted full pathnames of images
 * \param[in]    onebit        set to 1 to enforce 1 bpp tiffg4 encoding
 * \param[in]    savecolor     if %onebit == 1, set to 1 to save color
 * \param[in]    scalefactor   scaling factor applied to each image; > 0.0
 * \param[in]    quality       for jpeg: 0 for default (50; otherwise 25 - 95.
 * \param[in]    title         [optional] pdf title; can be null
 * \param[in]    fileout       pdf file of all images
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *    (1) This function is designed to optionally scale and compress a set of
 *        images, wrapping them in a pdf in the order given in the input %sa.
 *    (2) It does the image processing for prog/compresspdf.c.
 *    (3) Images in the output pdf are encoded with either tiffg4 or jpeg (DCT),
 *        or a mixture of them depending on parameters %onebit and %savecolor.
 *    (4) Parameters %onebit and %savecolor work as follows:
 *        %onebit = 0: no depth conversion, default encoding depends on depth
 *        %onebit = 1, %savecolor = 0: all images converted to 1 bpp
 *        %onebit = 1, %savecolor = 1: images without color are converted
 *           to 1 bpp; images with color have the color preserved.
 *    (5) In use, if most of the pages are 1 bpp but some have color that needs
 *        to be preserved, %onebit and %savecolor should both be 1.  This
 *        causes DCT compression of color images and tiffg4 compression
 *        of monochrome images.
 *    (6) The images will be concatenated in the order given in %sa.
 *    (7) The scalefactor is applied to each image before encoding.
 *        If you enter a value <= 0.0, it will be set to 1.0.
 *    (8) Default jpeg quality is 50; otherwise, quality factors between
 *        25 and 95 are enforced.
 *    (9) Page images at 300 ppi are about 8 Mpixels.  RGB(A) rasters are
 *        then about 32 MB (1 bpp images are about 1 MB).  If there are
 *        more than 25 images, store the images after processing as an
 *        array of compressed images (a Pixac); otherwise, use a Pixa.
 * </pre>
 */
l_ok
compressFilesToPdf(SARRAY      *sa,
                   l_int32      onebit,
                   l_int32      savecolor,
                   l_float32    scalefactor,
                   l_int32      quality,
                   const char  *title,
                   const char  *fileout)
{
char      *fname;
l_int32    n, i, res;
l_int32    maxsmallset = 25;  /* max num images kept uncompressed in array */
l_float32  colorfract;
PIX       *pixs, *pix1, *pix2;
PIXA      *pixa1 = NULL;
PIXAC     *pixac1 = NULL;

    if (!sa)
        return ERROR_INT("sa not defined", __func__, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", __func__, 1);
    if (scalefactor <= 0) scalefactor = 1.0;
    if (quality <= 0) quality = 50;  /* default value */
    if (quality < 25) {
        L_WARNING("quality %d too low; setting to 25\n", __func__, quality);
        quality = 25;
    }
    if (quality > 95) {
        L_WARNING("quality %d too high; setting to 95\n", __func__, quality);
        quality = 95;
    }
    if ((n = sarrayGetCount(sa)) == 0)
        return ERROR_INT("sa is empty", __func__, 1);

    if (n <= maxsmallset)
        pixa1 = pixaCreate(n);
    else
        pixac1 = pixacompCreate(n);
    for (i = 0; i < n; i++) {
        if (i == 0)
            lept_stderr("page: ");
        else if (i % 10 == 0)
            lept_stderr("%d . ", i);
        fname = sarrayGetString(sa, i, L_NOCOPY);
        pixs = pixRead(fname);
        if (onebit) {
            if (savecolor) {
                pixColorFraction(pixs, 40, 224, 80, 4, NULL, &colorfract);
                if (colorfract > 0.01)  /* save the color; DCT encoding */
                    pix1 = pixClone(pixs);
                else
                    pix1 = pixConvertTo1(pixs, 180);
            } else {  /* do not save any color; tiffg4 encoding */
                pix1 = pixConvertTo1(pixs, 180);
            }
        } else {  /* default encoding: tiffg4 for 1 bpp; DCT for all else */
            pix1 = pixClone(pixs);
        }
        if (scalefactor == 1.0)
            pix2 = pixClone(pix1);
        else
            pix2 = pixScale(pix1, scalefactor, scalefactor);
        if (n <= maxsmallset) {
            pixaAddPix(pixa1, pix2, L_INSERT);
        } else {
            pixacompAddPix(pixac1, pix2, IFF_DEFAULT);
            pixDestroy(&pix2);
        }
        pixDestroy(&pixs);
        pixDestroy(&pix1);
    }

        /* Generate the pdf.  Compute the actual input resolution from
         * the pixel dimensions of the first image.  This will cause each
         * page to be printed to cover an 8.5 x 11 inch sheet of paper. */
    lept_stderr("\nWrite output to %s\n", fileout);
    if (n <= maxsmallset)
        pix1 = pixaGetPix(pixa1, 0, L_CLONE);
    else
        pix1 = pixacompGetPix(pixac1, 0);
    pixInferResolution(pix1, 11.0, &res);
    pixDestroy(&pix1);
    if (strcmp(title, "none") == 0)
        title = NULL;
    if (n <= maxsmallset) {
        pixaConvertToPdf(pixa1, res, 1.0, L_DEFAULT_ENCODE, quality,
                         title, fileout);
        pixaDestroy(&pixa1);
    } else {
        pixacompConvertToPdf(pixac1, res, 1.0, L_DEFAULT_ENCODE, quality,
                             title, fileout);
        pixacompDestroy(&pixac1);
    }
    return 0;
}


/*---------------------------------------------------------------------*
 *                       Crop images for prog/croppdf                  *
 *---------------------------------------------------------------------*/
/*!
 * \brief   cropFilesToPdf()
 *
 * \param[in]    sa            sorted full pathnames of images
 * \param[in]    threshold     threshold for binarization
 * \param[in]    lr_clear      full res pixels cleared at left and right sides
 * \param[in]    tb_clear      full res pixels cleared at top and bottom sides
 * \param[in]    edgeclean     parameter for removing edge noise (0-15)
 *                             default = 0 (no removal);
 * \param[in]    lr_add        full res expansion of crop box on left and right
 * \param[in]    tb_add        full res expansion of crop box on top and bottom
 * \param[in]    title         [optional] pdf title; can be null
 * \param[in]    fileout       pdf file of all images
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *    (1) This function is designed to optionally remove white space from
 *        around the page images, and generate a pdf that prints with
 *        foreground occupying much of the full page.
 *    (2) It does the image processing for prog/croppdf.c.
 *    (3) Images in the output pdf are 1 bpp and encoded with tiffg4.
 *    (4) See documentation in pixCropImage() for details on the processing.
 *    (5) The images will be concatenated in the order given in %sa.
 *    (6) Page images at 300 ppi are about 1 Mpixels.  We allow up to 200
 *        uncompressed rasters to be stored in memory.  If more than 200
 *        pages, the stored images are compressed with tiffg4.
 * </pre>
 */
l_ok
cropFilesToPdf(SARRAY      *sa,
               l_int32      threshold,
               l_int32      lr_clear,
               l_int32      tb_clear,
               l_int32      edgeclean,
               l_int32      lr_add,
               l_int32      tb_add,
               const char  *title,
               const char  *fileout)
{
char      *fname;
l_int32    n, i, res;
l_int32    maxsmallset = 200;  /* max num images kept uncompressed in array */
PIX       *pixs, *pix1;
PIXA      *pixa1 = NULL;
PIXAC     *pixac1 = NULL;

    if (!sa)
        return ERROR_INT("sa not defined", __func__, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", __func__, 1);
    if ((n = sarrayGetCount(sa)) == 0)
        return ERROR_INT("sa is empty", __func__, 1);

    if (n <= maxsmallset)
        pixa1 = pixaCreate(n);
    else
        pixac1 = pixacompCreate(n);
    for (i = 0; i < n; i++) {
        if (i == 0)
            lept_stderr("page: ");
        else if (i % 10 == 0)
            lept_stderr("%d . ", i);
        fname = sarrayGetString(sa, i, L_NOCOPY);
        pixs = pixRead(fname);
        pix1 = pixCropImage(pixs, threshold, lr_clear, tb_clear, edgeclean,
                            lr_add, tb_add, NULL, NULL);
        if (n <= maxsmallset)
            pixaAddPix(pixa1, pix1, L_INSERT);
        else
            pixacompAddPix(pixac1, pix1, IFF_TIFF_G4);
        pixDestroy(&pixs);
    }

        /* Generate the pdf.  Compute the actual input resolution from
         * the pixel dimensions of the first image.  This will cause each
         * page to be printed to cover an 8.5 x 11 inch sheet of paper. */
    lept_stderr("\nWrite output to %s\n", fileout);
    if (n <= maxsmallset)
        pix1 = pixaGetPix(pixa1, 0, L_CLONE);
    else
        pix1 = pixacompGetPix(pixac1, 0);
    pixInferResolution(pix1, 11.0, &res);
    pixDestroy(&pix1);
    if (strcmp(title, "none") == 0)
        title = NULL;
    if (n <= maxsmallset) {
        pixaConvertToPdf(pixa1, res, 1.0, L_G4_ENCODE, 0, title, fileout);
        pixaDestroy(&pixa1);
    } else {
        pixacompConvertToPdf(pixac1, res, 1.0, L_G4_ENCODE, 0, title, fileout);
        pixacompDestroy(&pixac1);
    }
    return 0;
}


/*---------------------------------------------------------------------*
 *        Cleanup and binarization of images for prog/cleanpdf         *
 *---------------------------------------------------------------------*/
/*!
 * \brief   cleanTo1bppFilesToPdf()
 *
 * \param[in]    sa            sorted full pathnames of images
 * \param[in]    res           either 300 or 600 ppi for output
 * \param[in]    darken        increase contrast: 0 = none; 9 = max
 * \param[in]    rotation      cw by 90 degrees: {0,1,2,3} represent
 *                             0, 90, 180 and 270 degree cw rotations
 * \param[in]    opensize      opening size of structuring element for noise
 *                             removal: {0 to skip; 2, 3}
 * \param[in]    title         [optional] pdf title; can be null
 * \param[in]    fileout       pdf file of all images
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *    (1) This deskews, optionally rotates and darkens, cleans background
 *        to white, binarizes and optionally removes small noise, and
 *        put the images into the pdf in the order given in %sa.
 *    (2) It does the image processing for prog/cleanpdf.c.
 *    (3) All images in the pdf are tiffg4 encoded.
 *    (4) For color and grayscale input, local background normalization is
 *        done to 200, and a threshold of 180 sets the maximum foreground
 *        value in the normalized image.
 *    (5) The %res parameter can be either 300 or 600 ppi.  If the input
 *        is gray or color and %res = 600, this does an interpolated 2x
 *        expansion before binarizing.
 *    (6) The %darken parameter adjusts the binarization to avoid losing
 *        lighter input pixels.  Contrast is increased as %darken increases
 *        from 0 (no adjustment) to a maximum of 9.
 *    (7) The #opensize parameter is the size of a square SEL used with
 *        opening to remove small speckle noise.  Allowed open sizes are 2,3.
 *        If this is to be used, try 2 before 3.
 *    (8) If there are more than 200 images, store the images after processing
 *        as an array of compressed images (a Pixac); otherwise, use a Pixa.
 * </pre>
 */
l_ok
cleanTo1bppFilesToPdf(SARRAY      *sa,
                      l_int32      res,
                      l_int32      darken,
                      l_int32      rotation,
                      l_int32      opensize,
                      const char  *title,
                      const char  *fileout)
{
char       sequence[32];
char      *fname;
l_int32    n, i;
l_int32    maxsmallset = 200;  /* max num images kept uncompressed in array */
PIX       *pixs, *pix1, *pix2, *pix3, *pix4, *pix5;
PIXA      *pixa1 = NULL;
PIXAC     *pixac1 = NULL;

    if (!sa)
        return ERROR_INT("sa not defined", __func__, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", __func__, 1);
    if (res == 0) res = 300;
    if (res != 300 && res != 600) {
        L_ERROR("invalid res = %d; res must be in {0, 300, 600}\n",
                __func__, res);
        return 1;
    }
    if (darken < 0 || darken > 9) {
        L_ERROR("invalid darken = %d; darken must be in {0,...,9}\n",
                __func__, darken);
        return 1;
    }
    if (rotation < 0 || rotation > 3) {
        L_ERROR("invalid rotation = %d; rotation must be in  {0,1,2,3}\n",
                __func__, rotation);
        return 1;
    }
    if (opensize > 3) {
        L_ERROR("invalid opensize = %d; opensize must be <= 3\n",
                __func__, opensize);
        return 1;
    }
    if ((n = sarrayGetCount(sa)) == 0)
        return ERROR_INT("sa is empty", __func__, 1);

    if (n <= maxsmallset)
        pixa1 = pixaCreate(n);
    else
        pixac1 = pixacompCreate(n);
    for (i = 0; i < n; i++) {
        fname = sarrayGetString(sa, i, L_NOCOPY);
        pixs = pixRead(fname);
        pix1 = pixConvertTo8Special(pixs);
        if (rotation > 0)
            pix2 = pixRotateOrth(pix1, rotation);
        else
            pix2 = pixClone(pix1);
        pix3 = pixFindSkewAndDeskew(pix2, 2, NULL, NULL);
        pix4 = pixAdaptTo1bppAndDarken(pix3, darken, res);
        if (opensize == 2 || opensize == 3) {
            snprintf(sequence, sizeof(sequence), "o%d.%d", opensize, opensize);
            pix5 = pixMorphSequence(pix4, sequence, 0);
        } else {
            pix5 = pixClone(pix4);
        }
        if (n <= maxsmallset) {
            pixaAddPix(pixa1, pix5, L_INSERT);
        } else {
            pixacompAddPix(pixac1, pix5, IFF_TIFF_G4);
            pixDestroy(&pix5);
        }
        pixDestroy(&pixs);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pix3);
        pixDestroy(&pix4);
    }

        /* Generate the pdf.  Compute the actual input resolution from
         * the pixel dimensions of the first image.  This will cause each
         * page to be printed to cover an 8.5 x 11 inch sheet of paper. */
    lept_stderr("Write output to %s\n", fileout);
    if (n <= maxsmallset)
        pix1 = pixaGetPix(pixa1, 0, L_CLONE);
    else
        pix1 = pixacompGetPix(pixac1, 0);
    pixInferResolution(pix1, 11.0, &res);
    pixDestroy(&pix1);
    if (strcmp(title, "none") == 0)
        title = NULL;

    if (n <= maxsmallset) {
        pixaConvertToPdf(pixa1, res, 1.0, L_G4_ENCODE, 0, title, fileout);
        pixaDestroy(&pixa1);
    } else {
        pixacompConvertToPdf(pixac1, res, 1.0, L_G4_ENCODE, 0, title, fileout);
        pixacompDestroy(&pixac1);
    }

    return 0;
}


    /* ----------------- Static helper functions ----------------- */

/*!
 * \brief   pixConvertTo8Special()
 *
 * \param[in]    pixs       any depth, with or without colormap
 * \return  8 bpp pix if OK; NULL on error
 *
 * <pre>
 * Notes:
 *    (1) This is a special version of pixConvert1To8() that removes any
 *        existing colormap and uses pixConvertRGBToGrayMinMax()
 *        to strongly render color into black.
 * </pre>
 */
static PIX *
pixConvertTo8Special(PIX  *pixs)
{
    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", __func__, NULL);

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

    L_ERROR("Invalid depth d = %d\n", __func__, d);
    return NULL;
}


/*!
 * \brief   pixAdaptTo1bppAndDarken()
 *
 * \param[in]    pixs         8 bpp grayscale
 * \param[in]    darken       increase contrast: 0 = none; 9 = max
 * \param[in]    res          either 300 or 600 ppi for output
 * \return  pixd (1 bpp) if OK, NULL on error
 *
 * <pre>
 * Notes:
 *    (1) This does background normalization with optional contrast
 *        enhancement, and binarizes to either 300 ppi or, with
 *        linear interpolation, to 600 ppi.
 *    (2) The caller must check that darken is in the set {0, ... 9}
 *        and res is in {300, 600}.
 * </pre>
 */
static PIX *
pixAdaptTo1bppAndDarken(PIX     *pixs,
                        l_int32  darken,
                        l_int32  res)
{
PIX  *pix1, *pixd;

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", __func__, NULL);
    pix1 = pixBackgroundNormSimple(pixs, NULL, NULL);

        /* Use a TRC to darken weak images */
    if (darken == 0)
        pixGammaTRC(pix1, pix1, 2.0, 50, 220);
    else if (darken == 1)
        pixGammaTRC(pix1, pix1, 1.8, 60, 215);
    else if (darken == 2)
        pixGammaTRC(pix1, pix1, 1.6, 70, 215);
    else if (darken == 3)
        pixGammaTRC(pix1, pix1, 1.4, 80, 210);
    else if (darken == 4)
        pixGammaTRC(pix1, pix1, 1.2, 90, 210);
    else if (darken == 5)
        pixGammaTRC(pix1, pix1, 1.0, 100, 210);
    else if (darken == 6)
        pixGammaTRC(pix1, pix1, 0.85, 110, 205);
    else if (darken == 7)
        pixGammaTRC(pix1, pix1, 0.7, 120, 205);
    else if (darken == 8)
        pixGammaTRC(pix1, pix1, 0.6, 130, 200);
    else  /* darken == 9 */
        pixGammaTRC(pix1, pix1, 0.5, 140, 195);

        /* Binarize the normalized image, using threshold = 180 */
    if (res == 300)
        pixd = pixThresholdToBinary(pix1, 180);
    else  /* res == 600 */
        pixd = pixScaleGray2xLIThresh(pix1, 180);
    pixDestroy(&pix1);
    return pixd;
}


/* --------------------------------------------*/
#endif  /* USE_PDFIO */
/* --------------------------------------------*/
