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
 *   numafunc.c
 *
 *      Simple extractions
 *          l_int32      numaGetMin()
 *          l_int32      numaGetMax()
 *          l_int32      numaGetSum()
 *          NUMA        *numaGetPartialSums()
 *          l_int32      numaGetSumOnInterval()
 *          NUMA        *numaMakeSequence()
 *          l_int32      numaGetNonzeroRange()
 *          NUMA        *numaClipToInterval()
 *
 *      Interpolation
 *          l_int32      numaInterpolateEqxVal()
 *          l_int32      numaInterpolateEqxInterval()
 *          l_int32      numaInterpolateArbxVal()
 *          l_int32      numaInterpolateArbxInterval()
 *
 *      Functions requiring interpolation
 *          l_int32      numaFitMax()
 *          l_int32      numaDifferentiateInterval()
 *          l_int32      numaIntegrateInterval()
 *
 *      Sorting
 *          NUMA        *numaSort()
 *          NUMA        *numaGetSortIndex()
 *          NUMA        *numaSortByIndex()
 *          l_int32      numaIsSorted()
 *          l_int32      numaSortPair()
 *
 *      Functions requiring sorting
 *          l_int32      numaGetMedian()
 *          l_int32      numaGetMode()
 *
 *      Transformations
 *          NUMA        *numaTransform()
 *          NUMA        *numaConvolve()
 *          NUMA        *numaConvertToInt()
 *
 *      Histograms
 *          NUMA        *numaMakeHistogram()
 *          NUMA        *numaMakeHistogramClipped()
 *          NUMA        *numaRebinHistogram()
 *          NUMA        *numaNormalizeHistogram()
 *          l_int32      numaMakeRankFromHistogram()
 *          l_int32      numaHistogramGetRankFromVal()
 *          l_int32      numaHistogramGetValFromRank()
 *
 *      Extrema finding
 *          NUMA        *numaFindPeaks()
 *          NUMA        *numaFindExtrema()
 *
 *      Numa combination
 *          l_int32      numaJoin()
 *
 *    Things to remember when using the Numa:
 *
 *    (1) The numa is a struct, not an array.  Always use accessors
 *        (see numabasic.c), never the fields directly.
 *
 *    (2) The number array holds l_float32 values.  It can also
 *        be used to store l_int32 values.  See numabasic.c for
 *        details on using the accessors.
 *
 *    (3) Occasionally, in the comments we denote the i-th element of a
 *        numa by na[i].  This is conceptual only -- the numa is not an array!
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


#ifndef  NO_CONSOLE_IO
#define  DEBUG_HISTO    0
#endif  /* ~NO_CONSOLE_IO */


/*----------------------------------------------------------------------*
 *                         Simple extractions                           *
 *----------------------------------------------------------------------*/
/*!
 *  numaGetMin()
 *
 *      Input:  na
 *              &minval (<optional return> min value)
 *              &iminloc (<optional return> index of min location)
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

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pminval && !piminloc)
        return ERROR_INT("nothing to don", procName, 1);
    if (pminval) *pminval = 0.0;
    if (piminloc) *piminloc = 0;

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

    if (pminval) *pminval = minval;
    if (piminloc) *piminloc = iminloc;
    return 0;
}


/*!
 *  numaGetMax()
 *
 *      Input:  na
 *              &maxval (<optional return> max value)
 *              &imaxloc (<optional return> index of max location)
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

    if (!na)
        return ERROR_INT("na not defined", procName, 1);
    if (!pmaxval && !pimaxloc)
        return ERROR_INT("nothing to don", procName, 1);
    if (pmaxval) *pmaxval = 0.0;
    if (pimaxloc) *pimaxloc = 0;

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

    if (pmaxval) *pmaxval = maxval;
    if (pimaxloc) *pimaxloc = imaxloc;
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
    if (!psum)
        return ERROR_INT("&sum not defined", procName, 1);

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
 *      (1) nasum[i] is the sum for all j <= i of na[j].
 *          So nasum[0] = na[0].
 *      (2) If you want to generate a rank function, where rank[0] - 0.0,
 *          insert a 0.0 at the beginning of the nasum array.
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
 *              eps (largest value considered to be zero)
 *              &first, &last (<return> interval of array indices
 *                             where values are nonzero)
 *      Return: 0 if OK, 1 on error or if no nonzero range is found.
 */
l_int32
numaGetNonzeroRange(NUMA      *na,
                    l_float32  eps,
                    l_int32   *pfirst,
                    l_int32   *plast)
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
        if (val > eps) {
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
        if (val > eps)
            break;
    }
    *plast = i;
    return 0;
}


/*!
 *  numaClipToInterval()
 *
 *      Input:  numa
 *              first, last (clipping interval)
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
 *                             Interpolation                            *
 *----------------------------------------------------------------------*/
