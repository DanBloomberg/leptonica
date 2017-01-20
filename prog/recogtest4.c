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
 *  recogtest4.c
 *
 *     Test splitting characters
 */

#include "string.h"
#include "allheaders.h"

l_int32 main(int    argc,
             char **argv)
{
char      *charstr;
l_int32    i, j, n, index, w, h, debug;
l_float32  score;
BOX       *box;
BOXA      *boxa;
NUMA      *naindex, *nascore;
PIX       *pixs, *pix1, *pixdb;
PIXA      *pixas, *pixap, *pixa;
L_RECOG   *recog;
SARRAY    *sachar;

    if (argc != 1) {
        fprintf(stderr, " Syntax: recogtest4\n");
        return 1;
    }

#if 1
    pixas = pixaRead("recogsets/train08.pa");
    pixap = pixaRead("recogsets/problem08.pa");
#elif 0
    pixas = pixaRead("recogsets/train_modern2.pa");
    pixap = pixaRead("recogsets/problem_modern2.pa");
#elif 1
    pixas = pixaRead("recogsets/lord.train.pa");
    pixap = pixaRead("recogsets/lord.problem.pa");
#endif

        /* Set up recog with averaged templates */
    recog = recogCreateFromPixa(pixas, 0, 0, 0, 128, 1);
    recogAverageSamples(recog, 1);  /* required for splitting */

#if 1
        /* Do one character */
    fprintf(stderr, "One character\n");
    pixs = pixaGetPix(pixap, 1, L_CLONE);
    recogCorrelationBestChar(recog, pixs, &box, &score, &index, &charstr,
                             &pix1);
    pixDisplay(pix1, 100, 800);
    boxDestroy(&box);
    lept_free(charstr);
    pixDestroy(&pixs);
    pixDestroy(&pix1);
#endif

#if 1
        /* Do one set of characters */
    fprintf(stderr, "One set of characters\n");
    n = pixaGetCount(pixap);
    pixs = pixaGetPix(pixap, 0, L_CLONE);
    pixDisplay(pixs, 100, 100);
    recogCorrelationBestRow(recog, pixs, &boxa, &nascore, &naindex, &sachar, 1);
    boxaWriteStream(stderr, boxa);
    numaWriteStream(stderr, nascore);
    pixDestroy(&pixs);
    boxaDestroy(&boxa);
    numaDestroy(&nascore);
    numaDestroy(&naindex);
    sarrayDestroy(&sachar);
#endif

#if 1
        /* Do each set of characters */
    fprintf(stderr, "Each set of characters\n");
    n = pixaGetCount(pixap);
    for (i = 0; i < n; i++) {
        if (i > 0) continue;
        pixs = pixaGetPix(pixap, i, L_CLONE);
        recogCorrelationBestRow(recog, pixs, &boxa, &nascore,
                                &naindex, &sachar, 1);
        boxaWriteStream(stderr, boxa);
        numaWriteStream(stderr, nascore);
        pixDestroy(&pixs);
        boxaDestroy(&boxa);
        numaDestroy(&nascore);
        numaDestroy(&naindex);
        sarrayDestroy(&sachar);
    }
#endif

#if 1
        /* Use the top-level call for each set of characters */
        /* Test 19: images 1 and 6 are interesting */
        /* Test modern1: image 2 */
        /* Test modern1: image 6 (just the Zw part especially) */
        /* Test modern1: modern-frag2.png has one component to be matched */
    debug = 1;
    n = pixaGetCount(pixap);
    fprintf(stderr, "n = %d\n", n);
    for (i = 0; i < n; i++) {
#if 0
        pixs = pixRead("modern-frag2.png");
#else
        pixs = pixaGetPix(pixap, i, L_CLONE);
#endif
        pixDisplay(pixs, 100, 800);
        if (debug) {
            recogIdentifyMultiple(recog, pixs, 0, -1, -1, 0,
                                  &boxa, NULL, &pixdb, 1);
            pixDisplay(pixdb, 300, 500);
            boxaWriteStream(stderr, boxa);
            rchaExtract(recog->rcha, NULL, &nascore, NULL, NULL, NULL,
                        NULL, NULL);
            numaWriteStream(stderr, nascore);
            numaDestroy(&nascore);
            pixDestroy(&pixdb);
        } else {
            recogIdentifyMultiple(recog, pixs, 0, -1, -1, 0,
                                  &boxa, NULL, NULL, 0);
        }
        pixDestroy(&pixs);
        boxaDestroy(&boxa);
    }
    if (debug) {
        pix1 = pixaDisplayTiledInRows(recog->pixadb_split, 1, 200,
                                      1.0, 0, 20, 3);
        pixDisplay(pix1, 0, 0);
        pixDestroy(&pix1);
    }
#endif

    recogShowContent(stderr, recog, 1, 1);

    recogDestroy(&recog);
    pixaDestroy(&pixap);
    pixaDestroy(&pixas);
    return 0;
}
