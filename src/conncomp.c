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
 *  conncomp.c:  stack-based filling and connected component extraction
 *
 *      4- and 8-connected components: bounding boxes and images
 *
 *           BOXA     *pixConnComp()
 *           BOXA     *pixConnCompPixa()
 *           BOXA     *pixConnCompBB()
 *
 *           l_int32   nextOnPixelInRaster()
 *           l_int32   nextOnPixelInRasterLow()
 *
 *           BOX      *pixSeedfillBB()
 *           BOX      *pixSeedfill4BB()
 *           BOX      *pixSeedfill8BB()
 *
 *      Stack helper functions for seedfill
 *
 *           void      pushFillsegBB()
 *           void      popFillsegBB()
 *
 *  The basic method in pixConnCompBB() is very simple.  We scan the
 *  image in raster order, looking for the next ON pixel.  When it
 *  is found, we erase it and every pixel of the 4- or 8-connected
 *  component to which it belongs, using Heckbert's seedfill
 *  algorithm.  As pixels are erased, we keep track of the
 *  minimum rectangle that encloses all erased pixels; after
 *  the connected component has been erased, we save its
 *  bounding box in an array of boxes.  When all pixels in the
 *  image have been erased, we have an array that describes every
 *  4- or 8-connected component in terms of its bounding box.
 *
 *  pixConnCompPixa() is a slight variation on pixConnCompBB(),
 *  where we additionally save an array of images (in a Pixa)
 *  of each of the 4- or 8-connected components.  This is done trivially
 *  by maintaining two temporary images.  We erase a component from one,
 *  and use the bounding box to extract the pixels within the b.b.
 *  from each of the two images.  An XOR between these subimages
 *  gives the erased component.  Then we erase the component from the
 *  second image using the XOR again, with the extracted component
 *  placed on the second image at the location of the bounding box.
 *  Rasterop does all the work.  At the end, we have an array
 *  of the 4- or 8-connected components, as well as an array of the
 *  bounding boxes that describe where they came from in the original image.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#ifndef  NO_CONSOLE_IO
#define   DEBUG    0
#endif  /* ~NO_CONSOLE_IO */


/*-----------------------------------------------------------------------*
 *                Bounding boxes of 4 Connected Components               *
 *-----------------------------------------------------------------------*/
/*!
 *  pixConnComp()
 *
 *      Input:  pixs (1 bpp)
 *              &pixa   (<optional> pixa of each c.c.)
 *              connectivity (4 or 8)
 *      Return: boxa, or null on error
 *
 *  Notes:
 *      (1) This is the top-level call, and it can be used instead
 *          of either pixConnCompBB() or pixConnCompPixa().
 */
