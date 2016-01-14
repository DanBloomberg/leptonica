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
 * subpixel_reg.c
 *
 *   Regression test for subpixel scaling.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

void AddTextAndSave(PIXA *pixa, PIX *pixs, l_int32 newrow,
                    BMF *bmf, const char *textstr,
                    l_int32 location, l_uint32 val);

const char  *textstr[] =
           {"Downscaled with sharpening",
            "Subpixel scaling; horiz R-G-B",
            "Subpixel scaling; horiz B-G-R",
            "Subpixel scaling; vert R-G-B",
            "Subpixel scaling; vert B-G-R"};

main(int    argc,
     char **argv)
{
BMF         *bmf, *bmftop;
PIX         *pixs, *pixg, *pixt, *pixd;
PIX         *pix1, *pix2, *pix3, *pix4, *pix5;
PIXA        *pixa;
static char  mainName[] = "subpixel_reg";

    pixa = pixaCreate(5);
    bmf = bmfCreate("./fonts", 6);
    bmftop = bmfCreate("./fonts", 10);
    pixs = pixRead("lucasta-47.jpg");
    pixg = pixScale(pixs, 0.4, 0.4);  /* 8 bpp grayscale */
    pix1 = pixConvertTo32(pixg);  /* 32 bpp rgb */
    AddTextAndSave(pixa, pix1, 1, bmf, textstr[0], L_ADD_BELOW, 0xff000000);
    pix2 = pixConvertGrayToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_RGB);
    AddTextAndSave(pixa, pix2, 0, bmf, textstr[1], L_ADD_BELOW, 0x00ff0000);
    pix3 = pixConvertGrayToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_BGR);
    AddTextAndSave(pixa, pix3, 0, bmf, textstr[2], L_ADD_BELOW, 0x0000ff00);
    pix4 = pixConvertGrayToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_VRGB);
    AddTextAndSave(pixa, pix4, 0, bmf, textstr[3], L_ADD_BELOW, 0x00ff0000);
    pix5 = pixConvertGrayToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_VBGR);
    AddTextAndSave(pixa, pix5, 0, bmf, textstr[4], L_ADD_BELOW, 0x0000ff00);

    pixt = pixaDisplay(pixa, 0, 0);
    pixd = pixAddSingleTextblock(pixt, bmftop,
                                 "Regression test for subpixel scaling: gray",
                                 0xff00ff00, L_ADD_ABOVE, NULL);
    pixWrite("/tmp/junkpixd1.png", pixd, IFF_PNG);
    pixDisplay(pixd, 50, 50);
    pixaDestroy(&pixa);
    pixDestroy(&pixs);
    pixDestroy(&pixg);
    pixDestroy(&pixt);
    pixDestroy(&pixd);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    pixDestroy(&pix4);
    pixDestroy(&pix5);

    pixa = pixaCreate(5);
    pixs = pixRead("fish24.jpg");
    pix1 = pixScale(pixs, 0.4, 0.4);  /* 32 bpp rgb */
    AddTextAndSave(pixa, pix1, 1, bmf, textstr[0], L_ADD_BELOW, 0xff000000);
    pix2 = pixConvertToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_RGB);
    AddTextAndSave(pixa, pix2, 0, bmf, textstr[1], L_ADD_BELOW, 0x00ff0000);
    pix3 = pixConvertToSubpixelRGB(pixs, 0.4, 0.35, L_SUBPIXEL_ORDER_BGR);
    AddTextAndSave(pixa, pix3, 0, bmf, textstr[2], L_ADD_BELOW, 0x0000ff00);
    pix4 = pixConvertToSubpixelRGB(pixs, 0.4, 0.45, L_SUBPIXEL_ORDER_VRGB);
    AddTextAndSave(pixa, pix4, 0, bmf, textstr[3], L_ADD_BELOW, 0x00ff0000);
    pix5 = pixConvertToSubpixelRGB(pixs, 0.4, 0.4, L_SUBPIXEL_ORDER_VBGR);
    AddTextAndSave(pixa, pix5, 0, bmf, textstr[4], L_ADD_BELOW, 0x0000ff00);

    pixt = pixaDisplay(pixa, 0, 0);
    pixd = pixAddSingleTextblock(pixt, bmftop,
                                 "Regression test for subpixel scaling: color",
                                 0xff00ff00, L_ADD_ABOVE, NULL);
    pixWrite("/tmp/junkpixd2.png", pixd, IFF_PNG);
    pixDisplay(pixd, 50, 350);
    pixaDestroy(&pixa);
    pixDestroy(&pixs);
    pixDestroy(&pixt);
    pixDestroy(&pixd);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    pixDestroy(&pix4);
    pixDestroy(&pix5);

    bmfDestroy(&bmf);
    bmfDestroy(&bmftop);
    return 0;
}


void
AddTextAndSave(PIXA        *pixa,
               PIX         *pixs,
               l_int32      newrow,
               BMF         *bmf,
               const char  *textstr,
               l_int32      location,
               l_uint32     val)
{
l_int32  n, ovf;
PIX     *pixt;

    pixt = pixAddSingleTextblock(pixs, bmf, textstr, val, location, &ovf);
    n = pixaGetCount(pixa);
    pixSaveTiledOutline(pixt, pixa, 1, newrow, 30, 2, 32);
    if (ovf) fprintf(stderr, "Overflow writing text in image %d\n", n + 1);
    pixDestroy(&pixt);
    return;
}

