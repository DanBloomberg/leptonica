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
 *  psio.c
 *                     
 *     This is a PostScript "device driver" for wrapping images
 *     in PostScript.  The images can be rendered by a PostScript
 *     interpreter, such as gs or the embedded interpreter in a
 *     PostScript printer.
 *
 *     Convert specified files to PS
 *          l_int32          convertFilesToPS()    [unix only]
 *          l_int32          sarrayConvertFilesToPS()
 *
 *     Convert any image file to PS for embedding
 *          l_int32          convertToPSEmbed()
 *
 *     For uncompressed images
 *          l_int32          pixWritePSEmbed()
 *          l_int32          pixWriteStreamPS()
 *          char            *pixWriteStringPS()
 *          void             getScaledParametersPS()
 *          l_int32          convertByteToHexAscii()
 *
 *     For jpeg compressed images
 *          l_int32          convertJpegToPSEmbed()
 *          l_int32          convertJpegToPS()
 *          l_int32          convertJpegToPSString()
 *
 *     For tiff g4 compressed images
 *          l_int32          convertTiffG4ToPSEmbed()
 *          l_int32          convertTiffG4ToPS()
 *          l_int32          convertTiffG4ToPSString()
 *
 *     For multipage tiff images
 *          l_int32          convertTiffMultipageToPS()
 *
 *     Write to memory
 *          l_int32          pixWriteMemPS()
 *
 *     Converting resolution
 *          l_int32          getResLetterPage()
 *          l_int32          getResA4Page()
 *
 *     Utility for encoding and decoding data with ascii85
 *          char            *encodeAscii85()
 *          l_int32         *convertChunkToAscii85()
 *          l_uint8         *decodeAscii85()
 *
 *  These PostScript converters are used in three different ways:
 *
 *  (1) For embedding a PS file in a program like TeX.  We must have
 *      a bounding box.  convertToPSEmbed() handles this for
 *      both level 1 and level 2 output, and prog/converttops
 *      wraps this in an executable.  converttops is a generalization
 *      of Thomas Merz's jpeg2ps wrapper, in that it works for
 *      all types (formats, depth, colormap) of input images and
 *      gives PS output in either compressed or uncompressed format,
 *      depending on an input flag.
 *
 *  (2) For composing a set of pages with any number of images
 *      painted on them, in DCT or G4 compressed format depending
 *      on if the image is grayscale/color or binary.  Because we
 *      append each PS string and specify the scaling and placement
 *      explicitly, one must NOT have a bounding box attached to
 *      each separate image.  
 *
 *  (3) For printing a page image or a set of page images, at a
 *      resolution that optimally fills the page.  Here we use
 *      a bounding box and scale the image appropriately.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

/* --------------------------------------------*/
#if  USE_PSIO   /* defined in environ.h */
 /* --------------------------------------------*/

static const char *TEMP_G4TIFF_FILE = "/usr/tmp/junk_temp_g4tiff.tif";
static const char *TEMP_JPEG_FILE   = "/usr/tmp/junk_temp_jpeg.jpg";

    /* MS VC++ can't handle array initialization with static consts ! */
#define L_BUF_SIZE      512

static const l_int32  DEFAULT_PRINTER_RES     = 300; /* default printing ppi */
static const l_int32  MIN_RES                 = 5;
static const l_int32  MAX_RES                 = 3000;
static const l_int32  MAX_85_LINE_COUNT       = 64;

    /* For computing resolution that fills page to desired amount */
static const l_int32  LETTER_WIDTH            = 612;   /* points */
static const l_int32  LETTER_HEIGHT           = 792;   /* points */
static const l_int32  A4_WIDTH                = 595;   /* points */
static const l_int32  A4_HEIGHT               = 842;   /* points */
static const l_float32  DEFAULT_FILL_FRACTION = 0.95;

static const l_uint32  power85[5] = {1,
                                     85,
                                     85 * 85,
                                     85 * 85 * 85,
                                     85 * 85 * 85 * 85};

#ifndef  NO_CONSOLE_IO
#define  DEBUG_JPEG     0
#define  DEBUG_G4       0
#endif  /* ~NO_CONSOLE_IO */

    /* This should be false for documents that are composited from
     * sequences of painted images, where more than one image can
     * be placed in an arbitrary location on any page.  
     * However, for images that are composited, we use special *Embed()
     * functions for writing PostScript with bounding boxes, so they
     * can be embedded in TeX files, e.g. */
#define  PRINT_BOUNDING_BOX      0


/*-------------------------------------------------------------*
 *                Convert files in a directory to PS           *
 *-------------------------------------------------------------*/
/*
 *  convertFilesToPS()
 *
 *      Input:  dirin (input directory)
 *              substr (<optional> substring filter on filenames; can be NULL)
 *              res (typ. 300 or 600 ppi)
 *              fileout (output ps file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This generates a PS file for all files in a specified directory
 *          that contain the substr pattern to be matched.
 *      (2) Colormaps are removed.
 *      (3) All images are written with level 2 compression.
 *          If the image is 1 bpp, use G4.  Otherwise, use DCT.
 *          If the image is not 1 bpp and not jpeg compressed,
 *          it is jpeg compressed with quality = 75, which will
 *          in general cause some degradation in the image.
 *      (4) The resolution is always confusing.  It is interpreted
 *          as the resolution of the output display device:  "If the
 *          input image were digitized at 300 ppi, what would it
 *          look like when displayed at res ppi."  So, for example,
 *          if res = 100 ppi, then the display pixels are 3x larger
 *          than the 300 ppi pixels, and the image will be rendered
 *          3x larger.
 *      (5) The size of the PostScript file is independent of the resolution,
 *          because the entire file is encoded.  The res parameter just
 *          tells the PS decomposer how to render the page.  Therefore,
 *          for minimum file size without loss of visual information,
 *          if the output res is less than 300, you should downscale
 *          the image to the output resolution before wrapping in PS.
 *      (6) The "canvas" on which the image is rendered, at the given
 *          output resolution, is a standard page size (8.5 x 11 in).
 *      (7) If the image is jpeg or tiffg4, we use the existing
 *          compressed string; otherwise it is necessary to decompress
 *          it, remove any existing colormap, and write it out in
 *          a temp file in one of these two formats.
 *      (8) This is unix only; it does not work on Windows.  However,
 *          see sarrayConvertFilesToPS(), which does the real work
 *          and compiles on all platforms.
 */