BOXA *
pixConnComp(PIX     *pixs,
            PIXA   **ppixa,
	    l_int32  connectivity)
{

    PROCNAME("pixConnComp");

    if (!pixs)
        return (BOXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (BOXA *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (BOXA *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    if (!ppixa)
	return pixConnCompBB(pixs, connectivity);
    else
	return pixConnCompPixa(pixs, ppixa, connectivity);
}


/*!
 *  pixConnCompPixa()
 *
 *      Input:  pixs (1 bpp)
 *              &pixa (return pixa of each c.c.)
 *              connectivity (4 or 8)
 *      Return: boxa, or null on error
 *
 *  Notes:
 *      (1) This finds bounding boxes of 4- or 8-connected components
 *          in a binary image, and saves images of each c.c
 *          in a pixa array.
 *      (2) It sets up 2 temporary pix, and for each c.c. that is
 *          located in raster order, it erases the c.c. from one pix,
 *          then uses the b.b. to extract the c.c. from the two pix using
 *          an XOR, and finally erases the c.c. from the second pix.
 *      (3) A clone of the returned boxa (where all boxes in the array
 *          are clones) is inserted into the pixa.
 */
BOXA *
pixConnCompPixa(PIX     *pixs,
                PIXA   **ppixa,
	        l_int32  connectivity)
{
l_int32  h, iszero;
l_int32  x, y, xstart, ystart;
PIX     *pixt1, *pixt2, *pixt3, *pixt4;
PIXA    *pixa;
BOX     *box;
BOXA    *boxa;
STACK   *stack, *auxstack;

    PROCNAME("pixConnCompPixa");

    if (!pixs)
        return (BOXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (BOXA *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (!ppixa)
        return (BOXA *)ERROR_PTR("&pixa not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (BOXA *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    *ppixa = NULL;  /* init */
    pixZero(pixs, &iszero);
    if (iszero)
        return boxaCreate(1);  /* return empty boxa */

    if ((pixt1 = pixCopy(NULL, pixs)) == NULL)
        return (BOXA *)ERROR_PTR("pixt1 not made", procName, NULL);
    if ((pixt2 = pixCopy(NULL, pixs)) == NULL)
        return (BOXA *)ERROR_PTR("pixt2 not made", procName, NULL);
    if ((pixa = pixaCreate(0)) == NULL)
        return (BOXA *)ERROR_PTR("pixa not made", procName, NULL);
    *ppixa = pixa;

    h = pixGetHeight(pixs);
    if ((stack = stackCreate(h)) == NULL)
        return (BOXA *)ERROR_PTR("stack not made", procName, NULL);
    if ((auxstack = stackCreate(0)) == NULL)
        return (BOXA *)ERROR_PTR("auxstack not made", procName, NULL);
    stack->auxstack = auxstack;
    if ((boxa = boxaCreate(0)) == NULL)
	return (BOXA *)ERROR_PTR("boxa not made", procName, NULL);

    xstart = 0;
    ystart = 0;
    while (1)
    {
        if (!nextOnPixelInRaster(pixt1, xstart, ystart, &x, &y))
            break;

        if ((box = pixSeedfillBB(pixt1, stack, x, y, connectivity)) == NULL)
            return (BOXA *)ERROR_PTR("box not made", procName, NULL);
	boxaAddBox(boxa, box, L_INSERT);

	    /* Save the c.c. and remove from pixt2 as well */
	pixt3 = pixClipRectangle(pixt1, box, NULL);
	pixt4 = pixClipRectangle(pixt2, box, NULL);
	pixXor(pixt3, pixt3, pixt4);
	pixRasterop(pixt2, box->x, box->y, box->w, box->h, PIX_SRC ^ PIX_DST,
		    pixt3, 0, 0);
	pixaAddPix(pixa, pixt3, L_INSERT);
	pixDestroy(&pixt4);

        xstart = x;
        ystart = y;
    }

#if  DEBUG
    pixCountPixels(pixt1, &iszero, NULL);
    fprintf(stderr, "Number of remaining pixels = %d\n", iszero);
    pixWrite("junkout7", pixt1, IFF_PNG);
#endif  /* DEBUG */

	/* Remove old boxa of pixa and replace with a clone copy */
    boxaDestroy(&pixa->boxa);
    pixa->boxa = boxaCopy(boxa, L_CLONE);

	/* Cleanup, freeing the fillsegs on each stack */
    stackDestroy(&stack, TRUE);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    return boxa;
}


/*!
 *  pixConnCompBB()
 *
 *      Input:  pixs (1 bpp)
 *              connectivity (4 or 8)
 *      Return: boxa, or null on error
 *
 * Notes:
 *     (1) Finds bounding boxes of 4- or 8-connected components
 *         in a binary image.
 *     (2) This works on a copy of the input pix.  The c.c. are located
 *         in raster order and erased one at a time.  In the process,
 *         the b.b. is computed and saved.
 */
BOXA *
pixConnCompBB(PIX     *pixs,
	      l_int32  connectivity)
{
l_int32  h, iszero;
l_int32  x, y, xstart, ystart;
PIX     *pixt;
BOX     *box;
BOXA    *boxa;
STACK   *stack, *auxstack;

    PROCNAME("pixConnCompBB");

    if (!pixs)
        return (BOXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (BOXA *)ERROR_PTR("pixs not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (BOXA *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    pixZero(pixs, &iszero);
    if (iszero)
        return boxaCreate(1);  /* return empty boxa */

    if ((pixt = pixCopy(NULL, pixs)) == NULL)
        return (BOXA *)ERROR_PTR("pixt not made", procName, NULL);

    h = pixGetHeight(pixs);
    if ((stack = stackCreate(h)) == NULL)
        return (BOXA *)ERROR_PTR("stack not made", procName, NULL);
    if ((auxstack = stackCreate(0)) == NULL)
        return (BOXA *)ERROR_PTR("auxstack not made", procName, NULL);
    stack->auxstack = auxstack;
    if ((boxa = boxaCreate(0)) == NULL)
	return (BOXA *)ERROR_PTR("boxa not made", procName, NULL);

    xstart = 0;
    ystart = 0;
    while (1)
    {
        if (!nextOnPixelInRaster(pixt, xstart, ystart, &x, &y))
            break;

        if ((box = pixSeedfillBB(pixt, stack, x, y, connectivity)) == NULL)
            return (BOXA *)ERROR_PTR("box not made", procName, NULL);
	boxaAddBox(boxa, box, L_INSERT);

        xstart = x;
        ystart = y;
    }

#if  DEBUG
    pixCountPixels(pixt, &iszero, NULL);
    fprintf(stderr, "Number of remaining pixels = %d\n", iszero);
    pixWrite("junkout7", pixt, IFF_PNG);
#endif  /* DEBUG */

	/* Cleanup, freeing the fillsegs on each stack */
    stackDestroy(&stack, TRUE);
    pixDestroy(&pixt);

    return boxa;
}


/*!
 *  nextOnPixelInRaster()
 *
 *      Input:  pixs (1 bpp)
 *              xstart, ystart  (starting point for search)
 *              &x, &y  (<return> coord value of next ON pixel)
 *      Return: 1 if a pixel is found; 0 otherwise or on error
 */
l_int32
nextOnPixelInRaster(PIX      *pix,
	            l_int32   xstart,
	            l_int32   ystart,
	            l_int32  *px,
	            l_int32  *py)
{
l_int32    w, h, wpl;
l_uint32  *data;

    PROCNAME("nextOnPixelInRaster");

    if (!pix)
	return ERROR_INT("pixs not defined", procName, 0);
    if (pixGetDepth(pix) != 1)
        return ERROR_INT("pix not 1 bpp", procName, 0);

    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    wpl = pixGetWpl(pix);
    data = pixGetData(pix);

    return nextOnPixelInRasterLow(data, w, h, wpl, xstart, ystart, px, py);
}


l_int32
nextOnPixelInRasterLow(l_uint32  *data,
                       l_int32    w,
                       l_int32    h,
                       l_int32    wpl,
                       l_int32    xstart,
                       l_int32    ystart,
                       l_int32   *px,
                       l_int32   *py)
{
l_int32    i, x, y, xend, startword;
l_uint32  *line, *pword;

	/* Look at the first word */
    line = data + ystart * wpl;
    pword = line + (xstart / 32);
    if (*pword) {
	xend = xstart - (xstart % 32) + 31;
	for (x = xstart; x <= xend && x < w; x++) {
	    if (GET_DATA_BIT(line, x)) {
		*px = x;
		*py = ystart;
		return 1;
	    }
	}
    }

	/* Continue with the rest of the line */
    startword = (xstart / 32) + 1;
    x = 32 * startword;
    for (pword = line + startword; x < w; pword++, x += 32) {
	if (*pword) {
	    for (i = 0; i < 32 && x < w; i++, x++) {
		if (GET_DATA_BIT(line, x)) {
		    *px = x;
		    *py = ystart;
		    return 1;
		}
	    }
	}
    }

	/* Continue with following lines */
    for (y = ystart + 1; y < h; y++) {
	line = data + y * wpl;
	for (pword = line, x = 0; x < w; pword++, x += 32) {
	    if (*pword) {
		for (i = 0; i < 32 && x < w; i++, x++) {
		    if (GET_DATA_BIT(line, x)) {
			*px = x;
			*py = y;
			return 1;
		    }
		}
	    }
	}
    }

    return 0;
}


/*!
 *  pixSeedfillBB()
 *
 *      Input:  pixs (1 bpp)
 *              stack (for holding fillsegs)
 *              x,y   (location of seed pixel)
 *              connectivity  (4 or 8)
 *      Return: box or null on error
 *
 *  Notes:
 *      (1) This is the high-level interface to Paul Heckbert's
 *          stack-based seedfill algorithm.
 */
BOX *
pixSeedfillBB(PIX     *pix,
	      STACK   *stack,
	      l_int32  x,
	      l_int32  y,
	      l_int32  connectivity)
{
BOX  *box;

    PROCNAME("pixSeedfillBB");

    if (!pix)
        return (BOX *)ERROR_PTR("pix not defined", procName, NULL);
    if (pixGetDepth(pix) != 1)
        return (BOX *)ERROR_PTR("pix not 1 bpp", procName, NULL);
    if (!stack)
        return (BOX *)ERROR_PTR("stack not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (BOX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    if (connectivity == 4) {
        if ((box = pixSeedfill4BB(pix, stack, x, y)) == NULL)
	    return (BOX *)ERROR_PTR("box not made", procName, NULL);
    }
    else if (connectivity == 8) {
        if ((box = pixSeedfill8BB(pix, stack, x, y)) == NULL)
	    return (BOX *)ERROR_PTR("box not made", procName, NULL);
    }
    else
        return (BOX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    return box;
}


/*!
 *  pixSeedfill4BB()
 *
 *      Input:  pixs (1 bpp)
 *              stack (for holding fillsegs)
 *              x,y   (location of seed pixel)
 *      Return: box or null on error.
 *
 *  Notes:
 *      (1) This is Paul Heckbert's stack-based 4-cc seedfill algorithm.
 *      (2) This operates on the input 1 bpp pix to remove the fg seed
 *          pixel, at (x,y), and all pixels that are 4-connected to it.
 *          The seed pixel at (x,y) must initially be ON.
 *      (3) Returns the bounding box of the erased 4-cc component.
 *      (4) Reference: see Paul Heckbert's stack-based seed fill algorithm
 *          in "Graphic Gems", ed. Andrew Glassner, Academic
 *          Press, 1990.  The algorithm description is given
 *          on pp. 275-277; working C code is on pp. 721-722.)
 *          The code here follows Heckbert's exactly, except
 *          we use function calls instead of macros for
 *          pushing data on and popping data off the stack.
 *          This makes sense to do because Heckbert's fixed-size
 *          stack with macros is dangerous: images exist that
 *          will overrun the stack and crash.   The stack utility
 *          here grows dynamically as needed, and the fillseg
 *          structures that are not in use are stored in another
 *          stack for reuse.  It should be noted that the 
 *          overhead in the function calls (vs. macros) is negligible.
 */
BOX *
pixSeedfill4BB(PIX     *pix,
	       STACK   *stack,
	       l_int32  x,
	       l_int32  y)
{
l_int32    xstart, wpl, x1, x2, dy;
l_int32    xmax, ymax;
l_int32    minx, maxx, miny, maxy;  /* for bounding box of this c.c. */
l_uint32  *data, *line;
BOX       *box;

    PROCNAME("pixSeedfill4BB");

    if (!pix)
        return (BOX *)ERROR_PTR("pix not defined", procName, NULL);
    if (pixGetDepth(pix) != 1)
        return (BOX *)ERROR_PTR("pix not 1 bpp", procName, NULL);
    if (!stack)
        return (BOX *)ERROR_PTR("stack not defined", procName, NULL);

    xmax = pixGetWidth(pix) - 1;
    ymax = pixGetHeight(pix) - 1;

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    line = data + y * wpl;

	/* Check pix value of seed; must be within the image and ON */
    if (x < 0 || x > xmax || y < 0 || y > ymax || (GET_DATA_BIT(line, x) == 0))
        return NULL;

        /* Init stack to seed:
	 * Must first init b.b. values to prevent valgrind from complaining;
	 * then init b.b. boundaries correctly to seed.  */
    minx = miny = 100000;
    maxx = maxy = 0;
    pushFillsegBB(stack, x, x, y, 1, ymax, &minx, &maxx, &miny, &maxy);
    pushFillsegBB(stack, x, x, y + 1, -1, ymax, &minx, &maxx, &miny, &maxy);
    minx = maxx = x;
    miny = maxy = y;

    while (stack->n > 0)
    {
            /* Pop segment off stack and fill a neighboring scan line */
	popFillsegBB(stack, &x1, &x2, &y, &dy);
        line = data + y * wpl;

            /* A segment of scanline y - dy for x1 <= x <= x2 was
             * previously filled.  We now explore adjacent pixels 
	     * in scan line y.  There are three regions: to the
	     * left of x1 - 1, between x1 and x2, and to the right of x2.
	     * These regions are handled differently.  Leaks are
	     * possible expansions beyond the previous segment and
	     * going back in the -dy direction.  These can happen 
	     * for x < x1 - 1 and for x > x2 + 1.  Any "leak" segments
	     * are plugged with a push in the -dy (opposite) direction.
	     * And any segments found anywhere are always extended
	     * in the +dy direction.  */
	for (x = x1; x >= 0 && (GET_DATA_BIT(line, x) == 1); x--)
	    CLEAR_DATA_BIT(line,x);
	if (x >= x1)  /* pix at x1 was off and was not cleared */
	    goto skip;
	xstart = x + 1;
	if (xstart < x1 - 1)   /* leak on left? */
	    pushFillsegBB(stack, xstart, x1 - 1, y, -dy,
	                  ymax, &minx, &maxx, &miny, &maxy);

	x = x1 + 1;
	do {
	    for (; x <= xmax && (GET_DATA_BIT(line, x) == 1); x++)
		CLEAR_DATA_BIT(line, x);
	    pushFillsegBB(stack, xstart, x - 1, y, dy,
	                  ymax, &minx, &maxx, &miny, &maxy);
	    if (x > x2 + 1)   /* leak on right? */
		pushFillsegBB(stack, x2 + 1, x - 1, y, -dy,
			      ymax, &minx, &maxx, &miny, &maxy);
    skip:   for (x++; x <= x2 &&
                      x <= xmax &&
                      (GET_DATA_BIT(line, x) == 0); x++)
		;
	    xstart = x;
	} while (x <= x2 && x <= xmax);
    }

    if ((box = boxCreate(minx, miny, maxx - minx + 1, maxy - miny + 1))
	    == NULL)
        return (BOX *)ERROR_PTR("box not made", procName, NULL);
    return box;
}
	    

/*!
 *  pixSeedfill8BB()
 *
 *      Input:  pixs (1 bpp)
 *              stack (for holding fillsegs)
 *              x,y   (location of seed pixel)
 *      Return: box or null on error.
 *
 *  Notes:
 *      (1) This is Paul Heckbert's stack-based 8-cc seedfill algorithm.
 *      (2) This operates on the input 1 bpp pix to remove the fg seed
 *          pixel, at (x,y), and all pixels that are 8-connected to it.
 *          The seed pixel at (x,y) must initially be ON.
 *      (3) Returns the bounding box of the erased 8-cc component.
 *      (4) Reference: see Paul Heckbert's stack-based seed fill algorithm
 *          in "Graphic Gems", ed. Andrew Glassner, Academic
 *          Press, 1990.  The algorithm description is given
 *          on pp. 275-277; working C code is on pp. 721-722.)
 *          The code here follows Heckbert's closely, except
 *          the leak checks are changed for 8 connectivity.
 *          See comments on pixSeedfill4BB() for more details.
 */
BOX *
pixSeedfill8BB(PIX     *pix,
	       STACK   *stack,
	       l_int32  x,
	       l_int32  y)
{
l_int32    xstart, wpl, x1, x2, dy;
l_int32    xmax, ymax;
l_int32    minx, maxx, miny, maxy;  /* for bounding box of this c.c. */
l_uint32  *data, *line;
BOX       *box;

    PROCNAME("pixSeedfill8BB");

    if (!pix)
        return (BOX *)ERROR_PTR("pix not defined", procName, NULL);
    if (pixGetDepth(pix) != 1)
        return (BOX *)ERROR_PTR("pix not 1 bpp", procName, NULL);
    if (!stack)
        return (BOX *)ERROR_PTR("stack not defined", procName, NULL);

    xmax = pixGetWidth(pix) - 1;
    ymax = pixGetHeight(pix) - 1;

    data = pixGetData(pix);
    wpl = pixGetWpl(pix);
    line = data + y * wpl;

	/* Check pix value of seed; must be ON */
    if (x < 0 || x > xmax || y < 0 || y > ymax || (GET_DATA_BIT(line, x) == 0))
        return NULL;

        /* Init stack to seed:
	 * Must first init b.b. values to prevent valgrind from complaining;
	 * then init b.b. boundaries correctly to seed.  */
    minx = miny = 100000;
    maxx = maxy = 0;
    pushFillsegBB(stack, x, x, y, 1, ymax, &minx, &maxx, &miny, &maxy);
    pushFillsegBB(stack, x, x, y + 1, -1, ymax, &minx, &maxx, &miny, &maxy);
    minx = maxx = x;
    miny = maxy = y;

    while (stack->n > 0)
    {
            /* Pop segment off stack and fill a neighboring scan line */
	popFillsegBB(stack, &x1, &x2, &y, &dy);
        line = data + y * wpl;

            /* A segment of scanline y - dy for x1 <= x <= x2 was
             * previously filled.  We now explore adjacent pixels 
	     * in scan line y.  There are three regions: to the
	     * left of x1, between x1 and x2, and to the right of x2.
	     * These regions are handled differently.  Leaks are
	     * possible expansions beyond the previous segment and
	     * going back in the -dy direction.  These can happen 
	     * for x < x1 and for x > x2.  Any "leak" segments
	     * are plugged with a push in the -dy (opposite) direction.
	     * And any segments found anywhere are always extended
	     * in the +dy direction.  */
	for (x = x1 - 1; x >= 0 && (GET_DATA_BIT(line, x) == 1); x--)
	    CLEAR_DATA_BIT(line,x);
	if (x >= x1 - 1)  /* pix at x1 - 1 was off and was not cleared */
	    goto skip;
	xstart = x + 1;
	if (xstart < x1)   /* leak on left? */
	    pushFillsegBB(stack, xstart, x1 - 1, y, -dy,
	                  ymax, &minx, &maxx, &miny, &maxy);

	x = x1;
	do {
	    for (; x <= xmax && (GET_DATA_BIT(line, x) == 1); x++)
		CLEAR_DATA_BIT(line, x);
	    pushFillsegBB(stack, xstart, x - 1, y, dy,
	                  ymax, &minx, &maxx, &miny, &maxy);
	    if (x > x2)   /* leak on right? */
		pushFillsegBB(stack, x2 + 1, x - 1, y, -dy,
			      ymax, &minx, &maxx, &miny, &maxy);
    skip:   for (x++; x <= x2 + 1 && 
                      x <= xmax &&
                      (GET_DATA_BIT(line, x) == 0); x++)
		;
	    xstart = x;
	} while (x <= x2 + 1 && x <= xmax);
    }

    if ((box = boxCreate(minx, miny, maxx - minx + 1, maxy - miny + 1))
	    == NULL)
        return (BOX *)ERROR_PTR("box not made", procName, NULL);
    return box;
}
	    


/*-----------------------------------------------------------------------*
 *               Stack helper functions: push and pop fillsegs           *
 *-----------------------------------------------------------------------*/
/*!
 *  pushFillsegBB()
 *
 *      Input:  stack
 *              xleft, xright
 *              y
 *              dy
 *              ymax,
 *              &minx (<return>)
 *              &maxx (<return>)
 *              &miny (<return>)
 *              &maxy (<return>)
 *      Return: void
 *
 *  Notes:
 *      (1) This adds a line segment to the stack, and returns its size.
 *      (2) The auxiliary stack is used as a storage area to recycle
 *          fillsegs that are no longer in use.  We only calloc new
 *          fillsegs if the auxiliary stack is empty.
 */
void
pushFillsegBB(STACK    *stack,
              l_int32   xleft,
              l_int32   xright,
              l_int32   y,
	      l_int32   dy,
	      l_int32   ymax,
	      l_int32  *pminx,
	      l_int32  *pmaxx,
	      l_int32  *pminy,
	      l_int32  *pmaxy)
{
FILLSEG  *fseg;
STACK    *auxstack;

    PROCNAME("pushFillsegBB");

    if (!stack)
	return ERROR_VOID(procName, "stack not defined");

    *pminx = L_MIN(*pminx, xleft);
    *pmaxx = L_MAX(*pmaxx, xright);
    *pminy = L_MIN(*pminy, y);
    *pmaxy = L_MAX(*pmaxy, y);

    if (y + dy >= 0 && y + dy <= ymax) {
	if ((auxstack = stack->auxstack) == NULL)
	    return ERROR_VOID("auxstack not defined", procName);

	    /* Get a fillseg to use */
	if (auxstack->n > 0)
	    fseg = (FILLSEG *)stackRemove(auxstack);
	else {
	    if ((fseg = (FILLSEG *)calloc(1, sizeof(FILLSEG))) == NULL)
		return ERROR_VOID("fillseg not made", procName);
	}

	fseg->xleft = xleft;
	fseg->xright = xright;
	fseg->y = y;
	fseg->dy = dy;
	stackAdd(stack, fseg);
    }
    return;
}


/*!
 *  popFillsegBB()
 * 
 *      Input:  stack
 *              &xleft (<return>)
 *              &xright (<return>)
 *              &y (<return>)
 *              &dy (<return>)
 *      Return: void
 *
 *  Notes:
 *      (1) This removes a line segment from the stack, and returns
 *          its size.
 *      (2) The surplussed fillseg is placed on the auxiliary stack
 *          for future use.
 */
void
popFillsegBB(STACK    *stack,
             l_int32  *pxleft,
             l_int32  *pxright,
             l_int32  *py,
             l_int32  *pdy)
{
FILLSEG  *fseg;
STACK    *auxstack;

    PROCNAME("popFillsegBB");

    if (!stack)
	return ERROR_VOID("stack not defined", procName);
    if ((auxstack = stack->auxstack) == NULL)
	return ERROR_VOID("auxstack not defined", procName);

    if ((fseg = (FILLSEG *)stackRemove(stack)) == NULL)
	return;

    *pxleft = fseg->xleft;
    *pxright = fseg->xright;
    *py = fseg->y + fseg->dy;  /* this now points to the new line */
    *pdy = fseg->dy;

	/* Save it for re-use */
    stackAdd(auxstack, fseg);
    return;
}

