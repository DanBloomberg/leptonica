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
 *  jpegio.c
 *                     
 *    Reading jpeg:
 *          PIX          *pixReadJpeg()  [ special top level ]
 *          PIX          *pixReadStreamJpeg()
 *
 *    Writing jpeg:
 *          l_int32       pixWriteJpeg()  [ special top level ]
 *          l_int32       pixWriteStreamJpeg()
 *
 *          static void   jpeg_error_do_not_exit();
 *
 *    Documentation: libjpeg.doc can be found, along with all
 *    source code, at ftp://ftp.uu.net/graphics/jpeg
 *    Download and untar the file:  jpegsrc.v6b.tar.gz
 *    A good paper on jpeg can also be found there: wallace.ps.gz
 *
 *    The functions in libjpeg make it very simple to compress
 *    and decompress images.  On input (decompression from file),
 *    3 component color images can be read into either an 8 bpp Pix
 *    with a colormap or a 32 bpp Pix with RGB components.  For output
 *    (compression to file), all color Pix, whether 8 bpp with a
 *    colormap or 32 bpp, are written compressed as a set of three
 *    8 bpp (rgb) images.
 *
 *    The default behavior of the jpeg library is to call exit.
 *    This is often undesirable, and the caller should make the
 *    decision when to abort a process.  So I inserted setjmp(s)
 *    in the reader and writer, wrote a static error handler that
 *    does not exit, and set up the cinfo structure so that the
 *    low-level jpeg library will call this error handler instead
 *    of the default function error_exit().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allheaders.h"

static void jpeg_error_do_not_exit(j_common_ptr cinfo);
static jmp_buf jpeg_jmpbuf;


/*---------------------------------------------------------------------*
 *                              Reading Jpeg                           *
 *---------------------------------------------------------------------*/
/*!
 *  pixReadJpeg()
 *
 *      Input:  filename
 *              colormap flag (0 means return RGB image if color;
 *                             1 means create colormap and return 8 bpp
 *                               palette image if color)
 *              reduction (scaling factor: 1, 2, 4 or 8)
 *             &pnwarn (<optional return> number of warnings about
 *                      corrupted data) 
 *      Return: pix, or null on error
 *
 *  Images reduced by factors of 2, 4 or 8 can be returned
 *  significantly faster than full resolution images.
 *
 *  The jpeg library will return warnings (or exit) if
 *  the jpeg data is bad.  Use this function if you want the
 *  jpeg library to create an 8 bpp palette image, or to
 *  tell if the jpeg data has been corrupted.  For corrupt jpeg
 *  data, there are two possible outcomes:
 *    (1) a damaged pix will be returned, along with a nonzero
 *        number of warnings, or
 *    (2) for sufficiently serious problems, the library will attempt
 *        to exit (caught by our error handler) and no pix will be returned.
 */
PIX *
pixReadJpeg(const char *filename,
	    l_int32     cmflag,
	    l_int32     reduction,
	    l_int32    *pnwarn)
{
FILE  *fp;
PIX   *pix;

    PROCNAME("pixReadJpeg");

    if (!filename)
        return (PIX *)ERROR_PTR("filename not defined", procName, NULL);
    if (pnwarn)
        *pnwarn = 0;  /* init */
    if (cmflag != 0 && cmflag != 1)
        cmflag = 0;  /* default */
    if (reduction != 1 && reduction != 2 && reduction != 4 && reduction != 8)
        return (PIX *)ERROR_PTR("reduction not in {1,2,4,8}", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIX *)ERROR_PTR("image file not found", procName, NULL);
    pix = pixReadStreamJpeg(fp, cmflag, reduction, pnwarn, 0);
    fclose(fp);

    if (!pix)
        return (PIX *)ERROR_PTR("image not returned", procName, NULL);
    return pix;
}


/*!
 *  pixReadStreamJpeg()
 *
 *      Input:  stream
 *              colormap flag (0 means return RGB image if color;
 *                             1 means create colormap and return 8 bpp
 *                               palette image if color)
 *              reduction (scaling factor: 1, 2, 4 or 8)
 *             &pnwarn (<optional return> number of warnings) 
 *              hint: a bitwise OR of L_HINT_* values
 *      Return: pix, or null on error
 *
 *  Usage: see pixReadJpeg()
 */