l_int32
convertFilesToPS(const char  *dirin,
                 const char  *substr,
                 l_int32      res,
                 const char  *fileout)
{
SARRAY  *sa;

    PROCNAME("convertFilesToPS");

    if (!dirin)
        return ERROR_INT("dirin not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);
    if (res <= 0) {
        L_INFO("setting res to 300 ppi", procName);
        res = 300;
    }
    if (res < 10 || res > 4000)
        L_WARNING("res is typically in the range 300-600 ppi", procName);

        /* Get all filtered and sorted full pathnames. */
    sa = getSortedPathnamesInDirectory(dirin, substr, 0, 0);

        /* Generate the PS file. */
    sarrayConvertFilesToPS(sa, res, fileout);
    sarrayDestroy(&sa);
    return 0;
}


/*
 *  sarrayConvertFilesToPS()
 *
 *      Input:  sarray (of full path names)
 *              res (typ. 300 or 600 ppi)
 *              fileout (output ps file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See convertFilesToPS()
 */
l_int32
sarrayConvertFilesToPS(SARRAY      *sa,
                       l_int32      res,
                       const char  *fileout)
{
char    *fname;
l_int32  i, d, nfiles, index, firstfile, format, retval;
FILE    *fp;
PIX     *pix, *pixt;

    PROCNAME("sarrayConvertFilesToPS");

    if (!sa)
        return ERROR_INT("sa not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);
    if (res <= 0) {
        L_INFO("setting res to 300 ppi", procName);
        res = 300;
    }
    if (res < 10 || res > 4000)
        L_WARNING("res is typically in the range 300-600 ppi", procName);

    nfiles = sarrayGetCount(sa);
    firstfile = TRUE;
    for (i = 0, index = 0; i < nfiles; i++) {
        fname = sarrayGetString(sa, i, 0);
        if ((fp = fopen(fname, "r")) == NULL)
            continue;
        format = findFileFormat(fp);
        fclose(fp);

            /* Convert to tiffg4 or jpeg if necessary */
        if (format != IFF_JFIF_JPEG && format != IFF_TIFF_G4) {
            if ((pix = pixRead(fname)) == NULL)
                continue;
            d = pixGetDepth(pix);
            if (d == 1) {
                pixWrite(TEMP_G4TIFF_FILE, pix, IFF_TIFF_G4);
                fname = stringNew(TEMP_G4TIFF_FILE);
                format = IFF_TIFF_G4;
            }
            else {
                pixt = pixRemoveColormap(pix, REMOVE_CMAP_BASED_ON_SRC);
                pixWrite(TEMP_JPEG_FILE, pixt, IFF_JFIF_JPEG);
                pixDestroy(&pixt);
                fname = stringNew(TEMP_JPEG_FILE);
                format = IFF_JFIF_JPEG;
            }
            pixDestroy(&pix);
        }
        else
            fname = stringNew(fname);

            /* Write it out */
        if (format == IFF_JFIF_JPEG) {
            if (firstfile) {
                retval = convertJpegToPS(fname, fileout, "w", 0, 0,
                                         res, 1.0, index + 1, TRUE);
                if (retval == 0) {
                    firstfile = FALSE;
                    index++;
                }
            }
            else {
                retval = convertJpegToPS(fname, fileout, "a", 0, 0,
                                         res, 1.0, index + 1, TRUE);
                if (retval == 0)
                    index++;
            }
        }
        else {  /* format == IFF_TIFF_G4) */
            if (firstfile) {
                retval = convertTiffG4ToPS(fname, fileout, "w", 0, 0,
                                           res, 1.0, index + 1, FALSE, TRUE);
                if (retval == 0) {
                    firstfile = FALSE;
                    index++;
                }
            }
            else {
                retval = convertTiffG4ToPS(fname, fileout, "a", 0, 0,
                                           res, 1.0, index + 1, FALSE, TRUE);
                if (retval == 0)
                    index++;
            }
        }
        FREE(fname);
    }
    
    return 0;
}


/*-------------------------------------------------------------*
 *            Convert any image file to PS for embedding       *
 *-------------------------------------------------------------*/
/*
 *  convertToPSEmbed()
 *
 *      Input:  filein (input image file)
 *              fileout (output ps file)
 *              level (1 - uncompressed,  2 - compressed)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is a wrapper function that generates a PS file with
 *          a bounding box, from any input image file.
 *      (2) Colormaps are removed.
 *      (3) If the image is not 1 bpp and is not jpeg compressed,
 *          and it is to be written as PS with DCT compression
 *          (level = 2), it will first be written to file as jpeg with
 *          quality = 75.  This will cause some degradation in the image.
 *      (4) The bounding box is required when a program such as TeX
 *          (through epsf) places and rescales the image.
 *      (5) The bounding box is sized for fitting the image to an
 *          8.5 x 11.0 inch page.
 */
l_int32
convertToPSEmbed(const char  *filein,
                 const char  *fileout,
                 l_int32      level)
{
l_int32  d, format;
FILE    *fp;
PIX     *pix, *pixs;

    PROCNAME("convertToPSEmbed");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);

    if (level == 1) {
        pixWritePSEmbed(filein, fileout);
        return 0;
    }

        /* We must write out level 2 PS */
    if ((fp = fopen(filein, "r")) == NULL)
        return ERROR_INT("filein not found", procName, 1);
    format = findFileFormat(fp);
    fclose(fp);

        /* Write out directly if in jpeg or tiff g4 formats */
    if (format == IFF_JFIF_JPEG) {
        convertJpegToPSEmbed(filein, fileout);
        return 0;
    }
    else if (format == IFF_TIFF_G4) {
        convertTiffG4ToPSEmbed(filein, fileout);
	return 0;
    }

        /* Must convert to jpeg or tiff g4 */
    if ((pixs = pixRead(filein)) == NULL)
        return ERROR_INT("image not read from file", procName, 1);
    d = pixGetDepth(pixs);
    if (d == 16)
        pix = pixConvert16To8(pixs, 1);
    else
        pix = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    d = pixGetDepth(pix);
    if (d == 1) {
        pixWrite(TEMP_G4TIFF_FILE, pix, IFF_TIFF_G4);
        convertTiffG4ToPSEmbed(TEMP_G4TIFF_FILE, fileout);
    }
    else {
        pixWrite(TEMP_JPEG_FILE, pix, IFF_JFIF_JPEG);
        convertJpegToPSEmbed(TEMP_JPEG_FILE, fileout);
    }

    pixDestroy(&pix);
    pixDestroy(&pixs);
    return 0;
}


/*-------------------------------------------------------------*
 *                  For uncompressed images                    *
 *-------------------------------------------------------------*/
/*!
 *  pixWritePSEmbed()
 *
 *      Input:  filein (input file)
 *              fileout (output ps file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is a simple wrapper function that generates an
 *          uncompressed PS file, with a bounding box.
 *      (2) The bounding box is required when a program such as TeX
 *          (through epsf) places and rescales the image.
 *      (3) The bounding box is sized for fitting the image to an
 *          8.5 x 11.0 inch page.
 */
l_int32
pixWritePSEmbed(const char  *filein,
                const char  *fileout)
{
l_int32    w, h;
l_float32  scale;
FILE      *fp;
PIX       *pix;

    PROCNAME("pixWritePSEmbed");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);

    if ((pix = pixRead(filein)) == NULL)
        return ERROR_INT("image not read from file", procName, 1);
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (w * 11.0 > h * 8.5)
        scale = 8.5 * 300. / (l_float32)w;
    else
        scale = 11.0 * 300. / (l_float32)h;

    if ((fp = fopen(fileout, "w")) == NULL)
        return ERROR_INT("file not opened for write", procName, 1);
    pixWriteStreamPS(fp, pix, NULL, 0, scale);
    fclose(fp);

    pixDestroy(&pix);
    return 0;
}


/*!
 *  pixWriteStreamPS()
 *
 *      Input:  stream
 *              pix
 *              box  (<optional>)
 *              res  (can use 0 for default of 300 ppi)
 *              scale (to prevent scaling, use either 1.0 or 0.0)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This writes image in PS format, optionally scaled,
 *          adjusted for the printer resolution, and with
 *          a bounding box.
 *      (2) For details on use of parameters, see pixWriteStringPS().
 */
l_int32
pixWriteStreamPS(FILE      *fp,
                 PIX       *pix,
                 BOX       *box,
                 l_int32    res,
                 l_float32  scale)
{
char    *pstring;
l_int32  length;
PIX     *pixc;

    PROCNAME("pixWriteStreamPS");

    if (!fp)
        return (l_int32)ERROR_INT("stream not open", procName, 1);
    if (!pix)
        return (l_int32)ERROR_INT("pix not defined", procName, 1);

    if ((pixc = pixConvertForPSWrap(pix)) == NULL)
        return (l_int32)ERROR_INT("pixc not made", procName, 1);

    pstring = pixWriteStringPS(pixc, box, res, scale);
    length = strlen(pstring);
    fwrite(pstring, 1, length, fp);
    FREE(pstring);
    pixDestroy(&pixc);

    return 0;
}


/*!
 *  pixWriteStringPS()
 *
 *      Input:  pix:  1, 2, 4, 8 bpp, with or without cmap; 32 bpp (RGB)
 *              box:  (a) If box == null, image is placed, optionally scaled,
 *                        in a standard b.b. at the center of the page.
 *                        This is to be used when another program like
 *                        TeX (through epsf) places the image.
 *                    (b) If box != null, image is placed without a
 *                        b.b. at the specified page location and with
 *                        (optional) scaling.  This is to be used when
 *                        you want to specify exactly where (and optionally
 *                        how big) you want the image to be.
 *                        Note that all coordinates are in PS convention,
 *                        with (0,0) at LL corner of the page:
 *                            (x,y)    location of LL corner of image, in mils.
 *                            (w,h)    scaled size, in mils.  Use 0 to
 *                                     scale with "scale" and "res" input.
 *              res:  resolution, in printer ppi.  Use 0 for default (300 ppi).
 *              scale: scale factor.  If no scaling is desired, use
 *                     either 1.0 or 0.0.   Scaling just resets the resolution
 *                     parameter; the actual scaling is done in the
 *                     interpreter at rendering time.  This is important:
 *                     it allows you to scale the image up without
 *                     increasing the file size.
 *
 *      Return: ps string if OK, or null on error
 *
 *  Notes:
 *      (1) OK, this seems a bit complicated, because there are various
 *          ways to scale and not to scale.  Here's a summary:
 *      (2) If you don't want any scaling at all:
 *           * if you are using a box:
 *               set w = 0, h = 0, and use scale = 1.0; it will print
 *               each pixel unscaled at printer resolution
 *           * if you are not using a box:
 *               set scale = 1.0; it will print at printer resolution
 *      (3) If you want the image to be a certain size in inches:
 *           * you must use a box and set the box (w,h) in mils
 *      (4) If you want the image to be scaled by a scale factor != 1.0:
 *           * if you are using a box:
 *               set w = 0, h = 0, and use the desired scale factor;
 *               the higher the printer resolution, the smaller the
 *               image will actually appear.
 *           * if you are not using a box:
 *               set the desired scale factor; the higher the printer
 *               resolution, the smaller the image will actually appear.
 *      (5) Another complication is the proliferation of distance units:
 *           * The interface distances are in milli-inches.
 *           * Three different units are used internally:
 *              - pixels  (units of 1/res inch)
 *              - printer pts (units of 1/72 inch)
 *              - inches
 *           * Here is a quiz on volume units from a reviewer:
 *             How many UK milli-cups in a US kilo-teaspoon?
 *               (Hint: 1.0 US cup = 0.75 UK cup + 0.2 US gill;
 *                      1.0 US gill = 24.0 US teaspoons)
 */