/*!
 *  numaInterpolateEqxVal()
 *
 *      Input:  startx (xval corresponding to first element in array)
 *              deltax (x increment between array elements)
 *              nay  (numa of ordinate values, assumed equally spaced)
 *              type (L_LINEAR_INTERP, L_QUADRATIC_INTERP)
 *              xval
 *              &yval (<return> interpolated value)
 *      Return: 0 if OK, 1 on error (e.g., if xval is outside range)
 *
 *  Notes:
 *      (1) Considering nay as a function of x, the x values
 *          are equally spaced
 *      (2) Caller should check for valid return.
 *
 *  For linear Lagrangian interpolation (through 2 data pts):
 *         y(x) = y1(x-x2)/(x1-x2) + y2(x-x1)/(x2-x1)
 *
 *  For quadratic Lagrangian interpolation (through 3 data pts):
 *         y(x) = y1(x-x2)(x-x3)/((x1-x2)(x1-x3)) +
 *                y2(x-x1)(x-x3)/((x2-x1)(x2-x3)) +
 *                y3(x-x1)(x-x2)/((x3-x1)(x3-x2))
 *
 */
l_int32
numaInterpolateEqxVal(l_float32   startx,
                      l_float32   deltax,
		      NUMA       *nay,
                      l_int32     type,
                      l_float32   xval,
                      l_float32  *pyval)
{
l_int32     i, n, i1, i2, i3;
l_float32   x1, x2, x3, fy1, fy2, fy3, d1, d2, d3, del, fi, maxx;
l_float32  *fa;

    PROCNAME("numaInterpolateEqxVal");

    if (!pyval)
        return ERROR_INT("&yval not defined", procName, 1);
    *pyval = 0.0;
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (deltax <= 0.0)
        return ERROR_INT("deltax not > 0", procName, 1);
    if (type != L_LINEAR_INTERP && type != L_QUADRATIC_INTERP)
        return ERROR_INT("invalid interp type", procName, 1);
    n = numaGetCount(nay);
    if (n < 2)
        return ERROR_INT("not enough points", procName, 1);
    if (type == L_QUADRATIC_INTERP && n == 2) {
        type = L_LINEAR_INTERP;
        L_WARNING("only 2 points; using linear interp", procName);
    }
    maxx = startx + deltax * (n - 1);
    if (xval < startx || xval > maxx)
        return ERROR_INT("xval is out of bounds", procName, 1);

    fa = numaGetFArray(nay, L_NOCOPY);
    fi = (xval - startx) / deltax;
    i = (l_int32)fi;
    del = fi - i;
    if (del == 0.0) {  /* no interpolation required */
        *pyval = fa[i];
	return 0;
    }

    if (type == L_LINEAR_INTERP) {
        *pyval = fa[i] + del * (fa[i + 1] - fa[i]);
	return 0;
    }

        /* Quadratic interpolation */
    d1 = d3 = 0.5 / (deltax * deltax);
    d2 = -2. * d1;
    if (i == 0) {
        i1 = i;
	i2 = i + 1;
	i3 = i + 2;
    }
    else {
        i1 = i - 1;
	i2 = i;
	i3 = i + 1;
    }
    x1 = startx + i1 * deltax;
    x2 = startx + i2 * deltax;
    x3 = startx + i3 * deltax;
    fy1 = d1 * fa[i1];
    fy2 = d2 * fa[i2];
    fy3 = d3 * fa[i3];
    *pyval = fy1 * (xval - x2) * (xval - x3) +
             fy2 * (xval - x1) * (xval - x3) +
             fy3 * (xval - x1) * (xval - x2);
    return 0;
}


/*!
 *  numaInterpolateArbxVal()
 *
 *      Input:  nax (numa of abscissa values)
 *              nay (numa of ordinate values, corresponding to nax)
 *              type (L_LINEAR_INTERP, L_QUADRATIC_INTERP)
 *              xval
 *              &yval (<return> interpolated value)
 *      Return: 0 if OK, 1 on error (e.g., if xval is outside range)
 *
 *  Notes:
 *      (1) The values in nax must be sorted in increasing order.
 *          If, additionally, they are equally spaced, you can use
 *          numaInterpolateEqxVal().
 *      (2) Caller should check for valid return.
 *      (3) Uses lagrangian interpolation.  See numaInterpolateEqxVal()
 *          for formulas.
 */
