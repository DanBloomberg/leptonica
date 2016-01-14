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
 *   dewarp_reg.c
 *
 *     Regression test for image dewarp based on text lines
 *
 *     We also test some of the fpix and dpix functions (scaling,
 *     serialization, interconversion)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


l_int32 main(int    argc,
             char **argv)
{
l_int32    i, n, index, display, success;
l_float32  a, b, c;
L_DEWARP  *dew, *dew2;
DPIX      *dpix1, *dpix2, *dpix3;
FILE      *fp;
FPIX      *fpix1, *fpix2, *fpix3;
NUMA      *nax, *nafit;
PIX       *pixs, *pixn, *pixg, *pixb, *pixt1, *pixt2;
PIX       *pixs2, *pixn2, *pixg2, *pixb2;
PTA       *pta, *ptad;
PTAA      *ptaa1, *ptaa2;

    if (regTestSetup(argc, argv, &fp, &display, &success, NULL))
              return 1;

    pixs = pixRead("1555-7.jpg");
    
        /* Normalize for varying background and binarize */
    pixn = pixBackgroundNormSimple(pixs, NULL, NULL);
    pixg = pixConvertRGBToGray(pixn, 0.5, 0.3, 0.2);
    pixb = pixThresholdToBinary(pixg, 130);
    pixDestroy(&pixn);
    pixDestroy(&pixg);
    pixWrite("/tmp/dewarp.0.png", pixb, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.0.png", 0, &success);
    pixDisplayWithTitle(pixb, 0, 0, "binarized input", display);

        /* Get the textline centers */
    ptaa1 = pixGetTextlineCenters(pixb, 0);
    pixt1 = pixCreateTemplate(pixs);
    pixt2 = pixDisplayPtaa(pixt1, ptaa1);
    pixWrite("/tmp/dewarp.1.png", pixt2, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.1.png", 1, &success);
    pixDisplayWithTitle(pixt2, 0, 500, "textline centers", display);
    pixDestroy(&pixt1);

        /* Remove short lines */
    ptaa2 = ptaaRemoveShortLines(pixb, ptaa1, 0.8, 0);

        /* Fit to quadratic */
    n = ptaaGetCount(ptaa2);
    for (i = 0; i < n; i++) {
        pta = ptaaGetPta(ptaa2, i, L_CLONE);
        ptaGetArrays(pta, &nax, NULL);
        ptaGetQuadraticLSF(pta, &a, &b, &c, &nafit);
        ptad = ptaCreateFromNuma(nax, nafit);
        pixDisplayPta(pixt2, pixt2, ptad);
        ptaDestroy(&pta);
        ptaDestroy(&ptad);
        numaDestroy(&nax);
        numaDestroy(&nafit);
    }
    pixWrite("/tmp/dewarp.2.png", pixt2, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.2.png", 2, &success);
    pixDisplayWithTitle(pixt2, 300, 500, "fitted lines superimposed", display);
    ptaaDestroy(&ptaa1);
    ptaaDestroy(&ptaa2);
    pixDestroy(&pixt2);

        /* Run with only vertical disparity correction */
    dew = dewarpCreate(pixb, 7, 30, 15, 0);
    dewarpBuildModel(dew, 0);
    dewarpApplyDisparity(dew, pixb, 0);
    pixWrite("/tmp/dewarp.3.png", dew->pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.3.png", 3, &success);
    pixDisplayWithTitle(dew->pixd, 400, 0, "fixed for vert disparity",
                        display);
    dewarpDestroy(&dew);

        /* Run with both vertical and horizontal disparity correction */
    dew = dewarpCreate(pixb, 7, 30, 15, 1);
    dewarpBuildModel(dew, 0);
    dewarpApplyDisparity(dew, pixb, 0);
    pixWrite("/tmp/dewarp.4.png", dew->pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.4.png", 4, &success);
    pixDisplayWithTitle(dew->pixd, 800, 0, "fixed for both disparities",
                        display);

        /* Read another image, normalize background and binarize */
    pixs2 = pixRead("1555-3.jpg");
    pixn2 = pixBackgroundNormSimple(pixs2, NULL, NULL);
    pixg2 = pixConvertRGBToGray(pixn2, 0.5, 0.3, 0.2);
    pixb2 = pixThresholdToBinary(pixg2, 130);
    pixDestroy(&pixn2);
    pixDestroy(&pixg2);
    pixWrite("/tmp/dewarp.5.png", pixb, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.5.png", 5, &success);
    pixDisplayWithTitle(pixb, 0, 400, "binarized input (2)", display);

        /* Minimize and re-apply previous disparity to this image */
    dewarpMinimize(dew);
    dewarpApplyDisparity(dew, pixb2, 0);
    pixWrite("/tmp/dewarp.6.png", dew->pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.6.png", 6, &success);
    pixDisplayWithTitle(dew->pixd, 400, 400, "fixed (2) for both disparities",
                        display);

        /* Write and read back minimized dewarp struct */
    dewarpWrite("/tmp/dewarp.1.dew", dew);
    regTestCheckFile(fp, argv, "/tmp/dewarp.1.dew", 7, &success);
    dew2 = dewarpRead("/tmp/dewarp.1.dew");
    dewarpWrite("/tmp/dewarp.2.dew", dew2);
    regTestCheckFile(fp, argv, "/tmp/dewarp.2.dew", 8, &success);
    regTestCompareFiles(fp, argv, 7, 8, &success);

        /* Apply dew2 to pixb2 */
    dewarpApplyDisparity(dew2, pixb2, 0);
    pixWrite("/tmp/dewarp.9.png", dew2->pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.9.png", 9, &success);
    pixDisplayWithTitle(dew->pixd, 800, 400, "fixed (3) for both disparities",
                        display);

        /* Minimize, repopulate disparity arrays, and apply again */
    dewarpMinimize(dew2);
    dewarpApplyDisparity(dew2, pixb2, 0);
    pixWrite("/tmp/dewarp.10.png", dew2->pixd, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.10.png", 10, &success);
    regTestCompareFiles(fp, argv, 9, 10, &success);
    pixDisplayWithTitle(dew->pixd, 900, 400, "fixed (4) for both disparities",
                        display);

        /* Test a few of the fpix functions */
    fpix1 = fpixClone(dew->sampvdispar);
    fpixWrite("/tmp/sampv.1.fpix", fpix1);
    regTestCheckFile(fp, argv, "/tmp/sampv.1.fpix", 11, &success);
    fpix2 = fpixRead("/tmp/sampv.1.fpix");
    fpixWrite("/tmp/sampv.2.fpix", fpix2);
    regTestCheckFile(fp, argv, "/tmp/sampv.2.fpix", 12, &success);
    regTestCompareFiles(fp, argv, 11, 12, &success);
    fpix3 = fpixScaleByInteger(fpix2, 30);
    pixt1 = fpixRenderContours(fpix3, -2., 2.0, 0.2);
    pixWrite("/tmp/dewarp.13.png", pixt1, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.13.png", 13, &success);
    pixDisplayWithTitle(pixt1, 0, 800, "v. disparity contours", display);
    fpixDestroy(&fpix1);
    fpixDestroy(&fpix2);
    fpixDestroy(&fpix3);
    pixDestroy(&pixt1);

        /* Test a few of the dpix functions */
    dpix1 = fpixConvertToDPix(dew->sampvdispar);
    dpixWrite("/tmp/sampv.1.dpix", dpix1);
    regTestCheckFile(fp, argv, "/tmp/sampv.1.dpix", 14, &success);
    dpix2 = dpixRead("/tmp/sampv.1.dpix");
    dpixWrite("/tmp/sampv.2.dpix", dpix2);
    regTestCheckFile(fp, argv, "/tmp/sampv.2.dpix", 15, &success);
    regTestCompareFiles(fp, argv, 14, 15, &success);
    dpix3 = dpixScaleByInteger(dpix2, 30);
    fpix3 = dpixConvertToFPix(dpix3);
    pixt1 = fpixRenderContours(fpix3, -2., 2.0, 0.2);
    pixWrite("/tmp/dewarp.16.png", pixt1, IFF_PNG);
    regTestCheckFile(fp, argv, "/tmp/dewarp.16.png", 16, &success);
    pixDisplayWithTitle(pixt1, 400, 800, "v. disparity contours", display);
    regTestCompareFiles(fp, argv, 13, 16, &success);
    dpixDestroy(&dpix1);
    dpixDestroy(&dpix2);
    dpixDestroy(&dpix3);
    fpixDestroy(&fpix3);
    pixDestroy(&pixt1);

    dewarpDestroy(&dew);
    dewarpDestroy(&dew2);
    pixDestroy(&pixs);
    pixDestroy(&pixb);
    pixDestroy(&pixs2);
    pixDestroy(&pixb2);
    regTestCleanup(argc, argv, fp, success, NULL);
    return 0;
}