char *
pixWriteStringPS(PIX       *pixs,
                 BOX       *box,
                 l_int32    res,
                 l_float32  scale)
{
char       nib1, nib2;
char       bigbuf[L_BUF_SIZE];
char      *hexdata, *pstring;
l_uint8    byteval;
l_int32    i, j, k, d, wpix, hpix;
l_float32  wpt, hpt, xpt, ypt;
l_int32    wpl, psbpl, hexbytes, boxflag, sampledepth;
l_uint32  *line, *data;
PIX       *pix;
SARRAY    *sa;

    PROCNAME("pixWriteStringPS");

    if (!pixs)
        return (char *)ERROR_PTR("pix not defined", procName, NULL);

    d = pixGetDepth(pixs);
    if (d == 2)
        pix = pixConvert2To8(pixs, 0, 85, 170, 255, 0);
    else if (d == 4)
        pix = pixConvert4To8(pixs, 0);
    else if (d == 16)
        pix = pixConvert16To8(pixs, 1);
    else
        pix = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);

        /* Get the factors by which PS scales and translates, in pts */
    pixGetDimensions(pix, &wpix, &hpix, &d);
    if (!box)
        boxflag = 0;  /* no scaling; b.b. at center */
    else
        boxflag = 1;  /* no b.b., specify placement and optional scaling */
    getScaledParametersPS(box, wpix, hpix, res, scale, &xpt, &ypt, &wpt, &hpt);

    if (d == 1)
        sampledepth = 1;
    else  /* d == 8 || d == 32 */
        sampledepth = 8;

        /* Convert image data to hex string */
    wpl = pixGetWpl(pix);
    if (d == 1 || d == 8)
        psbpl = (wpix * d + 7) / 8;   /* packed to byte boundary */
    else /* d == 32 */
        psbpl = 3 * wpix;   /* packed to byte boundary */
    data = pixGetData(pix);
    hexbytes = 2 * psbpl * hpix;  /* size of ps hex array */
    if ((hexdata = (char *)CALLOC(hexbytes + 1, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("hexdata not made", procName, NULL);
    if (d == 1 || d == 8) {
        for (i = 0, k = 0; i < hpix; i++) {
            line = data + i * wpl;
            for (j = 0; j < psbpl; j++) {
                byteval = GET_DATA_BYTE(line, j);
                convertByteToHexAscii(byteval, &nib1, &nib2);
                hexdata[k++] = nib1;
                hexdata[k++] = nib2;
            }
        }
    }
    else  {  /* d == 32; hexdata bytes packed RGBRGB..., 2 per sample */
        for (i = 0, k = 0; i < hpix; i++) {
            line = data + i * wpl;
            for (j = 0; j < wpix; j++) {
                byteval = GET_DATA_BYTE(line + j, 0);  /* red */
                convertByteToHexAscii(byteval, &nib1, &nib2);
                hexdata[k++] = nib1;
                hexdata[k++] = nib2;
                byteval = GET_DATA_BYTE(line + j, 1);  /* green */
                convertByteToHexAscii(byteval, &nib1, &nib2);
                hexdata[k++] = nib1;
                hexdata[k++] = nib2;
                byteval = GET_DATA_BYTE(line + j, 2);  /* blue */
                convertByteToHexAscii(byteval, &nib1, &nib2);
                hexdata[k++] = nib1;
                hexdata[k++] = nib2;
            }
        }
    }
    hexdata[k] = '\0';

    if ((sa = sarrayCreate(0)) == NULL)
        return (char *)ERROR_PTR("sa not made", procName, NULL);

    sarrayAddString(sa, "%!Adobe-PS", 1);
    if (boxflag == 0) {
        sprintf(bigbuf,
            "%%%%BoundingBox: %7.2f %7.2f %7.2f %7.2f",
            xpt, ypt, xpt + wpt, ypt + hpt);
        sarrayAddString(sa, bigbuf, 1);
    }
    else    /* boxflag == 1 */
        sarrayAddString(sa, "gsave", 1);

    if (d == 1)
        sarrayAddString(sa, "{1 exch sub} settransfer    %invert binary", 1);

    sprintf(bigbuf, "/bpl %d string def         %%bpl as a string", psbpl);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf,
        "%7.2f %7.2f translate         %%set image origin in pts", xpt, ypt);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf,
        "%7.2f %7.2f scale             %%set image size in pts", wpt, hpt);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf,
        "%d %d %d                 %%image dimensions in pixels",
            wpix, hpix, sampledepth);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf,
        "[%d %d %d %d %d %d]     %%mapping matrix: [wpix 0 0 -hpix 0 hpix]",
               wpix, 0, 0, -hpix, 0, hpix);
    sarrayAddString(sa, bigbuf, 1);

    if (boxflag == 0) {
        if (d == 1 || d == 8)
            sarrayAddString(sa, "{currentfile bpl readhexstring pop} image", 1);
        else  /* d == 32 */
            sarrayAddString(sa,
                "{currentfile bpl readhexstring pop} false 3 colorimage", 1);
    }
    else {  /* boxflag == 1 */
        if (d == 1 || d == 8)
            sarrayAddString(sa,
                "{currentfile bpl readhexstring pop} bind image", 1);
        else  /* d == 32 */
            sarrayAddString(sa,
                "{currentfile bpl readhexstring pop} bind false 3 colorimage",
                 1);
    }

    sarrayAddString(sa, hexdata, 0);

    if (boxflag == 0)
        sarrayAddString(sa, "\nshowpage", 1);
    else  /* boxflag == 1 */
        sarrayAddString(sa, "\ngrestore", 1);

    if ((pstring = sarrayToString(sa, 1)) == NULL)
        return (char *)ERROR_PTR("pstring not made", procName, NULL);

    sarrayDestroy(&sa);
    pixDestroy(&pix);
    return pstring;
}


/*!
 *  getScaledParametersPS()
 *
 *      Input:  box (<optional> location of image in mils; with
 *                   (x,y) being the LL corner)
 *              wpix (pix width in pixels)
 *              hpix (pix height in pixels)
 *              res (of printer; use 0 for default)
 *              scale (use 1.0 or 0.0 for no scaling) 
 *              &xpt (location of llx in pts)
 *              &ypt (location of lly in pts)
 *              &wpt (image width in pts)
 *              &hpt (image height in pts)
 *      Return: void (no arg checking)
 *
 *  Notes:
 *      (1) The image is always scaled, depending on res and scale.
 *      (2) If no box, the image is centered on the page.
 *      (3) If there is a box, the image is placed within it.
 */
void
getScaledParametersPS(BOX        *box,
                      l_int32     wpix,
                      l_int32     hpix,
                      l_int32     res,
                      l_float32   scale,
                      l_float32  *pxpt,
                      l_float32  *pypt,
                      l_float32  *pwpt,
                      l_float32  *phpt)
{
l_float32  winch, hinch, xinch, yinch, fres;

    PROCNAME("getScaledParametersPS");

    if (res == 0)
        res = DEFAULT_PRINTER_RES;
    fres = (l_float32)res;

        /* Allow the PS interpreter to scale the resolution */
    if (scale == 0.0)
        scale = 1.0;
    if (scale != 1.0) {
        fres = (l_float32)res / scale;
        res = (l_int32)fres;
    }

        /* Limit valid resolution interval */
    if (res < MIN_RES || res > MAX_RES) {
        L_WARNING_INT("res %d out of bounds; using default res; no scaling",
                      procName, res);
        res = DEFAULT_PRINTER_RES;
        fres = (l_float32)res;
    }

    if (!box) {  /* center on page */
        winch = (l_float32)wpix / fres;
        hinch = (l_float32)hpix / fres;
        xinch = (8.5 - winch) / 2.;
        yinch = (11.0 - hinch) / 2.;
    }
    else {
        if (box->w == 0)
            winch = (l_float32)wpix / fres;
        else
            winch = (l_float32)box->w / 1000.;
        if (box->h == 0)
            hinch = (l_float32)hpix / fres;
        else
            hinch = (l_float32)box->h / 1000.;
        xinch = (l_float32)box->x / 1000.;
        yinch = (l_float32)box->y / 1000.;
    }

    if (xinch < 0)
        L_WARNING("left edge < 0.0 inch", procName);
    if (xinch + winch > 8.5)
        L_WARNING("right edge > 8.5 inch", procName);
    if (yinch < 0.0)
        L_WARNING("bottom edge < 0.0 inch", procName);
    if (yinch + hinch > 11.0)
        L_WARNING("top edge > 11.0 inch", procName);

    *pwpt = 72. * winch;
    *phpt = 72. * hinch;
    *pxpt = 72. * xinch;
    *pypt = 72. * yinch;
    return;
}
    

/*!
 *  convertByteToHexAscii()
 *
 *      Input:  byteval  (input byte)
 *              &nib1, &nib2  (<return> two hex ascii characters)
 *      Return: void
 */
void
convertByteToHexAscii(l_uint8  byteval,
                      char    *pnib1,
                      char    *pnib2)
{
l_uint8  nib;

    nib = byteval >> 4;
    if (nib < 10)
        *pnib1 = '0' + nib;
    else
        *pnib1 = 'a' + (nib - 10);
    nib = byteval & 0xf;
    if (nib < 10)
        *pnib2 = '0' + nib;
    else
        *pnib2 = 'a' + (nib - 10);

    return;
}


/*-------------------------------------------------------------*
 *                  For jpeg compressed images                 *
 *-------------------------------------------------------------*/
/*!
 *  convertJpegToPSEmbed()
 *
 *      Input:  filein (input jpeg file)
 *              fileout (output ps file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This function takes a jpeg file as input and generates a DCT
 *          compressed, ascii85 encoded PS file, with a bounding box.
 *      (2) The bounding box is required when a program such as TeX
 *          (through epsf) places and rescales the image.
 *      (3) The bounding box is sized for fitting the image to an
 *          8.5 x 11.0 inch page.
 */
