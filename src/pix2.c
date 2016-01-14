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
 *  pix2.c
 *
 *      Pixel poking
 *           l_int32     pixGetPixel()
 *           l_int32     pixSetPixel()
 *           l_int32     pixClearPixel()
 *           l_int32     pixFlipPixel()
 *           void        setPixelLow()
 *
 *      Full image clear/set/set-to-arbitrary-value
 *           l_int32     pixClearAll()
 *           l_int32     pixSetAll()
 *           l_int32     pixSetAllArbitrary()
 *           l_int32     pixSetPadBits()
 *           l_int32     pixSetPadBitsBand()
 *
 *      Set border pixels to arbitrary value
 *           l_int32     pixSetOrClearBorder()
 *           l_int32     pixSetBorderVal()
 *
 *      Masked operations
 *           l_int32     pixSetMasked()
 *           l_int32     pixSetMaskedGeneral()
 *           l_int32     pixCombineMasked()
 *           l_int32     pixPaintThroughMask()
 *
 *      One and two-image boolean operations on arbitrary depth images
 *           PIX        *pixInvert()
 *           PIX        *pixOr()
 *           PIX        *pixAnd()
 *           PIX        *pixXor()
 *           PIX        *pixSubtract()
 *
 *      Pixel counting
 *           l_int32     pixZero()
 *           l_int32     pixCountPixels()
 *           NUMA       *pixaCountPixels()
 *           l_int32     pixCountPixelsInRow()
 *           NUMA       *pixCountPixelsByRow()
 *           l_int32     pixThresholdPixels()
 *           l_int32    *makePixelSumTab8()
 *
 *      Pixel histogram and averaging
 *           NUMA       *pixGrayHistogram()
 *           l_int32     pixGetAverageMasked()
 *
 *      Conversion between big and little endians
 *           PIX        *pixEndianByteSwapNew()
 *           l_int32     pixEndianByteSwap()
 *           l_int32     pixEndianTwoByteSwap()
 *
 *      Color sample setting and extraction
 *           PIX        *pixCreateRGBImage()
 *           PIX        *pixGetRGBComponent()
 *           l_int32     pixSetRGBComponent()
 *           l_int32     composeRGBPixel()
 *           l_int32     pixGetRGBLine()
 *
 *      Add and remove border
 *           PIX        *pixAddBorder()
 *           PIX        *pixRemoveBorder()
 *           PIX        *pixAddBorderGeneral()
 *           PIX        *pixRemoveBorderGeneral()
 *
 *      Test for pix equality
 *           l_int32     pixEqual()
 *           l_int32     pixEqualWithCmap()
 *
 *      Extract rectangle
 *           PIX        *pixClipRectangle()
 *
 *      Clip to foreground
 *           PIX        *pixClipToForeground()
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "allheaders.h"

static const l_uint32 rmask32[] = {0x0,
    0x00000001, 0x00000003, 0x00000007, 0x0000000f,
    0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
    0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
    0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff};


#ifndef  NO_CONSOLE_IO
#define   EQUAL_SIZE_WARNING      0
#endif  /* ~NO_CONSOLE_IO */



/*-------------------------------------------------------------*
 *                         Pixel poking                        *
 *-------------------------------------------------------------*/
/*!
 *  pixGetPixel()
 *
 *      Input:  pix
 *              (x,y) pixel coords
 *             &val (<return> pixel value)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixGetPixel(PIX       *pix,
	    l_int32    x,
	    l_int32    y,
	    l_uint32  *pval)
{
l_int32    w, h, d, wpl, val;
l_uint32  *line, *data;

    PROCNAME("pixGetPixel");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    if (!pval)
	return ERROR_INT("pval not defined", procName, 1);
    *pval = 0;

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (x < 0 || x >= w)
	return ERROR_INT("x out of bounds", procName, 1);
    if (y < 0 || y >= h)
	return ERROR_INT("y out of bounds", procName, 1);

    wpl = pixGetWpl(pix);
    data = pixGetData(pix);
    line = data + y * wpl;

    d = pixGetDepth(pix);
    switch (d)
    {
    case 1:
	val = GET_DATA_BIT(line, x);
	break;
    case 2:
	val = GET_DATA_DIBIT(line, x);
	break;
    case 4:
	val = GET_DATA_QBIT(line, x);
	break;
    case 8:
	val = GET_DATA_BYTE(line, x);
	break;
    case 16:
	val = GET_DATA_TWO_BYTES(line, x);
	break;
    case 32:
	val = line[x];
	break;
    default:
	return ERROR_INT("depth must be in {1,2,4,8,16,32} bpp", procName, 1);
    }

    *pval = val;
    return 0;
}


/*!
 *  pixSetPixel()
 *
 *      Input:  pix
 *              (x,y) pixel coords
 *              val (value to be inserted)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: the input value is not checked for overflow, and
 *        the sign bit (if any) is ignored.
 */
l_int32
pixSetPixel(PIX      *pix,
	    l_int32   x,
	    l_int32   y,
	    l_uint32  val)
{
l_int32    w, h, d, wpl;
l_uint32  *line, *data;

    PROCNAME("pixSetPixel");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (x < 0 || x >= w)
	return ERROR_INT("x out of bounds", procName, 1);
    if (y < 0 || y >= h)
	return ERROR_INT("y out of bounds", procName, 1);

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    line = data + y * wpl;

    d = pixGetDepth(pix);
    switch (d)
    {
    case 1:
	if (val)
	    SET_DATA_BIT(line, x);
	else
	    CLEAR_DATA_BIT(line, x);
	break;
    case 2:
	SET_DATA_DIBIT(line, x, val);
	break;
    case 4:
	SET_DATA_QBIT(line, x, val);
	break;
    case 8:
	SET_DATA_BYTE(line, x, val);
	break;
    case 16:
	SET_DATA_TWO_BYTES(line, x, val);
	break;
    case 32:
	line[x] = val;
	break;
    default:
	return ERROR_INT("depth must be in {1,2,4,8,16,32} bpp", procName, 1);
    }

    return 0;
}



/*!
 *  pixClearPixel()
 *
 *      Input:  pix
 *              (x,y) pixel coords
 *      Return: 0 if OK; 1 on error.
 */
l_int32
pixClearPixel(PIX     *pix,
	      l_int32  x,
	      l_int32  y)
{
l_int32    w, h, d, wpl;
l_uint32  *line, *data;

    PROCNAME("pixClearPixel");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (x < 0 || x >= w)
	return ERROR_INT("x out of bounds", procName, 1);
    if (y < 0 || y >= h)
	return ERROR_INT("y out of bounds", procName, 1);

    wpl = pixGetWpl(pix);
    data = pixGetData(pix);
    line = data + y * wpl;

    d = pixGetDepth(pix);
    switch (d)
    {
    case 1:
	CLEAR_DATA_BIT(line, x);
	break;
    case 2:
	CLEAR_DATA_DIBIT(line, x);
	break;
    case 4:
	CLEAR_DATA_QBIT(line, x);
	break;
    case 8:
	SET_DATA_BYTE(line, x, 0);
	break;
    case 16:
	SET_DATA_TWO_BYTES(line, x, 0);
	break;
    case 32:
	line[x] = 0;
	break;
    default:
	return ERROR_INT("depth must be in {1,2,4,8,16,32} bpp", procName, 1);
    }

    return 0;
}


/*!
 *  pixFlipPixel()
 *
 *      Input:  pix
 *              (x,y) pixel coords
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixFlipPixel(PIX     *pix,
	     l_int32  x,
	     l_int32  y)
{
l_int32    w, h, d, wpl;
l_uint32   val;
l_uint32  *line, *data;

    PROCNAME("pixFlipPixel");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (x < 0 || x >= w)
	return ERROR_INT("x out of bounds", procName, 1);
    if (y < 0 || y >= h)
	return ERROR_INT("y out of bounds", procName, 1);

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    line = data + y * wpl;

    d = pixGetDepth(pix);
    switch (d)
    {
    case 1:
	val = GET_DATA_BIT(line, x);
	if (val)
	    CLEAR_DATA_BIT(line, x);
	else
	    SET_DATA_BIT(line, x);
	break;
    case 2:
	val = GET_DATA_DIBIT(line, x);
	val ^= 0x3;
	SET_DATA_DIBIT(line, x, val);
	break;
    case 4:
	val = GET_DATA_QBIT(line, x);
	val ^= 0xf;
	SET_DATA_QBIT(line, x, val);
	break;
    case 8:
	val = GET_DATA_BYTE(line, x);
	val ^= 0xff;
	SET_DATA_BYTE(line, x, val);
	break;
    case 16:
	val = GET_DATA_TWO_BYTES(line, x);
	val ^= 0xffff;
	SET_DATA_TWO_BYTES(line, x, val);
	break;
    case 32:
	val = line[x] ^ 0xffffffff;
	line[x] = val;
	break;
    default:
	return ERROR_INT("depth must be in {1,2,4,8,16,32} bpp", procName, 1);
    }

    return 0;
}


/*!
 *  setPixelLow()
 *
 *      Input:  line (ptr to beginning of line),
 *              x (pixel location in line)
 *              depth (bpp)
 *              val (to be inserted)
 *      Return: void
 *
 *  Note: input variables are not checked
 */
void
setPixelLow(l_uint32  *line,
	    l_int32    x,
	    l_int32    depth,
	    l_uint32   val)
{
    switch (depth)
    {
    case 1:
	if (val)
	    SET_DATA_BIT(line, x);
	else
	    CLEAR_DATA_BIT(line, x);
	break;
    case 2:
	SET_DATA_DIBIT(line, x, val);
	break;
    case 4:
	SET_DATA_QBIT(line, x, val);
	break;
    case 8:
	SET_DATA_BYTE(line, x, val);
	break;
    case 16:
	SET_DATA_TWO_BYTES(line, x, val);
	break;
    case 32:
	line[x] = val;
	break;
    default:
	fprintf(stderr, "illegal depth in setPixelLow()\n");
    }

    return;
}



/*-------------------------------------------------------------*
 *     Full image clear/set/set-to-arbitrary-value/invert      *
 *-------------------------------------------------------------*/
/*!
 *  pixClearAll()
 *
 *      Input:  pix
 *      Return: 0 if OK, 1 on error
 *
 *  Action: clears all data to 0
 */
l_int32
pixClearAll(PIX  *pix)
{
    PROCNAME("pixClearAll");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    pixRasterop(pix, 0, 0, pixGetWidth(pix), pixGetHeight(pix),
                PIX_CLR, NULL, 0, 0);
    return 0;
}


/*!
 *  pixSetAll()
 *
 *      Input:  pix
 *      Return: 0 if OK, 1 on error
 *
 *  Action: sets all data to 1
 */
l_int32
pixSetAll(PIX  *pix)
{
    PROCNAME("pixSetAll");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    pixRasterop(pix, 0, 0, pixGetWidth(pix), pixGetHeight(pix),
                PIX_SET, NULL, 0, 0);
    return 0;
}


