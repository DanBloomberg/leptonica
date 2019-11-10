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
 * \file webpanimio.c
 * <pre>
 *
 *    Writing animated WebP
 *          l_int32          pixaWriteWebPAnim()
 *          l_int32          pixaWriteStreamWebPAnim()
 *          l_int32          pixaWriteMemWebPAnim()
 * </pre>
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#include "allheaders.h"

/* -----------------------------------------------*/
#if  HAVE_LIBWEBP_ANIM   /* defined in environ.h  */
/* -----------------------------------------------*/
#include "webp/decode.h"
#include "webp/encode.h"
#include "webp/mux.h"
#include "webp/demux.h"

/*---------------------------------------------------------------------*
 *                       Writing animated WebP                         *
 *---------------------------------------------------------------------*/
/*!
 * \brief   pixaWriteWebPAnim()
 *
 * \param[in]    filename
 * \param[in]    pixa        with images of all depths; cmap OK
 * \param[in]    loopcount   [0 for infinite]
 * \param[in]    duration    in ms, for each image
 * \param[in]    quality     0 - 100 for lossy; default ~80
 * \param[in]    lossless    use 1 for lossless; 0 for lossy
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) Special top-level function allowing specification of quality.
 * </pre>
 */
l_ok
pixaWriteWebPAnim(const char  *filename,
                  PIXA        *pixa,
                  l_int32      loopcount,
                  l_int32      duration,
                  l_int32      quality,
                  l_int32      lossless)
{
l_int32  ret;
FILE    *fp;

    PROCNAME("pixaWriteWebPAnim");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    if ((fp = fopenWriteStream(filename, "wb+")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    ret = pixaWriteStreamWebPAnim(fp, pixa, loopcount, duration,
                                  quality, lossless);
    fclose(fp);
    if (ret)
        return ERROR_INT("pixs not compressed to stream", procName, 1);
    return 0;
}


/*!
 * \brief   pixaWriteStreamWebPAnim()
 *
 * \param[in]    fp          file stream
 * \param[in]    pixa        with images of all depths; cmap OK
 * \param[in]    loopcount   [0 for infinite]
 * \param[in]    duration    in ms, for each image
 * \param[in]    quality     0 - 100 for lossy; default ~80
 * \param[in]    lossless    use 1 for lossless; 0 for lossy
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) See pixWriteMemWebP() for details.
 *      (2) Use 'free', and not leptonica's 'LEPT_FREE', for all heap data
 *          that is returned from the WebP library.
 * </pre>
 */
l_ok
pixaWriteStreamWebPAnim(FILE    *fp,
                        PIXA    *pixa,
                        l_int32  loopcount,
                        l_int32  duration,
                        l_int32  quality,
                        l_int32  lossless)
{
l_uint8  *filedata;
size_t    filebytes, nbytes;

    PROCNAME("pixaWriteStreamWebpAnim");

    if (!fp)
        return ERROR_INT("stream not open", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    filedata = NULL;
    pixaWriteMemWebPAnim(&filedata, &filebytes, pixa, loopcount,
                         duration, quality, lossless);
    rewind(fp);
    if (!filedata)
        return ERROR_INT("filedata not made", procName, 1);
    nbytes = fwrite(filedata, 1, filebytes, fp);
    free(filedata);
    if (nbytes != filebytes)
        return ERROR_INT("Write error", procName, 1);
    return 0;
}


/*!
 * \brief   pixaWriteMemWebPAnim()
 *
 * \param[out]   pencdata    webp encoded data of pixs
 * \param[out]   pencsize    size of webp encoded data
 * \param[in]    pixa        with images of any depth, cmapped OK
 * \param[in]    loopcount   [0 for infinite]
 * \param[in]    duration    in ms, for each image
 * \param[in]    quality     0 - 100 for lossy; default ~80
 * \param[in]    lossless    use 1 for lossless; 0 for lossy
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) See pixWriteMemWebP() for details of webp encoding of images.
 * </pre>
 */
l_ok
pixaWriteMemWebPAnim(l_uint8  **pencdata,
                     size_t    *pencsize,
                     PIXA      *pixa,
                     l_int32    loopcount,
                     l_int32    duration,
                     l_int32    quality,
                     l_int32    lossless)
{
l_int32                 i, n, same, w, h, wpl, ret;
l_uint8                *data;
PIX                    *pix1, *pix2;
WebPAnimEncoder        *enc;
WebPAnimEncoderOptions  enc_options;
WebPConfig              config;
WebPData                webp_data;
WebPMux                *mux = NULL;
WebPMuxAnimParams       newparams;
WebPPicture             frame;

    PROCNAME("pixaWriteMemWebPAnim");

    if (!pencdata)
        return ERROR_INT("&encdata not defined", procName, 1);
    *pencdata = NULL;
    if (!pencsize)
        return ERROR_INT("&encsize not defined", procName, 1);
    *pencsize = 0;
    if (!pixa)
        return ERROR_INT("&pixa not defined", procName, 1);
    if ((n = pixaGetCount(pixa)) == 0)
        return ERROR_INT("no images in pixa", procName, 1);
    if (loopcount < 0) loopcount = 0;
    if (lossless == 0 && (quality < 0 || quality > 100))
        return ERROR_INT("quality not in [0 ... 100]", procName, 1);

    pixaVerifyDimensions(pixa, &same, &w, &h);
    if (!same)
        return ERROR_INT("sizes of all pix are not the same", procName, 1);

        /* Set up the encoder */
    WebPAnimEncoderOptionsInit(&enc_options);
    enc = WebPAnimEncoderNew(w, h, &enc_options);

    for (i = 0; i < n; i++) {
            /* Make a frame for each image.  Convert the pix to RGBA with
             * an opaque alpha layer, and put the raster data in the frame. */
        pix1 = pixaGetPix(pixa, i, L_CLONE);
        pix2 = pixConvertTo32(pix1);
        pixSetComponentArbitrary(pix2, L_ALPHA_CHANNEL, 255);
        pixEndianByteSwap(pix2);
        data = (l_uint8 *)pixGetData(pix2);
        wpl = pixGetWpl(pix2);
        WebPPictureInit(&frame);
        frame.width = w;
        frame.height = h;
        WebPPictureImportRGBA(&frame, data, 4 * wpl);
        pixDestroy(&pix1);
        pixDestroy(&pix2);

            /* Add the frame data to the encoder, and clear its memory */
        WebPConfigInit(&config);
        config.lossless = lossless;
        config.quality = quality;
        WebPAnimEncoderAdd(enc, &frame, duration * i, &config);
        WebPPictureFree(&frame);
    }
    WebPAnimEncoderAdd(enc, NULL, duration * i, NULL);  /* add a blank frame */
    WebPAnimEncoderAssemble(enc, &webp_data);  /* encode the data */
    WebPAnimEncoderDelete(enc);

        /* Set the loopcount if requested.  Note that when you make a mux,
         * it imports the webp_data that was previously made, including
         * the webp encoded images.  Before you re-export that data using
         * WebPMuxAssemble(), free the heap data in webp_data.  There is an
         * example for setting the loop count in the webp distribution;
         * see gif2webp.c.  */
    if (loopcount > 0) {
        mux = WebPMuxCreate(&webp_data, 1);
        if (!mux) {
            L_ERROR("could not re-mux to add loop count\n", procName);
        } else {
            ret = WebPMuxGetAnimationParams(mux, &newparams);
            if (ret != WEBP_MUX_OK) {
                L_ERROR("failed to get loop count\n", procName);
            } else {
                newparams.loop_count = loopcount;
                ret = WebPMuxSetAnimationParams(mux, &newparams);
                if (ret != WEBP_MUX_OK)
                    L_ERROR("failed to set loop count\n", procName);
            }
            WebPDataClear(&webp_data);
            WebPMuxAssemble(mux, &webp_data);
            WebPMuxDelete(mux);
        }
    }

    *pencdata = (l_uint8 *)webp_data.bytes;
    *pencsize = webp_data.size;
    L_INFO("data size = %zu\n", procName, webp_data.size);
    return 0;
}


/* --------------------------------------------*/
#endif  /* HAVE_LIBWEBP_ANIM */
/* --------------------------------------------*/
