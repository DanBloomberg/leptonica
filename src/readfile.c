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
 *  readfile.c:  reads image on file into memory
 *
 *      Top-level functions for reading images from file
 *           PIXA      *pixaReadFiles()     [unix only]
 *           PIXA      *pixaReadFilesSA()
 *           PIX       *pixRead()
 *           PIX       *pixReadWithHint()
 *           PIX       *pixReadStream()
 *
 *      Format finders
 *           l_int32    findFileFormat()
 *           l_int32    findFileFormatBuffer()
 *           l_int32    fileFormatIsTiff()
 *
 *      Read from memory
 *           PIX       *pixReadMem()
 *
 *      Test function for I/O with different formats 
 *           l_int32    ioFormatTest()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


    /*  choose type of PIX to be generated  */
enum {
    READ_24_BIT_COLOR = 0,     /* read in as 24 (really 32) bit pix */
    CONVERT_TO_PALETTE = 1,    /* convert to 8 bit colormapped pix */
    READ_GRAY = 2              /* read gray only */
};

    /* Output files for ioFormatTest().
     * Note that the test for jpeg is not yet implemented */
static const char *FILE_BMP  =  "/tmp/junkout.bmp";
static const char *FILE_PNG  =  "/tmp/junkout.png";
static const char *FILE_PNM  =  "/tmp/junkout.pnm";
static const char *FILE_G3   =  "/tmp/junkout_g3.tif";
static const char *FILE_G4   =  "/tmp/junkout_g4.tif";
static const char *FILE_RLE  =  "/tmp/junkout_rle.tif";
static const char *FILE_PB   =  "/tmp/junkout_packbits.tif";
static const char *FILE_LZW  =  "/tmp/junkout_lzw.tif";
static const char *FILE_ZIP  =  "/tmp/junkout_zip.tif";
static const char *FILE_TIFF =  "/tmp/junkout.tif";
static const char *FILE_JPG  =  "/tmp/junkout.jpg"; 


/*---------------------------------------------------------------------*
 *          Top-level functions for reading images from file           *
 *---------------------------------------------------------------------*/
/*!
 *  pixaReadFiles()
 *
 *      Input:  dirname
 *              substr (<optional> substring filter on filenames; can be NULL)
 *      Return: pixa, or NULL on error
 *
 *  Notes:
 *      (1) 'dirname' is the full path for the directory.
 *      (2) 'substr' is the part of the file name (excluding
 *          the directory) that is to be matched.  All matching
 *          filenames are read into the Pixa.  If substr is NULL,
 *          all filenames are read into the Pixa.
 *      (3) This is unix only; it does not work on Windows.
 */
PIXA *
pixaReadFiles(const char  *dirname,
              const char  *substr)
{
PIXA    *pixa;
SARRAY  *sa;

    PROCNAME("pixaReadFiles");

    if (!dirname)
        return (PIXA *)ERROR_PTR("dirname not defined", procName, NULL);

    if ((sa = getSortedPathnamesInDirectory(dirname, substr, 0, 0)) == NULL)
        return (PIXA *)ERROR_PTR("sa not made", procName, NULL);

    pixa = pixaReadFilesSA(sa);
    sarrayDestroy(&sa);
    return pixa;
}


/*!
 *  pixaReadFilesSA()
 *
 *      Input:  sarray (full pathnames for all files)
 *      Return: pixa, or null on error
 */
PIXA *
pixaReadFilesSA(SARRAY  *sa)
{
char    *str;
l_int32  i, n;
PIX     *pix;
PIXA    *pixa;

    PROCNAME("pixaReadFilesSA");

    if (!sa)
        return (PIXA *)ERROR_PTR("sa not defined", procName, NULL);

    n = sarrayGetCount(sa);
    pixa = pixaCreate(n);
    for (i = 0; i < n; i++) {
        str = sarrayGetString(sa, i, L_NOCOPY);
        if ((pix = pixRead(str)) == NULL) {
            L_WARNING_STRING("pix not read from file %s", procName, str);
            continue;
        }
	pixaAddPix(pixa, pix, L_INSERT);
    }

    return pixa;
}


/*!
 *  pixRead()
 *
 *      Input:  filename (with full pathname or in local directory)
 *      Return: pix if OK; null on error
 */
PIX *
pixRead(const char  *filename)
{
FILE  *fp;
PIX   *pix;

    PROCNAME("pixRead");

    if (!filename)
        return (PIX *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIX *)ERROR_PTR("image file not found", procName, NULL);
    pix = pixReadStream(fp, 0);
    fclose(fp);

    if (!pix)
        return (PIX *)ERROR_PTR("image not returned", procName, NULL);
    return pix;
}

