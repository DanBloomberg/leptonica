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
 * writefile.c
 *
 *     High-level procedures for writing images to file:
 *        l_int32    pixaWriteFiles() 
 *        l_int32    pixWrite() 
 *        l_int32    pixWriteStream() 
 *
 *     Choose output format if default is requested
 *        l_int32    pixChooseOutputFormat()
 *
 *     Image display for debugging
 *        l_int32    pixDisplay()
 *        l_int32    pixDisplayWithTitle()
 *        l_int32    pixDisplayWrite()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

#if defined(__MINGW32__) || defined(_WIN32)
#define snprintf _snprintf
#endif

    /* MS VC++ can't handle array initialization with static consts ! */
#define L_BUF_SIZE      512

    /* For display using xv */
static const l_int32  MAX_DISPLAY_WIDTH = 1000;
static const l_int32  MAX_DISPLAY_HEIGHT = 800;

    /* PostScript output for printing */
static const l_float32  DEFAULT_SCALING = 1.0;

    /* Global array of image file format extension names */
const char *ImageFileFormatExtensions[] = {"unknown",
                                           "bmp",
                                           "jpg",
                                           "png",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "pnm",
                                           "ps"};


/*---------------------------------------------------------------------*
 *           Top-level procedures for writing images to file           *
 *---------------------------------------------------------------------*/
/*!
 *  pixaWriteFiles()
 *
 *      Input:  rootname
 *              pixa
 *              format  (defined in imageio.h)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixaWriteFiles(const char  *rootname,
               PIXA        *pixa,
               l_int32      format)
{
char     bigbuf[L_BUF_SIZE];
l_int32  i, n;
PIX     *pix;

    PROCNAME("pixaWriteFiles");

    if (!rootname)
        return ERROR_INT("rootname not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    n = pixaGetCount(pixa);
    for (i = 0; i < n; i++) {
        snprintf(bigbuf, L_BUF_SIZE, "%s%03d.%s", rootname, i, 
                 ImageFileFormatExtensions[format]);
        pix = pixaGetPix(pixa, i, L_CLONE);
        pixWrite(bigbuf, pix, format);
        pixDestroy(&pix);
    }

    return 0;
}


/*!
 *  pixWrite()
 *
 *      Input:  filename
 *              pix
 *              format  (defined in imageio.h)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) Open for write using binary mode (with the "b" flag)
 *          to avoid having Windows automatically translate the NL
 *          into CRLF, which corrupts image files.  On non-windows
 *          systems this flag should be ignored, per ISO C90.
 *          Thanks to Dave Bryan for pointing this out.
 */
l_int32
pixWrite(const char  *filename, 
         PIX         *pix, 
         l_int32      format)
{
FILE  *fp;

    PROCNAME("pixWrite");

    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);
    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);

    if ((fp = fopen(filename, "wb+")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);

    if (pixWriteStream(fp, pix, format)) {
        fclose(fp);
        return ERROR_INT("pix not written to stream", procName, 1);
    }

    fclose(fp);
    return 0;
}


/*!
 *  pixWriteStream()
 *
 *      Input:  stream
 *              pix
 *              format
 *      Return: 0 if OK; 1 on error.
 */
