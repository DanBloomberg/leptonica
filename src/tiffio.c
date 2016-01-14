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
 *  tiffio.c
 *                     
 *     Reading tiff:
 *             PIX       *pixReadTiff()    [ special top level ]
 *             PIX       *pixReadStreamTiff()
 *      static PIX       *pixReadFromTiffStream()
 *
 *     Writing tiff:
 *             l_int32    pixWriteTiffCustom()   [ special top level ]
 *             l_int32    pixWriteTiff()   [ special top level ]
 *             l_int32    pixWriteStreamTiff()
 *             l_int32    pixWriteToTiffStream()
 *      static l_int32    writeCustomTiffTags()
 *
 *     Information about tiff file
 *             l_int32    fprintTiffInfo()
 *             l_int32    tiffGetCount()
 *             l_int32    readHeaderTiff()
 *             l_int32    freadHeaderTiff()
 *             l_int32    findTiffCompression()
 *
 *     Open tiff stream from file stream
 *      static TIFF      *fopenTiff()
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "allheaders.h"

static const l_int32  DEFAULT_RESOLUTION = 300;   /* ppi */
static const l_int32  MAX_PAGES_IN_TIFF_FILE = 1000;  /* should be enough */


    /* All functions with TIFF interfaces are static */
static TIFF      *fopenTiff(FILE *fp, const char *modestr); 
static PIX       *pixReadFromTiffStream(TIFF *tif);
static l_int32    writeCustomTiffTags(TIFF *tif, NUMA *natags,
                                      SARRAY *savals, SARRAY  *satypes,
                                      NUMA *nasizes);

    /* This structure defines a transform to be performed on a TIFF image
     * (note that the same transformation can be represented in several different
     * ways using this structure since vflip+hflip+counterclockwise ==
     * clockwise) */
struct tiff_transform {
    int vflip;  /* if non-zero, image needs a vertical fip */
    int hflip;  /* if non-zero, image needs a horizontal flip */
    int rotate; /* -1 -> counterclockwise 90-degree rotation,
                  0 -> no rotation
                  1 -> clockwise 90-degree rotation */
};

    /* This describes the transformations needed for a given orientation
     * tag. This tag values start at 1, so you need to subtract 1 to get a
     * valid index into this array */