/*!
 *  pixReadWithHint()
 *
 *      Input:  filename (with full pathname or in local directory)
 *              hint (bitwise OR of L_HINT_* values for jpeg; use 0 for no hint)
 *      Return: pix if OK; null on error
 *
 *  Notes:
 *      (1) The hint is not binding, but may be used to optimize jpeg decoding.
 *          Use 0 for no hinting.
 */
PIX *
pixReadWithHint(const char  *filename,
                l_int32      hint)
{
FILE  *fp;
PIX   *pix;

    PROCNAME("pixReadWithHint");

    if (!filename)
        return (PIX *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIX *)ERROR_PTR("image file not found", procName, NULL);
    pix = pixReadStream(fp, hint);
    fclose(fp);

    if (!pix)
        return (PIX *)ERROR_PTR("image not returned", procName, NULL);
    return pix;
}


/*!
 *  pixReadStream()
 *
 *      Input:  fp (file stream)
 *              hint (bitwise OR of L_HINT_* values for jpeg; use 0 for no hint)
 *      Return: pix if OK; null on error
 *
 *  Notes:
 *      (1) The hint only applies to jpeg.
 */
PIX *
pixReadStream(FILE    *fp,
              l_int32  hint)
{
l_int32  format;
PIX     *pix;

    PROCNAME("pixReadStream");

    if (!fp)
        return (PIX *)ERROR_PTR("stream not defined", procName, NULL);
    pix = NULL;

    format = findFileFormat(fp);

    switch (format)
    {
    case IFF_BMP:
        if ((pix = pixReadStreamBmp(fp)) == NULL )
            return (PIX *)ERROR_PTR( "bmp: no pix returned", procName, NULL);
        break;

    case IFF_JFIF_JPEG:
        if ((pix = pixReadStreamJpeg(fp, READ_24_BIT_COLOR, 1, NULL, hint))
                == NULL)
            return (PIX *)ERROR_PTR( "jpeg: no pix returned", procName, NULL);
        break;

    case IFF_PNG:
        if ((pix = pixReadStreamPng(fp)) == NULL)
            return (PIX *)ERROR_PTR("png: no pix returned", procName, NULL);
        break;

    case IFF_TIFF:
    case IFF_TIFF_PACKBITS:
    case IFF_TIFF_RLE:
    case IFF_TIFF_G3:
    case IFF_TIFF_G4:
    case IFF_TIFF_LZW:
    case IFF_TIFF_ZIP:
        if ((pix = pixReadStreamTiff(fp, 0)) == NULL)  /* page 0 by default */
            return (PIX *)ERROR_PTR("tiff: no pix returned", procName, NULL);
        break;

    case IFF_PNM:
        if ((pix = pixReadStreamPnm(fp)) == NULL)
            return (PIX *)ERROR_PTR("pnm: no pix returned", procName, NULL);
        break;

    case IFF_GIF:
        if ((pix = pixReadStreamGif(fp)) == NULL)
            return (PIX *)ERROR_PTR("gif: no pix returned", procName, NULL);
        break;

    case IFF_UNKNOWN:
        return (PIX *)ERROR_PTR( "Unknown format: no pix returned",
                procName, NULL);
        break;
    }

    if (pix)
        pixSetInputFormat(pix, format);
    return pix;
}


/*---------------------------------------------------------------------*
 *                            Format finders                           *
 *---------------------------------------------------------------------*/
/*!
 *  findFileFormat()
 *
 *      Input:  fp (file stream)
 *      Return: format integer; 0 on error or if format not recognized
 *
 *  N.B.: this resets fp to BOF
 */
l_int32
findFileFormat(FILE  *fp)
{
l_uint8  firstbytes[8]; 
l_int32  format, ret;

    PROCNAME("findFileFormat");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 0);

    rewind(fp);
    if (fnbytesInFile(fp) < 8)
        return ERROR_INT("truncated file", procName, 0);

    ret = fread((char *)&firstbytes, 1, 8, fp);
    if (ret != 8)
        return ERROR_INT("failed to read first 8 bytes of file", procName, 0);
    rewind(fp);

    format = findFileFormatBuffer(firstbytes);
    if (format == IFF_TIFF) {
        findTiffCompression(fp, &format);
        rewind(fp);
    }
    return format;
}


/*!
 *  findFileFormatBuffer()
 *
 *      Input:  byte buffer (at least 8 bytes in size; we can't check) 
 *      Return: format integer; 0 on error or if format not recognized
 *
 *  Notes:
 *      (1) This determines the file format from the first 8 bytes in
 *          the compressed data stream, which are stored in memory.
 *      (2) For tiff files, this returns IFF_TIFF.  The specific tiff
 *          compression is then determined using findTiffCompression().
 */
