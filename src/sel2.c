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
 *  sel2.c
 *
 *      Contains definitions of simple structuring elements
 *
 *          SELA    *selaAddBasic()
 *               Linear horizontal and vertical
 *               Square
 *               Diagonals
 *
 *          SELA    *selaAddHitMiss()
 *               Isolated foreground pixel
 *               Horizontal and vertical edges
 *               Slanted edge
 *
 *          SELA    *selaAddDwaLinear()
 *          SELA    *selaAddDwaCombs()
 */

#include <stdio.h>
#include "allheaders.h"

    /* MSVC can't handle arrays dimensioned by static const integers */
#define  L_BUF_SIZE  512

    /* Linear brick sel sizes, including all those that are required
     * for decomposable sels up to size 63. */
static const l_int32  num_linear = 25;
static const l_int32  basic_linear[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 21, 25, 30, 31, 35, 40, 41, 45, 50, 51};


/*! 
 *  selaAddBasic()
 *
 *      Input:  sela (<optional>)
 *      Return: sela with additional sels, or null on error
 *
 *  Notes:
 *      (1) Adds the following sels:
 *            - all linear (horiz, vert) brick sels that are
 *              necessary for decomposable sels up to size 63
 *            - square brick sels up to size 10
 *            - 4 diagonal sels
 */
SELA *
selaAddBasic(SELA  *sela)
{
char     name[L_BUF_SIZE];
l_int32  i, size;
SEL     *sel;

    PROCNAME("selaAddBasic");

    if (!sela) {
        if ((sela = selaCreate(0)) == NULL)
            return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    }

    /*--------------------------------------------------------------*
     *             Linear horizontal and vertical sels              *
     *--------------------------------------------------------------*/
    for (i = 0; i < num_linear; i++) {
        size = basic_linear[i];
        sel = selCreateBrick(1, size, 0, size / 2, 1);
        snprintf(name, L_BUF_SIZE, "sel_%dh", size);
        selaAddSel(sela, sel, name, 0);
    }
    for (i = 0; i < num_linear; i++) {
        size = basic_linear[i];
        sel = selCreateBrick(size, 1, size / 2, 0, 1);
        snprintf(name, L_BUF_SIZE, "sel_%dv", size);
        selaAddSel(sela, sel, name, 0);
    }

    /*-----------------------------------------------------------*
     *                      2-d Bricks                           *
     *-----------------------------------------------------------*/
    for (i = 2; i <= 5; i++) {
        sel = selCreateBrick(i, i, i / 2, i / 2, 1);
        snprintf(name, L_BUF_SIZE, "sel_%d", i);
        selaAddSel(sela, sel, name, 0);
    }

    /*-----------------------------------------------------------*
     *                        Diagonals                          *
     *-----------------------------------------------------------*/
        /*  0c  1
            1   0  */ 
    sel = selCreateBrick(2, 2, 0, 0, 1);
    selSetElement(sel, 0, 0, 0);
    selSetElement(sel, 1, 1, 0);
    selaAddSel(sela, sel, "sel_2dp", 0);

        /*  1c  0
            0   1   */ 
    sel = selCreateBrick(2, 2, 0, 0, 1);
    selSetElement(sel, 0, 1, 0);
    selSetElement(sel, 1, 0, 0);
    selaAddSel(sela, sel, "sel_2dm", 0);

        /*  Diagonal, slope +, size 5 */
    sel = selCreate(5, 5, "sel_5dp");
    sel->cy = 2;
    sel->cx = 2;
    selSetElement(sel, 0, 4, 1);
    selSetElement(sel, 1, 3, 1);
    selSetElement(sel, 2, 2, 1);
    selSetElement(sel, 3, 1, 1);
    selSetElement(sel, 4, 0, 1);
    selaAddSel(sela, sel, "sel_5dp", 0);

        /*  Diagonal, slope -, size 5 */
    sel = selCreate(5, 5, "sel_5dm");
    sel->cy = 2;
    sel->cx = 2;
    selSetElement(sel, 0, 0, 1);
    selSetElement(sel, 1, 1, 1);
    selSetElement(sel, 2, 2, 1);
    selSetElement(sel, 3, 3, 1);
    selSetElement(sel, 4, 4, 1);
    selaAddSel(sela, sel, "sel_5dm", 0);

    return sela;
}


/*!
 *  selaAddHitMiss()
 *
 *      Input:  sela  (<optional>)
 *      Return: sela with additional sels, or null on error
 */
