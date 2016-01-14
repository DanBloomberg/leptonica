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
 * colorspacetest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
PIX         *pixs, *pixt;
PIX         *pixr, *pixg, *pixb;  /* for color content extraction */
PIXCMAP     *cmap;
static char  mainName[] = "colorspacetest";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  colorspacetest filein", mainName, 1));

    if ((pixs = pixRead(argv[1])) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
        /* Space conversion in rgb */
    pixDisplay(pixs, 0, 75);
    pixWrite("/usr/tmp/junkrgb1", pixs, IFF_PNG);
    pixt = pixConvertRGBToHSV(NULL, pixs);
    pixDisplay(pixt, 600, 75);
    pixConvertHSVToRGB(pixt, pixt);
    pixDisplay(pixt, 1200, 75);
    pixWrite("/usr/tmp/junkrgb2", pixt, IFF_PNG);
    pixDestroy(&pixt);

        /* Space conversion on a colormap */
    pixt = pixOctreeQuant(pixs, 25, 0);
    pixWrite("/usr/tmp/junkcmap1", pixt, IFF_PNG);
    pixDisplay(pixt, 0, 575);
    cmap = pixGetColormap(pixt);
    pixcmapWriteStream(stderr, cmap);
    pixcmapConvertRGBToHSV(cmap);
    pixcmapWriteStream(stderr, cmap);
    pixDisplay(pixt, 600, 575);
    pixcmapConvertHSVToRGB(cmap);
    pixWrite("/usr/tmp/junkcmap2", pixt, IFF_PNG);
    pixcmapWriteStream(stderr, cmap);
    pixDisplay(pixt, 1200, 575);
    pixDestroy(&pixt);

        /* Color content extraction */
    pixColorContent(pixs, &pixr, &pixg, &pixb);
    pixWrite("/usr/tmp/junkpixr", pixr, IFF_JFIF_JPEG);
    pixWrite("/usr/tmp/junkpixg", pixg, IFF_JFIF_JPEG);
    pixWrite("/usr/tmp/junkpixb", pixb, IFF_JFIF_JPEG);
    pixDestroy(&pixr);
    pixDestroy(&pixg);
    pixDestroy(&pixb);

    pixDestroy(&pixs);
    exit(0);
}