static struct tiff_transform tiff_orientation_transforms[] = {
    {0, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {1, 0, 0},
    {0, 1, -1},
    {0, 0, 1},
    {0, 1, 1},
    {0, 0, -1}
};


/*--------------------------------------------------------------*
 *                      Reading from file                       *
 *--------------------------------------------------------------*/
/*!
 *  pixReadTiff()
 *
 *      Input:  filename
 *              page number (0 based)
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) This is a version of pixRead(), specialized for tiff
 *          files, that allows specification of the page to be returned
 *      (2) We must call findFileFormat() to get the actual tiff 
 *          compression format.
 */
PIX *
pixReadTiff(const char  *filename,
            l_int32      n)
{
l_int32  format;
FILE    *fp;
PIX     *pix;

    PROCNAME("pixReadTiff");

    if (!filename)
        return (PIX *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIX *)ERROR_PTR("image file not found", procName, NULL);
    pix = pixReadStreamTiff(fp, n);
    format = findFileFormat(fp);
    fclose(fp);
    pixSetInputFormat(pix, format);

    if (!pix)
        return (PIX *)ERROR_PTR("image not returned", procName, NULL);
    return pix;
}


/*--------------------------------------------------------------*
 *                     Reading from stream                      *
 *--------------------------------------------------------------*/
/*!
 *  pixReadStreamTiff()
 *
 *      Input:  stream
 *              page number (0 based: start with 0)
 *      Return: pix, or null on error (e.g., if the page number is invalid)
 */
PIX *
pixReadStreamTiff(FILE    *fp,
                  l_int32  n)
{
l_int32  i, pagefound;
PIX     *pix;
TIFF    *tif;

    PROCNAME("pixReadStreamTiff");

    if (!fp)
        return (PIX *)ERROR_PTR("stream not defined", procName, NULL);

    if ((tif = fopenTiff(fp, "r")) == NULL)
        return (PIX *)ERROR_PTR("tif not opened", procName, NULL);

    pagefound = FALSE;
    pix = NULL;
    for (i = 0; i < MAX_PAGES_IN_TIFF_FILE; i++) {
        if (i == n) {
            pagefound = TRUE;
            pix = pixReadFromTiffStream(tif);
            break;
        }
        if (TIFFReadDirectory(tif) == 0)
            break;
    }

    if (pagefound == FALSE) {
        L_WARNING_INT("tiff page %d not found", procName, n);
        TIFFCleanup(tif);
        return NULL;
    }

        /* Set to generic input format; refinement is done
	 * in pixReadStream() using findFileFormat(). */
    pixSetInputFormat(pix, IFF_TIFF);
    TIFFCleanup(tif);
    return pix;
}


/*!
 *  pixReadFromTiffStream()
 *
 *      Input:  stream
 *      Return: pix, or null on error
 */
static PIX *
pixReadFromTiffStream(TIFF  *tif)
{
l_uint8   *linebuf, *data;
l_uint16   spp, bps, bpp, tiffbpl, photometry, orientation;
l_uint16  *redmap, *greenmap, *bluemap;
l_int32    d, wpl, bpl, i, j, k, ncolors;
l_uint32   w, h, res;
l_uint32  *line, *ppixel;
l_float32  fres;
PIX       *pix;
PIXCMAP   *cmap;

    PROCNAME("pixReadFromTiffStream");

    if (!tif)
        return (PIX *)ERROR_PTR("tif not defined", procName, NULL);

        /* Use default fields for bps and spp */
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    bpp = bps * spp;
    if (bpp > 24)
        return (PIX *)ERROR_PTR("can't handle bpp > 24", procName, NULL);
    if (spp == 1)
        d = bps;
    else if (spp == 3)
        d = 32;
    else
        return (PIX *)ERROR_PTR("spp not in set {1,3}", procName, NULL);

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    tiffbpl = TIFFScanlineSize(tif);

    if ((linebuf = (l_uint8 *)CALLOC(tiffbpl + 1, sizeof(l_uint8))) == NULL)
        return (PIX *)ERROR_PTR("calloc fail for linebuf", procName, NULL);
        
    if ((pix = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pix not made", procName, NULL);
    data = (l_uint8 *)pixGetData(pix);
    wpl = pixGetWpl(pix);
    bpl = 4 * wpl;

        /* Read the data */
    if (spp == 1) {
        for (i = 0 ; i < h ; i++) {
            if (TIFFReadScanline(tif, linebuf, i, 0) < 0)
                return (PIX *)ERROR_PTR("line read fail", procName, NULL);
            memcpy((char *)data, (char *)linebuf, tiffbpl);
            data += bpl;
        }
        if (bps <= 8)
            pixEndianByteSwap(pix);
        else   /* bps == 16 */
            pixEndianTwoByteSwap(pix);
    }
    else {  /* rgb */
        line = pixGetData(pix);
        for (i = 0 ; i < h ; i++, line += wpl)
        {
            if (TIFFReadScanline(tif, linebuf, i, 0) < 0)
                return (PIX *)ERROR_PTR("line read fail", procName, NULL);
            for (j = 0, k = 0, ppixel = line; j < w; j++) {
                SET_DATA_BYTE(ppixel, COLOR_RED, linebuf[k++]);
                SET_DATA_BYTE(ppixel, COLOR_GREEN, linebuf[k++]);
                SET_DATA_BYTE(ppixel, COLOR_BLUE, linebuf[k++]);
                ppixel++;
            } 
        }
    }

    if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &fres)) {
        res = (l_uint32)fres;
        pixSetXRes(pix, res);
    }
    if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &fres)) {
        res = (l_uint32)fres;
        pixSetYRes(pix, res);
    }

    if (TIFFGetField(tif, TIFFTAG_COLORMAP, &redmap, &greenmap, &bluemap)) {
            /* Save the colormap as a pix cmap.  Because the
             * tiff colormap components are 16 bit unsigned,
             * and go from black (0) to white (0xffff), the
             * the pix cmap takes the most significant byte. */
        if ((cmap = pixcmapCreate(bps)) == NULL)
            return (PIX *)ERROR_PTR("cmap not made", procName, NULL);
        ncolors = 1 << bps;
        for (i = 0; i < ncolors; i++)
            pixcmapAddColor(cmap, redmap[i] >> 8, greenmap[i] >> 8,
                            bluemap[i] >> 8);
        pixSetColormap(pix, cmap);
    }
    else {   /* No colormap: check photometry and invert if necessary */
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometry);
        if ((d == 1 && photometry == PHOTOMETRIC_MINISBLACK) ||
            (d == 8 && photometry == PHOTOMETRIC_MINISWHITE))
            pixInvert(pix, pix);
    }

    if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation)) {
        if (orientation >= 1 && orientation <= 8) {
            struct tiff_transform *transform =
              &tiff_orientation_transforms[orientation - 1];
            if (transform->vflip) pixFlipTB(pix, pix);
            if (transform->hflip) pixFlipLR(pix, pix);
            if (transform->rotate) {
                PIX *oldpix = pix;
                pix = pixRotate90(oldpix, transform->rotate);
                pixDestroy(&oldpix);
            }
        }
    }

    FREE(linebuf);
    return pix;
}


