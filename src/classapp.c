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
 *  classapp.c
 *
 *      Top-level jb2 correlation and rank-hausdorff
 *
 *         l_int32      jbCorrelation()
 *         l_int32      jbRankHausdorff()
 *
 *      Extract and classify words in textline order
 *
 *         JBCLASSER   *jbWordsInTextlines()
 *         l_int32      pixGetWordsInTextlines()
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  BUF_SIZE = 512;
static const l_int32  JB_WORDS_MIN_WIDTH = 5;  /* pixels */


/*------------------------------------------------------------------*
 *          Top-level jb2 correlation and rank-hausdorff            *
 *------------------------------------------------------------------*/
/*!
 *  jbCorrelation()
 *
 *       Input:  dirin (directory of input images)
 *               thresh (typically ~0.8)
 *               weight (typically ~0.6)
 *               components (JB_CONN_COMPS, JB_CHARACTERS, JB_WORDS)
 *               rootname (for output files)
 *               firstpage (0-based)
 *               npages (use 0 for all pages in dirin)
 *               renderflag (1 to render from templates; 0 to skip)
 *       Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See prog/jbcorrelation for generating more output (e.g.,
 *          for debugging)
 */
l_int32
jbCorrelation(const char  *dirin,
              l_float32    thresh,
              l_float32    weight,
              l_int32      components,
              const char  *rootname,
              l_int32      firstpage,
              l_int32      npages,
              l_int32      renderflag)
{
char        filename[BUF_SIZE];
l_int32     nfiles, i, numpages;
JBDATA     *data;
JBCLASSER  *classer;
PIX        *pix;
PIXA       *pixa;
SARRAY     *safiles;

    PROCNAME("jbCorrelation");

    if (!dirin)
        return ERROR_INT("dirin not defined", procName, 1);
    if (!rootname)
        return ERROR_INT("rootname not defined", procName, 1);
    if (components != JB_CONN_COMPS && components != JB_CHARACTERS &&
        components != JB_WORDS)
        return ERROR_INT("components invalid", procName, 1);

    safiles = getSortedPathnamesInDirectory(dirin, firstpage, npages);
    nfiles = sarrayGetCount(safiles);

        /* Classify components */
    classer = jbCorrelationInit(components, 0, 0, thresh, weight);
    jbAddPages(classer, safiles);

        /* Save data */
    data = jbDataSave(classer);
    jbDataWrite(rootname, data);

        /* Optionally, render pages using class templates */
    if (renderflag) {
        pixa = jbDataRender(data, FALSE);
        numpages = pixaGetCount(pixa);
        if (numpages != nfiles)
            fprintf(stderr, "numpages = %d, nfiles = %d, not equal!\n",
                    numpages, nfiles);
        for (i = 0; i < numpages; i++) {
            pix = pixaGetPix(pixa, i, L_CLONE);
            snprintf(filename, BUF_SIZE, "%s.%05d", rootname, i);
            fprintf(stderr, "filename: %s\n", filename);
            pixWrite(filename, pix, IFF_PNG);
            pixDestroy(&pix);
        }
        pixaDestroy(&pixa);
    }

    sarrayDestroy(&safiles);
    jbClasserDestroy(&classer);
    jbDataDestroy(&data);
    return 0;
}


/*!
 *  jbRankHaus()
 *
 *       Input:  dirin (directory of input images)
 *               size (of Sel used for dilation; typ. 2)
 *               rank (rank value of match; typ. 0.97)
 *               components (JB_CONN_COMPS, JB_CHARACTERS, JB_WORDS)
 *               rootname (for output files)
 *               firstpage (0-based)
 *               npages (use 0 for all pages in dirin)
 *               renderflag (1 to render from templates; 0 to skip)
 *       Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See prog/jbrankhaus for generating more output (e.g.,
 *          for debugging)
 */
