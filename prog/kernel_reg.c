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

static const char  *kdatastr = " 20.3    50   80  50   20 "
                               " 51.4   100  140  100  50 "
                               " 92.5   160  200  160  90 "
                               " 53.7   100  140  100  50 "
                               " 24.9    50   80   50  20 ";

main(int    argc,
     char **argv)
{
char        *str;
l_int32      i, j;
l_float32    sum;
L_KERNEL    *kel1, *kel2, *kel3, *kelx, *kely;
PIX         *pixs, *pixg, *pixd, *pixt, *pixt2;
PIXA        *pixa;
SARRAY      *sa;
static char  mainName[] = "kernel_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  kernel_reg", mainName, 1));

    pixa = pixaCreate(0);

        /* Test creating from a string */
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    pixd = kernelDisplayInPix(kel1, 41, 2);
    pixSaveTiled(pixd, pixa, 1, 1, 20, 8);
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
    sarrayAddString(sa, "20.5   50   80    50   20", L_COPY);
    sarrayAddString(sa, "82.    120  180   120  80", L_COPY);
    sarrayAddString(sa, "22.1   50   80    50   20", L_COPY);
    str = sarrayToString(sa, 1);
    arrayWrite("junkkernfile", "w", str, strlen(str));
    kel2 = kernelCreateFromFile("junkkernfile");
    pixd = kernelDisplayInPix(kel2, 41, 2);
    pixSaveTiled(pixd, pixa, 1, 1, 20, 0);
    pixWrite("junkker1.bmp", pixd, IFF_BMP);
    pixDestroy(&pixd);
    sarrayDestroy(&sa);
    FREE(str);
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
    pixSaveTiled(pixd, pixa, 1, 0, 20, 0);
    pixWrite("junkker2.bmp", pixd, IFF_BMP);
    pixDestroy(&pixd);
    pixDestroy(&pixt);
    kernelDestroy(&kel3);

        /* Test convolution with kel1 */
    pixs = pixRead("test24.jpg");
    pixg = pixScaleRGBToGrayFast(pixs, 3, COLOR_GREEN);
    pixSaveTiled(pixg, pixa, 1, 1, 20, 0);
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    pixd = pixConvolve(pixg, kel1, 8, 1);
    pixSaveTiled(pixd, pixa, 1, 0, 20, 0);
    pixWrite("junkker3.bmp", pixd, IFF_BMP);
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
    pixd = pixConvolve(pixg, kel2, 8, 1);
    pixSaveTiled(pixd, pixa, 1, 1, 20, 0);
    pixWrite("junkker4.bmp", pixd, IFF_BMP);
    pixt = pixBlockconv(pixg, 5, 5);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixWrite("junkker5.bmp", pixt, IFF_BMP);
    pixCompareGray(pixd, pixt, L_COMPARE_ABS_DIFF, GPLOT_X11, NULL,
                   NULL, NULL, NULL);
    pixDestroy(&pixs);
    pixDestroy(&pixg);
    pixDestroy(&pixd);
    pixDestroy(&pixt);
    kernelDestroy(&kel2);

        /* Test generation and convolution with gaussian kernel */
    pixs = pixRead("test8.jpg");
    pixSaveTiled(pixs, pixa, 1, 1, 20, 0);
    kel1 = makeGaussianKernel(5, 5, 3.0, 5.0);
    kernelGetSum(kel1, &sum);
    fprintf(stderr, "Sum for gaussian kernel = %f\n", sum);
    kernelWrite("junkgauss.kel", kel1);
    pixt = pixConvolve(pixs, kel1, 8, 1);
    pixt2 = pixConvolve(pixs, kel1, 16, 0);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 0);
    pixWrite("junkker6.bmp", pixt, IFF_BMP);
    pixDestroy(&pixt);
    pixDestroy(&pixt2);

    pixt = kernelDisplayInPix(kel1, 25, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixDestroy(&pixt);
    kernelDestroy(&kel1);
    pixDestroy(&pixs);

        /* Test generation and convolution with separable gaussian kernel */
    pixs = pixRead("test8.jpg");
    pixSaveTiled(pixs, pixa, 1, 1, 20, 0);
    makeGaussianKernelSep(5, 5, 3.0, 5.0, &kelx, &kely);
    kernelGetSum(kelx, &sum);
    fprintf(stderr, "Sum for x gaussian kernel = %f\n", sum);
    kernelGetSum(kely, &sum);
    fprintf(stderr, "Sum for y gaussian kernel = %f\n", sum);
    kernelWrite("junkgauss.kelx", kelx);
    kernelWrite("junkgauss.kely", kely);

    pixt = pixConvolveSep(pixs, kelx, kely, 8, 1);
    pixt2 = pixConvolveSep(pixs, kelx, kely, 16, 0);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 0);
    pixWrite("junkker7.bmp", pixt, IFF_BMP);
    pixDestroy(&pixt);
    pixDestroy(&pixt2);

    pixt = kernelDisplayInPix(kelx, 25, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixDestroy(&pixt);
    pixt = kernelDisplayInPix(kely, 25, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixDestroy(&pixt);
    kernelDestroy(&kelx);
    kernelDestroy(&kely);
    pixDestroy(&pixs);

        /* Test generation and convolution with diff of gaussians kernel */
/*    pixt = pixRead("marge.jpg");
    pixs = pixConvertRGBToLuminance(pixt);
    pixDestroy(&pixt); */
    pixs = pixRead("test8.jpg");
    pixSaveTiled(pixs, pixa, 1, 1, 20, 0);
    kel1 = makeDoGKernel(7, 7, 1.5, 2.7);
    kernelGetSum(kel1, &sum);
    fprintf(stderr, "Sum for DoG kernel = %f\n", sum);
    kernelWrite("junkdog.kel", kel1);
    pixt = pixConvolve(pixs, kel1, 8, 0);
/*    pixInvert(pixt, pixt); */
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixWrite("junkker8.bmp", pixt, IFF_BMP);
    pixDestroy(&pixt);

    pixt = kernelDisplayInPix(kel1, 20, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 0);
    pixDestroy(&pixt);
    kernelDestroy(&kel1);
    pixDestroy(&pixs);

    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkkernel.jpg", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);
    return 0;
}

