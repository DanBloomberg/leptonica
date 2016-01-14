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
 * colorquanttest.c
 *
 *    for testing various octree quantizers
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  NCOLORS      250
#define  DITHER       1

    /* for simple colorspace filled with octcubes */
#define  OCTCUBE_LEVEL        3


main(int    argc,
     char **argv)
{
l_int32      nerrors, same;
PIX         *pixs, *pixc, *pixc1, *pixc2, *pixc3, *pixc4, *pixd;
char        *filein, *fileout;
static char  mainName[] = "colorquanttest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  colorquanttest filein fileout", mainName, 1));

    pixc = pixd = NULL;
    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));

#if 1   /* median cut quantizer */
    startTimer();
    pixc = pixMedianCutQuantGeneral(pixs, 0, 0, 256, 5, 1);
/*    pixc = pixMedianCutQuant(pixs, 1); */
    fprintf(stderr, "finished making cmapped pix: %7.3f secs\n", stopTimer());
    pixWrite(fileout, pixc, IFF_PNG);
    pixDestroy(&pixc);
#endif

#if 0   /* simple one-pass quantizer */
    startTimer();
    pixc = pixColorQuant1Pass(pixs, DITHER);
    fprintf(stderr, "finished making cmapped pix: %7.3f secs\n", stopTimer());
    pixWrite(fileout, pixc, IFF_PNG);

  #if 1
    pixd = pixRemoveColormap(pixc, REMOVE_CMAP_TO_FULL_COLOR);
    pixWrite("junkimage", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixDestroy(&pixc);
  #endif
#endif

#if 0   /* better 2-pass quantizer */
    startTimer();
    pixc = pixOctreeColorQuant(pixs, NCOLORS, DITHER);
    fprintf(stderr, "finished making cmapped pix: %7.3f secs\n", stopTimer());
    pixWrite(fileout, pixc, IFF_PNG);

  #if 1
    pixd = pixRemoveColormap(pixc, REMOVE_CMAP_TO_FULL_COLOR);
    pixWrite("junkimage", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixDestroy(&pixc);
  #endif
#endif

#if 0   /* simple octcube quantizer */
    pixd = pixFixedOctcubeQuantRGB(pixs, OCTCUBE_LEVEL);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#endif

#if 0   /* quantize to gray */
    pixc1 = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
/*    pixc = pixThresholdTo4bpp(pixc1, 16, 1); */
    pixc = pixThresholdTo2bpp(pixc1, 4, 1);
    pixWrite(fileout, pixc, IFF_PNG);
    pixDestroy(&pixc);
    pixDestroy(&pixc1);
#endif

#if 0   /* quantize to color */
    pixc1 = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    pixc = pixOctreeQuant(pixc1, 256, 1);
    pixWrite(fileout, pixc, IFF_PNG);
    pixDestroy(&pixc);
    pixDestroy(&pixc1);
#endif

#if 0   /* try to get all the colors  */
    pixc1 = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    pixc = pixFixedOctcubeQuantCmap(pixc1, 1);
    pixWrite(fileout, pixc, IFF_PNG);
    pixDestroy(&pixc);
    pixDestroy(&pixc1);
#endif

#if 0   /* simple 4 or 8 bpp quantizer */
    startTimer();
    pixc = pixOctreeQuant(pixs, 256, 0);
    fprintf(stderr, "finished making cmapped pix: %7.3f secs\n", stopTimer());
    pixWrite(fileout, pixc, IFF_PNG);
    pixDestroy(&pixc);
#endif

#if 0   /* simple 4 or 8 bpp quantizer; back to RGB; quantize again */
    pixc = pixOctreeQuant(pixs, 256, 0);
    pixcmapWriteStream(stderr, pixGetColormap(pixc));
    pixWrite(fileout, pixc, IFF_PNG);
    pixc1 = pixRemoveColormap(pixc, REMOVE_CMAP_BASED_ON_SRC);
    pixWrite("junkpixc1", pixc1, IFF_PNG);
    pixc2 = pixConvertRGBToColormap(pixc1, 5, &nerrors);
    pixWrite("junkpixc2", pixc2, IFF_PNG);
    pixcmapWriteStream(stderr, pixGetColormap(pixc2));
    pixEqual(pixc, pixc2, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixDestroy(&pixc);
    pixDestroy(&pixc1);
    pixDestroy(&pixc2);
#endif

#if 0   /* colormap --> RGB --> fixed octcube quant to 4 or 8 bpp
         *   --> back to RGB --> quantize again. */
    pixc1 = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    pixWrite("junkc1", pixc1, IFF_PNG);
    pixc2 = pixFixedOctcubeQuant(pixc1, 3);
    pixWrite("junkc2", pixc2, IFF_PNG);
    pixcmapWriteStream(stderr, pixGetColormap(pixc2));
    pixc3 = pixRemoveColormap(pixc2, REMOVE_CMAP_BASED_ON_SRC);
    pixWrite("junkc3", pixc3, IFF_PNG);
    pixc4 = pixConvertRGBToColormap(pixc3, 3, &nerrors);
    pixWrite("junkc4", pixc4, IFF_PNG);
    pixcmapWriteStream(stderr, pixGetColormap(pixc4));
    pixEqual(pixc1, pixc2, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc1, pixc3, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc1, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc2, pixc3, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc2, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc3, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixDestroy(&pixs);
    pixDestroy(&pixc1);
    pixDestroy(&pixc2);
    pixDestroy(&pixc3);
    pixDestroy(&pixc4);
#endif

#if 0   /* colormap --> RGB --> simple 4 or 8 bpp quantizer
         *   --> back to RGB --> quantize again.
	 * e.g., do this with small images */
    pixc1 = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    pixWrite("junkc1", pixc1, IFF_PNG);
    pixc2 = pixOctreeQuant(pixc1, 16, 1);
    pixWrite("junkc2", pixc2, IFF_PNG);
    pixcmapWriteStream(stderr, pixGetColormap(pixc2));
    pixc3 = pixRemoveColormap(pixc2, REMOVE_CMAP_BASED_ON_SRC);
    pixWrite("junkc3", pixc3, IFF_PNG);
    pixc4 = pixConvertRGBToColormap(pixc3, 2, &nerrors);
    pixWrite("junkc4", pixc4, IFF_PNG);
    pixcmapWriteStream(stderr, pixGetColormap(pixc4));
    pixEqual(pixc1, pixc2, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc1, pixc3, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc1, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc2, pixc3, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc2, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixEqual(pixc3, pixc4, &same);
    fprintf(stderr, "Same?  %d\n", same);
    pixDestroy(&pixs);
    pixDestroy(&pixc1);
    pixDestroy(&pixc2);
    pixDestroy(&pixc3);
    pixDestroy(&pixc4);
#endif

    exit(0);
}

