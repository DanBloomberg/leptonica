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
 * snapcolortest.c
 *
 *    This tests the color snapping in blend.c.
 *    It is used here to color the background on images in index.html.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_uint32   LEPTONICA_YELLOW = 0xffffe400;

main(int    argc,
     char **argv)
{
PIX         *pixs, *pixd;
static char  mainName[] = "snapcolortest";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  snapcolortest", mainName, 1));

    if ((pixs = pixRead("Leptonica.jpg")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));

        /* First, snap the color directly on the input rgb image. */
    pixDisplay(pixs, 100, 75);
    pixd = pixSnapColor(NULL, pixs, 0xffffff00, LEPTONICA_YELLOW, 30);
    pixDisplay(pixd, 100, 220);
    pixWrite("junklogo1", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);

        /* Then make a colormapped version and snap the color */
    pixd = pixOctreeQuantNumColors(pixs, 250, 0);
    pixDisplay(pixd, 500, 75);
    pixSnapColor(pixd, pixd, 0xffffff00, LEPTONICA_YELLOW, 30);
    pixDisplay(pixd, 500, 220);
    pixWrite("junklogo2", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixDestroy(&pixs);


        /* Set the background of the google searchbox to yellow.
	 * The input image is colormapped with all 256 colors used. */
    if ((pixs = pixRead("google-searchbox.png")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));

    pixDisplay(pixs, 900, 75);
    pixd = pixSnapColor(NULL, pixs, 0xffffff00, LEPTONICA_YELLOW, 30);
    pixDisplay(pixd, 900, 220);
    pixWrite("junklogo3", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixDestroy(&pixs);

    exit(0);
}