/*--------------------------------------------------------------*
 *                       Writing to file                        *
 *--------------------------------------------------------------*/
/*! 
 *  pixWriteTiffCustom()
 *
 *      Input:  filename (to write to)
 *              pix
 *              comptype (IFF_TIFF, IFF_TIFF_PACKBITS,
 *                        IFF_TIFF_G3, IFF_TIFF_G4)
 *                        IFF_TIFF_LZW, IFF_TIFF_ZIP)
 *              modestring ("a" or "w")
 *              natags (<optional> NUMA of custom tiff tags)
 *              savals (<optional> SARRAY of values)
 *              satypes (<optional> SARRAY of types)
 *              nasizes (<optional> NUMA of sizes)
 *      Return: 0 if OK, 1 on error
 *
 *  Usage:
 *      (1) This writes a page image to a tiff file, with optional
 *          extra tags defined in tiff.h
 *      (2) For multi-page tiff, write the first pix with mode "w" and
 *          all subsequent pix with mode "a".
 *      (3) For the custom tiff tags:
 *          (a) The three arrays {natags, savals, satypes} must all be
 *              either NULL or defined and of equal size.
 *          (b) If they are defined, the tags are an array of integers,
 *              the vals are an array of values in string format, and
 *              the types are an array of types in string format. 
 *          (c) All valid tags are definined in tiff.h.
 *          (d) The types allowed are the set of strings:
 *                "char*"
 *                "l_uint8*"
 *                "l_uint16"
 *                "l_uint32"
 *                "l_int32"
 *                "l_float64"
 *                "l_uint16-l_uint16" (note the dash; use it between the
 *                                    two l_uint16 vals in the val string)
 *              Of these, "char*" and "l_uint16" are the most commonly used.
 *          (e) The last array, nasizes, is also optional.  It is for
 *              tags that take an array of bytes for a value, a number of
 *              elements in the array, and a type that is either "char*"
 *              or "l_uint8*" (probably either will work). 
 *              Use NULL if there are no such tags.
 *          (f) VERY IMPORTANT: if there are any tags that require the
 *              extra size value, stored in nasizes, they must be
 *              written first!
 */
l_int32
pixWriteTiffCustom(const char  *filename,
                   PIX         *pix,
                   l_int32      comptype,
                   const char  *modestring,
                   NUMA        *natags,
                   SARRAY      *savals,
                   SARRAY      *satypes,
                   NUMA        *nasizes)
{
l_int32  ret;
TIFF    *tif;

    PROCNAME("pixWriteTiffCustom");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);

    if ((tif = TIFFOpen(filename, modestring)) == NULL)
        return ERROR_INT("tif not opened", procName, 1);
    ret = pixWriteToTiffStream(tif, pix, comptype, natags, savals,
                               satypes, nasizes);
    TIFFClose(tif);

    return ret;
}


/*! 
 *  pixWriteTiff()
 *
 *      Input:  filename (to write to)
 *              pix
 *              comptype (IFF_TIFF, IFF_TIFF_PACKBITS,
 *                        IFF_TIFF_G3, IFF_TIFF_G4,
 *                        IFF_TIFF_LZW, IFF_TIFF_ZIP)
 *              modestring ("a" or "w")
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Write the first pix with "w" and all subsequent ones with "a".
 */
l_int32
pixWriteTiff(const char  *filename,
             PIX         *pix,
             l_int32      comptype,
             const char  *modestring)
{
l_int32  ret;
TIFF    *tif;

    PROCNAME("pixWriteTiff");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1 );
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1 );

    if ((tif = TIFFOpen(filename, modestring)) == NULL)
        return ERROR_INT("tif not opened", procName, 1);
    ret = pixWriteToTiffStream(tif, pix, comptype, NULL, NULL, NULL, NULL);
    TIFFClose(tif);

    return ret;
}



