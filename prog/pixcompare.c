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
 * pixcompare.c
 *
 *    Compares two images, using the absolute value of the
 *    difference between pixel (or component values, for RGB).
 *    If one has a colormap and the other doesn't, the colormap
 *    is removed before making the comparison.
 *
 *    Here's an interesting observation.  Take an image that has
 *    been jpeg compressed at a quality = 75.  If you re-compress
 *    the image, what quality factor should be used to minimize
 *    the change?  Answer:  75 (!)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

    /* set one of these to 1 */
#define   XOR                   0
#define   SUBTRACT_1_FROM_2     0
#define   SUBTRACT_2_FROM_1     0
#define   ABS_DIFFERENCE        1

main(int    argc,
     char **argv)
{
l_int32      w, h, d1, d2, d, n, zero, first, last, same;
char        *filein1, *filein2, *fileout;
NUMA        *na, *na2;
PIX         *pixs1, *pixs2, *pixt1, *pixt2, *pixd;
PIXCMAP     *cmap1, *cmap2;
static char  mainName[] = "pixcompare";

    if (argc != 4)
	exit(ERROR_INT(" Syntax:  pixcompare filein1 filein2 fileout",
	               mainName, 1));

    filein1 = argv[1];
    filein2 = argv[2];
    fileout = argv[3];

    if ((pixs1 = pixRead(filein1)) == NULL)
	exit(ERROR_INT("pixs1 not made", mainName, 1));
    if ((pixs2 = pixRead(filein2)) == NULL)
	exit(ERROR_INT("pixs2 not made", mainName, 1));
    d1 = pixGetDepth(pixs1);
    d2 = pixGetDepth(pixs2);
    if (d1 == 1 || d2 == 1)
	exit(ERROR_INT("at least one pix is 1 bpp", mainName, 1));

    w = pixGetWidth(pixs1);
    if (w != pixGetWidth(pixs2))
	exit(ERROR_INT("image widths differ", mainName, 1));
    h = pixGetHeight(pixs1);
    if (h != pixGetHeight(pixs2))
	exit(ERROR_INT("image heights differ", mainName, 1));

    pixEqual(pixs1, pixs2, &same);
    if (same) {
	fprintf(stderr, "Images are identical\n");
        pixd = pixCreateTemplate(pixs1);  /* empty "diff" result */
	pixWrite(fileout, pixd, IFF_PNG);
	pixDestroy(&pixs1);
	pixDestroy(&pixs2);
	pixDestroy(&pixd);
	exit(0);
    }

        /* OK, they're different.  We have to first remove any colormaps. */
    cmap1 = pixGetColormap(pixs1);
    cmap2 = pixGetColormap(pixs2);
    if (cmap1) {
	pixt1 = pixRemoveColormap(pixs1, REMOVE_CMAP_BASED_ON_SRC);
	pixDestroy(&pixs1);
	pixs1 = pixt1;
    }
    if (cmap2) {
	pixt2 = pixRemoveColormap(pixs2, REMOVE_CMAP_BASED_ON_SRC);
	pixDestroy(&pixs2);
	pixs2 = pixt2;
    }

        /* compare depths without colormaps */
    d = pixGetDepth(pixs1);
    if (d != pixGetDepth(pixs2)) {
	fprintf(stderr, "Intrinsic pix depths are different\n");
	pixDestroy(&pixs1);
	pixDestroy(&pixs2);
	exit(0);
    }

#if XOR 
    fprintf(stderr, "xor: 1 ^ 2\n");
    pixRasterop(pixs1, 0, 0, w, h, PIX_SRC ^ PIX_DST, pixs2, 0, 0);
    pixZero(pixs1, &zero);
    if (zero)
	fprintf(stderr, "Images are identical\n");
    else
	fprintf(stderr, "Images differ\n");
    pixWrite(fileout, pixs1, IFF_PNG);
#elif  SUBTRACT_1_FROM_2
    fprintf(stderr, "subtract: 2 - 1\n");
    pixRasterop(pixs1, 0, 0, w, h, PIX_SRC & PIX_NOT(PIX_DST), pixs2, 0, 0);
    pixZero(pixs1, &zero);
    if (zero)
	fprintf(stderr, "Images are identical\n");
    else
	fprintf(stderr, "Images differ\n");
    pixWrite(fileout, pixs1, IFF_PNG);
#elif  SUBTRACT_2_FROM_1
    fprintf(stderr, "subtract: 1 - 2\n");
    pixRasterop(pixs1, 0, 0, w, h, PIX_DST & PIX_NOT(PIX_SRC), pixs2, 0, 0);
    pixZero(pixs1, &zero);
    if (zero)
	fprintf(stderr, "Images are identical\n");
    else
	fprintf(stderr, "Images differ\n");
    pixWrite(fileout, pixs1, IFF_PNG);
#else  /*  ABS_DIFFERENCE  */
    fprintf(stderr, "component-wise absdiff: 1 - 2\n");
    pixd = pixAbsDifference(pixs1, pixs2);
    if (d != 32) {
	na = pixGrayHistogram(pixd);
	numaGetNonzeroRange(na, &first, &last);
	na2 = numaClipToInterval(na, 0, last);
	gplotSimple1(na2, GPLOT_X11, "junkroot", "diff histo");
	numaDestroy(&na);
	numaDestroy(&na2);
    }
    else {  /* d == 32 */
	PIX  *pixr, *pixg, *pixb;
	pixr = pixGetRGBComponent(pixd, COLOR_RED);
	na = pixGrayHistogram(pixr);
	numaGetNonzeroRange(na, &first, &last);
	na2 = numaClipToInterval(na, 0, last);
	gplotSimple1(na2, GPLOT_X11, "junkrootred", "red diff histo");
	numaDestroy(&na);
	numaDestroy(&na2);
	pixDestroy(&pixr);
	pixg = pixGetRGBComponent(pixd, COLOR_GREEN);
	na = pixGrayHistogram(pixg);
	numaGetNonzeroRange(na, &first, &last);
	na2 = numaClipToInterval(na, 0, last);
	gplotSimple1(na2, GPLOT_X11, "junkrootgreen", "green diff histo");
	numaDestroy(&na);
	numaDestroy(&na2);
	pixDestroy(&pixg);
	pixb = pixGetRGBComponent(pixd, COLOR_BLUE);
	na = pixGrayHistogram(pixb);
	numaGetNonzeroRange(na, &first, &last);
	na2 = numaClipToInterval(na, 0, last);
	gplotSimple1(na2, GPLOT_X11, "junkrootblue", "blue diff histo");
	numaDestroy(&na);
	numaDestroy(&na2);
	pixDestroy(&pixb);
        pixWrite(fileout, pixd, IFF_PNG);
    }
    pixDestroy(&pixd);
#endif

    pixDestroy(&pixs1);
    pixDestroy(&pixs2);
    exit(0);
}

