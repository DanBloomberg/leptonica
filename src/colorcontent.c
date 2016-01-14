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
 *  colorcontent.c
 *                     
 *      Determines color content spatially, as a measure of the
 *      amount of divergence of each color component (R,G,B) from gray.
 *
 *         PIX     *pixColorContent()   ***
 *
 *  *** Note: this function makes an implicit assumption about RGB
 *            component ordering.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


/*!
 *  pixColorContent()
 *
 *      Input:  pixs  (32 bpp rgb or 8 bpp colormapped)
 *              &pixr (<optional> 8 bpp red 'content')
 *              &pixg (<optional> 8 bpp green 'content')
 *              &pixb (<optional> 8 bpp blue 'content')
 *      Return: 0 if OK, 1 on error
 *
 *  For an RGB image, a gray pixel is one where all three components
 *  are equal.  When they are not equal, the pixel has some color.
 *  How should this be defined?
 *
 *  Consider the special case where R and G are equal.
 *  Then it makes sense to think of the color as coming from B,
 *  and, specifically, the difference between B and R (or G).
 *  So we want to use a majority voting algorithm to determine
 *  which component is pulling the pixel off of gray.
 *
 *  What happens when R, G and B are all different?  If R and G are
 *  numerically close, most of the color is still in the B component,
 *  and a little bit can be considered in the R and G components.
 *  So we generalize the special case as follows: the amount of R
 *  in a pixel is the min of the absolute value of the difference
 *  between R and G and R and B.  And likewise for the other two
 *  components.  This reduces to the special cases above, and
 *  preserves the flavor of majority voting.  In effect, we assign
 *  the most "responsibility" for color to the component that is
 *  most DIFFERENT from the other two.
 */
l_int32
pixColorContent(PIX   *pixs,
                PIX  **ppixr,
                PIX  **ppixg,
                PIX  **ppixb)
{
l_int32    w, h, i, j, wplc, wplr, wplg, wplb;
l_int32    rval, gval, bval, rgdiff, rbdiff, gbdiff, colorval;
l_uint32   pixel;
l_uint32  *datac, *datar, *datag, *datab, *linec, *liner, *lineg, *lineb;
PIX       *pixc;   /* rgb */
PIX       *pixr, *pixg, *pixb;   /* 8 bpp grayscale */
PIXCMAP   *cmap;

    PROCNAME("pixColorContent");

    if (!pixs)
	return ERROR_INT("pixs not defined", procName, 1);
    if (!ppixr && !ppixg && !ppixb)
	return ERROR_INT("nothing to compute", procName, 1);

    cmap = pixGetColormap(pixs);
    if (!cmap && pixGetDepth(pixs) != 32)
	return ERROR_INT("pixs not cmapped or 32 bpp", procName, 1);
    if (cmap)
        pixc = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    else
        pixc = pixClone(pixs);

    pixr = pixg = pixb = NULL;
    w = pixGetWidth(pixc);
    h = pixGetHeight(pixc);
    if (ppixr) {
        pixr = pixCreate(w, h, 8);
        datar = pixGetData(pixr);
        wplr = pixGetWpl(pixr);
	*ppixr = pixr;
    }
    if (ppixg) {
        pixg = pixCreate(w, h, 8);
        datag = pixGetData(pixg);
        wplg = pixGetWpl(pixg);
	*ppixg = pixg;
    }
    if (ppixb) {
        pixb = pixCreate(w, h, 8);
        datab = pixGetData(pixb);
        wplb = pixGetWpl(pixb);
	*ppixb = pixb;
    }

    datac = pixGetData(pixc);
    wplc = pixGetWpl(pixc);
    for (i = 0; i < h; i++) {
        linec = datac + i * wplc;
	if (pixr)
            liner = datar + i * wplr;
	if (pixg)
            lineg = datag + i * wplg;
	if (pixb)
            lineb = datab + i * wplb;
        for (j = 0; j < w; j++) {
            pixel = linec[j];
            rval = pixel >> 24;
	    gval = (pixel >> 16) & 0xff;
	    bval = (pixel >> 8) & 0xff;
	    rgdiff = L_ABS(rval - gval);
	    rbdiff = L_ABS(rval - bval);
	    gbdiff = L_ABS(gval - bval);
	    if (pixr) {
                colorval = L_MIN(rgdiff, rbdiff);
		SET_DATA_BYTE(liner, j, colorval);
	    }
	    if (pixg) {
                colorval = L_MIN(rgdiff, gbdiff);
		SET_DATA_BYTE(lineg, j, colorval);
	    }
	    if (pixb) {
                colorval = L_MIN(rbdiff, gbdiff);
		SET_DATA_BYTE(lineb, j, colorval);
	    }
	}
    }

    pixDestroy(&pixc);
    return 0;
}


