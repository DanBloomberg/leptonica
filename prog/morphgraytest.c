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
 * morphgraytest.c
 *
 *      (1) Tests basic grayscale morphology, including speed
 *
 *      (2) Tests composite operations: tophat and hdome
 *           (for these, use the image aneurisms8.jpg)
 *
 *      (3) Tests the interpreter for grayscale morphology, as
 *          given in morphseq.c
 *
 *      (4) Tests duality for grayscale erode/dilate, open/close,
 *          and black/white tophat
 *
 *      (5) Demonstrates closing plus white tophat.  Note that this
 *          combination of operations can be quite useful.
 *      
 *      (6) Demonstrates a method of doing contrast enhancement
 *          by taking 3 * pixs and subtracting from this the
 *          closing and opening of pixs.  Do this both with the
 *          basic pix accumulation functions and with the cleaner
 *          Pixacc wrapper.   Verify the results are equivalent.
 *
 *      (7) Show how to extract the feynman diagrams from the stamp,
 *          using the tophat.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define     WSIZE              7
#define     HSIZE              7
#define     BUF_SIZE           512


static void pixCompare(PIX *pix, PIX *pix2, char *msg1, char *msg2);


main(int    argc,
     char **argv)
{
char           *filein, *fileout;
char            dilateseq[BUF_SIZE], erodeseq[BUF_SIZE];
char            openseq[BUF_SIZE], closeseq[BUF_SIZE];
char            wtophatseq[BUF_SIZE], btophatseq[BUF_SIZE];
l_int32         w, h;
PIX            *pixs, *pixt, *pixt2, *pixt3, *pixt3a, *pixt4;
PIX            *pixg, *pixd, *pixd1, *pixd2, *pixd3;
PIXACC         *pacc;
PIXCMAP        *cmap;
static char     mainName[] = "morphgraytest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  morphgraytest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);

    /* ---------- Choose an operation ----------  */
#if 0
    pixd = pixDilateGray(pixs, WSIZE, HSIZE);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#elif 0
    pixd = pixErodeGray(pixs, WSIZE, HSIZE);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#elif 0
    pixd = pixOpenGray(pixs, WSIZE, HSIZE);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#elif 0
    pixd = pixCloseGray(pixs, WSIZE, HSIZE);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#elif 0
    pixd = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_WHITE);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#elif 0
    pixd = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_BLACK);
    pixWrite(fileout, pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
#endif


    /* ---------- Speed ----------  */
#if 0 
    startTimer();
    pixd = pixCloseGray(pixs, WSIZE, HSIZE);
    fprintf(stderr, " Speed is %6.2f MPix/sec\n",
          (l_float32)(4 * w * h) / (1000000. * stopTimer()));
    pixWrite(fileout, pixd, IFF_PNG);
#endif

    /* -------- Test gray morph, including interpreter ------------ */
#if 0 
    pixDisplay(pixs, 350, 0);
    pixd = pixDilateGray(pixs, WSIZE, HSIZE);
    sprintf(dilateseq, "D%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, dilateseq, 50, 0);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);
    pixDestroy(&pixd);

    pixd = pixErodeGray(pixs, WSIZE, HSIZE);
    sprintf(erodeseq, "E%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, erodeseq, 50, 100);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);
    pixDestroy(&pixd);

    pixd = pixOpenGray(pixs, WSIZE, HSIZE);
    sprintf(openseq, "O%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, openseq, 50, 200);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);
    pixDestroy(&pixd);

    pixd = pixCloseGray(pixs, WSIZE, HSIZE);
    sprintf(closeseq, "C%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, closeseq, 50, 300);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);
    pixDestroy(&pixd);

    pixd = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_WHITE);
    sprintf(wtophatseq, "Tw%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, wtophatseq, 50, 400);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);
    pixDestroy(&pixd);

    pixd = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_BLACK);
    sprintf(btophatseq, "Tb%d.%d", WSIZE, HSIZE);
    pixg = pixGrayMorphSequence(pixs, btophatseq, 50, 500);
    pixCompare(pixd, pixg, "results are the same", "results are different" );
    pixDestroy(&pixg);

    pixWrite(fileout, pixd, IFF_PNG);
    pixDestroy(&pixd);
#endif

    /* ------------- Test erode/dilate duality -------------- */
#if 0
    pixd = pixDilateGray(pixs, WSIZE, HSIZE);
    pixInvert(pixs, pixs);
    pixd2 = pixErodeGray(pixs, WSIZE, HSIZE);
    pixInvert(pixd2, pixd2);
    pixCompare(pixd, pixd2, "results are the same", "results are different" );
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* ------------- Test open/close duality -------------- */
#if 0
    pixd = pixOpenGray(pixs, WSIZE, HSIZE);
    pixInvert(pixs, pixs);
    pixd2 = pixCloseGray(pixs, WSIZE, HSIZE);
    pixInvert(pixd2, pixd2);
    pixCompare(pixd, pixd2, "results are the same", "results are different" );
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* ------------- Test tophat duality -------------- */
#if 0
    pixd = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_WHITE);
    pixWrite(fileout, pixd, IFF_PNG);
    pixInvert(pixs, pixs);
    pixd2 = pixTophat(pixs, WSIZE, HSIZE, TOPHAT_BLACK);
    pixCompare(pixd, pixd2, "Correct: images are duals",
	       "Error: images are not duals" );
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
    pixInvert(pixs, pixs);

    pixd = pixGrayMorphSequence(pixs, "Tw9.5", 50, 100);
    pixInvert(pixs, pixs);
    pixd2 = pixGrayMorphSequence(pixs, "Tb9.5", 50, 300);
    pixCompare(pixd, pixd2, "Correct: images are duals",
	       "Error: images are not duals" );
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* ------------- Test opening/closing for large sels -------------- */
#if 0
    pixd = pixGrayMorphSequence(pixs,
	    "C9.9 + C19.19 + C29.29 + C39.39 + C49.49", 250, 100);
    pixDestroy(&pixd);
    pixd = pixGrayMorphSequence(pixs,
	    "O9.9 + O19.19 + O29.29 + O39.39 + O49.49", 250, 400);
    pixDestroy(&pixd);
#endif

    /* ---------- Closing plus white tophat result ------------ *
     *            Parameters: wsize, hsize = 9, 29             *
     * ---------------------------------------------------------*/
#if 0
    pixd = pixCloseGray(pixs, 9, 9);
    pixd1 = pixTophat(pixd, 9, 9, TOPHAT_WHITE);
    pixd2 = pixGrayMorphSequence(pixs, "C9.9 + TW9.9", 0, 0);
    pixCompare(pixd1, pixd2, "correct: same", "wrong: different");
    pixd3 = pixMaxDynamicRange(pixd1, L_LINEAR_SCALE);
    pixDisplay(pixd3, 100, 100);
    pixWrite(fileout, pixd3, IFF_JFIF_JPEG);
    pixDestroy(&pixd);
    pixDestroy(&pixd1);
    pixDestroy(&pixd2);
    pixDestroy(&pixd3);
    pixd = pixCloseGray(pixs, 29, 29);
    pixd1 = pixTophat(pixd, 29, 29, TOPHAT_WHITE);
    pixd2 = pixGrayMorphSequence(pixs, "C29.29 + Tw29.29", 0, 0);
    pixCompare(pixd1, pixd2, "correct: same", "wrong: different");
    pixd3 = pixMaxDynamicRange(pixd1, L_LINEAR_SCALE);
    pixDisplay(pixd3, 100, 400);
    pixDestroy(&pixd);
    pixDestroy(&pixd1);
    pixDestroy(&pixd2);
    pixDestroy(&pixd3);
#endif

    /* --------- hdome with parameter height = 100 ------------*/
#if 0
    pixd = pixHDome(pixs, 100);
    pixd2 = pixMaxDynamicRange(pixd, L_LINEAR_SCALE);
    pixDisplay(pixd2, 100, 400);
    pixWrite(fileout, pixd2, IFF_PNG);
    pixDestroy(&pixd2);
#endif

#if 0
    /* ----- Contrast enhancement with morph parameters 9, 9 -------*/
    pixd1 = pixInitAccumulate(w, h, 0x8000);
    pixAccumulate(pixd1, pixs, ARITH_ADD);
    pixMultConstAccumulate(pixd1, 3., 0x8000); 
    pixd2 = pixOpenGray(pixs, 9, 9);
    pixAccumulate(pixd1, pixd2, ARITH_SUBTRACT);
    pixDestroy(&pixd2);
    pixd2 = pixCloseGray(pixs, 9, 9);
    pixAccumulate(pixd1, pixd2, ARITH_SUBTRACT);
    pixDestroy(&pixd2);
    pixd = pixFinalAccumulate(pixd1, 0x8000, 8);
    pixDisplay(pixd, 200, 200);
    pixWrite(fileout, pixd, IFF_PNG);
    pixDestroy(&pixd1);

        /* Do the same thing with the Pixacc */
    pacc = pixaccCreate(w, h, 1);
    pixaccAdd(pacc, pixs);
    pixaccMultConst(pacc, 3.);
    pixd1 = pixOpenGray(pixs, 9, 9);
    pixaccSubtract(pacc, pixd1);
    pixDestroy(&pixd1);
    pixd1 = pixCloseGray(pixs, 9, 9);
    pixaccSubtract(pacc, pixd1);
    pixDestroy(&pixd1);
    pixd2 = pixaccFinal(pacc, 8);
    pixaccDestroy(&pacc);
    pixDisplay(pixd2, 500, 200);

    pixCompare(pixd, pixd2, "Correct: same", "Wrong: different");
    pixDestroy(&pixd);
    pixDestroy(&pixd2);
#endif

    /* ----  Tophat result on feynman stamp, to extract diagrams ----- */
#if 1 
    pixDestroy(&pixs);
    pixs = pixRead("feynman-stamp.jpg");

        /* Make output image to hold five intermediate images */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    pixd = pixCreate(5 * w + 18, h + 6, 32);  /* composite output image */
    pixSetAllArbitrary(pixd, 0x0000ff00);  /* set to blue */

        /* Paste in the input image */
    pixt = pixRemoveColormap(pixs, REMOVE_CMAP_TO_FULL_COLOR);
    pixRasterop(pixd, 3, 3, w, h, PIX_SRC, pixt, 0, 0);  /* 1st one */
    pixWrite("junkgray", pixt, IFF_JFIF_JPEG);
    pixDestroy(&pixt);

        /* Paste in the grayscale version */
    cmap = pixGetColormap(pixs);
    if (cmap)
	pixt = pixRemoveColormap(pixs, REMOVE_CMAP_TO_GRAYSCALE);
    else
        pixt = pixConvertRGBToGray(pixs, 0.33, 0.34, 0.33);
    pixt2 = pixConvertTo32(pixt);  /* 8 --> 32 bpp */
    pixRasterop(pixd, w + 6, 3, w, h, PIX_SRC, pixt2, 0, 0);  /* 2nd one */
    pixDestroy(&pixt2);

         /* Paste in a log dynamic range scaled version of the white tophat */
    pixt2 = pixTophat(pixt, 3, 3, TOPHAT_WHITE);
    pixt3a = pixMaxDynamicRange(pixt2, L_LOG_SCALE);
    pixt3 = pixConvertTo32(pixt3a);
    pixRasterop(pixd, 2 * w + 9, 3, w, h, PIX_SRC, pixt3, 0, 0);  /* 3rd */
    pixWrite("junktophat", pixt2, IFF_JFIF_JPEG);
    pixDestroy(&pixt3);
    pixDestroy(&pixt3a);
    pixDestroy(&pixt);

        /* Stretch the range and threshold to binary; paste it in */
    pixt3a = pixGammaTRC(NULL, pixt2, 1.0, 0, 80);
    pixt3 = pixThresholdToBinary(pixt3a, 70);
    pixt4 = pixConvertTo32(pixt3);
    pixRasterop(pixd, 3 * w + 12, 3, w, h, PIX_SRC, pixt4, 0, 0);  /* 4th */
    pixWrite("junkbin", pixt3, IFF_PNG);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3a);
    pixDestroy(&pixt4);

        /* Invert; this is the final result */
    pixInvert(pixt3, pixt3);
    pixt4 = pixConvertTo32(pixt3);
    pixRasterop(pixd, 4 * w + 15, 3, w, h, PIX_SRC, pixt4, 0, 0);  /* 5th */
    pixWrite("junkbininvert", pixt3, IFF_PNG);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkall", pixd, IFF_JFIF_JPEG);
    pixDestroy(&pixt3);
    pixDestroy(&pixt4);
    pixDestroy(&pixd);
#endif

    pixDestroy(&pixs);
    exit(0);
}



    /* simple comparison function */
static void pixCompare(PIX   *pix1,
                       PIX   *pix2,
                       char  *msg1,
                       char  *msg2)
{
l_int32  same, w;
    pixEqual(pix1, pix2, &same);
    if (same)
	fprintf(stderr, "%s\n", msg1);
    else {
	fprintf(stderr, "%s\n", msg2);
	w = pixGetWidth(pix1);
	pixDisplay(pix1, 100, 300);
	pixDisplay(pix2, 125 + w, 300);
    }
    return;
}


