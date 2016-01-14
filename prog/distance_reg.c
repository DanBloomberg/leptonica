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
 * distance_reg.c
 *
 *   This tests pixDistanceFunction for a variety of usage
 *   with all 8 combinations of these parameters:
 *
 *     connectivity :   4 or 8
 *     dest depth :     8 or 16
 *     boundary cond :  L_BOUNDARY_BG or L_BOUNDARY_FG
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   DISPLAY        1


static void TestDistance(PIXA *pixa, PIX *pixs, l_int32 conn,
                         l_int32 depth, l_int32 bc);

main(int    argc,
     char **argv)
{
char         buf[256];
l_int32      i, j, k, index, conn, depth, bc;
BOX         *box;
PIX         *pix, *pixs, *pixd;
PIXA        *pixa;
static char  mainName[] = "distance_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  distance_reg", mainName, 1));

    if ((pix = pixRead("feyn.tif")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    box = boxCreate(383, 338, 1480, 1050);
    pixs = pixClipRectangle(pix, box, NULL);
    pixDisplayWrite(NULL, -1);
    pixDisplayWrite(pixs, DISPLAY);
	    
    for (i = 0; i < 2; i++) {
        conn = 4 + 4 * i;
        for (j = 0; j < 2; j++) {
            depth = 8 + 8 * j;
            for (k = 0; k < 2; k++) {
                bc = k + 1;
                index = 4 * i + 2 * j + k;
                fprintf(stderr, "Set %d\n", index);
                pixa = pixaCreate(0);
                pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
                TestDistance(pixa, pixs, conn, depth, bc);
                pixd = pixaDisplay(pixa, 0, 0);
                sprintf(buf, "junkdist.%d", index);
                pixWrite(buf, pixd, IFF_JFIF_JPEG);
                pixaDestroy(&pixa);
                pixDestroy(&pixd);
            }
        }
    }

    system("/usr/bin/gthumb junk_write_display* &");

    boxDestroy(&box);
    pixDestroy(&pix);
    pixDestroy(&pixs);
    return 0;
}


static void
TestDistance(PIXA    *pixa,
             PIX     *pixs,
             l_int32  conn,
             l_int32  depth,
             l_int32  bc)
{
PIX  *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;

        /* Test the distance function and display */
    pixInvert(pixs, pixs);
    pixt1 = pixDistanceFunction(pixs, conn, depth, bc);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 0);
    pixInvert(pixs, pixs);
    pixDisplayWrite(pixt1, DISPLAY);
    pixt2 = pixMaxDynamicRange(pixt1, L_LOG_SCALE);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 0);
    pixDisplayWrite(pixt2, DISPLAY);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

	/* Test the distance function and display with contour rendering */
    pixInvert(pixs, pixs);
    pixt1 = pixDistanceFunction(pixs, conn, depth, bc);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 0);
    pixInvert(pixs, pixs);
    pixt2 = pixRenderContours(pixt1, 2, 4, 1);  /* binary output */
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 0);
    pixDisplayWrite(pixt2, DISPLAY);
    pixt3 = pixRenderContours(pixt1, 2, 4, depth);
    pixt4 = pixMaxDynamicRange(pixt3, L_LINEAR_SCALE);
    pixSaveTiled(pixt4, pixa, 1, 0, 20, 0);
    pixDisplayWrite(pixt4, DISPLAY);
    pixt5 = pixMaxDynamicRange(pixt3, L_LOG_SCALE);
    pixSaveTiled(pixt5, pixa, 1, 0, 20, 0);
    pixDisplayWrite(pixt5, DISPLAY);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
    pixDestroy(&pixt5);

	/* Label all pixels in each c.c. with a color equal to the
         * max distance of any pixel within that c.c. from the bg.
         * Note that we've normalized so the dynamic range extends
         * to 255.  For the image here, each unit of distance is
         * represented by about 21 grayscale units.  The largest
         * distance is 12.  */
    if (depth == 8) {
        pixt1 = pixDistanceFunction(pixs, conn, depth, bc);
        pixt4 = pixMaxDynamicRange(pixt1, L_LOG_SCALE);
        pixSaveTiled(pixt4, pixa, 1, 1, 20, 0);
        pixDisplayWrite(pixt4, DISPLAY);
        pixt2 = pixCreateTemplate(pixt1);
        pixSetMasked(pixt2, pixs, 255);
        pixSaveTiled(pixt2, pixa, 1, 0, 20, 0);
        pixDisplayWrite(pixt2, DISPLAY);
        pixSeedfillGray(pixt1, pixt2, 4);
        pixt3 = pixMaxDynamicRange(pixt1, L_LINEAR_SCALE);
        pixSaveTiled(pixt3, pixa, 1, 0, 20, 0);
        pixDisplayWrite(pixt3, DISPLAY);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixDestroy(&pixt3);
        pixDestroy(&pixt4);
    }

    return;
}


