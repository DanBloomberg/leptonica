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
 * flipdetect_reg.c
 *
 *   Tests 90 degree orientation of text and whether the text is
 *   mirror reversed.  Compares the rasterop with dwa implementations
 *   for speed.  Shows the typical 'confidence' outputs from the
 *   functions in flipdetect.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein;
l_int32      i;
l_float32    upconf1, upconf2, leftconf1, leftconf2, conf1, conf2;
PIX         *pixs, *pixt1, *pixt2;
static char  mainName[] = "flipdetect_reg";

    if (argc != 2)
	exit(ERROR_INT(" Syntax: flipdetect_reg filein", mainName, 1));

    filein = argv[1];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
	    
    fprintf(stderr, "\nTest orientation detection\n");
    startTimer();
    pixOrientDetect(pixs, &upconf1, &leftconf1, 0, 0);
    fprintf(stderr, "Time for rop orient test: %7.3f sec\n", stopTimer());

    startTimer();
    pixOrientDetectDwa(pixs, &upconf2, &leftconf2, 0, 0);
    fprintf(stderr, "Time for dwa orient test: %7.3f sec\n", stopTimer());

    if (upconf1 == upconf2 && leftconf1 == leftconf2) {
        fprintf(stderr, "Orient results identical\n");
        fprintf(stderr, "upconf = %7.3f, leftconf = %7.3f\n",
                upconf1, leftconf1);
    }
    else {
        fprintf(stderr, "Orient results differ\n");
        fprintf(stderr, "upconf1 = %7.3f, upconf2 = %7.3f\n", upconf1, upconf2);
        fprintf(stderr, "leftconf1 = %7.3f, leftconf2 = %7.3f\n",
                leftconf1, leftconf2);
    }

    pixt1 = pixCopy(NULL, pixs);
    fprintf(stderr, "\nTest orient detection for 4 orientations\n");
    for (i = 0; i < 4; i++) {
        pixOrientDetectDwa(pixt1, &upconf2, &leftconf2, 0, 0);
        fprintf(stderr, "upconf2 = %7.3f, leftconf2 = %7.3f\n",
                upconf2, leftconf2);
        if (i == 3) break;
        pixt2 = pixRotate90(pixt1, 1);
        pixDestroy(&pixt1);
        pixt1 = pixt2;
    }
    pixDestroy(&pixt1);

    fprintf(stderr, "\nTest mirror reverse detection\n");
    startTimer();
    pixMirrorDetect(pixs, &conf1, 0, 0);
    fprintf(stderr, "Time for rop mirror flip test: %7.3f sec\n", stopTimer());

    startTimer();
    pixMirrorDetectDwa(pixs, &conf2, 0, 0);
    fprintf(stderr, "Time for dwa mirror flip test: %7.3f sec\n", stopTimer());

    if (conf1 == conf2) {
        fprintf(stderr, "Mirror results identical\n");
        fprintf(stderr, "conf = %7.3f\n", conf1);
    }
    else {
        fprintf(stderr, "Mirror results differ\n");
        fprintf(stderr, "conf1 = %7.3f, conf2 = %7.3f\n", conf1, conf2);
    }

#if 0
        /* Debug only */
    pixUpDownDetect(pixs, &conf1, 0);
#endif

    pixDestroy(&pixs);
    exit(0);
}

