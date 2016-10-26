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
 * ccthin2_reg.c
 *
 *   Tests the examples in pixThinExamples()
 */

#include "allheaders.h"

int main(int    argc,
         char **argv)
{
BOX          *box;
PIX          *pixs, *pix1;
PIXA         *pixa;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
        return 1;

    pix1 = pixRead("feyn.tif");
    box = boxCreate(683, 799, 970, 479);
    pixs = pixClipRectangle(pix1, box, NULL);
    pixDestroy(&pix1);
    boxDestroy(&box);

    pixa = pixaCreate(0);

    pix1 = pixThinExamples(pixs, L_THIN_FG, 1, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 0 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 2, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 1 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 3, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 2 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 4, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 3 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 5, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 4 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 6, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 5 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_FG, 7, 0, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 6 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_BG, 8, 5, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 7 */
    pixaAddPix(pixa, pix1, L_INSERT);
    pix1 = pixThinExamples(pixs, L_THIN_BG, 9, 5, NULL);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 8 */
    pixaAddPix(pixa, pix1, L_INSERT);

        /* Display the thinning results */
    pix1 = pixaDisplayTiledAndScaled(pixa, 8, 500, 1, 0, 25, 2);
    regTestWritePixAndCheck(rp, pix1, IFF_PNG);  /* 9 */
    if (rp->display) {
        lept_mkdir("lept/thin");
        pixDisplayWithTitle(pix1, 0, 0, NULL, rp->display);
        fprintf(stderr, "Writing to: /tmp/lept/thin/ccthin2.pdf");
        pixaConvertToPdf(pixa, 0, 1.0, 0, 0, "Thin 2 Results",
                         "/tmp/lept/thin/ccthin2.pdf");
    }
    pixDestroy(&pix1);
    pixDestroy(&pixs);
    pixaDestroy(&pixa);

    return regTestCleanup(rp);
}


