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
 * colorsegtest.c
 *
 *   See colorseg.c for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32    MAX_DIST      = 75;
static const l_int32    MAX_COLORS    = 8;
static const l_int32    SEL_SIZE      = 5;
static const l_int32    FINAL_COLORS  = 4;


main(int    argc,
     char **argv)
{
PIX         *pixs, *pixd;
char        *filein, *fileout;
static char  mainName[] = "colorsegtest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  colorsegtest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
    startTimer();
    pixd = pixColorSegment(pixs, MAX_DIST, MAX_COLORS, SEL_SIZE, FINAL_COLORS);
    fprintf(stderr, "Time to segment: %7.3f sec\n", stopTimer());
    pixWrite(fileout, pixd, IFF_PNG);

    pixDestroy(&pixs);
    pixDestroy(&pixd);
    exit(0);
}

