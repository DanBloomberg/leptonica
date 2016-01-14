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
 *   numarray.c
 *
 *      Numa creation, destruction, copy, clone, etc.
 *          NUMA        *numaCreate()
 *          void        *numaDestroy()
 *          NUMA        *numaCopy()
 *          NUMA        *numaClone()
 *          l_int32      numaEmpty()
 *
 *      Add/remove number (float or integer)
 *          l_int32      numaAddNumber()
 *          l_int32      numaExtendArray()
 *          l_int32      numaInsertNumber()
 *          l_int32      numaRemoveNumber()
 *
 *      Numa accessors
 *          l_int32      numaGetCount()
 *          l_int32      numaGetIValue()
 *          l_int32      numaGetFValue()
 *          l_int32      numaSetValue()
 *          l_int32     *numaGetIArray()
 *          l_float32   *numaGetFArray()
 *          l_int32      numaGetRefcount()
 *          l_int32      numaChangeRefcount()
 *
 *      Misc computation
 *          l_int32      numaGetMax()
 *          l_int32      numaGetMin()
 *          l_int32      numaGetSum()
 *          NUMA        *numaGetPartialSums()
 *          l_int32      numaGetSumOnInterval()
 *          l_int32      numaFitMax()
 *          l_int32      numaInterpolate()
 *          NUMA        *numaSort()
 *          NUMA        *numaGetSortIndex()
 *          NUMA        *numaSortByIndex()
 *          l_int32      numaGetMedian()
 *          l_int32      numaGetMode()
 *          NUMA        *numaConvertToInt()
 *          NUMA        *numaMakeHistogram()
 *          NUMA        *numaMakeHistogramClipped()
 *          NUMA        *numaRebinHistogram()
 *          NUMA        *numaNormalizeHistogram()
 *          l_int32      numaHistogramGetRankFromVal()
 *          l_int32      numaHistogramGetValFromRank()
 *          NUMA        *numaConvolve()
 *          NUMA        *numaFindPeaks()
 *          NUMA        *numaMakeSequence()
 *          l_int32      numaGetNonzeroRange()
 *          NUMA        *numaClipToInterval()
 *
 *      Numa combination
 *          l_int32      numaJoin()
 *
 *      Serialize numa for I/O
 *          l_int32      numaRead()
 *          l_int32      numaReadStream()
 *          l_int32      numaWrite()
 *          l_int32      numaWriteStream()
 *
 *      Numaa creation, destruction
 *          NUMAA       *numaaCreate()
 *          void        *numaaDestroy()
 *
 *      Add Numa to Numaa
 *          l_int32      numaaAddNuma()
 *          l_int32      numaaExtendArray()
 *
 *      Numaa accessors
 *          l_int32      numaaGetCount()
 *          l_int32      numaaGetNumberCount()
 *          NUMA        *numaaGetNuma()
 *          NUMA        *numaaReplaceNuma()
 *          l_int32      numaaAddNumber()
 *       
 *      Numa2d creation, destruction
 *          NUMA2D      *numa2dCreate()
 *          void        *numa2dDestroy()
 *
 *      Numa2d Accessors
 *          l_int32      numa2dAddNumber()
 *          l_int32      numa2dGetCount()
 *          NUMA        *numa2dGetNuma()
 *          l_int32      numa2dGetFValue()
 *          l_int32      numa2dGetIValue()
  *
  *     NumaHash creation, destruction
  *         NUMAHASH    *numaHashCreate()
  *         void        *numaHashDestroy()
  *
  *     NumaHash Accessors
  *         NUMA        *numaHashGetNuma()
  *         void        *numaHashAdd()
 *
 *    The number array holds l_float32 values.  It can also
 *    be used to store l_int32 values.  This versatility is the
 *    reason it's called a "number" array.
 *
 *    To handle integer values:
 *
 *       - to add a new one to the array, use numaAddNumber() with
 *         the integer input.  The type will automatically be converted
 *         to l_float32 and stored.
 *
 *       - to set a value in the array, use numaSetValue() with
 *         the integer input.  It will be converted to l_float32 and stored.
 *
 *       - to get a value in the array, use either numaGetIValue()
 *         or numaGetFValue(), depending on whether you are retrieving
 *         an integer or a float.  This avoids forcing the user to do
 *         an explicit cast, which would happen in one of two ways:
 *           (a) return a l_float32 and cast it to an l_int32
 *           (b) cast the return directly to (l_float32 *) to
 *               satisfy the function prototype, as in
 *                 numaGetFValue(na, index, (l_float32 *)&ival);
 *              (OK, that is quite ugly!)
 *
 *    Tradition dictates that type conversions go automatically from
 *    l_int32 --> l_float32, even though it is possible to lose
 *    precision for large integers, whereas you must cast (l_int32)
 *    to go from l_float32 --> l_int32 because you're truncating
 *    to the integer value.
 *                   
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

    /* bin sizes in numaMakeHistogram() */
static const l_int32 BinSizeArray[] = {2, 5, 10, 20, 50, 100, 200, 500, 1000,\
                      2000, 5000, 10000, 20000, 50000, 100000, 200000,\
		      500000, 1000000, 2000000, 5000000, 10000000,\
		      200000000, 50000000, 100000000};
static const l_int32 NBinSizes = 24;

static const l_int32 INITIAL_PTR_ARRAYSIZE = 50;      /* n'importe quoi */


#ifndef  NO_CONSOLE_IO
#define  DEBUG_HISTO    0
#endif  /* ~NO_CONSOLE_IO */


/*--------------------------------------------------------------------------*
 *               Numa creation, destruction, copy, clone, etc.              *
 *--------------------------------------------------------------------------*/
/*!
 *  numaCreate()
 *
 *      Input:  size of number array to be alloc'd (0 for default)
 *      Return: na, or null on error
 */
NUMA *
numaCreate(l_int32  n)
{
NUMA  *na;

    PROCNAME("numaCreate");

    if (n <= 0)
	n = INITIAL_PTR_ARRAYSIZE;

    if ((na = (NUMA *)CALLOC(1, sizeof(NUMA))) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    if ((na->array = (l_float32 *)CALLOC(n, sizeof(l_float32))) == NULL)
        return (NUMA *)ERROR_PTR("number array not made", procName, NULL);

    na->nalloc = n;
    na->n = 0;
    na->refcount = 1;

    return na;
}


/*!
 *  numaDestroy()
 *
 *      Input:  &numa (<to be nulled if it exists>)
 *      Return: void
 *
 *  Note:
 *      - Decrements the ref count and, if 0, destroys the numa.
 *      - Always nulls the input ptr.
 */
void
numaDestroy(NUMA  **pna)
{
NUMA  *na;

    PROCNAME("numaDestroy");

    if (pna == NULL) {
	L_WARNING("ptr address is NULL", procName);
	return;
    }

    if ((na = *pna) == NULL)
	return;

	/* Decrement the ref count.  If it is 0, destroy the numa. */
    numaChangeRefcount(na, -1);
    if (numaGetRefcount(na) <= 0) {
        if (na->array)
            FREE(na->array);
        FREE(na);
    }

    *pna = NULL;
    return;
}

        
/*!
 *  numaCopy()
 *
 *      Input:  numa
 *      Return: copy of numa, or null on error
 */
NUMA *
numaCopy(NUMA  *na)
{
l_int32  i;
NUMA    *cna;

    PROCNAME("numaCopy");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);

    if ((cna = numaCreate(na->nalloc)) == NULL)
        return (NUMA *)ERROR_PTR("cna not made", procName, NULL);

    for (i = 0; i < na->n; i++)
	numaAddNumber(cna, na->array[i]);

    return cna;
}


/*!
 *  numaClone()
 *
 *      Input:  numa
 *      Return: ptr to same numa, or null on error
 */
NUMA *
numaClone(NUMA  *na)
{

    PROCNAME("numaClone");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);

    numaChangeRefcount(na, 1);
    return na;
}


/*!
 *  numaEmpty()
 *
 *      Input:  numa
 *      Return: 0 if OK; 1 on error
 *
 *  Note: This does not change the allocation of the array;
 *        it just clears the number of stored numbers.
 */
l_int32
numaEmpty(NUMA  *na)
{

    PROCNAME("numaEmpty");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);

    na->n = 0;
    return 0;
}



/*--------------------------------------------------------------------------*
 *                 Number array: add number and extend array                *
 *--------------------------------------------------------------------------*/
/*!
 *  numaAddNumber()
 *
 *      Input:  numa
 *              val  (float or int to be added; stored as a float)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaAddNumber(NUMA      *na,
              l_float32  val)
{
l_int32  n;

    PROCNAME("numaAddNumber");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    
    n = numaGetCount(na);
    if (n >= na->nalloc)
	numaExtendArray(na);
    na->array[n] = val;
    na->n++;
    return 0;
}


/*!
 *  numaExtendArray()
 *
 *      Input:  numa
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaExtendArray(NUMA  *na)
{

    PROCNAME("numaExtendArray");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);

    if ((na->array = (l_float32 *)reallocNew((void **)&na->array,
                                sizeof(l_float32) * na->nalloc,
                                2 * sizeof(l_float32) * na->nalloc)) == NULL)
	    return ERROR_INT("new ptr array not returned", procName, 1);

    na->nalloc *= 2;
    return 0;
}


/*!
 *  numaInsertNumber()
 *
 *      Input:  numa
 *              index (place to insert)
 *              val  (float32 or integer to be added)
 *      Return: 0 if OK, 1 on error
 *
 *  Note: This shouldn't be used repeatedly on large arrays,
 *        as the function is O(n).
 */