l_int32
convertJpegToPSEmbed(const char  *filein,
                     const char  *fileout)
{
char      *pstring, *outstr;
char      *data85;  /* ascii85 encoded file */
char       bigbuf[512];
l_uint8   *bindata;  /* binary encoded jpeg data (entire file) */
l_int32    bps, w, h, spp;
l_int32    nbinbytes, psbytes, nbytes85, totbytes;
l_float32  xpt, ypt, wpt, hpt;
SARRAY    *sa;

    PROCNAME("convertJpegToPSEmbed");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);

        /* The returned jpeg data in memory is the entire jpeg file,
         * which starts with ffd8 and ends with ffd9 */
    if (extractJpegDataFromFile(filein, &bindata, &nbinbytes,
                                &w, &h, &bps, &spp))
        return ERROR_INT("bindata not extracted from file", procName, 1);

        /* Convert entire jpeg file of encoded DCT data to ascii85 */
    data85 = encodeAscii85(bindata, nbinbytes, &nbytes85);
    FREE(bindata);
    if (!data85)
        return ERROR_INT("data85 not made", procName, 1);

        /* Scale for 20 pt boundary and otherwise full filling
         * in one direction on 8.5 x 11 inch device */
    xpt = 20.0;
    ypt = 20.0;
    if (w * 11.0 > h * 8.5) {
        wpt = 572.0;   /* 612 - 2 * 20 */
        hpt = wpt * (l_float32)h / (l_float32)w;
    }
    else {
        hpt = 752.0;   /* 792 - 2 * 20 */
        wpt = hpt * (l_float32)w / (l_float32)h;
    }

        /*  -------- Generate PostScript output -------- */
    if ((sa = sarrayCreate(50)) == NULL)
        return ERROR_INT("sa not made", procName, 1);

    sarrayAddString(sa, "%!PS-Adobe-3.0", 1);
    sarrayAddString(sa, "%%Creator: leptonica", 1);
    sprintf(bigbuf, "%%%%Title: %s", filein);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf,
        "%%%%BoundingBox: %7.2f %7.2f %7.2f %7.2f",
                   xpt, ypt, xpt + wpt, ypt + hpt);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "%%DocumentData: Clean7Bit", 1);
    sarrayAddString(sa, "%%LanguageLevel: 2", 1);
    sarrayAddString(sa, "%%EndComments", 1);
    sarrayAddString(sa, "%%Page: 1 1", 1);

    sarrayAddString(sa, "save", 1);
    sarrayAddString(sa, "/RawData currentfile /ASCII85Decode filter def", 1);
    sarrayAddString(sa, "/Data RawData << >> /DCTDecode filter def", 1);

    sprintf(bigbuf,
        "%7.2f %7.2f translate         %%set image origin in pts", xpt, ypt);
    sarrayAddString(sa, bigbuf, 1);

    sprintf(bigbuf,
        "%7.2f %7.2f scale             %%set image size in pts", wpt, hpt);
    sarrayAddString(sa, bigbuf, 1);

    if (spp == 1)
        sarrayAddString(sa, "/DeviceGray setcolorspace", 1);
    else if (spp == 3)
        sarrayAddString(sa, "/DeviceRGB setcolorspace", 1);
    else  /*spp == 4 */
        sarrayAddString(sa, "/DeviceCMYK setcolorspace", 1);
    
    sarrayAddString(sa, "{ << /ImageType 1", 1);
    sprintf(bigbuf, "     /Width %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "     /Height %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "     /ImageMatrix [ %d 0 0 %d 0 %d ]", w, -h, h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "     /DataSource Data", 1);
    sprintf(bigbuf, "     /BitsPerComponent %d", bps);
    sarrayAddString(sa, bigbuf, 1);

    if (spp == 1)
        sarrayAddString(sa, "     /Decode [0 1]", 1);
    else if (spp == 3)
        sarrayAddString(sa, "     /Decode [0 1 0 1 0 1]", 1);
    else   /* spp == 4 */
        sarrayAddString(sa, "     /Decode [0 1 0 1 0 1 0 1]", 1);
    
    sarrayAddString(sa, "  >> image", 1);
    sarrayAddString(sa, "  Data closefile", 1);
    sarrayAddString(sa, "  RawData flushfile", 1);
    sarrayAddString(sa, "  showpage", 1);
    sarrayAddString(sa, "  restore", 1);
    sarrayAddString(sa, "} exec", 1);

    if ((pstring = sarrayToString(sa, 1)) == NULL)
        return ERROR_INT("pstring not made", procName, 1);
    sarrayDestroy(&sa);
    psbytes = strlen(pstring);

        /* Add the ascii85 data */
    totbytes = psbytes + nbytes85;
    if ((outstr = (char *)CALLOC(totbytes + 4, sizeof(char))) == NULL)
        return ERROR_INT("outstr not made", procName, 1);
    memcpy(outstr, pstring, psbytes);
    memcpy(outstr + psbytes, data85, nbytes85);
    FREE(pstring);
    FREE(data85);

    if (arrayWrite(fileout, "w", outstr, totbytes))
        return ERROR_INT("ps string not written to file", procName, 1);
    FREE(outstr);
    return 0;
}


/*!
 *  convertJpegToPS()
 *
 *      Input:  filein (input jpeg file)
 *              fileout (output ps file)
 *              operation ("w" for write; "a" for append)
 *              x, y (location of LL corner of image, in pixels, relative
 *                    to the PostScript origin (0,0) at the LL corner
 *                    of the page)
 *              res (resolution of the input image, in ppi; use 0 for default)
 *              scale (scaling by printer; use 0.0 or 1.0 for no scaling)
 *              pageno (page number; must start with 1; you can use 0
 *                  if there is only one page.)
 *              endpage (boolean: TRUE if the last image to be
 *                  added to the page; FALSE otherwise)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is simpler to use than pixWriteStringPS(), and
 *          it outputs in level 2 PS as compressed DCT (overlaid
 *          with ascii85 encoding).
 *      (2) An output file can contain multiple pages, each with
 *          multiple images.  The arguments to convertJpegToPS()
 *          allow you to control placement of jpeg images on multiple
 *          pages within a PostScript file.
 *      (3) For the first image written to a file, use "w", which
 *          opens for write and clears the file.  For all subsequent
 *          images written to that file, use "a".
 *      (4) The (x, y) parameters give the LL corner of the image
 *          relative to the LL corner of the page.  They are in
 *          units of pixels if scale = 1.0.  If you use (e.g.)
 *          scale = 2.0, the image is placed at (2x, 2y) on the page,
 *          and the image dimensions are also doubled.
 *      (5) Display vs printed resolution:
 *           * If your display is 75 ppi and your image was created
 *             at a resolution of 300 ppi, you can get the image
 *             to print at the same size as it appears on your display
 *             by either setting scale = 4.0 or by setting  res = 75.
 *             Both tell the printer to make a 4x enlarged image.
 *           * If your image is generated at 150 ppi and you use scale = 1,
 *             it will be rendered such that 150 pixels correspond
 *             to 72 pts (1 inch on the printer).  This function does
 *             the conversion from pixels (with or without scaling) to
 *             pts, which are the units that the printer uses.
 *           * The printer will choose its own resolution to use
 *             in rendering the image, which will not affect the size
 *             of the rendered image.  That is because the output
 *             PostScript file describes the geometry in terms of pts,
 *             which are defined to be 1/72 inch.  The printer will
 *             only see the size of the image in pts, through the
 *             scale and translate parameters and the affine
 *             transform (the ImageMatrix) of the image.
 *      (6) To render multiple images on the same page, set
 *          endpage = FALSE for each image until you get to the
 *          last, for which you set endpage = TRUE.  This causes the
 *          "showpage" command to be invoked.  Showpage outputs
 *          the entire page and clears the raster buffer for the
 *          next page to be added.  Without a "showpage",
 *          subsequent images from the next page will overlay those
 *          previously put down.
 *      (7) For multiple pages, increment the page number, starting
 *          with page 1.  This allows PostScript (and PDF) to build
 *          a page directory, which viewers use for navigation.
 */
l_int32
convertJpegToPS(const char  *filein,
                const char  *fileout,
                const char  *operation,
                l_int32      x,
                l_int32      y,
                l_int32      res,
                l_float32    scale,
                l_int32      pageno,
                l_int32      endpage)
{
char    *outstr;
l_int32  nbytes;

    PROCNAME("convertJpegToPS");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);
    if (strcmp(operation, "w") && strcmp(operation, "a"))
        return ERROR_INT("operation must be \"w\" or \"a\"", procName, 1);

    if (convertJpegToPSString(filein, &outstr, &nbytes, x, y, res, scale,
                          pageno, endpage))
        return ERROR_INT("ps string not made", procName, 1);

    if (arrayWrite(fileout, operation, outstr, nbytes))
        return ERROR_INT("ps string not written to file", procName, 1);

    FREE(outstr);
    return 0;
}


/*!
 *  convertJpegToPSString()
 *
 *      Generates PS string in jpeg format from jpeg file
 *
 *      Input:  filein (input jpeg file)
 *              &poutstr (<return> PS string)
 *              &nbytes (<return> number of bytes in PS string)
 *              x, y (location of LL corner of image, in pixels, relative
 *                    to the PostScript origin (0,0) at the LL corner
 *                    of the page)
 *              res (resolution of the input image, in ppi; use 0 for default)
 *              scale (scaling by printer; use 0.0 or 1.0 for no scaling)
 *              pageno (page number; must start with 1; you can use 0
 *                  if there is only one page.)
 *              endpage (boolean: TRUE if the last image to be
 *                  added to the page; FALSE otherwise)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned PS character array is binary string, not a
 *          null-terminated ascii C string.  It has null bytes
 *          embedded in it!
 *
 *  Usage:  See convertJpegToPS()
 */
