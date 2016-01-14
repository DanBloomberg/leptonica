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
 * kernel_reg.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

static const char  *kdatastr = " 20    50   80  50   20 "
                               " 50   100  140  100  50 "
                               " 90   160  200  160  90 "
                               " 50   100  140  100  50 "
                               " 20    50   80   50  20 ";

main(int    argc,
     char **argv)
{
char        *str;
l_int32      i, j;
L_KERNEL    *kel1, *kel2, *kel3;
PIX         *pixs, *pixg, *pixd, *pixt;
SARRAY      *sa;
static char  mainName[] = "kernel_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  kernel_reg", mainName, 1));

        /* Test creating from a string */
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    pixd = kernelDisplayInPix(kel1, 41, 2);
    pixDisplay(pixd, 50, 0);
    pixWrite("junkpixd1", pixd, IFF_BMP);
    pixDestroy(&pixd);
    kernelDestroy(&kel1);

        /* Test read/write for kernel */
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    kernelWrite("junkkern1", kel1);
    kel2 = kernelRead("junkkern1");
    kernelWrite("junkkern2", kel2);
    system("diff -s junkkern1 junkkern2");
    kernelDestroy(&kel1);
    kernelDestroy(&kel2);

        /* Test creating from a file */
    sa = sarrayCreate(0);
    sarrayAddString(sa, "# small 3x3 kernel", L_COPY);
    sarrayAddString(sa, "3 5", L_COPY);
    sarrayAddString(sa, "1 2", L_COPY);
    sarrayAddString(sa, "20   50   80    50   20", L_COPY);
    sarrayAddString(sa, "80  120  180   120   80", L_COPY);
    sarrayAddString(sa, "20   50   80    50   20", L_COPY);
    str = sarrayToString(sa, 1);
    arrayWrite("junkkernfile", "w", str, strlen(str));
    kel2 = kernelCreateFromFile("junkkernfile");
    pixd = kernelDisplayInPix(kel2, 41, 2);
    pixDisplay(pixd, 50, 300);
    pixWrite("junkpixd2", pixd, IFF_BMP);
    sarrayDestroy(&sa);
    FREE(str);
    pixDestroy(&pixd);
    kernelDestroy(&kel2);

        /* Test creating from a pix */
    pixt = pixCreate(5, 3, 8);
    pixSetPixel(pixt, 0, 0, 20);
    pixSetPixel(pixt, 1, 0, 50);
    pixSetPixel(pixt, 2, 0, 80);
    pixSetPixel(pixt, 3, 0, 50);
    pixSetPixel(pixt, 4, 0, 20);
    pixSetPixel(pixt, 0, 1, 80);
    pixSetPixel(pixt, 1, 1, 120);
    pixSetPixel(pixt, 2, 1, 180);
    pixSetPixel(pixt, 3, 1, 120);
    pixSetPixel(pixt, 4, 1, 80);
    pixSetPixel(pixt, 0, 0, 20);
    pixSetPixel(pixt, 1, 2, 50);
    pixSetPixel(pixt, 2, 2, 80);
    pixSetPixel(pixt, 3, 2, 50);
    pixSetPixel(pixt, 4, 2, 20);
    kel3 = kernelCreateFromPix(pixt, 1, 2);
    pixd = kernelDisplayInPix(kel3, 41, 2);
    pixDisplay(pixd, 50, 500);
    pixWrite("junkpixd3", pixd, IFF_BMP);
    pixDestroy(&pixd);
    pixDestroy(&pixt);
    kernelDestroy(&kel3);

        /* Test convolution with kel1 */
    pixs = pixRead("test24.jpg");
    pixg = pixScaleRGBToGrayFast(pixs, 3, COLOR_GREEN);
    pixDisplay(pixg, 300, 0);
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    pixd = pixConvolve(pixg, kel1);
    pixDisplay(pixd, 300, 400);
    pixWrite("junkpixd4", pixd, IFF_BMP);
    pixDestroy(&pixs);
    pixDestroy(&pixg);
    pixDestroy(&pixd);
    kernelDestroy(&kel1);

        /* Test convolution with flat rectangular kel */
    pixs = pixRead("test24.jpg");
    pixg = pixScaleRGBToGrayFast(pixs, 3, COLOR_GREEN);
    kel2 = kernelCreate(11, 11);
    kernelSetOrigin(kel2, 5, 5);
    for (i = 0; i < 11; i++) {
        for (j = 0; j < 11; j++)
            kernelSetElement(kel2, i, j, 1);
    }
    pixd = pixConvolve(pixg, kel2);
    pixDisplay(pixd, 300, 800);
    pixWrite("junkpixd5", pixd, IFF_BMP);
    pixt = pixBlockconv(pixg, 5, 5);
    pixDisplay(pixd, 800, 800);
    pixWrite("junkpixd6", pixt, IFF_BMP);
    pixCompareGray(pixd, pixt, L_COMPARE_ABS_DIFF, GPLOT_X11, NULL,
                   NULL, NULL, NULL);
    pixDestroy(&pixs);
    pixDestroy(&pixg);
    pixDestroy(&pixd);
    pixDestroy(&pixt);
    kernelDestroy(&kel2);

    exit(0);
}