l_int32
numaInsertNumber(NUMA      *na,
	         l_int32    index,
                 l_float32  val)
{
l_int32  i, n;

    PROCNAME("numaInsertNumber");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    
    n = numaGetCount(na);
    if (index < 0 || index > n)
        return ERROR_INT("index not in {0...n}", procName, 1);
    if (n >= na->nalloc)
	numaExtendArray(na);

    for (i = n; i > index; i++)
	na->array[i] = na->array[i - 1];
    na->array[index] = val;
    na->n++;
    return 0;
}


/*!
 *  numaRemoveNumber()
 *
 *      Input:  numa
 *              index (element to be removed)
 *      Return: 0 if OK, 1 on error
 *
 *  Note: this shouldn't be used repeatedly on large arrays,
 *        as the function is O(n).
 */
l_int32
numaRemoveNumber(NUMA    *na,
	         l_int32  index)
{
l_int32  i, n;

    PROCNAME("numaRemoveNumber");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    
    n = numaGetCount(na);
    if (index < 0 || index >= n)
        return ERROR_INT("index not in {0...n - 1}", procName, 1);

    for (i = index; i < n - 1; i++)
	na->array[i] = na->array[i + 1];
    na->n--;
    return 0;
}



/*----------------------------------------------------------------------*
 *                            Numa accessors                            *
 *----------------------------------------------------------------------*/
/*!
 *  numaGetCount()
 *
 *      Input:  numa
 *      Return: count, or 0 if no numbers or on error
 */
l_int32
numaGetCount(NUMA  *na)
{

    PROCNAME("numaGetCount");

    if (!na)
        return ERROR_INT("na not defined", procName, 0);
    return na->n;
}
	

/*!
 *  numaGetFValue()
 *
 *      Input:  numa
 *              index (into numa)
 *              &val  (<return> float value)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetFValue(NUMA       *na,
              l_int32     index,
	      l_float32  *pval)
{
    PROCNAME("numaGetFValue");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0.0;

    if (index < 0 || index >= na->n)
        return ERROR_INT("index not valid", procName, 1);

    *pval = na->array[index];
    return 0;
}


/*!
 *  numaGetIValue()
 *
 *      Input:  numa
 *              index (into numa)
 *              &ival  (<return> integer value)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetIValue(NUMA     *na,
              l_int32   index,
	      l_int32  *pival)
{
    PROCNAME("numaGetIValue");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pival)
        return ERROR_INT("&ival not defined", procName, 1);
    *pival = 0;

    if (index < 0 || index >= na->n)
        return ERROR_INT("index not valid", procName, 1);

    *pival = (l_int32)(na->array[index] + 0.5);
    return 0;
}


/*!
 *  numaSetValue()
 *
 *      Input:  numa
 *              index   (to element to be set)
 *              val  (to set element)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaSetValue(NUMA      *na,
             l_int32    index,
             l_float32  val)
{

    PROCNAME("numaSetValue");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (index < 0 || index >= na->n)
        return ERROR_INT("index not valid", procName, 1);

    na->array[index] = val;
    return 0;
}


/*!
 *  numaGetIArray()
 *
 *      Input:  numa
 *      Return: a copy of the bare internal array, integerized
 *              by rounding, or null on error
 */
l_int32 *
numaGetIArray(NUMA  *na)
{
l_int32   i, ival;
l_int32  *array;

    PROCNAME("numaGetIArray");

    if (!na)
        return (l_int32 *)ERROR_PTR("na not defined", procName, NULL);

    if ((array = (l_int32 *)CALLOC(na->n, sizeof(l_int32))) == NULL)
        return (l_int32 *)ERROR_PTR("array not made", procName, NULL);
    for (i = 0; i < na->n; i++) {
	numaGetIValue(na, i, &ival);
	array[i] = ival;
    }

    return array;
}


/*!
 *  numaGetFArray()
 *
 *      Input:  numa
 *      Return: a copy of the bare internal array, or null on error
 */
l_float32 *
numaGetFArray(NUMA  *na)
{
l_int32     i;
l_float32   fval;
l_float32  *array;

    PROCNAME("numaGetFArray");

    if (!na)
        return (l_float32 *)ERROR_PTR("na not defined", procName, NULL);

    if ((array = (l_float32 *)CALLOC(na->n, sizeof(l_float32))) == NULL)
        return (l_float32 *)ERROR_PTR("array not made", procName, NULL);
    for (i = 0; i < na->n; i++) {
	numaGetFValue(na, i, &fval);
	array[i] = fval;
    }

    return array;
}


/*!
 *  numaGetRefCount()
 *
 *      Input:  na
 *      Return: refcount, or UNDEF on error
 */
l_int32
numaGetRefcount(NUMA  *na)
{
    PROCNAME("numaGetRefcount");

    if (!na)
        return ERROR_INT("na not defined", procName, UNDEF);
    return na->refcount;
}


/*!
 *  numaChangeRefCount()
 *
 *      Input:  na
 *              delta (change to be applied)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaChangeRefcount(NUMA    *na,
                   l_int32  delta)
{
    PROCNAME("numaChangeRefcount");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    na->refcount += delta;
    return 0;
}



/*----------------------------------------------------------------------*
 *                          Misc computation                            *
 *----------------------------------------------------------------------*/
/*!
 *  numaGetMax()
 *
 *      Input:  na
 *              &maxval (<return> max value)
 *              &imaxloc (<return> index of max location)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetMax(NUMA       *na,
           l_float32  *pmaxval,
           l_int32    *pimaxloc)
{
l_int32    i, n, imaxloc;
l_float32  val, maxval;

    PROCNAME("numaGetMax");

    *pmaxval = 0.0;  /* init */
    *pimaxloc = 0;

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pmaxval)
        return ERROR_INT("&maxval not defined", procName, 1);
    if (!pimaxloc)
        return ERROR_INT("&imaxloc not defined", procName, 1);

    maxval = -1000000000.;
    imaxloc = 0;
    n = numaGetCount(na);
    for (i = 0; i < n; i++) {
	numaGetFValue(na, i, &val);
	if (val > maxval) {
	    maxval = val;
	    imaxloc = i;
	}
    }
    *pmaxval = maxval;
    *pimaxloc = imaxloc;
    return 0;
}


/*!
 *  numaGetMin()
 *
 *      Input:  na
 *              &minval (<return> min value)
 *              &iminloc (<return> index of min location)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetMin(NUMA       *na,
           l_float32  *pminval,
           l_int32    *piminloc)
{
l_int32    i, n, iminloc;
l_float32  val, minval;

    PROCNAME("numaGetMin");

    *pminval = 0.0;  /* init */
    *piminloc = 0;

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pminval)
        return ERROR_INT("&minval not defined", procName, 1);
    if (!piminloc)
        return ERROR_INT("&iminloc not defined", procName, 1);

    minval = +1000000000.;
    iminloc = 0;
    n = numaGetCount(na);
    for (i = 0; i < n; i++) {
	numaGetFValue(na, i, &val);
	if (val < minval) {
	    minval = val;
	    iminloc = i;
	}
    }
    *pminval = minval;
    *piminloc = iminloc;
    return 0;
}


/*!
 *  numaGetSum()
 *
 *      Input:  na
 *              &sum (<return> sum of values)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaGetSum(NUMA       *na,
           l_float32  *psum)
{
l_int32    i, n;
l_float32  val, sum;

    PROCNAME("numaGetSum");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);

    sum = 0.0;
    n = numaGetCount(na);
    for (i = 0; i < n; i++) {
        numaGetFValue(na, i, &val);	
	sum += val;
    }
    *psum = sum;
    return 0;
}


/*!
 *  numaGetPartialSums()
 *
 *      Input:  na
 *      Return: nasum, or null on error
 *
 *  Notes: 
 *      (1) nasum[i] is the sum for all j <=i of na[j]
 */
NUMA *
numaGetPartialSums(NUMA  *na)
{
l_int32    i, n;
l_float32  val, sum;
NUMA      *nasum;

    PROCNAME("numaGetPartialSums");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);

    n = numaGetCount(na);
    nasum = numaCreate(n);
    sum = 0.0;
    for (i = 0; i < n; i++) {
        numaGetFValue(na, i, &val);	
	sum += val;
        numaAddNumber(nasum, sum);
    }
    return nasum;
}


/*!
 *  numaGetSumOnInterval()
 *
 *      Input:  na
 *              first (beginning index)
 *              last (final index)
 *              &sum (<return> sum of values in the index interval range)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaGetSumOnInterval(NUMA       *na,
		     l_int32     first,
		     l_int32     last,
                     l_float32  *psum)
{
l_int32    i, n, truelast;
l_float32  val, sum;

    PROCNAME("numaGetSumOnInterval");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!psum)
        return ERROR_INT("&sum not defined", procName, 1);
    *psum = 0.0;

    sum = 0.0;
    n = numaGetCount(na);
    if (first >= n)  /* not an error */
      return 0;
    truelast = L_MIN(last, n - 1);

    for (i = first; i <= truelast; i++) {
        numaGetFValue(na, i, &val);	
	sum += val;
    }
    *psum = sum;
    return 0;
}