l_int32
convertJpegToPSString(const char  *filein,
                      char       **poutstr,
                      l_int32     *pnbytes,
                      l_int32      x,
                      l_int32      y,
                      l_int32      res,
                      l_float32    scale,
                      l_int32      pageno,
                      l_int32      endpage)
{
char      *pstring, *outstr;
char      *data85;  /* ascii85 encoded file */
char       bigbuf[L_BUF_SIZE];
l_uint8   *bindata;  /* binary encoded jpeg data (entire file) */
l_int32    bps, w, h, spp;
l_int32    nbinbytes, psbytes, nbytes85, totbytes;
l_float32  xpt, ypt, wpt, hpt;
SARRAY    *sa;

    PROCNAME("convertJpegToPSString");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!poutstr)
        return ERROR_INT("&outstr not defined", procName, 1);
    if (!pnbytes)
        return ERROR_INT("&nbytes not defined", procName, 1);
    *poutstr = NULL;

        /* The returned jpeg data in memory is the entire jpeg file,
         * which starts with ffd8 and ends with ffd9 */
    if (extractJpegDataFromFile(filein, &bindata, &nbinbytes,
                                &w, &h, &bps, &spp))
        return ERROR_INT("bindata not extracted from file", procName, 1);

        /* Convert entire jpeg file of encoded DCT data to ascii85 */
    data85 = encodeAscii85(bindata, nbinbytes, &nbytes85);
    FREE(bindata);
    if (!data85)
        return ERROR_INT("data85 not made", procName, 1);

#if  DEBUG_JPEG
    fprintf(stderr, "w = %d, h = %d, bps = %d, spp = %d\n", w, h, bps, spp);
    fprintf(stderr, "nbinbytes = %d, nbytes85 = %d, ratio = %5.3f\n",
           nbinbytes, nbytes85, (l_float32)nbytes85 / (l_float32)nbinbytes);
#endif   /* DEBUG_JPEG */

        /* Get scaled location in pts */
    if (scale == 0.0)
        scale = 1.0;
    if (res == 0)
        res = DEFAULT_PRINTER_RES;
    xpt = scale * x * 72. / res;
    ypt = scale * y * 72. / res;
    wpt = scale * w * 72. / res;
    hpt = scale * h * 72. / res;

    if (pageno == 0)
        pageno = 1;

#if  DEBUG_JPEG
    fprintf(stderr, "xpt = %7.2f, ypt = %7.2f, wpt = %7.2f, hpt = %7.2f\n",
             xpt, ypt, wpt, hpt);
#endif   /* DEBUG_JPEG */

        /*  -------- Generate PostScript output -------- */
    if ((sa = sarrayCreate(50)) == NULL)
        return ERROR_INT("sa not made", procName, 1);

    sarrayAddString(sa, "%!PS-Adobe-3.0", 1);
    sarrayAddString(sa, "%%Creator: leptonica", 1);
    sprintf(bigbuf, "%%%%Title: %s", filein);
    sarrayAddString(sa, bigbuf, 1);

#if  PRINT_BOUNDING_BOX
    sprintf(bigbuf,
        "%%%%BoundingBox: %7.2f %7.2f %7.2f %7.2f",
                   xpt, ypt, xpt + wpt, ypt + hpt);
    sarrayAddString(sa, bigbuf, 1);
#endif  /* PRINT_BOUNDING_BOX */

    sarrayAddString(sa, "%%DocumentData: Clean7Bit", 1);
    sarrayAddString(sa, "%%LanguageLevel: 2", 1);
    sarrayAddString(sa, "%%EndComments", 1);
    sprintf(bigbuf, "%%%%Page: %d %d", pageno, pageno);
    sarrayAddString(sa, bigbuf, 1);

    sarrayAddString(sa, "save", 1);
    sarrayAddString(sa, "/RawData currentfile /ASCII85Decode filter def", 1);
    sarrayAddString(sa, "/Data RawData << >> /DCTDecode filter def", 1);

    sprintf(bigbuf,
        "%7.2f %7.2f translate         %%set image origin in pts", xpt, ypt);
    sarrayAddString(sa, bigbuf, 1);

    sprintf(bigbuf,
        "%7.2f %7.2f scale             %%set image size in pts", wpt, hpt);
    sarrayAddString(sa, bigbuf, 1);

    if (spp == 1)
        sarrayAddString(sa, "/DeviceGray setcolorspace", 1);
    else if (spp == 3)
        sarrayAddString(sa, "/DeviceRGB setcolorspace", 1);
    else  /*spp == 4 */
        sarrayAddString(sa, "/DeviceCMYK setcolorspace", 1);
    
    sarrayAddString(sa, "{ << /ImageType 1", 1);
    sprintf(bigbuf, "     /Width %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "     /Height %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "     /ImageMatrix [ %d 0 0 %d 0 %d ]", w, -h, h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "     /DataSource Data", 1);
    sprintf(bigbuf, "     /BitsPerComponent %d", bps);
    sarrayAddString(sa, bigbuf, 1);

    if (spp == 1)
        sarrayAddString(sa, "     /Decode [0 1]", 1);
    else if (spp == 3)
        sarrayAddString(sa, "     /Decode [0 1 0 1 0 1]", 1);
    else   /* spp == 4 */
        sarrayAddString(sa, "     /Decode [0 1 0 1 0 1 0 1]", 1);
    
    sarrayAddString(sa, "  >> image", 1);
    sarrayAddString(sa, "  Data closefile", 1);
    sarrayAddString(sa, "  RawData flushfile", 1);
    if (endpage == TRUE)
        sarrayAddString(sa, "  showpage", 1);
    sarrayAddString(sa, "  restore", 1);
    sarrayAddString(sa, "} exec", 1);

    if ((pstring = sarrayToString(sa, 1)) == NULL)
        return ERROR_INT("pstring not made", procName, 1);
    psbytes = strlen(pstring);

        /* Add the ascii85 data */
    totbytes = psbytes + nbytes85;
    *pnbytes = totbytes;
    if ((outstr = (char *)CALLOC(totbytes + 4, sizeof(char))) == NULL)
        return ERROR_INT("outstr not made", procName, 1);
    *poutstr = outstr;
    memcpy(outstr, pstring, psbytes);
    memcpy(outstr + psbytes, data85, nbytes85);

    sarrayDestroy(&sa);
    FREE(data85);
    FREE(pstring);
    return 0;
}


/*-------------------------------------------------------------*
 *                  For tiff g4 compressed images              *
 *-------------------------------------------------------------*/
/*!
 *  convertTiffG4ToPSEmbed()
 *
 *      Input:  filein (input jpeg file)
 *              fileout (output ps file)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This function takes a g4 compressed tif file as input and
 *          generates a g4 compressed, ascii85 encoded PS file, with
 *          a bounding box.
 *      (2) The bounding box is required when a program such as TeX
 *          (through epsf) places and rescales the image.
 *      (3) The bounding box is sized for fitting the image to an
 *          8.5 x 11.0 inch page.
 *      (4) We paint this through a mask, over whatever is below.
 */
