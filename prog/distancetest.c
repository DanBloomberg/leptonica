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
#define   DEPTH             16
#define   MAP_TYPE          L_LOG_SCALE


main(int    argc,
     char **argv)
{
char        *filein, *fileout;
PIX         *pixs, *pixd, *pixt, *pixt2, *pixt3, *pixm;
static char  mainName[] = "distancetest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  distancetest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
#if 0
        /* test the distance function */
    pixInvert(pixs, pixs);
    pixt = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixWrite("junkout1", pixt, IFF_PNG);
    pixd = pixMaxDynamicRange(pixt, MAP_TYPE);
    pixWrite(fileout, pixd, IFF_PNG);
#endif

#if 0
	/* test the distance function with contour rendering */
    pixInvert(pixs, pixs);
    pixt = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixt2 = pixRenderContours(pixt, 5, 10, 1);  /* binary output */
    pixWrite("junkout2", pixt2, IFF_PNG);
    pixt3 = pixRenderContours(pixt, 5, 10, DEPTH);
/*    pixd = pixMaxDynamicRange(pixt3, L_LINEAR_SCALE); */
    pixd = pixMaxDynamicRange(pixt3, L_LOG_SCALE);
    pixWrite(fileout, pixd, IFF_PNG);
#endif

#if 1
	/* label all pixels in each c.c. with the max distance */
    pixd = pixDistanceFunction(pixs, CONNECTIVITY, DEPTH);
    pixm = pixCreateTemplate(pixd);
    pixSetMasked(pixm, pixs, 255);
/*    pixWrite("junkout4", pixm, IFF_PNG); */
    pixSeedfillGray(pixd, pixm, 4);
    pixt = pixMaxDynamicRange(pixd, MAP_TYPE);
    pixWrite(fileout, pixt, IFF_PNG);
#endif

#if 0
	/* test the dynamic range map function */
    pixt = pixMaxDynamicRange(pixs, MAP_TYPE);
    pixWrite(fileout, pixt, IFF_PNG);
#endif

#if 0
    pixDestroy(&pixs);
    pixDestroy(&pixd);
    pixDestroy(&pixt);
#endif
    exit(0);
}

