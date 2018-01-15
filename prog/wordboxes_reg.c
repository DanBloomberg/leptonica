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
 * wordboxes_reg.c
 *
 *   This tests:
 *     - functions that make word boxes
 *     - the function that finds the nearest box to a given box in a boxa
 */

#include "allheaders.h"

void MakeWordBoxes1(PIX *pixs, l_int32 maxdil, L_REGPARAMS *rp);
void MakeWordBoxes2(PIX *pixs, l_int32 reduction, L_REGPARAMS  *rp);
void TestBoxaAdjacency(PIX *pixs, L_REGPARAMS  *rp);


int main(int    argc,
         char **argv)
{
BOXA         *boxa;
PIX          *pix1, *pix2, *pix3, *pix4;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
        return 1;

    pix1 = pixRead("lucasta.150.jpg");
    pix2 = pixConvertTo1(pix1, 140);  /* 150 ppi */
    pix3 = pixScale(pix2, 2.2, 2.2);   /* 300 ppi */
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pix1 = pixRead("zanotti-78.jpg");
    pix2 = pixConvertTo1(pix1, 128);  /* 150 ppi */
    pix4 = pixScale(pix2, 2.0, 2.0);   /* 300 ppi */
    pixDestroy(&pix1);
    pixDestroy(&pix2);

        /* Make word boxes using pixWordMaskByDilation() */
    MakeWordBoxes1(pix3, 20, rp);  /* 1 */
    MakeWordBoxes1(pix4, 20, rp);  /* 2 */

        /* Make word boxes using the higher-level functions
         * pixGetWordsInTextlines() and pixGetWordBoxesInTextlines() */
    MakeWordBoxes2(pix3, 1, rp);  /* 3, 4 */
    MakeWordBoxes2(pix4, 1, rp);  /* 5, 6 */

        /* Make word boxes using the higher-level functions
         * pixGetWordsInTextlines() and pixGetWordBoxesInTextlines() */
    MakeWordBoxes2(pix3, 2, rp);  /* 7, 8 */
    MakeWordBoxes2(pix4, 2, rp);  /* 9, 10 */

        /* Test boxa adjacency function */
    TestBoxaAdjacency(pix3, rp);

    pixDestroy(&pix3);
    pixDestroy(&pix4);
    return regTestCleanup(rp);
}

void
MakeWordBoxes1(PIX          *pixs,
               l_int32       maxdil,
               L_REGPARAMS  *rp)
{
BOXA  *boxa;
PIX   *pix1, *pixd;

    pixWordMaskByDilation(pixs, maxdil, &pix1, NULL);
    pixd = NULL;
    if (pix1) {
        boxa = pixConnComp(pix1, NULL, 8);
        pixd = pixConvertTo8(pixs, 1);
        pixRenderBoxaArb(pixd, boxa, 2, 255, 0, 0);
        boxaDestroy(&boxa);
    }
    regTestWritePixAndCheck(rp, pixd, IFF_PNG);
    pixDisplayWithTitle(pixd, 0, 100, NULL, rp->display);
    pixDestroy(&pix1);
    pixDestroy(&pixd);
}

