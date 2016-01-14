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
 *  seedfilllow.c
 *
 *      Seedfill:
 *               void   seedfillBinaryLow()
 *               void   seedfillGrayLow()
 *
 *      Distance function:
 *               void   distanceFunctionLow()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "allheaders.h"



/*-----------------------------------------------------------------------*
 *                 Vincent's Iterative Binary Seedfill                   *
 *-----------------------------------------------------------------------*/
/*!
 *  seedfillBinaryLow()
 *
 *       Notes: (1) this is an in-place fill, where the seed image is
 *                  filled, clipping to the filling mask, in one full
 *                  cycle of UL -> LR and LR -> UL raster scans
 *              (2) we assume the mask is a filling mask, not
 *                  a blocking mask
 *              (3) we assume that the RHS pad bits of the mask
 *                  are properly set to 0
 *              (4) we clip to the smallest dimensions to avoid invalid reads
 */
void
seedfillBinaryLow(l_uint32  *datas,
		  l_int32    hs,
		  l_int32    wpls,
		  l_uint32  *datam,
		  l_int32    hm,
		  l_int32    wplm,
		  l_int32    connectivity)
{
l_int32    i, j, h, wpl;
l_uint32   word, mask;
l_uint32   wordabove, wordleft, wordbelow, wordright;
l_uint32   wordprev;  /* test against this in previous iteration */
l_uint32  *lines, *linem;

    PROCNAME("seedfillBinaryLow");

    h = L_MIN(hs, hm);
    wpl = L_MIN(wpls, wplm);

    switch (connectivity)
    {
    case 4:
	    /* UL --> LR scan */
	for (i = 0; i < h; i++) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < wpl; j++) {
		word = *(lines + j);
		mask = *(linem + j);

		    /* OR from word above and from word to left; mask */
		if (i > 0) {
		    wordabove = *(lines - wpls + j);
		    word |= wordabove;
		}
		if (j > 0) {
		    wordleft = *(lines + j - 1);
		    word |= wordleft << 31;
		}
		word &= mask;

		    /* no need to fill horizontally? */
		if (!word || !(~word)) {
		    *(lines + j) = word;
		    continue;
		}

		while (1) {
		    wordprev = word;
		    word = (word | (word >> 1) | (word << 1)) & mask;
		    if ((word ^ wordprev) == 0) {
			*(lines + j) = word;
			break;
		    }
		}
	    }
	}

	    /* LR --> UL scan */
	for (i = h - 1; i >= 0; i--) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = wpl - 1; j >= 0; j--) {
		word = *(lines + j);
		mask = *(linem + j);

		    /* OR from word below and from word to right; mask */
		if (i < h - 1) {
		    wordbelow = *(lines + wpls + j);
		    word |= wordbelow;
		}
		if (j < wpl - 1) {
		    wordright = *(lines + j + 1);
		    word |= wordright >> 31;
		}
		word &= mask;

		    /* no need to fill horizontally? */
		if (!word || !(~word)) {
		    *(lines + j) = word;
		    continue;
		}

		while (1) {
		    wordprev = word;
		    word = (word | (word >> 1) | (word << 1)) & mask;
		    if ((word ^ wordprev) == 0) {
			*(lines + j) = word;
			break;
		    }
		}
	    }
	}
	break;

    case 8:
	    /* UL --> LR scan */
	for (i = 0; i < h; i++) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < wpl; j++) {
		word = *(lines + j);
		mask = *(linem + j);

		    /* OR from words above and from word to left; mask */
		if (i > 0) {
		    wordabove = *(lines - wpls + j);
		    word |= (wordabove | (wordabove << 1) | (wordabove >> 1));
		    if (j > 0)
			word |= (*(lines - wpls + j - 1)) << 31;
		    if (j < wpl - 1)
			word |= (*(lines - wpls + j + 1)) >> 31;
		}
		if (j > 0) {
		    wordleft = *(lines + j - 1);
		    word |= wordleft << 31;
		}
		word &= mask;

		    /* no need to fill horizontally? */
		if (!word || !(~word)) {
		    *(lines + j) = word;
		    continue;
		}

		while (1) {
		    wordprev = word;
		    word = (word | (word >> 1) | (word << 1)) & mask;
		    if ((word ^ wordprev) == 0) {
			*(lines + j) = word;
			break;
		    }
		}
	    }
	}

	    /* LR --> UL scan */
	for (i = h - 1; i >= 0; i--) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = wpl - 1; j >= 0; j--) {
		word = *(lines + j);
		mask = *(linem + j);

		    /* OR from words below and from word to right; mask */
		if (i < h - 1) {
		    wordbelow = *(lines + wpls + j);
		    word |= (wordbelow | (wordbelow << 1) | (wordbelow >> 1));
		    if (j > 0)
			word |= (*(lines + wpls + j - 1)) << 31;
		    if (j < wpl - 1)
			word |= (*(lines + wpls + j + 1)) >> 31;
		}
		if (j < wpl - 1) {
		    wordright = *(lines + j + 1);
		    word |= wordright >> 31;
		}
		word &= mask;

		    /* no need to fill horizontally? */
		if (!word || !(~word)) {
		    *(lines + j) = word;
		    continue;
		}

		while (1) {
		    wordprev = word;
		    word = (word | (word >> 1) | (word << 1)) & mask;
		    if ((word ^ wordprev) == 0) {
			*(lines + j) = word;
			break;
		    }
		}
	    }
	}
	break;

    default:
	ERROR_VOID("connectivity must be 4 or 8", procName);
    }

    return;
}



