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
 *        l_int32     pixaWriteFiles() 
 *        l_int32     pixWrite() 
 *        l_int32     pixWriteStream() 
 *        l_int32     pixWriteImpliedFormat() 
 *
 *     Selection of output format if default is requested
 *        l_int32     pixChooseOutputFormat()
 *        l_int32     getImpliedFileFormat()
 *        const char *getFormatExtension()
 *
 *     Write to memory
 *        l_int32     pixWriteMem()
 *
 *     Image display for debugging
 *        l_int32     pixDisplay()
 *        l_int32     pixDisplayWithTitle()
 *        l_int32     pixDisplayWrite()
 *        l_int32     pixDisplayWriteFormat()
 *        l_int32     pixSaveTiled()
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
static const l_int32  MAX_SIZE_FOR_PNG = 200;

    /* PostScript output for printing */
static const l_float32  DEFAULT_SCALING = 1.0;

    /* Global array of image file format extension names.
     * This is in 1-1 corrspondence with format enum in imageio.h. */
static const l_int32  NUM_EXTENSIONS = 14;
const char *ImageFileFormatExtensions[] = {"unknown",
                                           "bmp",
                                           "jpg",
                                           "png",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "tif",
                                           "pnm",
                                           "ps",
                                           "gif"};

    /* Local map of image file name extension to output format */
struct ExtensionMap
{
    char     extension[8];
    l_int32  format;
}; 
static const struct ExtensionMap extension_map[] = 
                            { { ".bmp",  IFF_BMP       },    
                              { ".jpg",  IFF_JFIF_JPEG },    
                              { ".jpeg", IFF_JFIF_JPEG },    
                              { ".png",  IFF_PNG       },    
                              { ".tif",  IFF_TIFF      },    
                              { ".tiff", IFF_TIFF      },    
                              { ".pnm",  IFF_PNM       },    
                              { ".gif",  IFF_GIF       },    
                              { ".ps",   IFF_PS        } };


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
    
    case IFF_TIFF:           /* uncompressed */
    case IFF_TIFF_PACKBITS:  /* compressed, binary only */
    case IFF_TIFF_RLE:       /* compressed, binary only */
    case IFF_TIFF_G3:        /* compressed, binary only */
    case IFF_TIFF_G4:        /* compressed, binary only */
    case IFF_TIFF_LZW:       /* compressed, all depths */
    case IFF_TIFF_ZIP:       /* compressed, all depths */
        return pixWriteStreamTiff(fp, pix, format);
        break;

    case IFF_PNM:
        return pixWriteStreamPnm(fp, pix);
        break;

    case IFF_GIF:
        return pixWriteStreamGif(fp, pix);
        break;
    
    case IFF_PS:
        return pixWriteStreamPS(fp, pix, NULL, 0, DEFAULT_SCALING);
        break;
    
    default:
        return ERROR_INT("unknown format", procName, 1);
        break;
    }

    return 0;
}


/*!
 *  pixWriteImpliedFormat()
 *
 *      Input:  filename
 *              pix
 *              quality (iff JPEG; 1 - 100, 0 for default)
 *              progressive (iff JPEG; 0 for baseline seq., 1 for progressive)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This determines the output format from the filename extension.
 *      (2) The last two args are ignored except for requests for jpeg files.
 *      (3) The jpeg default quality is 75.
 */
l_int32
pixWriteImpliedFormat(const char  *filename,
                      PIX         *pix,
                      l_int32      quality,
                      l_int32      progressive)
{
l_int32  format;

    PROCNAME("pixWriteImpliedFormat");

    if (!filename)
        return ERROR_INT ("filename not defined", procName, 1);
    if (!pix)
        return ERROR_INT ("pix not defined", procName, 1);

        /* Determine output format */
    format = getImpliedFileFormat(filename);
    if (format == IFF_UNKNOWN)
        format = IFF_PNG;
    else if (format == IFF_TIFF) {
        if (pixGetDepth(pix) == 1)
            format = IFF_TIFF_G4;
        else
            format = IFF_TIFF_LZW;
    }

    if (format == IFF_JFIF_JPEG) {
        quality = L_MIN(quality, 100);
        quality = L_MAX(quality, 0);
        if (progressive != 0 && progressive != 1) {
            progressive = 0;
            L_WARNING("invalid progressive; setting to baseline", procName);
        }
        if (quality == 0)
            quality = 75;
        pixWriteJpeg (filename, pix, quality, progressive);
    }
    else
        pixWrite(filename, pix, format);

    return 0;
}


