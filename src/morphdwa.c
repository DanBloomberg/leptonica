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
 *    Binary morphological (dwa) ops with brick Sels
 *         PIX     *pixDilateBrickDwa()
 *         PIX     *pixErodeBrickDwa()
 *         PIX     *pixOpenBrickDwa()
 *         PIX     *pixCloseBrickDwa()
 *
 *    Binary composite morphological (dwa) ops with brick Sels
 *         PIX     *pixDilateCompBrickDwa()
 *         PIX     *pixErodeCompBrickDwa()
 *         PIX     *pixOpenCompBrickDwa()
 *         PIX     *pixCloseCompBrickDwa()
 *
 *    These are higher-level interfaces for dwa morphology with brick Sels.
 *    Because many morphological operations are performed using
 *    separable brick Sels, it is useful to have a simple interface
 *    for this.
 *
 *    We have included all 58 of the brick Sels that are generated
 *    by selaAddBasic().  These are sufficient for all the decomposable
 *    bricks up to size 63, which is the limit for dwa Sels with
 *    origins at the center of the Sel.  If you try to apply a 
 *    non-decomposable operation with a Sel size that doesn't exist,
 *    the default is to call a decomposable operation instead.
 *
 *    We have also included use of all the 76 comb Sels that are generated
 *    by selaAddDwaCombs().  The generated code is in dwacomb.2.c
 *    and dwacomblow.2.c.  These are used for the composite dwa
 *    brick operations.
 *
 *    The non-composite brick operations, such as pixDilateBrickDwa(),
 *    will automatically default to the associated composite
 *    operation in situations where the requisite brick Sel has
 *    not been compiled into fmorphgen*.1.c.
 *
 *    If you want to use brick Sels that are not represented in the
 *    basic set of 58, you must generate the dwa code to implement them.
 *    You have three choices for how to use these:
 *
 *    (1) Add both the new Sels and the dwa code to the library:
 *        - For simplicity, add your new brick Sels to those defined
 *          in selaAddBasic().
 *        - Recompile the library.
 *        - Make prog/fmorphautogen.
 *        - Run prog/fmorphautogen, to generate new versions of the
 *          dwa code in fmorphgen.1.c and fmorphgenlow.1.c.
 *        - Copy these two files to src.
 *        - Recompile the library again.
 *        - Use the new brick Sels in your program and compile it.
 *
 *    (2) Make both the new Sels and dwa code outside the library,
 *        and link it directly to an executable:
 *        - Write a function to generate the new Sels in a Sela, and call
 *          fmorphautogen(sela, <N>, filename) to generate the code.
 *        - Compile your program that uses the newly generated function
 *          pixMorphDwa_<N>(), and link to the two new C files.
 *
 *    (3) Make the new Sels in the library and use the dwa code outside it:
 *        - Add code in the library to generate your new brick Sels.
 *          (It is suggested that you NOT add these Sels to the
 *          selaAddBasic() function; write a new function that generates
 *          a new Sela.)
 *        - Recompile the library.
 *        - Write a small program that generates the Sela and calls
 *          fmorphautogen(sela, <N>, filename) to generate the code.
 *        - Compile your program that uses the newly generated function
 *          pixMorphDwa_<N>(), and link to the two new C files.
 *       As an example of this approach, see prog/dwamorph*_reg.c:
 *        - added selaAddDwaLinear() to sel2.c
 *        - wrote dwamorph1_reg.c, to generate the dwa code.
 *        - compiled and linked the generated code with the application,
 *          dwamorph2_reg.c.  (Note: because this was a regression test,
 *          dwamorph1_reg also builds and runs the application program.)
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#ifndef  NO_CONSOLE_IO
#define  DEBUG_SEL_LOOKUP   0
#endif  /* ~NO_CONSOLE_IO */


/*-----------------------------------------------------------------*
 *           Binary morphological (dwa) ops with brick Sels        *
 *-----------------------------------------------------------------*/
/*!
 *  pixDilateBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement 2D brick Sels, using linear Sels generated
 *          with selaAddBasic().
 *      (2) A brick Sel has hits for all elements.
 *      (3) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (4) Do separably if both hsize and vsize are > 1.
 *      (5) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (6) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (7) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixDilateBrickDwa(NULL, pixs, ...);
 *          (b) pixDilateBrickDwa(pixs, pixs, ...);
 *          (c) pixDilateBrickDwa(pixd, pixs, ...);
 *      (8) The size of pixd is determined by pixs.
 */
