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
 *   pixtiling.c
 *
 *        PIXTILING       *pixTilingCreate()
 *        void            *pixTilingDestroy()
 *        l_int32          pixTilingGetCount()
 *        l_int32          pixTilingGetSize()
 *        PIX             *pixTilingGetTile()
 *        l_int32          pixTilingPaintTile()
 *          
 *
 *   This provides a simple way to split an image into tiles
 *   and to perform operations independently on each tile.
 *
 *   The tile created with pixTilingGetTile() can have pixels in
 *   adjacent tiles for computation.  The number of extra pixels
 *   on each side of the tile is given by an 'overlap' parameter
 *   to pixTilingCreate().  For tiles at the boundary of
 *   the input image, quasi-overlap pixels are created by reflection
 *   symmetry into the tile.
 *
 *   Here's a typical intended usage.  Suppose you want to parallelize
 *   the operation on an image, by operating on tiles.  For each
 *   tile, you want to generate an in-place image result at the same
 *   resolution.  Suppose you choose a one-dimensional vertical tiling,
 *   where the desired tile width is 256 pixels and the overlap is
 *   30 pixels on each side:
 *
 *     PIX *pixd = pixCreateTemplateNoInit(pixs);  // output
 *     PIXTILING  *pt = pixTilingCreate(pixs, 0, 1, 256, 0, 30);
 *     pixTilingGetCount(pt, &nx, NULL);
 *     for (j = 0; j < nx; j++) {
 *         PIX *pixt = pixTilingGetTile(pt, 0, j);
 *         SomeInPlaceOperation(pixt, overlap, ...);
 *         pixTilingPaintTile(pixd, 0, j, pixt, pt);
 *         pixDestroy(&pixt);
 *     }
 *
 *   Note the 'overlap' parameter in your function.  You use that to
 *   change only the pixels that are not in the overlap region, because
 *   those are the pixels that will be painted into the destination.
 *   This also allows you to implement the function without worrying
 *   about pixels that are influenced by the image boundary.
 *   Note also that the tiles are labeled by (i, j) = (row, column),
 *   and in this example there is one row and nx columns.
 */

#include <stdio.h>
#include <stdlib.h> 
#include "allheaders.h"


/*!
 *  pixTilingCreate()
 *
 *      Input:  pixs  (pix to be tiled; any depth; colormap OK)
 *              nx    (number of tiles across image)
 *              ny    (number of tiles down image)
 *              w     (desired width of each tile)
 *              h     (desired height of each tile)
 *              overlap (amount of overlap into neighboring tile on each side)
 *      Return: pixtiling, or null on error
 *
 *  Notes:
 *      (1) We put a clone of pixs in the PixTiling.
 *      (2) The input to pixTilingCreate() for horizontal tiling can be
 *          either the number of tiles across the image or the approximate
 *          width of the tiles.  If the latter, the actual width will be
 *          determined by making all tiles but the last of equal width, and
 *          making the last as close to the others as possible.  The same
 *          consideration is applied independently to the vertical tiling.
 *          To specify tile width, set nx = 0; to specify the number of
 *          tiles horizontally across the image, set w = 0.
 *      (3) If pixs is to be tiled in one-dimensional strips, use ny = 1 for
 *          vertical strips and nx = 1 for horizontal strips.
 *      (4) The overlap must not be larger than the width or height of
 *          the leftmost or bottommost tile(s).
 */
