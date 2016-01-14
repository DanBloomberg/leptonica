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
 * scaletest3.c
 *
 *      This tests a number of scaling operations, through the pixScale()
 *      interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
PIX         *pixs, *pixd;
static char  mainName[] = "scaletest3";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  scaletest3", mainName, 1));

#if 1
        /* test 1 bpp */
    pixs = pixRead("feyn.tif");
    pixd = pixScale(pixs, 0.32, 0.32);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixd);

    pixd = pixScaleToGray3(pixs);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixd);

    pixd = pixScaleToGray4(pixs);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixd);

    pixd = pixScaleToGray6(pixs);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixd);

    pixd = pixScaleToGray8(pixs);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixd);

    pixd = pixScaleToGray16(pixs);
    pixDisplay(pixd, 0, 0);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 2 bpp without colormap */
    pixs = pixRead("weasel-4g.png");
    pixd = pixScale(pixs, 2.25, 2.25);
    pixDisplay(pixd, 600, 0);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 0);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 0);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 2 bpp with colormap */
    pixs = pixRead("weasel-4c.png");
    pixd = pixScale(pixs, 2.25, 2.25);
    pixDisplay(pixd, 600, 100);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 100);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 100);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 4 bpp without colormap */
    pixs = pixRead("weasel-16g.png");
    pixd = pixScale(pixs, 1.72, 1.72);
    pixDisplay(pixd, 600, 200);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 200);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 200);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 4 bpp with colormap */
    pixs = pixRead("weasel-16c.png");
    pixd = pixScale(pixs, 1.72, 1.72);
    pixDisplay(pixd, 600, 300);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 300);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 300);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 8 bpp without colormap */
    pixs = pixRead("weasel-149g.png");
    pixd = pixScale(pixs, 1.92, 1.92);
    pixDisplay(pixd, 600, 400);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 400);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 400);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 8 bpp with colormap */
    pixs = pixRead("weasel-240c.png");
    pixd = pixScale(pixs, 1.92, 1.92);
    pixDisplay(pixd, 600, 400);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 700, 400);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 750, 400);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

#if 1
        /* test 32 bpp */
    pixs = pixRead("marge.jpg");
    pixd = pixScale(pixs, 1.42, 1.42);
    pixDisplay(pixd, 0, 400);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.85, 0.85);
    pixDisplay(pixd, 0, 500);
    pixDestroy(&pixd);
    pixd = pixScale(pixs, 0.65, 0.65);
    pixDisplay(pixd, 0, 600);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
#endif

    exit(0);
}