/*---------------------------------------------------------------------*
 *          Selection of output format if default is requested         *
 *---------------------------------------------------------------------*/
/*!
 *  pixChooseOutputFormat()
 *
 *      Input:  pix
 *      Return: output format, or 0 on error
 *
 *  Notes:
 *      (1) This should only be called if the requested format is IFF_DEFAULT.
 *      (2) If the pix wasn't read from a file, its input format value
 *          will be IFF_UNKNOWN, and in that case it is written out
 *          in a compressed but lossless format.
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
    if (format == IFF_UNKNOWN) {  /* output lossless */
        if (d == 1)
            format = IFF_TIFF_G4;
        else
            format = IFF_PNG;
    }

    return format;
}


/*!
 *  getImpliedFileFormat()
 *
 *      Input:  filename
 *      Return: output format, or IFF_UNKNOWN on error or invalid extension.
 *
 *  Notes:
 *      (1) This determines the output file format from the extension
 *          of the input filename.
 */
l_int32
getImpliedFileFormat(const char  *filename)
{
char    *extension;
int      i, numext;
l_int32  format = IFF_UNKNOWN;

    if (splitPathAtExtension (filename, NULL, &extension))
        return IFF_UNKNOWN;

    numext = sizeof(extension_map) / sizeof(extension_map[0]);
    for (i = 0; i < numext; i++) {
        if (!strcmp(extension, extension_map[i].extension)) { 
            format = extension_map[i].format;
            break;
        }
    }

    FREE(extension);
    return format;
}


/*!
 *  getFormatExtension()
 *
 *      Input:  format (integer)
 *      Return: extension (string), or null if format is out of range
 *
 *  Notes:
 *      (1) This string is NOT owned by the caller; it is just a pointer
 *          to a global string.  Do not free it.
 */
const char *
getFormatExtension(l_int32  format)
{
    PROCNAME("getFormatExtension");

    if (format < 0 || format >= NUM_EXTENSIONS)
        return (const char *)ERROR_PTR("format out of bounds", procName, NULL);

    return ImageFileFormatExtensions[format];
}


/*---------------------------------------------------------------------*
 *                            Write to memory                          *
 *---------------------------------------------------------------------*/
/*! 
 *  pixWriteMem()
 *
 *      Input:  &data (<return> data of tiff compressed image)
 *              &size (<return> size of returned data)
 *              pix
 *              format  (defined in imageio.h)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) On windows, this will only write tiff and PostScript to memory.
 *          For other formats, it requires open_memstream(3).
 *      (2) PostScript output is uncompressed, in hex ascii.
 *          Most printers support level 2 compression (tiff_g4 for 1 bpp,
 *          jpeg for 8 and 32 bpp).
 */
