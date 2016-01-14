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

    /* Control the output */
#define   HT_DISP      1
#define   WS_MASK      1
#define   TEXT_DISP    1
#define   BLOCK_DISP   1
#define   DFLAG        0


main(int    argc,
     char **argv)
{
l_int32      index, zero;
char        *filein;
BOXA        *boxatm, *boxahm;
PIX         *pixs;   /* input image, say, at 300 ppi */
PIX         *pixr;   /* image reduced to 150 ppi */
PIX         *pixhs;  /* image of halftone seed, 150 ppi */
PIX         *pixm;   /* image of mask of components, 150 ppi */
PIX         *pixhm1; /* image of halftone mask, 150 ppi */
PIX         *pixhm2; /* image of halftone mask, 300 ppi */
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
PIXA        *pixa;
PIXCMAP     *cmap;
PTAA        *ptaa;
static char  mainName[] = "pagesegtest2";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  pagesegtest2 filein", mainName, 1));

    filein = argv[1];
    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

        /* Reduce to 150 ppi */
    pixt1 = pixScaleToGray2(pixs);
    pixDisplayWrite(pixt1, L_MAX(WS_MASK, L_MAX(HT_DISP, BLOCK_DISP)));
    pixWrite("junk_orig.gray.150.png", pixt1, IFF_PNG);
    pixDestroy(&pixt1);
    pixr = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);

	/* Get seed for halftone parts */
    pixt1 = pixReduceRankBinaryCascade(pixr, 4, 4, 3, 0);
    pixt2 = pixOpenBrick(NULL, pixt1, 5, 5);
    pixhs = pixExpandBinaryPower2(pixt2, 8);
    pixDisplayWrite(pixhs, HT_DISP);
    pixWrite("junk_htseed.150.png", pixhs, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

	/* Get mask for connected regions */
    pixm = pixCloseSafeBrick(NULL, pixr, 4, 4);
    pixDisplayWrite(pixm, HT_DISP);
    pixWrite("junk_ccmask.150.png", pixm, IFF_PNG);

        /* Fill seed into mask to get halftone mask */
    pixhm1 = pixSeedfillBinary(NULL, pixhs, pixm, 4);
    pixDisplayWrite(pixhm1, HT_DISP);
    pixWrite("junk_htmask.150.png", pixhm1, IFF_PNG);
    pixhm2 = pixExpandBinaryPower2(pixhm1, 2);
    
        /* Extract halftone stuff */
    pixht = pixAnd(NULL, pixhm1, pixr);
    pixWrite("junk_ht.150.png", pixht, IFF_PNG);

        /* Extract non-halftone stuff */
    pixnht = pixXor(NULL, pixht, pixr);
    pixDisplayWrite(pixnht, TEXT_DISP);
    pixWrite("junk_text.150.png", pixnht, IFF_PNG);
    pixZero(pixht, &zero);
    if (zero)
	fprintf(stderr, "No halftone parts found\n");
    else
	fprintf(stderr, "Halftone parts found\n");

        /* Get bit-inverted image */
    pixi = pixInvert(NULL, pixnht);
    pixWrite("junk_invert.150.png", pixi, IFF_PNG);
    pixDisplayWrite(pixi, WS_MASK);

        /* The whitespace mask will break textlines where there
         * is a large amount of white space below or above.
         * We can prevent this by identifying regions of the
         * inverted image that have large horizontal (bigger than
         * the separation between columns) and significant
         * vertical extent (bigger than the separation between
         * textlines), and subtracting this from the whitespace mask. */
    pixt1 = pixMorphCompSequence(pixi, "o80.60", 0);
    pixt2 = pixSubtract(NULL, pixi, pixt1);
    pixDisplayWrite(pixt2, WS_MASK);
    pixDestroy(&pixt1);

	/* Identify vertical whitespace by opening inverted image */
    pixt3 = pixOpenBrick(NULL, pixt2, 5, 1);  /* removes thin vertical lines */
    pixvws = pixOpenBrick(NULL, pixt3, 1, 200);  /* gets long vertical lines */
    pixDisplayWrite(pixvws, L_MAX(TEXT_DISP, WS_MASK));
    pixWrite("junk_vertws.150.png", pixvws, IFF_PNG);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);

        /* Get proto (early processed) text line mask. */
	/* First close the characters and words in the textlines */
    pixtm1 = pixCloseSafeBrick(NULL, pixnht, 30, 1);
    pixDisplayWrite(pixtm1, TEXT_DISP);
    pixWrite("junk_textmask1.150.png", pixtm1, IFF_PNG);

	/* Next open back up the vertical whitespace corridors */
    pixtm2 = pixSubtract(NULL, pixtm1, pixvws);
    pixWrite("junk_textmask2.150.png", pixtm2, IFF_PNG);

	/* Do a small opening to remove noise */
    pixOpenBrick(pixtm2, pixtm2, 3, 3);
    pixDisplayWrite(pixtm2, TEXT_DISP);
    pixWrite("junk_textmask3.150.png", pixtm2, IFF_PNG);
    pixtm3 = pixExpandBinaryPower2(pixtm2, 2);

        /* Join pixels vertically to make text block mask */
    pixtb1 = pixMorphSequence(pixtm2, "c1.10 + o4.1", 0);
    pixDisplayWrite(pixtb1, BLOCK_DISP);
    pixWrite("junk_textblock1.150.png", pixtb1, IFF_PNG);

        /* Solidify the textblock mask and remove noise:
         *  (1) For each c.c., close the blocks and dilate slightly
	 *      to form a solid mask.
         *  (2) Small horizontal closing between components
	 *  (3) Open the white space between columns, again
         *  (4) Remove small components */
    pixt1 = pixMorphSequenceByComponent(pixtb1, "c30.30 + d3.3", 8, 0, 0, NULL);
    pixCloseSafeBrick(pixt1, pixt1, 10, 1);
    pixDisplayWrite(pixt1, BLOCK_DISP);
    pixt2 = pixSubtract(NULL, pixt1, pixvws);
    pixt3 = pixSelectBySize(pixt2, 25, 5, 8, L_SELECT_IF_BOTH,
                            L_SELECT_IF_GTE, NULL);
    pixDisplayWrite(pixt3, BLOCK_DISP);
    pixWrite("junk_textblock2.150.png", pixt3, IFF_PNG);
    pixtb2 = pixExpandBinaryPower2(pixt3, 2);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);

        /* Identify the outlines of each textblock */
    ptaa = pixGetOuterBordersPtaa(pixtb2);
    pixt1 = pixRenderRandomCmapPtaa(pixtb2, ptaa, 8, 1);
    cmap = pixGetColormap(pixt1);
    pixcmapResetColor(cmap, 0, 130, 130, 130);  /* set interior to gray */
    pixWrite("junk_textblock3.300.png", pixt1, IFF_PNG);
    pixDisplayWithTitle(pixt1, 480, 360, "textblock mask with outlines", DFLAG);
    ptaaDestroy(&ptaa);
    pixDestroy(&pixt1);

        /* Fill line mask (as seed) into the original */
    pixt1 = pixSeedfillBinary(NULL, pixtm3, pixs, 8);
    pixOr(pixtm3, pixtm3, pixt1);
    pixDestroy(&pixt1);
    pixWrite("junk_textmask.300.png", pixtm3, IFF_PNG);
    pixDisplayWithTitle(pixtm3, 480, 360, "textline mask 4", DFLAG);

        /* Fill halftone mask (as seed) into the original */
    pixt1 = pixSeedfillBinary(NULL, pixhm2, pixs, 8);
    pixOr(pixhm2, pixhm2, pixt1);
    pixDestroy(&pixt1);
    pixWrite("junk_htmask.300.png", pixhm2, IFF_PNG);
    pixDisplayWithTitle(pixhm2, 520, 390, "halftonemask 2", DFLAG);

        /* Find objects that are neither text nor halftones */
    pixt1 = pixSubtract(NULL, pixs, pixtm3);  /* remove text pixels */
    pixnon = pixSubtract(NULL, pixt1, pixhm2);  /* remove halftone pixels */
    pixWrite("junk_other.300.png", pixnon, IFF_PNG);
    pixDisplayWithTitle(pixnon, 540, 420, "other stuff", DFLAG);
    pixDestroy(&pixt1);

        /* Write out b.b. for text line mask and halftone mask components */
    boxatm = pixConnComp(pixtm3, NULL, 4);
    boxahm = pixConnComp(pixhm2, NULL, 8);
    boxaWrite("junk_textmask.boxa", boxatm);
    boxaWrite("junk_htmask.boxa", boxahm);

    pixa = pixaReadFiles(".", "junk_write_display");
    pixt1 = pixaDisplayTiledAndScaled(pixa, 8, 250, 4, 0, 25, 2);
    pixWrite("junktiles", pixt1, IFF_JFIF_JPEG);
    pixDestroy(&pixt1);
    pixaDestroy(&pixa);

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

