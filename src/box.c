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
 *   box.c
 *
 *      Box creation, copy, clone, destruction
 *           BOX      *boxCreate()
 *           BOX      *boxCopy()
 *           BOX      *boxClone()
 *           void      boxDestroy()
 *           void      boxFree()
 *
 *      Box accessors
 *           l_int32   boxGetGeometry()
 *           l_int32   boxGetRefcount()
 *           l_int32   boxChangeRefcount()
 *
 *      Box geometry
 *           l_int32   boxContains()
 *           l_int32   boxIntersects()
 *           BOXA     *boxaContainedInBox()
 *           BOXA     *boxaIntersectsBox()
 *           BOX      *boxClipToRectangle()
 *
 *      Boxa creation, copy, destruction
 *           BOXA     *boxaCreate()
 *           BOXA     *boxaCopy()
 *           void      boxaDestroy()
 *
 *      Boxa array extension
 *           l_int32   boxaAddBox()
 *           l_int32   boxaExtendArray()
 *
 *      Boxa accessors
 *           l_int32   boxaGetCount()
 *           l_int32   boxaGetBox()
 *           l_int32   boxaGetBoxGeometry()
 *
 *      Boxa array modifiers
 *           l_int32   boxaReplaceBox()
 *           l_int32   boxaInsertBox()
 *           l_int32   boxaRemoveBox()
 *
 *      Boxa combination
 *           l_int32   boxaJoin()
 *
 *      Other boxa functions
 *           l_int32   boxaGetExtent()
 *           l_int32   boxaSizeRange()
 *           BOXA     *boxaRemoveLargeComponents()
 *           BOXA     *boxaRemoveSmallComponents()
 *
 *      Boxa/Box transform
 *           BOX      *boxTransform()
 *           BOXA     *boxaTransform()
 *
 *      Boxa sort
 *           BOXA     *boxaSort()
 *           BOXA     *boxaSortByIndex()
 *           BOXAA    *boxaSort2d()
 *           BOXAA    *boxaSort2dByIndex()
 *
 *      Boxaa creation, destruction
 *           BOXAA    *boxaaCreate()
 *           void      boxaaDestroy()
 *
 *      Boxaa array extension
 *           l_int32   boxaaAddBoxa()
 *           l_int32   boxaaExtendArray()
 *
 *      Boxaa accessors
 *           l_int32   boxaaGetCount()
 *           l_int32   boxaaGetBoxCount()
 *           BOXA     *boxaaGetBoxa()
 *           l_int32   boxaaAddBox()
 *
 *      Other boxaa functions
 *           l_int32   boxaaGetExtent()
 *           BOXA     *boxaaFlattenToBoxa()
 *           l_int32   boxaaAlignBox()
 *
 *      Boxa/Boxaa display
 *           PIX      *boxaDisplay() 
 *           PIX      *boxaaDisplay() 
 *
 *      Boxaa serialized I/O
 *           BOXAA    *boxaaRead()
 *           BOXAA    *boxaaReadStream()
 *           l_int32   boxaaWrite()
 *           l_int32   boxaaWriteStream()
 *
 *      Boxa serialized I/O
 *           BOXA     *boxaRead()
 *           BOXA     *boxaReadStream()
 *           l_int32   boxaWrite()
 *           l_int32   boxaWriteStream()
 *
 *      Box print (for debug)
 *           l_int32   boxPrintStreamInfo()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

static const l_int32  INITIAL_PTR_ARRAYSIZE = 20;   /* n'import quoi */


/*---------------------------------------------------------------------*
 *                  Box creation, destruction and copy                 *
 *---------------------------------------------------------------------*/
/*!
 *  boxCreate()
 *
 *      Input:  x, y, width, height
 *      Return: box, or null on error
 *
 *  Notes:
 *      (1) This clips the box to the +quad.  If no part of the
 *          box is in the +quad, this returns NULL.
 */
BOX *
boxCreate(l_int32  x,
          l_int32  y,
          l_int32  w,
          l_int32  h)
{
BOX  *box;

    PROCNAME("boxCreate");

    if (w <= 0 || h <= 0)
        return (BOX *)ERROR_PTR("w and h not both > 0", procName, NULL);
    if (x < 0) {  /* take part in +quad */
        w = w + x;
        x = 0;
        if (w <= 0)
            return (BOX *)ERROR_PTR("x < 0 and box off +quad", procName, NULL);
    }
    if (y < 0) {  /* take part in +quad */
        h = h + y;
        y = 0;
        if (h <= 0)
            return (BOX *)ERROR_PTR("y < 0 and box off +quad", procName, NULL);
    }

    if ((box = (BOX *) CALLOC(1, sizeof(BOX))) == NULL)
        return (BOX *)ERROR_PTR("box not made", procName, NULL);
    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;
    box->refcount = 1;

    return box;
}


/*!
 *  boxCopy()
 *
 *      Input:  box
 *      Return: copy of box, or null on error
 */
BOX *
boxCopy(BOX  *box)
{
BOX  *boxc;

    PROCNAME("boxCopy");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);

    boxc = boxCreate(box->x, box->y, box->w, box->h);

    return boxc;
}


/*! 
 *  boxClone()
 *
 *      Input:  box
 *      Return: ptr to same box, or null on error
 */
BOX *
boxClone(BOX  *box)
{

    PROCNAME("boxClone");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);

    boxChangeRefcount(box, 1);
    return box;
}


/*!
 *  boxDestroy()
 *
 *      Input:  &box (<will be set to null before returning>)
 *      Return: void
 *
 *  Notes:
 *      (1) Decrements the ref count and, if 0, destroys the box.
 *      (2) Always nulls the input ptr.
 */
void
boxDestroy(BOX  **pbox)
{
BOX  *box;

    PROCNAME("boxDestroy");

    if (pbox == NULL) {
        L_WARNING("ptr address is null!", procName);
        return;
    }

    if ((box = *pbox) == NULL)
        return;

    boxFree(box);
    *pbox = NULL;
    return;
}


/*!
 *  boxFree()
 *
 *      Input:  box
 *      Return: void
 *
 *  Notes:
 *      (1) Decrements the ref count and, if 0, destroys the box.
 */
void
boxFree(BOX  *box)
{
    if (!box) return;

	/* Decrement the ref count.  If it is 0, destroy the box. */
    boxChangeRefcount(box, -1);
    if (boxGetRefcount(box) <= 0)
        FREE(box);
    return;
}


/*---------------------------------------------------------------------*
 *                              Box accessors                          *
 *---------------------------------------------------------------------*/
/*!
 *  boxGetGeometry()
 *
 *      Input:  box
 *              &x, &y, &w, &h (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxGetGeometry(BOX      *box,
               l_int32  *px,
               l_int32  *py,
               l_int32  *pw,
               l_int32  *ph)
{
    PROCNAME("boxGetGeometry");

    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (px) *px = box->x;
    if (py) *py = box->y;
    if (pw) *pw = box->w;
    if (ph) *ph = box->h;
    return 0;
}


l_int32
boxGetRefcount(BOX  *box)
{
    PROCNAME("boxGetRefcount");

    if (!box)
        return ERROR_INT("box not defined", procName, UNDEF);

    return box->refcount;
}


l_int32
boxChangeRefcount(BOX     *box,
                  l_int32  delta)
{
    PROCNAME("boxChangeRefcount");

    if (!box)
        return ERROR_INT("box not defined", procName, 1);

    box->refcount += delta;
    return 0;
}



/*---------------------------------------------------------------------*
 *                             Box geometry                            *
 *---------------------------------------------------------------------*/