/*--------------------------------------------------------------*
 *                       Writing to stream                      *
 *--------------------------------------------------------------*/
/*!
 *  pixWriteStreamTiff()
 *
 *      Input:  stream (opened for append or write)
 *              pix
 *              comptype (IFF_TIFF, IFF_TIFF_PACKBITS,
 *                        IFF_TIFF_G3, IFF_TIFF_G4,
 *                        IFF_TIFF_LZW, IFF_TIFF_ZIP)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) For images with bpp > 1, this resets the comptype, if
 *          necessary, to write uncompressed data.
 *      (2) G3 and G4 are only defined for 1 bpp.
 *      (3) We only allow PACKBITS for bpp = 1, because for bpp > 1
 *          it typically expands images that are not synthetically generated.
 *      (4) G4 compression is typically about twice as good as G3.
 *          G4 is excellent for binary compression of text/line-art,
 *          but terrible for halftones and dithered patterns.  (In
 *          fact, G4 on halftones can give a file that is larger
 *          than uncompressed!)  If a binary image has dithered
 *          regions, it is usually better to compress with png.
 */
l_int32
pixWriteStreamTiff(FILE    *fp,
                   PIX     *pix,
                   l_int32  comptype)
{
TIFF  *tif;

    PROCNAME("pixWriteStreamTiff");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1 );
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1 );

    if (pixGetDepth(pix) != 1 && comptype != IFF_TIFF &&
        comptype != IFF_TIFF_LZW && comptype != IFF_TIFF_ZIP) {
        L_WARNING("invalid compression type for image with bpp > 1", procName);
        comptype = IFF_TIFF;
    }

    if ((tif = fopenTiff(fp, "w")) == NULL)
        return ERROR_INT("tif not opened", procName, 1);

    if (pixWriteToTiffStream(tif, pix, comptype, NULL, NULL, NULL, NULL)) {
        TIFFCleanup(tif);
        return ERROR_INT("tif write error", procName, 1);
    }
    
    TIFFCleanup(tif);
    return 0;
}


/*!
 *  pixWriteToTiffStream()
 *
 *      Input:  tif (data structure, opened to a file)
 *              pix
 *              comptype  (IFF_TIFF: for any image; no compression
 *                         IFF_TIFF_PACKBITS: for 1 bpp only
 *                         IFF_TIFF_G4 and IFF_TIFF_G3: for 1 bpp only
 *                         IFF_TIFF_LZW, IFF_TIFF_ZIP: for any image
 *              natags (<optional> NUMA of custom tiff tags)
 *              savals (<optional> SARRAY of values)
 *              satypes (<optional> SARRAY of types)
 *              nasizes (<optional> NUMA of sizes)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This static function should be called through higher level
 *          functions, such as pixWriteTiffCustom(), pixWriteTiff(), or
 *          pixWriteStreamTiff().
 *      (2) We only allow PACKBITS for bpp = 1, because for bpp > 1
 *          it typically expands images that are not synthetically generated.
 *      (3) See pixWriteTiffCustom() for details on how to use
 *          the last four parameters for customized tiff tags.
 */
