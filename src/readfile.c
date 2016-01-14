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
 *      High level read functions
 *           PIXA      *pixaReadFiles()
 *           PIX       *pixRead()
 *           PIX       *pixReadWithHint()
 *           PIX       *pixReadStream()
 *
 *      Format helper
 *           l_int32    findFileFormat()
 *           l_int32    findFileFormatBuffer()
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
static const char *FILE_BMP =  "/usr/tmp/junkout.bmp";
static const char *FILE_PNG =  "/usr/tmp/junkout.png";
static const char *FILE_PNM =  "/usr/tmp/junkout.pnm";
static const char *FILE_G4 =   "/usr/tmp/junkout_g4.tif";
static const char *FILE_TIFF = "/usr/tmp/junkout.tif";
static const char *FILE_JPG =  "/usr/tmp/junkout.jpg"; 


/*---------------------------------------------------------------------*
 *         Top-level procedures for reading images from file           *
 *---------------------------------------------------------------------*/
/*!
 *  pixaReadFiles()
 *
 *      Input:  dirname
 *              root
 *      Return: pixa, or NULL on error
 *
 *  Notes:
 *      (1) 'dirname' is the full path for the directory.
 *      (2) 'root' is the part of the file name (excluding
 *          the directory) that is to be matched.  All matching
 *          filenames are read into the Pixa.
 */
PIXA *
pixaReadFiles(const char  *dirname,
              const char  *root)
{
char    *str;
l_int32  i, n1, n2, len;
PIX     *pix;
PIXA    *pixa;
SARRAY  *sa1, *sa2, *sa3;

    PROCNAME("pixaReadFiles");

    if (!dirname)
        return (PIXA *)ERROR_PTR("dirname not defined", procName, NULL);
    if (!root)
        return (PIXA *)ERROR_PTR("rootname not defined", procName, NULL);
    len = strlen(root);

    if ((sa1 = getFilenamesInDirectory(dirname)) == NULL)
        return (PIXA *)ERROR_PTR("sa1 not made", procName, NULL);

        /* Save the matching files */
    sa2 = sarrayCreate(0);
    n1 = sarrayGetCount(sa1);
    for (i = 0; i < n1; i++) {
        str = sarrayGetString(sa1, i, L_NOCOPY);
        if (!strncmp(str, root, len))
            sarrayAddString(sa2, str, L_COPY);
    }
    sarrayDestroy(&sa1);

    if ((n2 = sarrayGetCount(sa2)) == 0) {
        L_WARNING("no matching filenames", procName);
	return pixaCreate(1);   /* empty */
    }
    
        /* Sort the filenames and read the files */
    sa3 = sarraySort(NULL, sa2, L_SORT_INCREASING);
    if ((pixa = pixaCreate(n2)) == NULL)
        return (PIXA *)ERROR_PTR("pixa not made", procName, NULL);
    for (i = 0; i < n2; i++) {
        str = sarrayGetString(sa3, i, L_NOCOPY);
        if ((pix = pixRead(str)) == NULL)
            L_WARNING("pix not read from file", procName);
	pixaAddPix(pixa, pix, L_INSERT);
    }

    sarrayDestroy(&sa2);
    sarrayDestroy(&sa3);
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
 *              hint: a bitwise OR of L_HINT_* values. These are not
 *                    binding, but may be used to optimize the
 *                    decoding of images.
 *      Return: pix if OK; null on error
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
 *      Input:  file stream
 *              hint: a bitwise OR of L_HINT_* values
 *      Return: pix if OK; null on error
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
        if ((pix = pixReadStreamTiff(fp, 0)) == NULL)  /* page 0 by default */
            return (PIX *)ERROR_PTR("tiff: no pix returned", procName, NULL);
        break;

    case IFF_PNM:
        if ((pix = pixReadStreamPnm(fp)) == NULL)
            return (PIX *)ERROR_PTR("pnm: no pix returned", procName, NULL);
        break;

    case IFF_UNKNOWN:
        return (PIX *)ERROR_PTR( "Unknown format: no pix returned",
                procName, NULL);
        break;
    }

    pixSetInputFormat(pix, format);
    return pix;
}



/*!
 *  findFileFormat()
 *
 *      Input:  fp
 *      Return: format integer; 0 on error or if format not recognized
 *
 *  N.B.: this resets fp to BOF
 */
l_int32
findFileFormat(FILE  *fp)
{
l_uint8   firstbytes[12]; 

    PROCNAME("findFileFormat");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 0);

    rewind(fp);
    if (nbytesInFile(fp) < 12)
        return ERROR_INT("truncated file", procName, 0);

    fread((char *)&firstbytes, 1, 12, fp);
    rewind(fp);

    return findFileFormatBuffer(firstbytes);
}


/*!
 *  findFileFormatBuffer()
 *
 *      Input:  byte buffer (at least 12 bytes in size; we can't check) 
 *      Return: format integer; 0 on error or if format not recognized
 *
 *  Note: this allows you to determine the file format from the first
 *        12 bytes in the compressed data stream, which are stored
 *        in memory.
 */
