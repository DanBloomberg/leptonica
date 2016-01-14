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
 * ccthin2_reg.c
 *
 *   Tests the examples in pixThinExamples()
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32      index, maxiters, type;
BOX         *box;
PIX         *pix, *pixs, *pixd, *pixt;
PIXA        *pixa;
static char  mainName[] = "ccthin2_reg";

    if (argc != 1 && argc != 3)
	exit(ERROR_INT(" Syntax: ccthin2_reg [index maxiters]", mainName, 1));

    if ((pix = pixRead("feyn.tif")) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));
    box = boxCreate(683, 799, 970, 479);
    pixs = pixClipRectangle(pix, box, NULL);
    pixDisplayWrite(pixs, 1);

        /* Just do one of the examples */
    if (argc == 3) {
        index = atoi(argv[1]);
        maxiters = atoi(argv[2]);
        if (index <= 7)
            type = L_THIN_FG;
        else
            type = L_THIN_BG;
        pixt = pixThinExamples(pixs, type, index, maxiters, "junksels");
        pixDisplayWrite(pixt, 1);
        pixDestroy(&pixt);
        system("xv junksels &");
        system("gthumb junk_write_display* &");
        return 0;
    }

        /* Do all the examples */
    pixt = pixThinExamples(pixs, L_THIN_FG, 1, 0, "junksel_example1");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 2, 0, "junksel_example2");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 3, 0, "junksel_example3");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 4, 0, "junksel_example4");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 5, 0, "junksel_example5");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 6, 0, "junksel_example6");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_FG, 7, 0, "junksel_example7");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_BG, 8, 5, "junksel_example8");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
    pixt = pixThinExamples(pixs, L_THIN_BG, 9, 5, "junksel_example9");
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);

        /* Display the thinning results */
    pixa = pixaReadFiles(".", "junk_write_display");
    pixd = pixaDisplayTiledAndScaled(pixa, 8, 500, 1, 0, 25, 2);
    pixWrite("junktiles", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

        /* Display the sels used in the examples */
    pixa = pixaReadFiles(".", "junksel_example");
    pixd = pixaDisplayTiledInRows(pixa, 500, 0, 50);
    pixWrite("junksels", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    pixDestroy(&pix);
    pixDestroy(&pixs);
    boxDestroy(&box);

    system("gthumb junk_write_display* &");
    return 0;
}


