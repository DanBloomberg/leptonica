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
 * rank_reg.c
 *
 *   Tests grayscale MinMax and rank
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32       i, j;
PIX          *pixs, *pixt0, *pixt1;
static char   mainName[] = "rank_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax: rank_reg", mainName, 1));

    pixs = pixRead("test8.jpg");
    for (i = 1; i <= 4; i++) {
        pixt1 = pixScaleGrayRank2(pixs, i);
        pixDisplay(pixt1, 300 * (i - 1), 100);
        pixDestroy(&pixt1);
    }
    pixDestroy(&pixs);

    pixs = pixRead("test24.jpg");
    pixt1 = pixConvertRGBToLuminance(pixs);
    pixt0 = pixScale(pixt1, 1.5, 1.5);
    pixDestroy(&pixt1);
    for (i = 1; i <= 4; i++) {
        for (j = 1; j <= 4; j++) {
            pixt1 = pixScaleGrayRankCascade(pixt0, i, j, 0, 0);
            pixDisplayWrite(pixt1, 1);
	    pixDestroy(&pixt1);
	}
    }
    pixDestroy(&pixt0);
    pixDestroy(&pixs);

    system("gthumb junk_write_display* &");

    return 0;
}