PIX *
pixDilateBrickDwa(PIX     *pixd,
                  PIX     *pixs,
                  l_int32  hsize,
                  l_int32  vsize)
{
l_int32  found;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2, *pixt3;

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
    found = TRUE;
    if (hsize > 1) {
        selnameh = selaGetBrickName(sela, hsize, 1);
        if (!selnameh) found = FALSE;
    }
    if (vsize > 1) {
        selnamev = selaGetBrickName(sela, 1, vsize);
        if (!selnamev) found = FALSE;
    }
    selaDestroy(&sela);
    if (!found) {
        L_INFO("Calling the decomposable dwa function", procName);
	if (selnameh) FREE(selnameh);
	if (selnamev) FREE(selnamev);
	return pixDilateCompBrickDwa(pixd, pixs, hsize, vsize);
    }

    if (vsize == 1) {
        pixt2 = pixMorphDwa_1(NULL, pixs, L_MORPH_DILATE, selnameh);
        FREE(selnameh);
    }
    else if (hsize == 1) {
        pixt2 = pixMorphDwa_1(NULL, pixs, L_MORPH_DILATE, selnamev);
        FREE(selnamev);
    }
    else {
        pixt1 = pixAddBorder(pixs, 32, 0);
        pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh);
        pixFMorphopGen_1(pixt1, pixt3, L_MORPH_DILATE, selnamev);
        pixt2 = pixRemoveBorder(pixt1, 32);
        pixDestroy(&pixt1);
        pixDestroy(&pixt3);
        FREE(selnameh);
        FREE(selnamev);
    }

    if (!pixd)
        return pixt2;
    pixCopy(pixd, pixt2);
    pixDestroy(&pixt2);
    return pixd;
}


/*!
 *  pixErodeBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement 2D brick Sels, using linear Sels generated
 *          with selaAddBasic().
 *      (2) A brick Sel has hits for all elements.
 *      (3) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (4) Do separably if both hsize and vsize are > 1.
 *      (5) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (6) Note that we must always set or clear the border pixels
 *          before each operation, depending on the the b.c.
 *          (symmetric or asymmetric).
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixErodeBrickDwa(NULL, pixs, ...);
 *          (b) pixErodeBrickDwa(pixs, pixs, ...);
 *          (c) pixErodeBrickDwa(pixd, pixs, ...);
 *      (9) The size of the result is determined by pixs.
 */
PIX *
pixErodeBrickDwa(PIX     *pixd,
                 PIX     *pixs,
                 l_int32  hsize,
                 l_int32  vsize)
{
l_int32  found;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2, *pixt3;

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
    found = TRUE;
    if (hsize > 1) {
        selnameh = selaGetBrickName(sela, hsize, 1);
        if (!selnameh) found = FALSE;
    }
    if (vsize > 1) {
        selnamev = selaGetBrickName(sela, 1, vsize);
        if (!selnamev) found = FALSE;
    }
    selaDestroy(&sela);
    if (!found) {
        L_INFO("Calling the decomposable dwa function", procName);
	if (selnameh) FREE(selnameh);
	if (selnamev) FREE(selnamev);
	return pixErodeCompBrickDwa(pixd, pixs, hsize, vsize);
    }

    if (vsize == 1) {
        pixt2 = pixMorphDwa_1(NULL, pixs, L_MORPH_ERODE, selnameh);
        FREE(selnameh);
    }
    else if (hsize == 1) {
        pixt2 = pixMorphDwa_1(NULL, pixs, L_MORPH_ERODE, selnamev);
        FREE(selnamev);
    }
    else {
        pixt1 = pixAddBorder(pixs, 32, 0);
        pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh);
        pixFMorphopGen_1(pixt1, pixt3, L_MORPH_ERODE, selnamev);
        pixt2 = pixRemoveBorder(pixt1, 32);
        pixDestroy(&pixt1);
        pixDestroy(&pixt3);
        FREE(selnameh);
        FREE(selnamev);
    }

    if (!pixd)
        return pixt2;
    pixCopy(pixd, pixt2);
    pixDestroy(&pixt2);
    return pixd;
}


