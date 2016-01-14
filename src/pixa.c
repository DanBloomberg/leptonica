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
 *   pixa.c
 *
 *      Pixa creation, destruction, copying
 *           PIXA     *pixaCreate()
 *           PIXA     *pixaCreateFromPix()
 *           PIXA     *pixaCreateFromBoxa()
 *           PIXA     *pixaSplitPix()
 *           void      pixaDestroy()
 *           PIXA     *pixaCopy()
 *
 *      Pixa addition
 *           l_int32   pixaAddPix()
 *           l_int32   pixaExtendArray()
 *           l_int32   pixaAddBox()
 *
 *      Pixa accessors
 *           l_int32   pixaGetCount()
 *           l_int32   pixaChangeRefcount()
 *           PIX      *pixaGetPix()
 *           l_int32   pixaGetPixDimensions()
 *           PIX      *pixaGetBoxa()
 *           l_int32   pixaGetBoxaCount()
 *           BOX      *pixaGetBox()
 *           l_int32   pixaGetBoxGeometry()
 *
 *      Pixa array modifiers
 *           l_int32   pixaReplacePix()
 *           l_int32   pixaInsertPix()
 *           l_int32   pixaRemovePix()
 *
 *      Pixa combination
 *           PIXA     *pixaJoin()
 *
 *      Other pixa functions and related pix functions
 *           PIXA     *pixaSort()
 *           PIXA     *pixaSortByIndex()
 *           PIXAA    *pixaSort2dByIndex()
 *           PIXA     *pixaaFlattenToPixa()
 *           l_int32   pixaSizeRange()
 *           PIX      *pixRemoveLargeComponents()
 *           PIX      *pixRemoveSmallComponents()
 *           PIXA     *pixaRemoveLargeComponents()
 *           PIXA     *pixaRemoveSmallComponents()
 *           PIXA     *pixaClipToPix()
 *
 *      Pixaa creation, destruction
 *           PIXAA    *pixaaCreate()
 *           void      pixaaDestroy()
 *
 *      Pixaa addition
 *           l_int32   pixaaAddPixa()
 *           l_int32   pixaaExtendArray()
 *
 *      Pixaa accessors
 *           l_int32   pixaaGetCount()
 *           PIXA     *pixaaGetPixa()
 *
 *      Pixa Display
 *           PIX      *pixaDisplay()
 *           PIX      *pixaDisplayRandomCmap()
 *           PIX      *pixaDisplayOnLattice()
 *           PIX      *pixaDisplayUnsplit()
 *           PIX      *pixaDisplayTiled()
 *           PIX      *pixaDisplayTiledAndScaled()
 *
 *      Pixaa Display
 *           PIX      *pixaaDisplay()
 *           PIX      *pixaaDisplayByPixa()
 *
 *      Pixa serialized I/O
 *           PIXA     *pixaRead()
 *           PIXA     *pixaReadStream()
 *           l_int32   pixaWrite()
 *           l_int32   pixaWriteStream()
 *
 *   
 *   Note: The reference counting for the Pixa is analogous
 *         to that for the Boxa.  See pix.h for details.
 *         pixaCopy() provides three possible modes of copy.
 *         The basic rule is that however a Pixa is obtained
 *         (e.g., from pixaCreate*(), pixaCopy(), or a Pixaa accessor),
 *         it is necessary to call pixaDestroy() on it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   /* for sqrt() */
#include "allheaders.h"

static const l_int32  INITIAL_PTR_ARRAYSIZE = 20;   /* n'import quoi */


/*---------------------------------------------------------------------*
 *                    Pixa creation, destruction, copy                 *
 *---------------------------------------------------------------------*/
/*!
 *  pixaCreate()
 *
 *      Input:  n  (initial number of ptrs)
 *      Return: pixa, or null on error
 */
