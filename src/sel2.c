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
 *
 *               Linear horizontal
 *               Linear Vertical
 *               Square
 *               Diagonals
 *
 *          SELA    *selaAddHitMiss()
 *
 *               Isolated foreground pixel
 *               Horizontal and vertical edges
 *               Slanted edge
 */

#include <stdio.h>
#include "allheaders.h"


/*!
 *  selaAddBasic()
 *
 *      Input:  sela (<optional>)
 *      Return: sela with additional sels, or null on error
 */
SELA *
selaAddBasic(SELA  *sela)
{
SEL  *sel;

    PROCNAME("selaAddBasic");

    if (!sela) {
        if ((sela = selaCreate(0)) == NULL)
            return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    }

    /*--------------------------------------------------------------*
     *                     Linear horizontal sels                   *
     *--------------------------------------------------------------*/
    sel = selCreateBrick(1, 2, 0, 1, 1);
    selaAddSel(sela, sel, "sel_2h", 0);

    sel = selCreateBrick(1, 3, 0, 1, 1);
    selaAddSel(sela, sel, "sel_3h", 0);

    sel = selCreateBrick(1, 4, 0, 2, 1);
    selaAddSel(sela, sel, "sel_4h", 0);

    sel = selCreateBrick(1, 5, 0, 2, 1);
    selaAddSel(sela, sel, "sel_5h", 0);

    sel = selCreateBrick(1, 6, 0, 3, 1);
    selaAddSel(sela, sel, "sel_6h", 0);

    sel = selCreateBrick(1, 7, 0, 3, 1);
    selaAddSel(sela, sel, "sel_7h", 0);

    sel = selCreateBrick(1, 8, 0, 4, 1);
    selaAddSel(sela, sel, "sel_8h", 0);

    sel = selCreateBrick(1, 9, 0, 4, 1);
    selaAddSel(sela, sel, "sel_9h", 0);

    sel = selCreateBrick(1, 10, 0, 5, 1);
    selaAddSel(sela, sel, "sel_10h", 0);

    sel = selCreateBrick(1, 11, 0, 5, 1);
    selaAddSel(sela, sel, "sel_11h", 0);

    sel = selCreateBrick(1, 15, 0, 7, 1);
    selaAddSel(sela, sel, "sel_15h", 0);

    sel = selCreateBrick(1, 20, 0, 10, 1);
    selaAddSel(sela, sel, "sel_20h", 0);

    sel = selCreateBrick(1, 21, 0, 10, 1);
    selaAddSel(sela, sel, "sel_21h", 0);

    sel = selCreateBrick(1, 30, 0, 15, 1);
    selaAddSel(sela, sel, "sel_30h", 0);

    sel = selCreateBrick(1, 31, 0, 15, 1);
    selaAddSel(sela, sel, "sel_31h", 0);

    sel = selCreateBrick(1, 40, 0, 20, 1);
    selaAddSel(sela, sel, "sel_40h", 0);

    sel = selCreateBrick(1, 41, 0, 20, 1);
    selaAddSel(sela, sel, "sel_41h", 0);

    sel = selCreateBrick(1, 50, 0, 25, 1);
    selaAddSel(sela, sel, "sel_50h", 0);

    sel = selCreateBrick(1, 51, 0, 25, 1);
    selaAddSel(sela, sel, "sel_51h", 0);


    /*--------------------------------------------------------------*
     *                      Linear vertical sels                    *
     *--------------------------------------------------------------*/
    sel = selCreateBrick(2, 1, 1, 0, 1);
    selaAddSel(sela, sel, "sel_2v", 0);

    sel = selCreateBrick(3, 1, 1, 0, 1);
    selaAddSel(sela, sel, "sel_3v", 0);

    sel = selCreateBrick(4, 1, 2, 0, 1);
    selaAddSel(sela, sel, "sel_4v", 0);

    sel = selCreateBrick(5, 1, 2, 0, 1);
    selaAddSel(sela, sel, "sel_5v", 0);

    sel = selCreateBrick(6, 1, 3, 0, 1);
    selaAddSel(sela, sel, "sel_6v", 0);

    sel = selCreateBrick(7, 1, 3, 0, 1);
    selaAddSel(sela, sel, "sel_7v", 0);

    sel = selCreateBrick(8, 1, 4, 0, 1);
    selaAddSel(sela, sel, "sel_8v", 0);

    sel = selCreateBrick(9, 1, 4, 0, 1);
    selaAddSel(sela, sel, "sel_9v", 0);

    sel = selCreateBrick(10, 1, 5, 0, 1);
    selaAddSel(sela, sel, "sel_10v", 0);

    sel = selCreateBrick(11, 1, 5, 0, 1);
    selaAddSel(sela, sel, "sel_11v", 0);

    sel = selCreateBrick(15, 1, 7, 0, 1);
    selaAddSel(sela, sel, "sel_15v", 0);

    sel = selCreateBrick(20, 1, 10, 0, 1);
    selaAddSel(sela, sel, "sel_20v", 0);

    sel = selCreateBrick(21, 1, 10, 0, 1);
    selaAddSel(sela, sel, "sel_21v", 0);

    sel = selCreateBrick(30, 1, 15, 0, 1);
    selaAddSel(sela, sel, "sel_30v", 0);

    sel = selCreateBrick(31, 1, 15, 0, 1);
    selaAddSel(sela, sel, "sel_31v", 0);

    sel = selCreateBrick(40, 1, 20, 0, 1);
    selaAddSel(sela, sel, "sel_40v", 0);

    sel = selCreateBrick(41, 1, 20, 0, 1);
    selaAddSel(sela, sel, "sel_41v", 0);

    sel = selCreateBrick(50, 1, 25, 0, 1);
    selaAddSel(sela, sel, "sel_50v", 0);

    sel = selCreateBrick(51, 1, 25, 0, 1);
    selaAddSel(sela, sel, "sel_51v", 0);


    /*-----------------------------------------------------------*
     *                      2-d Bricks                           *
     *-----------------------------------------------------------*/
    sel = selCreateBrick(1, 1, 0, 0, 1);
    selaAddSel(sela, sel, "sel_1", 0);

    sel = selCreateBrick(2, 2, 1, 1, 1);
    selaAddSel(sela, sel, "sel_2", 0);

    sel = selCreateBrick(3, 3, 1, 1, 1);
    selaAddSel(sela, sel, "sel_3", 0);

    sel = selCreateBrick(4, 4, 2, 2, 1);
    selaAddSel(sela, sel, "sel_4", 0);

    sel = selCreateBrick(5, 5, 2, 2, 1);
    selaAddSel(sela, sel, "sel_5", 0);

    sel = selCreateBrick(6, 6, 3, 3, 1);
    selaAddSel(sela, sel, "sel_6", 0);

    sel = selCreateBrick(7, 7, 3, 3, 1);
    selaAddSel(sela, sel, "sel_7", 0);

    sel = selCreateBrick(8, 8, 4, 4, 1);
    selaAddSel(sela, sel, "sel_8", 0);

    sel = selCreateBrick(9, 9, 4, 4, 1);
    selaAddSel(sela, sel, "sel_9", 0);

    sel = selCreateBrick(10, 10, 5, 5, 1);
    selaAddSel(sela, sel, "sel_10", 0);



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