SELA *
selaAddHitMiss(SELA  *sela)
{
SEL  *sel;

    PROCNAME("selaAddHitMiss");

    if (!sela) {
        if ((sela = selaCreate(0)) == NULL)
            return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    }

#if 0   /*  use just for testing */
    sel = selCreateBrick(3, 3, 1, 1, 2);
    selaAddSel(sela, sel, "sel_bad", 0);
#endif


    /*--------------------------------------------------------------*
     *                   Isolated foreground pixel                  *
     *--------------------------------------------------------------*/
    sel = selCreateBrick(3, 3, 1, 1, 2);
    selSetElement(sel, 1, 1, 1);
    selaAddSel(sela, sel, "sel_3hm", 0);


    /*--------------------------------------------------------------*
     *                Horizontal and vertical edges                 *
     *--------------------------------------------------------------*/
    sel = selCreateBrick(2, 3, 0, 1, 1);
    selSetElement(sel, 1, 0, 2);
    selSetElement(sel, 1, 1, 2);
    selSetElement(sel, 1, 2, 2);
    selaAddSel(sela, sel, "sel_3de", 0);

    sel = selCreateBrick(2, 3, 1, 1, 1);
    selSetElement(sel, 0, 0, 2);
    selSetElement(sel, 0, 1, 2);
    selSetElement(sel, 0, 2, 2);
    selaAddSel(sela, sel, "sel_3ue", 0);

    sel = selCreateBrick(3, 2, 1, 0, 1);
    selSetElement(sel, 0, 1, 2);
    selSetElement(sel, 1, 1, 2);
    selSetElement(sel, 2, 1, 2);
    selaAddSel(sela, sel, "sel_3re", 0);

    sel = selCreateBrick(3, 2, 1, 1, 1);
    selSetElement(sel, 0, 0, 2);
    selSetElement(sel, 1, 0, 2);
    selSetElement(sel, 2, 0, 2);
    selaAddSel(sela, sel, "sel_3le", 0);


    /*--------------------------------------------------------------*
     *                       Slanted edge                           *
     *--------------------------------------------------------------*/
    sel = selCreateBrick(13, 6, 6, 2, 0);
    selSetElement(sel, 0, 3, 2);
    selSetElement(sel, 0, 5, 1);
    selSetElement(sel, 4, 2, 2);
    selSetElement(sel, 4, 4, 1);
    selSetElement(sel, 8, 1, 2);
    selSetElement(sel, 8, 3, 1);
    selSetElement(sel, 12, 0, 2);
    selSetElement(sel, 12, 2, 1);
    selaAddSel(sela, sel, "sel_sl1", 0);

    return sela;
}


/*!
 *  selaAddDwaLinear()
 *
 *      Input:  sela (<optional>)
 *      Return: sela with additional sels, or null on error
 *
 *  Notes:
 *      (1) Adds all linear (horizontal, vertical) sels from
 *          2 to 63 pixels in length, which are the sizes over
 *          which dwa code can be generated.
 */
SELA *
selaAddDwaLinear(SELA  *sela)
{
char     name[L_BUF_SIZE];
l_int32  i;
SEL     *sel;

    PROCNAME("selaAddDwaLinear");

    if (!sela) {
        if ((sela = selaCreate(0)) == NULL)
            return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    }

    for (i = 2; i < 64; i++) {
        sel = selCreateBrick(1, i, 0, i / 2, 1);
        snprintf(name, L_BUF_SIZE, "sel_%dh", i);
        selaAddSel(sela, sel, name, 0);
    }
    for (i = 2; i < 64; i++) {
        sel = selCreateBrick(i, 1, i / 2, 0, 1);
        snprintf(name, L_BUF_SIZE, "sel_%dv", i);
        selaAddSel(sela, sel, name, 0);
    }
    return sela;
}


/*!
 *  selaAddDwaCombs()
 *
 *      Input:  sela (<optional>)
 *      Return: sela with additional sels, or null on error
 *
 *  Notes:
 *      (1) Adds all comb (horizontal, vertical) Sels that are
 *          used in composite linear morphological operations
 *          up to 63 pixels in length, which are the sizes over
 *          which dwa code can be generated.
 */
SELA *
selaAddDwaCombs(SELA  *sela)
{
char     name[L_BUF_SIZE];
l_int32  i, f1, f2, prevsize, size;
SEL     *selh, *selv;

    PROCNAME("selaAddDwaCombs");

    if (!sela) {
        if ((sela = selaCreate(0)) == NULL)
            return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    }

    prevsize = 0;
    for (i = 4; i < 64; i++) {
        selectComposableSizes(i, &f1, &f2);
        size = f1 * f2;
        if (size == prevsize)
            continue;
        selectComposableSels(i, L_HORIZ, NULL, &selh);
        selectComposableSels(i, L_VERT, NULL, &selv);
        snprintf(name, L_BUF_SIZE, "sel_comb_%dh", size);
        selaAddSel(sela, selh, name, 0);
        snprintf(name, L_BUF_SIZE, "sel_comb_%dv", size);
        selaAddSel(sela, selv, name, 0);
        prevsize = size;
    }

    return sela;
}