/*!
 *  pixOpenBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement 2D brick Sels, using linear Sels generated
 *          with selaAddBasic().
 *      (2) A brick Sel has hits for all elements.
 *      (3) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (4) Do separably if both hsize and vsize are > 1.
 *      (5) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (6) Note that we must always set or clear the border pixels
 *          before each operation, depending on the the b.c.
 *          (symmetric or asymmetric).
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixOpenBrickDwa(NULL, pixs, ...);
 *          (b) pixOpenBrickDwa(pixs, pixs, ...);
 *          (c) pixOpenBrickDwa(pixd, pixs, ...);
 *      (9) The size of the result is determined by pixs.
 */
PIX *
pixOpenBrickDwa(PIX     *pixd,
                PIX     *pixs,
                l_int32  hsize,
                l_int32  vsize)
{
l_int32  found;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2, *pixt3;

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
    found = TRUE;
    if (hsize > 1) {
        selnameh = selaGetBrickName(sela, hsize, 1);
        if (!selnameh) found = FALSE;
    }
    if (vsize > 1) {
        selnamev = selaGetBrickName(sela, 1, vsize);
        if (!selnamev) found = FALSE;
    }
    selaDestroy(&sela);
    if (!found) {
        L_INFO("Calling the decomposable dwa function", procName);
	if (selnameh) FREE(selnameh);
	if (selnamev) FREE(selnamev);
	return pixOpenCompBrickDwa(pixd, pixs, hsize, vsize);
    }

    pixt1 = pixAddBorder(pixs, 32, 0);
    if (vsize == 1) {   /* horizontal only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_OPEN, selnameh);
        FREE(selnameh);
    }
    else if (hsize == 1) {   /* vertical only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_OPEN, selnamev);
        FREE(selnamev);
    }
    else {  /* do separable */
        pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh);
        pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_ERODE, selnamev);
        pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnameh);
        pixFMorphopGen_1(pixt2, pixt3, L_MORPH_DILATE, selnamev);
        FREE(selnameh);
        FREE(selnamev);
        pixDestroy(&pixt3);
    }
    pixt3 = pixRemoveBorder(pixt2, 32);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    if (!pixd)
        return pixt3;
    pixCopy(pixd, pixt3);
    pixDestroy(&pixt3);
    return pixd;
}


/*!
 *  pixCloseBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) This is a 'safe' closing; we add an extra border of 32 OFF
 *          pixels for the standard asymmetric b.c.
 *      (2) These implement 2D brick Sels, using linear Sels generated
 *          with selaAddBasic().
 *      (3) A brick Sel has hits for all elements.
 *      (4) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (5) Do separably if both hsize and vsize are > 1.
 *      (6) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (7) Note that we must always set or clear the border pixels
 *          before each operation, depending on the the b.c.
 *          (symmetric or asymmetric).
 *      (8) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (9) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixCloseBrickDwa(NULL, pixs, ...);
 *          (b) pixCloseBrickDwa(pixs, pixs, ...);
 *          (c) pixCloseBrickDwa(pixd, pixs, ...);
 *      (10) The size of the result is determined by pixs.
 */
PIX *
pixCloseBrickDwa(PIX     *pixd,
                 PIX     *pixs,
                 l_int32  hsize,
                 l_int32  vsize)
{
l_int32  bordercolor, bordersize, found;
char    *selnameh, *selnamev;
SELA    *sela;
PIX     *pixt1, *pixt2, *pixt3;

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
    found = TRUE;
    if (hsize > 1) {
        selnameh = selaGetBrickName(sela, hsize, 1);
        if (!selnameh) found = FALSE;
    }
    if (vsize > 1) {
        selnamev = selaGetBrickName(sela, 1, vsize);
        if (!selnamev) found = FALSE;
    }
    selaDestroy(&sela);
    if (!found) {
        L_INFO("Calling the decomposable dwa function", procName);
	if (selnameh) FREE(selnameh);
	if (selnamev) FREE(selnamev);
	return pixCloseCompBrickDwa(pixd, pixs, hsize, vsize);
    }

        /* For "safe closing" with ASYMMETRIC_MORPH_BC, we always need
         * an extra 32 OFF pixels around the image (in addition to 
	 * the 32 added pixels for all dwa operations), whereas with
         * SYMMETRIC_MORPH_BC this is not necessary. */
    bordercolor = getMorphBorderPixelColor(L_MORPH_ERODE, 1);
    if (bordercolor == 0)   /* asymmetric b.c. */
        bordersize = 64;
    else   /* symmetric b.c. */
        bordersize = 32;
    pixt1 = pixAddBorder(pixs, bordersize, 0);

    if (vsize == 1) {   /* horizontal only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_CLOSE, selnameh);
        FREE(selnameh);
    }
    else if (hsize == 1) {   /* vertical only */
        pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_CLOSE, selnamev);
        FREE(selnamev);
    }
    else {  /* do separable */
        pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh);
        pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev);
        pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnameh);
        pixFMorphopGen_1(pixt2, pixt3, L_MORPH_ERODE, selnamev);
        FREE(selnameh);
        FREE(selnamev);
        pixDestroy(&pixt3);
    }
    pixt3 = pixRemoveBorder(pixt2, bordersize);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);

    if (!pixd)
        return pixt3;
    pixCopy(pixd, pixt3);
    pixDestroy(&pixt3);
    return pixd;
}