/*!
 *  numaFitMax()
 *
 *      Input:  na  (numa of ordinate values, to fit a max to)
 *              &maxval (<return> max value)
 *              naloc (<optional> associated numa of abscissa values)
 *              &maxloc (<return> abscissa value that gives max value in na;
 *                   if naloc == null, this is given as an interpolated
 *                   index value)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: if naloc is given, there is no requirement that the
 *        data points are evenly spaced.  Lagrangian interpolation
 *        handles that.  The only requirement is that the
 *        data points are ordered so that the values in naloc
 *        are either increasing or decreasing.  We test to make
 *        sure that the sizes of na and naloc are equal, and it
 *        is assumed that the correspondences na[i] as a function
 *        of naloc[i] are properly arranged for all i.
 *
 *  The formula for Lagrangian interpolation through 3 data pts is:
 *       y(x) = y1(x-x2)(x-x3)/((x1-x2)(x1-x3)) +
 *              y2(x-x1)(x-x3)/((x2-x1)(x2-x3)) +
 *              y3(x-x1)(x-x2)/((x3-x1)(x3-x2))
 *
 *  Then the derivative, using the constants (c1,c2,c3) defined below,
 *  is set to 0:
 *       y'(x) = 2x(c1+c2+c3) - c1(x2+x3) - c2(x1+x3) - c3(x1+x2) = 0
 */
l_int32
numaFitMax(NUMA       *na,
           l_float32  *pmaxval,
           NUMA       *naloc,
           l_float32  *pmaxloc)
{
l_float32  val;
l_float32  smaxval;  /* start value of maximum sample, before interpolating */
l_int32    n, imaxloc;
l_float32  x1, x2, x3, y1, y2, y3, c1, c2, c3, a, b, xmax, ymax;

    PROCNAME("numaFitMax");

    *pmaxval = *pmaxloc = 0.0;  /* init */

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pmaxval)
        return ERROR_INT("&maxval not defined", procName, 1);
    if (!pmaxloc)
        return ERROR_INT("&maxloc not defined", procName, 1);

    n = numaGetCount(na);
    if (naloc) {
	if (n != numaGetCount(naloc))
	    return ERROR_INT("na and naloc of unequal size", procName, 1);
    }

    numaGetMax(na, &smaxval, &imaxloc);

	/* Simple case: max is at end point */
    if (imaxloc == 0 || imaxloc == n - 1) {
	*pmaxval = smaxval;
	if (naloc) {
	    numaGetFValue(naloc, imaxloc, &val);
	    *pmaxloc = val;
	}
	else 
	    *pmaxloc = imaxloc;
	return 0;
    }

	/* Interior point; use quadratic interpolation */
    y2 = smaxval;
    numaGetFValue(na, imaxloc - 1, &val);
    y1 = val;
    numaGetFValue(na, imaxloc + 1, &val);
    y3 = val;
    if (naloc) {
	numaGetFValue(naloc, imaxloc - 1, &val);
	x1 = val;
	numaGetFValue(naloc, imaxloc, &val);
	x2 = val;
	numaGetFValue(naloc, imaxloc + 1, &val);
	x3 = val;
    }
    else {
	x1 = imaxloc - 1;
	x2 = imaxloc;
	x3 = imaxloc + 1;
    }

	/* Can't interpolate; just use the max val in na
	 * and the corresponding one in naloc */
    if (x1 == x2 || x1 == x3 || x2 == x3) {
	*pmaxval = y2;
	*pmaxloc = x2;
	return 0;
    }

	/* Use lagrangian interpolation; set dy/dx = 0 */
    c1 = y1 / ((x1 - x2) * (x1 - x3));
    c2 = y2 / ((x2 - x1) * (x2 - x3));
    c3 = y3 / ((x3 - x1) * (x3 - x2));
    a = c1 + c2 + c3;
    b = c1 * (x2 + x3) + c2 * (x1 + x3) + c3 * (x1 + x2);
    xmax = b / (2 * a);
    ymax = c1 * (xmax - x2) * (xmax - x3) +
           c2 * (xmax - x1) * (xmax - x3) +
           c3 * (xmax - x1) * (xmax - x2);
    *pmaxval = ymax;
    *pmaxloc = xmax;

    return 0;
}


/*!
 *  numaInterpolate()
 *
 *      Input:  nas  (numa of ordinate values, assumed equally spaced)
 *              startval (location of first src val in interpolated array)
 *              incr (increment in interpolated array)
 *              size (size of output interpolated array)
 *      Return: nad (interpolated values), or null on error
 *
 *  Use quadratic Lagrangian interpolation (through 3 data pts):
 *        y(x) = y1(x-x2)(x-x3)/((x1-x2)(x1-x3)) +
 *               y2(x-x1)(x-x3)/((x2-x1)(x2-x3)) +
 *               y3(x-x1)(x-x2)/((x3-x1)(x3-x2))
 */
NUMA *
numaInterpolate(NUMA    *nas,
                l_int32  startval,
		l_int32  incr,
		l_int32  size)
{
l_int32     i, j, n, lastn, twoincr;
l_float32   y1, y2, y3, d1, d2, d3, val;
l_float32  *fa;
NUMA       *nad;

    PROCNAME("numaInterpolate");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    n = numaGetCount(nas);
    lastn = size - startval - (n - 1) * incr;
    if (startval > incr || L_ABS(lastn) > 2 * incr)
        L_WARNING("samples don't span output array", procName);

    if ((nad = numaCreate(size)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);
    fa = numaGetFArray(nas);
    d1 = d3 = 0.5 / (incr * incr);
    d2 = -2. * d1;
    twoincr = 2 * incr;
    for (i = 1; i < n - 1; i++) {
        y1 = d1 * fa[i - 1];
        y2 = d2 * fa[i];
        y3 = d3 * fa[i + 1];
        if (i == 1) {
            for (j = 0; j < startval; j++) {
                val = y1 * (j - incr - startval) * (j - twoincr - startval) +
                      y2 * (j - startval) * (j - twoincr - startval) +
                      y3 * (j - startval) * (j - incr - startval);
	        numaAddNumber(nad, val);
            }
        }
        for (j = 0; j < incr; j++) {
            val = y1 * (j - incr) * (j - twoincr) +
                  y2 * j * (j - twoincr) +
                  y3 * j * (j - incr);
	    numaAddNumber(nad, val);
        }
	if (i == n - 2) {
            for (j = 0; j < incr; j++) {  /* do last full interval */
                val = y1 * j * (j - incr) +
                      y2 * (incr + j) * (j - incr) +
                      y3 * (incr + j) * j;
	        numaAddNumber(nad, val);
            }
            for (j = 0; j < lastn; j++) {  /* extrapolate lastn pts */
                val = y1 * (j + incr) * j +
                      y2 * (j + twoincr) * j +
                      y3 * (j + twoincr) * (j + incr);
	        numaAddNumber(nad, val);
            }
        }
    }

    return nad;
}


/*!
 *  numaSort()
 *
 *      Input:  naout (output numa; can be NULL or equal to nain)
 *              nain (input numa)
 *              sortorder (L_SORT_INCREASING or L_SORT_DECREASING)
 *      Return: naout (output sorted numa), or null on error
 *       
 *  Notes:
 *      (1) Set naout = nain for in-place; otherwise, set naout = NULL.
 *      (2) Source: Shell sort, modified from K&R, 2nd edition, p.62.
 *          Slow but simple O(n logn) sort.
 */
NUMA *
numaSort(NUMA    *naout,
         NUMA    *nain,
         l_int32  sortorder)
{
l_int32     i, n, gap, j;
l_float32   tmp;
l_float32  *array;

    PROCNAME("numaSort");

    if (!nain)
        return (NUMA *)ERROR_PTR("nain not defined", procName, NULL);

        /* Make naout if necessary; otherwise do in-place */
    if (!naout)
        naout = numaCopy(nain);
    else if (nain != naout)
        return (NUMA *)ERROR_PTR("invalid: not in-place", procName, NULL);
    array = naout->array;  /* operate directly on the array */
    n = numaGetCount(naout);

	/* Shell sort */
    for (gap = n/2; gap > 0; gap = gap / 2) {
	for (i = gap; i < n; i++) {
	    for (j = i - gap; j >= 0; j -= gap) {
	        if ((sortorder == L_SORT_INCREASING &&
		     array[j] > array[j + gap]) || 
		    (sortorder == L_SORT_DECREASING &&
	             array[j] < array[j + gap]))
		{
		    tmp = array[j];
		    array[j] = array[j + gap];
		    array[j + gap] = tmp;
                }
	    }
	}
    }

    return naout;
}


/*!
 *  numaGetSortIndex()
 *
 *      Input:  na
 *              sortorder (L_SORT_INCREASING or L_SORT_DECREASING)
 *      Return: na giving an array of indices that would sort
 *              the input array, or null on error
 */
NUMA *
numaGetSortIndex(NUMA    *na,
                 l_int32  sortorder)
{
l_int32     i, n, gap, j;
l_float32   tmp;
l_float32  *array;   /* copy of input array */
l_float32  *iarray;  /* array of indices */
NUMA       *naisort;

    PROCNAME("numaGetSortIndex");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);

    n = numaGetCount(na);
    if ((array = numaGetFArray(na)) == NULL)
        return (NUMA *)ERROR_PTR("array not made", procName, NULL);
    if ((iarray = (l_float32 *)CALLOC(n, sizeof(l_float32))) == NULL)
        return (NUMA *)ERROR_PTR("iarray not made", procName, NULL);
    for (i = 0; i < n; i++)
        iarray[i] = i;

	/* Shell sort */
    for (gap = n/2; gap > 0; gap = gap / 2) {
	for (i = gap; i < n; i++) {
	    for (j = i - gap; j >= 0; j -= gap) {
	        if ((sortorder == L_SORT_INCREASING &&
		     array[j] > array[j + gap]) || 
		    (sortorder == L_SORT_DECREASING &&
	             array[j] < array[j + gap]))
		{
		    tmp = array[j];
		    array[j] = array[j + gap];
		    array[j + gap] = tmp;
		    tmp = iarray[j];
		    iarray[j] = iarray[j + gap];
		    iarray[j + gap] = tmp;
		}
	    }
	}
    }

    naisort = numaCreate(n);
    for (i = 0; i < n; i++) 
	numaAddNumber(naisort, iarray[i]);

    FREE(array);
    FREE(iarray);
    return naisort;
}


