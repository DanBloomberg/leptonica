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
 *  gifio.c
 *
 *    Read/write gif to file
 *          PIX        *pixReadStreamGif()
 *          l_int32     pixWriteStreamGif()
 *
 *    Read from memory   [not on windows]
 *          PIX        *pixReadMemGif()
 *
 *    This uses the gif library, version 4.1.6.  Do not use 4.1.4.
 *
 *    This module was generously contribued by Antony Dovgal.
 *    He can be contacted at:  tony *AT* daylessday.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef COMPILER_MSVC
#include <unistd.h>
#else
#include <io.h>
#endif  /* COMPILER_MSVC */
#include "allheaders.h"

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif  /* HAVE_CONFIG_H */

/* --------------------------------------------------------------------*/
#if  HAVE_LIBGIF  || HAVE_LIBUNGIF             /* defined in environ.h */
/* --------------------------------------------------------------------*/

#include "gif_lib.h"

/*---------------------------------------------------------------------*
 *                       Reading gif from file                         *
 *---------------------------------------------------------------------*/
/*!
 *  pixReadStreamGif()
 *
 *      Input:  stream
 *      Return: pix, or null on error
 */
PIX *
pixReadStreamGif(FILE  *fp)
{
l_int32          fd, wpl, i, j, w, h, d, cindex, ncolors;
l_int32          rval, gval, bval;
l_uint32        *data, *line;
GifFileType     *gif;
PIX             *pixd;
PIXCMAP         *cmap;
ColorMapObject  *gif_cmap;
SavedImage       si;

    PROCNAME("pixReadStreamGif");

    if ((fd = fileno(fp)) < 0)
        return (PIX *)ERROR_PTR("invalid file descriptor", procName, NULL);
#ifndef COMPILER_MSVC
    lseek(fd, 0, SEEK_SET);
#else
    _lseek(fd, 0, SEEK_SET);
#endif  /* COMPILER_MSVC */

    if ((gif = DGifOpenFileHandle(fd)) == NULL)
        return (PIX *)ERROR_PTR("invalid file or file not found",
                                procName, NULL);

        /* Read all the data, but use only the first image found */
    if (DGifSlurp(gif) != GIF_OK) {
        DGifCloseFile(gif);
        return (PIX *)ERROR_PTR("failed to read GIF data", procName, NULL);
    }

    if (gif->SavedImages == NULL) {
        DGifCloseFile(gif);
        return (PIX *)ERROR_PTR("no images found in GIF", procName, NULL);
    }

    si = gif->SavedImages[0];
    w = si.ImageDesc.Width;
    h = si.ImageDesc.Height;
    if (w <= 0 || h <= 0) {
        DGifCloseFile(gif);
        return (PIX *)ERROR_PTR("invalid image dimensions", procName, NULL);
    }

    if (si.RasterBits == NULL) {
        DGifCloseFile(gif);
        return (PIX *)ERROR_PTR("no raster data in GIF", procName, NULL);
    }

    if (si.ImageDesc.ColorMap) {
            /* private cmap for this image */
        gif_cmap = si.ImageDesc.ColorMap;
    }
    else if (gif->SColorMap) {
            /* global cmap for whole picture */
        gif_cmap = gif->SColorMap;
    }
    else {
            /* don't know where to take cmap from */
        DGifCloseFile(gif);
        return (PIX *)ERROR_PTR("color map is missing", procName, NULL);
    }

    ncolors = gif_cmap->ColorCount;
    if (ncolors <= 2)
        d = 1;
    else if (ncolors <= 4)
        d = 2;
    else if (ncolors <= 16)
        d = 4;
    else
        d = 8;
    if ((cmap = pixcmapCreate(d)) == NULL)
        return (PIX *)ERROR_PTR("cmap creation failed", procName, NULL);

    for (cindex = 0; cindex < ncolors; cindex++) {
        rval = gif_cmap->Colors[cindex].Red;
        gval = gif_cmap->Colors[cindex].Green;
        bval = gif_cmap->Colors[cindex].Blue;
        pixcmapAddColor(cmap, rval, gval, bval);
    }

    if ((pixd = pixCreate(w, h, d)) == NULL) {
        DGifCloseFile(gif);
        pixcmapDestroy(&cmap);
        return (PIX *)ERROR_PTR("failed to allocate pixd", procName, NULL);
    }
    pixSetColormap(pixd, cmap);

    wpl = pixGetWpl(pixd);
    data = pixGetData(pixd);
    for (i = 0; i < h; i++) {
        line = data + i * wpl;
	if (d == 1) {
            for (j = 0; j < w; j++) {
                if (si.RasterBits[i * w + j])
                    SET_DATA_BIT(line, j);
            }
        }
        else if (d == 2) {
            for (j = 0; j < w; j++)
                SET_DATA_DIBIT(line, j, si.RasterBits[i * w + j]);
        }
        else if (d == 4) {
            for (j = 0; j < w; j++)
                SET_DATA_QBIT(line, j, si.RasterBits[i * w + j]);
        }
        else {  /* d == 8 */
            for (j = 0; j < w; j++)
                SET_DATA_BYTE(line, j, si.RasterBits[i * w + j]);
        }
    }

    DGifCloseFile(gif);
    return pixd;
}


