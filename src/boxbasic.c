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
 *   boxbasic.c
 *
 *   Basic 'class' functions for box, boxa and boxaa,
 *   including accessors and serialization.
 *
 *      Box creation, copy, clone, destruction
 *           BOX      *boxCreate()
 *           BOX      *boxCopy()
 *           BOX      *boxClone()
 *           void      boxDestroy()
 *
 *      Box accessors
 *           l_int32   boxGetGeometry()
 *           l_int32   boxSetGeometry()
 *           l_int32   boxGetRefcount()
 *           l_int32   boxChangeRefcount()
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
 *      Boxaa creation, copy, destruction
 *           BOXAA    *boxaaCreate()
 *           BOXAA    *boxaaCopy()
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
 *
 *      Boxa array modifiers
 *           l_int32   boxaaReplaceBoxa()
 *           l_int32   boxaaInsertBoxa()
 *           l_int32   boxaaRemoveBoxa()
 *           l_int32   boxaaAddBox()
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
 *      Backward compatibility old boxaa read functions
 *           BOXAA    *boxaaReadVersion2()
 *           BOXAA    *boxaaReadStreamVersion2()
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

    if ((box = (BOX *)CALLOC(1, sizeof(BOX))) == NULL)
        return (BOX *)ERROR_PTR("box not made", procName, NULL);
    boxSetGeometry(box, x, y, w, h);
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

    boxChangeRefcount(box, -1);
    if (boxGetRefcount(box) <= 0)
        FREE(box);
    *pbox = NULL;
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

    if (px) *px = 0;
    if (py) *py = 0;
    if (pw) *pw = 0;
    if (ph) *ph = 0;
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (px) *px = box->x;
    if (py) *py = box->y;
    if (pw) *pw = box->w;
    if (ph) *ph = box->h;
    return 0;
}


/*!
 *  boxSetGeometry()
 *
 *      Input:  box
 *              x, y, w, h (use -1 to leave unchanged)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxSetGeometry(BOX     *box,
               l_int32  x,
               l_int32  y,
               l_int32  w,
               l_int32  h)
{
    PROCNAME("boxSetGeometry");

    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (x != -1) box->x = x;
    if (y != -1) box->y = y;
    if (w != -1) box->w = w;
    if (h != -1) box->h = h;
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
                               sizeof(BOX *) * boxa->nalloc,
                               2 * sizeof(BOX *) * boxa->nalloc)) == NULL)
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

    if (px) *px = 0;
    if (py) *py = 0;
    if (pw) *pw = 0;
    if (ph) *ph = 0;
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
 *              index  (to the index-th box)
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
 *      (1) This shifts box[i] --> box[i + 1] for all i >= index,
 *          and then inserts box as box[index].
 *      (2) To insert at the beginning of the array, set index = 0.
 *      (3) To append to the array, it's easier to use boxaAddBox().
 *      (4) This should not be used repeatedly to insert into large arrays,
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
 *      (1) This removes box[index] and then shifts
 *          box[i] --> box[i - 1] for all i > index.
 *      (2) It should not be used repeatedly to remove boxes from
 *          large arrays, because the function is O(n).
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

    array = boxa->box;
    boxDestroy(&array[index]);
    for (i = index + 1; i < n; i++)
        array[i - 1] = array[i];
    array[n - 1] = NULL;
    boxa->n--;

    return 0;
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

    PROCNAME("boxaaCreate");

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
 *  boxaaCopy()
 *
 *      Input:  baas (input boxaa to be copied)
 *              copyflag (L_COPY, L_CLONE)
 *      Return: baad (new boxaa, composed of copies or clones of the boxa
 *                    in baas), or null on error
 *
 *  Notes:
 *      (1) L_COPY makes a copy of each boxa in baas.
 *          L_CLONE makes a clone of each boxa in baas.
 */
BOXAA *
boxaaCopy(BOXAA   *baas,
          l_int32  copyflag)
{
l_int32  i, n;
BOXA    *boxa;
BOXAA   *baad;

    PROCNAME("boxaaCopy");

    if (!baas)
        return (BOXAA *)ERROR_PTR("baas not defined", procName, NULL);
    if (copyflag != L_COPY && copyflag != L_CLONE)
        return (BOXAA *)ERROR_PTR("invalid copyflag", procName, NULL);

    n = boxaaGetCount(baas);
    baad = boxaaCreate(n);
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(baas, i, copyflag);
        boxaaAddBoxa(baad, boxa, L_INSERT);
    }

    return baad;
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

    PROCNAME("boxaaDestroy");

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
                              sizeof(BOXA *) * baa->nalloc,
                              2 * sizeof(BOXA *) * baa->nalloc)) == NULL)
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
 *  boxaaInsertBoxa()
 *
 *      Input:  boxaa
 *              index (location in boxaa to insert new boxa)
 *              boxa (new boxa to be inserted)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This shifts boxa[i] --> boxa[i + 1] for all i >= index,
 *          and then inserts boxa as boxa[index].
 *      (2) To insert at the beginning of the array, set index = 0.
 *      (3) To append to the array, it's easier to use boxaaAddBoxa().
 *      (4) This should not be used repeatedly to insert into large arrays,
 *          because the function is O(n).
 */