/*!
 *  pixSetAllArbitrary()
 *
 *      Input:  pix
 *              val  (value to set all pixels)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixSetAllArbitrary(PIX      *pix,
                   l_uint32  val)
{
l_int32    i, j, w, h, d, wpl, npix;
l_uint32   maxval, wordval;
l_uint32  *data, *line;

    PROCNAME("pixSetAllArbitrary");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    d = pixGetDepth(pix); 
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);

    if (d == 32)
	maxval = 0xffffffff;
    else
	maxval = (1 << d) - 1;
    if (val < 0) {
        L_WARNING("invalid pixel value; set to 0", procName);
	val = 0;
    }
    if (val > maxval) {
        L_WARNING_INT("invalid pixel val; set to maxval = %d",
		      procName, maxval);
	val = maxval;
    }

	/* set up word to tile with */
    wordval = 0;
    npix = 32 / d;    /* number of pixels per 32 bit word */
    for (j = 0; j < npix; j++)
	wordval |= (val << (j * d));

    wpl = pixGetWpl(pix);
    data = pixGetData(pix);
    for (i = 0; i < h; i++) {
	line = data + i * wpl;
	for (j = 0; j < wpl; j++) {
	    *(line + j) = wordval;
	}
    }

    return 0;
}


/*!
 *  pixSetPadBits()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              val  (0 or 1)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The pad bits are the bits that expand each scanline to a
 *          multiple of 32 bits.  They are usually not used in
 *          image processing operations.  When boundary conditions
 *          are important, as in seedfill, they must be set properly.
 *      (2) This sets the value of the pad bits (if any) in the last
 *          32-bit word in each scanline.
 *      (3) For 32 bpp pix, there are no pad bits, so this is a no-op.
 */
l_int32
pixSetPadBits(PIX     *pix,
              l_int32  val)
{
l_int32    i, w, h, d, wpl, endbits, fullwords;
l_uint32   mask;
l_uint32  *data, *pword;

    PROCNAME("pixSetPadBits");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    if ((d = pixGetDepth(pix)) == 32)  /* no padding exists for 32 bpp */
        return 0;  

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    endbits = 32 - ((w * d) % 32);
    if (endbits == 32)  /* no partial word */
	return 0;
    fullwords = w * d / 32;

    mask = rmask32[endbits];
    if (val == 0)
	mask = ~mask;

    for (i = 0; i < h; i++) {
	pword = data + i * wpl + fullwords;
	if (val == 0) /* clear */
	    *pword = *pword & mask;
	else  /* set */
	    *pword = *pword | mask;
    }

    return 0;
}


/*!
 *  pixSetPadBitsBand()
 *
 *      Input:  pix (1, 2, 4, 8, 16, 32 bpp)
 *              by  (starting y value of band)
 *              bh  (height of band)
 *              val  (0 or 1)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The pad bits are the bits that expand each scanline to a
 *          multiple of 32 bits.  They are usually not used in
 *          image processing operations.  When boundary conditions
 *          are important, as in seedfill, they must be set properly.
 *      (2) This sets the value of the pad bits (if any) in the last
 *          32-bit word in each scanline, within the specified
 *          band of raster lines.
 *      (3) For 32 bpp pix, there are no pad bits, so this is a no-op.
 */
l_int32
pixSetPadBitsBand(PIX     *pix,
                  l_int32  by,
                  l_int32  bh,
                  l_int32  val)
{
l_int32    i, w, h, d, wpl, endbits, fullwords;
l_uint32   mask;
l_uint32  *data, *pword;

    PROCNAME("pixSetPadBitsBand");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    if ((d = pixGetDepth(pix)) == 32)  /* no padding exists for 32 bpp */
        return 0;  

    h = pixGetHeight(pix);
    if (by < 0)
	by = 0;
    if (by >= h)
	return ERROR_INT("start y not in image", procName, 1);
    if (by + bh > h)
	bh = h - by;

    w = pixGetWidth(pix);
    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    endbits = 32 - ((w * d) % 32);
    if (endbits == 32)  /* no partial word */
	return 0;
    fullwords = w * d / 32;

    mask = rmask32[endbits];
    if (val == 0)
	mask = ~mask;

    for (i = by; i < by + bh; i++) {
	pword = data + i * wpl + fullwords;
	if (val == 0) /* clear */
	    *pword = *pword & mask;
	else  /* set */
	    *pword = *pword | mask;
    }

    return 0;
}


/*-------------------------------------------------------------*
 *                       Set border pixels                     *
 *-------------------------------------------------------------*/
/*!
 *  pixSetOrClearBorder()
 *
 *      Input:  pixs (all depths)
 *              leftpix, rightpix, toppix, bottompix
 *              operation (PIX_SET or PIX_CLR)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The border region is defined to be the region in the
 *          image within a specific distance of each edge.  Here, we
 *          allow the pixels within a specified distance of each
 *          edge to be set independently.  This either sets or
 *          clears all pixels in the border region.
 *      (2) For binary images, use PIX_SET for black and PIX_CLR for white.
 *      (3) For grayscale or color images, use PIX_SET for white
 *          and PIX_CLR for black.
 */
l_int32
pixSetOrClearBorder(PIX     *pixs,
                    l_int32  leftpix,
                    l_int32  rightpix,
                    l_int32  toppix,
                    l_int32  bottompix,
	            l_int32  op)
{
l_int32  w, h;

    PROCNAME("pixSetOrClearBorder");

    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);
    if (op != PIX_SET && op != PIX_CLR)
	return ERROR_INT("op must be PIX_SET or PIX_CLR", procName, 1);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);

    pixRasterop(pixs, 0, 0, leftpix, h, op, NULL, 0, 0);
    pixRasterop(pixs, w - rightpix, 0, rightpix, h, op, NULL, 0, 0);
    pixRasterop(pixs, 0, 0, w, toppix, op, NULL, 0, 0);
    pixRasterop(pixs, 0, h - bottompix, w, bottompix, op, NULL, 0, 0);

    return 0;
}


/*!
 *  pixSetBorderVal()
 *
 *      Input:  pixs (8 or 32 bpp)
 *              leftpix, rightpix, toppix, bottompix
 *              val (value to set at each border pixel)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The border region is defined to be the region in the
 *          image within a specific distance of each edge.  Here, we
 *          allow the pixels within a specified distance of each
 *          edge to be set independently.  This sets the pixels
 *          in the border region to the given input value.
 *      (2) For efficiency, use pixSetOrClearBorder() if
 *          you're setting the border to either black or white.
 *      (3) If d != 32, the input value should be masked off
 *          to the appropriate number of least significant bits.
 *      (4) The code is easily generalized for 2, 4 or 16 bpp.
 */
l_int32
pixSetBorderVal(PIX      *pixs,
                l_int32   leftpix,
                l_int32   rightpix,
                l_int32   toppix,
                l_int32   bottompix,
	        l_uint32  val)
{
l_int32    w, h, d, wpls, i, j, bstart, rstart;
l_uint32  *datas, *lines;

    PROCNAME("pixSetBorderVal");

    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);
    d = pixGetDepth(pixs);
    if (d != 8 && d != 32)
	return ERROR_INT("depth must be 8 or 32 bpp", procName, 1);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);

    if (d == 8) {
	val &= 0xff;
	for (i = 0; i < toppix; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < w; j++)
		SET_DATA_BYTE(lines, j, val);
	}
	rstart = w - rightpix;
	bstart = h - bottompix;
	for (i = toppix; i < bstart; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < leftpix; j++)
		SET_DATA_BYTE(lines, j, val);
	    for (j = rstart; j < w; j++)
		SET_DATA_BYTE(lines, j, val);
	}
	for (i = bstart; i < h; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < w; j++)
		SET_DATA_BYTE(lines, j, val);
	}
    }
    else {   /* d == 32 */
	for (i = 0; i < toppix; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < w; j++)
	        *(lines + j) = val;
	}
	rstart = w - rightpix;
	bstart = h - bottompix;
	for (i = toppix; i < bstart; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < leftpix; j++)
	        *(lines + j) = val;
	    for (j = rstart; j < w; j++)
	        *(lines + j) = val;
	}
	for (i = bstart; i < h; i++) {
	    lines = datas + i * wpls;
	    for (j = 0; j < w; j++)
	        *(lines + j) = val;
	}
    }

    return 0;
}
	


/*-------------------------------------------------------------*
 *                        Masked operations                    *
 *-------------------------------------------------------------*/
/*!
 *  pixSetMasked()
 *
 *      Input:  pixd (8, 16 or 32 bpp)
 *              pixm (<optional> 1 bpp mask; no operation if NULL)
 *              val (value to set at each masked pixel)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) In-place operation.  Calls pixSetMaskedCmap() for colormapped
 *          images.
 *      (2) If pixm == NULL, a warning is given.
 *      (3) It is an implicitly aligned operation, where the UL
 *          corners of pixd and pixm coincide.  A warning is
 *          issued if the two image sizes differ significantly,
 *          but the operation proceeds.
 *      (4) Each pixel in pixd that co-locates with an ON pixel
 *          in pixm is set to the specified input value.
 *          Other pixels in pixd are not changed.
 *      (5) You can visualize this as painting the color through
 *          the mask, as a stencil.
 *      (6) If you do not want to have the UL corners aligned,
 *          use the function pixSetMaskedGeneral(), which requires
 *          you to input the UL corner of pixm relative to pixd.
 */
l_int32
pixSetMasked(PIX      *pixd,
             PIX      *pixm,
	     l_uint32  val)
{
l_int32    wd, hd, wm, hm, w, h, d, wpld, wplm;
l_int32    i, j, bitval, rval, gval, bval;
l_uint32  *datad, *datam, *lined, *linem;

    PROCNAME("pixSetMasked");

    if (!pixd)
	return ERROR_INT("pixd not defined", procName, 1);
    if (!pixm) {
	L_WARNING("no mask; nothing to do", procName);
        return 0;
    }
    if (pixGetColormap(pixd)) {
        rval = GET_DATA_BYTE(&val, COLOR_RED);
        gval = GET_DATA_BYTE(&val, COLOR_GREEN);
        bval = GET_DATA_BYTE(&val, COLOR_BLUE);
        return pixSetMaskedCmap(pixd, pixm, 0, 0, rval, gval, bval);
    }

    wd = pixGetWidth(pixd);
    hd = pixGetHeight(pixd);
    wm = pixGetWidth(pixm);
    hm = pixGetHeight(pixm);
    w = L_MIN(wd, wm);
    h = L_MIN(hd, hm);
    if (L_ABS(wd - wm) > 7 || L_ABS(hd - hm) > 7)  /* allow a small tolerance */
        L_WARNING("pixd and pixm sizes differ", procName);
    d = pixGetDepth(pixd);
    if (d != 8 && d != 16 && d != 32)
	return ERROR_INT("pixd not 8, 16 or 32 bpp", procName, 1);
    if (pixGetDepth(pixm) != 1)
	return ERROR_INT("pixm not 1 bpp", procName, 1);

    datad = pixGetData(pixd);
    datam = pixGetData(pixm);
    wpld = pixGetWpl(pixd);
    wplm = pixGetWpl(pixm);
    if (d == 8) {
        val &= 0xff;
        for (i = 0; i < h; i++) {
            lined = datad + i * wpld;
            linem = datam + i * wplm;
            for (j = 0; j < w; j++) {
		bitval = GET_DATA_BIT(linem, j);
		if (bitval)
		    SET_DATA_BYTE(lined, j, val);
            }
        }
    }
    else if (d == 16) {
	val &= 0xffff;
	for (i = 0; i < h; i++) {
	    lined = datad + i * wpld;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		bitval = GET_DATA_BIT(linem, j);
		if (bitval)
		    SET_DATA_TWO_BYTES(lined, j, val);
	    }
	}
    }
    else {  /*  d == 32 */
	for (i = 0; i < h; i++) {
	    lined = datad + i * wpld;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		bitval = GET_DATA_BIT(linem, j);
		if (bitval)
		    *(lined + j) = val;
	    }
	}
    }

    return 0;
}


