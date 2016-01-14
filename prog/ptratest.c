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
 * ptratest.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static void MakePtrasFromPixa(PIXA *pixa, PTRA **ppapix, PTRA **ppabox,
                              l_int32 copyflag);
static PIXA *ReconstructPixa(PTRA *papix, PTRA *pabox, l_int32 choose);
static PIXA *ReconstructPixa1(PTRA *papix, PTRA *pabox);
static PIXA *ReconstructPixa2(PTRA *papix, PTRA *pabox);
static void CopyPtras(PTRA *papixs, PTRA *paboxs,
                      PTRA **ppapixd, PTRA **ppaboxd);
static void DisplayResult(PIXA *pixac, PIXA **ppixa, l_int32 w, l_int32 h,
                          l_int32 newline);

#define  CHOOSE_RECON    2    /* 1 or 2 */


main(int    argc,
     char **argv)
{
l_int32      i, n, w, h, nactual, j;
BOX         *box;
BOXA        *boxa;
PIX         *pixs, *pixd, *pix;
PIXA        *pixas, *pixat, *pixac;
PTRA        *papix, *pabox, *papix2, *pabox2;
static char  mainName[] = "ptratest";

    if (argc != 1)
	exit(ERROR_INT(" Syntax: ptratest", mainName, 1));

    pixac = pixaCreate(0);

    if ((pixs = pixRead("lucasta.1.300.tif")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));
    pixGetDimensions(pixs, &w, &h, NULL);
    boxa = pixConnComp(pixs, &pixas, 8);
    pixDestroy(&pixs);
    boxaDestroy(&boxa);
    n = pixaGetCount(pixas);

        /* Fill ptras with clones and reconstruct */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_CLONE);
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 1);

        /* Remove every other one for the first half;
         * with compaction at each removal */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_COPY);
    for (i = 0; i < n / 2; i++) {
        if (i % 2 == 0) {
            pix = (PIX *)ptraRemove(papix, i, L_COMPACTION);
            box = (BOX *)ptraRemove(pabox, i, L_COMPACTION);
            pixDestroy(&pix);
            boxDestroy(&box);
        }
    }
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 0);

        /* Remove every other one for the entire set,
         * but without compaction at each removal */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_COPY);
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            pix = (PIX *)ptraRemove(papix, i, L_NO_COMPACTION);
            box = (BOX *)ptraRemove(pabox, i, L_NO_COMPACTION);
            pixDestroy(&pix);
            boxDestroy(&box);
        }
    }
    ptraCompactArray(papix);  /* now do the compaction */
    ptraCompactArray(pabox);
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 0);

        /* Fill ptras using insert at head, and reconstruct */
    papix = ptraCreate(n);
    pabox = ptraCreate(n);
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixas, i, L_CLONE);
        box = pixaGetBox(pixas, i, L_CLONE);
        ptraInsert(papix, 0, pix, L_MIN_DOWNSHIFT);
        ptraInsert(pabox, 0, box, L_FULL_DOWNSHIFT);
    }
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 1);

        /* Reverse the arrays by swapping */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_CLONE);
    for (i = 0; i < n / 2; i++) {
        ptraSwap(papix, i, n - i - 1);
        ptraSwap(pabox, i, n - i - 1);
    }
    ptraCompactArray(papix);  /* already compact; shouldn't do anything */
    ptraCompactArray(pabox);
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 0);

        /* Remove at the top of the array and push the hole to the end
         * by neighbor swapping (!).  This is O(n^2), so it's not a
         * recommended way to copy a ptra. [joke]  */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_CLONE);
    papix2 = ptraCreate(0);
    pabox2 = ptraCreate(0);
    while (1) {
        ptraGetActualCount(papix, &nactual);
        if (nactual == 0) break;
        ptraGetMaxIndex(papix, &n);
        pix = (PIX *)ptraRemove(papix, 0, L_NO_COMPACTION);
        box = (BOX *)ptraRemove(pabox, 0, L_NO_COMPACTION);
        ptraAdd(papix2, pix);
        ptraAdd(pabox2, box);
        for (i = 1; i < n; i++) {
           ptraSwap(papix, i - 1, i);
           ptraSwap(pabox, i - 1, i);
        }
    }
    ptraCompactArray(papix);  /* should be empty */
    ptraCompactArray(pabox);  /* ditto */
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 1);  /* nothing there */
    pixat = ReconstructPixa(papix2, pabox2, CHOOSE_RECON);
    ptraDestroy(&papix2, 0);
    ptraDestroy(&pabox2, 0);
    DisplayResult(pixac, &pixat, w, h, 0);

        /* Remove and insert one position above, allowing minimum downshift.
         * If you specify L_AUTO_DOWNSHIFT, because there is only 1 hole,
         * it will do a full downshift at each insert.  This is a
         * situation where the heuristic (expected number of holes)
         * fails to do the optimal thing. */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_CLONE);
    for (i = 1; i < n; i++) {
        pix = (PIX *)ptraRemove(papix, i, L_NO_COMPACTION);
        box = (BOX *)ptraRemove(pabox, i, L_NO_COMPACTION);
        ptraInsert(papix, i - 1, pix, L_MIN_DOWNSHIFT);
        ptraInsert(pabox, i - 1, box, L_MIN_DOWNSHIFT);
    }
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 1);

        /* Remove and insert one position above, but this time
         * forcing a full downshift at each step.  */
    MakePtrasFromPixa(pixas, &papix, &pabox, L_CLONE);
    for (i = 1; i < n; i++) {
        pix = (PIX *)ptraRemove(papix, i, L_NO_COMPACTION);
        box = (BOX *)ptraRemove(pabox, i, L_NO_COMPACTION);
        ptraInsert(papix, i - 1, pix, L_AUTO_DOWNSHIFT);
        ptraInsert(pabox, i - 1, box, L_AUTO_DOWNSHIFT);
    }
