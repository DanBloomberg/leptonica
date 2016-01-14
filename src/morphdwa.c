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
 *  morphdwa.c
 *
 *    Special binary morphological (dwa) ops with brick Sels
 *         PIX     *pixDilateBrickDwa()
 *         PIX     *pixErodeBrickDwa()
 *         PIX     *pixOpenBrickDwa()
 *         PIX     *pixCloseBrickDwa()
 *
 *    This is a higher-level interface for dwa morphology with brick Sels.
 *    Because many morphological operations are performed using
 *    separable brick Sels, it is useful to have a simple interface
 *    for this.
 *
 *    We have included all 52 of the brick Sels that are generated
 *    by selaAddBasic().  The only ones used here are the one-dimensional
 *    Sels (either horiz or vert).  If you want to use Sels that have
 *    a different width or height, you need to generate the dwa code
 *    to implement them.  Here's what you do:
 *
 *    (1) Add the new brick Sels to the sets of linear horizontal sels
 *        and linear vertical sels.
 *    (2) Recompile the library.
 *    (3) Make prog/fmorphautogen.
 *    (4) Run prog/fmorphautogen, to generate new versions of the
 *        dwa code in fmorphgen.1.c and fmorphgenlow.1.c.
 *    (5) Copy these two files to src.
 *    (6) Recompile the library again.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"



/*-----------------------------------------------------------------*
 *      Special binary morphological (dwa) ops with brick Sels     *
 *-----------------------------------------------------------------*/
/*!
 *  pixDilateBrickDwa()
 *
 *      Input:  pixd  (<optional>)
 *              pixs
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 *
 *  Notes:
 *      (1) Sel is a brick with all elements being hits
 *      (2) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (3) Do separably if both hsize and vsize are > 1.
 *      (4) Three modes of usage:
 *          - pixd = NULL : result into new pixd, which is returned
 *          - pixd exists, != pixs : puts result into pixd
 *          - pixd == pixs : in-place operation; writes result back to pixs
 */