/*!
 *  numaSortByIndex()
 * 
 *      Input:  nas
 *              naindex (na that maps from the new numa to the input numa)
 *      Return: nad (sorted), or null on error
 */
NUMA *
numaSortByIndex(NUMA  *nas,
		NUMA  *naindex)
{
l_int32    i, n, index;
l_float32  val;
NUMA      *nad;

    PROCNAME("numaSortByIndex");

    if (!nas)
	return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    if (!naindex)
	return (NUMA *)ERROR_PTR("naindex not defined", procName, NULL);

    n = numaGetCount(nas);
    nad = numaCreate(n);
    for (i = 0; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        numaGetFValue(nas, index, &val);
	numaAddNumber(nad, val);
    }

    return nad;
}


/*!
 *  numaGetMedian()
 *
 *      Input:  na
 *              &val  (<return> median val)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetMedian(NUMA       *na,
              l_float32  *pval)
{
l_int32  n;
NUMA    *nasort;

    PROCNAME("numaGetMedian");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0.0;  /* init */

    n = numaGetCount(na);
    if (n == 0)
	return 1;
    if ((nasort = numaSort(NULL, na, L_SORT_DECREASING)) == NULL)
	return ERROR_INT("nasort not made", procName, 1);
    numaGetFValue(nasort, n / 2, pval);

    numaDestroy(&nasort);
    return 0;
}


/*!
 *  numaGetMode()
 *
 *      Input:  na
 *              &val  (<return> mode val)
 *              &count  (<return> mode count)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaGetMode(NUMA       *na,
            l_float32  *pval,
	    l_int32    *pcount)
{
l_int32     i, n, maxcount, prevcount;
l_float32   val, maxval, prevval;
l_float32  *array;
NUMA       *nasort;

    PROCNAME("numaGetMode");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    if (!pcount)
        return ERROR_INT("&count not defined", procName, 1);

    *pval = 0.0;
    *pcount = 0;
    if ((n = numaGetCount(na)) == 0)
	return 1;

    if ((nasort = numaSort(NULL, na, L_SORT_DECREASING)) == NULL)
        return ERROR_INT("nas not made", procName, 1);
    if ((array = numaGetFArray(nasort)) == NULL)
        return ERROR_INT("array not made", procName, 1);

        /* Initialize with array[0] */
    prevval = array[0];
    prevcount = 1;
    maxval = prevval;
    maxcount = prevcount;

        /* Scan the sorted array */
    for (i = 1; i < n; i++) {
	val = array[i];
	if (val == prevval)
	    prevcount++;
	else {  /* new value */
	    if (prevcount > maxcount) {  /* new max */
	        maxcount = prevcount;
		maxval = prevval;
	    }
	    prevval = val;
	    prevcount = 1;
	}
    }

	/* Was the mode the last run of elements? */
    if (prevcount > maxcount) {
        maxcount = prevcount;
	maxval = prevval;
    }

    *pval = maxval;
    *pcount = maxcount;

    numaDestroy(&nasort);
    FREE(array);
    return 0;
}


/*!
 *  numaConvertToInt()
 *
 *      Input:  na
 *      Return: na with all values rounded to nearest integer, or
 *              null on error
 */
NUMA *
numaConvertToInt(NUMA  *nas)
{
l_int32  i, n, ival;
NUMA    *nad;

    PROCNAME("numaConvertToInt");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);

    n = numaGetCount(nas);
    if ((nad = numaCreate(n)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);
    for (i = 0; i < n; i++) {
	numaGetIValue(nas, i, &ival);
	numaAddNumber(nad, ival);
    }
    return nad;
}


/*!
 *  numaMakeHistogram()
 *
 *      Input:  na
 *              maxnbins (max number of histogram bins)
 *              &binsize  (<return> size of histogram bins)
 *              &binstart (<optional return> start val of minimum bin;
 *                         input NULL to force start at 0)
 *      Return: na consisiting of histogram of integerized values,
 *              or null on error.
 *
 *  Note: We specify the max number of input bins, and are returned the
 *        size of bins necessary to accommodate the input data.  The size
 *        is one of the sequence: {1, 2, 5, 10, 20, 50, ...}.
 *        If &binstart is given, all values are accommodated,
 *        and the min value of the starting bin is returned;
 *        otherwise, all negative values are discarded and
 *        the histogram bins start at 0.
 */
NUMA *
numaMakeHistogram(NUMA     *na,
                  l_int32   maxbins,
		  l_int32  *pbinsize,
		  l_int32  *pbinstart)
{
l_int32    i, n, ival, iloc, hval;
l_int32    iminval, imaxval, range, binsize, nbins, ibin;
l_float32  val, ratio;
NUMA      *nai, *nahist;

    PROCNAME("numaMakeHistogram");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);
    if (!pbinsize)
        return (NUMA *)ERROR_PTR("&binsize not defined", procName, NULL);

        /* Determine input range */
    numaGetMin(na, &val, &iloc);
    iminval = (l_int32)(val + 0.5);
    numaGetMax(na, &val, &iloc);
    imaxval = (l_int32)(val + 0.5);
    if (pbinstart == NULL) {  /* clip negative vals; start from 0 */
        iminval = 0;
	if (imaxval < 0)
	    return (NUMA *)ERROR_PTR("all values < 0", procName, NULL);
    }

        /* Determine binsize */
    range = imaxval - iminval + 1;
    if (range > maxbins - 1) {
        ratio = (l_float64)range / (l_float64)maxbins;
	binsize = 0;
	for (i = 0; i < NBinSizes; i++) {
	    if (ratio < BinSizeArray[i]) {
	        binsize = BinSizeArray[i];
		break;
	    }
	}
	if (binsize == 0)
	    return (NUMA *)ERROR_PTR("numbers too large", procName, NULL);
    }
    else
        binsize = 1;
    *pbinsize = binsize;
    nbins = 1 + range / binsize;  /* +1 seems to be sufficient */

        /* Redetermine iminval */
    if (pbinstart && binsize > 1) {
        if (iminval >= 0)
	    iminval = binsize * (iminval / binsize);
	else
	    iminval = binsize * ((iminval - binsize + 1) / binsize);
    }
    if (pbinstart)
	*pbinstart = iminval;

#if  DEBUG_HISTO
    fprintf(stderr, " imaxval = %d, range = %d, nbins = %d\n",
            imaxval, range, nbins);
#endif  /* DEBUG_HISTO */

        /* Use integerized data for input */
    if ((nai = numaConvertToInt(na)) == NULL)
        return (NUMA *)ERROR_PTR("nai not made", procName, NULL);
    n = numaGetCount(nai);

        /* Make histogram, converting value in input array 
	 * into a bin number for this histogram array. */
    if ((nahist = numaCreate(nbins)) == NULL)
        return (NUMA *)ERROR_PTR("nahist not made", procName, NULL);
    nahist->n = nbins;  /* fake the storage of nbins zeroes */
    for (i = 0; i < n; i++) {
	numaGetIValue(nai, i, &ival);
        ibin = (ival - iminval) / binsize;
	if (ibin >= 0 && ibin < nbins) {
	    numaGetIValue(nahist, ibin, &hval);
	    numaSetValue(nahist, ibin, hval + 1.0);
	}
    }

    numaDestroy(&nai);
    return nahist;
}


/*!
 *  numaMakeHistogramClipped()
 *
 *      Input:  na
 *              binsize (typically 1)
 *              maxsize (of histogram ordinate)
 *      Return: na consisiting of histogram of bins of size 'nbinsize',
 *              starting with input values 0 and going up to a maximum of
 *              'maxsize' by increments of 'binsize'; or null on error.
 *
 *  Note: This is a very simple function.  We integerize the array,
 *        and ignore all input values that are negative or larger than
 *        the maximum value (maxsize).  We bin the data in bins of
 *        size 'nbinsize', starting at 0.  With this constraint, we
 *        only use as many bins as are needed to hold the data.
 */