/*---------------------------------------------------------------------*
 *                         Writing gif to file                         *
 *---------------------------------------------------------------------*/
/*!
 *  pixWriteStreamGif()
 *
 *      Input:  stream
 *              pix (1, 2, 4, 8, 16 or 32 bpp)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) All output gif have colormaps.  If the pix is 32 bpp rgb,
 *          this quantizes the colors and writes out 8 bpp.
 *          If the pix is 16 bpp grayscale, it converts to 8 bpp first.
 *      (2) We can't write to memory using open_memstream() because
 *          the gif functions write through a file descriptor, not a
 *          file stream.
 */
l_int32
pixWriteStreamGif(FILE  *fp,
                  PIX   *pix)
{
char            *text;
l_int32          fd, wpl, i, j, w, h, d, ncolor, rval, gval, bval;
l_int32          gif_ncolor = 0;
l_uint32        *data, *line;
PIX             *pixd;
PIXCMAP         *cmap;
GifFileType     *gif;
ColorMapObject  *gif_cmap;
GifByteType     *gif_line;

    PROCNAME("pixWriteStreamGif");

    if (!fp)
        return ERROR_INT("stream not open", procName, 1);
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);
    rewind(fp);

    if ((fd = fileno(fp)) < 0)
        return ERROR_INT("invalid file descriptor", procName, 1);

    d = pixGetDepth(pix);
    if (d == 32) {
        pixd = pixConvertRGBToColormap(pix, 1);
    }
    else if (d > 1) {
        pixd = pixConvertTo8(pix, TRUE);
    }
    else {  /* d == 1; make sure there's a colormap */
        pixd = pixClone(pix);
        if (!pixGetColormap(pixd)) {
            cmap = pixcmapCreate(1);
            pixcmapAddColor(cmap, 255, 255, 255);
            pixcmapAddColor(cmap, 0, 0, 0);
            pixSetColormap(pixd, cmap);
        }
    }

    if (!pixd)
        return ERROR_INT("failed to convert image to indexed", procName, 1);
    d = pixGetDepth(pixd);

    if ((cmap = pixGetColormap(pixd)) == NULL) {
        pixDestroy(&pixd);
        return ERROR_INT("cmap is missing", procName, 1);
    }

        /* 'Round' the number of gif colors up to a power of 2 */
    ncolor = pixcmapGetCount(cmap);
    for (i = 0; i <= 8; i++) {
        if ((1 << i) >= ncolor) {
            gif_ncolor = (1 << i);
            break;
        }
    }
    if (gif_ncolor < 1) {
        pixDestroy(&pixd);
        return ERROR_INT("number of colors is invalid", procName, 1);
    }

        /* Save the cmap colors in a gif_cmap */
    if ((gif_cmap = MakeMapObject(gif_ncolor, NULL)) == NULL) {
        pixDestroy(&pixd);
        return ERROR_INT("failed to create GIF color map", procName, 1);
    }
    for (i = 0; i < gif_ncolor; i++) {
	rval = gval = bval = 0;
        if (ncolor > 0) {
            if (pixcmapGetColor(cmap, i, &rval, &gval, &bval) != 0) {
                pixDestroy(&pixd);
                FreeMapObject(gif_cmap);
                return ERROR_INT("failed to get color from color map",
                                 procName, 1);
            }
            ncolor--;
        }
        gif_cmap->Colors[i].Red = rval;
        gif_cmap->Colors[i].Green = gval;
        gif_cmap->Colors[i].Blue = bval;
    }

        /* Get the gif file handle */
    if ((gif = EGifOpenFileHandle(fd)) == NULL) {
        pixDestroy(&pixd);
        FreeMapObject(gif_cmap);
        return ERROR_INT("failed to create GIF image handle", procName, 1);
    }

    pixGetDimensions(pixd, &w, &h, NULL);
    if (EGifPutScreenDesc(gif, w, h, gif_cmap->BitsPerPixel, 0, gif_cmap)
        != GIF_OK) {
        pixDestroy(&pixd);
        FreeMapObject(gif_cmap);
        EGifCloseFile(gif);
        return ERROR_INT("failed to write screen description", procName, 1);
    }
    FreeMapObject(gif_cmap); /* not needed after this point */

    if (EGifPutImageDesc(gif, 0, 0, w, h, FALSE, NULL) != GIF_OK) {
        pixDestroy(&pixd);
        EGifCloseFile(gif);
        return ERROR_INT("failed to image screen description", procName, 1);
    }

    data = pixGetData(pixd);	
    wpl = pixGetWpl(pixd);
    if (d != 1 && d != 2 && d != 4 && d != 8) {
        pixDestroy(&pixd);
        EGifCloseFile(gif);
        return ERROR_INT("image depth is not in {1, 2, 4, 8}", procName, 1);
    }

    if ((gif_line = (GifByteType *)CALLOC(sizeof(GifByteType), w)) == NULL) {
        pixDestroy(&pixd);
        EGifCloseFile(gif);
        return ERROR_INT("mem alloc fail for data line", procName, 1);
    }

    for (i = 0; i < h; i++) {
        line = data + i * wpl;
            /* Gif's way of setting the raster line up for compression */
        for (j = 0; j < w; j++) {
            switch(d)
            {
            case 8:
                gif_line[j] = GET_DATA_BYTE(line, j);
                break;
            case 4:
                gif_line[j] = GET_DATA_QBIT(line, j);
                break;
            case 2:
                gif_line[j] = GET_DATA_DIBIT(line, j);
                break;
            case 1:
                gif_line[j] = GET_DATA_BIT(line, j);
                break;
            }
        }

            /* Compress and save the line */
        if (EGifPutLine(gif, gif_line, w) != GIF_OK) {
            FREE(gif_line);
            pixDestroy(&pixd);
            EGifCloseFile(gif);
            return ERROR_INT("failed to write data line into GIF", procName, 1);
        }
    }

        /* Write a text comment.  This must be placed after writing the
         * data (!!)  Note that because libgif does not provide a function
         * for reading comments from file, you will need another way
         * to read comments. */
    if ((text = pixGetText(pix)) != NULL) {
        if (EGifPutComment(gif, text) != GIF_OK)
            L_WARNING("gif comment not written", procName);
    }

    FREE(gif_line);
    pixDestroy(&pixd);
    EGifCloseFile(gif);
    return 0;
}