l_int32
numaInterpolateArbxVal(NUMA       *nax,
                       NUMA       *nay,
                       l_int32     type,
                       l_float32   xval,
                       l_float32  *pyval)
{
l_int32     i, im, nx, ny, i1, i2, i3;
l_float32   delu, dell, fract, d1, d2, d3;
l_float32   minx, maxx;
l_float32  *fax, *fay;

    PROCNAME("numaInterpolateArbxVal");

    if (!pyval)
        return ERROR_INT("&yval not defined", procName, 1);
    *pyval = 0.0;
    if (!nax)
        return ERROR_INT("nax not defined", procName, 1);
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (type != L_LINEAR_INTERP && type != L_QUADRATIC_INTERP)
        return ERROR_INT("invalid interp type", procName, 1);
    ny = numaGetCount(nay);
    nx = numaGetCount(nax);
    if (nx != ny)
        return ERROR_INT("nax and nay not same size arrays", procName, 1);
    if (ny < 2)
        return ERROR_INT("not enough points", procName, 1);
    if (type == L_QUADRATIC_INTERP && ny == 2) {
        type = L_LINEAR_INTERP;
        L_WARNING("only 2 points; using linear interp", procName);
    }
    numaGetFValue(nax, 0, &minx);
    numaGetFValue(nax, nx - 1, &maxx);
    if (xval < minx || xval > maxx)
        return ERROR_INT("xval is out of bounds", procName, 1);

    fax = numaGetFArray(nax, L_NOCOPY);
    fay = numaGetFArray(nay, L_NOCOPY);

        /* Linear search for interval.  We are guaranteed
         * to either return or break out of the loop.
         * In addition, we are assured that fax[i] - fax[im] > 0.0 */
    if (xval == fax[0]) {
        *pyval = fay[0];
        return 0;
    }
    for (i = 1; i < nx; i++) {
        delu = fax[i] - xval;
	if (delu >= 0.0) {  /* we've passed it */
            if (delu == 0.0) {
                *pyval = fay[i];
                return 0;
            }
            im = i - 1;
            dell = xval - fax[im];  /* >= 0 */
            break;
        }
    }
    fract = dell / (fax[i] - fax[im]);

    if (type == L_LINEAR_INTERP) {
        *pyval = fay[i] + fract * (fay[i + 1] - fay[i]);
	return 0;
    }

        /* Quadratic interpolation */
    if (im == 0) {
        i1 = im;
	i2 = im + 1;
	i3 = im + 2;
    }
    else {
        i1 = im - 1;
	i2 = im;
	i3 = im + 1;
    }
    d1 = (fax[i1] - fax[i2]) * (fax[i1] - fax[i3]);
    d2 = (fax[i2] - fax[i1]) * (fax[i2] - fax[i3]);
    d3 = (fax[i3] - fax[i1]) * (fax[i3] - fax[i2]);
    *pyval = fay[i1] * (xval - fax[i2]) * (xval - fax[i3]) / d1 +
             fay[i2] * (xval - fax[i1]) * (xval - fax[i3]) / d2 +
             fay[i3] * (xval - fax[i1]) * (xval - fax[i2]) / d3;
    return 0;
}


/*!
 *  numaInterpolateEqxInterval()
 *
 *      Input:  startx (xval corresponding to first element in nas)
 *              deltax (x increment between array elements in nas)
 *              nasy  (numa of ordinate values, assumed equally spaced)
 *              type (L_LINEAR_INTERP, L_QUADRATIC_INTERP)
 *              x0 (start value of interval)
 *              x1 (end value of interval)
 *              npts (number of points to evaluate function in interval)
 *              &nax (<optional return> array of x values in interval)
 *              &nay (<return> array of y values in interval)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Considering nasy as a function of x, the x values
 *          are equally spaced.
 *      (2) This creates nay (and optionally nax) of interpolated
 *          values over the specified interval (x0, x1).
 *      (3) If the interval (x0, x1) lies partially outside the array
 *          nasy (as interpreted by startx and deltax), it is an
 *          error and returns 1.
 */
l_int32
numaInterpolateEqxInterval(l_float32  startx,
                           l_float32  deltax,
                           NUMA      *nasy,
                           l_int32    type,
                           l_float32  x0,
                           l_float32  x1,
                           l_int32    npts,
                           NUMA     **pnax,
                           NUMA     **pnay)
{
l_int32     i, n;
l_float32   x, yval, maxx;
NUMA       *nax, *nay;

    PROCNAME("numaInterpolateEqxInterval");

    if (!pnay)
        return ERROR_INT("&nay not defined", procName, 1);
    *pnay = NULL;
    if (pnax)
        *pnax = NULL;
    if (!nasy)
        return ERROR_INT("nasy not defined", procName, 1);
    if (deltax <= 0.0)
        return ERROR_INT("deltax not > 0", procName, 1);
    if (type != L_LINEAR_INTERP && type != L_QUADRATIC_INTERP)
        return ERROR_INT("invalid interp type", procName, 1);
    n = numaGetCount(nasy);
    if (type == L_QUADRATIC_INTERP && n == 2) {
        type = L_LINEAR_INTERP;
        L_WARNING("only 2 points; using linear interp", procName);
    }
    maxx = startx + deltax * (n - 1);
    if (x0 < startx || x1 > maxx || x1 <= x0)
        return ERROR_INT("[x0 ... x1] is not valid", procName, 1);

    if ((nay = numaCreate(npts)) == NULL)
        return ERROR_INT("nay not made", procName, 1);
    *pnay = nay;
    if (pnax) {
        nax = numaCreate(npts);
	*pnax = nax;
    }

    for (i = 0; i < npts; i++) {
        x = x0 + i * (x1 - x0) / (l_float32)(npts - 1);
        if (pnax)
            numaAddNumber(nax, x);
        numaInterpolateEqxVal(startx, deltax, nasy, type, x, &yval);
	numaAddNumber(nay, yval);
    }

    return 0;
}
   