l_int32
findFileFormatBuffer(const l_uint8  *buf)
{
l_uint16  twobytepw;

    PROCNAME("findFileFormatBuffer");

    if (!buf)
        return ERROR_INT("byte buffer not defined", procName, 0);

        /* Check the bmp and tiff 2-byte header ids */
    ((char *)(&twobytepw))[0] = buf[0];
    ((char *)(&twobytepw))[1] = buf[1];

    if (convertOnBigEnd16(twobytepw) == BMP_ID)
        return IFF_BMP;

    if (twobytepw == TIFF_BIGEND_ID || twobytepw == TIFF_LITTLEEND_ID)
        return IFF_TIFF;

        /* Check for the p*m 2-byte header ids */
    if ((buf[0] == 'P' && buf[1] == '4') || /* newer packed */
        (buf[0] == 'P' && buf[1] == '1'))   /* old format */
            return IFF_PNM;

    if ((buf[0] == 'P' && buf[1] == '5') || /* newer */
        (buf[0] == 'P' && buf[1] == '2'))   /* old */
            return IFF_PNM;

    if ((buf[0] == 'P' && buf[1] == '6') || /* newer */
        (buf[0] == 'P' && buf[1] == '3'))   /* old */
            return IFF_PNM;

        /*  Consider the first 11 bytes of the standard JFIF JPEG header:
         *    - The first two bytes are the most important:  0xffd8.
         *    - The next two bytes are the jfif marker: 0xffe0. 
         *      Not all jpeg files have this marker.
         *    - The next two bytes are the header length.
         *    - The next 5 bytes are a null-terminated string.
         *      For JFIF, the string is "JFIF", naturally.  For others it
         *      can be "Exif" or just about anything else.
         *    - Because of all this variability, we only check the first
         *      two byte marker.  All jpeg files are identified as
         *      IFF_JFIF_JPEG.  */
    if (buf[0] == 0xff && buf[1] == 0xd8)
        return IFF_JFIF_JPEG;

        /* Check for the 8 byte PNG signature (png_signature in png.c):
         *       {137, 80, 78, 71, 13, 10, 26, 10}      */
    if (buf[0] == 137 && buf[1] == 80  && buf[2] == 78  && buf[3] == 71  &&
        buf[4] == 13  && buf[5] == 10  && buf[6] == 26  && buf[7] == 10)
            return IFF_PNG;

        /* Look for "GIF87a" or "GIF89a" */
    if (buf[0] == 'G' && buf[1] == 'I' && buf[2] == 'F' && buf[3] == '8' &&
        (buf[4] == '7' || buf[4] == '9') && buf[5] == 'a') {
        return IFF_GIF;
    }

        /* Format header not found */
    return IFF_UNKNOWN;
}


/*!
 *  fileFormatIsTiff()
 *
 *      Input:  fp (file stream)
 *      Return: 1 if file is tiff; 0 otherwise or on error
 */
l_int32
fileFormatIsTiff(FILE  *fp)
{
l_int32  format;

    PROCNAME("fileFormatIsTiff");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 0);

    format = findFileFormat(fp);
    if (format == IFF_TIFF || format == IFF_TIFF_PACKBITS ||
        format == IFF_TIFF_RLE || format == IFF_TIFF_G3 ||
        format == IFF_TIFF_G4 || format == IFF_TIFF_LZW ||
        format == IFF_TIFF_ZIP)
        return 1;
    else
        return 0;
}


/*---------------------------------------------------------------------*
 *                            Read from memory                         *
 *---------------------------------------------------------------------*/
/*!
 *  pixReadMem()
 *
 *      Input:  data (const; encoded)
 *              datasize (size of data)
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) This is a variation of pixReadStream(), where the data is read
 *          from a memory buffer rather than a file.
 *      (2) On windows, this will only read tiff formatted files from
 *          memory.  For other formats, it requires fmemopen(3).
 *          Attempts to read those formats will fail at runtime.
 *      (3) findFileFormatBuffer() requires up to 8 bytes to decide on
 *          the format.  That determines the constraint here.
 */