PIX *
pixDilateBrickDwa(PIX     *pixd,
                  PIX     *pixs,
                  l_int32  hsize,
                  l_int32  vsize)
{
char  *selnameh, *selnamev;
SELA  *sela;
PIX   *pixt1, *pixt2, *pixt3, *pixt4;

    PROCNAME("pixDilateBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    sela = selaAddBasic(NULL);
    if (hsize > 1) {
        if ((selnameh = selaGetBrickName(sela, hsize, 1)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa h-sel not defined", procName, pixd);
        }
    }
    if (vsize > 1) {
        if ((selnamev = selaGetBrickName(sela, 1, vsize)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa v-sel not defined", procName, pixd);
        }
    }
    selaDestroy(&sela);

    pixt1 = pixAddBorder(pixs, ADDED_BORDER, 0);
    if (hsize > 1) {
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh);
        FREE(selnameh);
    }
    else
        pixt2 = pixClone(pixt1);
    if (vsize > 1) {
        pixt3 = pixFMorphopGen_1(NULL, pixt2, L_MORPH_DILATE, selnamev);
        FREE(selnamev);
    }
    else
        pixt3 = pixClone(pixt2);
    pixt4 = pixRemoveBorder(pixt3, ADDED_BORDER);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);

    if (!pixd)
        return pixt4;
    pixCopy(pixd, pixt4);
    pixDestroy(&pixt4);
    return pixd;
}


/*!
 *  pixErodeBrickDwa()
 *
 *      Input:  pixd  (<optional>)
 *              pixs
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 *
 *  Notes:
 *      (1) Sel is a brick with all elements being hits
 *      (2) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (3) Do separably if both hsize and vsize are > 1.
 *      (4) Note that we must always set or clear the border pixels
 *          before each operation, depending on the the b.c.
 *          (symmetric or asymmetric).
 *      (5) Three modes of usage:
 *          - pixd = NULL : result into new pixd, which is returned
 *          - pixd exists, != pixs : puts result into pixd
 *          - pixd == pixs : in-place operation; writes result back to pixs
 */
PIX *
pixErodeBrickDwa(PIX     *pixd,
                 PIX     *pixs,
                 l_int32  hsize,
                 l_int32  vsize)
{
l_int32  bordercolor, erodeop;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2, *pixt3, *pixt4;

    PROCNAME("pixErodeBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    sela = selaAddBasic(NULL);
    if (hsize > 1) {
        if ((selnameh = selaGetBrickName(sela, hsize, 1)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa h-sel not defined", procName, pixd);
        }
    }
    if (vsize > 1) {
        if ((selnamev = selaGetBrickName(sela, 1, vsize)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa v-sel not defined", procName, pixd);
        }
    }
    selaDestroy(&sela);

    bordercolor = getMorphBorderPixelColor(L_MORPH_ERODE, 1);
    if (bordercolor == 1)
        erodeop = PIX_SET;
    else
        erodeop = PIX_CLR;
    pixt1 = pixAddBorder(pixs, ADDED_BORDER, bordercolor);
    if (hsize > 1) {
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        FREE(selnameh);
    }
    else
        pixt2 = pixClone(pixt1);
    if (vsize > 1) {
        pixt3 = pixFMorphopGen_1(NULL, pixt2, L_MORPH_ERODE, selnamev);
        FREE(selnamev);
    }
    else
        pixt3 = pixClone(pixt2);
    pixt4 = pixRemoveBorder(pixt3, ADDED_BORDER);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    pixDestroy(&pixt3);

    if (!pixd)
        return pixt4;
    pixCopy(pixd, pixt4);
    pixDestroy(&pixt4);
    return pixd;
}


/*!
 *  pixOpenBrickDwa()
 *
 *      Input:  pixd  (<optional>)
 *              pixs
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 *
 *  Notes:
 *      (1) Sel is a brick with all elements being hits
 *      (2) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (3) Do separably if both hsize and vsize are > 1.
 *      (4) Note that we must always set or clear the border pixels
 *          before each operation, depending on the type of operation
 *          and the b.c. (symmetric or asymmetric).
 *      (5) Three modes of usage:
 *          - pixd = NULL : result into new pixd, which is returned
 *          - pixd exists, != pixs : puts result into pixd
 *          - pixd == pixs : in-place operation; writes result back to pixs
 */
PIX *
pixOpenBrickDwa(PIX     *pixd,
                PIX     *pixs,
                l_int32  hsize,
                l_int32  vsize)
{
l_int32  bordercolor, erodeop;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2;

    PROCNAME("pixOpenBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    sela = selaAddBasic(NULL);
    if (hsize > 1) {
        if ((selnameh = selaGetBrickName(sela, hsize, 1)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa h-sel not defined", procName, pixd);
        }
    }
    if (vsize > 1) {
        if ((selnamev = selaGetBrickName(sela, 1, vsize)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa v-sel not defined", procName, pixd);
        }
    }
    selaDestroy(&sela);

    bordercolor = getMorphBorderPixelColor(L_MORPH_ERODE, 1);
    if (bordercolor == 1)
        erodeop = PIX_SET;
    else
        erodeop = PIX_CLR;
    pixt1 = pixAddBorder(pixs, ADDED_BORDER, bordercolor);
    if (vsize == 1) {   /* horizontal only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, PIX_CLR);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_DILATE, selnameh);
        FREE(selnameh);
        pixDestroy(&pixt2);
    }
    else if (hsize == 1) {   /* vertical only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnamev);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, PIX_CLR);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_DILATE, selnamev);
        FREE(selnamev);
        pixDestroy(&pixt2);
    }
    else {  /* do separable */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_ERODE, selnamev);
        pixSetOrClearBorder(pixt1, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, PIX_CLR);
        pixFMorphopGen_1(pixt2, pixt1, L_MORPH_DILATE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, PIX_CLR);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_DILATE, selnamev);
        FREE(selnameh);
        FREE(selnamev);
        pixDestroy(&pixt2);
    }
    pixt2 = pixRemoveBorder(pixt1, ADDED_BORDER);
    pixDestroy(&pixt1);

    if (!pixd)
        return pixt2;
    pixCopy(pixd, pixt2);
    pixDestroy(&pixt2);
    return pixd;
}


/*!
 *  pixCloseBrickDwa()
 *
 *      Input:  pixd  (<optional>)
 *              pixs
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 *
 *  Notes:
 *      (1) Sel is a brick with all elements being hits
 *      (2) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (3) Do separably if both hsize and vsize are > 1.
 *      (4) This is a 'safe' closing; we add an extra border of 32 OFF
 *          pixels for the standard asymmetric b.c.
 *      (5) Note that we must always set or clear the border pixels
 *          before each operation, depending on the type of operation
 *          and the b.c. (symmetric or asymmetric).
 *      (6) Three modes of usage:
 *          - pixd = NULL : result into new pixd, which is returned
 *          - pixd exists, != pixs : puts result into pixd
 *          - pixd == pixs : in-place operation; writes result back to pixs
 */
PIX *
pixCloseBrickDwa(PIX     *pixd,
                 PIX     *pixs,
                 l_int32  hsize,
                 l_int32  vsize)
{
l_int32  bordercolor, erodeop, bordersize;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2;

    PROCNAME("pixCloseBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    sela = selaAddBasic(NULL);
    if (hsize > 1) {
        if ((selnameh = selaGetBrickName(sela, hsize, 1)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa h-sel not defined", procName, pixd);
        }
    }
    if (vsize > 1) {
        if ((selnamev = selaGetBrickName(sela, 1, vsize)) == NULL) {
            selaDestroy(&sela);
            return (PIX *)ERROR_PTR("dwa v-sel not defined", procName, pixd);
        }
    }
    selaDestroy(&sela);

        /* For "safe closing" with ASYMMETRIC_MORPH_BC, we always need
         * an extra 32 OFF pixels around the image, whereas with
         * SYMMETRIC_MORPH_BC this is not necessary. */
    bordercolor = getMorphBorderPixelColor(L_MORPH_ERODE, 1);
    if (bordercolor == 0) {  /* asymmetric b.c. */
        erodeop = PIX_CLR;
        bordersize = 2 * ADDED_BORDER;  /* we need the extra 32 pixels of 0 */
    }
    else {  /* symmetric b.c. */
        erodeop = PIX_SET;
        bordersize = ADDED_BORDER;  /* just need 32 pixels */
    }
    pixt1 = pixAddBorder(pixs, bordersize, 0);

    if (vsize == 1) {   /* horizontal only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_ERODE, selnameh);
        FREE(selnameh);
        pixDestroy(&pixt2);
    }
    else if (hsize == 1) {   /* vertical only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnamev);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_ERODE, selnamev);
        FREE(selnamev);
        pixDestroy(&pixt2);
    }
    else {  /* do separable */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, PIX_CLR);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_DILATE, selnamev);
        pixSetOrClearBorder(pixt1, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        pixFMorphopGen_1(pixt2, pixt1, L_MORPH_ERODE, selnameh);
        pixSetOrClearBorder(pixt2, ADDED_BORDER, ADDED_BORDER, 
                            ADDED_BORDER, ADDED_BORDER, erodeop);
        pixFMorphopGen_1(pixt1, pixt2, L_MORPH_ERODE, selnamev);
        FREE(selnameh);
        FREE(selnamev);
        pixDestroy(&pixt2);
    }
    pixt2 = pixRemoveBorder(pixt1, bordersize);
    pixDestroy(&pixt1);

    if (!pixd)
        return pixt2;
    pixCopy(pixd, pixt2);
    pixDestroy(&pixt2);
    return pixd;
}


