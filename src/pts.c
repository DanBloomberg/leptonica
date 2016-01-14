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
 *   pts.c
 *
 *      Pta creation, destruction, copy, clone, empty
 *           PTA      *ptaCreate()
 *           void      ptaDestroy()
 *           PTA      *ptaCopy()
 *           PTA      *ptaClone()
 *           l_int32   ptaEmpty()
 *
 *      Pta array extension
 *           l_int32   ptaAddPt()
 *           l_int32   ptaExtendArrays()
 *
 *      Pta rearrangements
 *           l_int32   ptaJoin()
 *           PTA      *ptaReverse()
 *           PTA      *ptaCyclicPerm()
 *           PTA      *ptaSort()
 *           PTA      *ptaRemoveDuplicates()
 *
 *      Pta Accessors
 *           l_int32   ptaGetRefcount()
 *           l_int32   ptaChangeRefcount()
 *           l_int32   ptaGetCount()
 *           l_int32   ptaGetPt()
 *           l_int32   ptaGetIPt()
 *           l_int32   ptaGetArrays()
 *
 *      Ptaa creation, destruction
 *           PTAA     *ptaaCreate()
 *           void      ptaaDestroy()
 *
 *      Ptaa array extension
 *           l_int32   ptaaAddPta()
 *           l_int32   ptaaExtendArray()
 *
 *      Ptaa Accessors
 *           l_int32   ptaaGetCount()
 *           l_int32   ptaaGetPta()
 *  
 *      Ptaa serialized I/O
 *           PTAA     *ptaaRead()
 *           PTAA     *ptaaReadStream()
 *           l_int32   ptaaWrite()
 *           l_int32   ptaaWriteStream()
 *
 *      Pta serialized I/O
 *           PTA      *ptaRead()
 *           PTA      *ptaReadStream()
 *           l_int32   ptaWrite()
 *           l_int32   ptaWriteStream()
 *
 *      In use
 *           BOX      *ptaGetExtent()
 *           PTA      *ptaGetInsideBox()
 *           PTA      *pixFindCornerPixels()
 *           l_int32   pixPlotAlongPta()
 *           l_int32   ptaContainsPt()
 *           l_int32   ptaTestIntersection()
 *           PTA      *ptaTransform()
 *           PTA      *ptaSubsample()
 *           l_int32   ptaGetLinearLSF()
 *           PTA      *ptaGetPixelsFromPix()
 *           PIX      *pixGenerateFromPta()
 *           PTA      *ptaGetBoundaryPixels()
 *           PTAA     *ptaaGetBoundaryPixels()
 *
 *      Display Pta and Ptaa
 *           PIX      *pixDisplayPta()
 *           PIX      *pixDisplayPtaa()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

static const l_int32  INITIAL_PTR_ARRAYSIZE = 20;   /* n'import quoi */

    /* Default spreading factor for hashing pts in a plane */
static const l_int32  DEFAULT_SPREADING_FACTOR = 7500;


/*---------------------------------------------------------------------*
 *                Pta creation, destruction, copy, clone               *
 *---------------------------------------------------------------------*/
/*!
 *  ptaCreate()
 *
 *      Input:  n  (initial array sizes)
 *      Return: pta, or null on error.
 */