NUMA *
numaMakeHistogramClipped(NUMA    *na,
                         l_int32  binsize,
                         l_int32  maxsize)
{
l_int32    i, n, nbins, imaxval, imaxsize, imaxloc, ival, ibin, hval;
l_float32  maxval;
NUMA      *nai, *nahist;

    PROCNAME("numaMakeHistogramClipped");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);
    if (binsize < 1)
        binsize = 1;
    if (binsize > maxsize)
        binsize = maxsize;

    numaGetMax(na, &maxval, &imaxloc);
    imaxval = (l_int32)(maxval + 0.5);  /* largest value in input array */
    imaxsize = L_MIN(imaxval, maxsize + 1);  /* largest value accepted
                                              for binning */
    nbins = 1 + imaxsize / binsize;  /* size of histogram array */

    if ((nai = numaConvertToInt(na)) == NULL)
        return (NUMA *)ERROR_PTR("nai not made", procName, NULL);
    n = numaGetCount(nai);

    if ((nahist = numaCreate(nbins)) == NULL)
        return (NUMA *)ERROR_PTR("nahist not made", procName, NULL);
    nahist->n = nbins;  /* fake the storage of zeroes in each of the bins */
    for (i = 0; i < n; i++) {
	numaGetIValue(nai, i, &ival);
        ibin = ival / binsize;
	if (ibin >= 0 && ibin < nbins) {
	    numaGetIValue(nahist, ibin, &hval);
	    numaSetValue(nahist, ibin, hval + 1.0);
	}
    }

    numaDestroy(&nai);
    return nahist;
}


/*!
 *  numaRebinHistogram()
 *
 *      Input:  nas (input histogram)
 *              newsize (number of old bins contained in each new bin)
 *      Return: nad (more coarsely re-binned histogram), or null on error
 */
NUMA *
numaRebinHistogram(NUMA    *nas,
                   l_int32  newsize)
{
l_int32  i, j, ns, nd, index, count, val;
NUMA    *nad;

    PROCNAME("numaRebinHistogram");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    if (newsize <= 1)
        return (NUMA *)ERROR_PTR("newsize must be > 1", procName, NULL);
    if ((ns = numaGetCount(nas)) == 0)
        return (NUMA *)ERROR_PTR("no bins in nas", procName, NULL);

    nd = (ns + newsize - 1) / newsize;
    if ((nad = numaCreate(nd)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);

    for (i = 0; i < nd; i++) {  /* new bins */
        count = 0;
        index = i * newsize;
        for (j = 0; j < newsize; j++) {
            if (index < ns) {
                numaGetIValue(nas, index, &val);
                count += val;
                index++;
            }
        }
        numaAddNumber(nad, count);
    }

    return nad;
}


/*!
 *  numaNormalizeHistogram()
 *
 *      Input:  nas (input histogram)
 *              area (target sum of all numbers in dest histogram;
 *                    e.g., use area = 1.0 if this represents a
 *                    probability distribution)
 *      Return: nad (normalized histogram), or null on error
 */
NUMA *
numaNormalizeHistogram(NUMA      *nas,
                       l_float32  area)
{
l_int32    i, ns;
l_float32  sum, factor, fval;
NUMA      *nad;

    PROCNAME("numaNormalizeHistogram");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    if (area <= 0.0)
        return (NUMA *)ERROR_PTR("area must be > 0.0", procName, NULL);
    if ((ns = numaGetCount(nas)) == 0)
        return (NUMA *)ERROR_PTR("no bins in nas", procName, NULL);

    numaGetSum(nas, &sum);
    factor = area / sum;

    if ((nad = numaCreate(ns)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);

    for (i = 0; i < ns; i++) {
        numaGetFValue(nas, i, &fval);
        fval *= factor;
        numaAddNumber(nad, fval);
    }

    return nad;
}


/*!
 *  numaHistogramGetRankFromVal()
 *
 *      Input:  na
 *              startval (assigned to the first bin bucket)
 *              binsize
 *              rval (value of input sample for which we want the rank)
 *              &rank (<return> fraction of total samples below rval)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      - If we think of the histogram as a function y(x), normalized
 *        to 1, for a given input value of x, this computes the
 *        rank of x, which is the integral of y(x) from the start
 *        value of x to the input value.
 *      - This function only makes sense when applied to a Numa that
 *        is a histogram.  The values in the histogram can be ints and
 *        floats, and are computed as floats.  The rank is returned
 *        as a float between 0.0 and 1.0.
 *      - startval and binsize are used to compute x from the Numa index i.
 */
l_int32
numaHistogramGetRankFromVal(NUMA       *na,
                            l_int32     startval,
                            l_int32     binsize,
                            l_float32   rval,
                            l_float32  *prank)
{
l_int32    i, ibinval, n;
l_float32  binval, fractval, total, sum, val;

    PROCNAME("numaHistogramGetRankFromVal");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!prank)
        return ERROR_INT("prank not defined", procName, 1);
    if (binsize < 1)
        binsize = 1;
    if (rval < startval)
        return ERROR_INT("rval less than startval", procName, 1);

    n = numaGetCount(na);
    binval = (rval - (l_float32)startval) / (l_float32)binsize;
    if (binval >= (l_float32)n) {
        *prank = 1.0;
	return 0;
    }

    ibinval = (l_int32)binval;
    fractval = binval - (l_float32)ibinval;

    sum = 0.0;
    for (i = 0; i < ibinval; i++) {
        numaGetFValue(na, i, &val);
        sum += val;
    }
    numaGetFValue(na, ibinval, &val);
    sum += fractval * val;
    numaGetSum(na, &total);
    *prank = sum / total;

/*    fprintf(stderr, "binval = %7.3f, rank = %7.3f\n", binval, *prank); */

    return 0;
}


/*!
 *  numaHistogramGetValFromRank()
 *
 *      Input:  na
 *              startval (assigned to the first bin bucket)
 *              binsize
 *              rank (fraction of total samples)
 *              &rval (<return> approx. to the bin value)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      - If we think of the histogram as a function y(x), this returns
 *        the value x such that the integral of y(x) from the start
 *        value to x gives the fraction 'rank' of the integral
 *        of y(x) over all bins.
 *      - This function only makes sense when applied to a Numa that
 *        is a histogram.  The values in the histogram can be ints and
 *        floats, and are computed as floats.  The val is returned
 *        as a float, even though the buckets are of integer width.
 *      - startval and binsize are used to compute x from the Numa index i.
 */
l_int32
numaHistogramGetValFromRank(NUMA       *na,
                            l_int32     startval,
                            l_int32     binsize,
                            l_float32   rank,
                            l_float32  *prval)
{
l_int32    i, n;
l_float32  rankcount, total, sum, fract, val;

    PROCNAME("numaHistogramGetValFromRank");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!prval)
        return ERROR_INT("prval not defined", procName, 1);
    if (binsize < 1)
        binsize = 1;
    if (rank < 0.0) {
        L_WARNING("rank < 0; setting to 0.0", procName);
        rank = 0.0;
    }
    if (rank > 1.0) {
        L_WARNING("rank > 1.0; setting to 1.0", procName);
        rank = 1.0;
    }

    numaGetSum(na, &total);
    rankcount = rank * total;  /* count that corresponds to rank */
    n = numaGetCount(na);
    sum = 0.0;
    for (i = 0; i < n; i++) {
        numaGetFValue(na, i, &val);
        if (sum + val >= rankcount)
            break;
        sum += val;
    }
    if (val <= 0.0)  /* shouldn't happen at the break */
        fract = 0.0;
    else  /* sum + fract * val = rankcount */
        fract = (rankcount - sum) / val;

    *prval = (l_float32)startval + (l_float32)binsize * 
                ((l_float32)i + fract);

/*    fprintf(stderr, "rank = %7.3f, val = %7.3f\n", rank, *prval); */

    return 0;
}


/*!
 *  numaConvolve()
 *
 *      Input:  na
 *              halfwidth (of rectangular filter, minus the center)
 *      Return: na (after low-pass filtering), or null on error
 *
 *  Notes:
 *      (1) Full convolution takes place only from i = halfwidth to
 *          i = n - halfwidth - 1.  We do the end parts using only
 *          the partial array available.  We do not pad the ends with 0.
 *      (2) This implementation assumes specific fields in the Numa!
 */
NUMA *
numaConvolve(NUMA    *na,
             l_int32  halfwidth)
{
l_int32     i, n, rval;
l_float32   sum, norm;
l_float32  *array, *carray, *sumarray;
NUMA       *nac;

    PROCNAME("numaConvolve");

    if (!na)
        return (NUMA *)ERROR_PTR("na not defined", procName, NULL);
    n = numaGetCount(na);
    if (2 * halfwidth + 1 > n)
        L_WARNING("filter wider than input array!", procName);
    array = na->array;

    if ((nac = numaCreate(n)) == NULL)
        return (NUMA *)ERROR_PTR("nac not made", procName, NULL);
    carray = nac->array;
    nac->n = n;  /* fill with zeroes */

        /* Make sum array; note the indexing */
    if ((sumarray = (l_float32 *)CALLOC(n + 1, sizeof(l_float32))) == NULL)
        return (NUMA *)ERROR_PTR("sumarray not made", procName, NULL);
    sum = 0.0;
    sumarray[0] = 0.0;
    for (i = 0; i < n; i++) {
        sum += array[i];
	sumarray[i + 1] = sum;
    }

        /* Central part */
    norm = 1. / (2 * halfwidth + 1);
    rval = n - halfwidth;
    for (i = halfwidth; i < rval; ++i)
        carray[i] = norm *
	              (sumarray[i + halfwidth + 1] - sumarray[i - halfwidth]);

	/* Left side */
    for (i = 0; i < halfwidth; i++)
        carray[i] = sumarray[i + halfwidth + 1] / (halfwidth + i + 1);

	/* Right side */
    for (i = rval; i < n; i++)
        carray[i] = (1. / (n - i + halfwidth)) *
	               (sumarray[n] - sumarray[i - halfwidth]);
        
    FREE(sumarray);
    return nac;
}


