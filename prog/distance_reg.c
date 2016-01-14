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
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

    /* Test with all 8 combinations of these parameters */
#define   CONNECTIVITY   8   /* 4 or 8 */
#define   DEPTH          8  /* 8 or 16 bpp */
#define   BC             L_BOUNDARY_FG  /* L_BOUNDARY_FG or L_BOUNDARY_BG */

main(int    argc,
     char **argv)
{
BOX         *box;
PIX         *pix, *pixs, *pixd, *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
static char  mainName[] = "distance_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  distance_reg", mainName, 1));

    if ((pix = pixRead("feyn.tif")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    box = boxCreate(383, 338, 1480, 1050);
    pixs = pixClipRectangle(pix, box, NULL);
    pixDisplayWrite(pixt1, -1);  /* init */
    pixDisplayWrite(pixs, 1);
	    
        /* Test the distance function and display */
    pixInvert(pixs, pixs);
    pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH, BC);
    pixInvert(pixs, pixs);
    pixDisplayWrite(pixt1, 1);
    pixWrite("junkout1.png", pixt1, IFF_PNG);
    pixt2 = pixMaxDynamicRange(pixt1, L_LOG_SCALE);
    pixDisplayWrite(pixt2, 1);
    pixWrite("junkout2.png", pixt2, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

	/* Test the distance function and display with contour rendering */
    pixInvert(pixs, pixs);
    pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH, BC);
    pixInvert(pixs, pixs);
    pixt2 = pixRenderContours(pixt1, 2, 4, 1);  /* binary output */
    pixWrite("junkout3.png", pixt2, IFF_PNG);
    pixDisplayWrite(pixt2, 1);
    pixt3 = pixRenderContours(pixt1, 2, 4, DEPTH);
    pixt4 = pixMaxDynamicRange(pixt3, L_LINEAR_SCALE);
    pixWrite("junkout4.png", pixt4, IFF_PNG);
    pixDisplayWrite(pixt4, 1);
    pixt5 = pixMaxDynamicRange(pixt3, L_LOG_SCALE);
    pixWrite("junkout5.png", pixt5, IFF_PNG);
    pixDisplayWrite(pixt5, 1);
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
    if (DEPTH == 8) {
        pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH, BC);
        pixt4 = pixMaxDynamicRange(pixt1, L_LOG_SCALE);
        pixWrite("junkout6.png", pixt4, IFF_PNG);
        pixDisplayWrite(pixt4, 1);
        pixt2 = pixCreateTemplate(pixt1);
        pixSetMasked(pixt2, pixs, 255);
        pixWrite("junkout7.png", pixt2, IFF_PNG);
        pixDisplayWrite(pixt2, 1);
        pixSeedfillGray(pixt1, pixt2, 4);
        pixt3 = pixMaxDynamicRange(pixt1, L_LINEAR_SCALE);
        pixWrite("junkout8.png", pixt3, IFF_PNG);
        pixDisplayWrite(pixt3, 1);
        pixDestroy(&pixt1);
        pixDestroy(&pixt2);
        pixDestroy(&pixt3);
        pixDestroy(&pixt4);
    }

    system("/usr/bin/gthumb junk_write_display* &");

    boxDestroy(&box);
    pixDestroy(&pix);
    pixDestroy(&pixs);
    exit(0);
}