PIX *
pixReadStreamJpeg(FILE     *fp,
	          l_int32   cmflag,
	          l_int32   reduction,
	          l_int32  *pnwarn,
                  l_int32   hint)
{
l_uint8                        rval, gval, bval;
l_int32	                       i, j, k;
l_int32                        w, h, wpl, spp, ncolors, cindex;
l_uint32                      *data;
l_uint32                      *line, *ppixel;
JSAMPROW                       rowbuffer;
PIX 		              *pix;
PIXCMAP                       *cmap;
struct jpeg_decompress_struct  cinfo;
struct jpeg_error_mgr          jerr;

    PROCNAME("pixReadStreamJpeg");

    if (!fp)
        return (PIX *)ERROR_PTR("fp not defined", procName, NULL);
    if (pnwarn)
        *pnwarn = 0;  /* init */
    if (cmflag != 0 && cmflag != 1)
        cmflag = 0;  /* default */
    if (reduction != 1 && reduction != 2 && reduction != 4 && reduction != 8)
        return (PIX *)ERROR_PTR("reduction not in {1,2,4,8}", procName, NULL);

    if (BITS_IN_JSAMPLE != 8)  /* set in jmorecfg.h */
	return (PIX *)ERROR_PTR("BITS_IN_JSAMPLE != 8", procName, NULL);

    rewind(fp);

    pix = NULL;  /* init */
    if (setjmp(jpeg_jmpbuf)) {
        pixDestroy(&pix);
        FREE((void *)rowbuffer);
	return (PIX *)ERROR_PTR("internal jpeg error", procName, NULL);
    }

    rowbuffer = NULL;
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = jpeg_error_do_not_exit; /* catch error; do not exit! */

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.scale_denom = reduction;
    if (hint & L_HINT_GRAY)
        cinfo.out_color_space = JCS_GRAYSCALE;
    jpeg_calc_output_dimensions(&cinfo);

	/* Allocate the image and a row buffer */
    spp = cinfo.out_color_components;
    if (spp != 1 && spp != 3)
	return (PIX *)ERROR_PTR("spp must be 1 or 3", procName, NULL);
    w = cinfo.output_width;
    h = cinfo.output_height;
    if ((spp == 3) && (cmflag == 0))  /* get full color pix */
    {
        if ((rowbuffer = (JSAMPROW)CALLOC(sizeof(JSAMPLE), spp * w)) == NULL)
            return (PIX *)ERROR_PTR("rowbuffer not made", procName, NULL);
        if ((pix = pixCreate(w, h, 32)) == NULL)
	    return (PIX *)ERROR_PTR("pix not made", procName, NULL);
    }
    else {  /* 8 bpp gray or colormapped */
        if ((rowbuffer = (JSAMPROW)CALLOC(sizeof(JSAMPLE), w)) == NULL)
            return (PIX *)ERROR_PTR("rowbuffer not made", procName, NULL);
        if ((pix = pixCreate(w, h, 8)) == NULL)
	    return (PIX *)ERROR_PTR("pix not made", procName, NULL);
    }

    if (spp == 1)  /* Grayscale or colormapped */
        jpeg_start_decompress(&cinfo);
    else  {        /* Color; spp == 3 */
	if (cmflag == 0) {   /* -- 24 bit color in 32 bit pix -- */
            cinfo.quantize_colors = FALSE;
            jpeg_start_decompress(&cinfo);
	}
	else {      /* Color quantize to 8 bits */
            cinfo.quantize_colors = TRUE;
            cinfo.desired_number_of_colors = 256;
            jpeg_start_decompress(&cinfo);

	        /* Construct a pix cmap */
	    cmap = pixcmapCreate(8);
	    ncolors = cinfo.actual_number_of_colors;
            for (cindex = 0; cindex < ncolors; cindex++)
	    {
	        rval = cinfo.colormap[0][cindex];
	        gval = cinfo.colormap[1][cindex];
	        bval = cinfo.colormap[2][cindex];
		pixcmapAddColor(cmap, rval, gval, bval);
            }
	    pixSetColormap(pix, cmap);
	}
    }

    wpl  = pixGetWpl(pix);
    data = pixGetData(pix);

	/* Decompress */
    if ((spp == 3) && (cmflag == 0))
    {	/* -- 24 bit color -- */
        for (i = 0; i < h; i++) {
            if (jpeg_read_scanlines(&cinfo, &rowbuffer, (JDIMENSION)1) != 1)
	        return (PIX *)ERROR_PTR("bad read scanline", procName, NULL);
	    ppixel = data + i * wpl;
            for (j = k = 0; j < w; j++)
	    {
	        SET_DATA_BYTE(ppixel, COLOR_RED, rowbuffer[k++]);
	        SET_DATA_BYTE(ppixel, COLOR_GREEN, rowbuffer[k++]);
	        SET_DATA_BYTE(ppixel, COLOR_BLUE, rowbuffer[k++]);
		ppixel++;
            }
        }
    }
    else {    /* 8 bpp grayscale or colormapped pix */
        for (i = 0; i < h; i++) {
            if (jpeg_read_scanlines(&cinfo, &rowbuffer, (JDIMENSION)1) != 1)
	        return (PIX *)ERROR_PTR("bad read scanline", procName, NULL);
	    line = data + i * wpl;
            for (j = 0; j < w; j++)
	        SET_DATA_BYTE(line, j, rowbuffer[j]);
        }
    }

    if (pnwarn)
        *pnwarn = cinfo.err->num_warnings;

    switch (cinfo.density_unit)
    {
    case 1:  /* pixels per inch */
        pixSetXRes(pix, cinfo.X_density);
        pixSetYRes(pix, cinfo.Y_density);
        break;
    case 2:  /* pixels per centimeter */
	pixSetXRes(pix, (l_int32)((l_float32)cinfo.X_density * 2.54 + 0.5));
	pixSetYRes(pix, (l_int32)((l_float32)cinfo.Y_density * 2.54 + 0.5));
	break;
    default:   /* the pixel density may not be defined; ignore */
	break;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    FREE((void *)rowbuffer);

    return pix;
}



/*---------------------------------------------------------------------*
 *                             Writing Jpeg                            *
 *---------------------------------------------------------------------*/
/*!
 *  pixWriteJpeg()
 *
 *      Input:  filename
 *              pix
 *              quality (1 - 100; 75 is default)
 *              progressive (0 for baseline sequential; 1 for progressive)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixWriteJpeg(const char *filename, 
	     PIX        *pix, 
             l_int32     quality,
             l_int32     progressive)
{
FILE  *fp;

    PROCNAME("pixWriteJpeg");

    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);
    if (!filename)
	return ERROR_INT("filename not defined", procName, 1);

    if ((fp = fopen(filename, "wb+")) == NULL)
	return ERROR_INT("stream not opened", procName, 1);

    if (pixWriteStreamJpeg(fp, pix, quality, progressive)) {
	fclose(fp);
        return ERROR_INT("pix not written to stream", procName, 1);
    }

    fclose(fp);
    return 0;
}


/*!
 *  pixWriteStreamJpeg()
 *
 *      Input:  stream
 *              pix  (8 or 32 bpp)
 *              quality  (1 - 100; 75 is default value; 0 is also default)
 *              progressive (0 for baseline sequential; 1 for progressive)
 *      Return: 0 if OK, 1 on error
 *
 *  Action: there are three possibilities
 *     (1) grayscale image, no colormap: compress as 8 bpp image.
 *     (2) 24 bpp full color image: copy each line into the color
 *         line buffer, and compress as three 8 bpp images.
 *     (3) 8 bpp colormapped image: convert each line to three
 *         8 bpp line images in the color line buffer, and
 *         compress as three 8 bpp images.
 *
 *  Notes:
 *      (1) Under the covers, the library transforms rgb to a
 *          luminence-chromaticity triple, each component of which is
 *          also 8 bits, and compresses that.  It uses 2 Huffman tables,
 *          a higher resolution one (with more quantization levels)
 *          for luminosity and a lower resolution one for the chromas.
 *      (2) Progressive encoding gives better compression, at the
 *          expense of slower encoding and decoding.
 */
l_int32
pixWriteStreamJpeg(FILE    *fp,
	           PIX     *pix,
		   l_int32  quality,
                   l_int32  progressive)
{
l_uint8                      byteval;
l_int32                      xres, yres;
l_int32                      i, j, k;
l_int32                      w, h, d, wpl, spp, colorflg, rowsamples;
l_int32                     *rmap, *gmap, *bmap;
l_uint32                    *ppixel, *line, *data;
JSAMPROW                     rowbuffer;
PIXCMAP                     *cmap;
struct jpeg_compress_struct  cinfo;
struct jpeg_error_mgr        jerr;
const char                  *text;

    PROCNAME("pixWriteStreamJpeg");

    if (!fp)
	return ERROR_INT("stream not open", procName, 1);
    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    rewind(fp);

    if (setjmp(jpeg_jmpbuf)) {
        FREE((void *)rowbuffer);
        if (colorflg == 1) {
            FREE((void *)rmap);
            FREE((void *)gmap);
            FREE((void *)bmap);
        }
	return ERROR_INT("internal jpeg error", procName, 1);
    }

    rowbuffer = NULL;
    rmap = NULL;
    gmap = NULL;
    bmap = NULL;
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    d = pixGetDepth(pix);
    if (d != 8 && d != 32)
	return ERROR_INT("bpp must be 8 or 32", procName, 1);

    if (quality <= 0)
        quality = 75;  /* default */

    if (d == 32)
	colorflg = 2;    /* 24 bpp rgb; no colormap */
    else if ((cmap = pixGetColormap(pix)) == NULL)
	colorflg = 0;    /* 8 bpp grayscale; no colormap */
    else {   
	colorflg = 1;    /* 8 bpp; colormap */
	pixcmapToArrays(cmap, &rmap, &gmap, &bmap);
    }

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = jpeg_error_do_not_exit; /* catch error; do not exit! */

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);

    cinfo.image_width  = w;
    cinfo.image_height = h;

    if (colorflg == 0) {
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
    }
    else {  /* colorflg == 1 or 2 */
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
    }

    jpeg_set_defaults(&cinfo);

    xres = pixGetXRes(pix);
    yres = pixGetYRes(pix);
    if ((xres != 0) && (yres != 0)) {
	cinfo.density_unit = 1;  /* designates pixels per inch */
	cinfo.X_density = xres;
	cinfo.Y_density = yres;
    }

    jpeg_set_quality(&cinfo, quality, TRUE);
    if (progressive) {
        jpeg_simple_progression(&cinfo);
    }

    jpeg_start_compress(&cinfo, TRUE);

    if ((text = pixGetText(pix))) {
        jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET *)text, strlen(text));
    }

	/* Allocate row buffer */
    spp = cinfo.input_components;
    rowsamples = spp * w;
    if ((rowbuffer = (JSAMPROW)CALLOC(sizeof(JSAMPLE), rowsamples)) == NULL)
	return ERROR_INT("calloc fail for rowbuffer", procName, 1);

    data = pixGetData(pix);
    wpl  = pixGetWpl(pix);
    for (i = 0; i < h; i++) {
        line = data + i * wpl;
	if (colorflg == 0) {	/* 8 bpp gray */
	    for (j = 0; j < w; j++)
		rowbuffer[j] = GET_DATA_BYTE(line, j);
	}
	else if (colorflg == 1) {  /* 8 bpp colormapped */
	    for (j = 0; j < w; j++) {
		byteval = GET_DATA_BYTE(line, j);
		rowbuffer[3 * j + COLOR_RED] = rmap[byteval];
		rowbuffer[3 * j + COLOR_GREEN] = gmap[byteval];
		rowbuffer[3 * j + COLOR_BLUE] = bmap[byteval];
	    }
	}
	else { /* 24 bpp color */
	    ppixel = line;
	    for (j = k = 0; j < w; j++) {
		rowbuffer[k++] = GET_DATA_BYTE(ppixel, COLOR_RED);
		rowbuffer[k++] = GET_DATA_BYTE(ppixel, COLOR_GREEN);
		rowbuffer[k++] = GET_DATA_BYTE(ppixel, COLOR_BLUE);
		ppixel++;
	    }
	}
        jpeg_write_scanlines(&cinfo, &rowbuffer, 1);
    }

    jpeg_finish_compress(&cinfo);

    FREE((void *)rowbuffer);
    if (colorflg == 1) {
	FREE((void *)rmap);
	FREE((void *)gmap);
	FREE((void *)bmap);
    }

    jpeg_destroy_compress(&cinfo);
    
    return 0;
}


    /* The default jpeg error_exit() kills the process.
     * We don't want leptonica to allow this to happen.
     * If you want this default behavior, remove the
     * calls to this in the functions above. */
static void jpeg_error_do_not_exit(j_common_ptr cinfo)
{
    (*cinfo->err->output_message) (cinfo);
    jpeg_destroy(cinfo);
    longjmp(jpeg_jmpbuf, 0);
    return;
}
