/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*
 * pageseg_reg.c
 *
 *   This is a regresssion test for some of the page segmentation
 *   algorithms.  You can run some of these algorithms on any selected page
 *   image using prog/pagesegtest1.
 */

#include "allheaders.h"

int main(int    argc,
         char **argv)
{
l_int32       i, n, istable, score;
BOXA         *boxa;
PIX          *pixs, *pix1, *pix2, *pixhm, *pixtm, *pixtb, *pixdb;
PIXA         *pixadb;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
        return 1;

        /* Test the generic page segmentation */
    pixs = pixRead("pageseg1.tif");
    pixadb = pixaCreate(0);
    pixGetRegionsBinary(pixs, &pixhm, &pixtm, &pixtb, pixadb);
    pixDestroy(&pixhm);
    pixDestroy(&pixtm);
    pixDestroy(&pixtb);

    n = pixaGetCount(pixadb);
    for (i = 0; i < n; i++) {
        pix1 = pixaGetPix(pixadb, i, L_CLONE);
        regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 0 - 18 */
        pixDestroy(&pix1);
    }

        /* Display intermediate images and final segmentation */
    if (rp->display) {
        pix1 = pixaDisplayTiledAndScaled(pixadb, 32, 400, 4, 0, 20, 3);
        pixDisplay(pix1, 0, 0);
        pixDestroy(&pix1);
        pix1 = pixaGetPix(pixadb, 17, L_CLONE);
        pixDisplay(pix1, 580, 0);
        pixDestroy(&pix1);
        pix1 = pixaGetPix(pixadb, 18, L_CLONE);
        pixDisplay(pix1, 1220, 0);
        pixDestroy(&pix1);
    }
    pixaDestroy(&pixadb);

        /* Test the greedy rectangle finder for white space */
    pix1 = pixScale(pixs, 0.5, 0.5);
    pixFindLargeRectangles(pix1, 0, 20, &boxa, &pixdb);
    regTestWritePixAndCheck(rp, pixdb, IFF_PNG);  /* 19 */
    pixDisplayWithTitle(pixdb, 0, 700, NULL, rp->display);
    pixDestroy(&pixs);
    pixDestroy(&pix1);
    pixDestroy(&pixdb);
    boxaDestroy(&boxa);

        /* Test the table finder */
    pix1 = pixRead("table.15.tif");
    pixadb = pixaCreate(0);
    pixDecideIfTable(pix1, NULL, L_PORTRAIT_MODE, &score, pixadb);
    istable = (score >= 2) ? 1 : 0;
    regTestCompareValues(rp, 1.0, istable, 0.0);  /* 20 */
    pix2 = pixaDisplayTiledInRows(pixadb, 32, 2000, 1.0, 0, 30, 2);
    regTestWritePixAndCheck(rp, pix2, IFF_PNG);  /* 21 */
    pixDisplayWithTitle(pix2, 700, 700, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixaDestroy(&pixadb);

    pix1 = pixRead("table.27.tif");
    pixadb = pixaCreate(0);
    pixDecideIfTable(pix1, NULL, L_PORTRAIT_MODE, &score, pixadb);
    istable = (score >= 2) ? 1 : 0;
    regTestCompareValues(rp, 1.0, istable, 0.0);  /* 22 */
    pix2 = pixaDisplayTiledInRows(pixadb, 32, 2000, 1.0, 0, 30, 2);
    regTestWritePixAndCheck(rp, pix2, IFF_PNG);  /* 23 */
    pixDisplayWithTitle(pix2, 1000, 700, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixaDestroy(&pixadb);

    pix1 = pixRead("table.150.png");
    pixadb = pixaCreate(0);
    pixDecideIfTable(pix1, NULL, L_PORTRAIT_MODE, &score, pixadb);
    istable = (score >= 2) ? 1 : 0;
    regTestCompareValues(rp, 1.0, istable, 0.0);  /* 24 */
    pix2 = pixaDisplayTiledInRows(pixadb, 32, 2000, 1.0, 0, 30, 2);
    regTestWritePixAndCheck(rp, pix2, IFF_PNG);  /* 25 */
    pixDisplayWithTitle(pix2, 1300, 700, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixaDestroy(&pixadb);

    pix1 = pixRead("toc.99.tif");  /* not a table */
    pixadb = pixaCreate(0);
    pixDecideIfTable(pix1, NULL, L_PORTRAIT_MODE, &score, pixadb);
    istable = (score >= 2) ? 1 : 0;
    regTestCompareValues(rp, 0.0, istable, 0.0);  /* 26 */
    pix2 = pixaDisplayTiledInRows(pixadb, 32, 2000, 1.0, 0, 30, 2);
    regTestWritePixAndCheck(rp, pix2, IFF_PNG);  /* 27 */
    pixDisplayWithTitle(pix2, 1600, 700, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixaDestroy(&pixadb);

    return regTestCleanup(rp);
}