PIXTILING *
pixTilingCreate(PIX     *pixs,
		l_int32  nx,
		l_int32  ny,
		l_int32  w,
		l_int32  h,
		l_int32  overlap)
{
l_int32     width, height;
PIXTILING  *pt;

    PROCNAME("pixTilingCreate");

    if (!pixs)
        return (PIXTILING *)ERROR_PTR("pixs not defined", procName, NULL);
    if (nx < 1 && w < 1)
        return (PIXTILING *)ERROR_PTR("invalid width spec", procName, NULL);
    if (ny < 1 && h < 1)
        return (PIXTILING *)ERROR_PTR("invalid height spec", procName, NULL);

        /* Find the tile width and number of tiles.  All tiles except the
         * rightmost ones have the same width.  The width of the
         * rightmost ones are at least the width of the others and
         * less than twice that width.  Ditto for tile height. */
    pixGetDimensions(pixs, &width, &height, NULL);
    if (nx == 0)
        nx = L_MAX(1, width / w);
    w = width / nx;  /* possibly reset */
    if (ny == 0)
        ny = L_MAX(1, height / h);
    h = height / ny;  /* possibly reset */
    if (overlap > w || overlap > h) {
        L_INFO_INT2("tile width = %d, tile height = %d", procName, w, h);
        return (PIXTILING *)ERROR_PTR("overlap too large", procName, NULL);
    }

    if ((pt = (PIXTILING *)CALLOC(1, sizeof(PIXTILING))) == NULL)
        return (PIXTILING *)ERROR_PTR("pt not made", procName, NULL);
    pt->pix = pixClone(pixs);
    pt->overlap = overlap;
    pt->nx = nx;
    pt->ny = ny;
    pt->w = w;
    pt->h = h;
    return pt;
}
        

/*!
 *  pixTilingDestroy()
 *
 *      Input:  &pt (<will be set to null before returning>)
 *      Return: void
 */
void
pixTilingDestroy(PIXTILING  **ppt)
{
PIXTILING  *pt;

    PROCNAME("pixTilingDestroy");

    if (ppt == NULL) {
        L_WARNING("ptr address is null!", procName);
        return;
    }

    if ((pt = *ppt) == NULL)
        return;

    pixDestroy(&pt->pix);
    FREE(pt);
    *ppt = NULL;
    return;
}


/*!
 *  pixTilingGetCount()
 *
 *      Input:  pt (pixtiling)
 *              &nx (<optional return> nx; can be null)
 *              &ny (<optional return> ny; can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixTilingGetCount(PIXTILING  *pt,
                  l_int32    *pnx,
                  l_int32    *pny)
{
    PROCNAME("pixTilingGetCount");

    if (!pt)
        return ERROR_INT("pt not defined", procName, 1);
    if (pnx) *pnx = pt->nx;
    if (pny) *pny = pt->ny;
    return 0;
}


/*!
 *  pixTilingGetSize()
 *
 *      Input:  pt (pixtiling)
 *              &w (<optional return> tile width; can be null)
 *              &h (<optional return> tile height; can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixTilingGetSize(PIXTILING  *pt,
                 l_int32    *pw,
                 l_int32    *ph)
{
    PROCNAME("pixTilingGetSize");

    if (!pt)
        return ERROR_INT("pt not defined", procName, 1);
    if (pw) *pw = pt->w;
    if (ph) *ph = pt->h;
    return 0;
}


/*!
 *  pixTilingGetTile()
 *
 *      Input:  pt (pixtiling)
 *              i (tile row index)
 *              j (tile column index)
 *      Return: pixd (tile with appropriate boundary (overlap) pixels),
 *                    or null on error
 */
