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
 * adaptthresh.c
 *
 *    e.g., for w91frag.jpg, use (300, 300, 60, 1)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"



main(int    argc,
     char **argv)
{
PIX         *pixs, *pixd;
char        *filein, *fileout;
static char  mainName[] = "adaptthresh";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  adaptthresh filein fileout",
	       mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

    startTimer();
    pixd = pixAdaptThresholdToBinary(pixs, 300, 300, 50, 1);
    fprintf(stderr, "time for adapt thresh: %7.3f\n", stopTimer());

    pixWrite(fileout, pixd, IFF_PNG);

    pixDestroy(&pixs);
    pixDestroy(&pixd);
    exit(0);
}

