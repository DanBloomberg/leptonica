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
 * expand_reg.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  BINARY_IMAGE             "test1.png"
#define  TWO_BPP_IMAGE_NO_CMAP    "weasel2.4g.png"
#define  TWO_BPP_IMAGE_CMAP       "weasel2.4c.png"
#define  FOUR_BPP_IMAGE_NO_CMAP   "weasel4.16g.png"
#define  FOUR_BPP_IMAGE_CMAP      "weasel4.16c.png"
#define  EIGHT_BPP_IMAGE_NO_CMAP  "weasel8.149g.png"
#define  EIGHT_BPP_IMAGE_CMAP     "weasel8.240c.png"
#define  RGB_IMAGE                "marge.jpg"


main(int    argc,
     char **argv)
{
BOX         *box;
PIX         *pix, *pixs, *pixt;
char        *filename[8] = {BINARY_IMAGE,
                            TWO_BPP_IMAGE_NO_CMAP, TWO_BPP_IMAGE_CMAP,
                            FOUR_BPP_IMAGE_NO_CMAP, FOUR_BPP_IMAGE_CMAP,
                            EIGHT_BPP_IMAGE_NO_CMAP, EIGHT_BPP_IMAGE_CMAP,
                            RGB_IMAGE};
l_int32      i, w, h;
static char  mainName[] = "expand_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  expand_reg", mainName, 1));

    for (i = 0; i < 8; i++) {
        pixs = pixRead(filename[i]);
        pixt = pixExpandReplicate(pixs, 2);
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        pixt = pixExpandReplicate(pixs, 3);
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);

        if (i == 4) {
            pixt = pixScale(pixs, 3.0, 3.0);
            pixWrite("junkpixt", pixt, IFF_PNG);
            pixDestroy(&pixt);
        }
        pixDestroy(&pixs);
    }
              
    pix = pixRead("test1.png");
    pixGetDimensions(pix, &w, &h, NULL);
    for (i = 1; i <= 15; i++) {
        box = boxCreate(13 * i, 13 * i, w - 13 * i, h - 13 * i);
        pixs = pixClipRectangle(pix, box, NULL);
        pixt = pixExpandReplicate(pixs, 3);
        pixDisplayWrite(pixt, 1);
        boxDestroy(&box);
        pixDestroy(&pixt);
        pixDestroy(&pixs);
    }
    pixDestroy(&pix);

    system("/usr/bin/gthumb junk_write_display* &");
    return 0;
}