/*-----------------------------------------------------------------*
 *    Binary composite morphological (dwa) ops with brick Sels     *
 *-----------------------------------------------------------------*/
/*!
 *  pixDilateCompBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement a separable composite dilation with 2D brick Sels.
 *      (2) For efficiency, it may decompose each linear morphological
 *          operation into two (brick + comb).
 *      (3) A brick Sel has hits for all elements.
 *      (4) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (5) Do separably if both hsize and vsize are > 1.
 *      (6) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixDilateCompBrickDwa(NULL, pixs, ...);
 *          (b) pixDilateCompBrickDwa(pixs, pixs, ...);
 *          (c) pixDilateCompBrickDwa(pixd, pixs, ...);
 *      (9) The size of pixd is determined by pixs.
 *      (10) CAUTION: both hsize and vsize are being decomposed.
 *          The decomposer chooses a product of sizes (call them
 *          'terms') for each that is close to the input size,
 *           but not necessarily equal to it.  It attempts to optimize:
 *              (a) for consistency with the input values: the product
 *                  of terms is close to the input size
 *              (b) for efficiency of the operation: the sum of the
 *                  terms is small; ideally about twice the square
 *                   root of the input size.
 *           So, for example, if the input hsize = 37, which is
 *           a prime number, the decomposer will break this into two
 *           terms, 6 and 6, so that the net result is a dilation
 *           with hsize = 36.
 */
PIX *
pixDilateCompBrickDwa(PIX     *pixd,
                      PIX     *pixs,
                      l_int32  hsize,
                      l_int32  vsize)
{
char    *selnameh1, *selnameh2, *selnamev1, *selnamev2;
l_int32  hsize1, hsize2, vsize1, vsize2;
PIX     *pixt1, *pixt2, *pixt3;

    PROCNAME("pixDilateCompBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);
    if (hsize > 63 || vsize > 63)
        return (PIX *)ERROR_PTR("hsize and vsize not <= 63", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    hsize1 = hsize2 = vsize1 = vsize2 = 1;
    selnameh1 = selnameh2 = selnamev1 = selnamev2 = NULL;
    if (hsize > 1)
        getCompositeParameters(hsize, &hsize1, &hsize2, &selnameh1,
                               &selnameh2, NULL, NULL);
    if (vsize > 1)
        getCompositeParameters(vsize, &vsize1, &vsize2, NULL, NULL,
                               &selnamev1, &selnamev2);

#if DEBUG_SEL_LOOKUP
    fprintf(stderr, "nameh1=%s, nameh2=%s, namev1=%s, namev2=%s\n",
		    selnameh1, selnameh2, selnamev1, selnamev2);
    fprintf(stderr, "hsize1=%d, hsize2=%d, vsize1=%d, vsize2=%d\n",
		    hsize1, hsize2, vsize1, vsize2);
#endif  /* DEBUG_SEL_LOOKUP */

    pixt1 = pixAddBorder(pixs, 64, 0);
    if (vsize == 1) {
        if (hsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnameh2);
            pixDestroy(&pixt3);
        }
    }
    else if (hsize == 1) {
        if (vsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnamev1);
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnamev1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnamev2);
            pixDestroy(&pixt3);
        }
    }
    else {  /* vsize and hsize both > 1 */
        if (hsize2 == 1) 
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
        else {
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt3 = pixFMorphopGen_2(NULL, pixt2, L_MORPH_DILATE, selnameh2);
            pixDestroy(&pixt2);
        }
        if (vsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev1);
        else {
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt2, L_MORPH_DILATE, selnamev2);
        }
        pixDestroy(&pixt3);
    }
    pixDestroy(&pixt1);
    pixt1 = pixRemoveBorder(pixt2, 64);
    pixDestroy(&pixt2);
    if (selnameh1) FREE(selnameh1);
    if (selnameh2) FREE(selnameh2);
    if (selnamev1) FREE(selnamev1);
    if (selnamev2) FREE(selnamev2);

    if (!pixd)
        return pixt1;
    pixCopy(pixd, pixt1);
    pixDestroy(&pixt1);
    return pixd;
}