l_int32
pixWriteToTiffStream(TIFF    *tif,
                     PIX     *pix,
                     l_int32  comptype,
                     NUMA    *natags,
                     SARRAY  *savals,
                     SARRAY  *satypes,
                     NUMA    *nasizes)
{
l_uint8   *linebuf, *data;
l_uint16   redmap[256], greenmap[256], bluemap[256];
l_int32    w, h, d, i, j, k, wpl, bpl, tiffbpl, ncolors, cmapsize;
l_int32   *rmap, *gmap, *bmap;
l_uint32   xres, yres;
l_uint32  *line, *ppixel;
PIX       *pixt;
PIXCMAP   *cmap;
char      *text;

    PROCNAME("pixWriteToTiffStream");

    if (!tif)
        return ERROR_INT("tif stream not defined", procName, 1);
    if (!pix)
        return ERROR_INT( "pix not defined", procName, 1 );

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    d = pixGetDepth(pix);
    xres = pixGetXRes(pix);
    yres = pixGetYRes(pix);
    if (xres == 0) xres = DEFAULT_RESOLUTION;
    if (yres == 0) yres = DEFAULT_RESOLUTION;

        /* ------------------ Write out the header -------------  */
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, (l_uint32)RESUNIT_INCH);
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, (l_float64)xres);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, (l_float64)yres);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (l_uint32)w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (l_uint32)h);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

    if ((text = pixGetText(pix)) != NULL)
        TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, text);
        
    if (d == 1)
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
    else if (d == 32) {
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,
                       (l_uint16)8, (l_uint16)8, (l_uint16)8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (l_uint16)3);
    }
    else if ((cmap = pixGetColormap(pix)) == NULL)
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    else {  /* Save colormap in the tiff; not more than 256 colors */
        pixcmapToArrays(cmap, &rmap, &gmap, &bmap);
        ncolors = pixcmapGetCount(cmap);
        ncolors = L_MIN(256, ncolors);  /* max 256 */
        cmapsize = 1 << d;
        cmapsize = L_MIN(256, cmapsize);  /* power of 2; max 256 */
        if (ncolors > cmapsize) {
            L_WARNING("too many colors in cmap for tiff; truncating", procName);
            ncolors = cmapsize;
        }
        for (i = 0; i < ncolors; i++) {
            redmap[i] = (rmap[i] << 8) | rmap[i];
            greenmap[i] = (gmap[i] << 8) | gmap[i];
            bluemap[i] = (bmap[i] << 8) | bmap[i];
        }
        for (i = ncolors; i < cmapsize; i++)  /* init, even though not used */
            redmap[i] = greenmap[i] = bluemap[i] = 0;
        FREE(rmap);
        FREE(gmap);
        FREE(bmap);

        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (l_uint16)1);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (l_uint16)d);
        TIFFSetField(tif, TIFFTAG_COLORMAP, redmap, greenmap, bluemap);
    }

    if (d != 32) {
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (l_uint16)d);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (l_uint16)1);
    }

    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); 
    if (comptype == IFF_TIFF)  /* no compression */
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    else if (comptype == IFF_TIFF_G4)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
    else if (comptype == IFF_TIFF_G3)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX3);
    else if (comptype == IFF_TIFF_PACKBITS)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    else if (comptype == IFF_TIFF_LZW)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    else if (comptype == IFF_TIFF_ZIP)
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
    else { 
        L_WARNING("unknown tiff compression; using none", procName);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }

        /* This is a no-op if arrays are NULL */
    writeCustomTiffTags(tif, natags, savals, satypes, nasizes);

        /* ------------- Write out the image data -------------  */
    tiffbpl = TIFFScanlineSize(tif);
    wpl = pixGetWpl(pix);
    bpl = 4 * wpl;
    if (tiffbpl > bpl)
        fprintf(stderr, "Big trouble: tiffbpl = %d, bpl = %d\n", tiffbpl, bpl);
    if ((linebuf = (l_uint8 *)CALLOC(1, bpl)) == NULL)
        return ERROR_INT("calloc fail for linebuf", procName, 1);

        /* Use single strip for image */
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, h);

    if (d != 32) {
        if (d == 16)
            pixt = pixEndianTwoByteSwapNew(pix);
        else
            pixt = pixEndianByteSwapNew(pix);
        data = (l_uint8 *)pixGetData(pixt);
        for (i = 0 ; i < h; i++, data += bpl) {
            memcpy((char *)linebuf, (char *)data, tiffbpl);
            if (TIFFWriteScanline(tif, linebuf, i, 0) < 0)
                break;
        }
        pixDestroy(&pixt);
    }
    else {  /* d == 32 */
        line = pixGetData(pix);
        for (i = 0 ; i < h; i++, line += wpl) {
            line = pixGetData(pix) + i * wpl;
            for (j = 0, k = 0, ppixel = line; j < w; j++) {
                linebuf[k++] = GET_DATA_BYTE(ppixel, COLOR_RED);
                linebuf[k++] = GET_DATA_BYTE(ppixel, COLOR_GREEN);
                linebuf[k++] = GET_DATA_BYTE(ppixel, COLOR_BLUE);
                ppixel++;
            } 
            if (TIFFWriteScanline(tif, linebuf, i, 0) < 0)
                break;
        }
    }

/*    TIFFWriteDirectory(tif); */
    FREE(linebuf);

    return 0;
}