/*!
 *  pixSetMaskedGeneral()
 *
 *      Input:  pixd (8, 16 or 32 bpp)
 *              pixm (<optional> 1 bpp mask; no operation if null)
 *              val (value to set at each masked pixel)
 *              x, y (location of UL corner of pixm relative to pixd;
 *                    can be negative)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This is an in-place operation.
 *      (2) Alignment is explicit.  If you want the UL corners of
 *          the two images to be aligned, use pixSetMasked().
 *      (3) A typical use would be painting through the foreground
 *          of a small binary mask pixm, located somewhere on a
 *          larger pixd.  Other pixels in pixd are not changed.
 *      (4) You can visualize this as painting the color through
 *          the mask, as a stencil.
 *      (5) This uses rasterop to handle clipping and different depths of pixd.
 *      (6) If pixd has a colormap, you should call pixPaintThroughMask().
 *      (7) Why is this function here, if pixPaintThroughMask() does the
 *          same thing, and does it more generally?  I've retained it here
 *          to show how one can paint through a mask using only full
 *          image rasterops, rather than pixel peeking in pixm and poking
 *          in pixd.  It's somewhat baroque, but I found it amusing.
 */
l_int32
pixSetMaskedGeneral(PIX      *pixd,
                    PIX      *pixm,
	            l_uint32  val,
		    l_int32   x,
		    l_int32   y)
{
l_int32    wm, hm, d;
PIX       *pixmu, *pixc;

    PROCNAME("pixSetMaskedGeneral");

    if (!pixd)
	return ERROR_INT("pixd not defined", procName, 1);
    if (!pixm)  /* nothing to do */
        return 0;

    d = pixGetDepth(pixd);
    if (d != 8 && d != 16 && d != 32)
	return ERROR_INT("pixd not 8, 16 or 32 bpp", procName, 1);
    if (pixGetDepth(pixm) != 1)
	return ERROR_INT("pixm not 1 bpp", procName, 1);
    wm = pixGetWidth(pixm);
    hm = pixGetHeight(pixm);

        /* Unpack binary to depth d, with inversion:  1 --> 0, 0 --> 0xff... */
    if ((pixmu = pixUnpackBinary(pixm, d, 1)) == NULL)
	return ERROR_INT("pixmu not made", procName, 1);

        /* Clear stenciled pixels in pixd */
    pixRasterop(pixd, x, y, wm, hm, PIX_SRC & PIX_DST, pixmu, 0, 0);

        /* Generate image with requisite color */
    if ((pixc = pixCreateTemplate(pixmu)) == NULL)
	return ERROR_INT("pixc not made", procName, 1);
    pixSetAllArbitrary(pixc, val);

        /* Invert stencil mask, and paint color color into stencil */
    pixInvert(pixmu, pixmu);
    pixAnd(pixmu, pixmu, pixc);

        /* Finally, repaint stenciled pixels, with val, in pixd */
    pixRasterop(pixd, x, y, wm, hm, PIX_SRC | PIX_DST, pixmu, 0, 0);

    pixDestroy(&pixmu);
    pixDestroy(&pixc);
    return 0;
}


/*!
 *  pixCombineMasked()
 *
 *      Input:  pixd (8 or 32 bpp)
 *              pixs (8 or 32 bpp)
 *              pixm (<optional> 1 bpp mask; no operation if NULL)
 *      Return: 0 if OK; 1 on error
 *
 *  Action: sets each pixel in pixd that co-locates with an ON
 *          pixel in pixm to the corresponding value of pixs.
 *
 *  Implementation note:
 *      For 8 bpp selective masking, you might think that it would
 *      be faster to generate an 8 bpp version of pixm, using
 *      pixConvert1To8(pixm, 0, 255), and then use a general combine
 *      operation
 *          d = (d & ~m) | (s & m)
 *      on a word-by-word basis.  Not always.  The word-by-word combine
 *      takes a time that is independent of the mask data.  If the mask is
 *      relatively sparse, the byte-check method is actually faster!
 */
l_int32
pixCombineMasked(PIX  *pixd,
                 PIX  *pixs,
	         PIX  *pixm)
{
l_uint8    val;
l_int32    d, w, wd, wm, h, hd, hm, wpld, wpls, wplm, i, j;
l_uint32  *datad, *datas, *datam, *lined, *lines, *linem;

    PROCNAME("pixCombineMasked");

    if (!pixd)
	return ERROR_INT("pixd not defined", procName, 1);
    if (!pixm)  /* nothing to do */
	return 0;
    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);

    d = pixGetDepth(pixd);
    if (d != 8 && d != 32)
	return ERROR_INT("pixd not 8 or 32 bpp", procName, 1);
    if (pixGetDepth(pixm) != 1)
	return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (!pixSizesEqual(pixd, pixs))
	return ERROR_INT("pixs and pixd sizes differ", procName, 1);

    wd = pixGetWidth(pixd);
    hd = pixGetHeight(pixd);
    wm = pixGetWidth(pixm);
    hm = pixGetHeight(pixm);
    w = L_MIN(wd, wm);
    h = L_MIN(hd, hm);
    datad = pixGetData(pixd);
    datas = pixGetData(pixs);
    datam = pixGetData(pixm);
    wpld = pixGetWpl(pixd);
    wpls = pixGetWpl(pixs);
    wplm = pixGetWpl(pixm);

    if (d == 8) {
	for (i = 0; i < h; i++) {
	    lined = datad + i * wpld;
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		if (GET_DATA_BIT(linem, j)) {
		   val = GET_DATA_BYTE(lines, j);
		   SET_DATA_BYTE(lined, j, val);
		}
	    }
	}
    }
    else {  /* d == 32 */
	for (i = 0; i < h; i++) {
	    lined = datad + i * wpld;
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		if (GET_DATA_BIT(linem, j))
		   lined[j] = lines[j];
	    }
	}
    }

    return 0;
}


/*!
 *  pixPaintThroughMask()
 *
 *      Input:  pixd (8 or 32 bpp)
 *              pixm (<optional> 1 bpp mask)
 *              x, y (origin of pixm relative to pixd; can be negative)
 *              val (1 byte or rgb color to set at each masked pixel)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) In-place operation.  Calls pixSetMaskedCmap() for colormapped
 *          images.
 *      (2) For 8 bpp gray, we take the LSB of the color
 *      (3) If pixm == NULL, it's a no-op.
 *      (4) The mask origin is placed at (x,y) on pixd, and the
 *          operation is clipped to the intersection of rectangles.
 *      (5) For rgb, the components in val are in the canonical locations,
 *          with red in location COLOR_RED, etc.
 */
l_int32
pixPaintThroughMask(PIX      *pixd,
	            PIX      *pixm,
                    l_int32   x,
                    l_int32   y,
                    l_uint32  val)
{
l_int32    d, w, h, wm, hm, wpl, wplm, i, j, rval, gval, bval;
l_uint32  *data, *datam, *line, *linem;

    PROCNAME("pixPaintThroughMask");

    if (!pixd)
	return ERROR_INT("pixd not defined", procName, 1);
    if (!pixm)  /* nothing to do */
	return 0;
    if (pixGetColormap(pixd)) {
        rval = GET_DATA_BYTE(&val, COLOR_RED);
        gval = GET_DATA_BYTE(&val, COLOR_GREEN);
        bval = GET_DATA_BYTE(&val, COLOR_BLUE);
        return pixSetMaskedCmap(pixd, pixm, x, y, rval, gval, bval);
    }
    d = pixGetDepth(pixd);
    if (d != 8 && d != 32)
	return ERROR_INT("pixd not 8 or 32 bpp", procName, 1);
    if (pixGetDepth(pixm) != 1)
	return ERROR_INT("pixm not 1 bpp", procName, 1);

    w = pixGetWidth(pixd);
    h = pixGetHeight(pixd);
    wpl = pixGetWpl(pixd);
    data = pixGetData(pixd);
    wm = pixGetWidth(pixm);
    hm = pixGetHeight(pixm);
    wplm = pixGetWpl(pixm);
    datam = pixGetData(pixm);

    if (d == 8)
        val = val & 0xff;
    for (i = 0; i < hm; i++) {
        if (y + i < 0 || y + i >= h) continue;
        line = data + (y + i) * wpl;
        linem = datam + i * wplm;
        for (j = 0; j < wm; j++) {
	    if (x + j < 0 || x + j >= w) continue;
            if (GET_DATA_BIT(linem, j)) {
                switch (d)
                {
                case 8:
                    SET_DATA_BYTE(line, x + j, val);
                    break;
                case 32:
                    *(line + x + j) = val;
                    break;
                default:
                    return ERROR_INT("d not 8 or 32 bpp", procName, 1);
                }
            }
        }
    }

    return 0;
}
    

/*-------------------------------------------------------------*
 *    One and two-image boolean ops on arbitrary depth images  *
 *-------------------------------------------------------------*/
/*!
 *  pixInvert()
 *
 *      Input:  pixd  (<optional> destination: this can be null, 
 *                     equal to pixs, or different from pixs)
 *              pixs
 *      Return: pixd always
 *
 *  Inversion of pixs, independent of pixel depth.
 *  There are 3 cases:
 *      if pixd == null,   ~src --> new pixd
 *      if pixd == pixs,   ~src --> src  (in-place)
 *      if pixd != pixs,   ~src --> input pixd
 */
PIX *
pixInvert(PIX  *pixd,
          PIX  *pixs)
{
    PROCNAME("pixInvert");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);

    if (pixs != pixd) {
	if ((pixd = pixCopy(pixd, pixs)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }

    pixRasterop(pixd, 0, 0, pixGetWidth(pixd), pixGetHeight(pixd),
                PIX_NOT(PIX_DST), NULL, 0, 0);   /* invert pixd */

    return pixd;
}


/*!
 *  pixOr()
 *
 *      Input:  pixd  (<optional> destination: this can be null, 
 *                     equal to pixs1, or different from pixs1)
 *              pixs1 (can be == to pixd)
 *              pixs2 
 *      Return: pixd always
 *
 *  Notes:
 *      (1) This gives the union of two images with equal depth,
 *          aligning them to the the UL corner.
 *      (2) There are 3 cases:
 *            if pixd == null,   (src1 | src2) --> new pixd
 *            if pixd == pixs1,  (src1 | src2) --> src1  (in-place)
 *            if pixd != pixs1,  (src1 | src2) --> input pixd
 */
PIX *
pixOr(PIX  *pixd,
      PIX  *pixs1,
      PIX  *pixs2)
{
    PROCNAME("pixOr");

    if (!pixs1)
	return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixd);
    if (!pixs2)
	return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixd);
    if (pixs1 == pixs2)
	return (PIX *)ERROR_PTR("pixs1 and pixs2 must differ", procName, pixd);
    if (pixGetDepth(pixs1) != pixGetDepth(pixs2))
	return (PIX *)ERROR_PTR("depths not the same", procName, pixd);