/*!
 *  pixErodeCompBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement a separable composite erosion with 2D brick Sels.
 *      (2) For efficiency, it may decompose each linear morphological
 *          operation into two (brick + comb).
 *      (3) A brick Sel has hits for all elements.
 *      (4) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (5) Do separably if both hsize and vsize are > 1.
 *      (6) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixErodeCompBrickDwa(NULL, pixs, ...);
 *          (b) pixErodeCompBrickDwa(pixs, pixs, ...);
 *          (c) pixErodeCompBrickDwa(pixd, pixs, ...);
 *      (9) The size of pixd is determined by pixs.
 *      (10) CAUTION: both hsize and vsize are being decomposed.
 *          The decomposer chooses a product of sizes (call them
 *          'terms') for each that is close to the input size,
 *           but not necessarily equal to it.  It attempts to optimize:
 *              (a) for consistency with the input values: the product
 *                  of terms is close to the input size
 *              (b) for efficiency of the operation: the sum of the
 *                  terms is small; ideally about twice the square
 *                   root of the input size.
 *           So, for example, if the input hsize = 37, which is
 *           a prime number, the decomposer will break this into two
 *           terms, 6 and 6, so that the net result is a dilation
 *           with hsize = 36.
 */
PIX *
pixErodeCompBrickDwa(PIX     *pixd,
                     PIX     *pixs,
                     l_int32  hsize,
                     l_int32  vsize)
{
char    *selnameh1, *selnameh2, *selnamev1, *selnamev2;
l_int32  hsize1, hsize2, vsize1, vsize2;
PIX     *pixt1, *pixt2, *pixt3;

    PROCNAME("pixErodeCompBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);
    if (hsize > 63 || vsize > 63)
        return (PIX *)ERROR_PTR("hsize and vsize not <= 63", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    hsize1 = hsize2 = vsize1 = vsize2 = 1;
    selnameh1 = selnameh2 = selnamev1 = selnamev2 = NULL;
    if (hsize > 1)
        getCompositeParameters(hsize, &hsize1, &hsize2, &selnameh1,
                               &selnameh2, NULL, NULL);
    if (vsize > 1)
        getCompositeParameters(vsize, &vsize1, &vsize2, NULL, NULL,
                               &selnamev1, &selnamev2);

    pixt1 = pixAddBorder(pixs, 64, 0);
    if (vsize == 1) {
        if (hsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnameh2);
            pixDestroy(&pixt3);
        }
    }
    else if (hsize == 1) {
        if (vsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnamev1);
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnamev1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnamev2);
            pixDestroy(&pixt3);
        }
    }
    else {  /* vsize and hsize both > 1 */
        if (hsize2 == 1) 
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
        else {
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt3 = pixFMorphopGen_2(NULL, pixt2, L_MORPH_ERODE, selnameh2);
            pixDestroy(&pixt2);
        }
        if (vsize2 == 1) 
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_ERODE, selnamev1);
        else {
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt2, L_MORPH_ERODE, selnamev2);
        }
        pixDestroy(&pixt3);
    }
    pixDestroy(&pixt1);
    pixt1 = pixRemoveBorder(pixt2, 64);
    pixDestroy(&pixt2);
    if (selnameh1) FREE(selnameh1);
    if (selnameh2) FREE(selnameh2);
    if (selnamev1) FREE(selnamev1);
    if (selnamev2) FREE(selnamev2);

    if (!pixd)
        return pixt1;
    pixCopy(pixd, pixt1);
    pixDestroy(&pixt1);
    return pixd;
}


