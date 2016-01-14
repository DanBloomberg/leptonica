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
 * docseg1.c
 *
 *    Use witten.png
 *    Use rabi.png
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"



main(int    argc,
     char **argv)
{
l_int32      index, zero;
char        *filein, *fileout;
PIX         *pixs;   /* input image, 300 ppi */
PIX         *pix240; /* input image, reduce to 240 ppi */
PIX         *pixr;   /* image reduced to 60 ppi */
PIX         *pixhs;  /* image of halftone seed, 60 ppi */
PIX         *pixm;   /* image of mask of components, 60 ppi */
PIX         *pixhm;  /* image of halftone mask, 60 ppi */
PIX         *pixht;  /* image of halftone components, 60 ppi */
PIX         *pixnht; /* image without halftone components, 60 ppi */
PIX         *pixi;   /* inverted image, 60 ppi */
PIX         *pixvws; /* image of vertical whitespace, 60 ppi */
PIX         *pixctl; /* image of closed textlines, 60 ppi */
PIX         *pixptlm;/* image of proto text line mask, 60 ppi */
PIX         *pixtlm; /* image of refined text line mask, 60 ppi */
PIX         *pixtbm; /* image of text block mask, 60 ppi */
PIX         *pixnon; /* image of non-text or halftone, 60 ppi */
PIX         *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
SEL         *sel_2, *sel_3, *sel_4, *sel_5;
SEL         *sel_2h, *sel_3h, *sel_4h, *sel_5h, *sel_8h;
SEL         *sel_10h, *sel_20h, *sel_30h, *sel_40h, *sel_50h;
SEL         *sel_5v, *sel_10v, *sel_20v, *sel_30v, *sel_40v, *sel_50v;
SELA        *sela;
static char  mainName[] = "docseg1";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  docseg1 filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

    if ((pixs = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));

    sela = selaAddBasic(NULL);

    selaFindSelByName(sela, "sel_2", &index, &sel_2);
    selaFindSelByName(sela, "sel_3", &index, &sel_3);
    selaFindSelByName(sela, "sel_4", &index, &sel_4);
    selaFindSelByName(sela, "sel_5", &index, &sel_5);

    selaFindSelByName(sela, "sel_2h", &index, &sel_2h);
    selaFindSelByName(sela, "sel_3h", &index, &sel_3h);
    selaFindSelByName(sela, "sel_4h", &index, &sel_4h);
    selaFindSelByName(sela, "sel_5h", &index, &sel_5h);
    selaFindSelByName(sela, "sel_8h", &index, &sel_8h);
    selaFindSelByName(sela, "sel_10h", &index, &sel_10h);
    selaFindSelByName(sela, "sel_20h", &index, &sel_20h);
    selaFindSelByName(sela, "sel_30h", &index, &sel_30h);
    selaFindSelByName(sela, "sel_40h", &index, &sel_40h);
    selaFindSelByName(sela, "sel_50h", &index, &sel_50h);

    selaFindSelByName(sela, "sel_5v", &index, &sel_5v);
    selaFindSelByName(sela, "sel_10v", &index, &sel_10v);
    selaFindSelByName(sela, "sel_20v", &index, &sel_20v);
    selaFindSelByName(sela, "sel_30v", &index, &sel_30v);
    selaFindSelByName(sela, "sel_40v", &index, &sel_40v);
    selaFindSelByName(sela, "sel_50v", &index, &sel_50v);

        /* reduce to 60 ppi */
    pix240 = pixScale(pixs, 0.8, 0.8);  /* to 240 ppi */
    pixt1 = pixScaleToGray4(pix240);
    pixDisplay(pixt1, 0, 0);
    pixWrite("ds_orig.gray.60.png", pixt1, IFF_PNG);
    pixDestroy(&pixt1);
    pixr = pixReduceRankBinaryCascade(pix240, 2, 2, 0, 0);
    pixDisplay(pixr, 10, 10);
    pixWrite("ds_orig.60.png", pixr, IFF_PNG);

	/* get seed for halftone parts */
    pixt1 = pixReduceRankBinaryCascade(pixr, 4, 4, 0, 0);
    pixt2 = pixOpen(NULL, pixt1, sel_5);
    pixhs = pixExpandBinary(pixt2, 4);
    pixWrite("ds_htseed.60.png", pixhs, IFF_PNG);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDisplay(pixhs, 40, 40);

	/* get mask for connected regions */
    pixt1 = pixReduceRankBinaryCascade(pix240, 1, 1, 0, 0);
    pixm = pixClose(NULL, pixt1, sel_4);
    pixWrite("ds_ccmask.60.png", pixm, IFF_PNG);
    pixDestroy(&pixt1);
    pixDisplay(pixm, 70, 70);

        /* fill seed into mask to get halftone mask */
    pixhm = pixSeedfillBinary(NULL, pixhs, pixm, 4);
    pixWrite("ds_htmask.60.png", pixhm, IFF_PNG);
    pixDisplay(pixhm, 100, 100);
    
        /* extract halftone stuff */
    pixht = pixAnd(NULL, pixhm, pixr);
    pixWrite("ds_ht.60.png", pixht, IFF_PNG);
    pixDisplay(pixht, 130, 130);

        /* extract non-halftone stuff */
    pixnht = pixXor(NULL, pixht, pixr);
    pixWrite("ds_text.60.png", pixnht, IFF_PNG);
    pixDisplay(pixnht, 160, 160);
    pixZero(pixht, &zero);
    if (zero)
	fprintf(stderr, "No halftone parts found\n");
    else
	fprintf(stderr, "Halftone parts found\n");

        /* get bit-inverted image */
    pixi = pixInvert(NULL, pixnht);
    pixWrite("ds_invert.60.png", pixi, IFF_PNG);
    pixDisplay(pixi, 190, 190);

	/* identify vertical whitespace by opening inverted image */
    pixt1 = pixOpen(NULL, pixi, sel_5h);  /* removes thin vertical lines */
    pixvws = pixOpen(NULL, pixt1, sel_50v);  /* gets long vertical lines */
    pixWrite("ds_vertws.60.png", pixvws, IFF_PNG);
    pixDestroy(&pixt1);
    pixDisplay(pixvws, 220, 220);

        /* get proto (early processed) text line mask */
	/* first close the characters and words in the textlines */
    pixctl = pixClose(NULL, pixnht, sel_30h);
    pixWrite("ds_closedtextlm.60.png", pixctl, IFF_PNG);
    pixDisplay(pixctl, 240, 240);

	/* next open back up the vertical whitespace corridors */
    pixptlm = pixSubtract(NULL, pixctl, pixvws);
    pixWrite("ds_prototextlm.60.png", pixptlm, IFF_PNG);
    pixDisplay(pixptlm, 260, 260);

        /* close/open filter to get good text line mask */
    pixt1 = pixClose(NULL, pixptlm, sel_8h);
    pixtlm = pixOpen(NULL, pixt1, sel_8h);
    pixWrite("ds_textlm.60.png", pixtlm, IFF_PNG);
    pixDisplay(pixtlm, 280, 280);
    pixDestroy(&pixt1);

        /* join pixels to make text block mask */
    pixtbm = pixClose(NULL, pixtlm, sel_5v);
    pixWrite("ds_textbm.60.png", pixtbm, IFF_PNG);
    pixDisplay(pixtbm, 310, 310);

        /* find objects that are neither text nor halftones */
    pixt1 = pixAnd(NULL, pixnht, pixtbm);  /* extract text pixels */
    pixnon = pixXor(NULL, pixt1, pixnht);  /* remove */
    pixWrite("ds_other.60.png", pixnon, IFF_PNG);
    pixDisplay(pixnon, 340, 340);
    pixDestroy(&pixt1);


    pixWrite(fileout, pixtlm, IFF_PNG);

	/* clean up to test with valgrind */
    selaDestroy(&sela);

    pixDestroy(&pixs);
    pixDestroy(&pix240);
    pixDestroy(&pixr);
    pixDestroy(&pixhs);
    pixDestroy(&pixm);
    pixDestroy(&pixhm);
    pixDestroy(&pixht);
    pixDestroy(&pixnht);
    pixDestroy(&pixi);
    pixDestroy(&pixvws);
    pixDestroy(&pixctl);
    pixDestroy(&pixptlm);
    pixDestroy(&pixtlm);
    pixDestroy(&pixtbm);
    pixDestroy(&pixnon);

    exit(0);
}

