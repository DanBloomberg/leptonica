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
 * painttest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32     index;
l_uint32    val32;
BOX        *box;
PIX        *pixs, *pixg, *pixb, *pixt, *pixt1, *pixt2, *pixt3;
PIXCMAP    *cmap;
static char     mainName[] = "painttest";

    pixs = pixRead("lucasta.jpg");

        /* Color non-white pixels on RGB */
    pixt = pixConvert8To32(pixs);
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pixt, box, L_PAINT_DARK, 220, 0, 0, 255);
    pixWrite("junkpixt1.jpg", pixt, IFF_JFIF_JPEG);
    pixColorGray(pixt, NULL, L_PAINT_DARK, 220, 255, 100, 100);
    pixWrite("junkpixt2.jpg", pixt, IFF_JFIF_JPEG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Color non-white pixels on colormap */
    pixt = pixThresholdTo4bpp(pixs, 6, 1);
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pixt, box, L_PAINT_DARK, 220, 0, 0, 255);
    pixWrite("junkpixt3.png", pixt, IFF_PNG);
    pixColorGray(pixt, NULL, L_PAINT_DARK, 220, 255, 100, 100);
    pixWrite("junkpixt4.png", pixt, IFF_PNG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Color non-black pixels on RGB */
    pixt = pixConvert8To32(pixs);
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pixt, box, L_PAINT_LIGHT, 20, 0, 0, 255);
    pixWrite("junkpixt5.png", pixt, IFF_PNG);
    pixColorGray(pixt, NULL, L_PAINT_LIGHT, 80, 255, 100, 100);
    pixWrite("junkpixt6.png", pixt, IFF_PNG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Color non-black pixels on colormap */
    pixt = pixThresholdTo4bpp(pixs, 6, 1);
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pixt, box, L_PAINT_LIGHT, 20, 0, 0, 255);
    pixWrite("junkpixt7.png", pixt, IFF_PNG);
    pixColorGray(pixt, NULL, L_PAINT_LIGHT, 20, 255, 100, 100);
    pixWrite("junkpixt8.png", pixt, IFF_PNG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Add highlight color to RGB */
    pixt = pixConvert8To32(pixs);
    box = boxCreate(507, 5, 385, 45);
    pixg = pixClipRectangle(pixs, box, NULL);
    pixb = pixThresholdToBinary(pixg, 180);
    pixInvert(pixb, pixb);
    pixWrite("junkpixb.png", pixb, IFF_PNG);
    composeRGBPixel(50, 0, 250, &val32);
    pixPaintThroughMask(pixt, pixb, box->x, box->y, val32);
    boxDestroy(&box);
    pixDestroy(&pixg);
    pixDestroy(&pixb);
    box = boxCreate(236, 107, 262, 40);
    pixg = pixClipRectangle(pixs, box, NULL);
    pixb = pixThresholdToBinary(pixg, 180);
    pixInvert(pixb, pixb);
    composeRGBPixel(250, 0, 50, &val32);
    pixPaintThroughMask(pixt, pixb, box->x, box->y, val32);
    boxDestroy(&box);
    pixDestroy(&pixg);
    pixDestroy(&pixb);
    box = boxCreate(222, 208, 247, 43);
    pixg = pixClipRectangle(pixs, box, NULL);
    pixb = pixThresholdToBinary(pixg, 180);
    pixInvert(pixb, pixb);
    composeRGBPixel(60, 250, 60, &val32);
    pixPaintThroughMask(pixt, pixb, box->x, box->y, val32);
    boxDestroy(&box);
    pixDestroy(&pixg);
    pixDestroy(&pixb);
    pixWrite("junkpixt9.png", pixt, IFF_PNG);
    pixDestroy(&pixt);

        /* Add highlight color to colormap */
    pixt = pixThresholdTo4bpp(pixs, 5, 1);
    cmap = pixGetColormap(pixt);
    pixcmapGetIndex(cmap, 255, 255, 255, &index);
    box = boxCreate(507, 5, 385, 45);
    pixSetSelectCmap(pixt, box, index, 50, 0, 250);
    boxDestroy(&box);
    box = boxCreate(236, 107, 262, 40);
    pixSetSelectCmap(pixt, box, index, 250, 0, 50);
    boxDestroy(&box);
    box = boxCreate(222, 208, 247, 43);
    pixSetSelectCmap(pixt, box, index, 60, 250, 60);
    boxDestroy(&box);
    pixWrite("junkpixt10.png", pixt, IFF_PNG);
    pixDestroy(&pixt);

        /* Paint lines on RGB */
    pixt = pixConvert8To32(pixs);
    pixRenderLineArb(pixt, 450, 20, 850, 320, 5, 200, 50, 125);
    pixRenderLineArb(pixt, 30, 40, 440, 40, 5, 100, 200, 25);
    box = boxCreate(70, 80, 300, 245);
    pixRenderBoxArb(pixt, box, 3, 200, 200, 25);
    pixWrite("junkpixt11.jpg", pixt, IFF_JFIF_JPEG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Paint lines on colormap */
    pixt = pixThresholdTo4bpp(pixs, 5, 1);
    pixRenderLineArb(pixt, 450, 20, 850, 320, 5, 200, 50, 125);
    pixRenderLineArb(pixt, 30, 40, 440, 40, 5, 100, 200, 25);
    box = boxCreate(70, 80, 300, 245);
    pixRenderBoxArb(pixt, box, 3, 200, 200, 25);
    pixWrite("junkpixt12.png", pixt, IFF_PNG);
    pixDestroy(&pixt);
    boxDestroy(&box);

        /* Blend lines on RGB */
    pixt = pixConvert8To32(pixs);
    pixRenderLineBlend(pixt, 450, 20, 850, 320, 5, 200, 50, 125, 0.35);
    pixRenderLineBlend(pixt, 30, 40, 440, 40, 5, 100, 200, 25, 0.35);
    box = boxCreate(70, 80, 300, 245);
    pixRenderBoxBlend(pixt, box, 3, 200, 200, 25, 0.6);
    pixWrite("junkpixt13.jpg", pixt, IFF_JFIF_JPEG);
    pixDestroy(&pixt);
    boxDestroy(&box);

    pixDestroy(&pixs);
}