/*!
 *  pixOpenCompBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) These implement a separable composite opening with 2D brick Sels.
 *      (2) For efficiency, it may decompose each linear morphological
 *          operation into two (brick + comb).
 *      (3) A brick Sel has hits for all elements.
 *      (4) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (5) Do separably if both hsize and vsize are > 1.
 *      (6) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixOpenCompBrickDwa(NULL, pixs, ...);
 *          (b) pixOpenCompBrickDwa(pixs, pixs, ...);
 *          (c) pixOpenCompBrickDwa(pixd, pixs, ...);
 *      (9) The size of pixd is determined by pixs.
 *      (10) CAUTION: both hsize and vsize are being decomposed.
 *          The decomposer chooses a product of sizes (call them
 *          'terms') for each that is close to the input size,
 *           but not necessarily equal to it.  It attempts to optimize:
 *              (a) for consistency with the input values: the product
 *                  of terms is close to the input size
 *              (b) for efficiency of the operation: the sum of the
 *                  terms is small; ideally about twice the square
 *                   root of the input size.
 *           So, for example, if the input hsize = 37, which is
 *           a prime number, the decomposer will break this into two
 *           terms, 6 and 6, so that the net result is a dilation
 *           with hsize = 36.
 */
PIX *
pixOpenCompBrickDwa(PIX     *pixd,
                    PIX     *pixs,
                    l_int32  hsize,
                    l_int32  vsize)
{
char    *selnameh1, *selnameh2, *selnamev1, *selnamev2;
l_int32  hsize1, hsize2, vsize1, vsize2;
PIX     *pixt1, *pixt2, *pixt3;

    PROCNAME("pixOpenCompBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);
    if (hsize > 63 || vsize > 63)
        return (PIX *)ERROR_PTR("hsize and vsize not <= 63", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    hsize1 = hsize2 = vsize1 = vsize2 = 1;
    selnameh1 = selnameh2 = selnamev1 = selnamev2 = NULL;
    if (hsize > 1)
        getCompositeParameters(hsize, &hsize1, &hsize2, &selnameh1,
                               &selnameh2, NULL, NULL);
    if (vsize > 1)
        getCompositeParameters(vsize, &vsize1, &vsize2, NULL, NULL,
                               &selnamev1, &selnamev2);

    pixt1 = pixAddBorder(pixs, 64, 0);
    if (vsize == 1) {
        if (hsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnameh1);
        }
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnameh1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnameh2);
        }
    }
    else if (hsize == 1) {
        if (vsize2 == 1)  {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnamev1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev1);
	}
       	else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnamev1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnamev2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnamev2);
        }
    }
    else {  /* vsize and hsize both > 1 */
        if (hsize2 == 1 && vsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnameh1);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_DILATE, selnamev1);
        }
        else if (vsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_DILATE, selnameh1);
            pixFMorphopGen_2(pixt3, pixt2, L_MORPH_DILATE, selnameh2);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_DILATE, selnamev1);
        }
        else if (hsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt3, pixt2, L_MORPH_ERODE, selnamev2);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_DILATE, selnameh1);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnamev2);
        }
        else {   /* both directions are combed */
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_ERODE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_ERODE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnamev2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnameh1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnamev2);
        }
    }
    pixDestroy(&pixt3);

    pixDestroy(&pixt1);
    pixt1 = pixRemoveBorder(pixt2, 64);
    pixDestroy(&pixt2);
    if (selnameh1) FREE(selnameh1);
    if (selnameh2) FREE(selnameh2);
    if (selnamev1) FREE(selnamev1);
    if (selnamev2) FREE(selnamev2);

    if (!pixd)
        return pixt1;
    pixCopy(pixd, pixt1);
    pixDestroy(&pixt1);
    return pixd;
}


/*!
 *  pixCloseCompBrickDwa()
 *
 *      Input:  pixd  (<optional>; this can be null, equal to pixs,
 *                     or different from pixs)
 *              pixs (1 bpp)
 *              hsize (width of brick Sel)
 *              vsize (height of brick Sel)
 *      Return: pixd
 * 
 *  Notes:
 *      (1) This implements a separable composite safe closing with 2D
 *          brick Sels.
 *      (2) For efficiency, it may decompose each linear morphological
 *          operation into two (brick + comb).
 *      (3) A brick Sel has hits for all elements.
 *      (4) The origin of the Sel is at (x, y) = (hsize/2, vsize/2)
 *      (5) Do separably if both hsize and vsize are > 1.
 *      (6) It is necessary that both horizontal and vertical Sels
 *          of the input size are defined in the basic sela.
 *      (7) There are three cases:
 *          (a) pixd == null   (result into new pixd)
 *          (b) pixd == pixs   (in-place; writes result back to pixs)
 *          (c) pixd != pixs   (puts result into existing pixd)
 *      (8) For clarity, if the case is known, use these patterns:
 *          (a) pixd = pixCloseCompBrickDwa(NULL, pixs, ...);
 *          (b) pixCloseCompBrickDwa(pixs, pixs, ...);
 *          (c) pixCloseCompBrickDwa(pixd, pixs, ...);
 *      (9) The size of pixd is determined by pixs.
 *      (10) CAUTION: both hsize and vsize are being decomposed.
 *          The decomposer chooses a product of sizes (call them
 *          'terms') for each that is close to the input size,
 *           but not necessarily equal to it.  It attempts to optimize:
 *              (a) for consistency with the input values: the product
 *                  of terms is close to the input size
 *              (b) for efficiency of the operation: the sum of the
 *                  terms is small; ideally about twice the square
 *                   root of the input size.
 *           So, for example, if the input hsize = 37, which is
 *           a prime number, the decomposer will break this into two
 *           terms, 6 and 6, so that the net result is a dilation
 *           with hsize = 36.
 */
