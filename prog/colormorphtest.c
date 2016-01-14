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
 *  colormorphtest.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein, *fileout;
l_int32      size;
PIX         *pixs, *pixd;
static char  mainName[] = "colormorphtest";

    if (argc != 4)
        exit(ERROR_INT(" Syntax:  colormorphtest filein size fileout",
             mainName, 1));

    filein = argv[1];
    size = atoi(argv[2]);
    fileout = argv[3];

    if ((pixs = pixRead(filein)) == NULL)
        exit(ERROR_INT("pixs not read", mainName, 1));

    pixd = pixMorphColor(pixs, size, size, L_MORPH_DILATE);
    pixDisplayWithTitle(pixd, 100, 100, "Dilated");
    pixDestroy(&pixd);
    pixd = pixMorphColor(pixs, size, size, L_MORPH_ERODE);
    pixDisplayWithTitle(pixd, 300, 100, "Eroded");
    pixDestroy(&pixd);
    pixd = pixMorphColor(pixs, size, size, L_MORPH_OPEN);
    pixDisplayWithTitle(pixd, 500, 100, "Opened");
    pixDestroy(&pixd);
    pixd = pixMorphColor(pixs, size, size, L_MORPH_CLOSE);
    pixDisplayWithTitle(pixd, 700, 100, "Closed");

    pixWrite(fileout, pixd, IFF_PNG);
    pixDestroy(&pixs);
    pixDestroy(&pixd);
    exit(0);
}