/*!
 *  numaFindPeaks()
 *
 *      Input:  source na
 *              max number of peaks to be found
 *              fract1  (min fraction of peak value)
 *              fract2  (min slope)
 *      Return: peak na, or null on error.
 *
 * Note: the returned na consists of sets of four numbers representing
 *       the peak, in the following order: left edge; peak center;
 *       right edge; normalized peak area.
 */
NUMA *
numaFindPeaks(NUMA      *nas,
	      l_int32    nmax,
	      l_float32  fract1,
	      l_float32  fract2)
{
l_int32    i, k, n, maxloc, lloc, rloc;
l_float32  fmaxval, sum, total, newtotal, val, lastval;
l_float32  peakfract;
NUMA      *na, *napeak;

    PROCNAME("numaFindPeaks");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    n = numaGetCount(nas);
    numaGetSum(nas, &total);

	/* We munge this copy */
    if ((na = numaCopy(nas)) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    if ((napeak = numaCreate(4 * nmax)) == NULL)
        return (NUMA *)ERROR_PTR("napeak not made", procName, NULL);

    for (k = 0; k < nmax; k++) {
	numaGetSum(na, &newtotal);
	if (newtotal == 0.0)   /* sanity check */
	    break;
	numaGetMax(na, &fmaxval, &maxloc);
	sum = fmaxval;
	lastval = fmaxval;
	lloc = 0;
	for (i = maxloc - 1; i >= 0; --i) {
	    numaGetFValue(na, i, &val);
	    if (val == 0.0) {
		lloc = i + 1;
		break;
	    }
	    if (val > fract1 * fmaxval) {
		sum += val;
		lastval = val;
		continue;
	    }
	    if (lastval - val > fract2 * lastval) {
		sum += val;
		lastval = val;
		continue;
	    }
	    lloc = i;
	    break;
	}
	lastval = fmaxval;
	rloc = n - 1;
	for (i = maxloc + 1; i < n; ++i) {
	    numaGetFValue(na, i, &val);
	    if (val == 0.0) {
		rloc = i - 1;
		break;
	    }
	    if (val > fract1 * fmaxval) {
		sum += val;
		lastval = val;
		continue;
	    }
	    if (lastval - val > fract2 * lastval) {
		sum += val;
		lastval = val;
		continue;
	    }
	    rloc = i;
	    break;
	}
	peakfract = sum / total;
	numaAddNumber(napeak, lloc);
	numaAddNumber(napeak, maxloc);
	numaAddNumber(napeak, rloc);
	numaAddNumber(napeak, peakfract);

	for (i = lloc; i <= rloc; i++)
	    numaSetValue(na, i, 0.0);
    }

    numaDestroy(&na);
    return napeak;
}


/*!
 *  numaMakeSequence()
 *
 *      Input:  startval
 *              increment
 *              size (of sequence)
 *      Return: numa of sequence of evenly spaced values, or null on error
 */
NUMA *
numaMakeSequence(l_float32  startval,
		 l_float32  increment,
		 l_int32    size)
{
l_int32    i;
l_float32  val;
NUMA      *na;

    PROCNAME("numaMakeSequence");

    if ((na = numaCreate(size)) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);

    for (i = 0; i < size; i++) {
        val = startval + i * increment;
	numaAddNumber(na, val);
    }

    return na;
}


/*!
 *  numaGetNonzeroRange()
 *
 *      Input:  numa
 *             &first, &last (<return> interval of array indices
 *                            where values are nonzero)
 *      Return: 0 if OK, 1 on error or if no nonzero range is found.
 */
l_int32
numaGetNonzeroRange(NUMA     *na,
                    l_int32  *pfirst,
                    l_int32  *plast)
{
l_int32    n, i, found;
l_float32  val;

    PROCNAME("numaGetNonzeroRange");

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pfirst || !plast)
        return ERROR_INT("pfirst and plast not both defined", procName, 1);
    n = numaGetCount(na);
    found = FALSE;
    for (i = 0; i < n; i++) {
	numaGetFValue(na, i, &val);
	if (val != 0.0) {
	    found = TRUE;
	    break;
	}
    }
    if (!found) {
	*pfirst = n - 1;
	*plast = 0;
	return 1;
    }

    *pfirst = i;
    for (i = n - 1; i >= 0; i--) {
	numaGetFValue(na, i, &val);
	if (val != 0.0)
	    break;
    }
    *plast = i;
    return 0;
}


/*!
 *  numaClipToInterval()
 *
 *      Input:  numa
 *      Return: numa with the same values as the input, but clipped
 *              to the specified interval
 *
 *  Note: If you want the indices of the array values to be unchanged,
 *        use first = 0.
 *  Usage: This is useful to clip a histogram that has a few nonzero
 *         values to its nonzero range.
 */
NUMA *
numaClipToInterval(NUMA    *nas,
	           l_int32  first,
		   l_int32  last)
{
l_int32    n, i, truelast;
l_float32  val;
NUMA      *nad;

    PROCNAME("numaClipToInterval");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    if (first > last)
        return (NUMA *)ERROR_PTR("range not valid", procName, NULL);

    n = numaGetCount(nas);
    if (first >= n)
        return (NUMA *)ERROR_PTR("no elements in range", procName, NULL);
    truelast = L_MIN(last, n - 1);
    if ((nad = numaCreate(truelast - first + 1)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);
    for (i = first; i <= truelast; i++) {
	numaGetFValue(nas, i, &val);
	numaAddNumber(nad, val);
    }
    
    return nad;
}


/*----------------------------------------------------------------------*
 *                          Numa combination                            *
 *----------------------------------------------------------------------*/
/*!
 *  numaJoin()
 *
 *      Input:  nad  (dest numa; add to this one)
 *              nas  (source numa; add from this one)
 *              istart  (starting index in nas)
 *              iend  (ending index in nas; use 0 to cat all)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) istart < 0 is taken to mean 'read from the start' (istart = 0)
 *      (2) iend <= 0 means 'read to the end'
 */
l_int32
numaJoin(NUMA    *nad,
         NUMA    *nas,
         l_int32  istart,
         l_int32  iend)
{
l_int32    ns, i;
l_float32  val;

    PROCNAME("numaJoin");

    if (!nad)
	return ERROR_INT("nad not defined", procName, 1);
    if (!nas)
	return ERROR_INT("nas not defined", procName, 1);
    ns = numaGetCount(nas);
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
	numaGetFValue(nas, i, &val);
	numaAddNumber(nad, val);
    }

    return 0;
}



/*----------------------------------------------------------------------*
 *                        Serialize for I/O                             *
 *----------------------------------------------------------------------*/
/*!
 *  numaRead()
 *
 *      Input:  filename
 *      Return: na, or null on error
 */
NUMA *
numaRead(const char  *filename)
{
FILE  *fp;
NUMA  *na;

    PROCNAME("numaRead");

    if (!filename)
        return (NUMA *)ERROR_PTR("filename not defined", procName, NULL);

    if ((fp = fopenReadStream(filename)) == NULL)
        return (NUMA *)ERROR_PTR("stream not opened", procName, NULL);

    if ((na = numaReadStream(fp)) == NULL) {
	fclose(fp);
        return (NUMA *)ERROR_PTR("na not read", procName, NULL);
    }

    fclose(fp);
    return na;
}
	

/*!
 *  numaReadStream()
 *
 *      Input:  stream
 *      Return: numa, or null on error
 */
NUMA *
numaReadStream(FILE  *fp)
{
l_int32    i, n, index;
l_float32  val;
NUMA      *na;

    PROCNAME("numaReadStream");

    if (!fp)
        return (NUMA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "\nnumarray: number of numbers = %d\n", &n) != 1)
	return (NUMA *)ERROR_PTR("not a numarray file", procName, NULL);

    if ((na = numaCreate(n)) == NULL)
	return (NUMA *)ERROR_PTR("na not made", procName, NULL);

    for (i = 0; i < n; i++) {
	if ((fscanf(fp, "  array[%d] = %f\n", &index, &val)) != 2)
	    return (NUMA *)ERROR_PTR("bad input data", procName, NULL);
	numaAddNumber(na, val);
    }

    return na;
}
	

