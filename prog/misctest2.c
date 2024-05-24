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
 *   misctest2.c
 *        * Page cropping with light filtering
 *        * Page cropping with removal of fg on left and right sides
 *        * Demonstrate image cleaning function
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#include "allheaders.h"

int main(int    argc,
         char **argv)
{
PIX   *pix1, *pix2, *pix3;
PIXA  *pixa1;

    setLeptDebugOK(1);
    lept_mkdir("lept/misc");

        /* Page cropping with light filtering */
    pix1 = pixRead("tel_3.tif");
    pix2 = pixCropImage(pix1, 30, 30, 4, 25, 25, 1.15,
                        "/tmp/lept/misc/crop_tel3.pdf", NULL);
    pixDestroy(&pix1);
    pixDestroy(&pix2);

        /* Page cropping with removal of fg on left and right sides */
    pix1 = pixRead("boismort.1.tif");
    pix2 = pixCropImage(pix1, 50, 50, -1, 70, 70, 1.1,
                        "/tmp/lept/misc/crop_bois1.pdf", NULL);
    pixDestroy(&pix1);
    pixDestroy(&pix2);

        /* Page cropping with removal of fg on left and right sides */
    pix1 = pixRead("boismort.15.tif");
    pix2 = pixCropImage(pix1, 50, 50, -1, 70, 70, 1.1,
                        "/tmp/lept/misc/crop_bois15.pdf", NULL);
    pixDestroy(&pix1);
    pixDestroy(&pix2);

        /* Page cleaning */
    pixa1 = pixaCreate(3);
    pix1 = pixRead("tel_3.tif");
    pix2 = pixRotate(pix1, 0.02, L_ROTATE_SAMPLING, L_BRING_IN_WHITE, 0, 0);
    pix3 = pixCleanImage(pix2, 1, 0, 1, 0);
    pixaAddPix(pixa1, pix3, L_INSERT);
    pixDisplay(pix3, 800, 800);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pix1 = pixRead("w91frag.jpg");
    pixaAddPix(pixa1, pixScale(pix1, 2.5, 2.5), L_INSERT);
    pix2 = pixRotate(pix1, 0.02, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    pix3 = pixCleanImage(pix2, 1, 0, 1, 0);
    pixaAddPix(pixa1, pixScale(pix3, 2.5, 2.5), L_INSERT);
    pixDisplay(pix3, 1200, 800);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    lept_stderr("Writing /tmp/lept/misc/pageclean.pdf\n");
    pixaConvertToPdf(pixa1, 0, 1.0, L_DEFAULT_ENCODE, 50, NULL,
                     "/tmp/lept/misc/pageclean.pdf");
    pixaDestroy(&pixa1);
    
    return 0;
}