PIX *
pixCloseCompBrickDwa(PIX     *pixd,
                     PIX     *pixs,
                     l_int32  hsize,
                     l_int32  vsize)
{
char    *selnameh1, *selnameh2, *selnamev1, *selnamev2;
l_int32  hsize1, hsize2, vsize1, vsize2;
PIX     *pixt1, *pixt2, *pixt3;

    PROCNAME("pixCloseCompBrickDwa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, pixd);
    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not 1 bpp", procName, pixd);
    if (hsize < 1 || vsize < 1)
        return (PIX *)ERROR_PTR("hsize and vsize not >= 1", procName, pixd);
    if (hsize > 63 || vsize > 63)
        return (PIX *)ERROR_PTR("hsize and vsize not <= 63", procName, pixd);

    if (hsize == 1 && vsize == 1)
        return pixCopy(pixd, pixs);

    hsize1 = hsize2 = vsize1 = vsize2 = 1;
    selnameh1 = selnameh2 = selnamev1 = selnamev2 = NULL;
    if (hsize > 1)
        getCompositeParameters(hsize, &hsize1, &hsize2, &selnameh1,
                               &selnameh2, NULL, NULL);
    if (vsize > 1)
        getCompositeParameters(vsize, &vsize1, &vsize2, NULL, NULL,
                               &selnamev1, &selnamev2);

    pixt3 = NULL;
    pixt1 = pixAddBorder(pixs, 64, 0);
    if (vsize == 1) {
        if (hsize2 == 1)
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_CLOSE, selnameh1);
        else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnameh1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnameh2);
        }
    }
    else if (hsize == 1) {
        if (vsize2 == 1)
            pixt2 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_CLOSE, selnamev1);
       	else {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnamev1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnamev2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnamev2);
        }
    }
    else {  /* vsize and hsize both > 1 */
        if (hsize2 == 1 && vsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnameh1);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_ERODE, selnamev1);
        }
        else if (vsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_ERODE, selnameh1);
            pixFMorphopGen_2(pixt3, pixt2, L_MORPH_ERODE, selnameh2);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_ERODE, selnamev1);
        }
        else if (hsize2 == 1) {
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_1(NULL, pixt3, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt3, pixt2, L_MORPH_DILATE, selnamev2);
            pixFMorphopGen_1(pixt2, pixt3, L_MORPH_ERODE, selnameh1);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnamev2);
        }
        else {   /* both directions are combed */
            pixt3 = pixFMorphopGen_1(NULL, pixt1, L_MORPH_DILATE, selnameh1);
            pixt2 = pixFMorphopGen_2(NULL, pixt3, L_MORPH_DILATE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_DILATE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_DILATE, selnamev2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnameh1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnameh2);
            pixFMorphopGen_1(pixt3, pixt2, L_MORPH_ERODE, selnamev1);
            pixFMorphopGen_2(pixt2, pixt3, L_MORPH_ERODE, selnamev2);
        }
    }
    pixDestroy(&pixt3);

    pixDestroy(&pixt1);
    pixt1 = pixRemoveBorder(pixt2, 64);
    pixDestroy(&pixt2);
    if (selnameh1) FREE(selnameh1);
    if (selnameh2) FREE(selnameh2);
    if (selnamev1) FREE(selnamev1);
    if (selnamev2) FREE(selnamev2);

    if (!pixd)
        return pixt1;
    pixCopy(pixd, pixt1);
    pixDestroy(&pixt1);
    return pixd;
}