/*!
 *  writeCustomTiffTags()
 *
 *      Input:  tif
 *              natags (<optional> NUMA of custom tiff tags)
 *              savals (<optional> SARRAY of values)
 *              satypes (<optional> SARRAY of types)
 *              nasizes (<optional> NUMA of sizes)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes: 
 *      (1) This static function should be called indirectly through
 *          higher level functions, such as pixWriteTiffCustom(),
 *          which call pixWriteToTiffStream().  See details in
 *          pixWriteTiffCustom() for using the 4 input arrays.
 *      (2) This is a no-op if the first 3 arrays are all NULL.
 *      (3) Otherwise, the first 3 arrays must be defined and all
 *          of equal size.
 *      (4) The fourth array is always optional.
 *      (5) The most commonly used types are "char*" and "u_int16".
 *          See tiff.h for a full listing of the tiff tags. 
 *          Note that many of these tags, in particular the bit tags,
 *          are intended to be private, and cannot be set by this function.
 *          Examples are the STRIPOFFSETS and STRIPBYTECOUNTS tags,
 *          which are bit tags that are automatically set in the header,
 *          and can be extracted using tiffdump.
 */
static l_int32
writeCustomTiffTags(TIFF    *tif,
                    NUMA    *natags,
                    SARRAY  *savals,
                    SARRAY  *satypes,
                    NUMA    *nasizes)
{
char      *sval, *type;
l_int32    i, n, ns, size, tagval, val;
l_float64  dval;
l_uint32   uval, uval2;

    PROCNAME("writeCustomTiffTags");

    if (!tif)
        return ERROR_INT("tif stream not defined", procName, 1);
    if (!natags && !savals && !satypes)
        return 0;
    if (!natags || !savals || !satypes)
        return ERROR_INT("not all arrays defined", procName, 1);
    n = numaGetCount(natags);
    if ((sarrayGetCount(savals) != n) || (sarrayGetCount(satypes) != n))
        return ERROR_INT("not all sa the same size", procName, 1);

        /* The sized arrays (4 args to TIFFSetField) are written first */
    if (nasizes) {
        ns = numaGetCount(nasizes);
        if (ns > n)
            return ERROR_INT("too many 4-arg tag calls", procName, 1);
        for (i = 0; i < ns; i++) {
            numaGetIValue(natags, i, &tagval);
            sval = sarrayGetString(savals, i, 0);
            type = sarrayGetString(satypes, i, 0);
            numaGetIValue(nasizes, i, &size);
            if (strcmp(type, "char*") && strcmp(type, "l_uint8*"))
                L_WARNING("array type not char* or l_uint8*; ignore", procName);
            TIFFSetField(tif, tagval, size, sval);
        }
    }
    else
        ns = 0;

        /* The typical tags (3 args to TIFFSetField) are now written */
    for (i = ns; i < n; i++) {
        numaGetIValue(natags, i, &tagval);
        sval = sarrayGetString(savals, i, 0);
        type = sarrayGetString(satypes, i, 0);
        if (!strcmp(type, "char*")) {
            TIFFSetField(tif, tagval, sval);
        }
        else if (!strcmp(type, "l_uint16")) {
            if (sscanf(sval, "%u", &uval) == 1) {
                TIFFSetField(tif, tagval, (l_uint16)uval);
            }
            else {
                fprintf(stderr, "val %s not of type %s\n", sval, type);
                return ERROR_INT("custom tag(s) not written", procName, 1);
            }
        }
        else if (!strcmp(type, "l_uint32")) {
            if (sscanf(sval, "%u", &uval) == 1) {
                TIFFSetField(tif, tagval, uval);
            }
            else {
                fprintf(stderr, "val %s not of type %s\n", sval, type);
                return ERROR_INT("custom tag(s) not written", procName, 1);
            }
        }
        else if (!strcmp(type, "l_int32")) {
            if (sscanf(sval, "%d", &val) == 1) {
                TIFFSetField(tif, tagval, val);
            }
            else {
                fprintf(stderr, "val %s not of type %s\n", sval, type);
                return ERROR_INT("custom tag(s) not written", procName, 1);
            }
        }
        else if (!strcmp(type, "l_float64")) {
            if (sscanf(sval, "%f", &dval) == 1) {
                TIFFSetField(tif, tagval, dval);
            }
            else {
                fprintf(stderr, "val %s not of type %s\n", sval, type);
                return ERROR_INT("custom tag(s) not written", procName, 1);
            }
        }
        else if (!strcmp(type, "l_uint16-l_uint16")) {
            if (sscanf(sval, "%u-%u", &uval, &uval2) == 2) {
                TIFFSetField(tif, tagval, (l_uint16)uval, (l_uint16)uval2);
            }
            else {
                fprintf(stderr, "val %s not of type %s\n", sval, type);
                return ERROR_INT("custom tag(s) not written", procName, 1);
            }
        }
        else
            return ERROR_INT("unknown type; tag(s) not written", procName, 1);
    }
    return 0;
}
    