/*!
 *  boxContains()
 *
 *      Input:  box1, box2
 *              &result (<return> 1 if box2 is entirely contained within
 *                       box1, and 0 otherwise)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxContains(BOX     *box1,
            BOX     *box2,
            l_int32 *presult)
{
    PROCNAME("boxContains");

    if (!box1 || !box2)
        return ERROR_INT("box1 and box2 not both defined", procName, 1);

    if ((box1->x <= box2->x) &&
        (box1->y <= box2->y) &&
        (box1->x + box1->w >= box2->x + box2->w) &&
        (box1->y + box1->h >= box2->y + box2->h))
        *presult = 1;
    else
        *presult = 0;
    return 0;
}
                


/*!
 *  boxIntersects()
 *
 *      Input:  box1, box2
 *              &result (<return> 1 if any part of box2 is contained
 *                      in box1, and 0 otherwise)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxIntersects(BOX      *box1,
              BOX      *box2,
              l_int32  *presult)
{
l_int32  left1, left2, top1, top2, right1, right2, bot1, bot2;

    PROCNAME("boxIntersects");

    if (!box1 || !box2)
        return ERROR_INT("box1 and box2 not both defined", procName, 1);

    left1 = box1->x;
    left2 = box2->x;
    top1 = box1->y;
    top2 = box2->y;
    right1 = box1->x + box1->w - 1;
    bot1 = box1->y + box1->h - 1;
    right2 = box2->x + box2->w - 1;
    bot2 = box2->y + box2->h - 1;
    if ((bot2 >= top1) && (bot1 >= top2) &&
         (right1 >= left2) && (right2 >= left1))
        *presult = 1;
    else
        *presult = 0;
    return 0;
}
                

/*!
 *  boxaContainedInBox()
 *
 *      Input:  boxas, box
 *      Return: boxad (boxa with all boxes in boxas that are
 *                     entirely contained in box; or NULL if no boxes
 *                     in boxas are contained within box, or on error)
 */
BOXA *
boxaContainedInBox(BOXA  *boxas,
                   BOX   *box)
{
l_int32  i, n, val;
BOX     *boxt;
BOXA    *boxad;

    PROCNAME("boxaContainedInBox");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!box)
        return (BOXA *)ERROR_PTR("box not defined", procName, NULL);
    if ((n = boxaGetCount(boxas)) == 0)
        return (BOXA *)ERROR_PTR("no boxes in boxas", procName, NULL);

    boxad = boxaCreate(0);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxas, i, L_CLONE);
        boxContains(box, boxt, &val);
        if (val == 1)
            boxaAddBox(boxad, boxt, L_COPY);
        boxDestroy(&boxt);  /* destroy the clone */
    }

    return boxad;
}


/*!
 *  boxaIntersectsBox()
 *
 *      Input:  boxas, box
 *      Return  boxad (boxa with all boxes in boxas that intersect box;
 *                     or NULL if no boxes intersect the box, or on error)
 */
BOXA *
boxaIntersectsBox(BOXA  *boxas,
                  BOX   *box)
{
l_int32  i, n, val;
BOX     *boxt;
BOXA    *boxad;

    PROCNAME("boxaIntersectsBox");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!box)
        return (BOXA *)ERROR_PTR("box not defined", procName, NULL);
    if ((n = boxaGetCount(boxas)) == 0)
        return (BOXA *)ERROR_PTR("no boxes in boxas", procName, NULL);

    boxad = boxaCreate(0);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxas, i, L_CLONE);
        boxIntersects(box, boxt, &val);
        if (val == 1)
            boxaAddBox(boxad, boxt, L_COPY);
        boxDestroy(&boxt);  /* destroy the clone */
    }

    return boxad;
}


/*!
 *  boxClipToRectangle()
 *
 *      Input:  box
 *              wi, hi (rectangle representing image)
 *      Return: part of box within given rectangle, or NULL on error
 *              or if box is entirely outside the rectangle
 *
 *  Note: the rectangle is assumed to go from (0,0) to (wi - 1, hi - 1)
 */
BOX *
boxClipToRectangle(BOX     *box,
                   l_int32  wi,
                   l_int32  hi)
{
BOX  *boxd;

    PROCNAME("boxClipToRectangle");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);
    if (box->x >= wi || box->y >= hi ||
        box->x + box->w <= 0 || box->y + box->h <= 0)
        return (BOX *)ERROR_PTR("box outside rectangle", procName, NULL);

    boxd = boxCopy(box);
    if (boxd->x < 0) {
        boxd->w += boxd->x;
        boxd->x = 0;
    }
    if (boxd->y < 0) {
        boxd->h += boxd->y;
        boxd->y = 0;
    }
    if (boxd->x + boxd->w > wi)
        boxd->w = wi - boxd->x;
    if (boxd->y + boxd->h > hi)
        boxd->h = hi - boxd->y;
    return boxd;
}



/*---------------------------------------------------------------------*
 *             Boxa creation, destruction, copy, extension             *
 *---------------------------------------------------------------------*/
/*!
 *  boxaCreate()
 *
 *      Input:  n  (initial number of ptrs)
 *      Return: boxa, or null on error
 */