l_int32
jbRankHaus(const char  *dirin,
           l_int32      size,
           l_float32    rank,
           l_int32      components,
           const char  *rootname,
           l_int32      firstpage,
           l_int32      npages,
           l_int32      renderflag)
{
char        filename[BUF_SIZE];
l_int32     nfiles, i, numpages;
JBDATA     *data;
JBCLASSER  *classer;
PIX        *pix;
PIXA       *pixa;
SARRAY     *safiles;

    PROCNAME("jbRankHaus");

    if (!dirin)
        return ERROR_INT("dirin not defined", procName, 1);
    if (!rootname)
        return ERROR_INT("rootname not defined", procName, 1);
    if (components != JB_CONN_COMPS && components != JB_CHARACTERS &&
        components != JB_WORDS)
        return ERROR_INT("components invalid", procName, 1);

    safiles = getSortedPathnamesInDirectory(dirin, firstpage, npages);
    nfiles = sarrayGetCount(safiles);

        /* Classify components */
    classer = jbRankHausInit(components, 0, 0, size, rank);
    jbAddPages(classer, safiles);

        /* Save data */
    data = jbDataSave(classer);
    jbDataWrite(rootname, data);

        /* Optionally, render pages using class templates */
    if (renderflag) {
        pixa = jbDataRender(data, FALSE);
        numpages = pixaGetCount(pixa);
        if (numpages != nfiles)
            fprintf(stderr, "numpages = %d, nfiles = %d, not equal!\n",
                    numpages, nfiles);
        for (i = 0; i < numpages; i++) {
            pix = pixaGetPix(pixa, i, L_CLONE);
            snprintf(filename, BUF_SIZE, "%s.%05d", rootname, i);
            fprintf(stderr, "filename: %s\n", filename);
            pixWrite(filename, pix, IFF_PNG);
            pixDestroy(&pix);
        }
        pixaDestroy(&pixa);
    }

    sarrayDestroy(&safiles);
    jbClasserDestroy(&classer);
    jbDataDestroy(&data);
    return 0;
}



/*------------------------------------------------------------------*
 *           Extract and classify words in textline order           *
 *------------------------------------------------------------------*/
/*!
 *  jbWordsInTextlines()
 *
 *      Input:  dirin (directory of input pages)
 *              maxwidth (of word mask components, to be kept)
 *              maxheight (of word mask components, to be kept)
 *              thresh (on correlation; 0.80 is reasonable)
 *              weight (for handling thick text; 0.6 is reasonable)
 *              natl (<return> numa with textline index for each component)
 *              firstpage (0-based)
 *              npages (use 0 for all pages in dirin)
 *      Return: classer (for the set of pages)
 *
 *  Notes:
 *      (1) This is a high-level function.  See prog/jbwords for example
 *          of usage.
 */
JBCLASSER *
jbWordsInTextlines(const char  *dirin,
		   l_int32      maxwidth,
		   l_int32      maxheight,
                   l_float32    thresh,
                   l_float32    weight,
		   NUMA       **pnatl,
		   l_int32      firstpage,
		   l_int32      npages)
{
char       *fname;
l_int32     nfiles, i;
BOXA       *boxa;
JBDATA     *data;
JBCLASSER  *classer;
NUMA       *nai, *natl;
PIX        *pix;
PIXA       *pixa;
SARRAY     *safiles;

    PROCNAME("jbWordsInTextlines");

    if (!dirin)
        return (JBCLASSER *)ERROR_PTR("dirin not defined", procName, NULL);
    if (!pnatl)
        return (JBCLASSER *)ERROR_PTR("&natl not defined", procName, NULL);

    safiles = getSortedPathnamesInDirectory(dirin, firstpage, npages);
    nfiles = sarrayGetCount(safiles);

        /* Classify components */
    classer = jbCorrelationInit(JB_WORDS, maxwidth, maxheight, thresh, weight);
    classer->safiles = sarrayCopy(safiles);
    natl = numaCreate(0);
    *pnatl = natl;
    for (i = 0; i < nfiles; i++) {
        fname = sarrayGetString(safiles, i, 0);
	if ((pix = pixRead(fname)) == NULL) {
            L_WARNING_INT("image file %d not read", procName, i);
            continue;
        }
        classer->w = pixGetWidth(pix);
        classer->h = pixGetHeight(pix);
        pixGetWordsInTextlines(pix, JB_WORDS_MIN_WIDTH, maxwidth, maxheight,
                               &boxa, &pixa, &nai);
	jbAddPageComponents(classer, pix, boxa, pixa);
	numaJoin(natl, nai, 0, 0);
	pixDestroy(&pix);
	numaDestroy(&nai);
	boxaDestroy(&boxa);
	pixaDestroy(&pixa);
    }

    sarrayDestroy(&safiles);
    return classer;
}


