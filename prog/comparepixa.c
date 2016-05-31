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
 *  comparepixa.c
 *
 *     comparepixa file1 file2 nx ny tw spacing border fontsize fileout
 *
 *   This reads two pixa or pixacomp from files and renders them
 *   interleaved, side-by-side in a pdf.  A warning is issued if the
 *   input image arrays have different lengths.
 *
 *   The integers nx and ny specify how many side-by-side pairs
 *   are displayed on each pdf page.  For example, if nx = 1 and ny = 2,
 *   then two pairs are shown, one above the other.
 *
 *   The input pix are scaled to tw, the target width, then paired
 *   up with %spacing and an optional %border.
 *
 *   The pairs are then mosaiced, depending on %nx and %ny, into
 *   a set of larger images.  The %spacing and %border parameters
 *   are used here as well.   To label each pair with the index from
 *   the input arrays, choose fontsize in {4, 6, 8, 10, 12, 14, 16, 18, 20}.
 *   To skip labelling, set %fontsize = 0.
 *
 *   This set of images is rendered into a pdf and written to %fileont.
 *
 *   Typical numbers for the input parameters are:
 *      %nx = small integer (1 - 4)
 *      %ny = 2 * %nx
 *      %tw = 200 - 500 pixels
 *      %spacing = 10
 *      %border = 2
 *      %fontsize = 10
 */

#include <string.h>
#include "allheaders.h"

    /* Static helper */
SARRAY *SarrayGenerateIntegers(l_int32  n);


int main(int    argc,
         char **argv)
{
l_int32      nx, ny, tw, spacing, border, fontsize, npairs;
PIXA        *pixa1, *pixa2, *pixa3, *pixa4, *pixa5;
SARRAY      *sa;
static char  mainName[] = "comparepixa";

    if (argc != 10) {
        fprintf(stderr, "Syntax error in comparepixa:\n"
           "   comparepixa file1 file2 nx ny tw spacing border"
           " fontsize fileout\n");
        return 1;
    }

        /* Input files can be either pixa or pixacomp */
    if ((pixa1 = pixaReadBoth(argv[1])) == NULL)
        return ERROR_INT("pixa1 not read", mainName, 1);
    if ((pixa2 = pixaReadBoth(argv[2])) == NULL)
        return ERROR_INT("pixa2 not read", mainName, 1);
    if ((pixa3 = pixaInterleave(pixa1, pixa2, L_CLONE)) == NULL)
        return ERROR_INT("pixa3 not made", mainName, 1);
    pixaDestroy(&pixa1);
    pixaDestroy(&pixa2);

        /* Pair the images up side/by/side */
    nx = atoi(argv[3]);
    ny = atoi(argv[4]);
    tw = atoi(argv[5]);
    spacing = atoi(argv[6]);
    border = atoi(argv[7]);
    fontsize = atoi(argv[8]);
    pixa4 = pixaConvertToNUpPixa(pixa3, NULL, 2, 1, tw, spacing, border, 0);
    pixaDestroy(&pixa3);

        /* Mosaic the pairs */
    npairs = pixaGetCount(pixa4);
    sa = (fontsize > 0) ? SarrayGenerateIntegers(npairs) : NULL;
    pixa5 = pixaConvertToNUpPixa(pixa4, sa, nx, ny, 2 * tw,
                                 spacing, border, fontsize);
    pixaDestroy(&pixa4);
    sarrayDestroy(&sa);

        /* Output as pdf */
    pixaConvertToPdf(pixa5, 0, 1.0, 0, 0, NULL, argv[9]);
    pixaDestroy(&pixa5);
    return 0;
}


SARRAY *
SarrayGenerateIntegers(l_int32  n)
{
char     buf[32];
l_int32  i;
SARRAY  *sa;

    sa = sarrayCreate(n);
    for (i = 0; i < n; i++) {
        snprintf(buf, sizeof(buf), "%d", i);
        sarrayAddString(sa, buf, L_COPY);
    }
    return sa;
}