/*    ptraCompactArray(papix);
    ptraCompactArray(pabox); */
    pixat = ReconstructPixa(papix, pabox, CHOOSE_RECON);
    ptraDestroy(&papix, 0);
    ptraDestroy(&pabox, 0);
    DisplayResult(pixac, &pixat, w, h, 0);

    pixd = pixaDisplay(pixac, 0, 0);
    pixDisplay(pixd, 100, 100);
    pixWrite("junkptra.png", pixd, IFF_PNG);
    pixDestroy(&pixd);
    pixaDestroy(&pixac);
    pixaDestroy(&pixas);
    return 0;
}
	    

static void
MakePtrasFromPixa(PIXA    *pixa,
                  PTRA   **ppapix,
                  PTRA   **ppabox,
                  l_int32  copyflag)
{
l_int32  i, n;
BOX     *box;
PIX     *pix;
PTRA    *papix, *pabox;

    n = pixaGetCount(pixa);
    papix = ptraCreate(n);
    pabox = ptraCreate(n);
    for (i = 0; i < n; i++) {
        pix = pixaGetPix(pixa, i, copyflag);
        box = pixaGetBox(pixa, i, copyflag);
        ptraAdd(papix, pix);
        ptraAdd(pabox, box);
    }

    *ppapix = papix;
    *ppabox = pabox;
    return;
}


static PIXA *
ReconstructPixa(PTRA   *papix,
                PTRA   *pabox,
                l_int32 choose)
{
PIXA  *pixa;

    if (choose == 1)
        pixa = ReconstructPixa1(papix, pabox);
    else
        pixa = ReconstructPixa2(papix, pabox);
    return pixa;
}


static PIXA *
ReconstructPixa1(PTRA  *papix,
                 PTRA  *pabox)
{
l_int32  i, n, nactual;
BOX     *box;
PIX     *pix;
PIXA    *pixat;

    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "Before removal:  n = %4d, actual = %4d\n", n, nactual);

    pixat = pixaCreate(n);
    for (i = 0; i < n; i++) {
        pix = (PIX *)ptraRemove(papix, i, L_NO_COMPACTION);
        box = (BOX *)ptraRemove(pabox, i, L_NO_COMPACTION);
        if (pix) pixaAddPix(pixat, pix, L_INSERT);
        if (box) pixaAddBox(pixat, box, L_INSERT);
    }

    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "After removal:   n = %4d, actual = %4d\n\n", n, nactual);

    return pixat;
}


static PIXA *
ReconstructPixa2(PTRA  *papix,
                 PTRA  *pabox)
{
l_int32  i, n, nactual;
BOX     *box;
PIX     *pix;
PIXA    *pixat;

    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "Before removal:    n = %4d, actual = %4d\n", n, nactual);

        /* Remove half */
    pixat = pixaCreate(n);
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            pix = (PIX *)ptraRemove(papix, i, L_NO_COMPACTION);
            box = (BOX *)ptraRemove(pabox, i, L_NO_COMPACTION);
            if (pix) pixaAddPix(pixat, pix, L_INSERT);
            if (box) pixaAddBox(pixat, box, L_INSERT);
        }
    }

        /* Compact */
    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "Before compaction: n = %4d, actual = %4d\n", n, nactual);
    ptraCompactArray(papix);
    ptraCompactArray(pabox);
    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "After compaction:  n = %4d, actual = %4d\n", n, nactual);

        /* Remove the rest (and test compaction with removal) */
    while (1) {
        ptraGetActualCount(papix, &nactual);
        if (nactual == 0) break;

        pix = (PIX *)ptraRemove(papix, 0, L_COMPACTION);
        box = (BOX *)ptraRemove(pabox, 0, L_COMPACTION);
        pixaAddPix(pixat, pix, L_INSERT);
        pixaAddBox(pixat, box, L_INSERT);
    }

    ptraGetMaxIndex(papix, &n);
    ptraGetActualCount(papix, &nactual);
    fprintf(stderr, "After removal:     n = %4d, actual = %4d\n\n", n, nactual);

    return pixat;
}


static void
CopyPtras(PTRA   *papixs,
          PTRA   *paboxs,
          PTRA  **ppapixd,
          PTRA  **ppaboxd)
{
l_int32  i, n;
BOX     *box;
PIX     *pix;

    ptraGetMaxIndex(papixs, &n);
    *ppapixd = ptraCreate(n);
    *ppaboxd = ptraCreate(n);
    for (i = 0; i < n; i++) {
        pix = pixCopy(NULL, (PIX *)ptraGetPtrToItem(papixs, i));
        box = boxCopy((BOX *)ptraGetPtrToItem(paboxs, i));
        ptraAdd(*ppapixd, pix);
        ptraAdd(*ppaboxd, box);
    }
    return;
}


static void
DisplayResult(PIXA   *pixac,
              PIXA  **ppixa,
              l_int32  w,
              l_int32  h,
              l_int32  newline)
{
PIX   *pixd;

    pixd = pixaDisplay(*ppixa, w, h);
    pixSaveTiled(pixd, pixac, 1, newline, 30, 8);
    pixDestroy(&pixd);
    pixaDestroy(ppixa);
    return;
}