/*!
 *  numaInterpolateArbxInterval()
 *
 *      Input:  nax (numa of abscissa values)
 *              nay (numa of ordinate values, corresponding to nax)
 *              type (L_LINEAR_INTERP, L_QUADRATIC_INTERP)
 *              x0 (start value of interval)
 *              x1 (end value of interval)
 *              npts (number of points to evaluate function in interval)
 *              &nadx (<optional return> array of x values in interval)
 *              &nady (<return> array of y values in interval)
 *      Return: 0 if OK, 1 on error (e.g., if x0 or x1 is outside range)
 *
 *  Notes:
 *      (1) The values in nax must be sorted in increasing order.
 *          If they are not sorted, we do it here, and complain.
 *      (2) If the values in nax are equally spaced, you can use
 *          numaInterpolateEqxInterval().
 *      (3) Caller should check for valid return.
 *      (4) We don't call numaInterpolateArbxVal() for each output
 *          point, because that requires an O(n) search for
 *          each point.  Instead, we do a single O(n) pass through
 *          nax, saving the indices to be used for each output yval.
 *      (5) Uses lagrangian interpolation.  See numaInterpolateEqxVal()
 *          for formulas.
 */
l_int32
numaInterpolateArbxInterval(NUMA       *nax,
                            NUMA       *nay,
                            l_int32     type,
                            l_float32   x0,
                            l_float32   x1,
                            l_int32     npts,
                            NUMA      **pnadx,
                            NUMA      **pnady)
{
l_int32     i, im, j, nx, ny, i1, i2, i3, sorted;
l_int32    *index;
l_float32   del, xval, yval, excess, fract, minx, maxx, d1, d2, d3;
l_float32  *fax, *fay;
NUMA       *nasx, *nasy, *nadx, *nady;

    PROCNAME("numaInterpolateArbxInterval");

    if (!pnady)
        return ERROR_INT("&nady not defined", procName, 1);
    *pnady = NULL;
    if (pnadx) *pnadx = NULL;
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (!nax)
        return ERROR_INT("nax not defined", procName, 1);
    if (type != L_LINEAR_INTERP && type != L_QUADRATIC_INTERP)
        return ERROR_INT("invalid interp type", procName, 1);
    if (x0 > x1)
        return ERROR_INT("x0 > x1", procName, 1);
    ny = numaGetCount(nay);
    nx = numaGetCount(nax);
    if (nx != ny)
        return ERROR_INT("nax and nay not same size arrays", procName, 1);
    if (ny < 2)
        return ERROR_INT("not enough points", procName, 1);
    if (type == L_QUADRATIC_INTERP && ny == 2) {
        type = L_LINEAR_INTERP;
        L_WARNING("only 2 points; using linear interp", procName);
    }
    numaGetMin(nax, &minx, NULL);
    numaGetMax(nax, &maxx, NULL);
    if (x0 < minx || x1 > maxx)
        return ERROR_INT("xval is out of bounds", procName, 1);

        /* Make sure that nax is sorted in increasing order */
    numaIsSorted(nax, L_SORT_INCREASING, &sorted);
    if (!sorted) {
        L_WARNING("we are sorting nax in increasing order", procName);
        numaSortPair(nax, nay, L_SORT_INCREASING, &nasx, &nasy);
    }
    else {
        nasx = numaClone(nax);
        nasy = numaClone(nay);
    }

    fax = numaGetFArray(nasx, L_NOCOPY);
    fay = numaGetFArray(nasy, L_NOCOPY);

        /* Get array of indices into fax for interpolated locations */
    if ((index = (l_int32 *)CALLOC(npts, sizeof(l_int32))) == NULL)
        return ERROR_INT("ind not made", procName, 1);
    del = (x1 - x0) / (npts - 1.0);
    for (i = 0, j = 0; j < nx, i < npts; i++) {
        xval = x0 + i * del;
        while (j < nx - 1 && xval > fax[j])
            j++;
        if (xval == fax[j])
            index[i] = L_MIN(j, nx - 1);
        else    /* the index of fax[] is just below xval */
            index[i] = L_MAX(j - 1, 0);
    }

        /* For each point to be interpolated, get the y value */
    nady = numaCreate(npts);
    *pnady = nady;
    if (pnadx) {
        nadx = numaCreate(npts);
	*pnadx = nadx;
    }
    for (i = 0; i < npts; i++) {
        xval = x0 + i * del;
	if (pnadx)
            numaAddNumber(nadx, xval);
	im = index[i];
        excess = xval - fax[im];
	if (excess == 0.0) {
            numaAddNumber(nady, fay[im]);
            continue;
        }
	fract = excess / (fax[im + 1] - fax[im]);

        if (type == L_LINEAR_INTERP) {
            yval = fay[im] + fract * (fay[im + 1] - fay[im]);
            numaAddNumber(nady, yval);
	    continue;
	}

            /* Quadratic interpolation */
        if (im == 0) {
            i1 = im;
            i2 = im + 1;
            i3 = im + 2;
        }
        else {
            i1 = im - 1;
            i2 = im;
            i3 = im + 1;
        }
        d1 = (fax[i1] - fax[i2]) * (fax[i1] - fax[i3]);
        d2 = (fax[i2] - fax[i1]) * (fax[i2] - fax[i3]);
        d3 = (fax[i3] - fax[i1]) * (fax[i3] - fax[i2]);
        yval = fay[i1] * (xval - fax[i2]) * (xval - fax[i3]) / d1 +
               fay[i2] * (xval - fax[i1]) * (xval - fax[i3]) / d2 +
               fay[i3] * (xval - fax[i1]) * (xval - fax[i2]) / d3;
        numaAddNumber(nady, yval);
    }

    FREE(index);
    numaDestroy(&nasx);
    numaDestroy(&nasy);
    return 0;
}