BOXA *
boxaCreate(l_int32  n)
{
BOXA  *boxa;

    PROCNAME("boxaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((boxa = (BOXA *)CALLOC(1, sizeof(BOXA))) == NULL)
        return (BOXA *)ERROR_PTR("boxa not made", procName, NULL);
    boxa->n = 0;
    boxa->nalloc = n;
    boxa->refcount = 1;
    
    if ((boxa->box = (BOX **)CALLOC(n, sizeof(BOX *))) == NULL)
        return (BOXA *)ERROR_PTR("boxa ptrs not made", procName, NULL);

    return boxa;
}


/*!
 *  boxaCopy()
 *
 *      Input:  boxa
 *              copyflag (L_COPY, L_CLONE, L_COPY_CLONE)
 *      Return: new boxa, or null on error
 *
 *  Notes:
 *      (1) See pix.h for description of the copyflag.
 *      (2) The copy-clone makes a new boxa that holds clones of each box.
 */
BOXA *
boxaCopy(BOXA    *boxa,
         l_int32  copyflag)
{
l_int32  i;
BOX     *boxc;
BOXA    *boxac;

    PROCNAME("boxaCopy");

    if (!boxa)
        return (BOXA *)ERROR_PTR("boxa not defined", procName, NULL);

    if (copyflag == L_CLONE) {
        boxa->refcount++;
        return boxa;
    }

    if (copyflag != L_COPY && copyflag != L_COPY_CLONE)
        return (BOXA *)ERROR_PTR("invalid copyflag", procName, NULL);

    if ((boxac = boxaCreate(boxa->nalloc)) == NULL)
        return (BOXA *)ERROR_PTR("boxac not made", procName, NULL);
    for (i = 0; i < boxa->n; i++) {
        if (copyflag == L_COPY)
            boxc = boxaGetBox(boxa, i, L_COPY);
        else   /* copy-clone */
            boxc = boxaGetBox(boxa, i, L_CLONE);
        boxaAddBox(boxac, boxc, L_INSERT);
    }
    return boxac;
}


/*!
 *  boxaDestroy()
 *
 *      Input:  &boxa (<will be set to null before returning>)
 *      Return: void
 *
 *  Note:
 *      - Decrements the ref count and, if 0, destroys the boxa.
 *      - Always nulls the input ptr.
 */
void
boxaDestroy(BOXA  **pboxa)
{
l_int32  i;
BOXA    *boxa;

    PROCNAME("boxaDestroy");

    if (pboxa == NULL) {
        L_WARNING("ptr address is null!", procName);
        return;
    }

    if ((boxa = *pboxa) == NULL)
        return;

        /* Decrement the ref count.  If it is 0, destroy the boxa. */
    boxa->refcount--;
    if (boxa->refcount <= 0) {
        for (i = 0; i < boxa->n; i++)
            boxDestroy(&boxa->box[i]);
        FREE(boxa->box);
        FREE(boxa);
    }

    *pboxa = NULL;
    return;
}


/*!
 *  boxaAddBox()
 *
 *      Input:  boxa
 *              box  (to be added)
 *              copyflag (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaAddBox(BOXA    *boxa,
           BOX     *box,
           l_int32  copyflag)
{
l_int32  n;
BOX     *boxc;

    PROCNAME("boxaAddBox");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);

    if (copyflag == L_INSERT)
        boxc = box;
    else if (copyflag == L_COPY)
        boxc = boxCopy(box);
    else if (copyflag == L_CLONE)
        boxc = boxClone(box);
    else
        return ERROR_INT("invalid copyflag", procName, 1);
    if (!boxc)
        return ERROR_INT("boxc not made", procName, 1);

    n = boxaGetCount(boxa);
    if (n >= boxa->nalloc)
        boxaExtendArray(boxa);
    boxa->box[n] = boxc;
    boxa->n++;

    return 0;
}


/*!
 *  boxaExtendArray()
 *
 *      Input:  boxa
 *      Return: 0 if OK; 1 on error
 */
l_int32
boxaExtendArray(BOXA  *boxa)
{

    PROCNAME("boxaExtendArray");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    if ((boxa->box = (BOX **)reallocNew((void **)&boxa->box,
                               sizeof(l_intptr_t) * boxa->nalloc,
                               2 * sizeof(l_intptr_t) * boxa->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);

    boxa->nalloc = 2 * boxa->nalloc;
    return 0;
}



/*---------------------------------------------------------------------*
 *                             Boxa accessors                          *
 *---------------------------------------------------------------------*/
/*!
 *  boxaGetCount()
 *
 *      Input:  boxa
 *      Return: count, or 0 on error
 */
l_int32
boxaGetCount(BOXA  *boxa)
{

    PROCNAME("boxaGetCount");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 0);

    return boxa->n;
}


/*!
 *  boxaGetBox()
 *
 *      Input:  boxa
 *              index  (to the index-th box)
 *              accessflag  (L_COPY or L_CLONE)
 *      Return: box, or null on error
 */
BOX *
boxaGetBox(BOXA    *boxa,
           l_int32  index,
           l_int32  accessflag)
{
    PROCNAME("boxaGetBox");

    if (!boxa)
        return (BOX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (index < 0 || index >= boxa->n)
        return (BOX *)ERROR_PTR("index not valid", procName, NULL);

    if (accessflag == L_COPY)
        return boxCopy(boxa->box[index]);
    else if (accessflag == L_CLONE)
        return boxClone(boxa->box[index]);
    else
        return (BOX *)ERROR_PTR("invalid accessflag", procName, NULL);
}


/*!
 *  boxaGetBoxGeometry()
 *
 *      Input:  boxa
 *              index  (to the index-th box)
 *              &x, &y, &w, &h (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaGetBoxGeometry(BOXA     *boxa,
                   l_int32   index,
                   l_int32  *px,
                   l_int32  *py,
                   l_int32  *pw,
                   l_int32  *ph)
{
BOX  *box;

    PROCNAME("boxaGetBoxGeometry");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (index < 0 || index >= boxa->n)
        return ERROR_INT("index not valid", procName, 1);

    if ((box = boxaGetBox(boxa, index, L_CLONE)) == NULL)
        return ERROR_INT("box not found!", procName, 1);
    boxGetGeometry(box, px, py, pw, ph);
    boxDestroy(&box);
    return 0;
}


/*---------------------------------------------------------------------*
 *                        Boxa array modifiers                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaReplaceBox()
 *
 *      Input:  boxa
 *              index  (to the index-th pix)
 *              box (insert to replace existing one)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) In-place replacement of one box.
 *      (2) The previous box at that location is destroyed.
 */
l_int32
boxaReplaceBox(BOXA    *boxa,
               l_int32  index,
               BOX     *box)
{
    PROCNAME("boxaReplaceBox");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (index < 0 || index >= boxa->n)
        return ERROR_INT("index not valid", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);

    boxDestroy(&(boxa->box[index]));
    boxa->box[index] = box;
    return 0;
}


/*!
 *  boxaInsertBox()
 *
 *      Input:  boxa
 *              index (location in boxa to insert new value)
 *              box (new box to be inserted)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This shifts boxa[i] --> boxa[i + 1] for all i >= index,
 *          and then inserts val as na[index].
 *      (2) To insert at the beginning of the array, set index = 0.
 *      (3) To append to the array, it's easier to use boxaAddBox().
 *      (4) This should not be used repeatedly on large arrays,
 *          because the function is O(n).
 */
l_int32
boxaInsertBox(BOXA    *boxa,
              l_int32  index,
              BOX     *box)
{
l_int32  i, n;
BOX    **array;

    PROCNAME("boxaInsertBox");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    n = boxaGetCount(boxa);
    if (index < 0 || index > n)
        return ERROR_INT("index not in {0...n}", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);

        /* Insert the new box; use internals; extend array first if req'd */
    if (n >= boxa->nalloc)
        boxaExtendArray(boxa);
    array = boxa->box;
    boxa->n++;
    for (i = n; i > index; i--)
        array[i] = array[i - 1];
    array[index] = box;

    return 0;
}


/*!
 *  boxaRemoveBox()
 *
 *      Input:  boxa
 *              index (of box to be removed)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This removes boxa[index] and then shifts
 *          boxa[i] --> boxa[i - 1] for all i > index.
 *      (2) It should not be used repeatedly on large arrays,
 *          because the function is O(n).
 */
l_int32
boxaRemoveBox(BOXA    *boxa,
              l_int32  index)
{
l_int32  i, n;
BOX    **array;

    PROCNAME("boxaRemoveBox");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    n = boxaGetCount(boxa);
    if (index < 0 || index >= n)
        return ERROR_INT("index not in {0...n - 1}", procName, 1);

        /* Use internals for simplicity */
    array = boxa->box;
    boxDestroy(&array[index]);
    for (i = index + 1; i < n; i++)
        array[i - 1] = array[i];
    array[n - 1] = NULL;
    boxa->n--;

    return 0;
}


/*----------------------------------------------------------------------*
 *                          Boxa Combination                            *
 *----------------------------------------------------------------------*/
/*!
 *  boxaJoin()
 *
 *      Input:  boxad  (dest boxa; add to this one)
 *              boxas  (source boxa; add from this one)
 *              istart  (starting index in nas)
 *              iend  (ending index in nas; use 0 to cat all)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This appends a clone of each indicated box in boxas to boxad
 *      (2) istart < 0 is taken to mean 'read from the start' (istart = 0)
 *      (3) iend <= 0 means 'read to the end'
 */
l_int32
boxaJoin(BOXA    *boxad,
         BOXA    *boxas,
         l_int32  istart,
         l_int32  iend)
{
l_int32  ns, i;
BOX     *box;

    PROCNAME("boxaJoin");

    if (!boxad)
        return ERROR_INT("boxad not defined", procName, 1);
    if (!boxas)
        return ERROR_INT("boxas not defined", procName, 1);
    ns = boxaGetCount(boxas);
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
        box = boxaGetBox(boxas, i, L_CLONE);
        boxaAddBox(boxad, box, L_INSERT);
    }

    return 0;
}



/*---------------------------------------------------------------------*
 *                        Other Boxa functions                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaGetExtent()
 *
 *      Input:  boxa
 *              &w  (<optional return> width)
 *              &h  (<optional return> height)
 *              &box (<optional return>, minimum box containing all boxes
 *                    in boxa)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned w and h are the minimum size image
 *          that would contain all boxes untranslated.
 */
l_int32
boxaGetExtent(BOXA     *boxa,
              l_int32  *pw,
              l_int32  *ph,
              BOX     **pbox)
{
l_int32  i, n, x, y, w, h, xmax, ymax, xmin, ymin;

    PROCNAME("boxaGetExtent");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (!pw && !ph && !pbox) {
        L_WARNING("no ptrs defined", procName);
        return 1;
    }

    if (pw) *pw = 0;
    if (ph) *ph = 0;
    if (pbox) *pbox = NULL;
    n = boxaGetCount(boxa);
    if (n == 0)
        return ERROR_INT("no boxes in boxa", procName, 1);

    xmax = ymax = 0;
    xmin = ymin = 100000000;
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
        xmin = L_MIN(xmin, x);
        ymin = L_MIN(ymin, y);
        xmax = L_MAX(xmax, x + w);
        ymax = L_MAX(ymax, y + h);
    }
    if (pw) *pw = xmax;
    if (ph) *ph = ymax;
    if (pbox)
      *pbox = boxCreate(xmin, ymin, xmax - xmin, ymax - ymin);

    return 0;
}


/*!
 *  boxaSizeRange()
 *
 *      Input:  boxa
 *              &minw, &minh, &maxw, &maxh (<optional return> range of
 *                                          dimensions of box in the array)
 *      Return: 0 if OK, 1 on error
 */
l_int32  
boxaSizeRange(BOXA     *boxa,
              l_int32  *pminw,
              l_int32  *pminh,
              l_int32  *pmaxw,
              l_int32  *pmaxh)
{
l_int32  minw, minh, maxw, maxh, i, n, w, h;

    PROCNAME("boxaSizeRange");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (!pminw && !pmaxw && !pminh && !pmaxh)
        return ERROR_INT("no data can be returned", procName, 1);
    
    minw = minh = 100000000;
    maxw = maxh = 0;
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, NULL, NULL, &w, &h);
        if (w < minw)
            minw = w;
        if (h < minh)
            minh = h;
        if (w > maxw)
            maxw = w;
        if (h > maxh)
            maxh = h;
    }

    if (pminw) *pminw = minw;
    if (pminh) *pminh = minh;
    if (pmaxw) *pmaxw = maxw;
    if (pmaxh) *pmaxh = maxh;

    return 0;
}


