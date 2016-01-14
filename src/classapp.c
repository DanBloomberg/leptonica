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
 *         l_int32      pixGetWordBoxesInTextlines()
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  BUF_SIZE = 512;
static const l_int32  JB_WORDS_MIN_WIDTH = 5;  /* pixels */
static const l_int32  JB_WORDS_MIN_HEIGHT = 3;  /* pixels */


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

    safiles = getSortedPathnamesInDirectory(dirin, NULL, firstpage, npages);
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

    safiles = getSortedPathnamesInDirectory(dirin, NULL, firstpage, npages);
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
 *              reduction (1 for full res; 2 for half-res)
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
 *      (2) Typically, words can be found reasonably well at a resolution
 *          of about 150 ppi.  For highest accuracy, you should use 300 ppi.
 *          Assuming that the input images are 300 ppi, use reduction = 1
 *          for finding words at full res, and reduction = 2 for finding
 *          them at 150 ppi.
 */
JBCLASSER *
jbWordsInTextlines(const char  *dirin,
                   l_int32      reduction,
                   l_int32      maxwidth,
                   l_int32      maxheight,
                   l_float32    thresh,
                   l_float32    weight,
                   NUMA       **pnatl,
                   l_int32      firstpage,
                   l_int32      npages)
{
char       *fname;
l_int32     nfiles, i, w, h;
BOXA       *boxa;
JBCLASSER  *classer;
NUMA       *nai, *natl;
PIX        *pix;
PIXA       *pixa;
SARRAY     *safiles;

    PROCNAME("jbWordsInTextlines");

    if (!pnatl)
        return (JBCLASSER *)ERROR_PTR("&natl not defined", procName, NULL);
    *pnatl = NULL;
    if (!dirin)
        return (JBCLASSER *)ERROR_PTR("dirin not defined", procName, NULL);
    if (reduction != 1 && reduction != 2)
        return (JBCLASSER *)ERROR_PTR("reduction not in {1,2}", procName, NULL);

    safiles = getSortedPathnamesInDirectory(dirin, NULL, firstpage, npages);
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
	pixGetDimensions(pix, &w, &h, NULL);
        if (reduction == 1) {
            classer->w = w;
            classer->h = h;
        }
	else {  /* reduction == 2 */
            classer->w = w / 2;
            classer->h = h / 2;
        }
        pixGetWordsInTextlines(pix, reduction, JB_WORDS_MIN_WIDTH,
                               JB_WORDS_MIN_HEIGHT, maxwidth, maxheight,
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
 *              reduction (1 for full res; 2 for half-res)
 *              minwidth, minheight (of saved components; smaller are discarded)
 *              maxwidth, maxheight (of saved components; larger are discarded)
 *              &boxad (<return> word boxes sorted in textline line order)
 *              &pixad (<return> word images sorted in textline line order)
 *              &naindex (<return> index of textline for each word)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The input should be at a resolution of about 300 ppi.
 *          The word masks can be computed at either 150 ppi or 300 ppi.
 *          For the former, set reduction = 2.
 *      (2) The four size constraints on saved components are all
 *          used at 2x reduction.
 *      (3) The result are word images (and their b.b.), extracted in
 *          textline order, all at 2x reduction, and with a numa giving
 *          the textline index for each word.
 *      (4) The pixa and boxa interfaces should make this type of
 *          application simple to put together.  The steps are:
 *           - generate first estimate of word masks
 *           - get b.b. of these, and remove the small and big ones
 *           - extract pixa of the word mask from these boxes
 *           - extract pixa of the actual word images, using word masks
 *           - sort actual word images in textline order (2d)
 *           - flatten them to a pixa (1d), saving the textline index
 *             for each pix
 *      (5) In an actual application, it may be desirable to pre-filter
 *          the input image to remove large components, to extract
 *          single columns of text, and to deskew them.
 */
l_int32
pixGetWordsInTextlines(PIX     *pixs,
                       l_int32  reduction,
                       l_int32  minwidth,
                       l_int32  minheight,
                       l_int32  maxwidth,
                       l_int32  maxheight,
                       BOXA   **pboxad,
                       PIXA   **ppixad,
                       NUMA   **pnai)
{
l_int32  maxsize;
BOXA    *boxa1, *boxa2, *boxa3, *boxad;
BOXAA   *baa;
NUMA    *nai;
NUMAA   *naa;
PIXA    *pixa1, *pixa2, *pixad;
PIX     *pixt1, *pixt2;
PIXAA   *paa;

    PROCNAME("pixGetWordsInTextlines");

    if (!pboxad || !ppixad || !pnai)
        return ERROR_INT("&boxad, &pixad, &nai not all defined", procName, 1);
    *pboxad = NULL;
    *ppixad = NULL;
    *pnai = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (reduction != 1 && reduction != 2)
        return ERROR_INT("reduction not in {1,2}", procName, 1);

    if (reduction == 1) {
        pixt1 = pixClone(pixs);
        maxsize = 14;
    }
    else {  /* reduction == 2 */
        pixt1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
        maxsize = 7;
    }

        /* First estimate of the word masks */
    pixt2 = pixWordMaskByDilation(pixt1, maxsize, NULL);

        /* Get the bounding boxes of the words. First remove the
         * small ones, which can be due to punctuation that was
         * not joined to a word.  Then remove the large ones, which are
         * also not likely to be words.  Here, pixa1 contains
         * the masks over each word.  */
    boxa1 = pixConnComp(pixt2, NULL, 8);
    boxa2 = boxaSelectBySize(boxa1, minwidth, minheight, L_SELECT_IF_BOTH,
                             L_SELECT_IF_GTE, NULL);
    boxa3 = boxaSelectBySize(boxa2, maxwidth, maxheight, L_SELECT_IF_BOTH,
                             L_SELECT_IF_LTE, NULL);
    pixa1 = pixaCreateFromBoxa(pixt2, boxa3, NULL);

        /* Generate a pixa of the actual word images, not the mask images. */
    pixa2 = pixaClipToPix(pixa1, pixt1);

        /* Sort the bounding boxes of these words, saving the
         * index mapping that will allow us to sort the pixa identically. */
    baa = boxaSort2d(boxa3, &naa, -1, -1, 4);
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
    boxaDestroy(&boxa3);
    boxaaDestroy(&baa);
    pixaaDestroy(&paa);
    numaaDestroy(&naa);
    return 0;
}


/*!
 *  pixGetWordBoxesInTextlines()
 *
 *      Input:  pixs (1 bpp, 300 ppi)
 *              reduction (1 for full res; 2 for half-res)
 *              minwidth, minheight (of saved components; smaller are discarded)
 *              maxwidth, maxheight (of saved components; larger are discarded)
 *              &boxad (<return> word boxes sorted in textline line order)
 *              &naindex (<return> index of textline for each word)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The input should be at a resolution of about 300 ppi.
 *          The word masks can be computed at either 150 ppi or 300 ppi.
 *          For the former, set reduction = 2.
 *      (2) In an actual application, it may be desirable to pre-filter
 *          the input image to remove large components, to extract
 *          single columns of text, and to deskew them.
 *      (3) This is a special version that just finds the word boxes
 *          in line order, with a numa giving the textline index for
 *          each word.  See pixGetWordsInTextlines() for more details.
 */
l_int32
pixGetWordBoxesInTextlines(PIX     *pixs,
                           l_int32  reduction,
                           l_int32  minwidth,
                           l_int32  minheight,
                           l_int32  maxwidth,
                           l_int32  maxheight,
                           BOXA   **pboxad,
                           NUMA   **pnai)
{
l_int32  maxsize;
BOXA    *boxa1, *boxa2, *boxa3, *boxad;
BOXAA   *baa;
NUMA    *nai;
PIX     *pixt1, *pixt2;

    PROCNAME("pixGetWordBoxesInTextlines");

    if (!pboxad || !pnai)
        return ERROR_INT("&boxad and &nai not both defined", procName, 1);
    *pboxad = NULL;
    *pnai = NULL;
    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (reduction != 1 && reduction != 2)
        return ERROR_INT("reduction not in {1,2}", procName, 1);

    if (reduction == 1) {
        pixt1 = pixClone(pixs);
        maxsize = 14;
    }
    else {  /* reduction == 2 */
        pixt1 = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
        maxsize = 7;
    }

        /* First estimate of the word masks */
    pixt2 = pixWordMaskByDilation(pixt1, maxsize, NULL);

        /* Get the bounding boxes of the words, and remove the
         * small ones, which can be due to punctuation that was
         * not joined to a word, and the large ones, which are
         * also not likely to be words. */
    boxa1 = pixConnComp(pixt2, NULL, 8);
    boxa2 = boxaSelectBySize(boxa1, minwidth, minheight,
                             L_SELECT_IF_BOTH, L_SELECT_IF_GTE, NULL);
    boxa3 = boxaSelectBySize(boxa2, maxwidth, maxheight,
                             L_SELECT_IF_BOTH, L_SELECT_IF_LTE, NULL);

        /* 2D sort the bounding boxes of these words. */
    baa = boxaSort2d(boxa3, NULL, 3, -5, 5);

        /* Flatten the boxaa, saving the boxa index for each box */
    boxad = boxaaFlattenToBoxa(baa, &nai, L_CLONE);

    *pnai = nai;
    *pboxad = boxad;

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    boxaDestroy(&boxa1);
    boxaDestroy(&boxa2);
    boxaDestroy(&boxa3);
    boxaaDestroy(&baa);
    return 0;
}