l_int32
findFileFormatBuffer(const l_uint8  *buf)
{
l_uint16  twobytepw;

    PROCNAME("findFileFormatBuffer");

    if (!buf)
        return ERROR_INT("byte buffer not defined", procName, 0);

        /* check the bmp and tiff 2-byte header ids */
    ((char *)(&twobytepw))[0] = buf[0];
    ((char *)(&twobytepw))[1] = buf[1];

    if (convertOnBigEnd16(twobytepw) == BMP_ID)
        return IFF_BMP;

    if (twobytepw == TIFF_BIGEND_ID || twobytepw == TIFF_LITTLEEND_ID)
        return IFF_TIFF;

        /* check for the p*m 2-byte header ids */
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

        /*  check for the 8 byte PNG signature (png_signature in png.c):
         *        {137, 80, 78, 71, 13, 10, 26, 10}      */
    if (buf[0] == 137 && buf[1] == 80  && buf[2] == 78  && buf[3] == 71  &&
        buf[4] == 13  && buf[5] == 10  && buf[6] == 26  && buf[7] == 10)
            return IFF_PNG;

        /* format header not found */
    return IFF_UNKNOWN;
}


/*
 *  ioFormatTest()
 *
 *      Input:  filename (input file)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: This writes and reads a set of output files in different formats
 *        to /usr/tmp/, and tests that the result before and after
 *        is unchanged.  It should work properly on input images of
 *        any depth, with and without colormaps.
 */
l_int32
ioFormatTest(const char  *filename)
{
l_int32      d, equal, problems;
PIX         *pixs, *pixc, *pixt, *pixt2;
PIXCMAP     *cmap;

    PROCNAME("ioFormatTest");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);

    if ((pixs = pixRead(filename)) == NULL)
        return ERROR_INT("pix not made", procName, 1);

        /* Note that the reader automatically removes colormaps
         * from 1 bpp BMP images, but not from 8 bpp BMP images.
         * Therefore, if our 8 bpp image initially doesn't have a
         * colormap, we are going to need to remove it from any
         * pix read from a BMP file!  */
    pixc = pixClone(pixs);  /* laziness */
    cmap = pixGetColormap(pixc);  /* colormap; can be NULL */
    d = pixGetDepth(pixc);

    problems = FALSE;
    switch (d)
    {
    case 1:
    case 8:
    case 32:
            /* BMP always writes colormaps for 1 and 8 bpp, so we
             * remove it if the input image doesn't have a colormap */
        pixWrite(FILE_BMP, pixc, IFF_BMP);
        pixt = pixRead(FILE_BMP);
        if (!cmap)
            pixt2 = pixRemoveColormap(pixt, REMOVE_CMAP_BASED_ON_SRC);
        else
            pixt2 = pixClone(pixt);
        pixEqual(pixc, pixt2, &equal);
        if (!equal) {
            fprintf(stderr, "bad bmp image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);
        pixDestroy(&pixt2);

        pixWrite(FILE_PNG, pixc, IFF_PNG);
        pixt = pixRead(FILE_PNG);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad png image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);

            /* Because 8 bpp tiff always writes 256 entry colormaps, the
             * colormap sizes may be different for 8 bpp images with
             * colormap; we are testing if the image content is the same */
        pixWrite(FILE_TIFF, pixc, IFF_TIFF);
        pixt = pixRead(FILE_TIFF);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad tiff uncompressed image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);

        if (d == 1) {
            pixWrite(FILE_G4, pixc, IFF_TIFF_G4);
            pixt = pixRead(FILE_G4);
            pixEqual(pixc, pixt, &equal);
            if (!equal) {
                fprintf(stderr, "bad tiff g4 image\n");
                problems = TRUE;
            }
            pixDestroy(&pixt);
        }

            /* pnm doesn't have colormaps, so when we write colormapped
             * pix out as pnm, the colormap is removed.  Thus for the test,
             * we must remove the colormap from pixc before testing.  */
        pixWrite(FILE_PNM, pixc, IFF_PNM);
        pixt = pixRead(FILE_PNM);
        if (cmap)
            pixt2 = pixRemoveColormap(pixc, REMOVE_CMAP_BASED_ON_SRC);
        else
            pixt2 = pixClone(pixc);
        pixEqual(pixt, pixt2, &equal);
        if (!equal) {
            fprintf(stderr, "bad pnm image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);
        pixDestroy(&pixt2);
        break;
    case 2:
    case 4:
        pixWrite(FILE_PNG, pixc, IFF_PNG);
        pixt = pixRead(FILE_PNG);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad png 2 or 4 bpp image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);

            /* I believe the 2 and 4 bpp tiff images with colormaps
             * have colormap sizes 4 and 16, rsp.  This test should
             * work properly on the content, regardless of the number
             * of color entries in pixc. */
        pixWrite(FILE_TIFF, pixc, IFF_TIFF);
        pixt = pixRead(FILE_TIFF);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad tiff uncompressed image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);

            /* Note: this works in this test, but nobody else can
             * read 2 bpp colormapped bmp files that are generated here! */
        pixWrite(FILE_BMP, pixc, IFF_BMP);
        pixt = pixRead(FILE_BMP);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad bmp 2 or 4 bpp image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);
        break;
    case 16:
        pixWrite(FILE_PNG, pixc, IFF_PNG);
        pixt = pixRead(FILE_PNG);
        pixEqual(pixc, pixt, &equal);
        if (!equal) {
            fprintf(stderr, "bad png 16 bpp image\n");
            problems = TRUE;
        }
        pixDestroy(&pixt);
    default:
        return ERROR_INT("d not in {1,2,4,8,16,32}", procName, 1);
    }

    if (problems == FALSE)
        L_INFO("all formats read and written OK", procName);

    pixDestroy(&pixc);
    pixDestroy(&pixs);
    return 0;
}