PIX *
pixReadMem(const l_uint8  *data,
           size_t          size)
{
l_int32  format;
PIX     *pix;

    PROCNAME("pixReadMem");

    if (!data)
        return (PIX *)ERROR_PTR("data not defined", procName, NULL);
    if (size < 8)
        return (PIX *)ERROR_PTR("size < 8", procName, NULL);
    pix = NULL;

    format = findFileFormatBuffer(data);

    switch (format)
    {
    case IFF_BMP:
        if ((pix = pixReadMemBmp(data, size)) == NULL )
            return (PIX *)ERROR_PTR( "bmp: no pix returned", procName, NULL);
        break;

    case IFF_JFIF_JPEG:
        if ((pix = pixReadMemJpeg(data, size, READ_24_BIT_COLOR, 1, NULL, 0))
                == NULL)
            return (PIX *)ERROR_PTR( "jpeg: no pix returned", procName, NULL);
        break;

    case IFF_PNG:
        if ((pix = pixReadMemPng(data, size)) == NULL)
            return (PIX *)ERROR_PTR("png: no pix returned", procName, NULL);
        break;

    case IFF_TIFF:
    case IFF_TIFF_PACKBITS:
    case IFF_TIFF_RLE:
    case IFF_TIFF_G3:
    case IFF_TIFF_G4:
    case IFF_TIFF_LZW:
    case IFF_TIFF_ZIP:
            /* Reading page 0 by default */
        if ((pix = pixReadMemTiff(data, size, 0)) == NULL)
            return (PIX *)ERROR_PTR("tiff: no pix returned", procName, NULL);
        break;

    case IFF_PNM:
        if ((pix = pixReadMemPnm(data, size)) == NULL)
            return (PIX *)ERROR_PTR("pnm: no pix returned", procName, NULL);
        break;

    case IFF_GIF:
        if ((pix = pixReadMemGif(data, size)) == NULL)
            return (PIX *)ERROR_PTR("gif: no pix returned", procName, NULL);
        break;

    case IFF_UNKNOWN:
        return (PIX *)ERROR_PTR("Unknown format: no pix returned",
                procName, NULL);
        break;
    }

        /* Set the input format.  For tiff reading from memory we lose
         * the actual input format; for 1 bpp, default to G4.  */
    if (pix) {
        if (format == IFF_TIFF && pixGetDepth(pix) == 1)
            format = IFF_TIFF_G4;
        pixSetInputFormat(pix, format);
    }

    return pix;
}


/*---------------------------------------------------------------------*
 *             Test function for I/O with different formats            *
 *---------------------------------------------------------------------*/
/*!
 *  ioFormatTest()
 *
 *      Input:  filename (input file)
 *      Return: 0 if OK; 1 on error or if the test fails
 *
 *  Notes:
 *      (1) This writes and reads a set of output files losslessly
 *          in different formats to /tmp, and tests that the
 *          result before and after is unchanged.
 *      (2) This should work properly on input images of any depth,
 *          with and without colormaps.
 *      (3) All supported formats are tested for bmp, png, tiff and
 *          non-ascii pnm.  Ascii pnm also works (but who'd ever want
 *          to use it?)   We allow 2 bpp bmp, although it's not
 *          supported elsewhere.  And we don't support reading
 *          16 bpp png, although this can be turned on in pngio.c.
 */
