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
 * cmapquanttest.c
 *
 *   Tests quantization of rgb image to a specific colormap.
 *   Does this by starting with a grayscale image, doing a grayscale
 *   quantization with a colormap in the dest, then adding new
 *   colors, scaling (which removes the colormap), and finally
 *   re-quantizing back to the original colormap.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  LEVEL    3

main(int    argc,
     char **argv)
{
l_uint32    *rtab, *gtab, *btab;
l_int32     *cmaptab;
BOX         *box;
PIX         *pixs, *pixt1, *pixt2, *pixt3, *pixt4;
PIXCMAP     *cmap;
static char  mainName[] = "cmapquanttest";

    pixs = pixRead("lucasta.jpg");

        /* Convert to 4 bpp with 6 levels and a colormap */
    pixt1 = pixThresholdTo4bpp(pixs, 6, 1);

        /* Color some non-white pixels, preserving antialiasing, and
         * adding these colors to the colormap */
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pixt1, box, L_PAINT_DARK, 220, 0, 0, 255);
    pixWrite("junkpixt1", pixt1, IFF_PNG);
    boxDestroy(&box);

        /* Scale up by 1.5; losing the colormap */
    startTimer();
    pixt2 = pixScale(pixt1, 1.5, 1.5);
    fprintf(stderr, "Time to scale by 1.5x = %7.3f sec\n", stopTimer());
    pixWrite("junkpixt2", pixt2, IFF_PNG);

        /* Re-quantize using the same colormap */
    startTimer();
    cmap = pixGetColormap(pixt1);
    pixt3 = pixOctcubeQuantFromCmap(pixt2, cmap, LEVEL, L_EUCLIDEAN_DISTANCE);
    fprintf(stderr, "Time to requantize to cmap = %7.3f sec\n", stopTimer());
    pixWrite("junkpixt3", pixt3, IFF_PNG);

        /* Re-quantize first making the tables and then 
         * using the lower-level function */
    startTimer();
    makeRGBToIndexTables(&rtab, &gtab, &btab, LEVEL);
    cmaptab = pixcmapToOctcubeLUT(cmap, LEVEL, L_EUCLIDEAN_DISTANCE);
    fprintf(stderr, "Time to make tables = %7.3f sec\n", stopTimer());
    startTimer();
    pixt4 = pixOctcubeQuantFromCmapLUT(pixt2, cmap, cmaptab, rtab, gtab, btab);
    fprintf(stderr, "Time for lowlevel re-quant = %7.3f sec\n", stopTimer());
    pixWrite("junkpixt4", pixt4, IFF_PNG);
    FREE(cmaptab);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);

    pixDestroy(&pixs);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
}


