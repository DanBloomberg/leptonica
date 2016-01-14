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
 *  fpix_reg.c
 *
 *    Regression test for a number of functions in the FPix utility.
 *    FPix allows you to do floating point operations such as
 *    convolution, with conversions to and from Pix.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

main(int    argc,
char **argv)
{
l_float32    sum, diff;
FPIX        *fpixs, *fpixs2, *fpixs3, *fpixt1, *fpixt2, *fpixd;
L_KERNEL    *kel, *kelx, *kely;
PIX         *pixs, *pixs2, *pixs3, *pixt, *pixd;
PIX         *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
PIXA        *pixa;
static char  mainName[] = "fpix_reg";

    if (argc != 1)
        return ERROR_INT(" Syntax: fpix_reg", mainName, 1);

    pixa = pixaCreate(0);


    kel = makeGaussianKernel(5, 5, 3.0, 4.0);
    kernelGetSum(kel, &sum);
    fprintf(stderr, "Sum for 2d gaussian kernel = %f\n", sum);
    pixt = kernelDisplayInPix(kel, 41, 2);
    pixSaveTiled(pixt, pixa, 1, 1, 20, 8);
    pixDestroy(&pixt);

    makeGaussianKernelSep(5, 5, 3.0, 4.0, &kelx, &kely);
    kernelGetSum(kelx, &sum);
    fprintf(stderr, "Sum for x gaussian kernel = %f\n", sum);
    kernelGetSum(kely, &sum);
    fprintf(stderr, "Sum for y gaussian kernel = %f\n", sum);
    pixt = kernelDisplayInPix(kelx, 41, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);
    pixt = kernelDisplayInPix(kely, 41, 2);
    pixSaveTiled(pixt, pixa, 1, 0, 20, 8);
    pixDestroy(&pixt);

        /* Use fpixRasterop() to generate source image */
    pixs = pixRead("test8.jpg");
    pixs2 = pixRead("karen8.jpg");
    pixRasterop(pixs, 150, 125, 150, 100, PIX_SRC, pixs2, 75, 100);

        /* Convolution directly with pix */
    pixt1 = pixConvolve(pixs, kel, 8, 1);
    pixSaveTiled(pixt1, pixa, 1, 1, 20, 8);
    pixt2 = pixConvolveSep(pixs, kelx, kely, 8, 1);
    pixSaveTiled(pixt2, pixa, 1, 0, 20, 8);

        /* Convolution indirectly with fpix */
    fpixs = pixConvertToFPix(pixs);
    fpixs2 = pixConvertToFPix(pixs2);
    fpixRasterop(fpixs, 150, 125, 150, 100, fpixs2, 75, 100);
    fpixt1 = fpixConvolve(fpixs, kel, 1);
    pixt3 = fpixConvertToPix(fpixt1, 8, L_CLIP_TO_ZERO, 1);
    pixSaveTiled(pixt3, pixa, 1, 1, 20, 8);
    fpixt2 = fpixConvolveSep(fpixs, kelx, kely, 1);
    pixt4 = fpixConvertToPix(fpixt2, 8, L_CLIP_TO_ZERO, 1);
    pixSaveTiled(pixt4, pixa, 1, 0, 20, 8);
    fpixDestroy(&fpixt1);
    fpixDestroy(&fpixt2);

        /* Comparison of results */
    pixCompareGray(pixt1, pixt2, L_COMPARE_ABS_DIFF, 0, NULL,
                   &diff, NULL, NULL);
    fprintf(stderr, "Ave diff of pixConvolve and pixConvolveSep: %f\n", diff);
    pixCompareGray(pixt3, pixt4, L_COMPARE_ABS_DIFF, 0, NULL,
                   &diff, NULL, NULL);
    fprintf(stderr, "Ave diff of fpixConvolve and fpixConvolveSep: %f\n", diff);
    pixCompareGray(pixt1, pixt3, L_COMPARE_ABS_DIFF, 0, NULL,
                   &diff, NULL, NULL);
    fprintf(stderr, "Ave diff of pixConvolve and fpixConvolve: %f\n", diff);
    pixCompareGray(pixt2, pixt4, L_COMPARE_ABS_DIFF, GPLOT_PNG, NULL,
                   &diff, NULL, NULL);
    fprintf(stderr, "Ave diff of pixConvolveSep and fpixConvolveSep: %f\n",
            diff);

        /* Test arithmetic operations; add in a fraction rotated by 180 */
    pixs3 = pixRotate180(NULL, pixs);
    fpixs3 = pixConvertToFPix(pixs3);
    fpixd = fpixLinearCombination(NULL, fpixs, fpixs3, 20.0, 5.0);
    fpixAddMultConstant(fpixd, 0.0, 23.174);   /* multiply up in magnitude */
    pixd = fpixDisplayMaxDynamicRange(fpixd);  /* bring back to 8 bpp */
    pixSaveTiled(pixs, pixa, 1, 1, 20, 8);
    pixSaveTiled(pixd, pixa, 1, 0, 20, 8);
    fpixDestroy(&fpixd);
    pixDestroy(&pixd);

        /* Save the comparison graph; gnuplot should have made it by now! */
    fprintf(stderr, "NOT an error if the next line is\n"
            "    Error in findFileFormat: truncated file\n");
    pixt5 = pixRead("/usr/tmp/junkgrayroot.png");
    pixSaveTiled(pixt5, pixa, 1, 1, 20, 8);

        /* Display results */
    pixd = pixaDisplay(pixa, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkfpix.jpg", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixaDestroy(&pixa);

    pixDestroy(&pixs);
    pixDestroy(&pixs2);
    pixDestroy(&pixs3);
    fpixDestroy(&fpixs);
    fpixDestroy(&fpixs2);
    fpixDestroy(&fpixs3);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
    pixDestroy(&pixt5);
    kernelDestroy(&kel);
    kernelDestroy(&kelx);
    kernelDestroy(&kely);
    return 0;
}
