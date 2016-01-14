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
 *   ptra.c
 *
 *      Ptra creation and destruction
 *          PTRA        *ptraCreate()
 *          void        *ptraDestroy()
 *
 *      Add/insert/remove/replace generic ptr object
 *          l_int32      ptraAdd()
 *          l_int32      ptraExtendArray()
 *          l_int32      ptraInsert()
 *          void        *ptraRemove()
 *          void        *ptraReplace()
 *          l_int32      ptraSwap()
 *          l_int32      ptraCompactArray()
 *
 *      Other array operations
 *          l_int32      ptraReverse()
 *          l_int32      ptraJoin()
 *
 *      Simple Ptra accessors
 *          l_int32      ptraGetMaxIndex()
 *          l_int32      ptraGetActualCount()
 *          void        *ptraGetPtr()
 *
 *    (1) The Ptra is a struct, not an array.  Always use the accessors
 *        in this file, never the fields directly.
 *    (2) It is not required that the items on the ptr array be
 *        compacted, so in general there will be null pointers.
 *        A compacted array will remain compacted on removal if
 *        arbitrary items are removed with compaction, or if items
 *        are removed from the end of the array.
 *    (3) For addition to and removal from the end of the array, this
 *        functions exactly like a stack, and with the same O(1) cost.
 *    (4) This differs from the generic stack in that we allow
 *        random access for insertion, removal and replacement.
 *        Removal can be done without compacting the array.
 *        Insertion into a null ptr in the array has no effect on
 *        the other pointers, but insertion into a location already
 *        occupied by an item has a cost proportional to the
 *        distance to the end of the array.  Null ptrs are valid
 *        input args for both insertion and replacement; this
 *        allows arbitrary swapping.
 *    (5) The item at the end of the array (i.e., the one with the
 *        largest index) is indexed by pa->n - 1.  We are calling
 *        pa->n the "maxindex": it is in fact 1 greater than the
 *        index of the item at the end of the array.
 *    (6) In referring to the array: the first ptr is the "top" or
 *        "beginning"; the last pointer is the "bottom" or "end";
 *        items are shifted "up" to the top when compaction occurs;
 *        and items are shifted "down" to the bottom when forced to
 *        move due to an insertion.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32 INITIAL_PTR_ARRAYSIZE = 20;      /* n'importe quoi */


/*--------------------------------------------------------------------------*
 *                       Ptra creation and destruction                      *
 *--------------------------------------------------------------------------*/
/*!
 *  ptraCreate()
 *
 *      Input:  size of ptr array to be alloc'd (0 for default)
 *      Return: pa, or null on error
 */
