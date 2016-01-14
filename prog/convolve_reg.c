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
 *  convolve_reg.c
 *
 *    Tests a number of convolution functions.
 */

#include "allheaders.h"

static const char  *kdatastr = " 20    50   80  50   20 "
                               " 50   100  140  100  50 "
                               " 90   160  200  160  90 "
                               " 50   100  140  100  50 "
                               " 20    50   80   50  20 ";


main(int    argc,
     char **argv)
{
l_int32       i, j, sizex, sizey, count, success, display;
FILE         *fp;
FPIX         *fpixv, *fpixrv;
L_KERNEL     *kel1, *kel2;
PIX          *pixs, *pixacc, *pixg, *pixt, *pixd;
PIX          *pixb, *pixm, *pixms, *pixrv, *pix1, *pix2, *pix3, *pix4;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &fp, &display, &success, &rp))
	return 1;

    count = 0;

        /* Test pixBlockconvGray() on 8 bpp */
    pixs = pixRead("test8.jpg");
    pixacc = pixBlockconvAccum(pixs);
    pixd = pixBlockconvGray(pixs, pixacc, 3, 5);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 100, 0, NULL, display);
    pixDestroy(&pixacc);
    pixDestroy(&pixd);

        /* Test pixBlockconv() on 8 bpp */
    pixd = pixBlockconv(pixs, 9, 8);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 200, 0, NULL, display);
    pixDestroy(&pixd);
    pixDestroy(&pixs);

        /* Test pixBlockrank() on 1 bpp */
    pixs = pixRead("test1.png");
    pixacc = pixBlockconvAccum(pixs);
    for (i = 0; i < 3; i++) {
        pixd = pixBlockrank(pixs, pixacc, 4, 4, 0.25 + 0.25 * i);
        regTestWritePixAndCheck(pixd, IFF_PNG, &count, rp);
        pixDisplayWithTitle(pixd, 300 + 100 * i, 0, NULL, display);
        pixDestroy(&pixd);
    }

        /* Test pixBlocksum() on 1 bpp */
    pixd = pixBlocksum(pixs, pixacc, 16, 16);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 700, 0, NULL, display);
    pixDestroy(&pixd);
    pixDestroy(&pixacc);
    pixDestroy(&pixs);

        /* Test pixCensusTransform() */
    pixs = pixRead("test24.jpg");
    pixg = pixScaleRGBToGrayFast(pixs, 2, COLOR_GREEN);
    pixd = pixCensusTransform(pixg, 10, NULL);
    regTestWritePixAndCheck(pixd, IFF_PNG, &count, rp);
    pixDisplayWithTitle(pixd, 800, 0, NULL, display);
    pixDestroy(&pixd);

        /* Test generic convolution with kel1 */
    kel1 = kernelCreateFromString(5, 5, 2, 2, kdatastr);
    pixd = pixConvolve(pixg, kel1, 8, 1);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 100, 500, NULL, display);
    pixDestroy(&pixd);

        /* Test convolution with flat rectangular kel */
    kel2 = kernelCreate(11, 11);
    kernelSetOrigin(kel2, 5, 5);
    for (i = 0; i < 11; i++) {
        for (j = 0; j < 11; j++)
            kernelSetElement(kel2, i, j, 1);
    }
    pixd = pixConvolve(pixg, kel2, 8, 1);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 200, 500, NULL, display);
    pixDestroy(&pixd);
    kernelDestroy(&kel1);
    kernelDestroy(&kel2);

        /* Test pixBlockconv() on 32 bpp */
    pixt = pixScaleBySampling(pixs, 0.5, 0.5);
    pixd = pixBlockconv(pixt, 4, 6);
    regTestWritePixAndCheck(pixd, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixd, 300, 500, NULL, display);
    pixDestroy(&pixt);
    pixDestroy(&pixd);
    pixDestroy(&pixs);
    pixDestroy(&pixg);

        /* Test pixWindowedMean() and pixWindowedMeanSquare() on 8 bpp */
    pixs = pixRead("feyn-fract2.tif");
    pixg = pixConvertTo8(pixs, 0);
    sizex = 5;
    sizey = 20;
    pixb = pixAddBorderGeneral(pixg, sizex + 1, sizex + 1,
                               sizey + 1, sizey + 1, 0);
    pixm = pixWindowedMean(pixb, sizex, sizey, 1);
    pixms = pixWindowedMeanSquare(pixb, sizex, sizey);
    regTestWritePixAndCheck(pixm, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixm, 100, 0, NULL, display);
    pixDestroy(&pixs);
    pixDestroy(&pixb);

        /* Test pixWindowedVariance() on 8 bpp */
    pixWindowedVariance(pixm, pixms, &fpixv, &fpixrv);
    pixrv = fpixConvertToPix(fpixrv, 8, L_CLIP_TO_ZERO, 1);
    regTestWritePixAndCheck(pixrv, IFF_JFIF_JPEG, &count, rp);
    pixDisplayWithTitle(pixrv, 100, 250, NULL, display);
    pix1 = fpixDisplayMaxDynamicRange(fpixv);
    pix2 = fpixDisplayMaxDynamicRange(fpixrv);
    pixDisplayWithTitle(pix1, 100, 500, "Variance", display);
    pixDisplayWithTitle(pix2, 100, 750, "RMS deviation", display);
    regTestWritePixAndCheck(pix1, IFF_JFIF_JPEG, &count, rp);
    regTestWritePixAndCheck(pix2, IFF_JFIF_JPEG, &count, rp);
    fpixDestroy(&fpixv);
    fpixDestroy(&fpixrv);
    pixDestroy(&pixm);
    pixDestroy(&pixms);
    pixDestroy(&pixrv);

        /* Test again all windowed functions with simpler interface */
    pixWindowedStats(pixg, sizex, sizey, NULL, NULL, &fpixv, &fpixrv);
    pix3 = fpixDisplayMaxDynamicRange(fpixv);
    pix4 = fpixDisplayMaxDynamicRange(fpixrv);
    regTestComparePix(fp, argv, pix1, pix3, 0, &success);
    regTestComparePix(fp, argv, pix2, pix4, 0, &success);
    pixDestroy(&pixg);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    pixDestroy(&pix4);
    fpixDestroy(&fpixv);
    fpixDestroy(&fpixrv);

    regTestCleanup(argc, argv, fp, success, rp);
    return 0;
}