l_int32
pixWriteStream(FILE    *fp, 
               PIX     *pix, 
               l_int32  format)
{
    PROCNAME("pixWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);

    if (format == IFF_DEFAULT)
        format = pixChooseOutputFormat(pix);

    switch(format)
    {
    case IFF_BMP:
        pixWriteStreamBmp(fp, pix);
        break;

    case IFF_JFIF_JPEG:   /* default quality; baseline sequential */
        return pixWriteStreamJpeg(fp, pix, 75, 0);
        break;
    
    case IFF_PNG:   /* no gamma value stored */
        return pixWriteStreamPng(fp, pix, 0.0);
        break;
    
    case IFF_TIFF:  /* uncompressed */
        return pixWriteStreamTiff(fp, pix, IFF_TIFF);
        break;

    case IFF_TIFF_PACKBITS:  /* on binary only */
        return pixWriteStreamTiff(fp, pix, IFF_TIFF_PACKBITS);
        break;

    case IFF_TIFF_G3:  /* on binary only */
        return pixWriteStreamTiff(fp, pix, IFF_TIFF_G3);
        break;

    case IFF_TIFF_G4:  /* on binary only */
        return pixWriteStreamTiff(fp, pix, IFF_TIFF_G4);
        break;

    case IFF_PNM:
        return pixWriteStreamPnm(fp, pix);

    case IFF_PS:
        return pixWriteStreamPS(fp, pix, NULL, 0, DEFAULT_SCALING);
        break;
    
    default:
        return ERROR_INT("unknown format", procName, 1);
        break;
    }

    return 0;
}



/*---------------------------------------------------------------------*
 *                           Choose output format                      *
 *---------------------------------------------------------------------*/
/*!
 *  pixChooseOutputFormat()
 *
 *      Input:  pix
 *      Return: output format, or 0 on error
 *
 *  Notes:
 *      (1) This should only be called if the requested format is IFF_DEFAULT.
 */
l_int32
pixChooseOutputFormat(PIX  *pix)
{
l_int32  d, format;

    PROCNAME("pixChooseOutputFormat");

    if (!pix)
        return ERROR_INT("pix not defined", procName, 0);

    d = pixGetDepth(pix);
    format = pixGetInputFormat(pix);
    if (format == IFF_UNKNOWN) {
        if (d < 8)
            format = IFF_PNG;
        else
            format = IFF_JFIF_JPEG;
    }
    else if (format == IFF_TIFF && d == 1)
        format = IFF_TIFF_G4;

    return format;
}



/*---------------------------------------------------------------------*
 *                       Image display for debugging                   *
 *---------------------------------------------------------------------*/
/*!
 *  pixDisplay()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              x, y  (location of xv frame)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This uses xv to display.  It must be on your $PATH variable.
 *      (2) Because xv reduces images to fit the screen, we do this
 *          reduction in advance, and write it out to a temporary file
 *          in the current directory with the name "junk_xv_display.*"
 *      (3) This function uses a static internal variable to number
 *          output files written by a single process.  Behavior
 *          with a shared library may be unpredictable.
 */
l_int32
pixDisplay(PIX     *pixs,
           l_int32  x,
           l_int32  y)
{
    return pixDisplayWithTitle(pixs, x, y, NULL, 1);
}


/*!
 *  pixDisplayWithTitle()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              x, y  (location of xv frame)
 *              title (<optional> on xv window; can be NULL);
 *              dispflag (0 to disable; 1 to write)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) See notes for pixDisplay().
 *      (2) This displays the image if dispflag == 1.
 */
l_int32
pixDisplayWithTitle(PIX         *pixs,
                    l_int32      x,
                    l_int32      y,
                    const char  *title,
		    l_int32      dispflag)
{
char           *tempname;
char            buffer[L_BUF_SIZE];
static l_int32  index = 0;  /* caution: not .so or thread safe */
l_int32         w, h, d;
l_float32       ratw, rath, ratmin;
PIX            *pixt;

    PROCNAME("pixDisplayWithTitle");

    if (dispflag == 0) return 0;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    pixGetDimensions(pixs, &w, &h, &d);
    if (w <= MAX_DISPLAY_WIDTH && h <= MAX_DISPLAY_HEIGHT)
        pixt = pixClone(pixs);
    else {
        ratw = (l_float32)MAX_DISPLAY_WIDTH / (l_float32)w;
        rath = (l_float32)MAX_DISPLAY_HEIGHT / (l_float32)h;
        ratmin = L_MIN(ratw, rath);
        if (ratmin < 0.125 && d == 1)
            pixt = pixScaleToGray8(pixs);
        else if (ratmin < 0.25 && d == 1)
            pixt = pixScaleToGray4(pixs);
        else if (ratmin < 0.33 && d == 1)
            pixt = pixScaleToGray3(pixs);
        else if (ratmin < 0.5 && d == 1)
            pixt = pixScaleToGray2(pixs);
        else
            pixt = pixScale(pixs, ratmin, ratmin);
        if (!pixt)
            return ERROR_INT("pixt not made", procName, 1);
    }

    if (index == 0) {
        snprintf(buffer, L_BUF_SIZE, "rm -f junk_xv_display.*");
        system(buffer);
    }

    index++;
    if (pixGetDepth(pixt) < 8) {
        snprintf(buffer, L_BUF_SIZE, "junk_xv_display.%03d.png", index);
        pixWrite(buffer, pixt, IFF_PNG);
    }
    else {
        snprintf(buffer, L_BUF_SIZE, "junk_xv_display.%03d.jpg", index);
        pixWrite(buffer, pixt, IFF_JFIF_JPEG);
    }
    tempname = stringNew(buffer);

    if (title)
        snprintf(buffer, L_BUF_SIZE,
                 "xv -quit -geometry +%d+%d -name \"%s\" %s &",
                 x, y, title, tempname);
    else
        snprintf(buffer, L_BUF_SIZE,
                 "xv -quit -geometry +%d+%d %s &", x, y, tempname);
    system(buffer);

    pixDestroy(&pixt);
    FREE(tempname);
    return 0;
}


/*!
 *  pixDisplayWrite()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              reduction (-1 to reset/erase; 0 to disable;
 *                         otherwise this is a reduction factor)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This writes files if reduction > 0.  These can be
 *          displayed, ordered in a tiled representation, with,
 *          for example, gthumb.
 *      (2) All previously written files can be erased by calling with
 *          reduction < 0; the value of pixs is ignored.
 *      (3) If reduction > 1 and depth == 1, this does a scale-to-gray
 *          reduction.
 *      (4) This function uses a static internal variable to number
 *          output files written by a single process.  Behavior
 *          with a shared library may be unpredictable.
 */
l_int32
pixDisplayWrite(PIX     *pixs,
                l_int32  reduction)
{
char            buffer[L_BUF_SIZE];
l_float32       scale;
PIX            *pixt;
static l_int32  index = 0;  /* caution: not .so or thread safe */

    PROCNAME("pixDisplayWrite");

    if (reduction == 0) return 0;

    if (reduction < 0) {
        index = 0;  /* reset; this will cause erasure at next call to write */
	return 0;
    }

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    if (index == 0) {
        snprintf(buffer, L_BUF_SIZE,
            "rm -f junk_write_display.*.png junk_write_display.*.jpg");
        system(buffer);
    }
    index++;

    if (reduction == 1)
        pixt = pixClone(pixs);
    else {
        scale = 1. / (l_float32)reduction;
	if (pixGetDepth(pixs) == 1)
            pixt = pixScaleToGray(pixs, scale);
        else
            pixt = pixScale(pixs, scale, scale);
    }

    if (pixGetDepth(pixt) < 8) {
        snprintf(buffer, L_BUF_SIZE, "junk_write_display.%03d.png", index);
        pixWrite(buffer, pixt, IFF_PNG);
    }
    else {
        snprintf(buffer, L_BUF_SIZE, "junk_write_display.%03d.jpg", index);
        pixWrite(buffer, pixt, IFF_JFIF_JPEG);
    }
    pixDestroy(&pixt);

    return 0;
}