/*----------------------------------------------------------------------*
 *                     Functions requiring interpolation                *
 *----------------------------------------------------------------------*/
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
 *  numaDifferentiateInterval()
 *
 *      Input:  nax (numa of abscissa values)
 *              nay (numa of ordinate values, corresponding to nax)
 *              x0 (start value of interval)
 *              x1 (end value of interval)
 *              npts (number of points to evaluate function in interval)
 *              &nadx (<optional return> array of x values in interval)
 *              &nady (<return> array of derivatives in interval)
 *      Return: 0 if OK, 1 on error (e.g., if x0 or x1 is outside range)
 *
 *  Notes:
 *      (1) The values in nax must be sorted in increasing order.
 *          If they are not sorted, it is done in the interpolation
 *          step, and a warning is issued.
 *      (2) Caller should check for valid return.
 */
l_int32
numaDifferentiateInterval(NUMA       *nax,
                          NUMA       *nay,
                          l_float32   x0,
                          l_float32   x1,
                          l_int32     npts,
                          NUMA      **pnadx,
                          NUMA      **pnady)
{
l_int32     i, nx, ny;
l_float32   minx, maxx, der, invdel;
l_float32  *fay;
NUMA       *nady, *naiy;

    PROCNAME("numaDifferentiateInterval");

    if (!pnady)
        return ERROR_INT("&nady not defined", procName, 1);
    *pnady = NULL;
    if (pnadx) *pnadx = NULL;
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (!nax)
        return ERROR_INT("nax not defined", procName, 1);
    if (x0 > x1)
        return ERROR_INT("x0 > x1", procName, 1);
    ny = numaGetCount(nay);
    nx = numaGetCount(nax);
    if (nx != ny)
        return ERROR_INT("nax and nay not same size arrays", procName, 1);
    if (ny < 2)
        return ERROR_INT("not enough points", procName, 1);
    numaGetMin(nax, &minx, NULL);
    numaGetMax(nax, &maxx, NULL);
    if (x0 < minx || x1 > maxx)
        return ERROR_INT("xval is out of bounds", procName, 1);
    if (npts < 2)
        return ERROR_INT("npts < 2", procName, 1);

        /* Generate interpolated array over specified interval */
    if (numaInterpolateArbxInterval(nax, nay, L_LINEAR_INTERP, x0, x1,
                                    npts, pnadx, &naiy))
        return ERROR_INT("interpolation failed", procName, 1);

    nady = numaCreate(npts);
    *pnady = nady;
    invdel = 0.5 * ((l_float32)npts - 1.0) / (x1 - x0);
    fay = numaGetFArray(naiy, L_NOCOPY);

        /* Compute and save derivatives */
    der = 0.5 * invdel * (fay[1] - fay[0]);
    numaAddNumber(nady, der);
    for (i = 1; i < npts - 1; i++)  {
        der = invdel * (fay[i + 1] - fay[i - 1]);
        numaAddNumber(nady, der);
    }
    der = 0.5 * invdel * (fay[npts - 1] - fay[npts - 2]);
    numaAddNumber(nady, der);

    numaDestroy(&naiy);
    return 0;
}