/*--------------------------------------------------------------*
 *                   Print info to stream                       *
 *--------------------------------------------------------------*/
/*
 *  fprintTiffInfo()
 * 
 *      Input:  stream (for output of tag data)
 *              tiffile (input)
 *      Return: 0 if OK; 1 on error
 */
l_int32
fprintTiffInfo(FILE        *fpout,
               const char  *tiffile)
{
TIFF  *tif;

    PROCNAME("fprintTiffInfo");

    if (!tiffile)
        return ERROR_INT("tiffile not defined", procName, 1);
    if (!fpout)
        return ERROR_INT("stream out not defined", procName, 1);

    if ((tif = TIFFOpen(tiffile, "r")) == NULL)
        return ERROR_INT("tif not open for read", procName, 1);

    TIFFPrintDirectory(tif, fpout, 0);
    TIFFClose(tif);

    return 0;
}


/*--------------------------------------------------------------*
 *                   Get count from stream                      *
 *--------------------------------------------------------------*/
/*
 *  tiffGetCount()
 * 
 *      Input:  stream (opened for read)
 *              &n (<return> number of images)
 *      Return: 0 if OK; 1 on error
 */
l_int32
tiffGetCount(FILE     *fp,
             l_int32  *pn)
{
l_int32  i;
TIFF    *tif;

    PROCNAME("tiffGetCount");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pn)
        return ERROR_INT("&n not defined", procName, 1);
    *pn = 0;

    if ((tif = fopenTiff(fp, "r")) == NULL)
        return ERROR_INT("tif not open for read", procName, 1);

    for (i = 1; i < MAX_PAGES_IN_TIFF_FILE; i++) {
        if (TIFFReadDirectory(tif) == 0)
            break;
    }
    *pn = i;
    TIFFCleanup(tif);
    return 0;
}


/*--------------------------------------------------------------*
 *              Get some tiff header information                *
 *--------------------------------------------------------------*/
/*!
 *  readHeaderTiff()
 *
 *      Input:  filename
 *              &width (<return>)
 *              &height (<return>)
 *              &bps (<return> bits per sample -- 1, 2, 4 or 8)
 *              &spp (<return>; samples per pixel -- 1 or 3)
 *              &res (<optional return>; resolution in x dir; NULL to ignore)
 *              &cmap (<optional return>; colormap exists; input NULL to ignore)
 *      Return: 0 if OK, 1 on error
 * 
 *  Notes:
 *      (1) If there is a colormap, cmap is returned as 1; else 0.
 */
l_int32
readHeaderTiff(const char *filename,
               l_int32    *pwidth,
               l_int32    *pheight,
               l_int32    *pbps,
               l_int32    *pspp,
               l_int32    *pres,
               l_int32    *pcmap)
{
l_int32  ret, format;
FILE    *fp;

    PROCNAME("readHeaderTiff");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!pwidth || !pheight || !pbps || !pspp)
        return ERROR_INT("input ptr(s) not all defined", procName, 1);
    *pwidth = *pheight = *pbps = *pspp = 0;
    if (pres) *pres = 0;
    if (pcmap) *pcmap = 0;
    if ((fp = fopenReadStream(filename)) == NULL)
        return ERROR_INT("image file not found", procName, 1);
    format = findFileFormat(fp);
    if (format != IFF_TIFF && format != IFF_TIFF_PACKBITS &&
        format != IFF_TIFF_G3 && format != IFF_TIFF_G4 &&
        format != IFF_TIFF_LZW && format != IFF_TIFF_ZIP)
        return ERROR_INT("file not tiff format", procName, 1);
    ret = freadHeaderTiff(fp, pwidth, pheight, pbps, pspp, pres, pcmap);
    fclose(fp);
    return ret;
}