/*!
 *  numaWrite()
 *
 *      Input:  filename, na
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaWrite(const char  *filename,
          NUMA        *na)
{
FILE  *fp;

    PROCNAME("numaWrite");

    if (!filename)
        return ERROR_INT("filename not defined", procName, 1);
    if (!na)
        return ERROR_INT("na not defined", procName, 1);

    if ((fp = fopen(filename, "w")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    if (numaWriteStream(fp, na))
        return ERROR_INT("na not written to stream", procName, 1);
    fclose(fp);

    return 0;
}
	

/*!
 *  numaWriteStream()
 *
 *      Input:  stream, na
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaWriteStream(FILE  *fp,
                NUMA  *na)
{
l_int32  i, n;

    PROCNAME("numaWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!na)
        return ERROR_INT("na not defined", procName, 1);

    n = numaGetCount(na);
    fprintf(fp, "\nnumarray: number of numbers = %d\n", n);
    for (i = 0; i < n; i++)
	fprintf(fp, "  array[%d] = %f\n", i, na->array[i]);
    fprintf(fp, "\n");

    return 0;
}
	


/*--------------------------------------------------------------------------*
 *                     Numaa creation, destruction                          *
 *--------------------------------------------------------------------------*/
/*!
 *  numaaCreate()
 *
 *      Input:  size of numa ptr array to be alloc'd (0 for default)
 *      Return: naa, or null on error
 *
 */
NUMAA *
numaaCreate(l_int32  n)
{
NUMAA  *naa;

    PROCNAME("numaaCreate");

    if (n <= 0)
	n = INITIAL_PTR_ARRAYSIZE;

    if ((naa = (NUMAA *)CALLOC(1, sizeof(NUMAA))) == NULL)
        return (NUMAA *)ERROR_PTR("naa not made", procName, NULL);
    if ((naa->numa = (NUMA **)CALLOC(n, sizeof(NUMA *))) == NULL)
        return (NUMAA *)ERROR_PTR("numa ptr array not made", procName, NULL);

    naa->nalloc = n;
    naa->n = 0;

    return naa;
}


/*!
 *  numaaDestroy()
 *
 *      Input: &numaa <to be nulled if it exists>
 *      Return: void
 */
void
numaaDestroy(NUMAA  **pnaa)
{
l_int32  i;
NUMAA   *naa;

    PROCNAME("numaaDestroy");

    if (pnaa == NULL) {
	L_WARNING("ptr address is NULL!", procName);
	return;
    }

    if ((naa = *pnaa) == NULL)
	return;

    for (i = 0; i < naa->n; i++)
	numaDestroy(&naa->numa[i]);
    FREE(naa->numa);
    FREE(naa);
    *pnaa = NULL;

    return;
}

        

/*--------------------------------------------------------------------------*
 *                              Add Numa to Numaa                           *
 *--------------------------------------------------------------------------*/
/*!
 *  numaaAddNuma()
 *
 *      Input:  numaa
 *              numa     (to be added)
 *              copyflag  (L_INSERT, L_COPY, L_CLONE)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaaAddNuma(NUMAA   *naa,
             NUMA    *na,
	     l_int32  copyflag)
{
l_int32  n;
NUMA    *nac;

    PROCNAME("numaaAddNuma");

    if (!naa)
        return ERROR_INT("naa not defined", procName, 1);
    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    
    if (copyflag == L_INSERT)
	nac = na;
    else if (copyflag == L_COPY) {
	if ((nac = numaCopy(na)) == NULL)
	    return ERROR_INT("nac not made", procName, 1);
    }
    else if (copyflag == L_CLONE)
        nac = numaClone(na);
    else
        return ERROR_INT("invalid copyflag", procName, 1);

    n = numaaGetCount(naa);
    if (n >= naa->nalloc)
	numaaExtendArray(naa);
    naa->numa[n] = nac;
    naa->n++;
    return 0;
}


/*!
 *  numaaExtendArray()
 *
 *      Input:  numaa
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaaExtendArray(NUMAA  *naa)
{

    PROCNAME("numaaExtendArray");

    if (!naa)
        return ERROR_INT("naa not defined", procName, 1);

    if ((naa->numa = (NUMA **)reallocNew((void **)&naa->numa,
                              sizeof(l_intptr_t) * naa->nalloc,
                              2 * sizeof(l_intptr_t) * naa->nalloc)) == NULL)
	    return ERROR_INT("new ptr array not returned", procName, 1);

    naa->nalloc *= 2;
    return 0;
}


/*----------------------------------------------------------------------*
 *                           Numaa accessors                            *
 *----------------------------------------------------------------------*/
/*!
 *  numaaGetCount()
 *
 *      Input:  numaa
 *      Return: count (number of numa), or 0 if no numa or on error
 */
l_int32
numaaGetCount(NUMAA  *naa)
{
    PROCNAME("numaaGetCount");

    if (!naa)
        return ERROR_INT("naa not defined", procName, 0);
    return naa->n;
}
	

/*!
 *  numaaGetNumberCount()
 *
 *      Input:  numaa
 *      Return: count (number of numbers), or 0 if no numbers or on error
 */
l_int32
numaaGetNumberCount(NUMAA  *naa)
{
NUMA    *na;
l_int32  n, sum, i;

    PROCNAME("numaaGetNumberCount");

    if (!naa)
        return ERROR_INT("naa not defined", procName, 0);

    n = numaaGetCount(naa);
    for (sum = 0, i = 0; i < n; i++) {
        na = numaaGetNuma(naa, i, L_CLONE);
	sum += numaGetCount(na);
	numaDestroy(&na);
    }

    return sum;
}
	

/*!
 *  numaaGetNuma()
 *
 *      Input:  numaa
 *              index  (to the index-th numa)
 *              accessflag   (L_COPY or L_CLONE)
 *      Return: numa, or null on error
 */
NUMA *
numaaGetNuma(NUMAA   *naa,
             l_int32  index,
	     l_int32  accessflag)
{
    PROCNAME("numaaGetNuma");

    if (!naa)
	return (NUMA *)ERROR_PTR("naa not defined", procName, NULL);
    if (index < 0 || index >= naa->n)
	return (NUMA *)ERROR_PTR("index not valid", procName, NULL);

    if (accessflag == L_COPY)
	return numaCopy(naa->numa[index]);
    else if (accessflag == L_CLONE)
	return numaClone(naa->numa[index]);
    else
	return (NUMA *)ERROR_PTR("invalid accessflag", procName, NULL);
}


/*!
 *  numaaReplaceNuma()
 *
 *      Input:  numaa
 *              index  (to the index-th numa)
 *              numa (insert and replace any existing one)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Any existing numa is destroyed, and the input one
 *          is inserted in its place.
 *      (2) If the index is invalid, return 1 (error)
 */
l_int32
numaaReplaceNuma(NUMAA   *naa,
                 l_int32  index,
	         NUMA    *na)
{
l_int32  n;

    PROCNAME("numaaReplaceNuma");

    if (!naa)
	return ERROR_INT("naa not defined", procName, 1);
    if (!na)
	return ERROR_INT("na not defined", procName, 1);
    n = numaaGetCount(naa);
    if (index < 0 || index >= n)
	return ERROR_INT("index not valid", procName, 1);

    numaDestroy(&naa->numa[index]);
    naa->numa[index] = na;
    return 0;
}



/*!
 *  numaaAddNumber()
 *
 *      Input:  naa
 *              index (of numa within numaa)
 *              val  (float or int to be added; stored as a float)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Adds to an existing numa only.
 */
l_int32
numaaAddNumber(NUMAA     *naa,
               l_int32    index,
               l_float32  val)
{
l_int32  n;
NUMA    *na;

    PROCNAME("numaaAddNumber");

    if (!naa)
        return ERROR_INT("naa not defined", procName, 1);
    n = numaaGetCount(naa);
    if (index < 0 || index >= n)
        return ERROR_INT("invalid index in naa", procName, 1);

    na = numaaGetNuma(naa, index, L_CLONE);
    numaAddNumber(na, val);
    numaDestroy(&na);
    return 0;
}


/*--------------------------------------------------------------------------*
 *                      Numa2d creation, destruction                        *
 *--------------------------------------------------------------------------*/
/*!
 *  numa2dCreate()
 *
 *      Input:  nrows (of 2d array)
 *              ncols (of 2d array)
 *              initsize (initial size of each allocated numa)
 *      Return: numa2d, or null on error
 *
 *  Notes:
 *      (1) The numa2d holds a doubly-indexed array of numa.
 *      (2) The numa ptr array is initialized with all ptrs set to NULL.
 *      (3) The numas are created only when a number is to be stored
 *          at an index (i,j) for which a numa has not yet been made.
 */
NUMA2D *
numa2dCreate(l_int32  nrows,
             l_int32  ncols,
	     l_int32  initsize)
{
l_int32  i;
NUMA2D  *na2d;

    PROCNAME("numa2dCreate");

    if (nrows <= 1 || ncols <= 1)
        return (NUMA2D *)ERROR_PTR("rows, cols not both >= 1", procName, NULL);

    if ((na2d = (NUMA2D *)CALLOC(1, sizeof(NUMA2D))) == NULL)
        return (NUMA2D *)ERROR_PTR("na2d not made", procName, NULL);
    na2d->nrows = nrows;
    na2d->ncols = ncols;
    na2d->initsize = initsize;

        /* Set up the 2D array */
    if ((na2d->numa = (NUMA ***)CALLOC(nrows, sizeof(NUMA **))) == NULL)
        return (NUMA2D *)ERROR_PTR("numa row array not made", procName, NULL);
    for (i = 0; i < nrows; i++) {
        if ((na2d->numa[i] = (NUMA **)CALLOC(ncols, sizeof(NUMA *))) == NULL)
	    return (NUMA2D *)ERROR_PTR("numa cols not made", procName, NULL);
    }

    return na2d;
}


