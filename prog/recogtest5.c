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
 *  recogtest5.c
 *
 *     Test document image decoding (DID) approach to splitting characters
 */

#include "string.h"
#include "allheaders.h"

static PIX *GetBigComponent(PIX *pixs);


l_int32 main(int    argc,
             char **argv)
{
char       buf[256];
l_int32    i, n, item;
l_int32    example[6] = {17, 20, 21, 22, 23, 24};  /* for decoding */
PIX       *pix1, *pix2, *pixdb;
PIXA      *pixa1, *pixa2;
L_RECOG   *recog;

    if (argc != 1) {
        fprintf(stderr, " Syntax: recogtest5\n");
        return 1;
    }

    lept_mkdir("lept/recog");

        /* Generate the recognizer */
    pixa1 = pixaRead("recog/sets/train01.pa");
    recog = recogCreateFromPixa(pixa1, 0, 40, 0, 128, 1);
    recogAverageSamples(recog, 1);
    recogWrite("/tmp/lept/recog/rec1.rec", recog);

        /* Show the templates */
    recogDebugAverages(recog, 1);
    recogShowMatchesInRange(recog, recog->pixa_tr, 0.0, 1.0, 1);

        /* Get a set of problem images to decode */
    pixa2 = pixaRead("recog/sets/test01.pa");

        /* Decode a subset of them */
    for (i = 0; i < 6; i++) {
        item = example[i];
        pix1 = pixaGetPix(pixa2, item, L_CLONE);
        pixDisplay(pix1, 100, 100);
        pix2 = GetBigComponent(pix1);
        recogDecode(recog, pix2, 2, &pixdb);
        pixDisplay(pixdb, 300, 100);
        snprintf(buf, sizeof(buf), "/tmp/lept/recog/did-%d.png", item);
        pixWrite(buf, pixdb, IFF_PNG);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pixdb);
    }

    pixaDestroy(&pixa1);
    pixaDestroy(&pixa2);
    recogDestroy(&recog);
    return 0;
}

static PIX *
GetBigComponent(PIX  *pixs)
{
BOX  *box;
PIX  *pix1, *pixd;

    pix1 = pixMorphSequence(pixs, "c40.7 + o20.15 + d25.1", 0);
    pixClipToForeground(pix1, NULL, &box);
    pixd = pixClipRectangle(pixs, box, NULL);
    pixDestroy(&pix1);
    boxDestroy(&box);
    return pixd;
}


