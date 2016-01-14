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
 * fliptest.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein;
l_float32    v1, h1, v2, h2;
PIX         *pixs;
static char  mainName[] = "fliptest";

    if (argc != 2)
	exit(ERROR_INT(" Syntax: fliptest filein", mainName, 1));

    filein = argv[1];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
    startTimer();
    pixPageFlipDetect(pixs, &v1, &h1, 0, 0);
    fprintf(stderr, "Time for standard test: %7.3f sec\n", stopTimer());

    startTimer();
    pixPageFlipDetectDWA(pixs, &v2, &h2, 0, 0);
    fprintf(stderr, "Time for dwa test: %7.3f sec\n", stopTimer());

    if (v1 == v2 && h1 == h2) {
        fprintf(stderr, "Results identical\n");
        fprintf(stderr, "v = %7.3f, h = %7.3f\n", v1, h1);
    }
    else {
        fprintf(stderr, "Results differ\n");
        fprintf(stderr, "v1 = %7.3f, h1 = %7.3f\n", v1, h1);
        fprintf(stderr, "v2 = %7.3f, h2 = %7.3f\n", v2, h2);
    }
    pixDestroy(&pixs);
    exit(0);
}