/*!
 *  numaIntegrateInterval()
 *
 *      Input:  nax (numa of abscissa values)
 *              nay (numa of ordinate values, corresponding to nax)
 *              x0 (start value of interval)
 *              x1 (end value of interval)
 *              npts (number of points to evaluate function in interval)
 *              &sum (<return> integral of function over interval)
 *      Return: 0 if OK, 1 on error (e.g., if x0 or x1 is outside range)
 *
 *  Notes:
 *      (1) The values in nax must be sorted in increasing order.
 *          If they are not sorted, it is done in the interpolation
 *          step, and a warning is issued.
 *      (2) Caller should check for valid return.
 */
l_int32
numaIntegrateInterval(NUMA       *nax,
                      NUMA       *nay,
                      l_float32   x0,
                      l_float32   x1,
                      l_int32     npts,
                      l_float32  *psum)
{
l_int32     i, nx, ny;
l_float32   minx, maxx, sum, del;
l_float32  *fay;
NUMA       *naiy;

    PROCNAME("numaIntegrateInterval");

    if (!psum)
        return ERROR_INT("&sum not defined", procName, 1);
    *psum = 0.0;
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (!nax)
        return ERROR_INT("nax not defined", procName, 1);
    if (x0 > x1)
        return ERROR_INT("x0 > x1", procName, 1);
    if (npts < 2)
        return ERROR_INT("npts < 2", procName, 1);
    ny = numaGetCount(nay);
    nx = numaGetCount(nax);
    if (nx != ny)
        return ERROR_INT("nax and nay not same size arrays", procName, 1);
    if (ny < 2)
        return ERROR_INT("not enough points", procName, 1);
    numaGetMin(nax, &minx, NULL);
    numaGetMax(nax, &maxx, NULL);
    if (x0 < minx || x1 > maxx)
        return ERROR_INT("xval is out of bounds", procName, 1);

        /* Generate interpolated array over specified interval */
    if (numaInterpolateArbxInterval(nax, nay, L_LINEAR_INTERP, x0, x1,
                                    npts, NULL, &naiy))
        return ERROR_INT("interpolation failed", procName, 1);

    del = (x1 - x0) / ((l_float32)npts - 1.0);
    fay = numaGetFArray(naiy, L_NOCOPY);

        /* Compute integral (simple trapezoid) */
    sum = 0.5 * (fay[0] + fay[npts - 1]);
    for (i = 1; i < npts - 1; i++)
        sum += fay[i];
    *psum = del * sum;

    numaDestroy(&naiy);
    return 0;
}


/*----------------------------------------------------------------------*
 *                                Sorting                               *
 *----------------------------------------------------------------------*/
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
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return (NUMA *)ERROR_PTR("invalid sortorder", procName, NULL);

    n = numaGetCount(na);
    if ((array = numaGetFArray(na, L_COPY)) == NULL)
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
 *  numaIsSorted()
 * 
 *      Input:  nas
 *              sortorder (L_SORT_INCREASING or L_SORT_DECREASING)
 *              &sorted (<return> 1 if sorted; 0 if not)
 *      Return: 1 if OK; 0 on error
 *
 *  Notes:
 *      (1) This is a quick O(n) test if nas is sorted.  It is useful
 *          in situations where the array is likely to be already
 *          sorted, and a sort operation can be avoided.
 */
l_int32
numaIsSorted(NUMA     *nas,
             l_int32   sortorder,
	     l_int32  *psorted)
{
l_int32    i, n;
l_float32  preval, val;

    PROCNAME("numaIsSorted");

    if (!psorted)
        return ERROR_INT("&sorted not defined", procName, 1);
    *psorted = FALSE;
    if (!nas)
        return ERROR_INT("nas not defined", procName, 1);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return ERROR_INT("invalid sortorder", procName, 1);

    n = numaGetCount(nas);
    numaGetFValue(nas, 0, &preval);
    for (i = 1; i < n; i++) {
        numaGetFValue(nas, i, &val);
        if ((sortorder == L_SORT_INCREASING && val < preval) ||
            (sortorder == L_SORT_DECREASING && val > preval))
            return 0;
    }

    *psorted = TRUE;
    return 0;
}


/*!
 *  numaSortPair()
 * 
 *      Input:  nax, nay (input arrays)
 *              sortorder (L_SORT_INCREASING or L_SORT_DECREASING)
 *              &nasx (<return> sorted)
 *              &naxy (<return> sorted exactly in order of nasx)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This function sorts the two input arrays, nax and nay,
 *          together, using nax as the key for sorting.
 */
