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
 * baselinetest.c
 *
 *   This tests two things:
 *   (1) The ability to find a projective transform that will deskew
 *       textlines in an image with keystoning.
 *   (2) The ability to find baselines in a text image.
 */

#include "allheaders.h"

int main(int    argc,
         char **argv)
{
NUMA         *na;
PIX          *pixs, *pixd;
PTA          *pta;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
        return 1;

    pixs = pixRead("keystone.png");

        /* Test function for deskewing using projective transform
	 * on linear approximation for local skew angle */
    pixd = pixDeskewLocal(pixs, 10, 0, 0, 0.0, 0.0, 0.0);
    regTestWritePixAndCheck(rp, pixd, IFF_PNG);  /* 0 */

        /* Test baseline finder */
    na = pixFindBaselines(pixd, &pta, 1);
    regTestCheckFile(rp, "/tmp/lept/baseline/diff.png");  /* 1 */
    regTestCheckFile(rp, "/tmp/lept/baseline/loc.png");  /* 2 */
    regTestCheckFile(rp, "/tmp/lept/baseline/baselines.png");  /* 3 */
    if (rp->display) {
        l_fileDisplay("/tmp/lept/baseline/diff.png", 0, 0, 1.0);
        l_fileDisplay("/tmp/lept/baseline/loc.png", 700, 0, 1.0);
        l_fileDisplay("/tmp/lept/baseline/baselines.png", 1350, 0, 1.0);
    }
    pixDestroy(&pixd);
    numaDestroy(&na);
    ptaDestroy(&pta);

        /* Test function for finding local skew angles */
    na = pixGetLocalSkewAngles(pixs, 10, 0, 0, 0.0, 0.0, 0.0, NULL, NULL, 1);
    gplotSimple1(na, GPLOT_PNG, "/tmp/lept/baseline/ang", "Angles in degrees");
    regTestCheckFile(rp, "/tmp/lept/baseline/ang.png");  /* 4 */
    regTestCheckFile(rp, "/tmp/lept/baseline/skew.png");  /* 5 */
    if (rp->display) {
        l_fileDisplay("/tmp/lept/baseline/ang.png", 0, 550, 1.0);
        l_fileDisplay("/tmp/lept/baseline/skew.png", 700, 550, 1.0);
    }
    numaDestroy(&na);
    pixDestroy(&pixs);

    return regTestCleanup(rp);
}



