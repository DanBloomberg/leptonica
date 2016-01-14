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
 *     X Display using xv
 *        l_int32    pixDisplay()
 *        l_int32    pixDisplayWithName()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

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
        sprintf(bigbuf, "%s%03d.%s", rootname, i, 
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
 *  Note: We open for write using binary mode (with the "b" flag)
 *        to avoid having Windows automatically translate the NL
 *        into CRLF, which corrupts image files.  On non-windows
 *        systems this flag should be ignored, per ISO C90.
 *        Thanks to Dave Bryan for pointing this out.
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
 *       Input:  stream
 *               pix
 *               format
 *       Return: 0 if OK; 1 on error.
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
 *  Note: this should only be called if the requested format is IFF_DEFAULT.
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
	if (d <= 4)
	    format = IFF_PNG;
	else
	    format = IFF_JFIF_JPEG;
    }
    else if (format == IFF_TIFF && d == 1)
	format = IFF_TIFF_G4;

    return format;
}



/*---------------------------------------------------------------------*
 *                            X Display using xv                       *
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
    return pixDisplayWithTitle(pixs, x, y, NULL);
}


/*!
 *  pixDisplayWithTitle()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              x, y  (location of xv frame)
 *              title (<optional> on xv window; can be NULL);
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) See notes for pixDisplay()
 */
l_int32
pixDisplayWithTitle(PIX         *pixs,
                    l_int32      x,
	            l_int32      y,
                    const char  *title)

{
char           *tempname;
char            buffer[L_BUF_SIZE];
static l_int32  index = 0;  /* caution: not .so or thread safe */
l_int32         w, h, d;
l_float32       ratw, rath, ratmin;
PIX            *pixt;

    PROCNAME("pixDisplayWithTitle");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    d = pixGetDepth(pixs);
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
	sprintf(buffer, "rm -f junk_xv_display.*");
	system(buffer);
    }

    index++;
    sprintf(buffer, "junk_xv_display.%d", index);
    tempname = stringNew(buffer);
    if (pixGetDepth(pixt) < 8)
	pixWrite(tempname, pixt, IFF_PNG);
    else
	pixWrite(tempname, pixt, IFF_JFIF_JPEG);

    if (title)
        sprintf(buffer, "xv -quit -geometry +%d+%d -name \"%s\" %s &",
                x, y, title, tempname);
    else
        sprintf(buffer, "xv -quit -geometry +%d+%d %s &", x, y, tempname);
    system(buffer);

    pixDestroy(&pixt);
    FREE(tempname);

    return 0;
}