#if  EQUAL_SIZE_WARNING
    if (!pixSizesEqual(pixs1, pixs2))
	L_WARNING("pixs1 and pixs2 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */

    if (pixs1 != pixd) {
	if ((pixd = pixCopy(pixd, pixs1)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }

	/* src1 | src2 --> dest */
    pixRasterop(pixd, 0, 0, pixGetWidth(pixd), pixGetHeight(pixd),
                PIX_SRC | PIX_DST, pixs2, 0, 0);

    return pixd;
}


/*!
 *  pixAnd()
 *
 *      Input:  pixd  (<optional> destination: this can be null, 
 *                     equal to pixs1, or different from pixs1)
 *              pixs1 (can be == to pixd)
 *              pixs2 
 *      Return: pixd always
 *
 *  Notes:
 *      (1) This gives the intersection of two images with equal depth,
 *          aligning them to the the UL corner.
 *      (2) There are 3 cases:
 *            if pixd == null,   (src1 & src2) --> new pixd
 *            if pixd == pixs1,  (src1 & src2) --> src1  (in-place)
 *            if pixd != pixs1,  (src1 & src2) --> input pixd
 */
PIX *
pixAnd(PIX  *pixd,
       PIX  *pixs1,
       PIX  *pixs2)
{
    PROCNAME("pixAnd");

    if (!pixs1)
	return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixd);
    if (!pixs2)
	return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixd);
    if (pixs1 == pixs2)
	return (PIX *)ERROR_PTR("pixs1 and pixs2 must differ", procName, pixd);
    if (pixGetDepth(pixs1) != pixGetDepth(pixs2))
	return (PIX *)ERROR_PTR("depths not the same", procName, pixd);

#if  EQUAL_SIZE_WARNING
    if (!pixSizesEqual(pixs1, pixs2))
	L_WARNING("pixs1 and pixs2 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */

    if (pixs1 != pixd) {
	if ((pixd = pixCopy(pixd, pixs1)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }

	/* src1 & src2 --> dest */
    pixRasterop(pixd, 0, 0, pixGetWidth(pixd), pixGetHeight(pixd),
                PIX_SRC & PIX_DST, pixs2, 0, 0);

    return pixd;
}


/*!
 *  pixXor()
 *
 *      Input:  pixd  (<optional> destination: this can be null, 
 *                     equal to pixs1, or different from pixs1)
 *              pixs1 (can be == to pixd)
 *              pixs2 
 *      Return: pixd always
 *
 *  Notes:
 *      (1) This gives the XOR of two images with equal depth,
 *          aligning them to the the UL corner.
 *      (2) There are 3 cases:
 *            if pixd == null,   (src1 ^ src2) --> new pixd
 *            if pixd == pixs1,  (src1 ^ src2) --> src1  (in-place)
 *            if pixd != pixs1,  (src1 ^ src2) --> input pixd
 */
PIX *
pixXor(PIX  *pixd,
       PIX  *pixs1,
       PIX  *pixs2)
{
    PROCNAME("pixXor");

    if (!pixs1)
	return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixd);
    if (!pixs2)
	return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixd);
    if (pixs1 == pixs2)
	return (PIX *)ERROR_PTR("pixs1 and pixs2 must differ", procName, pixd);
    if (pixGetDepth(pixs1) != pixGetDepth(pixs2))
	return (PIX *)ERROR_PTR("depths not the same", procName, pixd);

#if  EQUAL_SIZE_WARNING
    if (!pixSizesEqual(pixs1, pixs2))
	L_WARNING("pixs1 and pixs2 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */

    if (pixs1 != pixd) {
	if ((pixd = pixCopy(pixd, pixs1)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }

	/* src1 ^ src2 --> dest */
    pixRasterop(pixd, 0, 0, pixGetWidth(pixd), pixGetHeight(pixd),
                PIX_SRC ^ PIX_DST, pixs2, 0, 0);

    return pixd;
}


/*!
 *  pixSubtract()
 *
 *      Input:  pixd  (<optional> destination: this can be null, 
 *                     equal to pixs1, equal to pixs2, or different
 *                     from both pixs1 and pixs2)
 *              pixs1 (can be == to pixd)
 *              pixs2 (can be == to pixd)
 *      Return: pixd always
 *
 *  Notes:
 *      (1) This gives the set subtraction of two images with equal depth,
 *          aligning them to the the UL corner.
 *      (2) Source pixs2 is always subtracted from source pixs1.
 *          The result is
 *                  pixs1 \ pixs2 = pixs1 & (~pixs2)
 *      (3) There are 4 cases.  The result can go to a new dest,
 *          in-place to either pixs1 or pixs2, or to an existing input dest:
 *              if pixd == null,   (src1 - src2) --> new pixd
 *              if pixd == pixs1,  (src1 - src2) --> src1  (in-place)
 *              if pixd == pixs2,  (src1 - src2) --> src2  (in-place)
 *              if pixd != pixs1 && pixd != pixs2),
 *                                 (src1 - src2) --> input pixd
 */
PIX *
pixSubtract(PIX  *pixd,
            PIX  *pixs1,
            PIX  *pixs2)
{
l_int32  w, h;

    PROCNAME("pixSubtract");

    if (!pixs1)
	return (PIX *)ERROR_PTR("pixs1 not defined", procName, pixd);
    if (!pixs2)
	return (PIX *)ERROR_PTR("pixs2 not defined", procName, pixd);
    if (pixs1 == pixs2)
	return (PIX *)ERROR_PTR("pixs1 and pixs2 must differ", procName, pixd);
    if (pixGetDepth(pixs1) != pixGetDepth(pixs2))
	return (PIX *)ERROR_PTR("depths not the same", procName, pixd);

#if  EQUAL_SIZE_WARNING
    if (!pixSizesEqual(pixs1, pixs2))
	L_WARNING("pixs1 and pixs2 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */

    w = pixGetWidth(pixs1);
    h = pixGetHeight(pixs1);

    if (!pixd) {
	if ((pixd = pixCopy(NULL, pixs1)) == NULL)
	    return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
	pixRasterop(pixd, 0, 0, w, h, PIX_DST & PIX_NOT(PIX_SRC),
	    pixs2, 0, 0);   /* src1 & (~src2)  */
    }
    else if (pixd == pixs1) {
	pixRasterop(pixd, 0, 0, w, h, PIX_DST & PIX_NOT(PIX_SRC),
	    pixs2, 0, 0);   /* src1 & (~src2)  */
    }
    else if (pixd == pixs2) {
	pixRasterop(pixd, 0, 0, w, h, PIX_NOT(PIX_DST) & PIX_SRC,
	    pixs1, 0, 0);   /* src1 & (~src2)  */
    }
    else  { /* pixd != pixs1 && pixd != pixs2 */
	if (pixGetDepth(pixd) != 1)
	    return (PIX *)ERROR_PTR("pixd not binary", procName, pixd);
#if  EQUAL_SIZE_WARNING
	if (!pixSizesEqual(pixd, pixs1))
	    L_WARNING("pixd and pixs1 not equal sizes", procName);
#endif  /* EQUAL_SIZE_WARNING */
	pixRasterop(pixd, 0, 0, w, h, PIX_SRC, pixs1, 0, 0);   /* copy */
	pixRasterop(pixd, 0, 0, w, h, PIX_DST & PIX_NOT(PIX_SRC),
	    pixs2, 0, 0);   /* src1 & (~src2)  */
    }

    return pixd;
}


/*-------------------------------------------------------------*
 *                         Pixel counting                      *
 *-------------------------------------------------------------*/
/*!
 *  pixZero()
 *
 *      Input:  pix
 *              &empty  (<return> boolean: 1 if no ON pixels, 0 otherwise)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixZero(PIX      *pix,
        l_int32  *pempty)
{
l_int32    w, h, wpl, i, j, fullwords, endbits;
l_uint32   endmask;
l_uint32  *data, *line;

    PROCNAME("pixZero");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    if (!pempty)
	return ERROR_INT("pempty not defined", procName, 1);

    w = pixGetWidth(pix) * pixGetDepth(pix);
    h = pixGetHeight(pix);
    wpl = pixGetWpl(pix);
    data = pixGetData(pix);

    fullwords = w / 32;
    endbits = w & 31;
    endmask = 0xffffffff << (32 - endbits);

    *pempty = 1;
    for (i = 0; i < h; i++) {
	line = data + wpl * i;
	for (j = 0; j < fullwords; j++)
	    if (*line++) {
		*pempty = 0;
		return 0;
	    }
	if (endbits) {
	    if (*line & endmask) {
		*pempty = 0;
		return 0;
	    }
	}
    }

    return 0;
}


/*!
 *  pixCountPixels()
 *
 *      Input:  binary pix
 *              &count (<return> count of ON pixels)
 *              tab8  (<optional> 8-bit pixel lookup table)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixCountPixels(PIX      *pix,
	       l_int32  *pcount,
	       l_int32  *tab8)
{
l_uint8    endmask;
l_int32    w, h, wpl, i, j;
l_int32    fullbytes, endbits, sum;
l_int32   *tab;
l_uint32  *line, *data;

    PROCNAME("pixCountPixels");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    if (!pcount)
	return ERROR_INT("pcount not defined", procName, 1);
    if (pixGetDepth(pix) != 1)
	return ERROR_INT("pix not 1 bpp", procName, 1);

    *pcount = 0;

    if (!tab8) {
	if ((tab = makePixelSumTab8()) == NULL)
	    return ERROR_INT("tab not made", procName, 1);
    }
    else
	tab = tab8;

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    wpl = pixGetWpl(pix);
    data = pixGetData(pix);

    fullbytes = w / 8;
    endbits = w & 7;
    endmask = 0xff << (8 - endbits);

    sum = 0;
    for (i = 0; i < h; i++) {
	line = data + wpl * i;
	for (j = 0; j < fullbytes; j++)
	    sum += tab[GET_DATA_BYTE(line, j)];
	if (endbits)
	    sum += tab[GET_DATA_BYTE(line, fullbytes) & endmask];
    }
    *pcount = sum;

    if (!tab8)
	FREE((void *)tab);
    return 0;
}


/*!
 *  pixaCountPixels()
 *
 *      Input:  pixa (array of binary pix)
 *      Return: na of ON pixels in each pix, or null on error
 */
NUMA *
pixaCountPixels(PIXA  *pixa)
{
l_int32   d, i, n, count;
l_int32  *tab;
NUMA     *na;
PIX      *pix;

    PROCNAME("pixaCountPixels");

    if (!pixa)
	return (NUMA *)ERROR_PTR("pix not defined", procName, NULL);

    if ((n = pixaGetCount(pixa)) == 0)
	return numaCreate(1);

    pix = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pix);
    pixDestroy(&pix);
    if (d != 1)
	return (NUMA *)ERROR_PTR("pixa not 1 bpp", procName, NULL);

    if ((tab = makePixelSumTab8()) == NULL)
	return (NUMA *)ERROR_PTR("tab not made", procName, NULL);

    if ((na = numaCreate(n)) == NULL)
	return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    for (i = 0; i < n; i++) {
	pix = pixaGetPix(pixa, i, L_CLONE);
	pixCountPixels(pix, &count, tab);
	numaAddNumber(na, count);
	pixDestroy(&pix);
    }
	
    FREE((void *)tab);
    return na;
}


/*!
 *  pixCountPixelsInRow()
 *
 *      Input:  binary pix
 *              row number
 *              &count (<return> sum of ON pixels in raster line)
 *              tab8  (<optional> 8-bit pixel lookup table)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixCountPixelsInRow(PIX      *pix,
                    l_int32   row,
	            l_int32  *pcount,
	            l_int32  *tab8)
{
l_uint8    endmask;
l_int32    j, w, h, wpl;
l_int32    fullbytes, endbits, sum;
l_int32   *tab;
l_uint32  *line;

    PROCNAME("pixCountPixelsInRow");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    if (!pcount)
	return ERROR_INT("pcount not defined", procName, 1);
    if (pixGetDepth(pix) != 1)
	return ERROR_INT("pix not 1 bpp", procName, 1);

    *pcount = 0;

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    if (row < 0 || row >= h)
	return ERROR_INT("row out of bounds", procName, 1);
    wpl = pixGetWpl(pix);
    line = pixGetData(pix) + row * wpl;

    fullbytes = w / 8;
    endbits = w & 7;
    endmask = 0xff << (8 - endbits);

    if (!tab8) {
	if ((tab = makePixelSumTab8()) == NULL)
	    return ERROR_INT("tab not made", procName, 1);
    }
    else
	tab = tab8;

    sum = 0;
    for (j = 0; j < fullbytes; j++)
	sum += tab[GET_DATA_BYTE(line, j)];
    if (endbits)
	sum += tab[GET_DATA_BYTE(line, fullbytes) & endmask];
    *pcount = sum;

    if (!tab8)
	FREE((void *)tab);
    return 0;
}


/*!
 *  pixCountPixelsByRow()
 *
 *      Input:  binary pix
 *              tab8  (<optional> 8-bit pixel lookup table)
 *      Return: na of counts, or null on error
 */
NUMA *
pixCountPixelsByRow(PIX      *pix,
	            l_int32  *tab8)
{
l_int32   w, h, i, count;
l_int32  *tab;
NUMA     *na;

    PROCNAME("pixCountPixelsByRow");

    if (!pix)
	return (NUMA *)ERROR_PTR("pix not defined", procName, NULL);
    if (pixGetDepth(pix) != 1)
	return (NUMA *)ERROR_PTR("pix not 1 bpp", procName, NULL);

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);

    if (!tab8) {
	if ((tab = makePixelSumTab8()) == NULL)
	    return (NUMA *)ERROR_PTR("tab not made", procName, NULL);
    }
    else
	tab = tab8;

    if ((na = numaCreate(h)) == NULL)
	return (NUMA *)ERROR_PTR("na not made", procName, NULL);

    for (i = 0; i < h; i++) {
	pixCountPixelsInRow(pix, i, &count, tab);
	numaAddNumber(na, count);
    }

    if (!tab8)
	FREE((void *)tab);

    return na;
}


/*!
 *  pixThresholdPixels()
 *
 *      Input:  binary pix
 *              threshold
 *              &above (<return> 1 if above threshold;
 *                               0 if equal to or less than threshold)
 *              tab8  (8-bit pixel lookup table)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: This sums the ON pixels and returns immediately if the count
 *        goes above threshold.  It is therefore more efficient
 *        for matching images (by running this function on the xor of
 *        the 2 images)  than using pixCountPixels(), which counts all
 *        pixels before returning.
 */
l_int32
pixThresholdPixels(PIX      *pix,
	           l_int32   thresh,
	           l_int32  *pabove,
	           l_int32  *tab8)
{
l_uint8    endmask;
l_int32    w, h, wpl, i, j;
l_int32    fullbytes, endbits, sum;
l_uint32  *line, *data;

    PROCNAME("pixThresholdPixels");

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);
    if (!pabove)
	return ERROR_INT("pabove not defined", procName, 1);
    if (!tab8)
	return ERROR_INT("tab8 not defined", procName, 1);
    if (pixGetDepth(pix) != 1)
	return ERROR_INT("pix not 1 bpp", procName, 1);

    *pabove = 0;  /* init */

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    wpl = pixGetWpl(pix);
    data = pixGetData(pix);

    fullbytes = w / 8;
    endbits = w & 7;
    endmask = 0xff << (8 - endbits);

    sum = 0;
    for (i = 0; i < h; i++) {
	line = data + wpl * i;
	for (j = 0; j < fullbytes; j++)
	    sum += tab8[GET_DATA_BYTE(line, j)];
	if (endbits)
	    sum += tab8[GET_DATA_BYTE(line, fullbytes) & endmask];
	if (sum > thresh) {
	    *pabove = 1;
	    return 0;
	}
    }

    return 0;
}


/*!
 *  makePixelSumTab8()
 *
 *      Input:  void
 *      Return: table of 256 l_int32, or null on error
 *
 *  This table of integers gives the number of 1 bits in the 8 bit index
 */
l_int32 *
makePixelSumTab8(void)
{
l_uint8   byte;
l_int32   i;
l_int32  *tab;

    PROCNAME("makePixelSumTab8");

    if ((tab = (l_int32 *)CALLOC(256, sizeof(l_int32))) == NULL)
	return (l_int32 *)ERROR_PTR("tab not made", procName, NULL);

    for (i = 0; i < 256; i++) {
        byte = (l_uint8)i;
	tab[i] = (byte & 0x1) +
	         ((byte >> 1) & 0x1) +
	         ((byte >> 2) & 0x1) +
	         ((byte >> 3) & 0x1) +
	         ((byte >> 4) & 0x1) +
	         ((byte >> 5) & 0x1) +
	         ((byte >> 6) & 0x1) +
	         ((byte >> 7) & 0x1);
    }

    return tab;
}


/*------------------------------------------------------------------*
 *                  Pixel histogram and averaging                   *
 *------------------------------------------------------------------*/
/*!
 *  pixGrayHistogram()
 *
 *      Input:  pix (1, 2, 4, 8, 16 bpp)
 *      Return: na (histogram), or null on error
 */
NUMA *
pixGrayHistogram(PIX  *pixs)
{
l_int32     i, j, w, h, d, wpl, val, size;
l_uint32   *data, *line;
l_float32  *array;
NUMA       *na;

    PROCNAME("pixGrayHistogram");

    if (!pixs)
	return (NUMA *)ERROR_PTR("pixs not defined", procName, NULL);
    d = pixGetDepth(pixs);
    if (d > 16)
	return (NUMA *)ERROR_PTR("depth not in {1,2,4,8,16}", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    wpl = pixGetWpl(pixs);
    data = pixGetData(pixs);

    size = 1 << d;

    na = numaCreate(size);
    na->n = size;  /* fake storage of zeroes */
    array = na->array;  /* don't do this at home */

    for (i = 0; i < h; i++) {
	line = data + i * wpl;
	switch (d) 
	{
	case 1:
	    for (j = 0; j < w; j++) {
		val = GET_DATA_BIT(line, j);
		array[val] += 1.0;
	    }
	    break;
	case 2:
	    for (j = 0; j < w; j++) {
		val = GET_DATA_DIBIT(line, j);
		array[val] += 1.0;
	    }
	    break;
	case 4:
	    for (j = 0; j < w; j++) {
		val = GET_DATA_QBIT(line, j);
		array[val] += 1.0;
	    }
	    break;
	case 8:
	    for (j = 0; j < w; j++) {
		val = GET_DATA_BYTE(line, j);
		array[val] += 1.0;
	    }
	    break;
	case 16:
	    for (j = 0; j < w; j++) {
		val = GET_DATA_TWO_BYTES(line, j);
		array[val] += 1.0;
	    }
	    break;
        default:
	    numaDestroy(&na);
	    return (NUMA *)ERROR_PTR("illegal depth", procName, NULL);
	}
    }

    return na;
}


/*!
 *  pixGetAverageMasked()
 *
 *      Input:  pixs (8 bpp, or colormapped)
 *              pixm (<optional> 1 bpp mask over which average is to be taken;
 *                    use all pixels if null)
 *              x, y (UL corner of pixm relative to the UL corner of pixs; 
 *                    can be < 0)
 *              factor (subsampling factor; >= 1)
 *              &val (<return> average value)
 *      Return: 0 if OK, 1 on error
 *  Notes:
 *      (1) Computes the average value of pixels in pixs that are under
 *          the fg of the optional mask.  If the mask is null, it
 *          computes the average of the pixels in pixs.
 *      (2) Set the subsampling factor > 1 to reduce the amount of
 *          computation.
 *      (3) Clipping of pixm (if it exists) to pixs is done in the inner loop.
 *      (4) Input x,y are ignored unless pixm exists.
 */
l_int32
pixGetAverageMasked(PIX      *pixs,
                    PIX      *pixm,
                    l_int32   x,
                    l_int32   y,
                    l_int32   factor,
                    l_int32  *pval)
{
l_int32     i, j, w, h, wm, hm, wplg, wplm, sum, count;
l_uint32   *datag, *datam, *lineg, *linem;
PIX        *pixg;

    PROCNAME("pixGetAverageMasked");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 8 && !pixGetColormap(pixs))
        return ERROR_INT("pixs neither 8 bpp nor colormapped", procName, 1);
    if (pixm && pixGetDepth(pixm) != 1)
        return ERROR_INT("pixm not 1 bpp", procName, 1);
    if (factor < 1)
        return ERROR_INT("sampling factor < 1", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0;  /* init */

    if (pixGetColormap(pixs))
        pixg = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    else
        pixg = pixClone(pixs);

    w = pixGetWidth(pixg);
    h = pixGetHeight(pixg);
    datag = pixGetData(pixg);
    wplg = pixGetWpl(pixg);

    sum = count = 0;
    if (!pixm) {
        for (i = 0; i < h; i += factor) {
            lineg = datag + i * wplg;
            for (j = 0; j < w; j += factor) {
                sum += GET_DATA_BYTE(lineg, j);
                count++;
            }
        }
        pixDestroy(&pixg);
        if (count == 0)
            return ERROR_INT("no pixels sampled", procName, 1);
        *pval = sum / count;
    }
    else {
        wm = pixGetWidth(pixm);
        hm = pixGetHeight(pixm);
        datam = pixGetData(pixm);
        wplm = pixGetWpl(pixm);
        for (i = 0; i < hm; i += factor) {
            if (y + i < 0 || y + i >= h) continue;
            lineg = datag + (y + i) * wplg;
            linem = datam + i * wplm;
            for (j = 0; j < wm; j += factor) {
                if (x + j < 0 || x + j >= w) continue;
                if (GET_DATA_BIT(linem, j)) {
                    sum += GET_DATA_BYTE(lineg, x + j);
                    count++;
                }
            }
        }
        pixDestroy(&pixg);
        if (count == 0)
            return ERROR_INT("no pixels sampled", procName, 1);
        *pval = sum / count;
    }
    return 0;
}


/*-------------------------------------------------------------*
 *                    Pixel endian conversion                  *
 *-------------------------------------------------------------*/
/*!
 *  pixEndianByteSwapNew()
 *
 *      Input:  pixs
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) This is used to convert the data in a pix to a
 *          serialized byte buffer in raster order, and, for RGB,
 *          in order RGBA.  This requires flipping bytes within
 *          each 32-bit word for little-endian platforms, because the
 *          words have a MSB-to-the-left rule, whereas byte raster-order
 *          requires the left-most byte in each word to be byte 0.
 *          For big-endians, no swap is necessary, so this returns a clone.
 *      (2) Unlike pixEndianByteSwap(), which swaps the bytes in-place,
 *          this returns a new pix (or a clone).  We provide this
 *          because often when serialization is done, the source
 *          pix needs to be restored to canonical little-endian order,
 *          and this requires a second byte swap.  In such a situation,
 *          it is twice as fast to make a new pix in big-endian order,
 *          use it, and destroy it.
 */
PIX *
pixEndianByteSwapNew(PIX  *pixs)
{
l_uint32  *datas, *datad;
l_int32    i, j, h, wpl;
l_uint32   word;
PIX       *pixd;

    PROCNAME("pixEndianByteSwapNew");
        
#ifdef L_BIG_ENDIAN

    return pixClone(pixs);

#else   /* L_LITTLE_ENDIAN */

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    datas = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    h = pixGetHeight(pixs);
    pixd = pixCreateTemplate(pixs);
    datad = pixGetData(pixd);
    for (i = 0; i < h; i++) {
	for (j = 0; j < wpl; j++, datas++, datad++) {
	    word = *datas;
	    *datad = (word >> 24) |
	            ((word >> 8) & 0x0000ff00) |
	            ((word << 8) & 0x00ff0000) |
	            (word << 24);
	}
    }

    return pixd;

#endif   /* L_BIG_ENDIAN */

}


/*!
 *  pixEndianByteSwap()
 *
 *      Input:  pix
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is used on little-endian platforms to swap
 *          the bytes within a word; bytes 0 and 3 are swapped,
 *          and bytes 1 and 2 are swapped.
 *      (2) This is required for little-endians in situations
 *          where we convert from a serialized byte order that is
 *          in raster order, as one typically has in file formats,
 *          to one with MSB-to-the-left in each 32-bit word, or v.v.
 *          See pix.h for a description of the canonical format
 *          (MSB-to-the left) that is used for both little-endian
 *          and big-endian platforms.   For big-endians, the
 *          MSB-to-the-left word order has the bytes in raster
 *          order when serialized, so no byte flipping is required.
 */
l_int32
pixEndianByteSwap(PIX  *pix)
{
l_uint32  *data;
l_int32    i, j, h, wpl;
l_uint32   word;

    PROCNAME("pixEndianByteSwap");
        
#ifdef L_BIG_ENDIAN

    return 0;

#else   /* L_LITTLE_ENDIAN */

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    h = pixGetHeight(pix);
    for (i = 0; i < h; i++) {
	for (j = 0; j < wpl; j++, data++) {
	    word = *data;
	    *data = (word >> 24) |
	            ((word >> 8) & 0x0000ff00) |
	            ((word << 8) & 0x00ff0000) |
	            (word << 24);
	}
    }

    return 0;

#endif   /* L_BIG_ENDIAN */

}


/*!
 *  pixEndianTwoByteSwap()
 *
 *      Input:  pix
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is used on little-endian platforms to swap the
 *          2-byte entities within a 32-bit word.
 *      (2) This is equivalent to a full byte swap, as performed
 *          by pixEndianByteSwap(), followed by byte swaps in
 *          each of the 16-bit entities separately.
 */
l_int32
pixEndianTwoByteSwap(PIX  *pix)
{
l_uint32  *data;
l_int32    i, j, h, wpl;
l_uint32   word;

    PROCNAME("pixEndianTwoByteSwap");
        
#ifdef L_BIG_ENDIAN

    return 0;

#else   /* L_LITTLE_ENDIAN */

    if (!pix)
	return ERROR_INT("pix not defined", procName, 1);

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    h = pixGetHeight(pix);
    for (i = 0; i < h; i++) {
	for (j = 0; j < wpl; j++, data++) {
	    word = *data;
	    *data = ((word << 16) & 0xffff0000) |
	            ((word >> 16) & 0x0000ffff);
	}
    }

    return 0;

#endif   /* L_BIG_ENDIAN */

}


/*-------------------------------------------------------------*
 *                Color sample setting and extraction          *
 *-------------------------------------------------------------*/
/*!
 *  pixCreateRGBImage()
 *
 *      Input:  8 bpp red pix
 *              8 bpp green pix
 *              8 bpp blue pix
 *      Return: 32 bpp pix, interleaved with 4 samples/pixel,
 *              or null on error
 *
 *  Notes:
 *      (1) the 4th byte, sometimes called the "alpha channel",
 *          and which is often used for blending between different
 *          images, is left with 0 value.
 *      (2) see Note (4) in pix.h for details on storage of
 *          8-bit samples within each 32-bit word.
 */
PIX *
pixCreateRGBImage(PIX  *pixr,
                  PIX  *pixg,
                  PIX  *pixb)
{
l_int32  w, h;
PIX     *pixd;

    PROCNAME("pixCreateRGBImage");

    if (!pixr)
        return (PIX *)ERROR_PTR("pixr not defined", procName, NULL);
    if (!pixg)
        return (PIX *)ERROR_PTR("pixg not defined", procName, NULL);
    if (!pixb)
        return (PIX *)ERROR_PTR("pixb not defined", procName, NULL);
    if (pixGetDepth(pixr) != 8)
        return (PIX *)ERROR_PTR("pixr not 8 bpp", procName, NULL);
    if (pixGetDepth(pixg) != 8)
        return (PIX *)ERROR_PTR("pixg not 8 bpp", procName, NULL);
    if (pixGetDepth(pixb) != 8)
        return (PIX *)ERROR_PTR("pixb not 8 bpp", procName, NULL);
 
    w = pixGetWidth(pixr);
    h = pixGetHeight(pixr);
    if (w != pixGetWidth(pixg) || w != pixGetWidth(pixb))
        return (PIX *)ERROR_PTR("widths not the same", procName, NULL);
    if (h != pixGetHeight(pixg) || h != pixGetHeight(pixb))
        return (PIX *)ERROR_PTR("heights not the same", procName, NULL);

    if ((pixd = pixCreate(w, h, 32)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixr);
    pixSetRGBComponent(pixd, pixr, COLOR_RED);
    pixSetRGBComponent(pixd, pixg, COLOR_GREEN);
    pixSetRGBComponent(pixd, pixb, COLOR_BLUE);

    return pixd;
}


/*!
 *  pixGetRGBComponent()
 *
 *      Input:  pixs  (32 bpp)
 *              color  (one of {COLOR_RED, COLOR_GREEN, COLOR_BLUE,
 *                      L_ALPHA_CHANNEL})
 *      Return: pixd, the selected 8 bpp component image of the
 *              input 32 bpp image, or null on error
 *
 *  Notes:
 *      (1) The alpha channel (in the 4th byte of each RGB pixel)
 *          is not used in leptonica.
 */
PIX *
pixGetRGBComponent(PIX     *pixs,
	           l_int32  color)
{
l_uint8    srcbyte;
l_uint32  *lines, *lined;
l_uint32  *datas, *datad;
l_int32    i, j, w, h;
l_int32    wpls, wpld;
PIX 	  *pixd;

    PROCNAME("pixGetRGBComponent");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (color != COLOR_RED && color != COLOR_GREEN &&
        color != COLOR_BLUE && color != L_ALPHA_CHANNEL)
        return (PIX *)ERROR_PTR("invalid color", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if ((pixd = pixCreate(w, h, 8)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    wpls = pixGetWpl(pixs);
    wpld = pixGetWpl(pixd);
    datas = pixGetData(pixs);
    datad = pixGetData(pixd);

    for (i = 0; i < h; i++) {
	lines = datas + i * wpls;
	lined = datad + i * wpld;
	for (j = 0; j < w; j++) {
	    srcbyte = GET_DATA_BYTE(lines + j, color);
	    SET_DATA_BYTE(lined, j, srcbyte);
	}
    }

    return pixd;
}


/*!
 *  pixSetRGBComponent()
 *
 *      Input:  pixd  (32 bpp)
 *              pixs  (8 bpp)
 *              color  (one of {COLOR_RED, COLOR_GREEN, COLOR_BLUE,
 *                      L_ALPHA_CHANNEL})
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This places the 8 bpp pixel in pixs into the
 *          specified color component (properly interleaved) in pixd.
 *      (2) The alpha channel component is not used in leptonica.
 */
l_int32
pixSetRGBComponent(PIX     *pixd,
	           PIX     *pixs,
	           l_int32  color)
{
l_uint8    srcbyte;
l_int32    i, j, w, h;
l_int32    wpls, wpld;
l_uint32  *lines, *lined;
l_uint32  *datas, *datad;

    PROCNAME("pixSetRGBComponent");

    if (!pixd)
        return ERROR_INT("pixd not defined", procName, 1);
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    if (pixGetDepth(pixd) != 32)
        return ERROR_INT("pixd not 32 bpp", procName, 1);
    if (pixGetDepth(pixs) != 8)
        return ERROR_INT("pixs not 8 bpp", procName, 1);
    if (color != COLOR_RED && color != COLOR_GREEN &&
        color != COLOR_BLUE && color != L_ALPHA_CHANNEL)
        return ERROR_INT("invalid color", procName, 1);
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if (w != pixGetWidth(pixd) || h != pixGetHeight(pixd))
	return ERROR_INT("sizes not commensurate", procName, 1);

    datas = pixGetData(pixs);
    datad = pixGetData(pixd);
    wpls = pixGetWpl(pixs);
    wpld = pixGetWpl(pixd);

    for (i = 0; i < h; i++) {
	lines = datas + i * wpls;
	lined = datad + i * wpld;
	for (j = 0; j < w; j++) {
	    srcbyte = GET_DATA_BYTE(lines, j);
	    SET_DATA_BYTE(lined + j, color, srcbyte);
	}
    }

    return 0;
}


/*!
 *  composeRGBPixel()
 *
 *      Input:  rval, gval, bval
 *              &rgbpixel  (<return> 32-bit pixel)
 *      Return: 0 if OK; 1 on error
 */
l_int32
composeRGBPixel(l_int32    rval,
	        l_int32    gval,
		l_int32    bval,
		l_uint32  *ppixel)
{
    PROCNAME("composeRGBPixel");

    if (!ppixel)
        return ERROR_INT("&pixel not defined", procName, 1);

    *ppixel = 0;  /* want the alpha byte to be 0 */
    SET_DATA_BYTE(ppixel, COLOR_RED, rval);
    SET_DATA_BYTE(ppixel, COLOR_GREEN, gval);
    SET_DATA_BYTE(ppixel, COLOR_BLUE, bval);
    return 0;
}


/*!
 *  pixGetRGBLine()
 *
 *      Input:  pixs  (32 bpp)
 *              row
 *              bufr  (array of red samples; size w bytes)
 *              bufg  (array of green samples; size w bytes)
 *              bufb  (array of blue samples; size w bytes)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) This puts rgb components from the input line in pixs
 *          into the given buffers.
 */
l_int32
pixGetRGBLine(PIX      *pixs,
	      l_int32   row,
	      l_uint8  *bufr,
	      l_uint8  *bufg,
	      l_uint8  *bufb)
{
l_uint32  *lines;
l_int32    j, w, h;
l_int32    wpls;

    PROCNAME("pixGetRGBLine");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 32)
        return ERROR_INT("pixs not 32 bpp", procName, 1);
    if (!bufr || !bufg || !bufb)
        return ERROR_INT("buffer not defined", procName, 1);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if (row < 0 || row >= h)
        return ERROR_INT("row out of bounds", procName, 1);
    wpls = pixGetWpl(pixs);
    lines = pixGetData(pixs) + row * wpls;

    for (j = 0; j < w; j++) {
	bufr[j] = GET_DATA_BYTE(lines + j, COLOR_RED);
	bufg[j] = GET_DATA_BYTE(lines + j, COLOR_GREEN);
	bufb[j] = GET_DATA_BYTE(lines + j, COLOR_BLUE);
    }

    return 0;
}


/*-------------------------------------------------------------*
 *                     Add and remove border                   *
 *-------------------------------------------------------------*/
/*!
 *  pixAddBorder()
 *
 *      Input:  pix
 *              npix (number of pixels to be added to each side)
 *              val  (value of added border pixels)
 *      Return: pix with the input pix centered, or null on error.
 *
 *  Notes:
 *      (1) For binary images:
 *             white:  val = 0
 *             black:  val = 1
 *      (2) For grayscale images:
 *             white:  val = 2 ** d - 1
 *             black:  val = 0
 *      (3) For rgb color images:
 *             white:  val = 0xffffff00
 *             black:  val = 0
 */
PIX *
pixAddBorder(PIX      *pixs,
	     l_int32   npix,
	     l_uint32  val)
{
l_int32  ws, hs, wd, hd, d;
PIX     *pixd;

    PROCNAME("pixAddBorder");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    if (npix == 0)
	return pixClone(pixs);

    d = pixGetDepth(pixs); 
    ws = pixGetWidth(pixs);
    hs = pixGetHeight(pixs);
    wd = ws + 2 * npix;
    hd = hs + 2 * npix;
    if ((pixd = pixCreate(wd, hd, d)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);

    pixSetAllArbitrary(pixd, val);   /* a little extra writing ! */

    pixRasterop(pixd, npix, npix, ws, hs, PIX_SRC, pixs, 0, 0);

    return pixd;
}


/*!
 *  pixRemoveBorder()
 *
 *      Input:  pixs
 *              npix (number to be removed from each of the 4 sides)
 *      Return: pixd, or null on error
 */
PIX *
pixRemoveBorder(PIX     *pixs,
	        l_int32  npix)
{
l_int32  ws, hs, wd, hd, d;
PIX     *pixd;

    PROCNAME("pixRemoveBorder");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    if (npix == 0)
	return pixClone(pixs);

    d = pixGetDepth(pixs); 
    ws = pixGetWidth(pixs);
    hs = pixGetHeight(pixs);
    wd = ws - 2 * npix;
    hd = hs - 2 * npix;
    if (wd <= 0)
	return (PIX *)ERROR_PTR("width must be > 0", procName, NULL);
    if (hd <= 0)
	return (PIX *)ERROR_PTR("height must be > 0", procName, NULL);
    if ((pixd = pixCreate(wd, hd, d)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);

	/* Rasterop from the center */
    pixRasterop(pixd, 0, 0, wd, hd, PIX_SRC, pixs, npix, npix);
    
    return pixd;
}


/*!
 *  pixAddBorderGeneral()
 *
 *      Input:  pix
 *              leftpix, rightpix, toppix, bottompix  (number of pixels
 *                   to be added to each side)
 *              val   (value of added border pixels)
 *      Return: pix with the input pix placed properly, or null on error
 *
 *  Notes:
 *      (1) For binary images:
 *             white:  val = 0
 *             black:  val = 1
 *      (2) For grayscale images:
 *             white:  val = 2 ** d - 1
 *             black:  val = 0
 *      (3) For rgb color images:
 *             white:  val = 0xffffff00
 *             black:  val = 0
 */
PIX *
pixAddBorderGeneral(PIX      *pixs,
	            l_int32   leftpix,
	            l_int32   rightpix,
	            l_int32   toppix,
	            l_int32   bottompix,
	            l_uint32  val)
{
l_int32  ws, hs, wd, hd, d;
PIX     *pixd;

    PROCNAME("pixAddBorderGeneral");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    d = pixGetDepth(pixs); 
    ws = pixGetWidth(pixs);
    hs = pixGetHeight(pixs);
    wd = ws + leftpix + rightpix;
    hd = hs + toppix + bottompix;
    if ((pixd = pixCreate(wd, hd, d)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);

    pixSetAllArbitrary(pixd, val);   /* a little extra writing ! */

    pixRasterop(pixd, leftpix, toppix, ws, hs, PIX_SRC, pixs, 0, 0);

    return pixd;
}


/*!
 *  pixRemoveBorderGeneral()
 *
 *      Input:  pixs
 *              leftpix, rightpix, toppix, bottompix  (number of pixels
 *                   to be removed from each side)
 *      Return: pixd (with pixels removed around border), or null on error
 */
PIX *
pixRemoveBorderGeneral(PIX     *pixs,
	               l_int32  leftpix,
	               l_int32  rightpix,
	               l_int32  toppix,
	               l_int32  bottompix)
{
l_int32  ws, hs, wd, hd, d;
PIX     *pixd;

    PROCNAME("pixRemoveBorderGeneral");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);

    d = pixGetDepth(pixs); 
    ws = pixGetWidth(pixs);
    hs = pixGetHeight(pixs);
    wd = ws - leftpix - rightpix;
    hd = hs - toppix - bottompix;
    if (wd <= 0)
	return (PIX *)ERROR_PTR("width must be > 0", procName, NULL);
    if (hd <= 0)
	return (PIX *)ERROR_PTR("height must be > 0", procName, NULL);
    if ((pixd = pixCreate(wd, hd, d)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);

    pixRasterop(pixd, 0, 0, wd, hd, PIX_SRC, pixs, leftpix, toppix);
    
    return pixd;
}


/*------------------------------------------------------------------*
 *                        Test for pix equality                     *
 *------------------------------------------------------------------*/
/*!
 *  pixEqual()
 *
 *      Input:  pix1
 *              pix2
 *              &same  (<return> 1 if same; 0 if different)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) Equality is defined as having the same pixel values for
 *          each respective image pixel.
 *      (2) This works on two pix of any depth.  If one or both pix
 *          have a colormap, the depths can be different and the
 *          two pix can still be equal.
 *      (3) If both pix have colormaps and the depths are equal,
 *          use the pixEqualWithCmap() function, which does a fast
 *          comparison if the colormaps are identical and a relatively
 *          slow comparison otherwise.
 *      (4) In all other cases, any existing colormaps must first be
 *          removed before doing pixel comparison.  After the colormaps
 *          are removed, the resulting two images must have the same depth.
 *          The "lowest common denominator" is RGB, but this is only
 *          chosen when necessary, or when both have colormaps but
 *          different depths.
 *      (5) For 32 bpp, ignore the bits in the 4th byte (the 'A' byte
 *          of the RGBA pixel)
 *      (6) For images without colormaps that are not 32 bpp, all bits
 *          in the image part of the data array must be identical.
 */
l_int32
pixEqual(PIX      *pix1,
         PIX      *pix2,
	 l_int32  *psame)
{
l_int32    w, h, d, d2, wpl1, wpl2, i, j, color;
l_int32    fullwords, linebits, endbits;
l_uint32   endmask;
l_uint32  *data1, *data2, *line1, *line2;
PIX       *pixt1, *pixt2;
PIXCMAP   *cmap1, *cmap2;

    PROCNAME("pixEqual");

    if (!psame)
	return ERROR_INT("psamel not defined", procName, 1);
    *psame = 0;  /* pix are different unless we exit after checking all data */

    if (!pix1)
	return ERROR_INT("pix1 not defined", procName, 1);
    if (!pix2)
	return ERROR_INT("pix2 not defined", procName, 1);

    w = pixGetWidth(pix1);
    if (w != pixGetWidth(pix2)) {
        L_INFO("pix widths unequal", procName);
	return 0;
    }
    h = pixGetHeight(pix1);
    if (h != pixGetHeight(pix2)) {
        L_INFO("pix heights unequal", procName);
	return 0;
    }

    cmap1 = pixGetColormap(pix1);
    cmap2 = pixGetColormap(pix2);
    d = pixGetDepth(pix1);
    d2 = pixGetDepth(pix2);

    if (!cmap1 && !cmap2 && (d != d2)) {
	L_INFO("pix depths unequal and no colormaps", procName);
	return 0;
    }

    if (cmap1 && cmap2 && (d == d2))   /* use special function */
	return pixEqualWithCmap(pix1, pix2, psame);
	
	/* Must remove colormaps if they exist, and in the process
	 * end up with the resulting images having the same depth. */
    if (cmap1 && !cmap2) {
        pixcmapHasColor(cmap1, &color);
        if (color && d2 <= 8)  /* can't be equal */
            return 0;
	if (d2 <= 8)
	    pixt1 = pixRemoveColormap(pix1, REMOVE_CMAP_TO_GRAYSCALE);
        else
	    pixt1 = pixRemoveColormap(pix1, REMOVE_CMAP_TO_FULL_COLOR);
        pixt2 = pixClone(pix2);
    }
    else if (!cmap1 && cmap2) {
        pixcmapHasColor(cmap2, &color);
        if (color && d <= 8)  /* can't be equal */
            return 0;
        pixt1 = pixClone(pix1);
	if (d <= 8)
	    pixt2 = pixRemoveColormap(pix2, REMOVE_CMAP_TO_GRAYSCALE);
        else
	    pixt2 = pixRemoveColormap(pix2, REMOVE_CMAP_TO_FULL_COLOR);
    }
    else if (cmap1 && cmap2) {  /* depths not equal; use rgb */
	pixt1 = pixRemoveColormap(pix1, REMOVE_CMAP_TO_FULL_COLOR);
	pixt2 = pixRemoveColormap(pix2, REMOVE_CMAP_TO_FULL_COLOR);
    }
    else {  /* no colormaps */
        pixt1 = pixClone(pix1);
        pixt2 = pixClone(pix2);
    }

    d = pixGetDepth(pixt1);
    if (d != pixGetDepth(pixt2)) {
	pixDestroy(&pixt1);
	pixDestroy(&pixt2);
        L_INFO("intrinsic pix depths unequal", procName);
	return 0;
    }

    wpl1 = pixGetWpl(pixt1);
    wpl2 = pixGetWpl(pixt2);
    data1 = pixGetData(pixt1);
    data2 = pixGetData(pixt2);

    if (d == 32) {  /* assume RGBA, with A = don't-care */
        for (i = 0; i < h; i++) {
            line1 = data1 + wpl1 * i;
            line2 = data2 + wpl2 * i;
            for (j = 0; j < wpl1; j++) {
                if ((*line1 ^ *line2) & 0xffffff00) {
		    pixDestroy(&pixt1);
		    pixDestroy(&pixt2);
                    return 0;
		}
                line1++;
                line2++;
            }
        }
    }
    else  {  /* all bits count */
        linebits = d * w;
        fullwords = linebits / 32;
        endbits = linebits & 31;
        endmask = 0xffffffff << (32 - endbits);
        for (i = 0; i < h; i++) {
            line1 = data1 + wpl1 * i;
            line2 = data2 + wpl2 * i;
            for (j = 0; j < fullwords; j++) {
                if (*line1 ^ *line2) {
		    pixDestroy(&pixt1);
		    pixDestroy(&pixt2);
                    return 0;
		}
                line1++;
                line2++;
            }
            if (endbits) {
                if ((*line1 ^ *line2) & endmask) {
		    pixDestroy(&pixt1);
		    pixDestroy(&pixt2);
                    return 0;
		}
            }
        }
    }

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    *psame = 1;
    return 0;
}


/*!
 *  pixEqualWithCmap()
 *
 *      Input:  pix1
 *              pix2
 *              &same
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This returns same = TRUE if the images have identical content.
 *      (2) Both pix must have a colormap, and be of equal size and depth.
 *          If these conditions are not satisfied, it is not an error;
 *          the returned result is same = FALSE.
 *      (3) We then check whether the colormaps are the same; if so,
 *          the comparison proceeds 32 bits at a time.
 *      (4) If the colormaps are different, the comparison is done by 
 *          slow brute force.
 */
l_int32
pixEqualWithCmap(PIX      *pix1,
	         PIX      *pix2,
		 l_int32  *psame)
{
l_int32    d, w, h, wpl1, wpl2, i, j, linebits, fullwords, endbits;
l_int32    nc1, nc2, samecmaps;
l_int32    rval1, rval2, gval1, gval2, bval1, bval2;
l_uint32   endmask, val1, val2;
l_uint32  *data1, *data2, *line1, *line2;
PIXCMAP   *cmap1, *cmap2;

    PROCNAME("pixEqualWithCmap");

    if (!psame)
	return ERROR_INT("&same not defined", procName, 1);
    *psame = 0;
    if (!pix1)
	return ERROR_INT("pix1 not defined", procName, 1);
    if (!pix2)
	return ERROR_INT("pix2 not defined", procName, 1);

    if (pixSizesEqual(pix1, pix2) == 0)
	return 0;

    cmap1 = pixGetColormap(pix1);
    cmap2 = pixGetColormap(pix2);
    if (!cmap1 || !cmap2) {
        L_INFO("both images don't have colormap", procName);
	return 0;
    }
    d = pixGetDepth(pix1);
    if (d != 1 && d != 2 && d != 4 && d != 8) {
        L_INFO("pix depth not in {1, 2, 4, 8}", procName);
	return 0;
    }

    nc1 = pixcmapGetCount(cmap1);
    nc2 = pixcmapGetCount(cmap2);
    samecmaps = TRUE;
    if (nc1 != nc2) {
        L_INFO("colormap sizes are different", procName);
        samecmaps = FALSE;
    }

	/* Check if colormaps are identical */
    if (samecmaps == TRUE) {
        for (i = 0; i < nc1; i++) {
            pixcmapGetColor(cmap1, i, &rval1, &gval1, &bval1);
            pixcmapGetColor(cmap2, i, &rval2, &gval2, &bval2);
            if (rval1 != rval2 || gval1 != gval2 || bval1 != bval2) {
                samecmaps = FALSE;
                break;
            }
        }
    }

    h = pixGetHeight(pix1);
    w = pixGetWidth(pix1);
    if (samecmaps == TRUE) {  /* colormaps are identical; compare by words */
        linebits = d * w;
	wpl1 = pixGetWpl(pix1);
	wpl2 = pixGetWpl(pix2);
	data1 = pixGetData(pix1);
	data2 = pixGetData(pix2);
        fullwords = linebits / 32;
        endbits = linebits & 31;
        endmask = 0xffffffff << (32 - endbits);
        for (i = 0; i < h; i++) {
            line1 = data1 + wpl1 * i;
            line2 = data2 + wpl2 * i;
            for (j = 0; j < fullwords; j++) {
                if (*line1 ^ *line2)
                    return 0;
                line1++;
                line2++;
            }
            if (endbits) {
                if ((*line1 ^ *line2) & endmask)
                    return 0;
            }
        }
	*psame = 1;
	return 0;
    }

        /* Colormaps aren't identical; compare pixel by pixel */
    for (i = 0; i < h; i++) {
	for (j = 0; j < w; j++) {
	    pixGetPixel(pix1, j, i, &val1);
	    pixGetPixel(pix2, j, i, &val2);
	    pixcmapGetColor(cmap1, val1, &rval1, &gval1, &bval1);
	    pixcmapGetColor(cmap2, val2, &rval2, &gval2, &bval2);
	    if (rval1 != rval2 || gval1 != gval2 || bval1 != bval2)
	        return 0;
	}
    }

    *psame = 1;
    return 0;
}


/*-------------------------------------------------------------*
 *                Extract rectangular region                   *
 *-------------------------------------------------------------*/
/*!
 *  pixClipRectangle()
 *
 *      Input:  pixs
 *              box  (requested clipping region; const)
 *              &boxc (<optional return> actual box of clipped region)
 *      Return: clipped pix, or null on error or if rectangle
 *              doesn't intersect pixs
 *
 *  Notes:
 *
 *  This should be simple.  Yet it is not, and there are choices to
 *  be made.
 *
 *  The box is defined relative to the pix coordinates.  However,
 *  if the box exceeds the pix boundaries, we have two choices:
 *
 *      (1) clip the box to the pix
 *      (2) make a new pix equal to the full box dimensions,
 *          but let rasterop do the clipping and positioning
 *          of the src with respect to the dest
 *
 *  Choice (2) immediately brings up the problem of what pixel values
 *  to use that were not taken from the src!  For example, on a grayscale
 *  image, do you want the pixels not taken from the src to be black
 *  or white or something else?  To implement choice 2, one needs to
 *  specify the color of these extra pixels.
 *
 *  So we adopt (1), and clip the box first, if necessary,
 *  before making the dest pix and doing the rasterop.  But there
 *  are still problems to consider.
 *
 *  First, imagine that the box has y < 0, so that some of it is
 *  above the src.  If you clip a piece of the image using this box,
 *  you get a Pix with a height less than the box height.  The trouble
 *  comes when you then paste the Pix back in using the same box:
 *  it will be shifted up, and clipped to the top of the dest Pix,
 *  thus losing pixels at the top!  Remember that we are first clipping
 *  the box to the src, and then extracting the pix using the clipped
 *  box.  So to prevent the shift on replacement, it is necessary to
 *  use the clipped box!
 *
 *  Accordingly, this function has a third (optional) argument, which is
 *  the input box clipped to the src pix.
 *
 *  Now, imagine that the box extends past the bottom of the pix:
 *       box->y  >  pixGetHeight(pixs) - 1
 *  This will not cause any trouble on replacement using the
 *  original box, because the piece clipped out will go back
 *  in the same place when replaced.
 *
 *  We're not finished!  Here's a different use.
 *
 *  Suppose you want to clip a small pix (pix2) to a
 *  large one (pix1), and to preserve the alignment for some later operation.
 *  (For example, see blend.c).  The aligment of the two images is
 *  typically given by the origin of the smaller pix2 at (x, y)
 *  relative to the origin of the larger pix1.  Here, the "box" you
 *  use to clip pix2 is actually pix1 (properly translated), and
 *  it is defined by:
 *       box->x = -x
 *       box->y = -y
 *       box->w = pixGetWidth(pix1)
 *       box->h = pixGetHeight(pix1)
 *
 *  Consider again the two cases:
 *     (1) pix2 overhangs pix1 at the bottom, where
 *            y + pixGetHeight(pix2) > pixGetHeight(pix1)
 *         Then the lower part of pix2 is clipped, and
 *         it is properly placed with its origin at (x, y)
 *     (2) pix2 overhangs pix1 at the top, where
 *            y < 0
 *         Then the upper part of the pix2 is clipped, and it is
 *         properly placed with its origin at (x, y = 0)
 *
 *  So the general prescription for this use is:
 *     clipping: boxCreate(-x, -y, pixGetWidth(pix1), pixGetHeight(pix1))
 *     placement: origin (x, y) of pix2 is at:
 *          (L_MAX(0, x),  L_MAX(0, y))
 */
PIX *
pixClipRectangle(PIX   *pixs,
	         BOX   *box,
		 BOX  **pboxc)
{
l_int32  w, h, overw, overh, d;
BOX     *boxc;
PIX     *pixd;

    PROCNAME("pixClipRectangle");

    if (!pixs)
	return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!box)
	return (PIX *)ERROR_PTR("box not defined", procName, NULL);
    if (pboxc)
	*pboxc = NULL;

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    boxc = boxCopy(box);

        /* Clip boxc if necessary */
    if (boxc->x < 0) {
	boxc->w += boxc->x;  /* decrease the width */
	boxc->x = 0;
    }
    overw = boxc->x + boxc->w - w;
    if (overw > 0)
	boxc->w -= overw;  /* decrease the width */
    if (boxc->y < 0) {
	boxc->h += boxc->y;  /* decrease the height */
	boxc->y = 0;
    }
    overh = boxc->y + boxc->h - h;
    if (overh > 0)
	boxc->h -= overh;  /* decrease the height */

        /* Check: any pixels in the box? */
    if (boxc->w == 0 || boxc->h == 0) {  /* box outside of pix */
	boxDestroy(&boxc);
	L_WARNING("box doesn't overlap pix", procName);
	return NULL;
    }

        /* Now, we are guaranteed that boxc fits within pixs,
	 * so that all pixels in pixd are written by the rasterop. */
    d = pixGetDepth(pixs);
    if ((pixd = pixCreate(boxc->w, boxc->h, d)) == NULL)
	return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixCopyColormap(pixd, pixs);
    pixRasterop(pixd, 0, 0, boxc->w, boxc->h, PIX_SRC, pixs, boxc->x, boxc->y);

    if (pboxc)
	*pboxc = boxc;
    else
	boxDestroy(&boxc);

    return pixd;
}


/*-------------------------------------------------------------*
 *              Extract min rectangle with ON pixels           *
 *-------------------------------------------------------------*/
/*!
 *  pixClipToForeground()
 *
 *      Input:  pixs (1 bpp)
 *              &pixd  (<optional return> clipped pix returned)
 *              &box   (<optional return> bounding box)
 *      Return: 0 if OK; 1 on error or if there are no fg pixels
 *
 *  Notes:
 *      (1) At least one of {&pixd, &box} must be specified.
 *      (2) If there are no fg pixels, the returned ptrs are null.
 */
l_int32
pixClipToForeground(PIX   *pixs,
                    PIX  **ppixd,
                    BOX  **pbox)
{
l_int32    w, h, d, wpl, nfullwords, extra, i, j;
l_int32    minx, miny, maxx, maxy;
l_uint32   result, mask;
l_uint32  *data, *line;
BOX       *box;

    PROCNAME("pixClipToForeground");

    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);
    if ((d = pixGetDepth(pixs)) != 1)
	return ERROR_INT("pixs not binary", procName, 1);
    if (!ppixd && !pbox)
	return ERROR_INT("neither &pixd nor &pbox defined", procName, 1);

    w = pixGetWidth(pixs);
    nfullwords = w / 32;
    extra = w & 31;
    mask = ~rmask32[32 - extra];
    h = pixGetHeight(pixs);
    wpl = pixGetWpl(pixs);
    data = pixGetData(pixs);

    result = 0;
    for (i = 0, miny = 0; i < h; i++, miny++) {
	line = data + i * wpl;
	for (j = 0; j < nfullwords; j++)
	    result |= line[j];
	if (extra)
	    result |= (line[j] & mask);
	if (result)
	    break;
    }
    if (miny == h) {  /* no ON pixels */
        if (ppixd)
	    *ppixd = NULL;
	if (pbox)
	    *pbox = NULL;
        return 1;
    }

    result = 0;
    for (i = h - 1, maxy = h - 1; i >= 0; i--, maxy--) {
	line = data + i * wpl;
	for (j = 0; j < nfullwords; j++)
	    result |= line[j];
	if (extra)
	    result |= (line[j] & mask);
	if (result)
	    break;
    }

    minx = 0;
    for (j = 0, minx = 0; j < w; j++, minx++) {
	for (i = 0; i < h; i++) {
	    line = data + i * wpl;
	    if (GET_DATA_BIT(line, j))
		goto minx_found;
	}
    }

minx_found:
    for (j = w - 1, maxx = w - 1; j >= 0; j--, maxx--) {
	for (i = 0; i < h; i++) {
	    line = data + i * wpl;
	    if (GET_DATA_BIT(line, j))
		goto maxx_found;
	}
    }

maxx_found:
    box = boxCreate(minx, miny, maxx - minx + 1, maxy - miny + 1);

    if (ppixd)
	*ppixd = pixClipRectangle(pixs, box, NULL);
    if (pbox)
	*pbox = box;
    else
	boxDestroy(&box);

    return 0;
}