/*-----------------------------------------------------------------------*
 *                 Vincent's Iterative Grayscale Seedfill                *
 *-----------------------------------------------------------------------*/
/*!
 *  seedfillGrayLow()
 *
 *          1  2  3
 *          4  x  5
 *          6  7  8
 */
void
seedfillGrayLow(l_uint32  *datas,
		l_int32    w,
		l_int32    h,
		l_int32    wpls,
		l_uint32  *datam,
		l_int32    wplm,
		l_int32    connectivity)
{
l_uint8    val2, val3, val4, val5, val7, val8;
l_uint8    val, maxval, maskval;
l_int32    i, j, imax, jmax;
l_uint32  *lines, *linem;

    PROCNAME("seedfillGrayLow");

    imax = h - 1;
    jmax = w - 1;

    switch (connectivity)
    {
    case 4:
	    /* UL --> LR scan */
	for (i = 0; i < h; i++) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		if ((maskval = GET_DATA_BYTE(linem, j)) > 0) {
		    maxval = 0;
		    if (i > 0)
			maxval = GET_DATA_BYTE(lines - wpls, j);
		    if (j > 0) {
			val4 = GET_DATA_BYTE(lines, j - 1);
			maxval = L_MAX(maxval, val4);
		    }
		    val = GET_DATA_BYTE(lines, j);
		    maxval = L_MAX(maxval, val);
		    val = L_MIN(maxval, maskval);
		    SET_DATA_BYTE(lines, j, val);
		}
	    }
	}

	    /* LR --> UL scan */
	for (i = imax; i >= 0; i--) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = jmax; j >= 0; j--) {
		if ((maskval = GET_DATA_BYTE(linem, j)) > 0) {
		    maxval = 0;
		    if (i < imax)
			maxval = GET_DATA_BYTE(lines + wpls, j);
		    if (j < jmax) {
			val5 = GET_DATA_BYTE(lines, j + 1);
			maxval = L_MAX(maxval, val5);
		    }
		    val = GET_DATA_BYTE(lines, j);
		    maxval = L_MAX(maxval, val);
		    val = L_MIN(maxval, maskval);
		    SET_DATA_BYTE(lines, j, val);
		}
	    }
	}
	break;

    case 8:
	    /* UL --> LR scan */
	for (i = 0; i < h; i++) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = 0; j < w; j++) {
		if ((maskval = GET_DATA_BYTE(linem, j)) > 0) {
		    maxval = 0;
		    if (i > 0) {
			if (j > 0)
			    maxval = GET_DATA_BYTE(lines - wpls, j - 1);
			if (j < jmax) {
			    val2 = GET_DATA_BYTE(lines - wpls, j + 1);
			    maxval = L_MAX(maxval, val2);
			}
			val3 = GET_DATA_BYTE(lines - wpls, j);
			maxval = L_MAX(maxval, val3);
		    }
		    if (j > 0) {
			val4 = GET_DATA_BYTE(lines, j - 1);
			maxval = L_MAX(maxval, val4);
		    }
		    val = GET_DATA_BYTE(lines, j);
		    maxval = L_MAX(maxval, val);
		    val = L_MIN(maxval, maskval);
		    SET_DATA_BYTE(lines, j, val);
		}
	    }
	}

	    /* LR --> UL scan */
	for (i = imax; i >= 0; i--) {
	    lines = datas + i * wpls;
	    linem = datam + i * wplm;
	    for (j = jmax; j >= 0; j--) {
		if ((maskval = GET_DATA_BYTE(linem, j)) > 0) {
		    maxval = 0;
		    if (i < imax) {
			if (j > 0)
			    maxval = GET_DATA_BYTE(lines + wpls, j - 1);
			if (j < jmax) {
			    val8 = GET_DATA_BYTE(lines + wpls, j + 1);
			    maxval = L_MAX(maxval, val8);
			}
			val7 = GET_DATA_BYTE(lines + wpls, j);
			maxval = L_MAX(maxval, val7);
		    }
		    if (j < jmax) {
			val5 = GET_DATA_BYTE(lines, j + 1);
			maxval = L_MAX(maxval, val5);
		    }
		    val = GET_DATA_BYTE(lines, j);
		    maxval = L_MAX(maxval, val);
		    val = L_MIN(maxval, maskval);
		    SET_DATA_BYTE(lines, j, val);
		}
	    }
	}
	break;

    default:
	ERROR_VOID("connectivity must be 4 or 8", procName);
    }

    return;
}