PIXA *
pixaCreate(l_int32  n)
{
PIXA  *pixa;

    PROCNAME("pixaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((pixa = (PIXA *)CALLOC(1, sizeof(PIXA))) == NULL)
        return (PIXA *)ERROR_PTR("pixa not made", procName, NULL);
    pixa->n = 0;
    pixa->nalloc = n;
    pixa->refcount = 1;
    
    if ((pixa->pix = (PIX **)CALLOC(n, sizeof(PIX *))) == NULL)
        return (PIXA *)ERROR_PTR("pix ptrs not made", procName, NULL);

    if ((pixa->boxa = boxaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("boxa not made", procName, NULL);

    return pixa;
}


/*!
 *  pixaCreateFromPix()
 *
 *      Input:  pixs  (with individual components on a lattice)
 *              n   (number of components)
 *              cellw   (width of each cell)
 *              cellh   (height of each cell)
 *      Return: pixa, or null on error
 *
 *  Note: for bpp = 1, we truncate each retrieved pix to
 *        the ON pixels, which we assume for now start at (0,0)
 */
PIXA *
pixaCreateFromPix(PIX     *pixs,
                  l_int32  n,
                  l_int32  cellw,
                  l_int32  cellh)
{
l_int32  w, h, d, nw, nh, i, j, index;
PIX     *pix, *pixt;
PIXA    *pixa;

    PROCNAME("pixaCreateFromPix");

    if (!pixs)
        return (PIXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (n <= 0)
        return (PIXA *)ERROR_PTR("n must be > 0", procName, NULL);

    if ((pixa = pixaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("pixa not made", procName, NULL);
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    d = pixGetDepth(pixs);
    if ((pixt = pixCreate(cellw, cellh, d)) == NULL)
        return (PIXA *)ERROR_PTR("pixt not made", procName, NULL);

    nw = (w + cellw - 1) / cellw;
    nh = (h + cellh - 1) / cellh;
    for (i = 0, index = 0; i < nh; i++) {
        for (j = 0; j < nw && index < n; j++, index++) {
            pixRasterop(pixt, 0, 0, cellw, cellh, PIX_SRC, pixs,
                   j * cellw, i * cellh);
            if (d == 1 && !pixClipToForeground(pixt, &pix, NULL))
                pixaAddPix(pixa, pix, L_INSERT);
            else
                pixaAddPix(pixa, pixt, L_COPY);
        }
    }

    pixDestroy(&pixt);
    return pixa;
}


/*!
 *  pixaCreateFromBoxa()
 *
 *      Input:  pixs
 *              boxa
 *              &cropwarn (<optional return> TRUE if the boxa extent
 *                         is larger than pixs.
 *      Return: pixad, or null on error
 *
 *  Notes:
 *      (1) This simply extracts from pixs the region corresponding to each
 *          box in the boxa.
 *      (2) The 3rd arg is optional.  If the extent of the boxa exceeds the
 *          size of the pixa, so that some boxes are either clipped
 *          or entirely outside the pix, a warning is returned as TRUE.
 *      (3) pixad will have only the properly clipped elements, and
 *          the internal boxa will be correct.
 */
PIXA *
pixaCreateFromBoxa(PIX      *pixs,
                   BOXA     *boxa,
                   l_int32  *pcropwarn)
{
l_int32  i, n, w, h, wbox, hbox, cropwarn;
BOX     *box, *boxc;
PIX     *pixd;
PIXA    *pixad;

    PROCNAME("pixaCreateFromBoxa");

    if (!pixs)
        return (PIXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!boxa)
        return (PIXA *)ERROR_PTR("boxa not defined", procName, NULL);

    n = boxaGetCount(boxa);
    if ((pixad = pixaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("pixad not made", procName, NULL);

    boxaGetExtent(boxa, &wbox, &hbox, NULL);
    pixGetDimensions(pixs, &w, &h, NULL);
    cropwarn = FALSE;
    if (wbox > w || hbox > h)
        cropwarn = TRUE;
    if (pcropwarn)
        *pcropwarn = cropwarn;

    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_COPY);
        if (cropwarn) {  /* if box is outside pixs, pixd is NULL */
            if (pixd = pixClipRectangle(pixs, box, &boxc)) {  /* may be NULL */
                pixaAddPix(pixad, pixd, L_INSERT);
                pixaAddBox(pixad, boxc, L_INSERT);
            }
            boxDestroy(&box);
        }
        else {
            pixd = pixClipRectangle(pixs, box, NULL);
            pixaAddPix(pixad, pixd, L_INSERT);
            pixaAddBox(pixad, box, L_INSERT);
        }
    }

    return pixad;
}
    

/*!
 *  pixaSplitPix()
 *
 *      Input:  pixs  (with individual components on a lattice)
 *              nx   (number of mosaic cells horizontally)
 *              ny   (number of mosaic cells vertically)
 *              borderwidth  (of added border on all sides)
 *              bordercolor  (in our RGBA format: 0xrrggbbaa)
 *      Return: pixa, or null on error
 *
 *  Notes:
 *      (1) This is a variant on pixaCreateFromPix(), where we
 *          simply divide the image up into (approximately) equal
 *          subunits.  If you want the subimages to have essentially
 *          the same aspect ratio as the input pix, use nx = ny.
 *      (2) If borderwidth is 0, we ignore the input bordercolor and
 *          redefine it to white.
 *      (3) The bordercolor is always used to initialize each tiled pix,
 *          so that if the src is clipped, the unblitted part will
 *          be this color.  This avoids 1 pixel wide black stripes at the
 *          left and lower edges.
 */
PIXA *
pixaSplitPix(PIX      *pixs,
             l_int32   nx,
             l_int32   ny,
             l_int32   borderwidth,
             l_uint32  bordercolor)
{
l_int32  w, h, d, cellw, cellh, i, j;
PIX     *pixt;
PIXA    *pixa;

    PROCNAME("pixaSplitPix");

    if (!pixs)
        return (PIXA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (nx <= 0 || ny <= 0)
        return (PIXA *)ERROR_PTR("nx and ny must be > 0", procName, NULL);
    borderwidth = L_MAX(0, borderwidth);

    if ((pixa = pixaCreate(nx * ny)) == NULL)
        return (PIXA *)ERROR_PTR("pixa not made", procName, NULL);
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    d = pixGetDepth(pixs);
    cellw = (w + nx - 1) / nx;  /* round up */
    cellh = (h + ny - 1) / ny;

    for (i = 0; i < ny; i++) {
        for (j = 0; j < nx; j++) {
            if ((pixt = pixCreate(cellw + 2 * borderwidth,
                                  cellh + 2 * borderwidth, d)) == NULL)
                return (PIXA *)ERROR_PTR("pixt not made", procName, NULL);
            pixCopyColormap(pixt, pixs);
            if (borderwidth == 0) {  /* initialize full image to white */
                if (d == 1)
                    pixClearAll(pixt);
                else
                    pixSetAll(pixt);
            }
            else
                pixSetAllArbitrary(pixt, bordercolor);
            pixRasterop(pixt, borderwidth, borderwidth, cellw, cellh,
                        PIX_SRC, pixs, j * cellw, i * cellh);
            pixaAddPix(pixa, pixt, L_INSERT);
        }
    }

    return pixa;
}


/*!
 *  pixaDestroy()
 *
 *      Input:  &pixa (<can be nulled>)
 *      Return: void
 *
 *  Notes:
 *      (1) Decrements the ref count and, if 0, destroys the pixa.
 *      (2) Always nulls the input ptr.
 */
void
pixaDestroy(PIXA  **ppixa)
{
l_int32  i;
PIXA    *pixa;

    PROCNAME("pixaDestroy");

    if (ppixa == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((pixa = *ppixa) == NULL)
        return;

        /* Decrement the refcount.  If it is 0, destroy the pixa. */
    pixaChangeRefcount(pixa, -1);
    if (pixa->refcount <= 0) {
        for (i = 0; i < pixa->n; i++)
            pixDestroy(&pixa->pix[i]);
        FREE(pixa->pix);
        boxaDestroy(&pixa->boxa);
        FREE(pixa);
    }

    *ppixa = NULL;
    return;
}


/*!
 *  pixaCopy()
 *
 *      Input:  pixas
 *              copyflag:
 *                L_COPY makes a new pixa and copies each pix and each box
 *                L_CLONE gives a new ref-counted handle to the input pixa
 *                L_COPY_CLONE makes a new pixa and inserts clones of
 *                    all pix and boxes
 *      Return: new pixa, or null on error
 *
 *  Note: see pix.h for description of the copy types.
 */
PIXA *
pixaCopy(PIXA    *pixa,
         l_int32  copyflag)
{
l_int32  i;
BOX     *boxc;
PIX     *pixc;
PIXA    *pixac;

    PROCNAME("pixaCopy");

    if (!pixa)
        return (PIXA *)ERROR_PTR("pixa not defined", procName, NULL);

    if (copyflag == L_CLONE) {
        pixaChangeRefcount(pixa, 1);
        return pixa;
    }

    if (copyflag != L_COPY && copyflag != L_COPY_CLONE)
        return (PIXA *)ERROR_PTR("invalid copyflag", procName, NULL);

    if ((pixac = pixaCreate(pixa->n)) == NULL)
        return (PIXA *)ERROR_PTR("pixac not made", procName, NULL);
    for (i = 0; i < pixa->n; i++) {
        if (copyflag == L_COPY) {
            pixc = pixaGetPix(pixa, i, L_COPY);
            boxc = pixaGetBox(pixa, i, L_COPY);
        }
        else {  /* copy-clone */
            pixc = pixaGetPix(pixa, i, L_CLONE);
            boxc = pixaGetBox(pixa, i, L_CLONE);
        }
        pixaAddPix(pixac, pixc, L_INSERT);
        pixaAddBox(pixac, boxc, L_INSERT);
    }

    return pixac;
}



/*---------------------------------------------------------------------*
 *                              Pixa addition                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaAddPix()
 *
 *      Input:  pixa
 *              pix  (to be added)
 *              copyflag (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixaAddPix(PIXA    *pixa,
           PIX     *pix,
           l_int32  copyflag)
{
l_int32  n;
PIX     *pixc;

    PROCNAME("pixaAddPix");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);

    if (copyflag == L_INSERT)
        pixc = pix;
    else if (copyflag == L_COPY)
        pixc = pixCopy(NULL, pix);
    else if (copyflag == L_CLONE)
        pixc = pixClone(pix);
    else
        return ERROR_INT("invalid copyflag", procName, 1);
    if (!pixc)
        return ERROR_INT("pixc not made", procName, 1);

    n = pixaGetCount(pixa);
    if (n >= pixa->nalloc)
        pixaExtendArray(pixa);
    pixa->pix[n] = pixc;
    pixa->n++;

    return 0;
}


/*!
 *  pixaExtendArray()
 *
 *      Input:  pixa
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) We extend the boxa array simultaneously.  This is
 *          necessary in case we are NOT adding boxes simultaneously
 *          with adding pix.  We always want the sizes of the
 *          pixa and boxa ptr arrays to be equal.
 */
l_int32
pixaExtendArray(PIXA  *pixa)
{
    PROCNAME("pixaExtendArray");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    if ((pixa->pix = (PIX **)reallocNew((void **)&pixa->pix,
                             sizeof(l_intptr_t) * pixa->nalloc,
                             2 * sizeof(l_intptr_t) * pixa->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);
    pixa->nalloc = 2 * pixa->nalloc;
    boxaExtendArray(pixa->boxa);
    return 0;
}


/*!
 *  pixaAddBox()
 *
 *      Input:  pixa
 *              box
 *              copyflag (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaAddBox(PIXA    *pixa,
           BOX     *box,
           l_int32  copyflag)
{
    PROCNAME("pixaAddBox");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (copyflag != L_INSERT && copyflag != L_COPY && copyflag != L_CLONE)
        return ERROR_INT("invalid copyflag", procName, 1);
    if (!pixa->boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    boxaAddBox(pixa->boxa, box, copyflag);
    return 0;
}



/*---------------------------------------------------------------------*
 *                             Pixa accessors                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaGetCount()
 *
 *      Input:  pixa
 *      Return: count, or 0 if no pixa
 */
l_int32
pixaGetCount(PIXA  *pixa)
{
    PROCNAME("pixaGetCount");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 0);

    return pixa->n;
}


/*!
 *  pixaChangeRefcount()
 *
 *      Input:  pixa
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaChangeRefcount(PIXA    *pixa,
                   l_int32  delta)
{
    PROCNAME("pixaChangeRefcount");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    pixa->refcount += delta;
    return 0;
}


/*!
 *  pixaGetPix()
 *
 *      Input:  pixa
 *              index  (to the index-th pix)
 *              accesstype  (L_COPY or L_CLONE)
 *      Return: pix, or null on error
 */
PIX *
pixaGetPix(PIXA    *pixa,
           l_int32  index,
           l_int32  accesstype)
{
    PROCNAME("pixaGetPix");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    if (index < 0 || index >= pixa->n)
        return (PIX *)ERROR_PTR("index not valid", procName, NULL);

    if (accesstype == L_COPY)
        return pixCopy(NULL, pixa->pix[index]);
    else if (accesstype == L_CLONE)
        return pixClone(pixa->pix[index]);
    else
        return (PIX *)ERROR_PTR("invalid accesstype", procName, NULL);
}


/*!
 *  pixaGetPixDimensions()
 *
 *      Input:  pixa
 *              index  (to the index-th box)
 *              &w, &h, &d (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaGetPixDimensions(PIXA     *pixa,
                     l_int32   index,
                     l_int32  *pw,
                     l_int32  *ph,
                     l_int32  *pd)
{
PIX  *pix;

    PROCNAME("pixaGetPixDimensions");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (index < 0 || index >= pixa->n)
        return ERROR_INT("index not valid", procName, 1);

    if ((pix = pixaGetPix(pixa, index, L_CLONE)) == NULL)
        return ERROR_INT("pix not found!", procName, 1);
    pixGetDimensions(pix, pw, ph, pd);
    pixDestroy(&pix);
    return 0;
}


/*!
 *  pixaGetBoxa()
 *
 *      Input:  pixa
 *              accesstype  (L_COPY, L_CLONE, L_COPY_CLONE)
 *      Return: boxa, or null on error
 */
BOXA *
pixaGetBoxa(PIXA    *pixa,
            l_int32  accesstype)
{
    PROCNAME("pixaGetBoxa");

    if (!pixa)
        return (BOXA *)ERROR_PTR("pixa not defined", procName, NULL);
    if (!pixa->boxa)
        return (BOXA *)ERROR_PTR("boxa not defined", procName, NULL);
    if (accesstype != L_COPY && accesstype != L_CLONE &&
        accesstype != L_COPY_CLONE)
        return (BOXA *)ERROR_PTR("invalid accesstype", procName, NULL);

    return boxaCopy(pixa->boxa, accesstype);
}


/*!
 *  pixaGetBoxaCount()
 *
 *      Input:  pixa
 *      Return: count, or 0 on error
 */
l_int32
pixaGetBoxaCount(PIXA  *pixa)
{
    PROCNAME("pixaGetBoxaCount");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 0);
    
    return boxaGetCount(pixa->boxa);
}


/*!
 *  pixaGetBox()
 *
 *      Input:  pixa
 *              index  (to the index-th pix)
 *              accesstype  (L_COPY or L_CLONE)
 *      Return: box (if null, not automatically an error), or null on error
 *
 *  Notes:
 *      (1) There is always a boxa with a pixa, and it is initialized so
 *          that each box ptr is NULL.
 *      (2) In general, we expect that there is either a box associated
 *          with each pix, or no boxes at all in the boxa.
 *      (3) Having no boxes is thus not an automatic error.  Whether it
 *          is an actual error is determined by the calling program.
 *          If the caller expects to get a box, it is an error; see, e.g.,
 *          pixaGetBoxGeometry().
 */
BOX *
pixaGetBox(PIXA    *pixa,
           l_int32  index,
           l_int32  accesstype)
{
BOX  *box;

    PROCNAME("pixaGetBox");

    if (!pixa)
        return (BOX *)ERROR_PTR("pixa not defined", procName, NULL);
    if (!pixa->boxa)
        return (BOX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (index < 0 || index >= pixa->boxa->n)
        return (BOX *)ERROR_PTR("index not valid", procName, NULL);
    if (accesstype != L_COPY && accesstype != L_CLONE)
        return (BOX *)ERROR_PTR("invalid accesstype", procName, NULL);

    box = pixa->boxa->box[index];
    if (box) {
        if (accesstype == L_COPY)
            return boxCopy(box);
        else  /* accesstype == L_CLONE */
            return boxClone(box);
    }
    else
        return NULL;
}


/*!
 *  pixaGetBoxGeometry()
 *
 *      Input:  pixa
 *              index  (to the index-th box)
 *              &x, &y, &w, &h (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaGetBoxGeometry(PIXA     *pixa,
                   l_int32   index,
                   l_int32  *px,
                   l_int32  *py,
                   l_int32  *pw,
                   l_int32  *ph)
{
BOX  *box;

    PROCNAME("pixaGetBoxGeometry");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (index < 0 || index >= pixa->n)
        return ERROR_INT("index not valid", procName, 1);

    if ((box = pixaGetBox(pixa, index, L_CLONE)) == NULL)
        return ERROR_INT("box not found!", procName, 1);
    boxGetGeometry(box, px, py, pw, ph);
    boxDestroy(&box);
    return 0;
}


/*---------------------------------------------------------------------*
 *                       Pixa array modifiers                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaReplacePix()
 *
 *      Input:  pixa
 *              index  (to the index-th pix)
 *              pix (insert to replace existing one)
 *              box (<optional> insert to replace existing)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      - In-place replacement of one pix
 *      - The previous pix at that location is destroyed
 */
l_int32
pixaReplacePix(PIXA    *pixa,
               l_int32  index,
               PIX     *pix,
               BOX     *box)
{
BOXA  *boxa;

    PROCNAME("pixaReplacePix");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (index < 0 || index >= pixa->n)
        return ERROR_INT("index not valid", procName, 1);
    if (!pix)
        return ERROR_INT("pix not defined", procName, 1);

    pixDestroy(&(pixa->pix[index]));
    pixa->pix[index] = pix;

    if (box) {
        boxa = pixa->boxa;
        if (index > boxa->n)
            return ERROR_INT("boxa index not valid", procName, 1);
        boxaReplaceBox(boxa, index, box);
    }

    return 0;
}


/*!
 *  pixaInsertPix()
 *
 *      Input:  pixa
 *              index (of pix to be removed)
 *              pixs (new pix to be inserted)
 *              box (<optional> new box to be inserted)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This shifts pixa[i] --> pixa[i + 1] for all i >= index,
 *          and then inserts at pixa[index].
 *      (2) To insert at the beginning of the array, set index = 0.
 *      (3) It should not be used repeatedly on large arrays,
 *          because the function is O(n).
 *      (4) To append a pix to a pixa, it's easier to use pixaAddPix().
 */
l_int32
pixaInsertPix(PIXA    *pixa,
              l_int32  index,
              PIX     *pixs,
              BOX     *box)
{
l_int32  i, n;

    PROCNAME("pixaInsertPix");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    n = pixaGetCount(pixa);
    if (index < 0 || index > n)
        return ERROR_INT("index not in {0...n}", procName, 1);
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);

    if (n >= pixa->nalloc)
        pixaExtendArray(pixa);
    pixa->n++;
    for (i = n; i > index; i--)
      pixa->pix[i] = pixa->pix[i - 1];
    pixa->pix[index] = pixs;

        /* Optionally, insert the box */
    if (box)
        boxaInsertBox(pixa->boxa, index, box);

    return 0;
}


/*!
 *  pixaRemovePix()
 *
 *      Input:  pixa
 *              index (of pix to be removed)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This shifts pixa[i] --> pixa[i - 1] for all i > index.
 *      (2) It should not be used repeatedly on large arrays,
 *          because the function is O(n).
 *      (3) The corresponding box is removed as well, if it exists.
 */
l_int32
pixaRemovePix(PIXA    *pixa,
              l_int32  index)
{
l_int32  i, n;
BOXA    *boxa;
PIX    **array;

    PROCNAME("pixaRemovePix");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    n = pixaGetCount(pixa);
    if (index < 0 || index >= n)
        return ERROR_INT("index not in {0...n - 1}", procName, 1);

        /* Remove the pix */
    array = pixa->pix;
    pixDestroy(&array[index]);
    for (i = index + 1; i < n; i++)
        array[i - 1] = array[i];
    array[n - 1] = NULL;
    pixa->n--;

        /* Remove the box if it exists */
    if ((boxa = pixa->boxa) != NULL)
        boxaRemoveBox(boxa, index);

    return 0;
}


/*---------------------------------------------------------------------*
 *                           Pixa combination                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaJoin()
 *
 *      Input:  pixad  (dest pixa; add to this one)
 *              pixas  (source pixa; add from this one)
 *              istart  (starting index in nas)
 *              iend  (ending index in nas; use 0 to cat all)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This appends a clone of each indicated pix in pixas to pixad
 *      (2) istart < 0 is taken to mean 'read from the start' (istart = 0)
 *      (3) iend <= 0 means 'read to the end'
 */
l_int32
pixaJoin(PIXA    *pixad,
         PIXA    *pixas,
         l_int32  istart,
         l_int32  iend)
{
l_int32  ns, i;
BOXA    *boxas, *boxad;
PIX     *pix;

    PROCNAME("pixaJoin");

    if (!pixad)
        return ERROR_INT("pixad not defined", procName, 1);
    if (!pixas)
        return ERROR_INT("pixas not defined", procName, 1);
    ns = pixaGetCount(pixas);
    if (istart < 0)
        istart = 0;
    if (istart >= ns)
        return ERROR_INT("istart out of bounds", procName, 1);
    if (iend <= 0)
        iend = ns - 1;
    if (iend >= ns)
        return ERROR_INT("iend out of bounds", procName, 1);
    if (istart > iend)
        return ERROR_INT("istart > iend; nothing to add", procName, 1);

    for (i = istart; i <= iend; i++) {
        pix = pixaGetPix(pixas, i, L_CLONE);
        pixaAddPix(pixad, pix, L_INSERT);
    }

    boxas = pixaGetBoxa(pixas, L_CLONE);
    boxad = pixaGetBoxa(pixad, L_CLONE);
    boxaJoin(boxad, boxas, 0, 0);
    boxaDestroy(&boxas);  /* just the clones */
    boxaDestroy(&boxad);

    return 0;
}



/*---------------------------------------------------------------------*
 *                         Other pixa functions                        *
 *---------------------------------------------------------------------*/
/*!
 *  pixaSort()
 *
 *      Input:  pixas
 *              sorttype (L_SORT_BY_X, L_SORT_BY_Y, L_SORT_BY_WIDTH,
 *                        L_SORT_BY_HEIGHT, L_SORT_BY_MIN_DIMENSION,
 *                        L_SORT_BY_MAX_DIMENSION, L_SORT_BY_PERIMETER,
 *                        L_SORT_BY_AREA)
 *              sortorder  (L_SORT_INCREASING, L_SORT_DECREASING)
 *              &naindex (<optional return> index of sorted order into
 *                        original array)
 *              copyflag (L_COPY, L_CLONE)
 *      Return: pixad (sorted version of pixas), or null on error
 *
 *  Notes:
 *      (1) This sorts based on the data in the boxa.  If the boxa
 *          count is not the same as the pixa count, this returns an error.
 *      (2) The copyflag refers to the pix and box copies that are
 *          inserted into the sorted pixa.  These are either L_COPY
 *          or L_CLONE.
 */
PIXA *
pixaSort(PIXA    *pixas,
         l_int32  sorttype,
         l_int32  sortorder,
         NUMA   **pnaindex,
         l_int32  copyflag)
{
l_int32  i, n, x, y, w, h, size;
BOXA    *boxa;
NUMA    *na, *naindex;
PIXA    *pixad;

    PROCNAME("pixaSort");

    if (!pixas)
        return (PIXA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (sorttype != L_SORT_BY_X && sorttype != L_SORT_BY_Y && 
        sorttype != L_SORT_BY_WIDTH && sorttype != L_SORT_BY_HEIGHT &&
        sorttype != L_SORT_BY_MIN_DIMENSION &&
        sorttype != L_SORT_BY_MAX_DIMENSION &&
        sorttype != L_SORT_BY_PERIMETER &&
        sorttype != L_SORT_BY_AREA)
        return (PIXA *)ERROR_PTR("invalid sort type", procName, NULL);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return (PIXA *)ERROR_PTR("invalid sort order", procName, NULL);
    if (copyflag != L_COPY && copyflag != L_CLONE)
        return (PIXA *)ERROR_PTR("invalid copy flag", procName, NULL);

    if ((boxa = pixas->boxa) == NULL)
        return (PIXA *)ERROR_PTR("boxa not found", procName, NULL);
    n = pixaGetCount(pixas);
    if (boxaGetCount(boxa) != n)
        return (PIXA *)ERROR_PTR("boxa and pixa counts differ", procName, NULL);

        /* Build up numa of specific data */
    if ((na = numaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("na not made", procName, NULL);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
        switch (sorttype)
        {
        case L_SORT_BY_X:
            numaAddNumber(na, x);
            break;
        case L_SORT_BY_Y:
            numaAddNumber(na, y);
            break;
        case L_SORT_BY_WIDTH:
            numaAddNumber(na, w);
            break;
        case L_SORT_BY_HEIGHT:
            numaAddNumber(na, h);
            break;
        case L_SORT_BY_MIN_DIMENSION:
            size = L_MIN(w, h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_MAX_DIMENSION:
            size = L_MAX(w, h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_PERIMETER:
            size = w + h;
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_AREA:
            size = w * h;
            numaAddNumber(na, size);
            break;
        default:
            L_WARNING("invalid sort type", procName);
        }
    }

        /* Get the sort index for data array */
    if ((naindex = numaGetSortIndex(na, sortorder)) == NULL)
        return (PIXA *)ERROR_PTR("naindex not made", procName, NULL);

        /* Build up sorted pixa using sort index */
    if ((pixad = pixaSortByIndex(pixas, naindex, copyflag)) == NULL)
        return (PIXA *)ERROR_PTR("pixad not made", procName, NULL);

    if (pnaindex)
        *pnaindex = naindex;
    else
        numaDestroy(&naindex);
    numaDestroy(&na);
    return pixad;
}


/*!
 *  pixaSortByIndex()
 * 
 *      Input:  pixas
 *              naindex (na that maps from the new pixa to the input pixa)
 *              copyflag (L_COPY, L_CLONE)
 *      Return: pixad (sorted), or null on error
 */
PIXA *
pixaSortByIndex(PIXA    *pixas,
                NUMA    *naindex,
                l_int32  copyflag)
{
l_int32  i, n, index;
BOX     *box;
PIX     *pix;
PIXA    *pixad;

    PROCNAME("pixaSortByIndex");

    if (!pixas)
        return (PIXA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (!naindex)
        return (PIXA *)ERROR_PTR("naindex not defined", procName, NULL);
    if (copyflag != L_CLONE && copyflag != L_COPY)
        return (PIXA *)ERROR_PTR("invalid copyflag", procName, NULL);

    n = pixaGetCount(pixas);
    pixad = pixaCreate(n);
    for (i = 0; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        pix = pixaGetPix(pixas, index, copyflag);
        box = pixaGetBox(pixas, index, copyflag);
        pixaAddPix(pixad, pix, L_INSERT);
        pixaAddBox(pixad, box, L_INSERT);
    }

    return pixad;
}


/*!
 *  pixaSort2dByIndex()
 * 
 *      Input:  pixas
 *              naa (numaa that maps from the new pixaa to the input pixas)
 *              copyflag (L_CLONE or L_COPY)
 *      Return: pixaa (sorted), or null on error
 */
PIXAA *
pixaSort2dByIndex(PIXA    *pixas,
                  NUMAA   *naa,
                  l_int32  copyflag)
{
l_int32  pixtot, ntot, i, j, n, nn, index;
BOX     *box;
NUMA    *na;
PIX     *pix;
PIXA    *pixa;
PIXAA   *pixaa;

    PROCNAME("pixaSort2dByIndex");

    if (!pixas)
        return (PIXAA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (!naa)
        return (PIXAA *)ERROR_PTR("naindex not defined", procName, NULL);

        /* Check counts */
    ntot = numaaGetNumberCount(naa);
    pixtot = pixaGetCount(pixas);
    if (ntot != pixtot)
        return (PIXAA *)ERROR_PTR("element count mismatch", procName, NULL);

    n = numaaGetCount(naa);
    pixaa = pixaaCreate(n);
    for (i = 0; i < n; i++) {
        na = numaaGetNuma(naa, i, L_CLONE);
        nn = numaGetCount(na);
        pixa = pixaCreate(nn);
        for (j = 0; j < nn; j++) {
            numaGetIValue(na, j, &index);
            pix = pixaGetPix(pixas, index, copyflag);
            box = pixaGetBox(pixas, index, copyflag);
            pixaAddPix(pixa, pix, L_INSERT);
            pixaAddBox(pixa, box, L_INSERT);
        }
        pixaaAddPixa(pixaa, pixa, L_INSERT);
        numaDestroy(&na);
    }

    return pixaa;
}


/*!
 *  pixaaFlattenToPixa()
 *
 *      Input:  pixaa
 *              &naindex  (<optional return> the pixa index in the pixaa)
 *              copyflag  (L_COPY or L_CLONE)
 *      Return: pixa, or null on error
 *
 *  Notes:
 *      (1) This 'flattens' the pixaa to a pixa, taking the pix in
 *          order in the first pixa, then the second, etc.
 *      (2) If &naindex is defined, we generate a Numa that gives, for
 *          each pix in the pixaa, the index of the pixa to which it belongs.
 */
PIXA *
pixaaFlattenToPixa(PIXAA   *pixaa,
                   NUMA   **pnaindex,
                   l_int32  copyflag)
{
l_int32  i, j, m, n;
BOX     *box;
NUMA    *naindex;
PIX     *pix;
PIXA    *pixa, *pixat;

    PROCNAME("pixaaFlattenToPixa");

    if (!pixaa)
        return (PIXA *)ERROR_PTR("pixaa not defined", procName, NULL);
    if (copyflag != L_COPY && copyflag != L_CLONE)
        return (PIXA *)ERROR_PTR("invalid copyflag", procName, NULL);
    if (pnaindex) {
        naindex = numaCreate(0);
        *pnaindex = naindex;
    }

    n = pixaaGetCount(pixaa);
    pixa = pixaCreate(n);
    for (i = 0; i < n; i++) {
        pixat = pixaaGetPixa(pixaa, i, L_CLONE);
        m = pixaGetCount(pixat);
        for (j = 0; j < m; j++) {
            pix = pixaGetPix(pixat, j, copyflag);
            box = pixaGetBox(pixat, j, copyflag);
            pixaAddPix(pixa, pix, L_INSERT);
            pixaAddBox(pixa, box, L_INSERT);
            if (pnaindex)
                numaAddNumber(naindex, i);  /* save 'row' number */
        }
        pixaDestroy(&pixat);
    }

    return pixa;
}


/*!
 *  pixaSizeRange()
 *
 *      Input:  pixa
 *              &minw, &minh, &maxw, &maxh (<optional return> range of
 *                                          dimensions of pix in the array)
 *      Return: 0 if OK, 1 on error
 */
l_int32  
pixaSizeRange(PIXA     *pixa,
              l_int32  *pminw,
              l_int32  *pminh,
              l_int32  *pmaxw,
              l_int32  *pmaxh)
{
l_int32  minw, minh, maxw, maxh, i, n, w, h;
PIX     *pix;

    PROCNAME("pixaSizeRange");

    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (!pminw && !pmaxw && !pminh && !pmaxh)
        return ERROR_INT("no data can be returned", procName, 1);
    
    minw = minh = 1000000;
    maxw = maxh = 0;
    n = pixaGetCount(pixa);
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, L_CLONE);
        w = pixGetWidth(pix);
        h = pixGetHeight(pix);
        if (w < minw)
            minw = w;
        if (h < minh)
            minh = h;
        if (w > maxw)
            maxw = w;
        if (h > maxh)
            maxh = h;
        pixDestroy(&pix);
    }

    if (pminw) *pminw = minw;
    if (pminh) *pminh = minh;
    if (pmaxw) *pmaxw = maxw;
    if (pmaxh) *pmaxh = maxh;

    return 0;
}


/*!
 *  pixRemoveLargeComponents()
 *
 *      Input:  pixs
 *              maxwidth, maxheight (max dimensions allowed)
 *              connectivity (4 or 8)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              ifsame (L_CLONE or L_COPY to be returned if not changed)
 *              &changed (<optional return> 1 if changed; 0 otherwise)
 *      Return: pixd with components removed, or NULL on error
 *
 *  Notes:
 *      (1) If no elements removed, 'ifsame' determines if a clone or
 *          a copy is returned.
 *      (2) If type == L_REMOVE_IF_EITHER, removes a component if
 *          either dimension violates the max size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a component only if
 *          both dimensions violate the max size constraint.
 */
PIX *
pixRemoveLargeComponents(PIX      *pixs,
                         l_int32   maxwidth,
                         l_int32   maxheight,
                         l_int32   connectivity,
                         l_int32   type,
                         l_int32   ifsame,
                         l_int32  *pchanged)
{
l_int32  w, h, empty, changed, count;
BOXA    *boxa;
PIX     *pixd;
PIXA    *pixas, *pixad;

    PROCNAME("pixRemoveLargeComponents");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (PIX *)ERROR_PTR("invalid type", procName, NULL);
    if (ifsame != L_CLONE && ifsame != L_COPY)
        return (PIX *)ERROR_PTR("invalid ifsame flag", procName, NULL);
    if (pchanged) *pchanged = FALSE;
    
        /* Check if any components exist */
    pixZero(pixs, &empty);
    if (empty)
        return pixClone(pixs);

        /* Remove large components */
    boxa = pixConnComp(pixs, &pixas, connectivity); 
    pixad = pixaRemoveLargeComponents(pixas, maxwidth, maxheight,
                                      type, &changed);
    boxaDestroy(&boxa);
    pixaDestroy(&pixas);
    if (!changed) {
        pixaDestroy(&pixad);
        if (ifsame == L_CLONE)
            return pixClone(pixs);
        else
            return pixCopy(NULL, pixs);
    }
    else {
        if (pchanged) *pchanged = TRUE;
        pixGetDimensions(pixs, &w, &h, NULL);
        count = pixaGetCount(pixad);
        if (count == 0)  /* return empty pix */
            pixd = pixCreateTemplate(pixs);
        else
            pixd = pixaDisplay(pixad, w, h);
        pixaDestroy(&pixad);
        return pixd;
    }
}


/*!
 *  pixRemoveSmallComponents()
 *
 *      Input:  pixs
 *              minwidth, minheight (min dimensions allowed)
 *              connectivity (4 or 8)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              ifsame (L_CLONE or L_COPY to be returned if not changed)
 *              &changed (<optional return> 1 if changed; 0 otherwise)
 *      Return: pixd with components removed, or NULL on error
 *
 *  Notes:
 *      (1) If no elements removed, 'ifsame' determines if a clone or
 *          a copy is returned.
 *      (2) If type == L_REMOVE_IF_EITHER, removes a component if
 *          either dimension violates the min size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a component only if
 *          both dimensions violate the min size constraint.
 */
PIX *
pixRemoveSmallComponents(PIX      *pixs,
                         l_int32   minwidth,
                         l_int32   minheight,
                         l_int32   connectivity,
                         l_int32   type,
                         l_int32   ifsame,
                         l_int32  *pchanged)
{
l_int32  w, h, empty, changed, count;
BOXA    *boxa;
PIX     *pixd;
PIXA    *pixas, *pixad;

    PROCNAME("pixRemoveSmallComponents");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (PIX *)ERROR_PTR("invalid type", procName, NULL);
    if (ifsame != L_CLONE && ifsame != L_COPY)
        return (PIX *)ERROR_PTR("invalid ifsame flag", procName, NULL);
    if (pchanged) *pchanged = FALSE;
    
        /* Check if any components exist */
    pixZero(pixs, &empty);
    if (empty)
        return pixClone(pixs);

        /* Remove small components */
    boxa = pixConnComp(pixs, &pixas, connectivity); 
    pixad = pixaRemoveSmallComponents(pixas, minwidth, minheight,
                                      type, &changed);
    boxaDestroy(&boxa);
    pixaDestroy(&pixas);
    if (!changed) {
        pixaDestroy(&pixad);
        if (ifsame == L_CLONE)
            return pixClone(pixs);
        else
            return pixCopy(NULL, pixs);
    }
    else {
        if (pchanged) *pchanged = TRUE;
        pixGetDimensions(pixs, &w, &h, NULL);
        count = pixaGetCount(pixad);
        if (count == 0)  /* return empty pix */
            pixd = pixCreateTemplate(pixs);
        else
            pixd = pixaDisplay(pixad, w, h);
        pixaDestroy(&pixad);
        return pixd;
    }
}


/*!
 *  pixaRemoveLargeComponents()
 *
 *      Input:  pixa
 *              maxwidth, maxheight (max dimensions allowed)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Returns a clone if no components are removed.
 *      (2) Uses pix and box clones in the new pixa.
 *      (3) If type == L_REMOVE_IF_EITHER, removes a component if
 *          either dimension violates the max size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a component only if
 *          both dimensions violate the max size constraint.
 */
PIXA *
pixaRemoveLargeComponents(PIXA    *pixas,
                          l_int32  maxwidth,
                          l_int32  maxheight,
                          l_int32  type,
                          l_int32 *pchanged)
{
l_int32  i, n, w, h, maxw, maxh;
BOX     *box;
PIX     *pix;
PIXA    *pixad;

    PROCNAME("pixaRemoveLargeComponents");

    if (!pixas)
        return (PIXA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (PIXA *)ERROR_PTR("invalid type", procName, NULL);
    if (pchanged) *pchanged = FALSE;

        /* Check if all components satisfy constraint. */
    pixaSizeRange(pixas, NULL, NULL, &maxw, &maxh);
    if (type == L_REMOVE_IF_EITHER && (maxw <= maxwidth && maxh <= maxheight))
        return pixaCopy(pixas, L_CLONE);
    if (type == L_REMOVE_IF_BOTH && (maxw <= maxwidth || maxh <= maxheight))
        return pixaCopy(pixas, L_CLONE);

        /* Find the large components and don't include them in pixad */
    if (pchanged) *pchanged = TRUE;
    n = pixaGetCount(pixas);
    pixad = pixaCreate(n);
    for (i = 0; i < n; i++) {
        box = pixaGetBox(pixas, i, L_CLONE);
        boxGetGeometry(box, NULL, NULL, &w, &h);
        if ((type == L_REMOVE_IF_EITHER && (w > maxwidth || h > maxheight)) ||
            (type == L_REMOVE_IF_BOTH && (w > maxwidth && h > maxheight))) {
            boxDestroy(&box);
        }
        else {
            pix = pixaGetPix(pixas, i, L_CLONE);
            pixaAddPix(pixad, pix, L_INSERT);
            pixaAddBox(pixad, box, L_INSERT);
        }
    }
                
    return pixad;
}


/*!
 *  pixaRemoveSmallComponents()
 *
 *      Input:  pixa
 *              minwidth, minheight (min dimensions allowed)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Returns a clone if no components are removed.
 *      (2) Uses pix and box clones in the new pixa.
 *      (3) If type == L_REMOVE_IF_EITHER, removes a component if
 *          either dimension violates the min size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a component only if
 *          both dimensions violate the min size constraint.
 */
PIXA *
pixaRemoveSmallComponents(PIXA    *pixas,
                          l_int32  minwidth,
                          l_int32  minheight,
                          l_int32  type,
                          l_int32 *pchanged)
{
l_int32  i, n, w, h, minw, minh;
BOX     *box;
PIX     *pix;
PIXA    *pixad;

    PROCNAME("pixaRemoveSmallComponents");

    if (!pixas)
        return (PIXA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (PIXA *)ERROR_PTR("invalid type", procName, NULL);
    if (pchanged) *pchanged = FALSE;

        /* Check if all components satisfy constraint. */
    pixaSizeRange(pixas, &minw, &minh, NULL, NULL);
    if (type == L_REMOVE_IF_EITHER && (minw >= minwidth && minh >= minheight))
        return pixaCopy(pixas, L_CLONE);
    if (type == L_REMOVE_IF_BOTH && (minw >= minwidth || minh >= minheight))
        return pixaCopy(pixas, L_CLONE);

        /* Find the small components and don't include them in pixad */
    if (pchanged) *pchanged = TRUE;
    n = pixaGetCount(pixas);
    pixad = pixaCreate(n);
    for (i = 0; i < n; i++) {
        box = pixaGetBox(pixas, i, L_CLONE);
        boxGetGeometry(box, NULL, NULL, &w, &h);
        if ((type == L_REMOVE_IF_EITHER && (w < minwidth || h < minheight)) ||
            (type == L_REMOVE_IF_BOTH && (w < minwidth && h < minheight))) {
            boxDestroy(&box);
        }
        else {
            pix = pixaGetPix(pixas, i, L_CLONE);
            pixaAddPix(pixad, pix, L_INSERT);
            pixaAddBox(pixad, box, L_INSERT);
        }
    }
                
    return pixad;
}


/*!
 *  pixaClipToPix()
 *
 *      Input:  pixas
 *              pixs
 *      Return: pixad, or null on error
 *
 *  Notes:
 *      (1) This is intended for use in situations where pixas
 *          was originally generated from the input pixs.
 *      (2) Returns a pixad where each pix in pixas is ANDed
 *          with its associated region of the input pixs.  This
 *          region is specified by the the box that is associated
 *          with the pix.
 *      (3) In a typical application of this function, pixas has
 *          a set of region masks, so this generates a pixa of
 *          the parts of pixs that correspond to each region
 *          mask component, along with the bounding box for
 *          the region.
 */
PIXA *
pixaClipToPix(PIXA  *pixas,
              PIX   *pixs)
{
l_int32  i, n;
BOX     *box;
PIX     *pix, *pixc;
PIXA    *pixad;

    PROCNAME("pixaClipToPix");

    if (!pixas)
        return (PIXA *)ERROR_PTR("pixas not defined", procName, NULL);
    if (!pixs)
        return (PIXA *)ERROR_PTR("pixs not defined", procName, NULL);

    n = pixaGetCount(pixas);
    if ((pixad = pixaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("pixad not made", procName, NULL);

    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixas, i, L_CLONE);
        box = pixaGetBox(pixas, i, L_COPY);
        pixc = pixClipRectangle(pixs, box, NULL);
        pixAnd(pixc, pixc, pix);
        pixaAddPix(pixad, pixc, L_INSERT);
        pixaAddBox(pixad, box, L_INSERT);
        pixDestroy(&pix);
    }

    return pixad;
}
    

/*---------------------------------------------------------------------*
 *                    Pixaa creation and destruction                   *
 *---------------------------------------------------------------------*/
/*!
 *  pixaaCreate()
 *
 *      Input:  n  (initial number of ptrs)
 *      Return: pixaa, or null on error
 */
PIXAA *
pixaaCreate(l_int32  n)
{
PIXAA  *pixaa;

    PROCNAME("pixaaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((pixaa = (PIXAA *)CALLOC(1, sizeof(PIXAA))) == NULL)
        return (PIXAA *)ERROR_PTR("pixaa not made", procName, NULL);
    pixaa->n = 0;
    pixaa->nalloc = n;
    
    if ((pixaa->pixa = (PIXA **)CALLOC(n, sizeof(PIXA *))) == NULL)
        return (PIXAA *)ERROR_PTR("pixa ptrs not made", procName, NULL);

    return pixaa;
}


/*!
 *  pixaaDestroy()
 *
 *      Input:  &pixaa <to be nulled>
 *      Return: void
 */
void
pixaaDestroy(PIXAA  **ppixaa)
{
l_int32  i;
PIXAA   *pixaa;

    PROCNAME("pixaaDestroy");

    if (ppixaa == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((pixaa = *ppixaa) == NULL)
        return;

    for (i = 0; i < pixaa->n; i++)
        pixaDestroy(&pixaa->pixa[i]);
    FREE(pixaa->pixa);

    FREE(pixaa);
    *ppixaa = NULL;

    return;
}


/*---------------------------------------------------------------------*
 *                             Pixaa addition                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaaAddPixa()
 *
 *      Input:  pixaa
 *              pixa  (to be added)
 *              copyflag:
 *                L_INSERT inserts the pixa directly
 *                L_COPY makes a new pixa and copies each pix and each box
 *                L_CLONE gives a new handle to the input pixa
 *                L_COPY_CLONE makes a new pixa and inserts clones of
 *                    all pix and boxes
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixaaAddPixa(PIXAA   *pixaa,
             PIXA    *pixa,
             l_int32  copyflag)
{
l_int32  n;
PIXA    *pixac;

    PROCNAME("pixaaAddPixa");

    if (!pixaa)
        return ERROR_INT("pixaa not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);
    if (copyflag != L_INSERT && copyflag != L_COPY &&
        copyflag != L_CLONE && copyflag != L_COPY_CLONE)
        return ERROR_INT("invalid copyflag", procName, 1);

    if (copyflag == L_INSERT)
        pixac = pixa;
    else {
        if ((pixac = pixaCopy(pixa, copyflag)) == NULL)
            return ERROR_INT("pixac not made", procName, 1);
    }

    n = pixaaGetCount(pixaa);
    if (n >= pixaa->nalloc)
        pixaaExtendArray(pixaa);
    pixaa->pixa[n] = pixac;
    pixaa->n++;

    return 0;
}


/*!
 *  pixaaExtendArray()
 *
 *      Input:  pixaa
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixaaExtendArray(PIXAA  *pixaa)
{
    PROCNAME("pixaaExtendArray");

    if (!pixaa)
        return ERROR_INT("pixaa not defined", procName, 1);

    if ((pixaa->pixa = (PIXA **)reallocNew((void **)&pixaa->pixa,
                             sizeof(l_intptr_t) * pixaa->nalloc,
                             2 * sizeof(l_intptr_t) * pixaa->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);

    pixaa->nalloc = 2 * pixaa->nalloc;
    return 0;
}



/*---------------------------------------------------------------------*
 *                            Pixaa accessors                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaaGetCount()
 *
 *      Input:  pixaa
 *      Return: count, or 0 if no pixaa
 */
l_int32
pixaaGetCount(PIXAA  *pixaa)
{
    PROCNAME("pixaaGetCount");

    if (!pixaa)
        return ERROR_INT("pixaa not defined", procName, 0);

    return pixaa->n;
}


/*!
 *  pixaaGetPixa()
 *
 *      Input:  pixaa
 *              index  (to the index-th pixa)
 *              accesstype  (L_COPY, L_CLONE, L_COPY_CLONE)
 *      Return: pixa, or null on error
 *
 *  Notes:
 *      (1) L_COPY makes a new pixa with a copy of every pix
 *      (2) L_CLONE just makes a new reference to the pixa,
 *          and bumps the counter.  You would use this, for example,
 *          when you need to extract some data from a pix within a
 *          pixa within a pixaa.
 *      (3) L_COPY_CLONE makes a new pixa with a clone of every pix
 *          and box
 *      (4) In all cases, you must invoke pixaDestroy() on the returned pixa
 */
PIXA *
pixaaGetPixa(PIXAA    *pixaa,
             l_int32  index,
             l_int32  accesstype)
{
PIXA  *pixa;

    PROCNAME("pixaaGetPixa");

    if (!pixaa)
        return (PIXA *)ERROR_PTR("pixaa not defined", procName, NULL);
    if (index < 0 || index >= pixaa->n)
        return (PIXA *)ERROR_PTR("index not valid", procName, NULL);
    if (accesstype != L_COPY && accesstype != L_CLONE &&
        accesstype != L_COPY_CLONE)
        return (PIXA *)ERROR_PTR("invalid accesstype", procName, NULL);

    if ((pixa = pixaa->pixa[index]) == NULL)  /* shouldn't happen! */
        return (PIXA *)ERROR_PTR("no pixa[index]", procName, NULL);
    return pixaCopy(pixa, accesstype);
}


/*---------------------------------------------------------------------*
 *                               Pixa Display                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaDisplay()
 *
 *      Input:  pixa
 *              w, h (if set to 0, determines the size from the
 *                    b.b. of the components in pixa)
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) This uses the boxes to place each pix in the rendered composite.
 *      (2) Set w = h = 0 to use the b.b. of the components to determine
 *          the size of the returned pix.
 *      (3) If the pixa is empty, returns an empty 1 bpp pix.
 */
PIX *
pixaDisplay(PIXA    *pixa,
            l_int32  w,
            l_int32  h)
{
l_int32  i, n, d, xb, yb, wb, hb;
PIX     *pixt, *pixd;

    PROCNAME("pixaDisplay");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    
    n = pixaGetCount(pixa);
    if (n == 0 && w == 0 && h == 0)
        return (PIX *)ERROR_PTR("no components; no size", procName, NULL);
    if (n == 0) {
        L_WARNING("no components; returning empty 1 bpp pix", procName);
        return pixCreate(w, h, 1);
    }

        /* If w and h not input, determine the minimum size required
         * to contain the origin and all c.c. */
    if (w == 0 || h == 0)
        boxaGetExtent(pixa->boxa, &w, &h, NULL);

        /* Use the first pix in pixa to determine the depth.  */
    pixt = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pixt);
    pixDestroy(&pixt);

    if ((pixd = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if (pixaGetBoxGeometry(pixa, i, &xb, &yb, &wb, &hb)) {
            L_WARNING("no box found!", procName);
            continue;
        }
        pixt = pixaGetPix(pixa, i, L_CLONE);
        pixRasterop(pixd, xb, yb, wb, hb, PIX_PAINT, pixt, 0, 0);
        pixDestroy(&pixt);
    }

    return pixd;
}


/*!
 *  pixaDisplayRandomCmap()
 *
 *      Input:  pixa (of 1 bpp components, with boxa)
 *              w, h (if set to 0, determines the size from the
 *                    b.b. of the components in pixa)
 *      Return: pix (8 bpp, cmapped, with random colors on the components),
 *              or null on error
 *
 *  Notes:
 *      (1) This uses the boxes to place each pix in the rendered composite.
 *      (2) By default, the background color is: black, cmap index 0.
 *          This can be changed by pixcmapResetColor()
 */
PIX *
pixaDisplayRandomCmap(PIXA    *pixa,
                      l_int32  w,
                      l_int32  h)
{
l_int32   i, n, d, index, xb, yb, wb, hb;
PIX      *pixs, *pixt, *pixd;
PIXCMAP  *cmap;

    PROCNAME("pixaDisplayRandomCmap");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    
    n = pixaGetCount(pixa);
    if (n == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);

        /* Use the first pix in pixa to verify depth is 1 bpp  */
    pixs = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pixs);
    pixDestroy(&pixs);
    if (d != 1)
        return (PIX *)ERROR_PTR("components not 1 bpp", procName, NULL);

        /* If w and h not input, determine the minimum size required
         * to contain the origin and all c.c. */
    if (w == 0 || h == 0)
        boxaGetExtent(pixa->boxa, &w, &h, NULL);

        /* Set up an 8 bpp dest pix, with a colormap with 254 random colors */
    if ((pixd = pixCreate(w, h, 8)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    cmap = pixcmapCreateRandom(8);
    pixSetColormap(pixd, cmap);

        /* Color each component and blit it in */
    for (i = 0; i < n; i++) {
        index = 1 + (i % 255);
        pixaGetBoxGeometry(pixa, i, &xb, &yb, &wb, &hb);
        pixs = pixaGetPix(pixa, i, L_CLONE);
        pixt = pixConvert1To8(NULL, pixs, 0, index);
        pixRasterop(pixd, xb, yb, wb, hb, PIX_PAINT, pixt, 0, 0);
        pixDestroy(&pixs);
        pixDestroy(&pixt);
    }

    return pixd;
}


/*!
 *  pixaDisplayOnLattice()
 *
 *      Input:  pixa
 *              xspace
 *              yspace
 *      Return: pix of composite images, or null on error
 *
 *  Notes:
 *      (1) This places each pix on sequentially on a regular lattice
 *          in the rendered composite.  If a pix is too large to fit in the
 *          allocated lattice space, it is not rendered.
 *      (2) This is useful when putting bitmaps of components,
 *          such as characters, into a single image.
 */
PIX *
pixaDisplayOnLattice(PIXA    *pixa,
                     l_int32  xspace,
                     l_int32  yspace)
{
l_int32  n, nw, nh, w, h, d, wt, ht;
l_int32  index, i, j;
PIX     *pixt, *pixd;

    PROCNAME("pixaDisplayOnLattice");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    
    if ((n = pixaGetCount(pixa)) == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);
    nw = (l_int32)sqrt(n);
    nh = (n + nw - 1) / nw;
    w = xspace * nw;
    h = yspace * nh;

        /* Use the first pix in pixa to determine the depth.  */
    pixt = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pixt);
    pixDestroy(&pixt);

    if ((pixd = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    
    index = 0;
    for (i = 0; i < nh; i++) {
        for (j = 0; j < nw && index < n; j++, index++) {
            pixt = pixaGetPix(pixa, index, L_CLONE);
            wt = pixGetWidth(pixt);
            ht = pixGetHeight(pixt);
            if (wt > xspace || ht > yspace) {
                fprintf(stderr, "pix(%d) omitted; size %dx%d\n", index, wt, ht);
                pixDestroy(&pixt);
                continue;
            }
            pixRasterop(pixd, j * xspace, i * yspace, wt, ht, 
                        PIX_PAINT, pixt, 0, 0);
            pixDestroy(&pixt);
        }
    }

    return pixd;
}


/*!
 *  pixaDisplayUnsplit()
 *
 *      Input:  pixa
 *              nx   (number of mosaic cells horizontally)
 *              ny   (number of mosaic cells vertically)
 *              borderwidth  (of added border on all sides)
 *              bordercolor  (in our RGBA format: 0xrrggbbaa)
 *      Return: pix of tiled images, or null on error
 *
 *  Notes:
 *      (1) This is a logical inverse of pixaSplitPix().  It
 *          constructs a pix from a mosaic of tiles, all of equal size.
 *      (2) For added generality, a border of arbitrary color can
 *          be added to each of the tiles.
 *      (3) In use, pixa will typically have either been generated
 *          from pixaSplitPix() or will derived from a pixa that
 *          was so generated.
 *      (4) All pix in the pixa must be of equal depth.
 */
PIX *
pixaDisplayUnsplit(PIXA     *pixa,
                   l_int32   nx,
                   l_int32   ny,
                   l_int32   borderwidth,
                   l_uint32  bordercolor)
{
l_int32  w, h, d, wt, ht;
l_int32  i, j, k, x, y, n;
PIX     *pixt, *pixd;

    PROCNAME("pixaDisplayUnsplit");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    if (nx <= 0 || ny <= 0)
        return (PIX *)ERROR_PTR("nx and ny must be > 0", procName, NULL);
    if ((n = pixaGetCount(pixa)) == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);
    if (n != nx * ny)
        return (PIX *)ERROR_PTR("n != nx * ny", procName, NULL);
    borderwidth = L_MAX(0, borderwidth);

    pixt = pixaGetPix(pixa, 0, L_CLONE);
    wt = pixGetWidth(pixt);
    ht = pixGetHeight(pixt);
    d = pixGetDepth(pixt);
    pixDestroy(&pixt);
    w = nx * (wt + 2 * borderwidth);
    h = ny * (ht + 2 * borderwidth);

    if ((pixd = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    if (borderwidth > 0)
        pixSetAllArbitrary(pixd, bordercolor);

    y = borderwidth;
    for (i = 0, k = 0; i < ny; i++) {
        x = borderwidth;
        for (j = 0; j < nx; j++, k++) {
            pixt = pixaGetPix(pixa, k, L_CLONE);
            pixRasterop(pixd, x, y, wt, ht, PIX_SRC, pixt, 0, 0);
            pixDestroy(&pixt);
            x += wt + 2 * borderwidth;
        }
        y += ht + 2 * borderwidth;
    }

    return pixd;
}


/*!
 *  pixaDisplayTiled()
 *
 *      Input:  pixa
 *              maxwidth (of output image)
 *              background (0 for white, 1 for black)
 *              spacing
 *      Return: pix of tiled images, or null on error
 *
 *  Notes:
 *      (1) This saves a pixa to a single image file of width not to
 *          exceed maxwidth, with background color either white or black,
 *          and with each subimage spaced on a regular lattice.
 *      (2) The lattice size is determined from the largest width and height,
 *          separately, of all pix in the pixa.
 *      (3) All pix in the pixa must be of equal depth.
 */
PIX *
pixaDisplayTiled(PIXA    *pixa,
                 l_int32  maxwidth,
                 l_int32  background,
                 l_int32  spacing)
{
l_int32  w, h, wmax, hmax, wd, hd, d;
l_int32  i, j, n, ni, ncols, nrows;
l_int32  ystart, xstart, wt, ht;
PIX     *pix, *pixd;

    PROCNAME("pixaDisplayTiled");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    
        /* Find the largest width and height of the subimages */
    if ((n = pixaGetCount(pixa)) == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);
    wmax = hmax = 0;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, L_CLONE);
        w = pixGetWidth(pix);
        h = pixGetHeight(pix);
        if (i == 0)
            d = pixGetDepth(pix);
        else if (d != pixGetDepth(pix)) {
            pixDestroy(&pix);
            return (PIX *)ERROR_PTR("depths not equal", procName, NULL);
        }
        if (w > wmax)
            wmax = w;
        if (h > hmax)
            hmax = h;
        pixDestroy(&pix);
    }

        /* Get the number of rows and columns and the output image size */
    spacing = L_MAX(spacing, 0);
    ncols = (l_int32)((l_float32)(maxwidth - spacing) /
                      (l_float32)(wmax + spacing));
    nrows = (n + ncols - 1) / ncols;
    wd = wmax * ncols + spacing * (ncols + 1);
    hd = hmax * nrows + spacing * (nrows + 1);
    pixd = pixCreate(wd, hd, d);

#if 0
    fprintf(stderr, " nrows = %d, ncols = %d, wmax = %d, hmax = %d\n",
            nrows, ncols, wmax, hmax);
    fprintf(stderr, " space = %d, wd = %d, hd = %d, n = %d\n",
            space, wd, hd, n);
#endif

        /* Reset the background color if necessary */
    if ((background == 1 && d == 1) || (background == 0 && d != 1))
        pixSetAll(pixd);

        /* Blit the images to the dest */
    for (i = 0, ni = 0; i < nrows; i++) {
        ystart = spacing + i * (hmax + spacing);
        for (j = 0; j < ncols && ni < n; j++, ni++) {
            xstart = spacing + j * (wmax + spacing);
            pix = pixaGetPix(pixa, ni, L_CLONE);
            wt = pixGetWidth(pix);
            ht = pixGetHeight(pix);
            pixRasterop(pixd, xstart, ystart, wt, ht, PIX_SRC, pix, 0, 0);
            pixDestroy(&pix);
        }
    }

    return pixd;
}


/*!
 *  pixaDisplayTiledAndScaled()
 *
 *      Input:  pixa
 *              outdepth (output depth: 1, 8 or 32 bpp)
 *              tilewidth (each pix is scaled to this width)
 *              ncols (number of tiles in each row)
 *              background (0 for white, 1 for black; this is the color
 *                 of the spacing between the images)
 *              spacing  (between images, and on outside)
 *              border (width of additional black border on each image;
 *                      use 0 for no border)
 *      Return: pix of tiled images, or null on error
 *
 *  Notes:
 *      (1) This can be used to tile a number of renderings of
 *          an image that are at different scales and depths.
 *      (2) Each image, after scaling and optionally adding the
 *          black border, has width 'tilewidth'.  Thus, the border does
 *          not affect the spacing between the image tiles.  The
 *          maximum allowed border width is tilewidth / 5.
 */
PIX *
pixaDisplayTiledAndScaled(PIXA    *pixa,
                          l_int32  outdepth,
			  l_int32  tilewidth,
			  l_int32  ncols,
                          l_int32  background,
                          l_int32  spacing,
			  l_int32  border)
{
l_int32    x, y, w, h, wd, hd, d;
l_int32    i, n, nrows, maxht, ninrow, irow, bordval;
l_int32   *rowht;
l_float32  scalefact;
PIX       *pix, *pixn, *pixt, *pixb, *pixd;
PIXA      *pixan;

    PROCNAME("pixaDisplayTiledAndScaled");

    if (!pixa)
        return (PIX *)ERROR_PTR("pixa not defined", procName, NULL);
    if (outdepth != 1 && outdepth != 8 && outdepth != 32)
        return (PIX *)ERROR_PTR("outdepth not in {1, 8, 32}", procName, NULL);
    if (border < 0 || border > tilewidth / 5)
        border = 0;
    
    if ((n = pixaGetCount(pixa)) == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);

        /* Normalize scale and depth for each pix; optionally add border */
    pixan = pixaCreate(n);
    bordval = (outdepth == 1) ? 1 : 0;
    for (i = 0; i < n; i++) {
        if ((pix = pixaGetPix(pixa, i, L_CLONE)) == NULL)
            continue;

        pixGetDimensions(pix, &w, &h, &d);
        scalefact = (l_float32)(tilewidth - 2 * border) / (l_float32)w;
        if (d == 1 && outdepth > 1 && scalefact < 1.0)
            pixt = pixScaleToGray(pix, scalefact);
        else
            pixt = pixScale(pix, scalefact, scalefact);

        if (outdepth == 1)
            pixn = pixConvertTo1(pixt, 128);
        else if (outdepth == 8)
            pixn = pixConvertTo8(pixt, FALSE);
        else  /* outdepth == 32 */
            pixn = pixConvertTo32(pixt);
        pixDestroy(&pixt);

        if (border)
            pixb = pixAddBorder(pixn, border, bordval);
        else
            pixb = pixClone(pixn);

	pixaAddPix(pixan, pixb, L_INSERT);
	pixDestroy(&pix);
	pixDestroy(&pixn);
    }
    if ((n = pixaGetCount(pixan)) == 0) { /* should not have changed! */
        pixaDestroy(&pixan);
        return (PIX *)ERROR_PTR("no components", procName, NULL);
    }

        /* Determine the size of each row and of pixd */
    wd = tilewidth * ncols + spacing * (ncols + 1);
    nrows = (n + ncols - 1) / ncols;
    if ((rowht = (l_int32 *)CALLOC(nrows, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("rowht array not made", procName, NULL);
    maxht = 0;
    ninrow = 0;
    irow = 0;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixan, i, L_CLONE);
	ninrow++;
	pixGetDimensions(pix, &w, &h, NULL);
	maxht = L_MAX(h, maxht);
	if (ninrow == ncols) {
            rowht[irow] = maxht;
	    maxht = ninrow = 0;  /* reset */
	    irow++;
	}
	pixDestroy(&pix);
    }
    if (ninrow > 0) {   /* last fencepost */
        rowht[irow] = maxht;
	irow++;  /* total number of rows */
    }
    nrows = irow;
    hd = spacing * (nrows + 1);
    for (i = 0; i < nrows; i++)
        hd += rowht[i];

    pixd = pixCreate(wd, hd, outdepth);
    if ((background == 1 && outdepth == 1) ||
        (background == 0 && outdepth != 1))
        pixSetAll(pixd);

        /* Now blit images to pixd */
    x = y = spacing;
    irow = 0;
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixan, i, L_CLONE);
	pixGetDimensions(pix, &w, &h, NULL);
	if (i && ((i % ncols) == 0)) {  /* start new row */
            x = spacing;
	    y += spacing + rowht[irow];
	    irow++;
	}
	pixRasterop(pixd, x, y, w, h, PIX_SRC, pix, 0, 0);
	x += tilewidth + spacing;
	pixDestroy(&pix);
    }

    pixaDestroy(&pixan);
    FREE(rowht);
    return pixd;
}


/*---------------------------------------------------------------------*
 *                              Pixaa Display                          *
 *---------------------------------------------------------------------*/
/*!
 *  pixaaDisplay()
 *
 *      Input:  pixaa
 *              w, h (if set to 0, determines the size from the
 *                    b.b. of the components in pixaa)
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) Each pix of the pixaa is displayed at the location given by its box
 */
PIX *
pixaaDisplay(PIXAA   *pixaa,
             l_int32  w,
             l_int32  h)
{
l_int32  i, j, n, na, d, wmax, hmax, xb, yb, wb, hb;
PIX     *pixt, *pixd;
PIXA    *pixa;

    PROCNAME("pixaaDisplay");

    if (!pixaa)
        return (PIX *)ERROR_PTR("pixaa not defined", procName, NULL);
    
    n = pixaaGetCount(pixaa);
    if (n == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);

        /* If w and h not input, determine the minimum size required
         * to contain the origin and all c.c. */
    if (w == 0 || h == 0) {
        wmax = hmax = 0;
        for (i = 0; i < n; i++) {
            pixa = pixaaGetPixa(pixaa, i, L_CLONE);
            boxaGetExtent(pixa->boxa, &w, &h, NULL);
            wmax = L_MAX(wmax, w);
            hmax = L_MAX(hmax, h);
            pixaDestroy(&pixa);
        }
        w = wmax;
        h = hmax;
    }

        /* Get depth from first pix */
    pixa = pixaaGetPixa(pixaa, 0, L_CLONE);
    pixt = pixaGetPix(pixa, 0, L_CLONE);
    d = pixGetDepth(pixt);
    pixaDestroy(&pixa);
    pixDestroy(&pixt);

    if ((pixd = pixCreate(w, h, d)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    
    for (i = 0; i < n; i++) {
        pixa = pixaaGetPixa(pixaa, i, L_CLONE);
        na = pixaGetCount(pixa);
        for (j = 0; j < na; j++) {
            pixaGetBoxGeometry(pixa, j, &xb, &yb, &wb, &hb);
            pixt = pixaGetPix(pixa, j, L_CLONE);
            pixRasterop(pixd, xb, yb, wb, hb, PIX_PAINT, pixt, 0, 0);
            pixDestroy(&pixt);
        }
        pixaDestroy(&pixa);
    }

    return pixd;
}


/*!
 *  pixaaDisplayByPixa()
 *
 *      Input:  pixaa
 *              xspace between pix in pixa
 *              yspace between pixa
 *              max width of output pix
 *      Return: pix, or null on error
 *
 *  Notes:
 *      (1) Displays each pixa on a line (or set of lines),
 *          in order from top to bottom.  Within each pixa,
 *          the pix are displayed in order from left to right.
 *      (2) The size of each pix in each pixa is assumed to be
 *          approximately equal to the size of the first pix in
 *          the pixa.  If this assumption is not correct, this
 *          function will not work properly.
 */
PIX *
pixaaDisplayByPixa(PIXAA   *pixaa,
                   l_int32  xspace,
                   l_int32  yspace,
                   l_int32  maxw)
{
l_int32  i, j, npixa, npix;
l_int32  width, height, depth, nlines, lwidth;
l_int32  x, y, w, h, w0, h0;
PIX     *pixt, *pixd;
PIXA    *pixa;

    PROCNAME("pixaaDisplayByPixa");

    if (!pixaa)
        return (PIX *)ERROR_PTR("pixaa not defined", procName, NULL);
    
    if ((npixa = pixaaGetCount(pixaa)) == 0)
        return (PIX *)ERROR_PTR("no components", procName, NULL);

        /* Get size of output pix.  The width is the minimum of the
         * maxw and the largest pixa line width.  The height is whatever
         * it needs to be to accommodate all pixa. */
    height = 2 * yspace;
    width = 0;
    for (i = 0; i < npixa; i++) {
        pixa = pixaaGetPixa(pixaa, i, L_CLONE);
        npix = pixaGetCount(pixa);
        pixt = pixaGetPix(pixa, 0, L_CLONE);
        if (i == 0)
            depth = pixGetDepth(pixt);
        w = pixGetWidth(pixt);
        lwidth = npix * (w + xspace);
        nlines = (lwidth + maxw - 1) / maxw;
        if (nlines > 1)
            width = maxw;
        else
            width = L_MAX(lwidth, width);
        height += nlines * (pixGetHeight(pixt) + yspace);
        pixDestroy(&pixt);
        pixaDestroy(&pixa);
    }

    if ((pixd = pixCreate(width, height, depth)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

        /* Now layout the pix by pixa */
    y = yspace;
    for (i = 0; i < npixa; i++) {
        x = 0;
        pixa = pixaaGetPixa(pixaa, i, L_CLONE);
        npix = pixaGetCount(pixa);
        for (j = 0; j < npix; j++) {
            pixt = pixaGetPix(pixa, j, L_CLONE);
            if (j == 0) {
                w0 = pixGetWidth(pixt);
                h0 = pixGetHeight(pixt);
            }
            w = pixGetWidth(pixt);
            if (width == maxw && x + w >= maxw) {
                x = 0;
                y += h0 + yspace;
            }
            h = pixGetHeight(pixt);
            pixRasterop(pixd, x, y, w, h, PIX_PAINT, pixt, 0, 0);
            pixDestroy(&pixt);
            x += w0 + xspace;
        }
        y += h0 + yspace;
        pixaDestroy(&pixa);
    }

    return pixd;
}



/*---------------------------------------------------------------------*
 *                          Pixa serialized I/O                        *
 *---------------------------------------------------------------------*/
/*!
 *  pixaRead()
 *
 *      Input:  filename
 *      Return: pixa, or null on error
 *
 *  Notes:
 *      (1) The pix are stored in the file as png.
 */
PIXA *
pixaRead(const char  *filename)
{
FILE  *fp;
PIXA  *pixa;

    PROCNAME("pixaRead");

    if (!filename)
        return (PIXA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (PIXA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((pixa = pixaReadStream(fp)) == NULL) {
        fclose(fp);
        return (PIXA *)ERROR_PTR("pixa not read", procName, NULL);
    }

    fclose(fp);
    return pixa;
}


/*!
 *  pixaReadStream()
 *
 *      Input:  stream
 *      Return: pixa, or null on error
 */
PIXA *
pixaReadStream(FILE  *fp)
{
l_int32  n, i, xres, yres, version;
l_int32  ignore;
BOXA    *boxa;
PIX     *pix;
PIXA    *pixa;

    PROCNAME("pixaReadStream");

    if (!fp)
        return (PIXA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nPixa Version %d\n", &version) != 1)
        return (PIXA *)ERROR_PTR("not a pixa file", procName, NULL);
    if (version != PIXA_VERSION_NUMBER)
        return (PIXA *)ERROR_PTR("invalid pixa version", procName, NULL);
    if (fscanf(fp, "Number of pix = %d\n", &n) != 1)
        return (PIXA *)ERROR_PTR("not a pixa file", procName, NULL);

    if ((pixa = pixaCreate(n)) == NULL)
        return (PIXA *)ERROR_PTR("pixa not made", procName, NULL);
    if ((boxa = boxaReadStream(fp)) == NULL)
        return (PIXA *)ERROR_PTR("boxa not made", procName, NULL);
    boxaDestroy(&pixa->boxa);
    pixa->boxa = boxa;

    for (i = 0; i < n; i++) {
        if ((fscanf(fp, " pix[%d]: xres = %d, yres = %d\n",
              &ignore, &xres, &yres)) != 3)
            return (PIXA *)ERROR_PTR("res reading", procName, NULL);
        if ((pix = pixReadStreamPng(fp)) == NULL)
            return (PIXA *)ERROR_PTR("pix not read", procName, NULL);
        pixSetXRes(pix, xres);
        pixSetYRes(pix, yres);
        pixaAddPix(pixa, pix, L_INSERT);
    }

    return pixa;
}


/*!
 *  pixaWrite()
 *
 *      Input:  filename
 *              pixa
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The pix are written to file in png format.
 */
l_int32
pixaWrite(const char  *filename,
          PIXA        *pixa)
{
FILE  *fp;

    PROCNAME("pixaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (pixaWriteStream(fp, pixa))
        return ERROR_INT("pixa not written to stream", procName, 1);
    fclose(fp);

    return 0;
}


/*!
 *  pixaWriteStream()
 *
 *      Input:  stream
 *              pixa
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixaWriteStream(FILE  *fp,
                PIXA  *pixa)
{
l_int32  n, i;
PIX     *pix;

    PROCNAME("pixaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pixa)
        return ERROR_INT("pixa not defined", procName, 1);

    n = pixaGetCount(pixa);
    fprintf(fp, "\nPixa Version %d\n", PIXA_VERSION_NUMBER);
    fprintf(fp, "Number of pix = %d\n", n);
    boxaWriteStream(fp, pixa->boxa);
    for (i = 0; i < n; i++) {
        if ((pix = pixaGetPix(pixa, i, L_CLONE)) == NULL)
            return ERROR_INT("pix not found", procName, 1);
        fprintf(fp, " pix[%d]: xres = %d, yres = %d\n",
                i, pix->xres, pix->yres);
        pixWriteStreamPng(fp, pix, 0.0);
        pixDestroy(&pix);
    }
    return 0;
}

