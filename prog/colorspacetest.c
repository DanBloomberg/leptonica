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
	    
        /* Colorspace conversion in rgb */
    pixDisplayWrite(pixs, 1);
    pixt = pixConvertRGBToHSV(NULL, pixs);
    pixDisplayWrite(pixt, 1);
    pixConvertHSVToRGB(pixt, pixt);
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);

        /* Colorspace conversion on a colormap */
    pixt = pixOctreeQuant(pixs, 25, 0);
    pixDisplayWrite(pixt, 1);
    cmap = pixGetColormap(pixt);
    pixcmapWriteStream(stderr, cmap);
    pixcmapConvertRGBToHSV(cmap);
    pixcmapWriteStream(stderr, cmap);
    pixDisplayWrite(pixt, 1);
    pixcmapConvertHSVToRGB(cmap);
    pixcmapWriteStream(stderr, cmap);
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);

        /* Color content extraction */
    pixColorContent(pixs, &pixr, &pixg, &pixb);
    pixDisplayWrite(pixr, 1);
    pixDisplayWrite(pixg, 1);
    pixDisplayWrite(pixb, 1);
    pixDestroy(&pixr);
    pixDestroy(&pixg);
    pixDestroy(&pixb);

    system("gthumb junk_write_display* &");

    pixDestroy(&pixs);
    return 0;
}