l_int32
convertTiffG4ToPSEmbed(const char  *filein,
                       const char  *fileout)
{
char      *pstring, *pstring2, *outstr;
char      *data85;  /* ascii85 encoded ccitt g4 data */
char       bigbuf[512];
l_uint8   *bindata;  /* binary encoded ccitt g4 data */
l_int32    minisblack;   /* TRUE or FALSE */
l_int32    w, h;
l_int32    nbinbytes, nbytes85, psbytes, psbytes2, totbytes;
l_float32  xpt, ypt, wpt, hpt;
SARRAY    *sa, *sa2;

    PROCNAME("convertTiffG4ToPSEmbed");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);

        /* The returned ccitt g4 data in memory is the block of
         * bytes in the tiff file, starting after 8 bytes and
         * ending before the directory. */ 
    if (extractTiffG4DataFromFile(filein, &bindata, &nbinbytes,
                                  &w, &h, &minisblack))
        return ERROR_INT("bindata not extracted from file", procName, 1);

        /* Convert the ccittg4 encoded data to ascii85 */
    data85 = encodeAscii85(bindata, nbinbytes, &nbytes85);
    FREE(bindata);
    if (!data85)
        return ERROR_INT("data85 not made", procName, 1);

        /* Scale for 20 pt boundary and otherwise full filling
         * in one direction on 8.5 x 11 inch device */
    xpt = 20.0;
    ypt = 20.0;
    if (w * 11.0 > h * 8.5) {
        wpt = 572.0;   /* 612 - 2 * 20 */
        hpt = wpt * (l_float32)h / (l_float32)w;
    }
    else {
        hpt = 752.0;   /* 792 - 2 * 20 */
        wpt = hpt * (l_float32)w / (l_float32)h;
    }

        /*  -------- Generate PostScript output -------- */
    if ((sa = sarrayCreate(50)) == NULL)
        return ERROR_INT("sa not made", procName, 1);

    sarrayAddString(sa, "%!PS-Adobe-3.0", 1);
    sarrayAddString(sa, "%%Creator: leptonica", 1);
    sprintf(bigbuf, "%%%%Title: %s", filein);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "%%DocumentData: Clean7Bit", 1);
    sprintf(bigbuf,
        "%%%%BoundingBox: %7.2f %7.2f %7.2f %7.2f",
                xpt, ypt, xpt + wpt, ypt + hpt);
    sarrayAddString(sa, bigbuf, 1);

    sarrayAddString(sa, "%%LanguageLevel: 2", 1);
    sarrayAddString(sa, "%%EndComments", 1);
    sarrayAddString(sa, "%%Page: 1 1", 1);

    sarrayAddString(sa, "save", 1);
    sarrayAddString(sa, "100 dict begin", 1);

    sprintf(bigbuf,
        "%7.2f %7.2f translate         %%set image origin in pts", xpt, ypt);
    sarrayAddString(sa, bigbuf, 1);

    sprintf(bigbuf,
        "%7.2f %7.2f scale             %%set image size in pts", wpt, hpt);
    sarrayAddString(sa, bigbuf, 1);

    sarrayAddString(sa, "/DeviceGray setcolorspace", 1);

    sarrayAddString(sa, "{", 1);
    sarrayAddString(sa, "  /RawData currentfile /ASCII85Decode filter def", 1);
    sarrayAddString(sa, "  << ", 1);
    sarrayAddString(sa, "    /ImageType 1", 1);
    sprintf(bigbuf, "    /Width %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "    /Height %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "    /ImageMatrix [ %d 0 0 %d 0 %d ]", w, -h, h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "    /BitsPerComponent 1", 1);
    sarrayAddString(sa, "    /Interpolate true", 1);
    if (minisblack)
        sarrayAddString(sa, "    /Decode [1 0]", 1);
    else  /* miniswhite; typical for 1 bpp */
        sarrayAddString(sa, "    /Decode [0 1]", 1);
    sarrayAddString(sa, "    /DataSource RawData", 1);
    sarrayAddString(sa, "        <<", 1);
    sarrayAddString(sa, "          /K -1", 1);
    sprintf(bigbuf, "          /Columns %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "          /Rows %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "        >> /CCITTFaxDecode filter", 1);
    sarrayAddString(sa, "  >> imagemask", 1);
    sarrayAddString(sa, "  RawData flushfile", 1);
    sarrayAddString(sa, "  showpage", 1);
    sarrayAddString(sa, "}", 1);

    sarrayAddString(sa, "%%BeginData:", 1);
    sarrayAddString(sa, "exec", 1);

    if ((pstring = sarrayToString(sa, 1)) == NULL)
        return ERROR_INT("pstring not made", procName, 1);
    psbytes = strlen(pstring);
    sarrayDestroy(&sa);

        /* Concat the trailing data */
    sa2 = sarrayCreate(10);
    sarrayAddString(sa2, "%%EndData", 1);
    sarrayAddString(sa2, "end", 1);
    sarrayAddString(sa2, "restore", 1);
    if ((pstring2 = sarrayToString(sa2, 1)) == NULL)
        return ERROR_INT("pstring2 not made", procName, 1);
    psbytes2 = strlen(pstring2);
    sarrayDestroy(&sa2);

        /* Add the ascii85 data */
    totbytes = psbytes + psbytes2 + nbytes85;
    if ((outstr = (char *)CALLOC(totbytes + 4, sizeof(char))) == NULL)
        return ERROR_INT("outstr not made", procName, 1);
    memcpy(outstr, pstring, psbytes);
    memcpy(outstr + psbytes, data85, nbytes85);
    memcpy(outstr + psbytes + nbytes85, pstring2, psbytes2);
    FREE(data85);
    FREE(pstring);
    FREE(pstring2);

    if (arrayWrite(fileout, "w", outstr, totbytes))
        return ERROR_INT("ps string not written to file", procName, 1);
    FREE(outstr);
    return 0;
}


/*!
 *  convertTiffG4ToPS()
 *
 *      Input:  filein (input tiff g4 file)
 *              fileout (output ps file)
 *              operation ("w" for write; "a" for append)
 *              x, y (location of LL corner of image, in pixels, relative
 *                    to the PostScript origin (0,0) at the LL corner
 *                    of the page)
 *              res (resolution of the input image, in ppi; typ. values
 *                   are 300 and 600; use 0 for automatic determination
 *                   based on image size)
 *              scale (scaling by printer; use 0.0 or 1.0 for no scaling)
 *              pageno (page number; must start with 1; you can use 0
 *                  if there is only one page.)
 *              mask (boolean: use TRUE if just painting through fg;
 *                    FALSE if painting both fg and bg.
 *              endpage (boolean: use TRUE if the last image to be
 *                  added to the page; FALSE otherwise)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See the usage comments in convertJpegToPS(), some of
 *          which are repeated here.
 *      (2) This is a wrapper for tiff g4.  The PostScript that
 *          is generated is expanded by about 5/4 (due to the
 *          ascii85 encoding.  If you convert to pdf (ps2pdf), the
 *          ascii85 decoder is automatically invoked, so that the
 *          pdf wrapped g4 file is essentially the same size as
 *          the original g4 file.  It's useful to have the PS
 *          file ascii85 encoded, because many printers will not
 *          print binary PS files.
 *      (3) For the first image written to a file, use "w", which
 *          opens for write and clears the file.  For all subsequent
 *          images written to that file, use "a".
 *      (4) To render multiple images on the same page, set
 *          endpage = FALSE for each image until you get to the
 *          last, for which you set endpage = TRUE.  This causes the
 *          "showpage" command to be invoked.  Showpage outputs
 *          the entire page and clears the raster buffer for the
 *          next page to be added.  Without a "showpage",
 *          subsequent images from the next page will overlay those
 *          previously put down.
 *      (5) For multiple images to the same page, where you are writing
 *          both jpeg and tiff-g4, you have two options:
 *           (a) write the g4 first, as either image (mask == false)
 *               or imagemask (mask == true), and then write the
 *               jpeg over it.
 *           (b) write the jpeg first and as the last item, write
 *               the g4 as an imagemask (mask == true), to paint
 *               through the foreground only.  
 *          We have this flexibility with the tiff-g4 because it is 1 bpp.
 *      (6) For multiple pages, increment the page number, starting
 *          with page 1.  This allows PostScript (and PDF) to build
 *          a page directory, which viewers use for navigation.
 */
l_int32
convertTiffG4ToPS(const char  *filein,
                  const char  *fileout,
                  const char  *operation,
                  l_int32      x,
                  l_int32      y,
                  l_int32      res,
                  l_float32    scale,
                  l_int32      pageno,
                  l_int32      mask,
                  l_int32      endpage)
{
char    *outstr;
l_int32  nbytes;

    PROCNAME("convertTiffG4ToPS");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);
    if (strcmp(operation, "w") && strcmp(operation, "a"))
        return ERROR_INT("operation must be \"w\" or \"a\"", procName, 1);

    if (convertTiffG4ToPSString(filein, &outstr, &nbytes, x, y, res, scale,
                          pageno, mask, endpage))
        return ERROR_INT("ps string not made", procName, 1);

    if (arrayWrite(fileout, operation, outstr, nbytes))
        return ERROR_INT("ps string not written to file", procName, 1);

    FREE(outstr);
    return 0;
}


/*!
 *  convertTiffG4ToPSString()
 *
 *      Input:  filein (input tiff g4 file)
 *              &poutstr (<return> PS string)
 *              &nbytes (<return> number of bytes in PS string)
 *              x, y (location of LL corner of image, in pixels, relative
 *                    to the PostScript origin (0,0) at the LL corner
 *                    of the page)
 *              res (resolution of the input image, in ppi; typ. values
 *                   are 300 and 600; use 0 for automatic determination
 *                   based on image size)
 *              scale (scaling by printer; use 0.0 or 1.0 for no scaling)
 *              pageno (page number; must start with 1; you can use 0
 *                  if there is only one page.)
 *              mask (boolean: use TRUE if just painting through fg;
 *                    FALSE if painting both fg and bg.
 *              endpage (boolean: use TRUE if the last image to be
 *                  added to the page; FALSE otherwise)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Generates PS string in G4 compressed tiff format from G4 tiff file.
 *      (2) The returned PS character array is binary string, not a
 *          null-terminated C string.  It has null bytes embedded in it!
 *      (3) For usage, see convertTiffG4ToPS().
 */
l_int32
convertTiffG4ToPSString(const char  *filein,
                        char       **poutstr,
                        l_int32     *pnbytes,
                        l_int32      x,
                        l_int32      y,
                        l_int32      res,
                        l_float32    scale,
                        l_int32      pageno,
                        l_int32      mask,
                        l_int32      endpage)
{
char      *pstring, *pstring2, *outstr;
char      *data85;  /* ascii85 encoded ccitt g4 data */
char       bigbuf[L_BUF_SIZE];
l_uint8   *bindata;  /* binary encoded ccitt g4 data */
l_int32    minisblack;   /* TRUE or FALSE */
l_int32    w, h;
l_int32    nbinbytes, nbytes85, psbytes, psbytes2, totbytes;
l_float32  xpt, ypt, wpt, hpt;
SARRAY    *sa, *sa2;

    PROCNAME("convertTiffG4ToPSString");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!poutstr)
        return ERROR_INT("&outstr not defined", procName, 1);
    if (!pnbytes)
        return ERROR_INT("&nbytes not defined", procName, 1);
    *poutstr = NULL;

        /* The returned ccitt g4 data in memory is the block of
         * bytes in the tiff file, starting after 8 bytes and
         * ending before the directory. */ 
    if (extractTiffG4DataFromFile(filein, &bindata, &nbinbytes,
                                  &w, &h, &minisblack))
        return ERROR_INT("bindata not extracted from file", procName, 1);

