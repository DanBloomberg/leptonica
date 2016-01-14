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
 * pagesegtest2.c
 *
 *    This gives examples of the use of binary morphology for
 *    some simple and fast document segmentation operations.
 *
 *    The operations are carried out at 2x reduction.
 *    For images scanned at 300 ppi, this is typically
 *    high enough resolution for accurate results.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32      index, zero;
char        *filein, *fileout;
BOXA        *boxatm, *boxahm;
PIX         *pixs;   /* input image, say, at 300 ppi */
PIX         *pixr;   /* image reduced to 150 ppi */
PIX         *pixhs;  /* image of halftone seed, 150 ppi */
PIX         *pixm;   /* image of mask of components, 150 ppi */
PIX         *pixhm1;  /* image of halftone mask, 150 ppi */
PIX         *pixhm2;  /* image of halftone mask, 300 ppi */
PIX         *pixht;  /* image of halftone components, 150 ppi */
PIX         *pixnht; /* image without halftone components, 150 ppi */
PIX         *pixi;   /* inverted image, 150 ppi */
PIX         *pixvws; /* image of vertical whitespace, 150 ppi */
PIX         *pixtm1; /* image of closed textlines, 150 ppi */
PIX         *pixtm2; /* image of refined text line mask, 150 ppi */
PIX         *pixtm3; /* image of refined text line mask, 300 ppi */
PIX         *pixtb1; /* image of text block mask, 150 ppi */
PIX         *pixtb2; /* image of text block mask, 300 ppi */
PIX         *pixnon; /* image of non-text or halftone, 150 ppi */
PIX         *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
PIXCMAP     *cmap;
PTAA        *ptaa;
static char  mainName[] = "pagesegtest2";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  pagesegtest2 filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

        /* Reduce to 150 ppi */
    pixt1 = pixScaleToGray2(pixs);
    pixDisplayWithTitle(pixt1, 0, 0, "input image");
    pixWrite("junk_orig.gray.150.png", pixt1, IFF_PNG);
    pixDestroy(&pixt1);
    pixr = pixReduceRankBinaryCascade(pixs, 2, 0, 0, 0);
    pixDisplayWithTitle(pixr, 40, 30, "input binary at 2x");
    pixWrite("junk_orig.150.png", pixr, IFF_PNG);

	/* Get seed for halftone parts */
    pixt1 = pixReduceRankBinaryCascade(pixr, 4, 4, 3, 0);
    pixt2 = pixOpenBrick(NULL, pixt1, 5, 5);
    pixhs = pixExpandBinary(pixt2, 8);
    pixWrite("junk_htseed.150.png", pixhs, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDisplayWithTitle(pixhs, 80, 60, "halftone seed");

	/* Get mask for connected regions */
    pixt1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
    pixm = pixCloseBrick(NULL, pixt1, 4, 4);
    pixWrite("junk_ccmask.150.png", pixm, IFF_PNG);
    pixDestroy(&pixt1);
    pixDisplayWithTitle(pixm, 120, 90, "halftone seed mask");

        /* Fill seed into mask to get halftone mask */
    pixhm1 = pixSeedfillBinary(NULL, pixhs, pixm, 4);
    pixWrite("junk_htmask.150.png", pixhm1, IFF_PNG);
    pixDisplayWithTitle(pixhm1, 160, 120, "halftone mask");
    
        /* Extract halftone stuff */
    pixht = pixAnd(NULL, pixhm1, pixr);
    pixWrite("junk_ht.150.png", pixht, IFF_PNG);
    pixDisplayWithTitle(pixht, 200, 150, "halftone stuff");

        /* Extract non-halftone stuff */
    pixnht = pixXor(NULL, pixht, pixr);
    pixWrite("junk_text.150.png", pixnht, IFF_PNG);
    pixDisplayWithTitle(pixnht, 240, 180, "non-halftone stuff");
    pixZero(pixht, &zero);
    if (zero)
	fprintf(stderr, "No halftone parts found\n");
    else
	fprintf(stderr, "Halftone parts found\n");

        /* Get bit-inverted image */
    pixi = pixInvert(NULL, pixnht);
    pixWrite("junk_invert.150.png", pixi, IFF_PNG);
    pixDisplayWithTitle(pixi, 280, 210, "inverted non-halftone");

	/* Identify vertical whitespace by opening inverted image */
    pixt1 = pixOpenBrick(NULL, pixi, 5, 1);  /* removes thin vertical lines */
    pixvws = pixOpenBrick(NULL, pixt1, 1, 200);  /* gets long vertical lines */
    pixWrite("junk_vertws.150.png", pixvws, IFF_PNG);
    pixDestroy(&pixt1);
    pixDisplayWithTitle(pixvws, 320, 240, "whitespace mask");

        /* Get proto (early processed) text line mask */
	/* first close the characters and words in the textlines */
    pixtm1 = pixCloseBrick(NULL, pixnht, 30, 1);
    pixWrite("junk_textmask1.150.png", pixtm1, IFF_PNG);
    pixDisplayWithTitle(pixtm1, 360, 270, "textline mask 1");

	/* Next open back up the vertical whitespace corridors */
    pixtm2 = pixSubtract(NULL, pixtm1, pixvws);
    pixWrite("junk_textmask2.150.png", pixtm2, IFF_PNG);
    pixDisplayWithTitle(pixtm2, 400, 300, "textline mask 2");

	/* Do a small opening to remove noise */
    pixOpenBrick(pixtm2, pixtm2, 3, 3);
    pixWrite("junk_textmask3.150.png", pixtm2, IFF_PNG);
    pixDisplayWithTitle(pixtm2, 400, 300, "textline mask 3");

        /* Join pixels vertically to make text block mask */
    pixtb1 = pixCloseBrick(NULL, pixtm2, 1, 10);
    pixWrite("junk_textblock1.150.png", pixtb1, IFF_PNG);
    pixDisplayWithTitle(pixtb1, 440, 330, "textblock mask");

        /* Solidify the textblock mask and remove noise:
         *  (1) Close the blocks and dilate slightly to form a solid mask.
         *  (2) Open the result to form a seed.
         *  (3) Fill from seed into mask, to remove the noise.
         *  (4) Expand the result to full res.  */
    pixt1 = pixMorphSequenceByComponent(pixtb1, "c30.30 + d3.3", 8, 0, 0, NULL);
    pixt2 = pixMorphSequenceByComponent(pixt1, "o20.20", 8, 0, 0, NULL);
    pixt3 = pixSeedfillBinary(NULL, pixt1, pixt2, 8);
    pixtb2 = pixExpandBinary(pixt3, 2);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);
    pixWrite("junk_textblock2.300.png", pixtb2, IFF_PNG);
    pixDisplayWithTitle(pixtb2, 480, 360, "textblock mask");

        /* Identify the outlines of each textblock */
    ptaa = pixGetOuterBordersPtaa(pixtb2);
    pixt1 = pixRenderRandomCmapPtaa(pixtb2, ptaa, 8);
    cmap = pixGetColormap(pixt1);
    pixcmapResetColor(cmap, 0, 130, 130, 130);
    pixWrite("junk_textblock3.300.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 480, 360, "textblock mask with outlines");
    ptaaDestroy(&ptaa);
    pixDestroy(&pixt1);

        /* Expand line masks to full resolution, and fill into the original */
    pixtm3 = pixExpandBinary(pixtm2, 2);
    pixt1 = pixSeedfillBinary(NULL, pixtm3, pixs, 8);
    pixOr(pixtm3, pixtm3, pixt1);
    pixDestroy(&pixt1);
    pixWrite("junk_textmask.300.png", pixtm3, IFF_PNG);
    pixDisplayWithTitle(pixtm3, 480, 360, "textline mask 4");
    pixhm2 = pixExpandBinary(pixhm1, 2);
    pixt1 = pixSeedfillBinary(NULL, pixhm2, pixs, 8);
    pixOr(pixhm2, pixhm2, pixt1);
    pixDestroy(&pixt1);
    pixWrite("junk_htmask.300.png", pixhm2, IFF_PNG);
    pixDisplayWithTitle(pixhm2, 520, 390, "halftonemask 2");

        /* Find objects that are neither text nor halftones */
    pixt1 = pixSubtract(NULL, pixs, pixtm3);  /* remove text pixels */
    pixnon = pixSubtract(NULL, pixt1, pixhm2);  /* remove halftone pixels */
    pixWrite("junk_other.300.png", pixnon, IFF_PNG);
    pixDisplayWithTitle(pixnon, 540, 420, "other stuff");
    pixDestroy(&pixt1);

        /* Write out b.b. for text line mask and halftone mask components */
    boxatm = pixConnComp(pixtm3, NULL, 4);
    boxahm = pixConnComp(pixhm2, NULL, 8);
    boxaWrite("junk_textmask.boxa", boxatm);
    boxaWrite("junk_htmask.boxa", boxahm);

    pixWrite(fileout, pixtm3, IFF_PNG);

	/* clean up to test with valgrind */
    pixDestroy(&pixs);
    pixDestroy(&pixr);
    pixDestroy(&pixhs);
    pixDestroy(&pixm);
    pixDestroy(&pixhm1);
    pixDestroy(&pixhm2);
    pixDestroy(&pixht);
    pixDestroy(&pixnht);
    pixDestroy(&pixi);
    pixDestroy(&pixvws);
    pixDestroy(&pixtm1);
    pixDestroy(&pixtm2);
    pixDestroy(&pixtm3);
    pixDestroy(&pixtb1);
    pixDestroy(&pixtb2);
    pixDestroy(&pixnon);
    boxaDestroy(&boxatm);
    boxaDestroy(&boxahm);

    exit(0);
}