/*!
 *  numa2dDestroy()
 *
 *      Input:  &numa2d (<to be nulled if it exists>)
 *      Return: void
 */
void
numa2dDestroy(NUMA2D  **pna2d)
{
l_int32  i, j;
NUMA2D  *na2d;

    PROCNAME("numa2dDestroy");

    if (pna2d == NULL) {
	L_WARNING("ptr address is NULL!", procName);
	return;
    }

    if ((na2d = *pna2d) == NULL)
	return;

    for (i = 0; i < na2d->nrows; i++) {
        for (j = 0; j < na2d->ncols; j++)
	    numaDestroy(&na2d->numa[i][j]);
        FREE(na2d->numa[i]);
    }
    FREE(na2d->numa);
    FREE(na2d);
    *pna2d = NULL;

    return;
}


        
/*--------------------------------------------------------------------------*
 *                               Numa2d accessors                           *
 *--------------------------------------------------------------------------*/
/*!
 *  numa2dAddNumber()
 *
 *      Input:  na2d
 *              row of 2d array
 *              col of 2d array
 *              val  (float or int to be added; stored as a float)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numa2dAddNumber(NUMA2D    *na2d,
                l_int32    row,
	        l_int32    col,
	        l_float32  val)
{
NUMA  *na;

    PROCNAME("numa2dAddNumber");

    if (!na2d)
        return ERROR_INT("na2d not defined", procName, 1);
    if (row < 0 || row >= na2d->nrows)
        return ERROR_INT("row out of bounds", procName, 1);
    if (col < 0 || col >= na2d->ncols)
        return ERROR_INT("col out of bounds", procName, 1);

    if ((na = na2d->numa[row][col]) == NULL) {
        na = numaCreate(na2d->initsize);
	na2d->numa[row][col] = na;
    }
    numaAddNumber(na, val);
    return 0;
}


/*!
 *  numa2dGetCount()
 *
 *      Input:  na2d
 *              row of 2d array
 *              col of 2d array
 *      Return: size of numa at [row][col], or 0 if the numa doesn't exist
 *              or on error
 */
l_int32
numa2dGetCount(NUMA2D  *na2d,
               l_int32  row,
	       l_int32  col)
{
NUMA  *na;

    PROCNAME("numa2dGetCount");

    if (!na2d)
        return ERROR_INT("na2d not defined", procName, 0);
    if (row < 0 || row >= na2d->nrows)
        return ERROR_INT("row out of bounds", procName, 0);
    if (col < 0 || col >= na2d->ncols)
        return ERROR_INT("col out of bounds", procName, 0);
    if ((na = na2d->numa[row][col]) == NULL)
        return 0;
    else
        return na->n;
}


/*!
 *  numa2dGetNuma()
 *
 *      Input:  na2d
 *              row of 2d array
 *              col of 2d array
 *      Return: na (a clone of the numa if it exists) or null if it doesn't
 *
 *  Note: this does not give an error if the index is out of bounds.
 */
NUMA *
numa2dGetNuma(NUMA2D     *na2d,
              l_int32     row,
	      l_int32     col)
{
NUMA  *na;

    PROCNAME("numa2dGetNuma");

    if (!na2d)
        return (NUMA *)ERROR_PTR("na2d not defined", procName, NULL);
    if (row < 0 || row >= na2d->nrows || col < 0 || col >= na2d->ncols)
        return NULL;
    if ((na = na2d->numa[row][col]) == NULL)
        return NULL;
    return numaClone(na);
}


/*!
 *  numa2dGetFValue()
 *
 *      Input:  na2d
 *              row of 2d array
 *              col of 2d array
 *              index (into numa)
 *              &val (<return> float value)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numa2dGetFValue(NUMA2D     *na2d,
                l_int32     row,
	        l_int32     col,
	        l_int32     index,
	        l_float32  *pval)
{
NUMA  *na;

    PROCNAME("numa2dGetFValue");

    if (!na2d)
        return ERROR_INT("na2d not defined", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0.0;

    if (row < 0 || row >= na2d->nrows)
        return ERROR_INT("row out of bounds", procName, 1);
    if (col < 0 || col >= na2d->ncols)
        return ERROR_INT("col out of bounds", procName, 1);
    if ((na = na2d->numa[row][col]) == NULL)
        return ERROR_INT("numa does not exist", procName, 1);

    return numaGetFValue(na, index, pval);
}


/*!
 *  numa2dGetIValue()
 *
 *      Input:  na2d
 *              row of 2d array
 *              col of 2d array
 *              index (into numa)
 *              &val (<return> integer value)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numa2dGetIValue(NUMA2D   *na2d,
                l_int32   row,
	        l_int32   col,
	        l_int32   index,
	        l_int32  *pval)
{
NUMA  *na;

    PROCNAME("numa2dGetIValue");

    if (!na2d)
        return ERROR_INT("na2d not defined", procName, 1);
    if (!pval)
        return ERROR_INT("&val not defined", procName, 1);
    *pval = 0;

    if (row < 0 || row >= na2d->nrows)
        return ERROR_INT("row out of bounds", procName, 1);
    if (col < 0 || col >= na2d->ncols)
        return ERROR_INT("col out of bounds", procName, 1);
    if ((na = na2d->numa[row][col]) == NULL)
        return ERROR_INT("numa does not exist", procName, 1);

    return numaGetIValue(na, index, pval);
}


/*--------------------------------------------------------------------------*
 *               Number array hash: Creation and destruction                *
 *--------------------------------------------------------------------------*/
/*!
 *  numaHashCreate()
 *
 *      Input: nbuckets (the number of buckets in the hash table,
 *                       which should be prime.)
 *             initsize (initial size of each allocated numa; 0 for default)
 *      Return: ptr to new nahash, or null on error
 *
 *  Note: actual numa are created only as required by numaHashAdd()
 */
NUMAHASH *
numaHashCreate(l_int32  nbuckets,
               l_int32  initsize)
{
NUMAHASH  *nahash;

    PROCNAME("numaHashCreate");
    
    if (nbuckets <= 0)
        return (NUMAHASH *)ERROR_PTR("negative hash size", procName, NULL);
    if ((nahash = (NUMAHASH *)CALLOC(1, sizeof(NUMAHASH))) == NULL)
        return (NUMAHASH *)ERROR_PTR("nahash not made", procName, NULL);
    if ((nahash->numa = (NUMA **)CALLOC(nbuckets, sizeof(NUMA *))) == NULL) {
        FREE(nahash);
        return (NUMAHASH *)ERROR_PTR("numa ptr array not made", procName, NULL);
    }

    nahash->nbuckets = nbuckets;
    nahash->initsize = initsize;
    return nahash;
}


/*!
 *  numaHashDestroy()
 *
 *      Input:  &nahash (<to be nulled, if it exists>)
 *      Return: void
 */
void
numaHashDestroy(NUMAHASH **pnahash)
{
NUMAHASH  *nahash;
l_int32    i;

    PROCNAME("numaHashDestroy");

    if (pnahash == NULL) {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }

    if ((nahash = *pnahash) == NULL)
        return;

    for (i = 0; i < nahash->nbuckets; i++)
        numaDestroy(&nahash->numa[i]);
    FREE(nahash->numa);
    FREE(nahash);
    *pnahash = NULL;
}


/*--------------------------------------------------------------------------*
 *               Number array hash: Add elements and return numas
 *--------------------------------------------------------------------------*/
/*!
 *  numaHashGetNuma()
 *
 *      Input:  nahash
 *              key  (key to be hashed into a bucket number)
 *      Return: ptr to numa
 */
NUMA *
numaHashGetNuma(NUMAHASH  *nahash,
                l_uint32   key)
{
l_int32  bucket;
NUMA    *na;

    PROCNAME("numaHashGetNuma");

    if (!nahash)
        return (NUMA *)ERROR_PTR("nahash not defined", procName, NULL);
    bucket = key % nahash->nbuckets;
    na = nahash->numa[bucket];
    if (na)
        return numaClone(na);
    else
        return NULL;
}

/*!
 *  numaHashAdd()
 *
 *      Input:  nahash
 *              key  (key to be hashed into a bucket number)
 *              value  (float value to be appended to the specific numa)
 *      Return: 0 if OK; 1 on error
 */
l_int32
numaHashAdd(NUMAHASH  *nahash,
            l_uint32   key,
            l_float32  value)
{
l_int32  bucket;
NUMA    *na;

    PROCNAME("numaHashAdd");

    if (!nahash)
        return ERROR_INT("nahash not defined", procName, 1);
    if (key < 0)
        return ERROR_INT("key < 0", procName, 1);
    bucket = key % nahash->nbuckets;
    na = nahash->numa[bucket];
    if (!na) {
        if ((na = numaCreate(nahash->initsize)) == NULL)
            return ERROR_INT("na not made", procName, 1);
        nahash->numa[bucket] = na;
    }
    numaAddNumber(na, value);
    return 0;
}


