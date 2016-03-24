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
 * dithertest.c
 *
 *    Input is 8 bpp grayscale
 */

#include "allheaders.h"

static const l_float32  GAMMA = 1.0;

int main(int    argc,
         char **argv)
{
char        *filein;
FILE        *fp;
PIX         *pix, *pixs, *pixd;
PIXCMAP     *cmap;
static char  mainName[] = "dithertest";

    if (argc != 2)
        return ERROR_INT(" Syntax:  dithertest filein", mainName, 1);

    filein = argv[1];

    lept_mkdir("lept/dither");

    if ((pix = pixRead(filein)) == NULL)
        return ERROR_INT("pix not made", mainName, 1);
    if (pixGetDepth(pix) != 8)
        return ERROR_INT("pix not 8 bpp", mainName, 1);
    pixs = pixGammaTRC(NULL, pix, GAMMA, 0, 255);

    startTimer();
    pixd = pixDitherToBinary(pixs);
    fprintf(stderr, " time for binarized dither = %7.3f sec\n", stopTimer());
    pixDisplayWrite(pixd, 1);
    pixDestroy(&pixd);

         /* Dither to 2 bpp, with colormap */
    startTimer();
    pixd = pixDitherTo2bpp(pixs, 1);
    fprintf(stderr, " time for dither = %7.3f sec\n", stopTimer());
    pixDisplayWrite(pixd, 1);
    cmap = pixGetColormap(pixd);
    pixcmapWriteStream(stderr, cmap);
    pixDestroy(&pixd);

         /* Dither to 2 bpp, without colormap */
    startTimer();
    pixd = pixDitherTo2bpp(pixs, 0);
    fprintf(stderr, " time for dither = %7.3f sec\n", stopTimer());
    pixDisplayWrite(pixd, 1);
    pixDestroy(&pixd);

        /* Dither 2x upscale to 1 bpp */
    startTimer();
    pixd = pixScaleGray2xLIDither(pixs);
    fprintf(stderr, " time for scale/dither = %7.3f sec\n", stopTimer());
    pixDisplayWrite(pixd, 1);
    pixDestroy(&pixd);

        /* Dither 4x upscale to 1 bpp */
    startTimer();
    pixd = pixScaleGray4xLIDither(pixs);
    fprintf(stderr, " time for scale/dither = %7.3f sec\n", stopTimer());
    pixDisplayWrite(pixd, 1);
    pixDestroy(&pixd);

    fprintf(stderr, "Writing to: /tmp/lept/dither/dither.pdf");
    pixDisplayMultiple(150, 1.0, "/tmp/lept/dither/dither.pdf");
    pixDestroy(&pix);
    pixDestroy(&pixs);
    return 0;
}