/*!
 *  freadHeaderTiff()
 *
 *      Input:  stream
 *              &width (<return>)
 *              &height (<return>)
 *              &bps (<return> bits per sample -- 1, 2, 4 or 8)
 *              &spp (<return>; samples per pixel -- 1 or 3)
 *              &res (<optional return>; resolution in x dir; NULL to ignore)
 *              &cmap (<optional return>; colormap exists; input NULL to ignore)
 *      Return: 0 if OK, 1 on error
 */
l_int32
freadHeaderTiff(FILE     *fp,
                l_int32  *pwidth,
                l_int32  *pheight,
                l_int32  *pbps,
                l_int32  *pspp,
                l_int32  *pres,
                l_int32  *pcmap)
{
l_uint16   bps, spp;
l_uint16  *rmap, *gmap, *bmap;
l_uint32   w, h;
l_float32  fres;
TIFF      *tif;

    PROCNAME("freadHeaderTiff");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pwidth || !pheight || !pbps || !pspp)
        return ERROR_INT("input ptr(s) not all defined", procName, 1);
    
    if ((tif = fopenTiff(fp, "r")) == NULL)
        return ERROR_INT("tif not opened", procName, 1);

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    *pwidth = w;
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    *pheight = h;
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    *pbps = bps;
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    *pspp = spp;

    if (pres) {
        *pres = 300;
        if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &fres))
            *pres = (l_int32)fres;
    }
            
    if (pcmap) {
        *pcmap = 0;
        if (TIFFGetField(tif, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap))
            *pcmap = 1;
    }

    TIFFCleanup(tif);
    return 0;
}


/*!
 *  findTiffCompression()
 *
 *      Input:  stream
 *              &comp (<return> compression type)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned compression type is that defined in 
 *          the enum in imageio.h.  It is not the tiff flag value.
 *      (2) The compression type is initialized to IFF_UNKNOWN.
 *          If it is not one of the specified types, the returned
 *          type is IFF_TIFF, which indicates no compression.
 */
l_int32
findTiffCompression(FILE     *fp,
                    l_int32  *pcomp)
{
l_uint16  comp;
TIFF     *tif;

    PROCNAME("findTiffCompression");

    if (!pcomp)
        return ERROR_INT("&comp not defined", procName, 1);
    *pcomp = IFF_UNKNOWN;  /* init */
    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    
    if ((tif = fopenTiff(fp, "r")) == NULL)
        return ERROR_INT("tif not opened", procName, 1);
    TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &comp);
    switch (comp)
    {
    case COMPRESSION_CCITTFAX4:
        *pcomp = IFF_TIFF_G4;
        break;
    case COMPRESSION_CCITTFAX3:
        *pcomp = IFF_TIFF_G3;
        break;
    case COMPRESSION_CCITTRLE:
        *pcomp = IFF_TIFF_RLE;
        break;
    case COMPRESSION_PACKBITS:
        *pcomp = IFF_TIFF_PACKBITS;
        break;
    case COMPRESSION_LZW:
        *pcomp = IFF_TIFF_LZW;
        break;
    case COMPRESSION_ADOBE_DEFLATE:
        *pcomp = IFF_TIFF_ZIP;
        break;
    default:
        *pcomp = IFF_TIFF;
        break;
    }

    TIFFCleanup(tif);
    return 0;
}


/*--------------------------------------------------------------*
 *               Open tiff stream from file stream              *
 *--------------------------------------------------------------*/
/*!
 *  fopenTiff()
 *
 *      Input:  stream
 *              modestring ("r", "w", ...)
 *      Return: tiff (data structure, opened for a file descriptor)
 *
 *  Notes:
 *      (1) Why is this here?  Leffler did not provide a function that
 *          takes a stream and gives a TIFF.  He only gave one that
 *          generates a TIFF starting with a file descriptor.  So we
 *          need to make it here, because it is useful to have functions
 *          that take a stream as input.
 *      (2) Requires lseek to rewind to BOF; fseek won't hack it.
 */
static TIFF *
fopenTiff(FILE        *fp,
          const char  *modestring)
{
l_int32  fd;

    PROCNAME("fopenTiff");

    if (!fp)
        return (TIFF *)ERROR_PTR("stream not opened", procName, NULL);
    if (!modestring)
        return (TIFF *)ERROR_PTR("modestring not defined", procName, NULL);

    if ((fd = fileno(fp)) < 0)
        return (TIFF *)ERROR_PTR("invalid file descriptor", procName, NULL);
    lseek(fd, 0, SEEK_SET);

    return TIFFFdOpen(fd, "TIFFstream", modestring);
}