/*!
 *  boxaRemoveLargeComponents()
 *
 *      Input:  boxa
 *              maxwidth, maxheight (max dimensions allowed)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Returns a clone if no components are removed.
 *      (2) Uses box clones in the new boxa.
 *      (3) If type == L_REMOVE_IF_EITHER, removes a box if
 *          either dimension violates the max size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a box only if
 *          both dimensions violate the max size constraint.
 */
BOXA *
boxaRemoveLargeComponents(BOXA     *boxas,
                          l_int32   maxwidth,
                          l_int32   maxheight,
                          l_int32   type,
                          l_int32  *pchanged)
{
l_int32  i, n, w, h, maxw, maxh;
BOX     *box;
BOXA    *boxad;

    PROCNAME("boxaRemoveLargeComponents");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (BOXA *)ERROR_PTR("invalid type", procName, NULL);
    if (pchanged) *pchanged = FALSE;
    
        /* Check if all components satisfy constraint. */
    boxaSizeRange(boxas, NULL, NULL, &maxw, &maxh);
    if (type == L_REMOVE_IF_EITHER && (maxw <= maxwidth && maxh <= maxheight))
        return boxaCopy(boxas, L_CLONE);
    if (type == L_REMOVE_IF_BOTH && (maxw <= maxwidth || maxh <= maxheight))
        return boxaCopy(boxas, L_CLONE);

    if (pchanged) *pchanged = TRUE;
    n = boxaGetCount(boxas);
    boxad = boxaCreate(n);
    for (i = 0; i < n; i++) {
        if ((box = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("box not found", procName, NULL);
        boxGetGeometry(box, NULL, NULL, &w, &h);
        if ((type == L_REMOVE_IF_EITHER && (w > maxwidth || h > maxheight)) ||
            (type == L_REMOVE_IF_BOTH && (w > maxwidth && h > maxheight)))
            boxDestroy(&box);
        else
            boxaAddBox(boxad, box, L_INSERT);
    }
                
    return boxad;
}


/*!
 *  boxaRemoveSmallComponents()
 *
 *      Input:  boxa
 *              minwidth, minheight (min dimensions allowed)
 *              type (L_REMOVE_IF_EITHER, L_REMOVE_IF_BOTH)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Returns a clone if no components are removed.
 *      (2) Uses box clones in the new boxa.
 *      (3) If type == L_REMOVE_IF_EITHER, removes a component if
 *          either dimension violates the min size constraint.
 *          If type == L_REMOVE_IF_BOTH, removes a component only if
 *          both dimensions violate the min size constraint.
 */
BOXA *
boxaRemoveSmallComponents(BOXA     *boxas,
                          l_int32   minwidth,
                          l_int32   minheight,
                          l_int32   type,
                          l_int32  *pchanged)
{
l_int32  i, n, w, h, minw, minh;
BOX     *box;
BOXA    *boxad;

    PROCNAME("boxaRemoveSmallComponents");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (type != L_REMOVE_IF_EITHER && type != L_REMOVE_IF_BOTH)
        return (BOXA *)ERROR_PTR("invalid type", procName, NULL);
    if (pchanged) *pchanged = FALSE;
    
        /* Check if all components satisfy constraint. */
    boxaSizeRange(boxas, &minw, &minh, NULL, NULL);
    if (type == L_REMOVE_IF_EITHER && (minw >= minwidth && minh >= minheight))
        return boxaCopy(boxas, L_CLONE);
    if (type == L_REMOVE_IF_BOTH && (minw >= minwidth || minh >= minheight))
        return boxaCopy(boxas, L_CLONE);

    if (pchanged) *pchanged = TRUE;
    n = boxaGetCount(boxas);
    boxad = boxaCreate(n);
    for (i = 0; i < n; i++) {
        if ((box = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("box not found", procName, NULL);
        boxGetGeometry(box, NULL, NULL, &w, &h);
        if ((type == L_REMOVE_IF_EITHER && (w < minwidth || h < minheight)) ||
            (type == L_REMOVE_IF_BOTH && (w < minwidth && h < minheight)))
            boxDestroy(&box);
        else
            boxaAddBox(boxad, box, L_INSERT);
    }
                
    return boxad;
}



/*---------------------------------------------------------------------*
 *                        Boxa/Box transform                           *
 *---------------------------------------------------------------------*/
/*!
 *  boxaTransform()
 * 
 *      Input:  boxa
 *              shiftx, shifty
 *              scalex, scaley
 *      Return: boxad, or null on error
 *
 *  Note: we shift first, then scale everything
 */
BOXA *
boxaTransform(BOXA      *boxas,
              l_int32    shiftx,
              l_int32    shifty,
              l_float32  scalex,
              l_float32  scaley)
{
l_int32  i, n;
BOX     *boxs, *boxd;
BOXA    *boxad;

    PROCNAME("boxaTransform");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    n = boxaGetCount(boxas);
    if ((boxad = boxaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("boxad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if ((boxs = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("boxs not found", procName, NULL);
        boxd = boxTransform(boxs, shiftx, shifty, scalex, scaley);
        boxDestroy(&boxs);
        boxaAddBox(boxad, boxd, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxTransform()
 * 
 *      Input:  boxs
 *              shiftx, shifty
 *              scalex, scaley
 *      Return: boxd, or null on error
 *
 *  Note: we shift first, then scale everything.
 */
BOX *
boxTransform(BOX       *box,
             l_int32    shiftx,
             l_int32    shifty,
             l_float32  scalex,
             l_float32  scaley)
{
    PROCNAME("boxTransform");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);
    return boxCreate((l_int32)(scalex * (box->x + shiftx) + 0.5),
                     (l_int32)(scaley * (box->y + shifty) + 0.5),
                     (l_int32)(L_MAX(1.0, scalex * box->w + 0.5)),
                     (l_int32)(L_MAX(1.0, scaley * box->h + 0.5)));
}


/*---------------------------------------------------------------------*
 *                              Boxa sort                              *
 *---------------------------------------------------------------------*/
/*!
 *  boxaSort()
 * 
 *      Input:  boxa
 *              sorttype (L_SORT_BY_X, L_SORT_BY_Y, L_SORT_BY_WIDTH,
 *                        L_SORT_BY_HEIGHT, L_SORT_BY_MIN_DIMENSION,
 *                        L_SORT_BY_MAX_DIMENSION, L_SORT_BY_PERIMETER,
 *                        L_SORT_BY_AREA)
 *              sortorder  (L_SORT_INCREASING, L_SORT_DECREASING)
 *              &naindex (<optional return> index of sorted order into
 *                        original array)
 *      Return: boxad (sorted version of boxas), or null on error
 */
BOXA *
boxaSort(BOXA    *boxas,
         l_int32  sorttype,
         l_int32  sortorder,
         NUMA   **pnaindex)
{
l_int32  i, n, size;
BOX     *box;
BOXA    *boxad;
NUMA    *na, *naindex;

    PROCNAME("boxaSort");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (sorttype != L_SORT_BY_X && sorttype != L_SORT_BY_Y && 
        sorttype != L_SORT_BY_WIDTH && sorttype != L_SORT_BY_HEIGHT &&
        sorttype != L_SORT_BY_MIN_DIMENSION &&
        sorttype != L_SORT_BY_MAX_DIMENSION &&
        sorttype != L_SORT_BY_PERIMETER && sorttype != L_SORT_BY_AREA)
        return (BOXA *)ERROR_PTR("invalid sort type", procName, NULL);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return (BOXA *)ERROR_PTR("invalid sort order", procName, NULL);

        /* Build up numa of specific data */
    n = boxaGetCount(boxas);
    if ((na = numaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("na not made", procName, NULL);
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxas, i, L_CLONE);
        if (!box)
          return (BOXA *)ERROR_PTR("box not found", procName, NULL);
        switch (sorttype)
        {
        case L_SORT_BY_X:
            numaAddNumber(na, box->x);
            break;
        case L_SORT_BY_Y:
            numaAddNumber(na, box->y);
            break;
        case L_SORT_BY_WIDTH:
            numaAddNumber(na, box->w);
            break;
        case L_SORT_BY_HEIGHT:
            numaAddNumber(na, box->h);
            break;
        case L_SORT_BY_MIN_DIMENSION:
            size = L_MIN(box->w, box->h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_MAX_DIMENSION:
            size = L_MAX(box->w, box->h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_PERIMETER:
            size = box->w + box->h;
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_AREA:
            size = box->w * box->h;
            numaAddNumber(na, size);
            break;
        default:
            L_WARNING("invalid sort type", procName);
        }
        boxDestroy(&box);
    }

        /* Get the sort index for data array */
    if ((naindex = numaGetSortIndex(na, sortorder)) == NULL)
        return (BOXA *)ERROR_PTR("naindex not made", procName, NULL);

        /* Build up sorted boxa using sort index */
    boxad = boxaSortByIndex(boxas, naindex);

    if (pnaindex)
        *pnaindex = naindex;
    else
        numaDestroy(&naindex);
    numaDestroy(&na);
    return boxad;
}


/*!
 *  boxaSortByIndex()
 * 
 *      Input:  boxas
 *              naindex (na that maps from the new boxa to the input boxa)
 *      Return: boxad (sorted), or null on error
 */
BOXA *
boxaSortByIndex(BOXA  *boxas,
                NUMA  *naindex)
{
l_int32  i, n, index;
BOX     *box;
BOXA    *boxad;

    PROCNAME("boxaSortByIndex");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!naindex)
        return (BOXA *)ERROR_PTR("naindex not defined", procName, NULL);

    n = boxaGetCount(boxas);
    boxad = boxaCreate(n);
    for (i = 0; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        box = boxaGetBox(boxas, index, L_COPY);
        boxaAddBox(boxad, box, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxaSort2d()
 * 
 *      Input:  boxas
 *              &naa (<optional return> numaa with sorted indices
 *                    whose values are the indices of the input array)
 *              delta1 (min overlap that permits aggregation of a box
 *                      onto a boxa of horizontally-aligned boxes; pass 1)
 *              delta2 (min overlap that permits aggregation of a box
 *                      onto a boxa of horizontally-aligned boxes; pass 2)
 *              minh1 (components less than this height either join an
 *                     existing boxa or are set aside for pass 2)
 *      Return: boxaa (2d sorted version of boxa), or null on error
 *
 *  Notes:
 *      (1) The final result is a sort where the 'fast scan' direction is
 *          left to right, and the 'slow scan' direction is from top
 *          to bottom.  Each boxa in the boxaa represents a sorted set
 *          of boxes from left to right.
 *      (2) Two passes are used to aggregate the boxas, which can corresond
 *          to characters or words in a line of text.  In pass 1, only
 *          taller components, which correspond to xheight or larger,
 *          are permitted to start a new boxa, whereas in pass 2,
 *          the remaining vertically-challenged components are allowed
 *          to join an existing boxa or start a new one.
 *      (3) If delta1 < 0, the first pass allows aggregation when
 *          boxes in the same boxa do not overlap vertically.
 *          The distance by which they can miss and still be aggregated
 *          is the absolute value |delta1|.   Similar for delta2 on
 *          the second pass.
 *      (4) On the first pass, any component of height less than minh1
 *          cannot start a new boxa; it's put aside for later insertion.
 *      (5) On the second pass, any small component that doesn't align
 *          with an existing boxa can start a new one.
 *      (6) This can be used to identify lines of text from
 *          character or word bounding boxes.
 */
BOXAA *
boxaSort2d(BOXA    *boxas,
           NUMAA  **pnaad,
           l_int32  delta1,
           l_int32  delta2,
           l_int32  minh1)
{
l_int32  i, index, h, nt, ne, n, m, ival;
BOX     *box;
BOXA    *boxa, *boxae, *boxan, *boxat1, *boxat2, *boxav, *boxavs;
BOXAA   *baa, *baad;
NUMA    *naindex, *nae, *nan, *nah, *nav, *nat1, *nat2, *nad;
NUMAA   *naa, *naad;

    PROCNAME("boxaSort2d");

    if (!boxas)
        return (BOXAA *)ERROR_PTR("boxas not defined", procName, NULL);

        /* Sort from left to right */
    if ((boxa = boxaSort(boxas, L_SORT_BY_X, L_SORT_INCREASING, &naindex))
                    == NULL)
        return (BOXAA *)ERROR_PTR("boxa not made", procName, NULL);

        /* First pass: assign taller boxes to boxa by row */
    nt = boxaGetCount(boxa);
    baa = boxaaCreate(0);
    naa = numaaCreate(0);
    boxae = boxaCreate(0);  /* save small height boxes here */
    nae = numaCreate(0);  /* keep track of small height boxes */
    for (i = 0; i < nt; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        boxGetGeometry(box, NULL, NULL, NULL, &h);
        if (h < minh1) {  /* save for 2nd pass */
            boxaAddBox(boxae, box, L_INSERT);
            numaAddNumber(nae, i);
        }
        else {
            n = boxaaGetCount(baa);
            boxaaAlignBox(baa, box, delta1, &index);
            if (index < n) {  /* append to an existing boxa */
                boxaaAddBox(baa, index, box, L_INSERT);
            }
            else {  /* doesn't align, need new boxa */
                boxan = boxaCreate(0);
                boxaAddBox(boxan, box, L_INSERT);
                boxaaAddBoxa(baa, boxan, L_INSERT);
                nan = numaCreate(0);
                numaaAddNuma(naa, nan, L_INSERT);
            }
            numaGetIValue(naindex, i, &ival);
            numaaAddNumber(naa, index, ival);
        }
    }
    boxaDestroy(&boxa);
    numaDestroy(&naindex);

        /* Second pass: feed in small height boxes;
         * TODO: this correctly, using local y position! */
    ne = boxaGetCount(boxae);
    for (i = 0; i < ne; i++) {
        box = boxaGetBox(boxae, i, L_CLONE);
        n = boxaaGetCount(baa);
        boxaaAlignBox(baa, box, delta2, &index);
        if (index < n) {  /* append to an existing boxa */
            boxaaAddBox(baa, index, box, L_INSERT);
        }
        else {  /* doesn't align, need new boxa */
            boxan = boxaCreate(0);
            boxaAddBox(boxan, box, L_INSERT);
            boxaaAddBoxa(baa, boxan, L_INSERT);
            nan = numaCreate(0);
            numaaAddNuma(naa, nan, L_INSERT);
        }
        numaGetIValue(nae, i, &ival);  /* location in original boxas */
        numaaAddNumber(naa, index, ival);
    }

        /* Sort each boxa in the boxaa */
    m = boxaaGetCount(baa);
    for (i = 0; i < m; i++) {
        boxat1 = boxaaGetBoxa(baa, i, L_CLONE);
        boxat2 = boxaSort(boxat1, L_SORT_BY_X, L_SORT_INCREASING, &nah);
        boxaaReplaceBoxa(baa, i, boxat2);
        nat1 = numaaGetNuma(naa, i, L_CLONE);
        nat2 = numaSortByIndex(nat1, nah);
        numaaReplaceNuma(naa, i, nat2);
        boxaDestroy(&boxat1);
        numaDestroy(&nat1);
        numaDestroy(&nah);
    }

        /* Sort boxa vertically within boxaa, using the first box
         * in each boxa. */
    m = boxaaGetCount(baa);
    boxav = boxaCreate(m);  /* holds first box in each boxa in baa */
    naad = numaaCreate(m);
    if (pnaad)
        *pnaad = naad;
    baad = boxaaCreate(m);
    for (i = 0; i < m; i++) {
        boxat1 = boxaaGetBoxa(baa, i, L_CLONE);
        box = boxaGetBox(boxat1, 0, L_CLONE); 
        boxaAddBox(boxav, box, L_INSERT);
        boxaDestroy(&boxat1);
    }
    boxavs = boxaSort(boxav, L_SORT_BY_Y, L_SORT_INCREASING, &nav);
    for (i = 0; i < m; i++) {
        numaGetIValue(nav, i, &index);
        boxa = boxaaGetBoxa(baa, index, L_CLONE);
        boxaaAddBoxa(baad, boxa, L_INSERT);
        nad = numaaGetNuma(naa, index, L_CLONE);
        numaaAddNuma(naad, nad, L_INSERT);
    }

/*    fprintf(stderr, "box count = %d, numaa count = %d\n", nt,
            numaaGetNumberCount(naad)); */

    boxaaDestroy(&baa);
    boxaDestroy(&boxav);
    boxaDestroy(&boxavs);
    boxaDestroy(&boxae);
    numaDestroy(&nav);
    numaDestroy(&nae);
    numaaDestroy(&naa);
    if (!pnaad)
        numaaDestroy(&naad);

    return baad;
}


/*!
 *  boxaSort2dByIndex()
 * 
 *      Input:  boxas
 *              naa (numaa that maps from the new baa to the input boxa)
 *      Return: baa (sorted boxaa), or null on error
 */
BOXAA *
boxaSort2dByIndex(BOXA   *boxas,
                  NUMAA  *naa)
{
l_int32  ntot, boxtot, i, j, n, nn, index;
BOX     *box;
BOXA    *boxa;
BOXAA   *baa;
NUMA    *na;

    PROCNAME("boxaSort2dByIndex");

    if (!boxas)
        return (BOXAA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!naa)
        return (BOXAA *)ERROR_PTR("naindex not defined", procName, NULL);

        /* Check counts */
    ntot = numaaGetNumberCount(naa);
    boxtot = boxaGetCount(boxas);
    if (ntot != boxtot)
        return (BOXAA *)ERROR_PTR("element count mismatch", procName, NULL);

    n = numaaGetCount(naa);
    baa = boxaaCreate(n);
    for (i = 0; i < n; i++) {
        na = numaaGetNuma(naa, i, L_CLONE);
        nn = numaGetCount(na);
        boxa = boxaCreate(nn);
        for (j = 0; j < nn; j++) {
            numaGetIValue(na, i, &index);
            box = boxaGetBox(boxas, index, L_COPY);
            boxaAddBox(boxa, box, L_INSERT);
        }
        boxaaAddBoxa(baa, boxa, L_INSERT);
        numaDestroy(&na);
    }

    return baa;
}


/*--------------------------------------------------------------------------*
 *                     Boxaa creation, destruction                          *
 *--------------------------------------------------------------------------*/
/*!
 *  boxaaCreate()
 *
 *      Input:  size of boxa ptr array to be alloc'd (0 for default)
 *      Return: baa, or null on error
 */
BOXAA *
boxaaCreate(l_int32  n)
{
BOXAA  *baa;

    PROCNAME("numaaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((baa = (BOXAA *)CALLOC(1, sizeof(BOXAA))) == NULL)
        return (BOXAA *)ERROR_PTR("baa not made", procName, NULL);
    if ((baa->boxa = (BOXA **)CALLOC(n, sizeof(BOXA *))) == NULL)
        return (BOXAA *)ERROR_PTR("boxa ptr array not made", procName, NULL);

    baa->nalloc = n;
    baa->n = 0;

    return baa;
}


/*!
 *  boxaaDestroy()
 *
 *      Input:  &boxaa (<will be set to null before returning>)
 *      Return: void
 */
void
boxaaDestroy(BOXAA  **pbaa)
{
l_int32  i;
BOXAA   *baa;

    PROCNAME("boxnumaaDestroy");

    if (pbaa == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((baa = *pbaa) == NULL)
        return;

    for (i = 0; i < baa->n; i++)
        boxaDestroy(&baa->boxa[i]);
    FREE(baa->boxa);
    FREE(baa);
    *pbaa = NULL;

    return;
}

        

/*--------------------------------------------------------------------------*
 *                              Add Boxa to Boxaa                           *
 *--------------------------------------------------------------------------*/
/*!
 *  boxaaAddBoxa()
 *
 *      Input:  boxaa
 *              boxa     (to be added)
 *              copyflag  (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaaAddBoxa(BOXAA   *baa,
             BOXA    *ba,
             l_int32  copyflag)
{
l_int32  n;
BOXA    *bac;

    PROCNAME("boxaaAddBoxa");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    if (!ba)
        return ERROR_INT("ba not defined", procName, 1);
    if (copyflag != L_INSERT && copyflag != L_COPY && copyflag != L_CLONE)
        return ERROR_INT("invalid copyflag", procName, 1);
    
    if (copyflag == L_INSERT)
        bac = ba;
    else
        bac = boxaCopy(ba, copyflag);

    n = boxaaGetCount(baa);
    if (n >= baa->nalloc)
        boxaaExtendArray(baa);
    baa->boxa[n] = bac;
    baa->n++;
    return 0;
}


/*!
 *  boxaaExtendArray()
 *
 *      Input:  boxaa
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaaExtendArray(BOXAA  *baa)
{

    PROCNAME("boxaaExtendArray");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);

    if ((baa->boxa = (BOXA **)reallocNew((void **)&baa->boxa,
                              sizeof(l_intptr_t) * baa->nalloc,
                              2 * sizeof(l_intptr_t) * baa->nalloc)) == NULL)
            return ERROR_INT("new ptr array not returned", procName, 1);

    baa->nalloc *= 2;
    return 0;
}


/*----------------------------------------------------------------------*
 *                           Boxaa accessors                            *
 *----------------------------------------------------------------------*/
/*!
 *  boxaaGetCount()
 *
 *      Input:  boxaa
 *      Return: count (number of boxa), or 0 if no boxa or on error
 */
l_int32
boxaaGetCount(BOXAA  *baa)
{
    PROCNAME("boxaaGetCount");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 0);
    return baa->n;
}
        

/*!
 *  boxaaGetBoxCount()
 *
 *      Input:  boxaa
 *      Return: count (number of boxes), or 0 if no boxes or on error
 */
l_int32
boxaaGetBoxCount(BOXAA  *baa)
{
BOXA    *boxa;
l_int32  n, sum, i;

    PROCNAME("boxaaGetBoxCount");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 0);

    n = boxaaGetCount(baa);
    for (sum = 0, i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(baa, i, L_CLONE);
        sum += boxaGetCount(boxa);
        boxaDestroy(&boxa);
    }

    return sum;
}
        

/*!
 *  boxaaGetBoxa()
 *
 *      Input:  boxaa
 *              index  (to the index-th boxa)
 *              accessflag   (L_COPY or L_CLONE)
 *      Return: boxa, or null on error
 */
BOXA *
boxaaGetBoxa(BOXAA   *baa,
             l_int32  index,
             l_int32  accessflag)
{
l_int32  n;

    PROCNAME("boxaaGetBoxa");

    if (!baa)
        return (BOXA *)ERROR_PTR("baa not defined", procName, NULL);
    n = boxaaGetCount(baa);
    if (index < 0 || index >= n)
        return (BOXA *)ERROR_PTR("index not valid", procName, NULL);
    if (accessflag != L_COPY && accessflag != L_CLONE)
        return (BOXA *)ERROR_PTR("invalid accessflag", procName, NULL);

    return boxaCopy(baa->boxa[index], accessflag);
}


/*!
 *  boxaaReplaceBoxa()
 *
 *      Input:  boxaa
 *              index  (to the index-th boxa)
 *              boxa (insert and replace any existing one)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Any existing boxa is destroyed, and the input one
 *          is inserted in its place.
 *      (2) If the index is invalid, return 1 (error)
 */
l_int32
boxaaReplaceBoxa(BOXAA   *baa,
                 l_int32  index,
                 BOXA    *boxa)
{
l_int32  n;

    PROCNAME("boxaaReplaceBoxa");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    n = boxaaGetCount(baa);
    if (index < 0 || index >= n)
        return ERROR_INT("index not valid", procName, 1);

    boxaDestroy(&baa->boxa[index]);
    baa->boxa[index] = boxa;
    return 0;
}


/*!
 *  boxaaAddBox()
 *
 *      Input:  boxaa
 *              index (of boxa with boxaa)
 *              box (to be added)
 *              accessflag (L_INSERT, L_COPY or L_CLONE)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Adds to an existing boxa only.
 */
l_int32
boxaaAddBox(BOXAA   *baa,
            l_int32  index,
            BOX     *box,
            l_int32  accessflag)
{
l_int32  n;
BOXA    *boxa;
    PROCNAME("boxaaAddBox");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    n = boxaaGetCount(baa);
    if (index < 0 || index >= n)
        return ERROR_INT("index not valid", procName, 1);
    if (accessflag != L_INSERT && accessflag != L_COPY && accessflag != L_CLONE)
        return ERROR_INT("invalid accessflag", procName, 1);

    boxa = boxaaGetBoxa(baa, index, L_CLONE);
    boxaAddBox(boxa, box, accessflag);
    boxaDestroy(&boxa);
    return 0;
}


/*---------------------------------------------------------------------*
 *                        Other Boxaa functions                        *
 *---------------------------------------------------------------------*/
/*!
 *  boxaaGetExtent()
 *
 *      Input:  boxaa
 *              &w  (<optional return> width)
 *              &h  (<optional return> height)
 *              &box (<optional return>, minimum box containing all boxa
 *                    in boxaa)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned w and h are the minimum size image
 *          that would contain all boxes untranslated.
 */
l_int32
boxaaGetExtent(BOXAA    *boxaa,
               l_int32  *pw,
               l_int32  *ph,
               BOX     **pbox)
{
l_int32  i, j, n, m, x, y, w, h, xmax, ymax, xmin, ymin;
BOXA    *boxa;

    PROCNAME("boxaaGetExtent");

    if (!boxaa)
        return ERROR_INT("boxaa not defined", procName, 1);
    if (!pw && !ph && !pbox) {
        L_WARNING("no ptrs defined", procName);
        return 1;
    }

    if (pw) *pw = 0;
    if (ph) *ph = 0;
    if (pbox) *pbox = NULL;
    n = boxaaGetCount(boxaa);
    if (n == 0)
        return ERROR_INT("no boxa in boxaa", procName, 1);

    xmax = ymax = 0;
    xmin = ymin = 100000000;
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(boxaa, i, L_CLONE);
        m = boxaGetCount(boxa);
        for (j = 0; j < m; j++) {
            boxaGetBoxGeometry(boxa, j, &x, &y, &w, &h);
            xmin = L_MIN(xmin, x);
            ymin = L_MIN(ymin, y);
            xmax = L_MAX(xmax, x + w);
            ymax = L_MAX(ymax, y + h);
        }
    }
    if (pw) *pw = xmax;
    if (ph) *ph = ymax;
    if (pbox)
      *pbox = boxCreate(xmin, ymin, xmax - xmin, ymax - ymin);

    return 0;
}


/*!
 *  boxaaFlattenToBoxa()
 *
 *      Input:  boxaa
 *              &naindex  (<optional return> the boxa index in the boxaa)
 *              copyflag  (L_COPY or L_CLONE)
 *      Return: boxa, or null on error
 *
 *  Notes:
 *      (1) This 'flattens' the boxaa to a boxa, taking the boxes in
 *          order in the first boxa, then the second, etc.
 *      (2) If &naindex is defined, we generate a Numa that gives, for
 *          each box in the boxaa, the index of the boxa to which it belongs.
 */
BOXA *
boxaaFlattenToBoxa(BOXAA   *baa,
                   NUMA   **pnaindex,
                   l_int32  copyflag)
{
l_int32  i, j, m, n;
BOXA    *boxa, *boxat;
BOX     *box;
NUMA    *naindex;

    PROCNAME("boxaaFlattenToBoxa");

    if (!baa)
        return (BOXA *)ERROR_PTR("baa not defined", procName, NULL);
    if (copyflag != L_COPY && copyflag != L_CLONE)
        return (BOXA *)ERROR_PTR("invalid copyflag", procName, NULL);
    if (pnaindex) {
        naindex = numaCreate(0);
        *pnaindex = naindex;
    }

    n = boxaaGetCount(baa);
    boxa = boxaCreate(n);
    for (i = 0; i < n; i++) {
        boxat = boxaaGetBoxa(baa, i, L_CLONE);
        m = boxaGetCount(boxat);
        for (j = 0; j < m; j++) {
            box = boxaGetBox(boxat, j, copyflag);
            boxaAddBox(boxa, box, L_INSERT);
            if (pnaindex)
                numaAddNumber(naindex, i);  /* save 'row' number */
        }
        boxaDestroy(&boxat);
    }

    return boxa;
}


/*!
 *  boxaaAlignBox()
 * 
 *      Input:  boxaa
 *              box (to be aligned with the last of one of the boxa
 *                   in boxaa, if possible)
 *              delta (amount by which consecutive components can miss
 *                     in overlap and still be included in the array)
 *              &index (of boxa with best overlap, or if none match,
 *                      this is the index of the next boxa to be generated)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is not greedy; it finds the boxa whose last box has
 *          the biggest overlap with the input box.
 */
l_int32
boxaaAlignBox(BOXAA    *baa,
              BOX      *box,
              l_int32   delta,
              l_int32  *pindex)
{
l_int32  i, n, m, y, yt, h, ht, ovlp, maxovlp, maxindex;
BOXA    *boxa;

    PROCNAME("boxaaAlignBox");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (!pindex)
        return ERROR_INT("&index not defined", procName, 1);

    n = boxaaGetCount(baa);
    boxGetGeometry(box, NULL, &y, NULL, &h);
    maxovlp = -10000000;
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(baa, i, L_CLONE);
        if ((m = boxaGetCount(boxa)) == 0) {
            L_WARNING("no boxes in boxa", procName);
            continue;
        }
        boxaGetBoxGeometry(boxa, m - 1, NULL, &yt, NULL, &ht);  /* last one */
        boxaDestroy(&boxa);

            /* Overlap < 0 means the components do not overlap vertically */
        if (yt >= y)
            ovlp = y + h - 1 - yt;
        else
            ovlp = yt + ht - 1 - y;
        if (ovlp > maxovlp) {
            maxovlp = ovlp;
            maxindex = i;
        }
    }

    if (maxovlp + delta >= 0)
        *pindex = maxindex;
    else
        *pindex = n;
    return 0;
}


/*---------------------------------------------------------------------*
 *                          Boxa/Boxaa display                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaDisplay()
 *
 *      Input:  boxa
 *              linewidth
 *              w (of pix; use 0 if determined by boxa)
 *              h (of pix; use 0 if determined by boxa)
 *      Return: 0 if OK, 1 on error
 */
PIX *
boxaDisplay(BOXA    *boxa,
            l_int32  linewidth,
            l_int32  w,
            l_int32  h)
{
l_int32  i, n;
BOX     *box;
PIX     *pix;

    PROCNAME("boxaDisplay");

    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (w == 0 || h == 0) 
        boxaGetExtent(boxa, &w, &h, NULL);

    pix = pixCreate(w, h, 1);
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        pixRenderBox(pix, box, linewidth, L_SET_PIXELS);
        boxDestroy(&box);
    }

    return pix;
}


/*!
 *  boxaaDisplay()
 *
 *      Input:  boxaa
 *              linewba (line width to display boxa)
 *              linewb (line width to display box)
 *              colorba (color to display boxa)
 *              colorb (color to display box)
 *              w (of pix; use 0 if determined by boxaa)
 *              h (of pix; use 0 if determined by boxaa)
 *      Return: 0 if OK, 1 on error
 */
PIX *
boxaaDisplay(BOXAA    *boxaa,
             l_int32   linewba,
             l_int32   linewb,
             l_uint32  colorba,
             l_uint32  colorb,
             l_int32   w,
             l_int32   h)
{
l_int32   i, j, n, m, rbox, gbox, bbox, rboxa, gboxa, bboxa;
BOX      *box;
BOXA     *boxa;
PIX      *pix;
PIXCMAP  *cmap;

    PROCNAME("boxaaDisplay");

    if (!boxaa)
        return (PIX *)ERROR_PTR("boxaa not defined", procName, NULL);
    if (w == 0 || h == 0) 
        boxaaGetExtent(boxaa, &w, &h, NULL);

    pix = pixCreate(w, h, 8);
    cmap = pixcmapCreate(8);
    pixSetColormap(pix, cmap);
    rbox = GET_DATA_BYTE(&colorb, COLOR_RED);
    gbox = GET_DATA_BYTE(&colorb, COLOR_GREEN);
    bbox = GET_DATA_BYTE(&colorb, COLOR_BLUE);
    rboxa = GET_DATA_BYTE(&colorba, COLOR_RED);
    gboxa = GET_DATA_BYTE(&colorba, COLOR_GREEN);
    bboxa = GET_DATA_BYTE(&colorba, COLOR_BLUE);
    pixcmapAddColor(cmap, 255, 255, 255);
    pixcmapAddColor(cmap, rbox, gbox, bbox);
    pixcmapAddColor(cmap, rboxa, gboxa, bboxa);
    
    n = boxaaGetCount(boxaa);
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(boxaa, i, L_CLONE);   
        boxaGetExtent(boxa, NULL, NULL, &box);
        pixRenderBoxArb(pix, box, linewba, rboxa, gboxa, bboxa);
        boxDestroy(&box);
        m = boxaGetCount(boxa);
        for (j = 0; j < m; j++) {
            box = boxaGetBox(boxa, j, L_CLONE);
            pixRenderBoxArb(pix, box, linewb, rbox, gbox, bbox);
            boxDestroy(&box);
        }
        boxaDestroy(&boxa);
    }

    return pix;
}


/*---------------------------------------------------------------------*
 *                        Boxaa serialized I/O                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaaRead()
 *
 *      Input:  filename
 *      Return: boxaa, or null on error
 */
BOXAA *
boxaaRead(const char  *filename)
{
FILE   *fp;
BOXAA  *baa;

    PROCNAME("boxaaRead");

    if (!filename)
        return (BOXAA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (BOXAA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((baa = boxaaReadStream(fp)) == NULL) {
        fclose(fp);
        return (BOXAA *)ERROR_PTR("boxaa not read", procName, NULL);
    }

    fclose(fp);
    return baa;
}


/*!
 *  boxaaReadStream()
 *
 *      Input:  stream
 *      Return: boxaa, or null on error
 */
BOXAA *
boxaaReadStream(FILE  *fp)
{
l_int32  n, i, x, y, w, h, version;
l_int32  ignore;
BOXA    *boxa;
BOXAA   *baa;

    PROCNAME("boxaaReadStream");

    if (!fp)
        return (BOXAA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nBoxaa Version %d\n", &version) != 1)
        return (BOXAA *)ERROR_PTR("not a boxaa file", procName, NULL);
    if (version != BOXA_VERSION_NUMBER)
        return (BOXAA *)ERROR_PTR("invalid boxa version", procName, NULL);
    if (fscanf(fp, "Number of boxa = %d\n", &n) != 1)
        return (BOXAA *)ERROR_PTR("not a boxaa file", procName, NULL);

    if ((baa = boxaaCreate(n)) == NULL)
        return (BOXAA *)ERROR_PTR("boxaa not made", procName, NULL);
        
    for (i = 0; i < n; i++) {
        if (fscanf(fp, " Boxa[%d]: x = %d, y = %d, w = %d, h = %d\n",
                &ignore, &x, &y, &w, &h) != 5)
            return (BOXAA *)ERROR_PTR("boxa descr not valid", procName, NULL);
        if ((boxa = boxaReadStream(fp)) == NULL)
            return (BOXAA *)ERROR_PTR("boxa not made", procName, NULL);
        boxaaAddBoxa(baa, boxa, L_INSERT);
    }

    return baa;
}


/*!
 *  boxaaWrite()
 *
 *      Input:  filename
 *              boxaa
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaaWrite(const char  *filename,
           BOXAA       *baa)
{
FILE  *fp;

    PROCNAME("boxaaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (boxaaWriteStream(fp, baa))
        return ERROR_INT("baa not written to stream", procName, 1);
    fclose(fp);

    return 0;
}


/*!
 *  boxaaWriteStream()
 *
 *      Input: stream
 *             boxaa
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaaWriteStream(FILE   *fp,
                 BOXAA  *baa)
{
l_int32  n, i, x, y, w, h;
BOX     *box;
BOXA    *boxa;

    PROCNAME("boxaaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);

    n = boxaaGetCount(baa);
    fprintf(fp, "\nBoxaa Version %d\n", BOXA_VERSION_NUMBER);
    fprintf(fp, "Number of boxa = %d\n", n);
    for (i = 0; i < n; i++) {
        if ((boxa = boxaaGetBoxa(baa, i, L_CLONE)) == NULL)
            return ERROR_INT("boxa not found", procName, 1);
        boxaGetExtent(boxa, NULL, NULL, &box);
        boxGetGeometry(box, &x, &y, &w, &h);
        fprintf(fp, " Boxa[%d]: x = %d, y = %d, w = %d, h = %d\n",
                i, x, y, w, h);
        boxaWriteStream(fp, boxa);
        boxDestroy(&box);
        boxaDestroy(&boxa);
    }
    return 0;
}


/*---------------------------------------------------------------------*
 *                         Boxa serialized I/O                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaRead()
 *
 *      Input:  filename
 *      Return: boxa, or null on error
 */
BOXA *
boxaRead(const char  *filename)
{
FILE  *fp;
BOXA  *boxa;

    PROCNAME("boxaRead");

    if (!filename)
        return (BOXA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (BOXA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((boxa = boxaReadStream(fp)) == NULL) {
        fclose(fp);
        return (BOXA *)ERROR_PTR("boxa not read", procName, NULL);
    }

    fclose(fp);
    return boxa;
}


/*!
 *  boxaReadStream()
 *
 *      Input:  stream
 *      Return: boxa, or null on error
 */
BOXA *
boxaReadStream(FILE  *fp)
{
l_int32  n, i, x, y, w, h, version;
l_int32  ignore;
BOX     *box;
BOXA    *boxa;

    PROCNAME("boxaReadStream");

    if (!fp)
        return (BOXA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nBoxa Version %d\n", &version) != 1)
        return (BOXA *)ERROR_PTR("not a boxa file", procName, NULL);
    if (version != BOXA_VERSION_NUMBER)
        return (BOXA *)ERROR_PTR("invalid boxa version", procName, NULL);
    if (fscanf(fp, "Number of boxes = %d\n", &n) != 1)
        return (BOXA *)ERROR_PTR("not a boxa file", procName, NULL);

    if ((boxa = boxaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("boxa not made", procName, NULL);
        
    for (i = 0; i < n; i++) {
        if (fscanf(fp, "  Box[%d]: x = %d, y = %d, w = %d, h = %d\n",
                &ignore, &x, &y, &w, &h) != 5)
            return (BOXA *)ERROR_PTR("box descr not valid", procName, NULL);
        if ((box = boxCreate(x, y, w, h)) == NULL)
            return (BOXA *)ERROR_PTR("box not made", procName, NULL);
        boxaAddBox(boxa, box, L_INSERT);
    }

    return boxa;
}


/*!
 *  boxaWrite()
 *
 *      Input:  filename
 *              boxa
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaWrite(const char  *filename,
          BOXA        *boxa)
{
FILE  *fp;

    PROCNAME("boxaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (boxaWriteStream(fp, boxa))
        return ERROR_INT("boxa not written to stream", procName, 1);
    fclose(fp);

    return 0;
}


/*!
 *  boxaWriteStream()
 *
 *      Input: stream
 *             boxa
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaWriteStream(FILE  *fp,
                BOXA  *boxa)
{
l_int32  n, i;
BOX     *box;

    PROCNAME("boxaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    n = boxaGetCount(boxa);
    fprintf(fp, "\nBoxa Version %d\n", BOXA_VERSION_NUMBER);
    fprintf(fp, "Number of boxes = %d\n", n);
    for (i = 0; i < n; i++) {
        if ((box = boxaGetBox(boxa, i, L_CLONE)) == NULL)
            return ERROR_INT("box not found", procName, 1);
        fprintf(fp, "  Box[%d]: x = %d, y = %d, w = %d, h = %d\n",
                i, box->x, box->y, box->w, box->h);
        boxDestroy(&box);
    }
    return 0;
}



/*---------------------------------------------------------------------*
 *                            Debug printing                           *
 *---------------------------------------------------------------------*/
/*!
 *  boxPrintStreamInfo()
 *
 *      Input:  stream
 *              box
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This outputs information about the box, for debugging.
 *      (2) Use serialization functions to write to file if you want
 *          to read the data back.
 */
l_int32
boxPrintStreamInfo(FILE  *fp,
                   BOX   *box)
{
    PROCNAME("boxPrintStreamInfo");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);

    fprintf(fp, " Box x (pixels) =           %d\n", box->x);
    fprintf(fp, " Box y (pixels) =           %d\n", box->y);
    fprintf(fp, " Box width (pixels) =       %d\n", box->w);
    fprintf(fp, " Box height (pixels) =      %d\n", box->h);

    return 0;
}