PTRA *
ptraCreate(l_int32  n)
{
PTRA  *pa;

    PROCNAME("ptraCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((pa = (PTRA *)CALLOC(1, sizeof(PTRA))) == NULL)
        return (PTRA *)ERROR_PTR("pa not made", procName, NULL);
    if ((pa->array = (void **)CALLOC(n, sizeof(void *))) == NULL)
        return (PTRA *)ERROR_PTR("ptr array not made", procName, NULL);

    pa->nalloc = n;
    pa->n = pa->nactual = 0;

    return pa;
}


/*!
 *  ptraDestroy()
 *
 *      Input:  &ptra (<to be nulled>)
 *              freeflag (TRUE to free each remaining struct in the array)
 *      Return: void
 *
 *  Notes:
 *      (1) If freeflag is TRUE, frees each struct in the array.
 *      (2) If freeflag is FALSE but there are elements on the array,
 *          gives a warning and destroys the array.  This will
 *          cause a memory leak of all the items that were on the array.
 *          So if the items require their own destroy function, they
 *          must be destroyed before the ptra.
 *      (3) To destroy the ptra, we destroy the ptr array, then
 *          the ptra, and then null the contents of the input ptr.
 */
void
ptraDestroy(PTRA   **ppa,
            l_int32  freeflag)
{
l_int32  i, nactual;
void    *item;
PTRA    *pa;

    PROCNAME("ptraDestroy");

    if (ppa == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }
    if ((pa = *ppa) == NULL)
        return;

    ptraGetActualCount(pa, &nactual);
    if (nactual > 0) {
        if (freeflag) {
            for (i = 0; i < pa->n; i++) {
                if ((item = ptraRemove(pa, i, L_NO_COMPACTION)) != NULL)
                    FREE(item);
            }
        }
        else
            L_WARNING_INT("memory leak of %d items in ptra", procName, nactual);
    }

    FREE(pa->array);
    FREE(pa);
    *ppa = NULL;
    return;
}


/*--------------------------------------------------------------------------*
 *               Add/insert/remove/replace generic ptr object               *
 *--------------------------------------------------------------------------*/
/*!
 *  ptraAdd()
 *
 *      Input:  ptra
 *              item  (generic ptr to a struct)
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptraAdd(PTRA  *pa,
        void  *item)
{
l_int32  n;

    PROCNAME("ptraAdd");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    if (!item)
        return ERROR_INT("item not defined", procName, 1);
    
    ptraGetMaxIndex(pa, &n);
    if (n >= pa->nalloc && ptraExtendArray(pa))
        return ERROR_INT("extension failure", procName, 1);
    pa->array[n] = (void *)item;
    pa->n++;
    pa->nactual++;
    return 0;
}


/*!
 *  ptraExtendArray()
 *
 *      Input:  ptra 
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptraExtendArray(PTRA  *pa)
{
    PROCNAME("ptraExtendArray");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);

    if ((pa->array = (void **)reallocNew((void **)&pa->array,
                                sizeof(l_intptr_t) * pa->nalloc,
                                2 * sizeof(l_intptr_t) * pa->nalloc)) == NULL)
            return ERROR_INT("new ptr array not returned", procName, 1);

    pa->nalloc *= 2;
    return 0;
}


/*!
 *  ptraInsert()
 *
 *      Input:  ptra 
 *              index (location in ptra to insert new value)
 *              item  (generic ptr to a struct; can be null)
 *              shiftflag (L_AUTO_DOWNSHIFT, L_MIN_DOWNSHIFT, L_FULL_DOWNSHIFT)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This checks first to see if the location is valid, and
 *          then if there is presently an item there.  If there is not,
 *          it is simply inserted into that location.
 *      (2) If there is an item at the insert location, items must be
 *          moved down to make room for the insert.  In the downward
 *          shift there are three options, given by @shiftflag.
 *            - If @shiftflag == L_AUTO_DOWNSHIFT, a decision is made
 *              whether, in a cascade of items, to downshift a minimum
 *              amount or for all items above @index.  The decision is
 *              based on the expectation of finding holes (null ptrs)
 *              between @index and the bottom of the array.
 *              Assuming the holes are distributed uniformly, if 2 or more
 *              holes are expected, we do a minimum shift.
 *            - If @shiftflag == L_MIN_DOWNSHIFT, the downward shifting
 *              cascade of items progresses a minimum amount, until
 *              the first empty slot is reached.  This mode requires
 *              some computation before the actual shifting is done.
 *            - If @shiftflag == L_FULL_DOWNSHIFT, a shifting cascade is
 *              performed where pa[i] --> pa[i + 1] for all i >= index.
 *              Then, the item is inserted at pa[index].
 *      (3) If you are not using L_AUTO_DOWNSHIFT, the rule of thumb is
 *          to use L_FULL_DOWNSHIFT if the array is compacted (each
 *          element points to an item), and to use L_MIN_DOWNSHIFT
 *          if there are a significant number of null pointers.
 *          There is no penalty to using L_MIN_DOWNSHIFT for a
 *          compacted array, however, because the full shift is required
 *          and we don't do the O(n) computation to look for holes.
 *      (4) This should not be used repeatedly on large arrays,
 *          because the function is generally O(n).
 */
l_int32
ptraInsert(PTRA    *pa,
           l_int32  index,
           void    *item,
           l_int32  shiftflag)
{
l_int32    i, ihole, n;
l_float32  nexpected;

    PROCNAME("ptraInsert");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    ptraGetMaxIndex(pa, &n);
    if (index < 0 || index > n)
        return ERROR_INT("index not in [0 ... n]", procName, 1);
    if (shiftflag != L_AUTO_DOWNSHIFT && shiftflag != L_MIN_DOWNSHIFT &&
        shiftflag != L_FULL_DOWNSHIFT)
        return ERROR_INT("invalid shiftflag", procName, 1);

    if (item) pa->nactual++;
    if (index == pa->nalloc) {  /* can happen when index == n */
        if (ptraExtendArray(pa))
            return ERROR_INT("extension failure", procName, 1);
    }

        /* We are inserting into a hole or adding to the end of the array.
         * No existing items are moved. */
    if (pa->array[index] == NULL) {
        pa->array[index] = item;
        if (item && index == n)  /* if new item put at the end of the array */
            pa->n++;
        return 0;
    }

        /* We are inserting at the location of an existing item,
         * forcing the existing item and those below to shift down */
    if (n >= pa->nalloc && ptraExtendArray(pa))
        return ERROR_INT("extension failure", procName, 1);

        /* If there are no holes, do a full downshift.
         * Otherwise, if L_AUTO_DOWNSHIFT, use the expected number
         * of holes between index and n to determine the shift mode */
    if (n == pa->nactual)
        shiftflag = L_FULL_DOWNSHIFT;
    else if (L_AUTO_DOWNSHIFT) {
        if (n < 10)
            shiftflag = L_FULL_DOWNSHIFT;  /* no big deal */
        else {
            nexpected = (l_float32)(n - pa->nactual) *
                         (l_float32)((n - index) / n);
            shiftflag = (nexpected > 2.0) ? L_MIN_DOWNSHIFT : L_FULL_DOWNSHIFT;
        }
    }

    if (shiftflag == L_MIN_DOWNSHIFT) {  /* run down looking for a hole */
        for (ihole = index + 1; ihole < n; ihole++) {
             if (pa->array[ihole] == NULL)
                 break;
        }
    }
    else   /* L_FULL_DOWNSHIFT */
        ihole = n;

    for (i = ihole; i > index; i--)
        pa->array[i] = pa->array[i - 1];
    pa->array[index] = (void *)item;
    if (ihole == n)  /* the last item was shifted down */
        pa->n++;

    return 0;
}


/*!
 *  ptraRemove()
 *
 *      Input:  ptra
 *              index (element to be removed)
 *              flag (L_NO_COMPACTION, L_COMPACTION)
 *      Return: item, or null on error
 *
 *  Notes:
 *      (1) If flag == L_NO_COMPACTION, this removes the item and
 *          nulls the ptr on the array.  If it takes the last item
 *          in the array, pa->n is reduced to the next item.
 *      (2) If flag == L_COMPACTION, this compacts the array for
 *          for all i >= index.  It should not be used repeatedly on
 *          large arrays, because compaction is O(n).
 *      (3) The ability to remove without automatic compaction allows
 *          removal with cost O(1).
 */
void *
ptraRemove(PTRA    *pa,
           l_int32  index,
           l_int32  flag)
{
l_int32  i, n, fromend, icurrent;
void    *item;

    PROCNAME("ptraRemove");

    if (!pa)
        return (void *)ERROR_PTR("pa not defined", procName, NULL);
    ptraGetMaxIndex(pa, &n);
    if (index < 0 || index >= n)
        return (void *)ERROR_PTR("index not in [0 ... n-1]", procName, NULL);

    item = pa->array[index];
    if (item)
        pa->nactual--;
    pa->array[index] = NULL;
  
        /* If we took the last item, need to reduce pa->n */
    fromend = (index == n - 1);
    if (fromend) {
        for (i = index - 1; i >= 0; i--) {
            if (pa->array[i])
                break;
        }
        pa->n = i + 1;
        n = i + 1;
    }

        /* Compact from index to the end of the array */
    if (!fromend && flag == L_COMPACTION) {
        for (icurrent = index, i = index + 1; i < n; i++) {
            if (pa->array[i])
                pa->array[icurrent++] = pa->array[i];
        }
        pa->n = icurrent;
    }
    return item;
}


/*!
 *  ptraReplace()
 *
 *      Input:  ptra
 *              index (element to be replaced)
 *              item  (new generic ptr to a struct; can be null)
 *              freeflag (TRUE to free old item; FALSE to return it)
 *      Return: item  (old item, if it exists and is not freed),
 *                     or null on error
 */
void *
ptraReplace(PTRA    *pa,
            l_int32  index,
            void    *item,
            l_int32  freeflag)
{
l_int32  n;
void    *olditem;

    PROCNAME("ptraReplace");

    if (!pa)
        return (void *)ERROR_PTR("pa not defined", procName, NULL);
    ptraGetMaxIndex(pa, &n);
    if (index < 0 || index >= n)
        return (void *)ERROR_PTR("index not in [0 ... n-1]", procName, NULL);

    olditem = pa->array[index];
    pa->array[index] = item;
    if (!item && olditem)
        pa->nactual--;
    else if (item && !olditem)
        pa->nactual++;

    if (freeflag == FALSE)
        return olditem;

    if (olditem)
        FREE(olditem);
    return NULL;
}


/*!
 *  ptraSwap()
 *
 *      Input:  ptra
 *              index1
 *              index2
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptraSwap(PTRA    *pa,
         l_int32  index1,
         l_int32  index2)
{
l_int32  n;
void    *item;

    PROCNAME("ptraSwap");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    if (index1 == index2)
        return 0;
    ptraGetMaxIndex(pa, &n);
    if (index1 < 0 || index1 >= n || index2 < 0 || index2 >= n)
        return ERROR_INT("invalid index: not in [0 ... n-1]", procName, 1);

    item = ptraRemove(pa, index1, L_NO_COMPACTION);
    item = ptraReplace(pa, index2, item, FALSE);
    ptraInsert(pa, index1, item, L_MIN_DOWNSHIFT);
    return 0;
}


/*!
 *  ptraCompactArray()
 *
 *      Input:  ptra 
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This compacts the items on the array, filling any empty ptrs.
 *      (2) This does not change the size of the array of ptrs.
 */
l_int32
ptraCompactArray(PTRA  *pa)
{
l_int32  i, n, nactual, index;

    PROCNAME("ptraCompactArray");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    ptraGetMaxIndex(pa, &n);
    ptraGetActualCount(pa, &nactual);
    if (n == nactual) return 0;

        /* Compact the array */
    for (i = 0, index = 0; i < n; i++) {
        if (pa->array[i])
             pa->array[index++] = pa->array[i];
    }
    pa->n = index;
    if (nactual != index)
        L_ERROR_INT("index = %d; != nactual", procName, index);

    return 0;
}


/*----------------------------------------------------------------------*
 *                        Other array operations                        *
 *----------------------------------------------------------------------*/
/*!
 *  ptraReverse()
 *
 *      Input:  ptra
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptraReverse(PTRA  *pa)
{
l_int32  i, n;

    PROCNAME("ptraReverse");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    ptraGetMaxIndex(pa, &n);

    for (i = 0; i < n / 2; i++)
        ptraSwap(pa, i, n - i - 1);
    return 0;
}


/*!
 *  ptraJoin()
 *
 *      Input:  ptra1 (add to this one)
 *              ptra2 (appended to ptra1, and emptied of items; can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptraJoin(PTRA  *pa1,
         PTRA  *pa2)
{
l_int32  i, n;
void    *item;

    PROCNAME("ptraJoin");

    if (!pa1)
        return ERROR_INT("pa1 not defined", procName, 1);
    if (!pa2)
        return 0;

    ptraGetMaxIndex(pa2, &n);
    for (i = 0; i < n; i++) {
        item = ptraRemove(pa2, i, L_NO_COMPACTION);
        ptraAdd(pa1, item);
    }
    
    ptraGetMaxIndex(pa2, &n);
    fprintf(stderr, "n = %d\n", n);

    return 0;
}



/*----------------------------------------------------------------------*
 *                        Simple ptra accessors                         *
 *----------------------------------------------------------------------*/
/*!
 *  ptraGetMaxIndex()
 *
 *      Input:  ptra
 *              &maxindex (<return> 1 + index of last item in the array);
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The largest index to an item in the array is @maxindex - 1.
 *          @maxindex is the number of items that would be in the array
 *          if there were no null pointers between 0 and @maxindex - 1.
 *          However, because the internal ptr array need not be compacted,
 *          there may be null pointers at indices below @maxindex - 1;
 *          for example, if items have been removed.
 *      (2) When an item is added to the end of the array, it goes
 *          into pa->array[maxindex], and maxindex is then incremented by 1.
 */
l_int32
ptraGetMaxIndex(PTRA     *pa,
                l_int32  *pmaxindex)
{
    PROCNAME("ptraGetMaxIndex");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    if (!pmaxindex)
        return ERROR_INT("&maxindex not defined", procName, 1);
    *pmaxindex = pa->n;
    return 0;
}
        

/*!
 *  ptraGetActualCount()
 *
 *      Input:  ptra
 *              &count (<return> actual number of items on the ptr array)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) The actual number of items on the ptr array, pa->nactual,
 *          will be smaller than pa->n if the array is not compacted.
 */
l_int32
ptraGetActualCount(PTRA     *pa,
                   l_int32  *pcount)
{
    PROCNAME("ptraGetActualCount");

    if (!pa)
        return ERROR_INT("pa not defined", procName, 1);
    if (!pcount)
        return ERROR_INT("&count not defined", procName, 1);
    *pcount = pa->nactual;

    return 0;
}


/*!
 *  ptraGetPtrToItem()
 *
 *      Input:  ptra
 *              index (element to fetch pointer to)
 *      Return: item (just a pointer to it)
 *
 *  Notes:
 *      (1) The item remains on the Ptra and is 'owned' by it, so
 *          the item must not be destroyed.
 */
void *
ptraGetPtrToItem(PTRA    *pa,
                 l_int32  index)
{

    PROCNAME("ptraGetPtrToItem");

    if (!pa)
        return (void *)ERROR_PTR("pa not defined", procName, NULL);
    if (index < 0 || index >= pa->n)
        return (void *)ERROR_PTR("index not in [0 ... n-1]", procName, NULL);

    return pa->array[index];
}

