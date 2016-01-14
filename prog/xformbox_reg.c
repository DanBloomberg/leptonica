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
 *   xformbox_reg.c
 *
 *      Tests ordered box transforms (rotation, scaling, translation).
 *      Also tests the various box hashing graphics operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  SHIFTX = 50;
static const l_int32  SHIFTY = 70;
static const l_float32  SCALEX = 1.17;
static const l_float32  SCALEY = 1.13;
static const l_float32  ROTATION = 0.10;   /* radian */

l_int32 RenderTransformedBoxa(PIX *pixt, BOXA *boxa, l_int32 i);


main(int    argc,
     char **argv)
{
l_int32      i, n, rval, gval, bval, order;
BOX         *box, *boxt;
BOXA        *boxa, *boxat;
PIX         *pix, *pixs, *pixt, *pixt1, *pixt2, *pixt3;
PIXA        *pixa;
static char  mainName[] = "xformbox_reg";

        /* First, test hash rendering in 3 modes */
    pixs = pixRead("feyn.tif");
    box = boxCreate(461, 429, 1393, 342);
    pixt1 = pixClipRectangle(pixs, box, NULL);
    boxa = pixConnComp(pixt1, NULL, 8);
    n = boxaGetCount(boxa);
    pixt2 = pixConvertTo8(pixt1, 1);
    pixt3 = pixConvertTo32(pixt1);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxa, i, L_CLONE);
	rval = (1413 * i) % 256;
	gval = (4917 * i) % 256;
	bval = (7341 * i) % 256;
	pixRenderHashBox(pixt1, boxt, 8, 2, i % 4, 1, L_SET_PIXELS);
	pixRenderHashBoxArb(pixt2, boxt, 7, 2, i % 4, 1, rval, gval, bval);
	pixRenderHashBoxBlend(pixt3, boxt, 7, 2, i % 4, 1, rval, gval, bval,
                              0.5);
	boxDestroy(&boxt);
    }
    pixDisplay(pixt1, 0, 0);
    pixDisplay(pixt2, 0, 300);
    pixDisplay(pixt3, 0, 570);
    pixWrite("junkpixt1", pixt1, IFF_PNG);
    pixWrite("junkpixt2", pixt2, IFF_PNG);
    pixWrite("junkpixt3", pixt3, IFF_PNG);

    boxaDestroy(&boxa);
    boxDestroy(&box);
    pixDestroy(&pixs);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);


        /* Next, test box transforms with either translation or scaling
         * combined with rotation.  Demonstrate that the order of
         * the operations does not matter. */
    pix = pixRead("feyn.tif");
    box = boxCreate(420, 360, 1500, 465);
    pixt = pixClipRectangle(pix, box, NULL);
    pixs = pixAddBorderGeneral(pixt, 0, 200, 0, 0, 0);
    boxDestroy(&box);
    pixDestroy(&pix);
    pixDestroy(&pixt);
    boxa = pixConnComp(pixs, NULL, 8);
    n = boxaGetCount(boxa);
    pixa = pixaCreate(0);

    pixt = pixConvertTo32(pixs);
    for (i = 0; i < 3; i++) {
        if (i == 0)
            order = L_TR_SC_RO;
        else if (i == 1)
            order = L_TR_RO_SC;
        else
            order = L_SC_TR_RO;
        boxat = boxaTransformOrdered(boxa, SHIFTX, SHIFTY, 1.0, 1.0,
                                     450, 250, ROTATION, order);
        RenderTransformedBoxa(pixt, boxat, i);
        boxaDestroy(&boxat);
    }
    pixSaveTiled(pixt, pixa, 1, 1, 30, 32);
    pixDestroy(&pixt);

    pixt = pixConvertTo32(pixs);
    for (i = 0; i < 3; i++) {
        if (i == 0)
            order = L_RO_TR_SC;
        else if (i == 1)
            order = L_RO_SC_TR;
        else
            order = L_SC_RO_TR;
        boxat = boxaTransformOrdered(boxa, SHIFTX, SHIFTY, 1.0, 1.0,
                                     450, 250, ROTATION, order);
        RenderTransformedBoxa(pixt, boxat, i + 4);
        boxaDestroy(&boxat);
    }
    pixSaveTiled(pixt, pixa, 1, 1, 30, 0);
    pixDestroy(&pixt);

    pixt = pixConvertTo32(pixs);
    for (i = 0; i < 3; i++) {
        if (i == 0)
            order = L_TR_SC_RO;
        else if (i == 1)
            order = L_SC_RO_TR;
        else
            order = L_SC_TR_RO;
        boxat = boxaTransformOrdered(boxa, 0, 0, SCALEX, SCALEY,
                                     450, 250, ROTATION, order);
        RenderTransformedBoxa(pixt, boxat, i + 8);
        boxaDestroy(&boxat);
    }
    pixSaveTiled(pixt, pixa, 1, 1, 30, 0);
    pixDestroy(&pixt);

    pixt = pixConvertTo32(pixs);
    for (i = 0; i < 3; i++) {
        if (i == 0)
            order = L_RO_TR_SC;
        else if (i == 1)
            order = L_RO_SC_TR;
        else
            order = L_TR_RO_SC;
        boxat = boxaTransformOrdered(boxa, 0, 0, SCALEX, SCALEY,
                                     450, 250, ROTATION, order);
        RenderTransformedBoxa(pixt, boxat, i + 16);
        boxaDestroy(&boxat);
    }
    pixSaveTiled(pixt, pixa, 1, 1, 30, 0);
    pixDestroy(&pixt);

    pixt = pixaDisplay(pixa, 0, 0);
    pixWrite("junkpixt", pixt, IFF_PNG);
    pixDisplay(pixt, 1000, 0);
    pixDestroy(&pixt);
    pixDestroy(&pixs);
    boxaDestroy(&boxa);
    pixaDestroy(&pixa);

    return 0;
}


l_int32
RenderTransformedBoxa(PIX    *pixt,
                      BOXA   *boxa,
                      l_int32 i)
{
l_int32  j, n, rval, gval, bval;
BOX     *box;

    n = boxaGetCount(boxa);
    rval = (1413 * i) % 256;
    gval = (4917 * i) % 256;
    bval = (7341 * i) % 256;
    for (j = 0; j < n; j++) {
        box = boxaGetBox(boxa, j, L_CLONE);
        pixRenderHashBoxArb(pixt, box, 10, 3, i % 4, 1, rval, gval, bval);
        boxDestroy(&box);
    }
    return 0;
}