l_int32
pixWriteMem(l_uint8  **pdata,
            size_t    *psize,
            PIX       *pix,
            l_int32    format)
{
l_int32  ret;

    PROCNAME("pixWriteMem");

    if (!pdata)
        return ERROR_INT("&data not defined", procName, 1 );
    if (!psize)
        return ERROR_INT("&size not defined", procName, 1 );
    if (!pix)
        return ERROR_INT("&pix not defined", procName, 1 );

    if (format == IFF_DEFAULT)
        format = pixChooseOutputFormat(pix);

    switch(format)
    {
    case IFF_BMP:
        ret = pixWriteMemBmp(pdata, psize, pix);
        break;

    case IFF_JFIF_JPEG:   /* default quality; baseline sequential */
        ret = pixWriteMemJpeg(pdata, psize, pix, 75, 0);
        break;
    
    case IFF_PNG:   /* no gamma value stored */
        ret = pixWriteMemPng(pdata, psize, pix, 0.0);
        break;
    
    case IFF_TIFF:           /* uncompressed */
    case IFF_TIFF_PACKBITS:  /* compressed, binary only */
    case IFF_TIFF_RLE:       /* compressed, binary only */
    case IFF_TIFF_G3:        /* compressed, binary only */
    case IFF_TIFF_G4:        /* compressed, binary only */
    case IFF_TIFF_LZW:       /* compressed, all depths */
    case IFF_TIFF_ZIP:       /* compressed, all depths */
        ret = pixWriteMemTiff(pdata, psize, pix, format);
        break;

    case IFF_PNM:
        ret = pixWriteMemPnm(pdata, psize, pix);
        break;

    case IFF_PS:
        ret = pixWriteMemPS(pdata, psize, pix, NULL, 0, DEFAULT_SCALING);
        break;
    
    default:
        return ERROR_INT("unknown format", procName, 1);
        break;
    }

    return ret;
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
    if (w <= MAX_DISPLAY_WIDTH && h <= MAX_DISPLAY_HEIGHT) {
        if (d == 16)  /* take MSB */
            pixt = pixConvert16To8(pixs, 1);
        else
            pixt = pixClone(pixs);
    }
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
    if (pixGetDepth(pixt) < 8 ||
        (w < MAX_SIZE_FOR_PNG && h < MAX_SIZE_FOR_PNG)) {
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
 *      (1) This defaults to jpeg output for pix that are 32 bpp or
 *          8 bpp without a colormap.  If you want to write all images
 *          losslessly, use format == IFF_PNG in pixDisplayWriteFormat().
 *      (2) See pixDisplayWriteFormat() for usage details.
 */
l_int32
pixDisplayWrite(PIX     *pixs,
                l_int32  reduction)
{
    return pixDisplayWriteFormat(pixs, reduction, IFF_JFIF_JPEG);
}


/*!
 *  pixDisplayWriteFormat()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              reduction (-1 to reset/erase; 0 to disable;
 *                         otherwise this is a reduction factor)
 *              format (IFF_PNG or IFF_JFIF_JPEG)
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
 *      (5) Output file format is as follows:
 *            format == IFF_JFIF_JPEG:
 *                png if d < 8 or d == 16 or if the output pix
 *                has a colormap.   Otherwise, output is jpg.
 *            format == IFF_PNG:
 *                png (lossless) on all images.
 *      (6) For 16 bpp, the choice of full dynamic range with log scale
 *          is the best for displaying these images.  Alternative outputs are
 *             pix8 = pixMaxDynamicRange(pixt, L_LINEAR_SCALE);
 *             pix8 = pixConvert16To8(pixt, 0);  // low order byte
 *             pix8 = pixConvert16To8(pixt, 1);  // high order byte
 */
l_int32
pixDisplayWriteFormat(PIX     *pixs,
                      l_int32  reduction,
                      l_int32  format)
{
char            buffer[L_BUF_SIZE];
l_float32       scale;
PIX            *pixt, *pix8;
static l_int32  index = 0;  /* caution: not .so or thread safe */

    PROCNAME("pixDisplayWriteFormat");

    if (reduction == 0) return 0;

    if (reduction < 0) {
        index = 0;  /* reset; this will cause erasure at next call to write */
        return 0;
    }

    if (format != IFF_JFIF_JPEG && format != IFF_PNG)
        return ERROR_INT("invalid format", procName, 1);
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

    if (pixGetDepth(pixt) == 16) {
        pix8 = pixMaxDynamicRange(pixt, L_LOG_SCALE);
        snprintf(buffer, L_BUF_SIZE, "junk_write_display.%03d.png", index);
        pixWrite(buffer, pix8, IFF_PNG);
        pixDestroy(&pix8);
    }
    else if (pixGetDepth(pixt) < 8 || pixGetColormap(pixt)) {
        snprintf(buffer, L_BUF_SIZE, "junk_write_display.%03d.png", index);
        pixWrite(buffer, pixt, IFF_PNG);
    }
    else {
        snprintf(buffer, L_BUF_SIZE, "junk_write_display.%03d.jpg", index);
        pixWrite(buffer, pixt, format);
    }
    pixDestroy(&pixt);

    return 0;
}


/*!
 *  pixSaveTiled()
 *
 *      Input:  pixs (1, 2, 4, 8, 32 bpp)
 *              pixa (the pix are accumulated here)
 *              reduction (0 to disable; otherwise this is a reduction factor)
 *              newrow (0 if placed on the same row as previous; 1 otherwise)
 *              space (horizontal and vertical spacing, in pixels)
 *              dp (depth of pixa; 8 or 32 bpp; only used on first call)
 *      Return: 0 if OK, 1 on error.
 *
 *  Notes:
 *      (1) Before calling this function for the first time, use
 *          pixaCreate() to make the @pixa that will accumulate the pix.
 *          This is passed in each time pixSaveTiled() is called.
 *      (2) @reduction is the integer reduction factor for the input
 *          image.  After reduction and possible depth conversion,
 *          the image is saved in the input pixa, along with a box
 *          that specifies the location to place it when tiled later.
 *          Disable saving the pix by setting reduction == 0.
 *      (3) @newrow and @space specify the location of the new pix
 *          with respect to the last one(s) that were entered.
 *      (4) @dp specifies the depth at which all pix are saved.  It can
 *          be only 8 or 32 bpp.  Any colormap is removed.  This is only
 *          used at the first invocation.
 *      (5) This function uses two variables from call to call.
 *          If they were static, the function would not be .so or thread
 *          safe, and furthermore, there would be interference with two or
 *          more pixa accumulating images at a time.  Consequently,
 *          we use the first pix in the pixa to store and obtain both
 *          the depth and the current position of the bottom (one pixel
 *          below the lowest image raster line when laid out using
 *          the boxa).  The bottom variable is stored in the input format
 *          field, which is the only field available for storing an int.
 */
l_int32
pixSaveTiled(PIX     *pixs,
             PIXA    *pixa,
             l_int32  reduction,
             l_int32  newrow,
             l_int32  space,
             l_int32  dp)
{
l_int32         n, top, left, bx, by, bw, w, h, depth, bottom;
l_float32       scale;
BOX            *box;
PIX            *pix, *pixt1, *pixt2;

    PROCNAME("pixSaveTiled");

    if (reduction == 0) return 0;

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    n = pixaGetCount(pixa);
    if (n == 0) {
        bottom = 0;
        if (dp != 8 && dp != 32) {
            L_WARNING("dp not 8 or 32 bpp; using 32", procName);
            depth = 32;
        } else
            depth = dp;
    }
    else {  /* extract the depth and bottom params from the first pix */
        pix = pixaGetPix(pixa, 0, L_CLONE);
        depth = pixGetDepth(pix);
        bottom = pixGetInputFormat(pix);  /* not typical usage! */
        pixDestroy(&pix);
    }
        
    if (reduction == 1)
        pixt1 = pixClone(pixs);
    else {
        scale = 1. / (l_float32)reduction;
        if (pixGetDepth(pixs) == 1)
            pixt1 = pixScaleToGray(pixs, scale);
        else
            pixt1 = pixScale(pixs, scale, scale);
    }
    if (depth == 8)
        pixt2 = pixConvertTo8(pixt1, 0);
    else
        pixt2 = pixConvertTo32(pixt1);
    pixDestroy(&pixt1);

        /* Find position of current pix (UL corner plus size) */
    if (n == 0) {
        top = 0;
        left = 0;
    }
    else if (newrow == 1) {
        top = bottom + space;
        left = 0;
    }
    else if (n > 0) {
        pixaGetBoxGeometry(pixa, n - 1, &bx, &by, &bw, NULL);
        top = by;
        left = bx + bw + space;
    }

    pixGetDimensions(pixt2, &w, &h, NULL);
    bottom = L_MAX(bottom, top + h);
    box = boxCreate(left, top, w, h);
    pixaAddPix(pixa, pixt2, L_INSERT);
    pixaAddBox(pixa, box, L_INSERT);

        /* Save the new bottom value */
    pix = pixaGetPix(pixa, 0, L_CLONE);
    pixSetInputFormat(pix, bottom);  /* not typical usage! */
    pixDestroy(&pix);

    return 0;
}