l_int32
numaSortPair(NUMA    *nax,
             NUMA    *nay,
             l_int32  sortorder,
             NUMA   **pnasx,
             NUMA   **pnasy)
{
l_int32  sorted;
NUMA    *naindex;

    PROCNAME("numaSortPair");

    if (!pnasx)
        return ERROR_INT("&nasx not defined", procName, 1);
    if (!pnasy)
        return ERROR_INT("&nasy not defined", procName, 1);
    *pnasx = *pnasy = NULL;
    if (!nax)
        return ERROR_INT("nax not defined", procName, 1);
    if (!nay)
        return ERROR_INT("nay not defined", procName, 1);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return ERROR_INT("invalid sortorder", procName, 1);

    numaIsSorted(nax, sortorder, &sorted);
    if (sorted == TRUE) {
        *pnasx = numaCopy(nax);
        *pnasy = numaCopy(nay);
    }
    else {
        naindex = numaGetSortIndex(nax, sortorder);
	*pnasx = numaSortByIndex(nax, naindex);
	*pnasy = numaSortByIndex(nay, naindex);
	numaDestroy(&naindex);
    }

    return 0;
}



/*----------------------------------------------------------------------*
 *                     Functions requiring sorting                      *
 *----------------------------------------------------------------------*/
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
    if ((array = numaGetFArray(nasort, L_NOCOPY)) == NULL)
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
    return 0;
}


/*----------------------------------------------------------------------*
 *                             Transformations                          *
 *----------------------------------------------------------------------*/
/*!
 *  numaTransform()
 *
 *      Input:  nas
 *              shift (add this to each number)
 *              scale (multiply each number by this)
 *      Return: na with all values shifted and scaled, or null on error
 *
 *  Notes:
 *      (1) Each number is shifted before scaling.
 *      (2) The operation sequence is opposite to that for Box and Pta:
 *          scale first, then shift.
 */
NUMA *
numaTransform(NUMA      *nas,
              l_float32  shift,
              l_float32  scale)
{
l_int32    i, n;
l_float32  val;
NUMA      *nad;

    PROCNAME("numaTransform");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);
    n = numaGetCount(nas);
    if ((nad = numaCreate(n)) == NULL)
        return (NUMA *)ERROR_PTR("nad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        numaGetFValue(nas, i, &val);
        val = scale * val + shift;
        numaAddNumber(nad, val);
    }
    return nad;
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


/*----------------------------------------------------------------------*
 *                     Functions requiring sorting                      *
 *----------------------------------------------------------------------*/
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
 *  numaMakeRankFromHistogram()
 *
 *      Input:  startx (xval corresponding to first element in nay)
 *              deltax (x increment between array elements in nay)
 *              nasy (input histogram, assumed equally spaced)
 *              npts (number of points to evaluate rank function)
 *              &nax (<optional return> array of x values in range)
 *              &nay (<return> rank array of specified npts)
 *      Return: 0 if OK, 1 on error
 */
l_int32
numaMakeRankFromHistogram(l_float32  startx,
                          l_float32  deltax,
                          NUMA      *nasy,
                          l_int32    npts,
                          NUMA     **pnax,
                          NUMA     **pnay)
{
l_int32    i, n;
l_float32  sum, fval;
NUMA      *nan, *nar;

    PROCNAME("numaMakeRankFromHistogram");

    if (!nasy)
        return ERROR_INT("nasy not defined", procName, 1);
    if (!pnay)
        return ERROR_INT("&nay not defined", procName, 1);
    if ((n = numaGetCount(nasy)) == 0)
        return ERROR_INT("no bins in nas", procName, 1);

        /* Normalize and generate the rank array corresponding to
         * the binned histogram. */
    nan = numaNormalizeHistogram(nasy, 1.0);
    nar = numaCreate(n + 1);  /* rank numa corresponding to nan */
    sum = 0.0;
    numaAddNumber(nar, sum);  /* first element is 0.0 */
    for (i = 0; i < n; i++) {
        numaGetFValue(nan, i, &fval);
        sum += fval;
        numaAddNumber(nar, sum);
    }

        /* Compute rank array on full range with specified
         * number of points and correspondence to x-values. */
    numaInterpolateEqxInterval(startx, deltax, nar, L_LINEAR_INTERP,
                               startx, startx + n * deltax, npts,
                               pnax, pnay);
    numaDestroy(&nan);
    numaDestroy(&nar);
    return 0;
}