l_int32
ioFormatTest(const char  *filename)
{
l_int32   d, equal, problems;
PIX      *pixs, *pixc, *pixt, *pixt2;
PIXCMAP  *cmap;

    PROCNAME("ioFormatTest");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);

    if ((pixs = pixRead(filename)) == NULL)
        return ERROR_INT("pixs not made", procName, 1);

        /* Note that the reader automatically removes colormaps
         * from 1 bpp BMP images, but not from 8 bpp BMP images.
         * Therefore, if our 8 bpp image initially doesn't have a
         * colormap, we are going to need to remove it from any
         * pix read from a BMP file. */
    pixc = pixClone(pixs);  /* laziness */
    cmap = pixGetColormap(pixc);  /* colormap; can be NULL */
    d = pixGetDepth(pixc);

    problems = FALSE;

        /* ----------------------- BMP -------------------------- */

        /* BMP works for 1, 2, 4, 8 and 32 bpp images.
         * It always writes colormaps for 1 and 8 bpp, so we must
         * remove it after readback if the input image doesn't have
         * a colormap.  Although we can write/read 2 bpp BMP, nobody
         * else can read them! */
    if (d == 1 || d == 8) {
        L_INFO("write/read bmp", procName);
        pixWrite(FILE_BMP, pixc, IFF_BMP);
        pixt = pixRead(FILE_BMP);
        if (!cmap)
            pixt2 = pixRemoveColormap(pixt, REMOVE_CMAP_BASED_ON_SRC);
        else
            pixt2 = pixClone(pixt);
        pixEqual(pixc, pixt2, &equal);
        if (!equal) {
            L_INFO("   **** bad bmp image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);
        pixDestroy(&pixt2);
    }

    if (d == 2 || d == 4 || d == 32) {
        L_INFO("write/read bmp", procName);
        pixWrite(FILE_BMP, pixc, IFF_BMP);
        pixt = pixRead(FILE_BMP);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad bmp image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);
    }

        /* ----------------------- PNG -------------------------- */

        /* PNG works for all depths, but here, because we strip
         * 16 --> 8 bpp on reading, we don't test png for 16 bpp. */
    if (d != 16) {
        L_INFO("write/read png", procName);
        pixWrite(FILE_PNG, pixc, IFF_PNG);
        pixt = pixRead(FILE_PNG);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad png image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);
    }

        /* ----------------------- TIFF -------------------------- */

        /* TIFF works for 1, 2, 4, 8, 16 and 32 bpp images.
         * Because 8 bpp tiff always writes 256 entry colormaps, the
         * colormap sizes may be different for 8 bpp images with
         * colormap; we are testing if the image content is the same.
         * Likewise, the 2 and 4 bpp tiff images with colormaps
         * have colormap sizes 4 and 16, rsp.  This test should
         * work properly on the content, regardless of the number
         * of color entries in pixc. */

        /* tiff uncompressed works for all pixel depths */
    L_INFO("write/read uncompressed tiff", procName);
    pixWrite(FILE_TIFF, pixc, IFF_TIFF);
    pixt = pixRead(FILE_TIFF);
    pixEqual(pixc, pixt, &equal);
    if (!equal) {
        L_INFO("   **** bad tiff uncompressed image ****", procName);
        problems = TRUE;
    }
    pixDestroy(&pixt);

        /* tiff lzw works for all pixel depths */
    L_INFO("write/read lzw compressed tiff", procName);
    pixWrite(FILE_LZW, pixc, IFF_TIFF_LZW);
    pixt = pixRead(FILE_LZW);
    pixEqual(pixc, pixt, &equal);
    if (!equal) {
        L_INFO("   **** bad tiff lzw compressed image ****", procName);
        problems = TRUE;
    }
    pixDestroy(&pixt);

        /* tiff adobe deflate (zip) works for all pixel depths */
    L_INFO("write/read zip compressed tiff", procName);
    pixWrite(FILE_ZIP, pixc, IFF_TIFF_ZIP);
    pixt = pixRead(FILE_ZIP);
    pixEqual(pixc, pixt, &equal);
    if (!equal) {
        L_INFO("   **** bad tiff zip compressed image ****", procName);
        problems = TRUE;
    }
    pixDestroy(&pixt);

        /* tiff g4, g3, rle and packbits work for 1 bpp */
    if (d == 1) {
        L_INFO("write/read g4 compressed tiff", procName);
        pixWrite(FILE_G4, pixc, IFF_TIFF_G4);
        pixt = pixRead(FILE_G4);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad tiff g4 image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);

        L_INFO("write/read g3 compressed tiff", procName);
        pixWrite(FILE_G3, pixc, IFF_TIFF_G3);
        pixt = pixRead(FILE_G3);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad tiff g3 image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);

        L_INFO("write/read rle compressed tiff", procName);
        pixWrite(FILE_RLE, pixc, IFF_TIFF_RLE);
        pixt = pixRead(FILE_RLE);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad tiff rle image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);

        L_INFO("write/read packbits compressed tiff", procName);
        pixWrite(FILE_PB, pixc, IFF_TIFF_PACKBITS);
        pixt = pixRead(FILE_PB);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            L_INFO("   **** bad tiff packbits image ****", procName);
            problems = TRUE;
        }
        pixDestroy(&pixt);
    }

        /* ----------------------- PNM -------------------------- */

        /* pnm works for 1, 2, 4, 8, 16 and 32 bpp.
         * pnm doesn't have colormaps, so when we write colormapped
         * pix out as pnm, the colormap is removed.  Thus for the test,
         * we must remove the colormap from pixc before testing.  */
    L_INFO("write/read pnm", procName);
    pixWrite(FILE_PNM, pixc, IFF_PNM);
    pixt = pixRead(FILE_PNM);
    if (cmap)
        pixt2 = pixRemoveColormap(pixc, REMOVE_CMAP_BASED_ON_SRC);
    else
        pixt2 = pixClone(pixc);
    pixEqual(pixt, pixt2, &equal);
    if (!equal) {
        L_INFO("   **** bad pnm image ****", procName);
        problems = TRUE;
    }
    pixDestroy(&pixt);
    pixDestroy(&pixt2);

    if (problems == FALSE)
        L_INFO("All formats read and written OK!", procName);

    pixDestroy(&pixc);
    pixDestroy(&pixs);
    return problems;
}