void
MakeWordBoxes2(PIX          *pixs,
               l_int32       reduction,
               L_REGPARAMS  *rp)
{
l_int32  default_minwidth = 10;
l_int32  default_minheight = 10;
l_int32  default_maxwidth = 400;
l_int32  default_maxheight = 70;
l_int32  minwidth, minheight, maxwidth, maxheight;
BOXA    *boxa1, *boxa2;
NUMA    *na;
PIX     *pixd1, *pixd2;
PIXA    *pixa;

    minwidth = default_minwidth / reduction;
    minheight = default_minheight / reduction;
    maxwidth = default_maxwidth / reduction;
    maxheight = default_maxheight / reduction;

        /* Get the word boxes */
    pixGetWordsInTextlines(pixs, reduction, minwidth, minheight,
                           maxwidth, maxheight, &boxa1, &pixa, &na);
    pixaDestroy(&pixa);
    numaDestroy(&na);
    if (reduction == 1)
        boxa2 = boxaCopy(boxa1, L_CLONE);
    else
        boxa2 = boxaTransform(boxa1, 0, 0, 2.0, 2.0);
    pixd1 = pixConvertTo8(pixs, 1);
    pixRenderBoxaArb(pixd1, boxa2, 2, 255, 0, 0);
    regTestWritePixAndCheck(rp, pixd1, IFF_PNG);
    pixDisplayWithTitle(pixd1, 700, 100, NULL, rp->display);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);

        /* Do it again with this interface.  The result should be the same. */
    pixGetWordBoxesInTextlines(pixs, reduction, minwidth, minheight,
                               maxwidth, maxheight, &boxa1, NULL);
    if (reduction == 1)
        boxa2 = boxaCopy(boxa1, L_CLONE);
    else
        boxa2 = boxaTransform(boxa1, 0, 0, 2.0, 2.0);
    pixd2 = pixConvertTo8(pixs, 1);
    pixRenderBoxaArb(pixd2, boxa2, 2, 255, 0, 0);
    if (regTestComparePix(rp, pixd1, pixd2)) {
        L_ERROR("pix not the same", "MakeWordBoxes2");
        pixDisplayWithTitle(pixd2, 1200, 100, NULL, rp->display);
    }
    pixDestroy(&pixd1);
    pixDestroy(&pixd2);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);
}

void
TestBoxaAdjacency(PIX          *pixs,
                  L_REGPARAMS  *rp)
{
l_int32  i, j, k, n; 
BOX     *box1, *box2;
BOXA    *boxa0, *boxa1, *boxa2;
PIX     *pix1, *pix2, *pix3;
NUMAA   *naai, *naad;

        /* Make a word mask and remove small components */
    pixWordMaskByDilation(pixs, 20, &pix1, NULL);
    boxa0 = pixConnComp(pix1, NULL, 8);
    boxa1 = boxaSelectBySize(boxa0, 8, 8, L_SELECT_IF_BOTH,
                             L_SELECT_IF_GT, NULL);
    pix2 = pixConvertTo8(pixs, 1);
    pixRenderBoxaArb(pix2, boxa1, 2, 255, 0, 0);
    regTestWritePixAndCheck(rp, pix2, IFF_PNG);
    pixDisplayWithTitle(pix2, 600, 600, NULL, rp->display);
    pixDestroy(&pix1);

        /* Find the adjacent boxes and their distances */
    boxaFindNearestBoxes(boxa1, L_NON_NEGATIVE, 0, &naai, &naad);
    numaaWrite("/tmp/lept/regout/index.naa", naai);
    regTestCheckFile(rp, "/tmp/lept/regout/index.naa");
    numaaWrite("/tmp/lept/regout/dist.naa", naad);
    regTestCheckFile(rp, "/tmp/lept/regout/dist.naa");

        /* For a few boxes, show the (up to 4) adjacent boxes */
    n = boxaGetCount(boxa1);
    pix3 = pixConvertTo32(pixs);
    for (i = 10; i < n; i += 25) {
        box1 = boxaGetBox(boxa1, i, L_COPY);
        pixRenderBoxArb(pix3, box1, 2, 255, 0, 0);
        boxa2 = boxaCreate(4);
        for (j = 0; j < 4; j++) {
            numaaGetValue(naai, i, j, NULL, &k);
            if (k >= 0) {
                box2 = boxaGetBox(boxa1, k, L_COPY);
                boxaAddBox(boxa2, box2, L_INSERT);
            }
        }
        pixRenderBoxaArb(pix3, boxa2, 2, 0, 255, 0);
        boxDestroy(&box1);
        boxaDestroy(&boxa2);
    }
    regTestWritePixAndCheck(rp, pix3, IFF_PNG);
    pixDisplayWithTitle(pix3, 1100, 600, NULL, rp->display);
         
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    boxaDestroy(&boxa0);
    boxaDestroy(&boxa1);
    numaaDestroy(&naai);
    numaaDestroy(&naad);
} 

