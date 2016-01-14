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
 * distancetest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   CONNECTIVITY      8
#define   DEPTH             8  /* or 16 */
#define   MAP_TYPE          L_LOG_SCALE


main(int    argc,
     char **argv)
{
BOX         *box;
PIX         *pix, *pixs, *pixd, *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
static char  mainName[] = "distancetest";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  distancetest", mainName, 1));

    if ((pix = pixRead("feyn.tif")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    box = boxCreate(383, 338, 1480, 1050);
    pixs = pixClipRectangle(pix, box, NULL);
	    
        /* Test the distance function */
    pixInvert(pixs, pixs);
    pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixWrite("junkout1", pixt1, IFF_PNG);
    pixt2 = pixMaxDynamicRange(pixt1, MAP_TYPE);
    pixWrite("junkout2", pixt2, IFF_PNG);
    pixInvert(pixs, pixs);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

	/* Test the distance function with contour rendering */
    pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixt2 = pixRenderContours(pixt1, 5, 10, 1);  /* binary output */
    pixWrite("junkout3", pixt2, IFF_PNG);
    pixt3 = pixRenderContours(pixt1, 5, 10, DEPTH);
    pixt4 = pixMaxDynamicRange(pixt3, L_LINEAR_SCALE);
    pixWrite("junkout4", pixt4, IFF_PNG);
    pixt5 = pixMaxDynamicRange(pixt3, L_LOG_SCALE);
    pixWrite("junkout5", pixt5, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
    pixDestroy(&pixt5);

	/* label all pixels in each c.c. with the max distance */
    pixt1 = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixt2 = pixCreateTemplate(pixt1);
    pixSetMasked(pixt2, pixs, 255);
    pixWrite("junkout6", pixt2, IFF_PNG);
    pixSeedfillGray(pixt1, pixt2, 4);
    pixt3 = pixMaxDynamicRange(pixt1, MAP_TYPE);
    pixWrite("junkout7", pixt3, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);

	/* test the dynamic range map function */
    pixt1 = pixMaxDynamicRange(pixs, MAP_TYPE);
    pixWrite("junkout8", pixt1, IFF_PNG);
    pixDestroy(&pixt1);

    pixDestroy(&pix);
    pixDestroy(&pixs);
    exit(0);
}

