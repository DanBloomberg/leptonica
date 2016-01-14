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
 *  edge.c
 *
 *      Sobel edge detecting filter
 *          PIX      *pixSobelEdgeFilter()
 *
 *  The Sobel edge detector uses these two simple gradient filters.
 *
 *       1    2    1             1    0   -1 
 *       0    0    0             2    0   -2
 *      -1   -2   -1             1    0   -1
 *
 *      (horizontal)             (vertical)
 *
 *  To use both the vertical and horizontal filters, set the orientation
 *  flag to L_ALL_EDGES; this sums the abs. value of their outputs,
 *  clipped to 255.
 *
 *  See comments below for displaying the resulting image with
 *  the edges dark, both for 8 bpp and 1 bpp.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


/*----------------------------------------------------------------------*
 *                    Sobel edge detecting filter                       *
 *----------------------------------------------------------------------*/
/*!
 *  pixSobelEdgeFilter()
 *
 *      Input:  pixs (8 bpp; no colormap)
 *              orientflag (L_HORIZONTAL_EDGES, L_VERTICAL_EDGES, L_ALL_EDGES)
 *      Return: pixd (8 bpp, edges are brighter), or null on error
 *
 *  Notes:
 *      (1) Invert pixd to see larger gradients as darker (grayscale).
 *      (2) To generate a binary image of the edges, threshold
 *          the result using pixThresholdToBinary().  If the high
 *          edge values are to be fg (1), invert after running
 *          pixThresholdToBinary().
 *      (3) Label the pixels as follows:
 *              1    4    7
 *              2    5    8
 *              3    6    9
 *          Read the data incrementally across the image and unroll
 *          the loop.
 *      (4) This runs at about 45 Mpix/sec on a 3 GHz processor.
 */
PIX  *
pixSobelEdgeFilter(PIX     *pixs,
                   l_int32  orientflag)
{
l_int32    w, h, d, i, j, wplt, wpld, gx, gy, vald;
l_int32    val1, val2, val3, val4, val5, val6, val7, val8, val9;
l_uint32  *datat, *linet, *datad, *lined;
PIX       *pixt, *pixd;

    PROCNAME("pixSobelEdgeFilter");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    pixGetDimensions(pixs, &w, &h, &d);
    if (d != 8)
        return (PIX *)ERROR_PTR("pixs not 8 bpp", procName, NULL);
    if (orientflag != L_HORIZONTAL_EDGES && orientflag != L_VERTICAL_EDGES &&
        orientflag != L_ALL_EDGES)
        return (PIX *)ERROR_PTR("invalid orientflag", procName, NULL);

        /* Add 1 pixel (mirrored) to each side of the image. */
    if ((pixt = pixAddMirroredBorder(pixs, 1, 1, 1, 1)) == NULL)
        return (PIX *)ERROR_PTR("pixt not made", procName, NULL);

        /* Compute filter output at each location. */
    pixd = pixCreateTemplate(pixs);
    datat = pixGetData(pixt);
    wplt = pixGetWpl(pixt);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        linet = datat + i * wplt;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            if (j == 0) {  /* start a new row */
                val1 = GET_DATA_BYTE(linet, j);
                val2 = GET_DATA_BYTE(linet + wplt, j);
                val3 = GET_DATA_BYTE(linet + 2 * wplt, j);
                val4 = GET_DATA_BYTE(linet, j + 1);
                val5 = GET_DATA_BYTE(linet + wplt, j + 1);
                val6 = GET_DATA_BYTE(linet + 2 * wplt, j + 1);
                val7 = GET_DATA_BYTE(linet, j + 2);
                val8 = GET_DATA_BYTE(linet + wplt, j + 2);
                val9 = GET_DATA_BYTE(linet + 2 * wplt, j + 2);
            } else {  /* shift right by 1 pixel; update incrementally */
                val1 = val4;
                val2 = val5;
                val3 = val6;
                val4 = val7;
                val5 = val8;
                val6 = val9;
                val7 = GET_DATA_BYTE(linet, j + 2);
                val8 = GET_DATA_BYTE(linet + wplt, j + 2);
                val9 = GET_DATA_BYTE(linet + 2 * wplt, j + 2);
            }
            if (orientflag == L_HORIZONTAL_EDGES)
                vald = L_ABS(val1 + 2 * val4 + val7
                             - val3 - 2 * val6 - val9) >> 3;
            else if (orientflag == L_VERTICAL_EDGES)
                vald = L_ABS(val1 + 2 * val2 + val3 - val7
                             - 2 * val8 - val9) >> 3;
            else {  /* L_ALL_EDGES */
                gx = L_ABS(val1 + 2 * val2 + val3 - val7
                           - 2 * val8 - val9) >> 3;
                gy = L_ABS(val1 + 2 * val4 + val7
                             - val3 - 2 * val6 - val9) >> 3;
                vald = L_MIN(255, gx + gy);
            }
            SET_DATA_BYTE(lined, j, vald);
        }
    }

    pixDestroy(&pixt);
    return pixd;
}