PIX *
pixTilingGetTile(PIXTILING  *pt,
                 l_int32     i,
                 l_int32     j)
{
l_int32  wpix, hpix, wt, ht, nx, ny, overlap, wtlast, htlast;
l_int32  left, top, xtraleft, xtraright, xtratop, xtrabot, width, height;
BOX     *box;
PIX     *pixs, *pixt, *pixd;

    PROCNAME("pixTilingGetTile");

    if (!pt)
        return (PIX *)ERROR_PTR("pt not defined", procName, NULL);
    if ((pixs = pt->pix) == NULL)
        return (PIX *)ERROR_PTR("pix not found", procName, NULL);
    pixTilingGetCount(pt, &nx, &ny);
    if (i < 0 || i >= ny)
        return (PIX *)ERROR_PTR("invalid row index i", procName, NULL);
    if (j < 0 || j >= nx)
        return (PIX *)ERROR_PTR("invalid column index j", procName, NULL);

        /* Grab the tile with as much overlap as exists within the
	 * input pix.   First, compute the (left, top) coordinates.  */
    pixGetDimensions(pixs, &wpix, &hpix, NULL);
    pixTilingGetSize(pt, &wt, &ht);
    overlap = pt->overlap;
    wtlast = wpix - wt * (nx - 1);
    htlast = hpix - ht * (ny - 1);
    left = L_MAX(0, j * wt - overlap);
    top = L_MAX(0, i * ht - overlap);

        /* Get the width and height of the tile, including whatever
	 * overlap is available. */
    if (nx == 1)
        width = wpix;
    else if (j == 0)
        width = wt + overlap;
    else if (j == nx - 1)
        width = wtlast + overlap;
    else
        width = wt + 2 * overlap;

    if (ny == 1)
        height = hpix;
    else if (i == 0)
        height = ht + overlap;
    else if (i == ny - 1)
        height = htlast + overlap;
    else
        height = ht + 2 * overlap;
    box = boxCreate(left, top, width, height);
    pixt = pixClipRectangle(pixs, box, NULL);
    boxDestroy(&box);

       /* Add overlap as a mirrored border, in the 8 special cases where
	* the tile touches the border of the input pix.  The xtratop (etc)
	* parameters are required where the tile is either full width
	* or full height.  */
    xtratop = xtrabot = xtraleft = xtraright = 0;
    if (nx == 1)
        xtraleft = xtraright = overlap;
    if (ny == 1)
        xtratop = xtrabot = overlap;
    if (i == 0 && j == 0)
        pixd = pixAddMirroredBorder(pixt, overlap, xtraright, overlap, xtrabot);
    else if (i == 0 && j == nx - 1)
        pixd = pixAddMirroredBorder(pixt, xtraleft, overlap, overlap, xtrabot);
    else if (i == ny - 1 && j == 0)
        pixd = pixAddMirroredBorder(pixt, overlap, xtraright, xtratop, overlap);
    else if (i == ny - 1 && j == nx - 1)
        pixd = pixAddMirroredBorder(pixt, xtraleft, overlap, xtratop, overlap);
    else if (i == 0)
        pixd = pixAddMirroredBorder(pixt, 0, 0, overlap, xtrabot);
    else if (i == ny - 1)
        pixd = pixAddMirroredBorder(pixt, 0, 0, xtratop, overlap);
    else if (j == 0)
        pixd = pixAddMirroredBorder(pixt, overlap, xtraright, 0, 0);
    else if (j == nx - 1)
        pixd = pixAddMirroredBorder(pixt, xtraleft, overlap, 0, 0);
    else
        pixd = pixClone(pixt); 
    pixDestroy(&pixt);

    return pixd;
}


/*!
 *  pixTilingPaintTile()
 *
 *      Input:  pixd (dest: paint tile onto this, without overlap)
 *              i (tile row index)
 *              j (tile column index)
 *              pixs (source: tile to be painted from)
 *              pt (pixtiling struct)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixTilingPaintTile(PIX        *pixd,
                   l_int32     i,
                   l_int32     j,
                   PIX        *pixs,
                   PIXTILING  *pt)
{
l_int32  overlap, width, height;

    PROCNAME("pixTilingPaintTile");

    if (!pixd)
        return ERROR_INT("pixd not defined", procName, 1);
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pt)
        return ERROR_INT("pt not defined", procName, 1);
    overlap = pt->overlap;
    if (i < 0 || i >= pt->ny)
        return ERROR_INT("invalid row index i", procName, 1);
    if (j < 0 || j >= pt->nx)
        return ERROR_INT("invalid column index j", procName, 1);

    width = pixGetWidth(pixs) - 2 * overlap;
    height = pixGetHeight(pixs) - 2 * overlap;
    pixRasterop(pixd, j * pt->w, i * pt->h, width, height, PIX_SRC,
                pixs, overlap, overlap);
    return 0;
}


