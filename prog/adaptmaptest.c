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
 * adaptmaptest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


#define  SIZE_X        10
#define  SIZE_Y        30
#define  BINTHRESH     50
#define  MINCOUNT      30

#define  BGVAL         200
#define  SMOOTH_X      2
#define  SMOOTH_Y      1

   /* wet-day.jpg; 0.63 scaling  */
#define  XS     151
#define  YS     225
#define  WS     913
#define  HS     1285


main(int    argc,
     char **argv)
{
l_int32      d;
PIX         *pixs, *pixg, *pixm, *pixmi, *pixd, *pixd2;
PIX         *pixmr, *pixmg, *pixmb, *pixmri, *pixmgi, *pixmbi;
PIX         *pixim;
char        *filein, *fileout;
static char  mainName[] = "adaptmaptest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  adaptmaptest filein fileout",
	       mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

    d = pixGetDepth(pixs);
    if (d != 8 && d != 32)
        exit(ERROR_INT("pix not 8 or 32 bpp", mainName, 1));

    /* process in grayscale */
#if 0
    startTimer();
    if (d == 32) {
        pixg = pixConvertRGBToGray(pixs, 0.33, 0.34, 0.33);
        fprintf(stderr, "time for conversion to gray: %7.3f\n", stopTimer());
    } else
        pixg = pixClone(pixs);
    
    startTimer();
    pixim = NULL;
    pixim = pixCreate(pixGetWidth(pixs), pixGetHeight(pixs), 1);
    pixRasterop(pixim, XS, YS, WS, HS, PIX_SET, NULL, 0, 0);
    pixm = pixGetBackgroundMap(pixg, pixim, SIZE_X, SIZE_Y,
                               BINTHRESH, MINCOUNT);
    fprintf(stderr, "time for adapt map generation: %7.3f\n", stopTimer());
    pixWrite("junkpixm", pixm, IFF_PNG);

    startTimer();
    pixmi = pixGetInvBackgroundMap(pixm, BGVAL, SMOOTH_X, SMOOTH_Y);
    fprintf(stderr, "time for inv map generation: %7.3f\n", stopTimer());
    pixWrite("junkpixmi", pixmi, IFF_PNG);

    startTimer();
    pixd = pixApplyInvBackgroundMap(pixg, pixmi, SIZE_X, SIZE_Y);
    fprintf(stderr, "time for applying inv map: %7.3f\n", stopTimer());
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);

    pixd2 = pixGammaTRCMasked(NULL, pixd, pixim, 1.0, 0, 190);
    pixInvert(pixim, pixim);
    pixGammaTRCMasked(pixd2, pixd2, pixim, 1.0, 60, 190);
    pixWrite("junkpixo", pixd2, IFF_JFIF_JPEG);
    pixDestroy(&pixim);
    pixDestroy(&pixg);
    pixDestroy(&pixm);
    pixDestroy(&pixmi);
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* process in color if input is 32 bpp */
#if 0
    if (d != 32)
        exit(ERROR_INT("pix not 32 bpp", mainName, 1));
    
    startTimer();
    pixmr = pixmg = pixmb = NULL;
    pixim = pixCreate(pixGetWidth(pixs), pixGetHeight(pixs), 1);
    pixRasterop(pixim, XS, YS, WS, HS, PIX_SET, NULL, 0, 0);
    pixGetBackgroundMaps(pixs, pixim, SIZE_X, SIZE_Y, BINTHRESH, MINCOUNT,
                         &pixmr, &pixmg, &pixmb);
    fprintf(stderr, "time for adapt map generation: %7.3f\n", stopTimer());
    pixWrite("junkpixmr", pixmr, IFF_PNG);
    pixWrite("junkpixmg", pixmg, IFF_PNG);
    pixWrite("junkpixmb", pixmb, IFF_PNG);

    startTimer();
    pixmri = pixGetInvBackgroundMap(pixmr, BGVAL, SMOOTH_X, SMOOTH_Y);
    pixmgi = pixGetInvBackgroundMap(pixmg, BGVAL, SMOOTH_X, SMOOTH_Y);
    pixmbi = pixGetInvBackgroundMap(pixmb, BGVAL, SMOOTH_X, SMOOTH_Y);
    fprintf(stderr, "time for inv map generation: %7.3f\n", stopTimer());
    pixWrite("junkpixmri", pixmri, IFF_PNG);
    pixWrite("junkpixmgi", pixmgi, IFF_PNG);
    pixWrite("junkpixmbi", pixmbi, IFF_PNG);

    startTimer();
    pixd = pixApplyInvBackgroundMaps(pixs, pixmri, pixmgi, pixmbi,
                                    SIZE_X, SIZE_Y);
    fprintf(stderr, "time for applying inv maps: %7.3f\n", stopTimer());
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);

    pixd2 = pixGammaTRCMasked(NULL, pixd, pixim, 1.0, 0, 190);
    pixInvert(pixim, pixim);
    pixGammaTRCMasked(pixd2, pixd2, pixim, 1.0, 60, 190);
    pixWrite("junkpixo", pixd2, IFF_JFIF_JPEG);
    pixDestroy(&pixmr);
    pixDestroy(&pixmg);
    pixDestroy(&pixmb);
    pixDestroy(&pixmri);
    pixDestroy(&pixmgi);
    pixDestroy(&pixmbi);
    pixDestroy(&pixim);
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* process in either gray or color, depending on the source */
#if 1
    startTimer();
    pixim = pixCreate(pixGetWidth(pixs), pixGetHeight(pixs), 1);
    pixRasterop(pixim, XS, YS, WS, HS, PIX_SET, NULL, 0, 0);
/*    pixd = pixBackgroundNorm(pixs, pixim, NULL,SIZE_X, SIZE_Y,
                               BINTHRESH, MINCOUNT,
                               BGVAL, SMOOTH_X, SMOOTH_Y); */
    pixd = pixBackgroundNorm(pixs, pixim, NULL, 5, 10, BINTHRESH, 20,
                             BGVAL, SMOOTH_X, SMOOTH_Y);
    fprintf(stderr, "time for bg normalization: %7.3f\n", stopTimer());
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);

    pixd2 = pixGammaTRCMasked(NULL, pixd, pixim, 1.0, 0, 190);
    pixInvert(pixim, pixim);
    pixGammaTRCMasked(pixd2, pixd2, pixim, 1.0, 60, 190);
    pixWrite("junkpixo", pixd2, IFF_JFIF_JPEG);

    pixDestroy(&pixd);
    pixDestroy(&pixd2);
    pixDestroy(&pixim);
#endif
    
    pixDestroy(&pixs);
    exit(0);
}