/*!
 *  pixGetWordsInTextlines()
 *
 *      Input:  pixs (1 bpp, 300 ppi)
 *              minwidth (of saved components)
 *              maxwidth, maxheight (of saved components; larger are discarded)
 *              &boxad (<return> word boxes sorted in textline line order)
 *              &pixad (<return> word images sorted in textline line order)
 *              &naindex (<return> index of textline for each word)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The input should be at a resolution of about 300 ppi.
 *          The word masks are computed at 2x reduction.
 *      (2) The pixa and boxa interfaces should make this type of
 *          application simple to put together.  The steps are:
 *           - generate first estimate of word masks
 *           - get b.b. of these, and remove the small ones
 *           - extract pixa of the word mask from these boxes
 *           - extract pixa of the actual word images, using word masks
 *           - sort actual word images in textline order (2d)
 *           - flatten them to a pixa (1d), saving the textline index
 *             for each pix
 *      (3) The result are word images (and their b.b.), extracted in
 *          textline order, and with a numa giving the textline index
 *          for each word.
 */
l_int32
pixGetWordsInTextlines(PIX     *pixs,
                       l_int32  minwidth,
                       l_int32  maxwidth,
                       l_int32  maxheight,
                       BOXA   **pboxad,
                       PIXA   **ppixad,
                       NUMA   **pnai)
{
BOXA     *boxa1, *boxa2, *boxad;
BOXAA    *baa;
NUMA     *nai;
NUMAA    *naa;
PIXA     *pixa1, *pixa2, *pixad;
PIX      *pixt1, *pixt2;
PIXAA    *paa;

    PROCNAME("pixGetWordsInTextlines");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pboxad || !ppixad || !pnai)
        return ERROR_INT("&boxad, &pixad, &nai not all defined", procName, 1);

        /* Work at about 150 ppi */
    pixt1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);

        /* First estimate of the word masks */
    pixt2 = pixWordMaskByDilation(pixt1, NULL);

        /* Get the bounding boxes of the words, and remove the
         * small ones, which can be due to punctuation that was
         * not joined to a word.  Here, pixa1 contains the
         * masks over each word.  */
    boxa1 = pixConnComp(pixt2, NULL, 8);
    boxa2 = boxaRemoveSmallComponents(boxa1, 5, 4, L_REMOVE_IF_EITHER, NULL);
    pixa1 = pixaCreateFromBoxa(pixt2, boxa2, NULL);

        /* Generate a pixa of the actual words images, not the mask images. */
    pixa2 = pixaClipToPix(pixa1, pixt1);

        /* Sort the bounding boxes of these words, saving the
         * index mapping that will allow us to sort the pixa identically. */
    baa = boxaSort2d(boxa2, &naa, -1, -1, 4);
    paa = pixaSort2dByIndex(pixa2, naa, L_CLONE);

        /* Flatten the word pixa */
    pixad = pixaaFlattenToPixa(paa, &nai, L_CLONE);
    boxad = pixaGetBoxa(pixad, L_COPY);

    *pnai = nai;
    *pboxad = boxad;
    *ppixad = pixad;

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixaDestroy(&pixa1);
    pixaDestroy(&pixa2);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);
    boxaaDestroy(&baa);
    pixaaDestroy(&paa);
    numaaDestroy(&naa);
    return 0;
}