PTA *
ptaCreate(l_int32  n)
{
PTA  *pta;

    PROCNAME("ptaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((pta = (PTA *)CALLOC(1, sizeof(PTA))) == NULL)
        return (PTA *)ERROR_PTR("pta not made", procName, NULL);
    pta->n = 0;
    pta->nalloc = n;
    ptaChangeRefcount(pta, 1);  /* sets to 1 */
    
    if ((pta->x = (l_float32 *)CALLOC(n, sizeof(l_float32))) == NULL)
        return (PTA *)ERROR_PTR("x array not made", procName, NULL);
    if ((pta->y = (l_float32 *)CALLOC(n, sizeof(l_float32))) == NULL)
        return (PTA *)ERROR_PTR("y array not made", procName, NULL);

    return pta;
}


/*!
 *  ptaDestroy()
 *
 *      Input:  &pta (<to be nulled>)
 *      Return: void
 *
 *  Note:
 *      - Decrements the ref count and, if 0, destroys the pta.
 *      - Always nulls the input ptr.
 */
void
ptaDestroy(PTA  **ppta)
{
PTA  *pta;

    PROCNAME("ptaDestroy");

    if (ppta == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((pta = *ppta) == NULL)
        return;

    ptaChangeRefcount(pta, -1);
    if (ptaGetRefcount(pta) <= 0) {
        FREE(pta->x);
        FREE(pta->y);
        FREE(pta);
    }

    *ppta = NULL;
    return;
}


/*!
 *  ptaCopy()
 *
 *      Input:  pta
 *      Return: copy of pta, or null on error
 */
PTA *
ptaCopy(PTA  *pta)
{
l_int32    i;
l_float32  x, y;
PTA       *npta;

    PROCNAME("ptaCopy");

    if (!pta)
        return (PTA *)ERROR_PTR("pta not defined", procName, NULL);

    if ((npta = ptaCreate(pta->nalloc)) == NULL)
        return (PTA *)ERROR_PTR("npta not made", procName, NULL);

    for (i = 0; i < pta->n; i++) {
        ptaGetPt(pta, i, &x, &y);
        ptaAddPt(npta, x, y);
    }

    return npta;
}


/*!
 *  ptaClone()
 *
 *      Input:  pta
 *      Return: ptr to same pta, or null on error
 */
PTA *
ptaClone(PTA  *pta)
{
    PROCNAME("ptaClone");

    if (!pta)
        return (PTA *)ERROR_PTR("pta not defined", procName, NULL);

    ptaChangeRefcount(pta, 1);
    return pta;
}


/*!
 *  ptaEmpty()
 *
 *      Input:  pta
 *      Return: 0 if OK, 1 on error
 *
 *  Note: this only resets the "n" field, for reuse
 */
l_int32
ptaEmpty(PTA  *pta)
{
    PROCNAME("ptaEmpty");

    if (!pta)
        return ERROR_INT("ptad not defined", procName, 1);
    pta->n = 0;
    return 0;
}


/*---------------------------------------------------------------------*
 *                         Pta array extension                         *
 *---------------------------------------------------------------------*/
/*!
 *  ptaAddPt()
 *
 *      Input:  pta 
 *              x, y
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptaAddPt(PTA       *pta,
         l_float32  x,
         l_float32  y)
{
l_int32  n;

    PROCNAME("ptaAddPt");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    n = pta->n;
    if (n >= pta->nalloc)
        ptaExtendArrays(pta);
    pta->x[n] = x;
    pta->y[n] = y;
    pta->n++;

    return 0;
}


/*!
 *  ptaExtendArrays()
 *
 *      Input:  pta
 *      Return: 0 if OK; 1 on error
 */
l_int32
ptaExtendArrays(PTA  *pta)
{
    PROCNAME("ptaExtendArrays");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    if ((pta->x = (l_float32 *)reallocNew((void **)&pta->x,
                               sizeof(l_float32) * pta->nalloc,
                               2 * sizeof(l_float32) * pta->nalloc)) == NULL)
        return ERROR_INT("new x array not returned", procName, 1);
    if ((pta->y = (l_float32 *)reallocNew((void **)&pta->y,
                               sizeof(l_float32) * pta->nalloc,
                               2 * sizeof(l_float32) * pta->nalloc)) == NULL)
        return ERROR_INT("new y array not returned", procName, 1);

    pta->nalloc = 2 * pta->nalloc;
    return 0;
}



/*---------------------------------------------------------------------*
 *                           Pta rearrangements                        *
 *---------------------------------------------------------------------*/
/*!
 *  ptaJoin()
 *
 *      Input:  ptad  (dest pta; add to this one)
 *              ptas  (source pta; add from this one)
 *              istart  (starting index in ptas)
 *              iend  (ending index in ptas; use 0 to cat all)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) istart < 0 is taken to mean 'read from the start' (istart = 0)
 *      (2) iend <= 0 means 'read to the end'
 */
l_int32
ptaJoin(PTA     *ptad,
        PTA     *ptas,
        l_int32  istart,
        l_int32  iend)
{
l_int32  ns, i, x, y;

    PROCNAME("ptaJoin");

    if (!ptad)
        return ERROR_INT("ptad not defined", procName, 1);
    if (!ptas)
        return ERROR_INT("ptas not defined", procName, 1);
    ns = ptaGetCount(ptas);
    if (istart < 0)
        istart = 0;
    if (istart >= ns)
        return ERROR_INT("istart out of bounds", procName, 1);
    if (iend <= 0)
        iend = ns - 1;
    if (iend >= ns)
        return ERROR_INT("iend out of bounds", procName, 1);
    if (istart > iend)
        return ERROR_INT("istart > iend; no pts", procName, 1);

    for (i = istart; i <= iend; i++) {
        ptaGetIPt(ptas, i, &x, &y);
        ptaAddPt(ptad, x, y);
    }

    return 0;
}


/*!
 *  ptaReverse()
 *
 *      Input:  ptas
 *              type  (0 for float values; 1 for integer values)
 *      Return: ptad (reversed pta), or null on error
 */
PTA  *
ptaReverse(PTA     *ptas,
           l_int32  type)
{
l_int32    n, i, ix, iy;
l_float32  x, y;
PTA       *ptad;

    PROCNAME("ptaReverse");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);

    n = ptaGetCount(ptas);
    if ((ptad = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("ptad not made", procName, NULL);
    for (i = n - 1; i >= 0; i--) {
        if (type == 0) {
            ptaGetPt(ptas, i, &x, &y);
            ptaAddPt(ptad, x, y);
        }
        else {  /* type == 1 */
            ptaGetIPt(ptas, i, &ix, &iy);
            ptaAddPt(ptad, ix, iy);
        }
    }

    return ptad;
}


/*!
 *  ptaCyclicPerm()
 *
 *      Input:  ptas
 *              xs, ys  (start point; must be in ptas)
 *      Return: ptad (cyclic permutation, starting and ending at (xs, ys),
 *              or null on error
 *
 *  Notes:
 *      (1) Check to insure that (a) ptas is a closed path where
 *          the first and last points are identical, and (b) the
 *          resulting pta also starts and ends on the same point
 *          (which in this case is (xs, ys).
 */
PTA  *
ptaCyclicPerm(PTA     *ptas,
              l_int32  xs,
              l_int32  ys)
{
l_int32  n, i, x, y, j, index, state;
l_int32  x1, y1, x2, y2;
PTA     *ptad;

    PROCNAME("ptaCyclicPerm");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);

    n = ptaGetCount(ptas);

        /* Verify input data */
    ptaGetIPt(ptas, 0, &x1, &y1);
    ptaGetIPt(ptas, n - 1, &x2, &y2);
    if (x1 != x2 || y1 != y2)
        return (PTA *)ERROR_PTR("start and end pts not same", procName, NULL);
    state = L_NOT_FOUND;
    for (i = 0; i < n; i++) {
        ptaGetIPt(ptas, i, &x, &y);
        if (x == xs && y == ys) {
            state = L_FOUND;
            break;
        }
    }
    if (state == L_NOT_FOUND)
        return (PTA *)ERROR_PTR("start pt not in ptas", procName, NULL);

    if ((ptad = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("ptad not made", procName, NULL);
    for (j = 0; j < n - 1; j++) {
        if (i + j < n - 1)
            index = i + j;
        else
            index = (i + j + 1) % n;
        ptaGetIPt(ptas, index, &x, &y);
        ptaAddPt(ptad, x, y);
    }
    ptaAddPt(ptad, xs, ys);

    return ptad;
}


/*!
 *  ptaSort()
 * 
 *      Input:  ptas
 *              sorttype (L_SORT_BY_X, L_SORT_BY_Y)
 *              sortorder  (L_SORT_INCREASING, L_SORT_DECREASING)
 *              &naindex (<optional return> index of sorted order into
 *                        original array)
 *      Return: ptad (sorted version of ptas), or null on error
 */
PTA *
ptaSort(PTA     *ptas,
        l_int32  sorttype,
        l_int32  sortorder,
        NUMA   **pnaindex)
{
l_int32    i, index, n;
l_float32  x, y;
PTA       *ptad;
NUMA      *na, *naindex;

    PROCNAME("ptaSort");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);
    if (sorttype != L_SORT_BY_X && sorttype != L_SORT_BY_Y)
        return (PTA *)ERROR_PTR("invalid sort type", procName, NULL);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return (PTA *)ERROR_PTR("invalid sort order", procName, NULL);

        /* Build up numa of specific data */
    n = ptaGetCount(ptas);
    if ((na = numaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("na not made", procName, NULL);
    for (i = 0; i < n; i++) {
        ptaGetPt(ptas, i, &x, &y);
        if (sorttype == L_SORT_BY_X)
            numaAddNumber(na, x);
        else
            numaAddNumber(na, y);
    }

        /* Get the sort index for data array */
    if ((naindex = numaGetSortIndex(na, sortorder)) == NULL)
        return (PTA *)ERROR_PTR("naindex not made", procName, NULL);

        /* Build up sorted pta using sort index */
    if ((ptad = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("ptad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        ptaGetPt(ptas, index, &x, &y);
        ptaAddPt(ptad, x, y);
    }

    if (pnaindex)
        *pnaindex = naindex;
    else
        numaDestroy(&naindex);
    numaDestroy(&na);
    return ptad;
}


/*!
 *  ptaRemoveDuplicates()
 * 
 *      Input:  ptas (assumed to be integer values)
 *              factor (should be larger than the largest point value;
 *                      use 0 for default)
 *      Return: ptad (with duplicates removed), or null on error
 */
PTA *
ptaRemoveDuplicates(PTA      *ptas,
                    l_uint32  factor)
{
l_int32    nsize, i, j, k, index, n, nvals;
l_int32    x, y, xk, yk;
l_int32   *ia;
PTA       *ptad;
NUMA      *na;
NUMAHASH  *nahash;

    PROCNAME("ptaRemoveDuplicates");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);
    if (factor == 0)
        factor = DEFAULT_SPREADING_FACTOR;

        /* Build up numaHash of indices, hashed by a key that is
         * a large linear combination of x and y values designed to
         * randomize the key. */
    nsize = 5507;  /* buckets in hash table; prime */
    nahash = numaHashCreate(nsize, 2);
    n = ptaGetCount(ptas);
    for (i = 0; i < n; i++) {
        ptaGetIPt(ptas, i, &x, &y);
        numaHashAdd(nahash, factor * x + y, (l_float32)i);
    }

    if ((ptad = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("ptad not made", procName, NULL);
    for (i = 0; i < nsize; i++) {
        na = numaHashGetNuma(nahash, i);
        if (!na) continue;

        nvals = numaGetCount(na);
            /* If more than 1 pt, compare exhaustively with double loop;
             * otherwise, just enter it. */
        if (nvals > 1) {
            if ((ia = (l_int32 *)CALLOC(nvals, sizeof(l_int32))) == NULL)
                return (PTA *)ERROR_PTR("ia not made", procName, NULL);
            for (j = 0; j < nvals; j++) {
                if (ia[j] == 1) continue;
                numaGetIValue(na, j, &index);
                ptaGetIPt(ptas, index, &x, &y);
                ptaAddPt(ptad, x, y);
                for (k = j + 1; k < nvals; k++) {
                    if (ia[k] == 1) continue;
                    numaGetIValue(na, k, &index);
                    ptaGetIPt(ptas, index, &xk, &yk);
                    if (x == xk && y == yk)  /* duplicate */
                        ia[k] = 1;
                }
            }
            FREE(ia);
        }
        else {
            numaGetIValue(na, 0, &index);
            ptaGetIPt(ptas, index, &x, &y);
            ptaAddPt(ptad, x, y);
        }
        numaDestroy(&na);  /* the clone */
    }

    numaHashDestroy(&nahash);
    return ptad;
}



/*---------------------------------------------------------------------*
 *                           Pta accessors                             *
 *---------------------------------------------------------------------*/
l_int32
ptaGetRefcount(PTA  *pta)
{
    PROCNAME("ptaGetRefcount");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);
    return pta->refcount;
}


l_int32
ptaChangeRefcount(PTA     *pta,
                  l_int32  delta)
{
    PROCNAME("ptaChangeRefcount");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);
    pta->refcount += delta;
    return 0;
}


/*!
 *  ptaGetCount()
 *
 *      Input:  pta
 *      Return: count, or 0 if no pta
 */
l_int32
ptaGetCount(PTA  *pta)
{
    PROCNAME("ptaGetCount");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 0);

    return pta->n;
}


/*!
 *  ptaGetPt()
 *
 *      Input:  pta
 *              index  (into arrays)
 *              &x, &y  (<return> float values)
 *      Return: 0 if OK; 1 on error
 */
l_int32
ptaGetPt(PTA        *pta,
         l_int32     index,
         l_float32  *px,
         l_float32  *py)
{
    PROCNAME("ptaGetPt");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    *px = pta->x[index];
    *py = pta->y[index];
    return 0;
}


/*!
 *  ptaGetIPt()
 *
 *      Input:  pta
 *              index  (into arrays)
 *              &x, &y  (<return> integer values)
 *      Return: 0 if OK; 1 on error
 */
l_int32
ptaGetIPt(PTA      *pta,
          l_int32   index,
          l_int32  *px,
          l_int32  *py)
{
    PROCNAME("ptaGetIPt");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    *px = (l_int32)(pta->x[index] + 0.5);
    *py = (l_int32)(pta->y[index] + 0.5);
    return 0;
}


/*!
 *  ptaGetArrays()
 *
 *      Input:  pta
 *              &nax, &nay  (<return> numas of x and y arrays)
 *      Return: 0 if OK; 1 on error or if pta is empty
 *
 *  Notes:
 *      (1) This copies the internal arrays into new Numas, and returns them.
 *      (2) Manipulates internal arrays in pta and numa directly.
 */
l_int32
ptaGetArrays(PTA    *pta,
             NUMA  **pnax,
             NUMA  **pnay)
{
l_int32   i, n;
NUMA     *nax, *nay;

    PROCNAME("ptaGetArrays");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);
    if (!pnax)
        return ERROR_INT("&nax not defined", procName, 1);
    if (!pnay)
        return ERROR_INT("&nay not defined", procName, 1);

    *pnax = *pnay = NULL;
    if ((n = ptaGetCount(pta)) == 0)
        return ERROR_INT("pta is empty", procName, 1);

    if ((nax = numaCreate(n)) == NULL)
        return ERROR_INT("nax not made", procName, 1);
    *pnax = nax;
    if ((nay = numaCreate(n)) == NULL)
        return ERROR_INT("nay not made", procName, 1);
    *pnay = nay;

        /* Use arrays directly for efficiency */
    for (i = 0; i < n; i++) {
        nax->array[i] = pta->x[i];
        nay->array[i] = pta->y[i];
    }
    nax->n = nay->n = n;
        
    return 0;
}



/*---------------------------------------------------------------------*
 *                     PTAA creation, destruction                      *
 *---------------------------------------------------------------------*/
/*!
 *  ptaaCreate()
 *
 *      Input:  n  (initial number of ptrs)
 *      Return: ptaa, or null on error
 */
PTAA *
ptaaCreate(l_int32  n)
{
PTAA  *ptaa;

    PROCNAME("ptaaCreate");

    if (n <= 0)
        n = INITIAL_PTR_ARRAYSIZE;

    if ((ptaa = (PTAA *)CALLOC(1, sizeof(PTAA))) == NULL)
        return (PTAA *)ERROR_PTR("ptaa not made", procName, NULL);
    ptaa->n = 0;
    ptaa->nalloc = n;

    if ((ptaa->pta = (PTA **)CALLOC(n, sizeof(PTA *))) == NULL)
        return (PTAA *)ERROR_PTR("pta ptrs not made", procName, NULL);
    
    return ptaa;
}


/*!
 *  ptaaDestroy()
 *
 *      Input:  &ptaa <to be nulled>
 *      Return: void
 */
void
ptaaDestroy(PTAA  **pptaa)
{
l_int32  i;
PTAA    *ptaa;

    PROCNAME("ptaaDestroy");

    if (pptaa == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((ptaa = *pptaa) == NULL)
        return;

    for (i = 0; i < ptaa->n; i++) 
        ptaDestroy(&ptaa->pta[i]);
    FREE(ptaa->pta);

    FREE(ptaa);
    *pptaa = NULL;
    return;
}


/*---------------------------------------------------------------------*
 *                          PTAA array extension                       *
 *---------------------------------------------------------------------*/
/*!
 *  ptaaAddPta()
 *
 *      Input:  ptaa
 *              pta  (to be added)
 *              copyflag  (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptaaAddPta(PTAA    *ptaa,
           PTA     *pta,
           l_int32  copyflag)
{
l_int32  n;
PTA     *ptac;

    PROCNAME("ptaaAddPta");

    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);
    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    if (copyflag == L_INSERT)
        ptac = pta;
    else if (copyflag == L_COPY) {
        if ((ptac = ptaCopy(pta)) == NULL)
            return ERROR_INT("ptac not made", procName, 1);
    }
    else if (copyflag == L_CLONE) {
        if ((ptac = ptaClone(pta)) == NULL)
            return ERROR_INT("pta clone not made", procName, 1);
    }
    else
        return ERROR_INT("invalid copyflag", procName, 1);

    n = ptaaGetCount(ptaa);
    if (n >= ptaa->nalloc)
        ptaaExtendArray(ptaa);
    ptaa->pta[n] = ptac;
    ptaa->n++;

    return 0;
}


/*!
 *  ptaaExtendArray()
 *
 *      Input:  ptaa
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptaaExtendArray(PTAA  *ptaa)
{
    PROCNAME("ptaaExtendArray");

    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);

    if ((ptaa->pta = (PTA **)reallocNew((void **)&ptaa->pta,
                             sizeof(PTA *) * ptaa->nalloc,
                             2 * sizeof(PTA *) * ptaa->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);

    ptaa->nalloc = 2 * ptaa->nalloc;
    return 0;
}


/*---------------------------------------------------------------------*
 *                          Ptaa accessors                             *
 *---------------------------------------------------------------------*/
/*!
 *  ptaaGetCount()
 *
 *      Input:  ptaa
 *      Return: count, or 0 if no ptaa
 */
l_int32
ptaaGetCount(PTAA  *ptaa)
{
    PROCNAME("ptaaGetCount");

    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 0);

    return ptaa->n;
}


/*!
 *  ptaaGetPta()
 *
 *      Input:  ptaa
 *              index  (to the i-th pta)
 *              accessflag  (L_COPY or L_CLONE)
 *      Return: pta, or null on error
 */
PTA *
ptaaGetPta(PTAA    *ptaa,
           l_int32  index,
           l_int32  accessflag)
{
    PROCNAME("ptaaGetPta");

    if (!ptaa)
        return (PTA *)ERROR_PTR("ptaa not defined", procName, NULL);
    if (index < 0 || index >= ptaa->n)
        return (PTA *)ERROR_PTR("index not valid", procName, NULL);

    if (accessflag == L_COPY)
        return ptaCopy(ptaa->pta[index]);
    else if (accessflag == L_CLONE)
        return ptaClone(ptaa->pta[index]);
    else
        return (PTA *)ERROR_PTR("invalid accessflag", procName, NULL);
}



/*---------------------------------------------------------------------*
 *                         Ptaa serialized I/O                         *
 *---------------------------------------------------------------------*/
/*!
 *  ptaaRead()
 *
 *      Input:  filename
 *      Return: ptaa, or null on error
 */
PTAA *
ptaaRead(const char  *filename)
{
FILE  *fp;
PTAA  *ptaa;

    PROCNAME("ptaaRead");

    if (!filename)
        return (PTAA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (PTAA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((ptaa = ptaaReadStream(fp)) == NULL) {
        fclose(fp);
        return (PTAA *)ERROR_PTR("ptaa not read", procName, NULL);
    }

    fclose(fp);
    return ptaa;
}


/*!
 *  ptaaReadStream()
 *
 *      Input:  stream
 *      Return: ptaa, or null on error
 */
PTAA *
ptaaReadStream(FILE  *fp)
{
l_int32  i, n, version;
PTA     *pta;
PTAA    *ptaa;

    PROCNAME("ptaaReadStream");

    if (!fp)
        return (PTAA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nPtaa Version %d\n", &version) != 1)
        return (PTAA *)ERROR_PTR("not a ptaa file", procName, NULL);
    if (version != PTA_VERSION_NUMBER)
        return (PTAA *)ERROR_PTR("invalid ptaa version", procName, NULL);
    if (fscanf(fp, "Number of Pta = %d\n", &n) != 1)
        return (PTAA *)ERROR_PTR("not a ptaa file", procName, NULL);

    if ((ptaa = ptaaCreate(n)) == NULL)
        return (PTAA *)ERROR_PTR("ptaa not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if ((pta = ptaReadStream(fp)) == NULL)
            return (PTAA *)ERROR_PTR("error reading pta", procName, NULL);
        ptaaAddPta(ptaa, pta, L_INSERT);
    }

    return ptaa;
}


/*!
 *  ptaaWrite()
 *
 *      Input:  filename
 *              ptaa
 *              type  (0 for float values; 1 for integer values)
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptaaWrite(const char  *filename,
          PTAA        *ptaa,
          l_int32      type)
{
FILE  *fp;

    PROCNAME("ptaaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (ptaaWriteStream(fp, ptaa, type))
        return ERROR_INT("ptaa not written to stream", procName, 1);
    fclose(fp);

    return 0;
}


/*!
 *  ptaaWriteStream()
 *
 *      Input:  stream
 *              ptaa
 *              type  (0 for float values; 1 for integer values)
 *      Return: 0 if OK; 1 on error
 */
l_int32
ptaaWriteStream(FILE    *fp,
                PTAA    *ptaa,
                l_int32  type)
{
l_int32  i, n;
PTA     *pta;

    PROCNAME("ptaaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);

    n = ptaaGetCount(ptaa);
    fprintf(fp, "\nPtaa Version %d\n", PTA_VERSION_NUMBER);
    fprintf(fp, "Number of Pta = %d\n", n);
    for (i = 0; i < n; i++) {
        pta = ptaaGetPta(ptaa, i, L_CLONE);
        ptaWriteStream(fp, pta, type);
        ptaDestroy(&pta);
    }

    return 0;
}


/*---------------------------------------------------------------------*
 *                         Pta serialized I/O                          *
 *---------------------------------------------------------------------*/
/*!
 *  ptaRead()
 *
 *      Input:  filename
 *      Return: pta, or null on error
 */
PTA *
ptaRead(const char  *filename)
{
FILE  *fp;
PTA   *pta;

    PROCNAME("ptaRead");

    if (!filename)
        return (PTA *)ERROR_PTR("filename not defined", procName, NULL);
    if ((fp = fopenReadStream(filename)) == NULL)
        return (PTA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((pta = ptaReadStream(fp)) == NULL) {
        fclose(fp);
        return (PTA *)ERROR_PTR("pta not read", procName, NULL);
    }

    fclose(fp);
    return pta;
}


/*!
 *  ptaReadStream()
 *
 *      Input:  stream
 *      Return: pta, or null on error
 */
PTA *
ptaReadStream(FILE  *fp)
{
char       typestr[128];
l_int32    i, n, ix, iy, type, version;
l_float32  x, y;
PTA       *pta;

    PROCNAME("ptaReadStream");

    if (!fp)
        return (PTA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\n Pta Version %d\n", &version) != 1)
        return (PTA *)ERROR_PTR("not a pta file", procName, NULL);
    if (version != PTA_VERSION_NUMBER)
        return (PTA *)ERROR_PTR("invalid pta version", procName, NULL);
    if (fscanf(fp, " Number of pts = %d; format = %s\n", &n, typestr) != 2)
        return (PTA *)ERROR_PTR("not a pta file", procName, NULL);
    if (!strcmp(typestr, "float"))
        type = 0;
    else  /* typestr is "integer" */
        type = 1;

    if ((pta = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("pta not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if (type == 0) {  /* data is float */
            fscanf(fp, "   (%f, %f)\n", &x, &y);
            ptaAddPt(pta, x, y);
        }
        else {   /* data is integer */
            fscanf(fp, "   (%d, %d)\n", &ix, &iy);
            ptaAddPt(pta, ix, iy);
        }
    }

    return pta;
}


/*!
 *  ptaWrite()
 *
 *      Input:  filename
 *              pta
 *              type  (0 for float values; 1 for integer values)
 *      Return: 0 if OK, 1 on error
 */
l_int32
ptaWrite(const char  *filename,
         PTA         *pta,
         l_int32      type)
{
FILE  *fp;

    PROCNAME("ptaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (ptaWriteStream(fp, pta, type))
        return ERROR_INT("pta not written to stream", procName, 1);
    fclose(fp);

    return 0;
}


/*!
 *  ptaWriteStream()
 *
 *      Input:  stream
 *              pta
 *              type  (0 for float values; 1 for integer values)
 *      Return: 0 if OK; 1 on error
 */
l_int32
ptaWriteStream(FILE    *fp,
               PTA     *pta,
               l_int32  type)
{
l_int32    i, n, ix, iy;
l_float32  x, y;

    PROCNAME("ptaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);

    n = ptaGetCount(pta);
    fprintf(fp, "\n Pta Version %d\n", PTA_VERSION_NUMBER);
    if (type == 0)
        fprintf(fp, " Number of pts = %d; format = float\n", n);
    else  /* type == 1 */
        fprintf(fp, " Number of pts = %d; format = integer\n", n);
    for (i = 0; i < n; i++) {
        if (type == 0) {  /* data is float */
            ptaGetPt(pta, i, &x, &y);
            fprintf(fp, "   (%f, %f)\n", x, y);
        }
        else {   /* data is integer */
            ptaGetIPt(pta, i, &ix, &iy);
            fprintf(fp, "   (%d, %d)\n", ix, iy);
        }
    }

    return 0;
}



/*---------------------------------------------------------------------*
 *                                In use                               *
 *---------------------------------------------------------------------*/
/*!
 *  ptaGetExtent()
 *
 *      Input:  pta
 *      Return: box, or null on error
 *
 *  Notes:
 *      (1) Returns a box of minimum size containing pts in pta.
 */
BOX *
ptaGetExtent(PTA  *pta)
{
l_int32  n, i, x, y, minx, maxx, miny, maxy;

    PROCNAME("ptaGetExtent");

    if (!pta)
        return (BOX *)ERROR_PTR("pta not defined", procName, NULL);

    minx = 10000000;
    miny = 10000000;
    maxx = -10000000;
    maxy = -10000000;
    n = ptaGetCount(pta);
    for (i = 0; i < n; i++) {
        ptaGetIPt(pta, i, &x, &y);
        if (x < minx) minx = x;
        if (x > maxx) maxx = x;
        if (y < miny) miny = y;
        if (y > maxy) maxy = y;
    }

    return boxCreate(minx, miny, maxx - minx + 1, maxy - miny + 1);
}


/*!
 *  ptaGetInsideBox()
 *
 *      Input:  ptas (input pts)
 *              box
 *      Return: ptad (of pts in ptas that are inside the box), or null on error
 */
PTA *
ptaGetInsideBox(PTA  *ptas,
                BOX  *box)
{
PTA       *ptad;
l_int32    n, i, contains;
l_float32  x, y;

    PROCNAME("ptaGetInsideBox");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!box)
        return (PTA *)ERROR_PTR("box not defined", procName, NULL);

    n = ptaGetCount(ptas);
    ptad = ptaCreate(0);
    for (i = 0; i < n; i++) {
        ptaGetPt(ptas, i, &x, &y);
        boxContainsPt(box, x, y, &contains);
        if (contains)
            ptaAddPt(ptad, x, y);
    }

    return ptad;
}


/*!
 *  pixFindCornerPixels()
 *
 *      Input:  pixs (1 bpp)
 *      Return: pta, or null on error
 *
 *  Notes:
 *      (1) Finds the 4 corner-most pixels, as defined by a search
 *          inward from each corner, using a 45 degree line.
 */
PTA *
pixFindCornerPixels(PIX  *pixs)
{
l_int32    i, j, x, y, w, h, wpl, mindim, found;
l_uint32  *data, *line;
PTA       *pta;

    PROCNAME("pixFindCornerPixels");

    if (!pixs)
        return (PTA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 1)
        return (PTA *)ERROR_PTR("pixs not 1 bpp", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    mindim = L_MIN(w, h);
    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);

    if ((pta = ptaCreate(4)) == NULL)
        return (PTA *)ERROR_PTR("pta not made", procName, NULL);

    for (found = FALSE, i = 0; i < mindim; i++) {
        for (j = 0; j <= i; j++) {
            y = i - j;
            line = data + y * wpl;
            if (GET_DATA_BIT(line, j)) {
                ptaAddPt(pta, j, y);
                found = TRUE;
                break;
            }
        }
        if (found == TRUE)
            break;
    }

    for (found = FALSE, i = 0; i < mindim; i++) {
        for (j = 0; j <= i; j++) {
            y = i - j;
            line = data + y * wpl;
            x = w - 1 - j;
            if (GET_DATA_BIT(line, x)) {
                ptaAddPt(pta, x, y);
                found = TRUE;
                break;
            }
        }
        if (found == TRUE)
            break;
    }

    for (found = FALSE, i = 0; i < mindim; i++) {
        for (j = 0; j <= i; j++) {
            y = h - 1 - i + j;
            line = data + y * wpl;
            if (GET_DATA_BIT(line, j)) {
                ptaAddPt(pta, j, y);
                found = TRUE;
                break;
            }
        }
        if (found == TRUE)
            break;
    }

    for (found = FALSE, i = 0; i < mindim; i++) {
        for (j = 0; j <= i; j++) {
            y = h - 1 - i + j;
            line = data + y * wpl;
            x = w - 1 - j;
            if (GET_DATA_BIT(line, x)) {
                ptaAddPt(pta, x, y);
                found = TRUE;
                break;
            }
        }
        if (found == TRUE)
            break;
    }

    return pta;
}

                
/*!
 *  pixPlotAlongPta()
 *
 *      Input: pixs (any depth)
 *             pta (set of points on which to plot)
 *             outformat (GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11,
 *                        GPLOT_LATEX)
 *             title (<optional> for plot; can be null)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) We remove any existing colormap and clip the pta to the input pixs.
 *      (2) This is a debugging function, and does not remove temporary
 *          plotting files that it generates.
 *      (3) If the image is RGB, three separate plots are generated.
 */
l_int32
pixPlotAlongPta(PIX         *pixs,
                PTA         *pta,
                l_int32      outformat,
                const char  *title)
{
char            buffer[128];
char           *rtitle, *gtitle, *btitle;
static l_int32  count = 0;  /* require separate temp files for each call */
l_int32         i, x, y, d, w, h, npts, rval, gval, bval;
l_uint32        val;
NUMA           *na, *nar, *nag, *nab;
PIX            *pixt;

    PROCNAME("pixPlotAlongLine");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);
    if (outformat != GPLOT_PNG && outformat != GPLOT_PS &&
        outformat != GPLOT_EPS && outformat != GPLOT_X11 &&
        outformat != GPLOT_LATEX) {
        L_WARNING("outformat invalid; using GPLOT_PNG", procName);
        outformat = GPLOT_PNG;
    }

    pixt = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    d = pixGetDepth(pixt);
    w = pixGetWidth(pixt);
    h = pixGetHeight(pixt);
    npts = ptaGetCount(pta);
    if (d == 32) {
        nar = numaCreate(npts);
        nag = numaCreate(npts);
        nab = numaCreate(npts);
        for (i = 0; i < npts; i++) {
            ptaGetIPt(pta, i, &x, &y);
            if (x < 0 || x >= w)
                continue;
            if (y < 0 || y >= h)
                continue;
            pixGetPixel(pixt, x, y, &val);
            rval = GET_DATA_BYTE(&val, COLOR_RED);
            gval = GET_DATA_BYTE(&val, COLOR_GREEN);
            bval = GET_DATA_BYTE(&val, COLOR_BLUE);
            numaAddNumber(nar, rval);
            numaAddNumber(nag, gval);
            numaAddNumber(nab, bval);
        }

        sprintf(buffer, "junkplot.%d", count++);
        rtitle = stringJoin("Red: ", title);
        gplotSimple1(nar, outformat, buffer, rtitle);
        sprintf(buffer, "junkplot.%d", count++);
        gtitle = stringJoin("Green: ", title);
        gplotSimple1(nag, outformat, buffer, gtitle);
        sprintf(buffer, "junkplot.%d", count++);
        btitle = stringJoin("Blue: ", title);
        gplotSimple1(nab, outformat, buffer, btitle);
        numaDestroy(&nar);
        numaDestroy(&nag);
        numaDestroy(&nab);
        FREE(rtitle);
        FREE(gtitle);
        FREE(btitle);
    }
    else {
        na = numaCreate(npts);
        for (i = 0; i < npts; i++) {
            ptaGetIPt(pta, i, &x, &y);
            if (x < 0 || x >= w)
                continue;
            if (y < 0 || y >= h)
                continue;
            pixGetPixel(pixt, x, y, &val);
            numaAddNumber(na, (l_float32)val);
        }

        sprintf(buffer, "junkplot.%d", count++);
        gplotSimple1(na, outformat, buffer, title);
        numaDestroy(&na);
    }
    pixDestroy(&pixt);
    return 0;
}


/*!
 *  ptaContainsPt()
 *
 *      Input:  pta
 *              x, y  (point)
 *      Return: 1 if contained, 0 otherwise or on error
 */
l_int32
ptaContainsPt(PTA     *pta,
              l_int32  x,
              l_int32  y)
{
l_int32  i, n, ix, iy;

    PROCNAME("ptaContainsPt");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 0);

    n = ptaGetCount(pta);
    for (i = 0; i < n; i++) {
        ptaGetIPt(pta, i, &ix, &iy);
        if (x == ix && y == iy)
            return 1;
    }
    return 0;
}


/*!
 *  ptaTestIntersection()
 *
 *      Input:  pta1, pta2
 *      Return: bval which is 1 if they have any elements in common; 
 *              0 otherwise or on error.
 */
l_int32
ptaTestIntersection(PTA  *pta1,
                    PTA  *pta2)
{
l_int32  i, j, n1, n2, x1, y1, x2, y2;

    PROCNAME("ptaTestIntersection");

    if (!pta1)
        return ERROR_INT("pta1 not defined", procName, 0);
    if (!pta2)
        return ERROR_INT("pta2 not defined", procName, 0);

    n1 = ptaGetCount(pta1);
    n2 = ptaGetCount(pta2);
    for (i = 0; i < n1; i++) {
        ptaGetIPt(pta1, i, &x1, &y1);
        for (j = 0; j < n2; j++) {
            ptaGetIPt(pta2, i, &x2, &y2);
            if (x1 == x2 && y1 == y2)
                return 1;
        }
    }

    return 0;
}


/*!
 *  ptaTransform()
 *
 *      Input:  pta 
 *              shiftx, shifty
 *              scalex, scaley
 *      Return: pta, or null on error
 *
 *  Notes:
 *      (1) Shift first, then scale.
 */
PTA *
ptaTransform(PTA       *ptas,
             l_int32    shiftx,
             l_int32    shifty,
	     l_float32  scalex,
	     l_float32  scaley)
{
l_int32  n, i, x, y;
PTA     *ptad;

    PROCNAME("ptaTransform");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);
    n = ptaGetCount(ptas);
    ptad = ptaCreate(n);
    for (i = 0; i < n; i++) {
        ptaGetIPt(ptas, i, &x, &y);
        x = (l_int32)(scalex * (x + shiftx) + 0.5);
        y = (l_int32)(scaley * (y + shifty) + 0.5);
        ptaAddPt(ptad, x, y);
    }

    return ptad;
}


/*!
 *  ptaSubsample()
 *
 *      Input:  ptas 
 *              subfactor (subsample factor, >= 1)
 *      Return: ptad (evenly sampled pt values from ptas, or null on error
 */
PTA *
ptaSubsample(PTA     *ptas,
             l_int32  subfactor)
{
l_int32    n, i;
l_float32  x, y;
PTA       *ptad;

    PROCNAME("pixSubsample");

    if (!ptas)
        return (PTA *)ERROR_PTR("ptas not defined", procName, NULL);
    if (subfactor < 1)
        return (PTA *)ERROR_PTR("subfactor < 1", procName, NULL);

    ptad = ptaCreate(0);
    n = ptaGetCount(ptas);
    for (i = 0; i < n; i++) {
        if (i % subfactor != 0) continue;
        ptaGetPt(ptas, i, &x, &y);
        ptaAddPt(ptad, x, y);
    }

    return ptad;
}


/*!
 *  ptaGetLinearLSF()
 *
 *      Input:  pta
 *              &a  (<optional return> slope a of least square fit: y = ax + b)
 *              &b  (<optional return> intercept b of least square fit)
 *              &nafit (<optional return> numa of least square fit)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) At least one of: &a and &b must not be null.
 *      (2) If both &a and &b are defined, this returns a and b that minimize:
 *
 *              sum (yi - axi -b)^2
 *               i
 *
 *          The method is simple: differentiate this expression w/rt a and b,
 *          and solve the resulting two equations for a and b in terms of
 *          various sums over the input data (xi, yi).
 *      (3) We also allow two special cases, where either a = 0 or b = 0:
 *           (a) If &a is given and &b = null, find the linear LSF that
 *               goes through the origin (b = 0).
 *           (b) If &b is given and &a = null, find the linear LSF with
 *               zero slope (a = 0).
 *      (4) If @nafit is defined, this returns an array of fitted values,
 *          corresponding to the two implicit Numa arrays (nax and nay) in pta.
 *          Thus, just as you can plot the data in pta as nay vs. nax,
 *          you can plot the linear least square fit as nafit vs. nax.
 */
l_int32
ptaGetLinearLSF(PTA        *pta,
                l_float32  *pa,
                l_float32  *pb,
                NUMA      **pnafit)
{
l_int32     n, i;
l_float32   factor, sx, sy, sxx, sxy, val;
l_float32  *xa, *ya;

    PROCNAME("ptaGetLinearLSF");

    if (!pta)
        return ERROR_INT("pta not defined", procName, 1);
    if (!pa && !pb)
        return ERROR_INT("&a and/or &b not defined", procName, 1);
    if (pa) *pa = 0.0;
    if (pb) *pb = 0.0;

    if ((n = ptaGetCount(pta)) < 2)
        return ERROR_INT("less than 2 pts not found", procName, 1);
    xa = pta->x;  /* not a copy */
    ya = pta->y;  /* not a copy */

    sx = sy = sxx = sxy = 0.;
    if (pa && pb) {
        for (i = 0; i < n; i++) {
            sx += xa[i];
            sy += ya[i];
            sxx += xa[i] * xa[i];
            sxy += xa[i] * ya[i];
        }
        factor = n * sxx - sx * sx;
        if (factor == 0.0)
            return ERROR_INT("no solution found", procName, 1);
        factor = 1. / factor;

        *pa = factor * ((l_float32)n * sxy - sx * sy);
        *pb = factor * (sxx * sy - sx * sxy);
    }
    else if (pa) {  /* line through origin */
        for (i = 0; i < n; i++) {
            sxx += xa[i] * xa[i];
            sxy += xa[i] * ya[i];
        }
        if (sxx == 0.0)
            return ERROR_INT("no solution found", procName, 1);
        *pa = sxy / sxx;
    }
    else {  /* a = 0; horizontal line */
        for (i = 0; i < n; i++)
            sy += ya[i];
        *pb = sy / (l_float32)n;
    }

    if (pnafit) {
        *pnafit = numaCreate(n);
        for (i = 0; i < n; i++) {
            val = (*pa) * xa[i] + *pb;
            numaAddNumber(*pnafit, val);
        }
    }

    return 0;
}


/*!
 *  ptaGetPixelsFromPix()
 *
 *      Input:  pixs (1 bpp)
 *              box (<optional> can be null)
 *      Return: pta, or null on error
 *
 *  Notes:
 *      (1) Generates a pta of fg pixels in the pix, within the box.
 *          If box == NULL, it uses the entire pix.
 */
PTA *
ptaGetPixelsFromPix(PIX  *pixs,
                    BOX  *box)
{
l_int32    i, j, w, h, wpl, xstart, xend, ystart, yend, bw, bh;
l_uint32  *data, *line;
PTA       *pta;

    PROCNAME("ptaGetPixelsFromPix");

    if (!pixs || (pixGetDepth(pixs) != 1))
        return (PTA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);

    pixGetDimensions(pixs, &w, &h, NULL);
    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    xstart = ystart = 0;
    xend = w - 1;
    yend = h - 1;
    if (box) {
        boxGetGeometry(box, &xstart, &ystart, &bw, &bh);
        xend = xstart + bw - 1;
        yend = ystart + bh - 1;
    }

    if ((pta = ptaCreate(0)) == NULL)
        return (PTA *)ERROR_PTR("pta not made", procName, NULL);
    for (i = ystart; i <= yend; i++) {
        line = data + i * wpl;
        for (j = xstart; j <= xend; j++) {
            if (GET_DATA_BIT(line, j))
                ptaAddPt(pta, j, i);
        }
    }

    return pta;
}


/*!
 *  pixGenerateFromPta()
 *
 *      Input:  pta
 *              w, h (of pix) 
 *      Return: pix (1 bpp), or null on error
 *
 *  Notes:
 *      (1) Points are rounded to nearest ints.
 *      (2) Any points outside (w,h) are silently discarded.
 *      (3) Output 1 bpp pix has values 1 for each point in the pta.
 */
PIX *
pixGenerateFromPta(PTA     *pta,
                   l_int32  w,
                   l_int32  h)
{
l_int32  n, i, x, y;
PIX     *pix;

    PROCNAME("pixGenerateFromPta");

    if (!pta)
        return (PIX *)ERROR_PTR("pta not defined", procName, NULL);

    if ((pix = pixCreate(w, h, 1)) == NULL)
        return (PIX *)ERROR_PTR("pix not made", procName, NULL);
    n = ptaGetCount(pta);
    for (i = 0; i < n; i++) {
        ptaGetIPt(pta, i, &x, &y);
	if (x < 0 || x >= w || y < 0 || y >= h)
            continue;
	pixSetPixel(pix, x, y, 1);
    }

    return pix;
}


/*!
 *  ptaGetBoundaryPixels()
 *
 *      Input:  pixs (1 bpp)
 *              type (L_BOUNDARY_FG, L_BOUNDARY_BG)
 *      Return: pta, or null on error
 *
 *  Notes:
 *      (1) This generates a pta of either fg or bg boundary pixels.
 */
PTA *
ptaGetBoundaryPixels(PIX     *pixs,
                     l_int32  type)
{
PIX  *pixt;
PTA  *pta;

    PROCNAME("ptaGetBoundaryPixels");

    if (!pixs || (pixGetDepth(pixs) != 1))
        return (PTA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (type != L_BOUNDARY_FG && type != L_BOUNDARY_BG)
        return (PTA *)ERROR_PTR("invalid type", procName, NULL);

    if (type == L_BOUNDARY_FG)
        pixt = pixMorphSequence(pixs, "e3.3", 0);
    else
        pixt = pixMorphSequence(pixs, "d3.3", 0);
    pixXor(pixt, pixt, pixs);
    pta = ptaGetPixelsFromPix(pixt, NULL);

    pixDestroy(&pixt);
    return pta;
}


/*!
 *  ptaaGetBoundaryPixels()
 *
 *      Input:  pixs (1 bpp)
 *              type (L_BOUNDARY_FG, L_BOUNDARY_BG)
 *              connectivity (4 or 8)
 *              &boxa (<optional return> bounding boxes of the c.c.)
 *              &pixa (<optional return> pixa of the c.c.)
 *      Return: ptaa, or null on error
 *
 *  Notes:
 *      (1) This generates a ptaa of either fg or bg boundary pixels,
 *          where each pta has the boundary pixels for a connected
 *          component.
 *      (2) We can't simply find all the boundary pixels and then select
 *          those within the bounding box of each component, because
 *          bounding boxes can overlap.  It is necessary to extract and
 *          dilate or erode each component separately.  Note also that
 *          special handling is required for bg pixels when the
 *          component touches the pix boundary.
 */
PTAA *
ptaaGetBoundaryPixels(PIX     *pixs,
                      l_int32  type,
                      l_int32  connectivity,
                      BOXA   **pboxa,
                      PIXA   **ppixa)
{
l_int32  i, n, w, h, x, y, bw, bh, left, right, top, bot;
BOXA    *boxa;
PIX     *pixt1, *pixt2;
PIXA    *pixa;
PTA     *pta1, *pta2;
PTAA    *ptaa;

    PROCNAME("ptaaGetBoundaryPixels");

    if (pboxa) *pboxa = NULL;
    if (ppixa) *ppixa = NULL;
    if (!pixs || (pixGetDepth(pixs) != 1))
        return (PTAA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (type != L_BOUNDARY_FG && type != L_BOUNDARY_BG)
        return (PTAA *)ERROR_PTR("invalid type", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PTAA *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    pixGetDimensions(pixs, &w, &h, NULL);
    boxa = pixConnComp(pixs, &pixa, connectivity);
    n = boxaGetCount(boxa);
    ptaa = ptaaCreate(0);
    for (i = 0; i < n; i++) {
        pixt1 = pixaGetPix(pixa, i, L_CLONE);
        boxaGetBoxGeometry(boxa, i, &x, &y, &bw, &bh);
        left = right = top = bot = 0;
        if (type == L_BOUNDARY_BG) {
            if (x > 0) left = 1;
            if (y > 0) top = 1;
            if (x + bw < w) right = 1;
            if (y + bh < h) bot = 1;
            pixt2 = pixAddBorderGeneral(pixt1, left, right, top, bot, 0);
        }
        else
            pixt2 = pixClone(pixt1);
        pta1 = ptaGetBoundaryPixels(pixt2, type);
        pta2 = ptaTransform(pta1, x - left, y - top, 1.0, 1.0);
        ptaaAddPta(ptaa, pta2, L_INSERT);
        ptaDestroy(&pta1);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
    }

    if (pboxa)
        *pboxa = boxa;
    else
        boxaDestroy(&boxa);
    if (ppixa)
        *ppixa = pixa;
    else
        pixaDestroy(&pixa);
    return ptaa;
}


/*---------------------------------------------------------------------*
 *                          Display Pta and Ptaa                       *
 *---------------------------------------------------------------------*/
/*!
 *  pixDisplayPta()
 *
 *      Input:  pixs (1, 2, 4, 8, 16 or 32 bpp)
 *              pta (of path to be plotted)
 *      Return: pixd (32 bpp RGB version of pixs, with path in green),
 *              or null on error
 */
PIX *
pixDisplayPta(PIX  *pixs,
              PTA  *pta)
{
l_int32   i, n, x, y;
l_uint32  rpixel, gpixel, bpixel;
PIX      *pixd;

    PROCNAME("pixDisplayPta");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!pta)
        return (PIX *)ERROR_PTR("pta not defined", procName, NULL);

    if ((pixd = pixConvertTo32(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    composeRGBPixel(255, 0, 0, &rpixel);  /* start point */
    composeRGBPixel(0, 255, 0, &gpixel);
    composeRGBPixel(0, 0, 255, &bpixel);  /* end point */

    n = ptaGetCount(pta);
    for (i = 0; i < n; i++) {
        ptaGetIPt(pta, i, &x, &y);
        if (i == 0)
            pixSetPixel(pixd, x, y, rpixel);
        else if (i < n - 1)
            pixSetPixel(pixd, x, y, gpixel);
        else
            pixSetPixel(pixd, x, y, bpixel);
    }

    return pixd;
}


/*!
 *  pixDisplayPtaa()
 *
 *      Input:  pixs (1, 2, 4, 8, 16 or 32 bpp)
 *              ptaa (array of paths to be plotted)
 *      Return: pixd (32 bpp RGB version of pixs, with paths plotted
 *                    in different colors), or null on error
 */
PIX *
pixDisplayPtaa(PIX   *pixs,
               PTAA  *ptaa)
{
l_int32    i, j, npta, npt, x, y, rv, gv, bv;
l_uint32  *pixela;
PIX       *pixd;
PTA       *pta;

    PROCNAME("pixDisplayPtaa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptaa)
        return (PIX *)ERROR_PTR("ptaa not defined", procName, NULL);
    npta = ptaaGetCount(ptaa);
    if (npta == 0)
        return (PIX *)ERROR_PTR("no pta", procName, NULL);

    if ((pixd = pixConvertTo32(pixs)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

        /* Make a colormap for the paths; this one approximates
         * three functions that are linear for each color over
         * half the number of paths. */
    if ((pixela = (l_uint32 *)CALLOC(npta, sizeof(l_uint32))) == NULL)
        return (PIX *)ERROR_PTR("calloc fail for pixela", procName, NULL);
    for (i = 0; i < npta; i++) {
        rv = L_MAX(0, 255 -  255 * (2 * i) / (npta + 1));
        bv = L_MIN(255, L_MAX(0, (255 * (3 + 2 * i - npta) / (npta + 1))));
        if (i < npta / 2)
            gv = L_MIN(255, (255 * 2 * i) / (npta + 1));
        else
            gv = L_MIN(255, L_MAX(0, 255 - 255 * (2 * i - npta) / npta));
        composeRGBPixel(rv, gv, bv, &pixela[i]);
    }

    for (i = 0; i < npta; i++) {
        pta = ptaaGetPta(ptaa, i, L_CLONE);
        npt = ptaGetCount(pta);
        for (j = 0; j < npt; j++) {
            ptaGetIPt(pta, j, &x, &y);
            pixSetPixel(pixd, x, y, pixela[i]);
        }
        ptaDestroy(&pta);
    }

    FREE(pixela);
    return pixd;
}