/*---------------------------------------------------------------------*
 *                           Read from memory                          *
 *---------------------------------------------------------------------*/
#if HAVE_FMEMOPEN

extern FILE *fmemopen(void *data, size_t size, const char *mode);

/*!
 *  pixReadMemGif()
 *
 *      Input:  cdata (const; png-encoded)
 *              size (of data)
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) The @size byte of @data must be a null character.
 */
PIX *
pixReadMemGif(const l_uint8  *cdata,
              size_t          size)
{
l_uint8  *data;
FILE     *fp;
PIX      *pix;

    PROCNAME("pixReadMemGif");

    if (!cdata)
        return (PIX *)ERROR_PTR("cdata not defined", procName, NULL);

    data = (l_uint8 *)cdata;  /* we're really not going to change this */
    if ((fp = fmemopen(data, size, "r")) == NULL)
        return (PIX *)ERROR_PTR("stream not opened", procName, NULL);
    pix = pixReadStreamGif(fp);
    fclose(fp);
    return pix;
}

#else

PIX *
pixReadMemGif(const l_uint8  *cdata,
              size_t           size)
{
    return (PIX *)ERROR_PTR(
        "gif read from memory not implemented on this platform",
        "pixReadMemGif", NULL);
}

#endif  /* HAVE_FMEMOPEN */

/* -----------------------------------------------------------------*/
#endif    /* HAVE_LIBGIF || HAVE_LIBUNGIF  */
/* -----------------------------------------------------------------*/