/*!
 *  numaHistogramGetRankFromVal()
 *
 *      Input:  na (histogram)
 *              startval (assigned to the first bin bucket)
 *              binsize
 *              rval (value of input sample for which we want the rank)
 *              &rank (<return> fraction of total samples below rval)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) If we think of the histogram as a function y(x), normalized
 *          to 1, for a given input value of x, this computes the
 *          rank of x, which is the integral of y(x) from the start
 *          value of x to the input value.
 *      (2) This function only makes sense when applied to a Numa that
 *          is a histogram.  The values in the histogram can be ints and
 *          floats, and are computed as floats.  The rank is returned
 *          as a float between 0.0 and 1.0.
 *      (3) startval and binsize are used to compute x from the Numa index i.
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
 *      Input:  na (histogram)
 *              startval (assigned to the first bin bucket)
 *              binsize
 *              rank (fraction of total samples)
 *              &rval (<return> approx. to the bin value)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) If we think of the histogram as a function y(x), this returns
 *          the value x such that the integral of y(x) from the start
 *          value to x gives the fraction 'rank' of the integral
 *          of y(x) over all bins.
 *      (2) This function only makes sense when applied to a Numa that
 *          is a histogram.  The values in the histogram can be ints and
 *          floats, and are computed as floats.  The val is returned
 *          as a float, even though the buckets are of integer width.
 *      (3) startval and binsize are used to compute x from the Numa index i.
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
    if (val <= 0.0)  /* can == 0 if rank == 0.0 */
        fract = 0.0;
    else  /* sum + fract * val = rankcount */
        fract = (rankcount - sum) / val;

    *prval = (l_float32)startval + (l_float32)binsize * 
                ((l_float32)i + fract);

/*    fprintf(stderr, "rank = %7.3f, val = %7.3f\n", rank, *prval); */

    return 0;
}


/*----------------------------------------------------------------------*
 *                             Extrema finding                          *
 *----------------------------------------------------------------------*/
/*!
 *  numaFindPeaks()
 *
 *      Input:  source na
 *              max number of peaks to be found
 *              fract1  (min fraction of peak value)
 *              fract2  (min slope)
 *      Return: peak na, or null on error.
 *
 * Notes:
 *     (1) The returned na consists of sets of four numbers representing
 *         the peak, in the following order:
 *            left edge; peak center; right edge; normalized peak area
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
 *  numaFindExtrema()
 *
 *      Input:  nas (input values)
 *              delta (relative amount to resolve peaks and valleys)
 *      Return: nad (locations of extrema), or null on error
 *
 *  Notes:
 *      (1) This returns a sequence of extrema (peaks and valleys).
 *      (2) The algorithm is analogous to that for determining
 *          mountain peaks.  Suppose we have a local peak, with
 *          bumps on the side.  Under what conditions can we consider
 *          those 'bumps' to be actual peaks?  The answer: if the
 *          bump is separated from the peak by a saddle that is at
 *          least 500 feet below the bump.
 *      (3) Operationally, suppose we are looking for a peak.
 *          We are keeping the largest value we've seen since the
 *          last valley, and are looking for a value that is delta
 *          BELOW our current peak.  When we find such a value,
 *          we label the peak, use the current value to label the
 *          valley, and then do the same operation in reverse (looking
 *          for a valley).
 */
NUMA *
numaFindExtrema(NUMA      *nas,
                l_float32  delta)
{
l_int32    i, n, found, loc, direction;
l_float32  startval, val, maxval, minval;
NUMA      *nad;

    PROCNAME("numaFindExtrema");

    if (!nas)
        return (NUMA *)ERROR_PTR("nas not defined", procName, NULL);

    n = numaGetCount(nas);
    nad = numaCreate(0);

        /* We don't know if we'll find a peak or valley first,
         * but use the first element of nas as the reference point.
         * Break when we deviate by 'delta' from the first point. */
    numaGetFValue(nas, 0, &startval);
    found = FALSE;
    for (i = 1; i < n; i++) {
        numaGetFValue(nas, i, &val);
        if (L_ABS(val - startval) >= delta) {
            found = TRUE;
            break;
        }
    }

    if (!found)
        return nad;  /* it's empty */

        /* Are we looking for a peak or a valley? */
    if (val > startval) {  /* peak */
        direction = 1;
        maxval = val;
    }
    else {
        direction = -1;
        minval = val;
    }
    loc = i;

        /* Sweep through the rest of the array, recording alternating
         * peak/valley extrema. */
    for (i = i + 1; i < n; i++) {
        numaGetFValue(nas, i, &val);
        if (direction == 1 && val > maxval ) {  /* new local max */
            maxval = val;
            loc = i;
        }
        else if (direction == -1 && val < minval ) {  /* new local min */
            minval = val;
            loc = i;
        }
        else if (direction == 1 && (maxval - val >= delta)) {
            numaAddNumber(nad, loc);  /* save the current max location */
            direction = -1;  /* reverse: start looking for a min */
            minval = val;
            loc = i;  /* current min location */
        }
        else if (direction == -1 && (val - minval >= delta)) {
            numaAddNumber(nad, loc);  /* save the current min location */
            direction = 1;  /* reverse: start looking for a max */
            maxval = val;
            loc = i;  /* current max location */
        }
    }

        /* Save the final extremum */
/*    numaAddNumber(nad, loc); */
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