l_int32
boxaaInsertBoxa(BOXAA   *baa,
                l_int32  index,
                BOXA    *boxa)
{
l_int32  i, n;
BOXA   **array;

    PROCNAME("boxaaInsertBoxa");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    n = boxaaGetCount(baa);
    if (index < 0 || index > n)
        return ERROR_INT("index not in {0...n}", procName, 1);
    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    if (n >= baa->nalloc)
        boxaaExtendArray(baa);
    array = baa->boxa;
    baa->n++;
    for (i = n; i > index; i--)
        array[i] = array[i - 1];
    array[index] = boxa;

    return 0;
}


/*!
 *  boxaaRemoveBoxa()
 *
 *      Input:  boxaa
 *              index  (of the boxa to be removed)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This removes boxa[index] and then shifts
 *          boxa[i] --> boxa[i - 1] for all i > index.
 *      (2) The removed boxaa is destroyed.
 *      (2) This should not be used repeatedly on large arrays,
 *          because the function is O(n).
 */
l_int32
boxaaRemoveBoxa(BOXAA   *baa,
                l_int32  index)
{
l_int32  i, n;
BOXA   **array;

    PROCNAME("boxaaRemoveBox");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    n = boxaaGetCount(baa);
    if (index < 0 || index >= n)
        return ERROR_INT("index not valid", procName, 1);

    array = baa->boxa;
    boxaDestroy(&array[index]);
    for (i = index + 1; i < n; i++)
        array[i - 1] = array[i];
    array[n - 1] = NULL;
    baa->n--;

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
    if (version != BOXAA_VERSION_NUMBER)
        return (BOXAA *)ERROR_PTR("invalid boxa version", procName, NULL);
    if (fscanf(fp, "Number of boxa = %d\n", &n) != 1)
        return (BOXAA *)ERROR_PTR("not a boxaa file", procName, NULL);

    if ((baa = boxaaCreate(n)) == NULL)
        return (BOXAA *)ERROR_PTR("boxaa not made", procName, NULL);

    for (i = 0; i < n; i++) {
        if (fscanf(fp, "\nBoxa[%d] extent: x = %d, y = %d, w = %d, h = %d",
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
    fprintf(fp, "\nBoxaa Version %d\n", BOXAA_VERSION_NUMBER);
    fprintf(fp, "Number of boxa = %d\n", n);

    for (i = 0; i < n; i++) {
        if ((boxa = boxaaGetBoxa(baa, i, L_CLONE)) == NULL)
            return ERROR_INT("boxa not found", procName, 1);
        boxaGetExtent(boxa, NULL, NULL, &box);
        boxGetGeometry(box, &x, &y, &w, &h);
        fprintf(fp, "\nBoxa[%d] extent: x = %d, y = %d, w = %d, h = %d",
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



/*---------------------------------------------------------------------*
 *    Version for reading v.2 boxaa; kept for backward compatibility   *
 *---------------------------------------------------------------------*/
/*!
 *  boxaaReadVersion2()
 *
 *      Input:  filename
 *      Return: boxaa, or null on error
 *
 *  Notes:
 *      (1) These old functions only work on version 2 boxaa.
 *          They will go to an archive directory sometime after Sept 2009.
 *      (2) The current format uses BOXAA_VERSION_NUMBER == 3)
 */
BOXAA *
boxaaReadVersion2(const char  *filename)
{
FILE   *fp;
BOXAA  *baa;

    PROCNAME("boxaaReadVersion2");

    if (!filename)
        return (BOXAA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (BOXAA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((baa = boxaaReadStreamVersion2(fp)) == NULL) {
        fclose(fp);
        return (BOXAA *)ERROR_PTR("boxaa not read", procName, NULL);
    }

    fclose(fp);
    return baa;
}


/*!
 *  boxaaReadStreamVersion2()
 *
 *      Input:  stream
 *      Return: boxaa, or null on error
 *
 */
BOXAA *
boxaaReadStreamVersion2(FILE  *fp)
{
l_int32  n, i, x, y, w, h, version;
l_int32  ignore;
BOXA    *boxa;
BOXAA   *baa;

    PROCNAME("boxaaReadStreamVersion2");

    if (!fp)
        return (BOXAA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nBoxaa Version %d\n", &version) != 1)
        return (BOXAA *)ERROR_PTR("not a boxaa file", procName, NULL);
    if (version != 2) {
        fprintf(stderr, "This is version %d\n", version);
        return (BOXAA *)ERROR_PTR("Not old version 2", procName, NULL);
    }
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