/*-----------------------------------------------------------------------*
 *                   Vincent's Distance Function method                  *
 *-----------------------------------------------------------------------*/
/*!
 *  distanceFunctionLow()
 */
void
distanceFunctionLow(l_uint32  *datad,
		    l_int32    w,
		    l_int32    h,
		    l_int32    d,
		    l_int32    wpld,
		    l_int32    connectivity)
{
l_uint16   val1, val2, val3, val4, val5, val6, val7, val8, minval, val;
l_int32    i, j, imax, jmax;
l_uint32  *lined;

    PROCNAME("distanceFunctionLow");

	/* further initialize the distance image, setting
	 * a single-pixel-width perimeter of 0 pixels */
    imax = h - 1;
    jmax = w - 1;
    for (j = 0; j < wpld; j++) {
	*(datad + j) = 0;
	*(datad + imax * wpld + j) = 0;
    }
    for (i = 1; i < imax; i++) {
	lined = datad + i * wpld;
	if (d == 8) {
	    SET_DATA_BYTE(lined, 0, 0);
	    SET_DATA_BYTE(lined, jmax, 0);
	}
	else {
	    SET_DATA_TWO_BYTES(lined, 0, 0);
	    SET_DATA_TWO_BYTES(lined, jmax, 0);
	}
    }

	/* one raster scan followed by one anti-raster scan */
    switch (connectivity)
    {
    case 4:
        if (d == 8) {
		/* UL --> LR scan */
	    for (i = 1; i < imax; i++) {
		lined = datad + i * wpld;
		for (j = 1; j < jmax; j++) {
		    if ((val = GET_DATA_BYTE(lined, j)) > 0) {
			val2 = GET_DATA_BYTE(lined - wpld, j);
			val4 = GET_DATA_BYTE(lined, j - 1);
			minval = L_MIN(val2, val4);
			minval = L_MIN(minval, 254);
			SET_DATA_BYTE(lined, j, minval + 1);
    /*	            fprintf(stderr, "Val = %d\n", GET_DATA_BYTE(lined, j)); */
		    }
		}
	    }

		/* LR --> UL scan */
	    for (i = imax - 1; i > 0; i--) {
		lined = datad + i * wpld;
		for (j = jmax - 1; j > 0; j--) {
		    if ((val = GET_DATA_BYTE(lined, j)) > 0) {
			val7 = GET_DATA_BYTE(lined + wpld, j);
			val5 = GET_DATA_BYTE(lined, j + 1);
			minval = L_MIN(val5, val7);
			minval = L_MIN(minval, val);
			minval = L_MIN(minval, 254);
			SET_DATA_BYTE(lined, j, minval + 1);
		    }
		}
	    }
        }
	else {  /* d == 16 */
		/* UL --> LR scan */
	    for (i = 1; i < imax; i++) {
		lined = datad + i * wpld;
		for (j = 1; j < jmax; j++) {
		    if ((val = GET_DATA_TWO_BYTES(lined, j)) > 0) {
			val2 = GET_DATA_TWO_BYTES(lined - wpld, j);
			val4 = GET_DATA_TWO_BYTES(lined, j - 1);
			minval = L_MIN(val2, val4);
			SET_DATA_TWO_BYTES(lined, j, minval + 1);
		    }
		}
	    }

		/* LR --> UL scan */
	    for (i = imax - 1; i > 0; i--) {
		lined = datad + i * wpld;
		for (j = jmax - 1; j > 0; j--) {
		    if ((val = GET_DATA_TWO_BYTES(lined, j)) > 0) {
			val7 = GET_DATA_TWO_BYTES(lined + wpld, j);
			val5 = GET_DATA_TWO_BYTES(lined, j + 1);
			minval = L_MIN(val5, val7);
			minval = L_MIN(minval, val);
			SET_DATA_TWO_BYTES(lined, j, minval + 1);
		    }
		}
	    }
	}
	break;

    case 8:
        if (d == 8) {
		/* UL --> LR scan */
	    for (i = 1; i < imax; i++) {
		lined = datad + i * wpld;
		for (j = 1; j < jmax; j++) {
		    if ((val = GET_DATA_BYTE(lined, j)) > 0) {
			val1 = GET_DATA_BYTE(lined - wpld, j - 1);
			val2 = GET_DATA_BYTE(lined - wpld, j);
			val3 = GET_DATA_BYTE(lined - wpld, j + 1);
			val4 = GET_DATA_BYTE(lined, j - 1);
			minval = L_MIN(val1, val2);
			minval = L_MIN(minval, val3);
			minval = L_MIN(minval, val4);
			minval = L_MIN(minval, 254);
			SET_DATA_BYTE(lined, j, minval + 1);
		    }
		}
	    }

		/* LR --> UL scan */
	    for (i = imax - 1; i > 0; i--) {
		lined = datad + i * wpld;
		for (j = jmax - 1; j > 0; j--) {
		    if ((val = GET_DATA_BYTE(lined, j)) > 0) {
			val8 = GET_DATA_BYTE(lined + wpld, j + 1);
			val7 = GET_DATA_BYTE(lined + wpld, j);
			val6 = GET_DATA_BYTE(lined + wpld, j - 1);
			val5 = GET_DATA_BYTE(lined, j + 1);
			minval = L_MIN(val8, val7);
			minval = L_MIN(minval, val6);
			minval = L_MIN(minval, val5);
			minval = L_MIN(minval, val);
			minval = L_MIN(minval, 254);
			SET_DATA_BYTE(lined, j, minval + 1);
		    }
		}
	    }
        }
	else {  /* d == 16 */
		/* UL --> LR scan */
	    for (i = 1; i < imax; i++) {
		lined = datad + i * wpld;
		for (j = 1; j < jmax; j++) {
		    if ((val = GET_DATA_TWO_BYTES(lined, j)) > 0) {
			val1 = GET_DATA_TWO_BYTES(lined - wpld, j - 1);
			val2 = GET_DATA_TWO_BYTES(lined - wpld, j);
			val3 = GET_DATA_TWO_BYTES(lined - wpld, j + 1);
			val4 = GET_DATA_TWO_BYTES(lined, j - 1);
			minval = L_MIN(val1, val2);
			minval = L_MIN(minval, val3);
			minval = L_MIN(minval, val4);
			SET_DATA_TWO_BYTES(lined, j, minval + 1);
		    }
		}
	    }

		/* LR --> UL scan */
	    for (i = imax - 1; i > 0; i--) {
		lined = datad + i * wpld;
		for (j = jmax - 1; j > 0; j--) {
		    if ((val = GET_DATA_TWO_BYTES(lined, j)) > 0) {
			val8 = GET_DATA_TWO_BYTES(lined + wpld, j + 1);
			val7 = GET_DATA_TWO_BYTES(lined + wpld, j);
			val6 = GET_DATA_TWO_BYTES(lined + wpld, j - 1);
			val5 = GET_DATA_TWO_BYTES(lined, j + 1);
			minval = L_MIN(val8, val7);
			minval = L_MIN(minval, val6);
			minval = L_MIN(minval, val5);
			minval = L_MIN(minval, val);
			SET_DATA_TWO_BYTES(lined, j, minval + 1);
		    }
		}
	    }
        }
	break;

    default:
	ERROR_VOID("connectivity must be 4 or 8", procName);
	break;
    }

    return;
}