#if  DEBUG_G4
/*    arrayWrite("junkarray", "w", bindata, nbinbytes); */
    fprintf(stderr, "nbinbytes = %d, w = %d, h = %d, minisblack = %d\n",
            nbinbytes, w, h, minisblack);
#endif   /* DEBUG_G4 */

        /* Convert the ccittg4 encoded data to ascii85 */
    data85 = encodeAscii85(bindata, nbinbytes, &nbytes85);
    FREE(bindata);
    if (!data85)
        return ERROR_INT("data85 not made", procName, 1);

        /* Get scaled location in pts */
    if (scale == 0.0)
        scale = 1.0;
    if (res == 0) {
        if (h <= 3300)
            res = 300;
        else
            res = 600;
    }
    xpt = scale * x * 72. / res;
    ypt = scale * y * 72. / res;
    wpt = scale * w * 72. / res;
    hpt = scale * h * 72. / res;

#if  DEBUG_G4
    fprintf(stderr, "xpt = %7.2f, ypt = %7.2f, wpt = %7.2f, hpt = %7.2f\n",
             xpt, ypt, wpt, hpt);
#endif   /* DEBUG_G4 */

        /*  -------- generate PostScript output -------- */
    if ((sa = sarrayCreate(50)) == NULL)
        return ERROR_INT("sa not made", procName, 1);

    sarrayAddString(sa, "%!PS-Adobe-3.0", 1);
    sarrayAddString(sa, "%%Creator: leptonica", 1);
    sprintf(bigbuf, "%%%%Title: %s", filein);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "%%DocumentData: Clean7Bit", 1);

#if  PRINT_BOUNDING_BOX
    sprintf(bigbuf,
        "%%%%BoundingBox: %7.2f %7.2f %7.2f %7.2f",
                xpt, ypt, xpt + wpt, ypt + hpt);
    sarrayAddString(sa, bigbuf, 1);
#endif  /* PRINT_BOUNDING_BOX */

    sarrayAddString(sa, "%%LanguageLevel: 2", 1);
    sarrayAddString(sa, "%%EndComments", 1);
    sprintf(bigbuf, "%%%%Page: %d %d", pageno, pageno);
    sarrayAddString(sa, bigbuf, 1);

    sarrayAddString(sa, "save", 1);
    sarrayAddString(sa, "100 dict begin", 1);

    sprintf(bigbuf,
        "%7.2f %7.2f translate         %%set image origin in pts", xpt, ypt);
    sarrayAddString(sa, bigbuf, 1);

    sprintf(bigbuf,
        "%7.2f %7.2f scale             %%set image size in pts", wpt, hpt);
    sarrayAddString(sa, bigbuf, 1);

    sarrayAddString(sa, "/DeviceGray setcolorspace", 1);

    sarrayAddString(sa, "{", 1);
    sarrayAddString(sa, "  /RawData currentfile /ASCII85Decode filter def", 1);
    sarrayAddString(sa, "  << ", 1);
    sarrayAddString(sa, "    /ImageType 1", 1);
    sprintf(bigbuf, "    /Width %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "    /Height %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "    /ImageMatrix [ %d 0 0 %d 0 %d ]", w, -h, h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "    /BitsPerComponent 1", 1);
    sarrayAddString(sa, "    /Interpolate true", 1);
    if (minisblack)
        sarrayAddString(sa, "    /Decode [1 0]", 1);
    else  /* miniswhite; typical for 1 bpp */
        sarrayAddString(sa, "    /Decode [0 1]", 1);
    sarrayAddString(sa, "    /DataSource RawData", 1);
    sarrayAddString(sa, "        <<", 1);
    sarrayAddString(sa, "          /K -1", 1);
    sprintf(bigbuf, "          /Columns %d", w);
    sarrayAddString(sa, bigbuf, 1);
    sprintf(bigbuf, "          /Rows %d", h);
    sarrayAddString(sa, bigbuf, 1);
    sarrayAddString(sa, "        >> /CCITTFaxDecode filter", 1);
    if (mask == TRUE)  /* just paint through the fg */
        sarrayAddString(sa, "  >> imagemask", 1);
    else  /* paint full image */
        sarrayAddString(sa, "  >> image", 1);
    sarrayAddString(sa, "  RawData flushfile", 1);
    if (endpage == TRUE)
        sarrayAddString(sa, "  showpage", 1);
    sarrayAddString(sa, "}", 1);

    sarrayAddString(sa, "%%BeginData:", 1);
    sarrayAddString(sa, "exec", 1);

    if ((pstring = sarrayToString(sa, 1)) == NULL)
        return ERROR_INT("pstring not made", procName, 1);
    psbytes = strlen(pstring);

        /* Concat the trailing data */
    sa2 = sarrayCreate(10);
    sarrayAddString(sa2, "%%EndData", 1);
    sarrayAddString(sa2, "end", 1);
    sarrayAddString(sa2, "restore", 1);
    if ((pstring2 = sarrayToString(sa2, 1)) == NULL)
        return ERROR_INT("pstring2 not made", procName, 1);
    psbytes2 = strlen(pstring2);

        /* Add the ascii85 data */
    totbytes = psbytes + psbytes2 + nbytes85;
    *pnbytes = totbytes;
    if ((outstr = (char *)CALLOC(totbytes + 4, sizeof(char))) == NULL)
        return ERROR_INT("outstr not made", procName, 1);
    *poutstr = outstr;
    memcpy(outstr, pstring, psbytes);
    memcpy(outstr + psbytes, data85, nbytes85);
    memcpy(outstr + psbytes + nbytes85, pstring2, psbytes2);

    sarrayDestroy(&sa);
    sarrayDestroy(&sa2);
    FREE(data85);
    FREE(pstring);
    FREE(pstring2);
    return 0;
}


/*-------------------------------------------------------------*
 *                     For tiff multipage files                *
 *-------------------------------------------------------------*/
/*!
 *  convertTiffMultipageToPS()
 *
 *      Input:  filein (input tiff multipage file)
 *              fileout (output ps file)
 *              tempfile (<optional> for temporary g4 tiffs;
 *                        use NULL for default)
 *              factor (for filling 8.5 x 11 inch page;
 *                      use 0.0 for DEFAULT_FILL_FRACTION)
 *      Return: 0 if OK, 1 on error
 *      
 *  Notes:
 *      (1) This converts a multipage tiff file of binary page images
 *          into a ccitt g4 compressed PS file.
 *      (2) If the images are generated from a standard resolution fax,
 *          the vertical resolution is doubled to give a normal-looking
 *          aspect ratio.
 */
l_int32
convertTiffMultipageToPS(const char  *filein,
                         const char  *fileout,
                         const char  *tempfile,
                         l_float32    fillfract)
{
const char   tempdefault[] = "/usr/tmp/junk_temp_g4.tif";
const char  *tempname;
l_int32      i, npages, w, h, istiff;
l_float32    scale;
PIX         *pix, *pixs;
FILE        *fp;

    PROCNAME("convertTiffMultipageToPS");

    if (!filein)
        return ERROR_INT("filein not defined", procName, 1);
    if (!fileout)
        return ERROR_INT("fileout not defined", procName, 1);

    if ((fp = fopen(filein, "r")) == NULL)
        return ERROR_INT("file not found", procName, 1);
    istiff = fileFormatIsTiff(fp);
    if (!istiff) {
        fclose(fp);
        return ERROR_INT("file not tiff format", procName, 1);
    }
    tiffGetCount(fp, &npages);
    fclose(fp);

    if (tempfile)
        tempname = tempfile;
    else
        tempname = tempdefault;

    if (fillfract == 0.0)
        fillfract = DEFAULT_FILL_FRACTION;

    for (i = 0; i < npages; i++) {
        if ((pix = pixReadTiff(filein, i)) == NULL)
             return ERROR_INT("pix not made", procName, 1);

        w = pixGetWidth(pix);
        h = pixGetHeight(pix);
        if (w == 1728 && h < w)   /* it's a std res fax */
            pixs = pixScale(pix, 1.0, 2.0);
        else
            pixs = pixClone(pix);

        pixWrite(tempname, pixs, IFF_TIFF_G4);
        scale = L_MIN(fillfract * 2550 / w, fillfract * 3300 / h);
        if (i == 0)
            convertTiffG4ToPS(tempname, fileout, "w", 0, 0, 300, scale,
                              i + 1, FALSE, TRUE);
        else
            convertTiffG4ToPS(tempname, fileout, "a", 0, 0, 300, scale,
                              i + 1, FALSE, TRUE);
        pixDestroy(&pix);
        pixDestroy(&pixs);
    }

    return 0;
}


/*---------------------------------------------------------------------*
 *                          Write to memory                            *
 *---------------------------------------------------------------------*/
/*!
 *  pixWriteMemPS()
 *
 *      Input:  &data (<return> data of tiff compressed image)
 *              &size (<return> size of returned data)
 *              pix
 *              box  (<optional>)
 *              res  (can use 0 for default of 300 ppi)
 *              scale (to prevent scaling, use either 1.0 or 0.0)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See pixWriteStringPS() for usage.
 *      (2) This is just a wrapper for pixWriteStringPS(), which
 *          writes uncompressed image data to memory.
 */
l_int32
pixWriteMemPS(l_uint8  **pdata,
              size_t    *psize,
              PIX       *pix,
              BOX       *box,
              l_int32    res,
              l_float32  scale)
{
    PROCNAME("pixWriteMemPS");

    if (!pdata)
        return ERROR_INT("&data not defined", procName, 1 );
    if (!psize)
        return ERROR_INT("&size not defined", procName, 1 );
    if (!pix)
        return ERROR_INT("&pix not defined", procName, 1 );

    *pdata = (l_uint8 *)pixWriteStringPS(pix, box, res, scale);
    *psize = strlen((char *)(*pdata));
    return 0;
}


/*-------------------------------------------------------------*
 *                    Converting resolution                    *
 *-------------------------------------------------------------*/
/*!
 *  getResLetterPage()
 *
 *      Input:  w (image width, pixels)
 *              h (image height, pixels)
 *              fillfract (fraction in linear dimension of full page, not
 *                         to be exceeded; use 0 for default)
 *      Return: 0 if OK, 1 on error
 */
l_int32
getResLetterPage(l_int32    w,
                 l_int32    h,
                 l_float32  fillfract)
{
l_int32  resw, resh, res;

    if (fillfract == 0.0)
        fillfract = DEFAULT_FILL_FRACTION;
    resw = (l_int32)((w * 72.) / (LETTER_WIDTH * fillfract));
    resh = (l_int32)((h * 72.) / (LETTER_HEIGHT * fillfract));
    res = L_MAX(resw, resh);
    return res;
}


/*!
 *  getResA4Page()
 *
 *      Input:  w (image width, pixels)
 *              h (image height, pixels)
 *              fillfract (fraction in linear dimension of full page, not
 *                        to be exceeded; use 0 for default)
 *      Return: 0 if OK, 1 on error
 */
l_int32
getResA4Page(l_int32    w,
             l_int32    h,
             l_float32  fillfract)
{
l_int32  resw, resh, res;

    if (fillfract == 0.0)
        fillfract = DEFAULT_FILL_FRACTION;
    resw = (l_int32)((w * 72.) / (A4_WIDTH * fillfract));
    resh = (l_int32)((h * 72.) / (A4_HEIGHT * fillfract));
    res = L_MAX(resw, resh);
    return res;
}



/*-------------------------------------------------------------*
 *      Utility for encoding and decoding data with ascii85    *
 *-------------------------------------------------------------*/
/*!
 *  encodeAscii85()
 *
 *      Input:  inarray (input data)
 *              insize (number of bytes in input array)
 *              &outsize (<return> number of bytes in output char array)
 *      Return: chara (with 64 characters + \n in each line)
 *
 *  Notes:
 *      (1) Ghostscript has a stack break if the last line of
 *          data only has a '>', so we avoid the problem by
 *          always putting '~>' on the last line.
 */
char *
encodeAscii85(l_uint8  *inarray,
              l_int32   insize,
              l_int32  *poutsize)
{
char    *chara;
char    *outbuf;
l_int32  maxsize, i, index, outindex, linecount, nbout, eof;

    PROCNAME("encodeAscii85");

    if (!inarray)
        return (char *)ERROR_PTR("inarray not defined", procName, NULL);

        /* Accumulate results in chara */
    maxsize = (l_int32)(80. + (insize * 5. / 4.) *
                        (1. + 2. / MAX_85_LINE_COUNT));
    if ((chara = (char *)CALLOC(maxsize, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("chara not made", procName, NULL);

    if ((outbuf = (char *)CALLOC(8, sizeof(char))) == NULL)
        return (char *)ERROR_PTR("outbuf not made", procName, NULL);

    linecount = 0;
    index = 0;
    outindex = 0;
    while (1) {
        eof = convertChunkToAscii85(inarray, insize, &index, outbuf, &nbout);
        for (i = 0; i < nbout; i++) {
            chara[outindex++] = outbuf[i];
            linecount++;
            if (linecount >= MAX_85_LINE_COUNT) {
                chara[outindex++] = '\n';
                linecount = 0;
            }
        }
        if (eof == TRUE) {
            if (linecount != 0)
                chara[outindex++] = '\n';
            chara[outindex++] = '~';
            chara[outindex++] = '>';
            chara[outindex++] = '\n';
            break;
        }
    }
    
    FREE(outbuf);
    *poutsize = outindex;
    return chara;
}


/*!
 *  convertChunkToAscii85()
 *
 *      Input:  inarray (input data)
 *              insize  (number of bytes in input array)
 *              &index (use and <return> -- ptr)
 *              outbuf (holds 8 ascii chars; we use no more than 7)
 *              &nbsout (<return> number of bytes written to outbuf)
 *      Return: boolean for eof (0 if more data, 1 if end of file)
 *    
 *  Notes:
 *      (1) Attempts to read 4 bytes and write 5.
 *      (2) Writes 1 byte if the value is 0; writes 2 extra bytes if EOF.
 */
l_int32
convertChunkToAscii85(l_uint8  *inarray,
                      l_int32   insize,
                      l_int32  *pindex,
                      char     *outbuf,
                      l_int32  *pnbout)
{
l_uint8   inbyte;
l_uint32  inword, val;
l_int32   eof, index, nread, nbout, i;

    eof = FALSE;
    index = *pindex;
    nread = L_MIN(4, (insize - index));
    if (insize == index + nread)
        eof = TRUE;
    *pindex += nread;  /* save new index */

        /* Read input data and save in l_uint32 */
    inword = 0;
    for (i = 0; i < nread; i++) {
        inbyte = inarray[index + i];
        inword += inbyte << (8 * (3 - i));
    }

#if 0
    fprintf(stderr, "index = %d, nread = %d\n", index, nread);
    fprintf(stderr, "inword = %x\n", inword);
    fprintf(stderr, "eof = %d\n", eof);
#endif
    
        /* Special case: output 1 byte only */
    if (inword == 0) {
        outbuf[0] = 'z';
        nbout = 1;
    }
    else { /* output nread + 1 bytes */
        for (i = 4; i >= 4 - nread; i--) {
            val = inword / power85[i];
            outbuf[4 - i] = (l_uint8)(val + '!');
            inword -= val * power85[i];
        }
        nbout = nread + 1;
    }
    *pnbout = nbout;

    return eof;
}


/*!
 *  decodeAscii85()
 *
 *      Input:  inarray (ascii85 input data)
 *              insize (number of bytes in input array)
 *              &outsize (<return> number of bytes in output l_uint8 array)
 *      Return: outarray (binary)
 *
 *  Notes:
 *      (1) We assume the data is properly encoded, so we do not check
 *          for invalid characters or the final '>' character.
 *      (2) We permit whitespace to be added to the encoding in an
 *          arbitrary way.
 */
l_uint8 *
decodeAscii85(char     *ina,
              l_int32   insize,
              l_int32  *poutsize)
{
char      inc;
char     *pin;
l_uint8   val;
l_uint8  *outa;
l_int32   maxsize, ocount, bytecount, index;
l_uint32  oword;

    PROCNAME("decodeAscii85");

    if (!ina)
        return (l_uint8 *)ERROR_PTR("ina not defined", procName, NULL);

        /* Accumulate results in outa */
    maxsize = (l_int32)(80. + (insize * 4. / 5.));  /* plenty big */
    if ((outa = (l_uint8 *)CALLOC(maxsize, sizeof(l_uint8))) == NULL)
        return (l_uint8 *)ERROR_PTR("outa not made", procName, NULL);

    pin = ina;
    ocount = 0;  /* byte index into outa */
    oword = 0;
    for (index = 0, bytecount = 0; index < insize; index++, pin++) {
        inc = *pin;

        if (inc == ' ' || inc == '\t' || inc == '\n' ||
            inc == '\f' || inc == '\r' || inc == '\v')  /* ignore white space */
            continue;

        val = inc - '!';
        if (val < 85) {
            oword = oword * 85 + val;
            if (bytecount < 4)
                bytecount++;
            else {  /* we have all 5 input chars for the oword */
                outa[ocount] = (oword >> 24) & 0xff;
                outa[ocount + 1] = (oword >> 16) & 0xff;
                outa[ocount + 2] = (oword >> 8) & 0xff;
                outa[ocount + 3] = oword & 0xff;
                ocount += 4;
                bytecount = 0;
                oword = 0;
            }
        }
        else if (inc == 'z' && bytecount == 0) {
            outa[ocount] = 0;
            outa[ocount + 1] = 0;
            outa[ocount + 2] = 0;
            outa[ocount + 3] = 0;
            ocount += 4;
        }
        else if (inc == '~') {  /* end of data */
            fprintf(stderr, " %d extra bytes output\n", bytecount - 1);
            switch (bytecount) {
            case 0:   /* normal eof */
            case 1:   /* error */
                break;
            case 2:   /* 1 extra byte */
                oword = oword * (85 * 85 * 85) + 0xffffff;
                outa[ocount] = (oword >> 24) & 0xff; 
                break;
            case 3:   /* 2 extra bytes */
                oword = oword * (85 * 85) + 0xffff;
                outa[ocount] = (oword >> 24) & 0xff; 
                outa[ocount + 1] = (oword >> 16) & 0xff; 
                break;
            case 4:   /* 3 extra bytes */
                oword = oword * 85 + 0xff;
                outa[ocount] = (oword >> 24) & 0xff; 
                outa[ocount + 1] = (oword >> 16) & 0xff; 
                outa[ocount + 2] = (oword >> 8) & 0xff; 
                break;
            }
            if (bytecount > 1)
                ocount += (bytecount - 1);
            break;
        }
    }
    *poutsize = ocount;

    return outa;
}

/* --------------------------------------------*/
#endif  /* USE_PSIO */
/* --------------------------------------------*/

