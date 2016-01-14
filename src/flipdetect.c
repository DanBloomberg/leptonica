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
 *  flipdetect.c
 *
 *      Page transformation detection:
 *          void      pixPageFlipDetect()
 *          void      pixPageFlipDetectDWA()
 *          
 *
 *      ==============================================================    
 *      Page transformation detection:
 *
 *      Once a page is deskewed (using the functions above) there are 8
 *      possible states that it can be in - only one is correct:
 *      
 *      0: correct     1          2          3
 *      +-----+     +-----+    +-----+    +-----+      
 *      | *** |     | *   |    | *** |    |   * |       
 *      | *   |     | *   |    |   * |    |   * |      
 *      | *   |     | *** |    |   * |    | *** |      
 *      +-----+     +-----+    +-----+    +-----+      
 *
 *          4            5             6            7
 *      +--------+   +--------+   +--------+   +--------+   
 *      | *****  |   |     *  |   | *****  |   | *      |   
 *      |     *  |   |     *  |   | *      |   | *      |   
 *      |     *  |   | *****  |   | *      |   | *****  |   
 *      +--------+   +--------+   +--------+   +--------+   
 *
 *      Each of these can be seen as applying some combination of
 *      a 90 degrees clockwise rotation, a vertical flip and a
 *      horizontal flip
 *
 *      R = Rotation
 *      H = Horizontal flip
 *      V = Vertical flip
 *
 *      RHV
 *      000  -> 0
 *      001  -> 1
 *      010  -> 2
 *      011  -> 3
 *      100  -> 4
 *      101  -> 5
 *      110  -> 6
 *      111  -> 7
 *      
 *      The rotation can be detected by trying finding the deskew
 *      confidence for an image segment, and for that image
 *      segment rotated 90 degrees counter-clockwise.
 *
 *      The flips can be detected, for English text, by noting
 *      that ascending letters (b, d, h, l, etc) out number
 *      decending ones (g, q, p etc).
 *
 *      Likewise, ascenders which stick out to the right (b, h, k)
 *      outnumber those which go to the left (d etc). Sadly, this
 *      signal isn't as strong as the ascenders and decenders and
 *      the resulting confidence values are lower.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

    /* SELs for pixPageFlipDetect() */
static const char *textsel1 = "x  oo "
                              "x oOo "
                              "x  o  "
                              "x     "
                              "xxxxxx";

static const char *textsel2 = " oo  x"
                              " oOo x"
                              "  o  x"
                              "     x"
                              "xxxxxx";

static const char *textsel3 = "xxxxxx"
                              "x     "
                              "x  o  "
                              "x oOo "
                              "x  oo ";
                          
static const char *textsel4 = "xxxxxx"
                              "     x"
                              "  o  x"
                              " oOo x"
                              " oo  x";

static const l_int32  DEFAULT_MIN_COUNT_UP_DOWN = 200;
static const l_int32  DEFAULT_MIN_COUNT_LEFT_RIGHT = 200;

/*!
 *  pixPageFlipDetect()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &vertconf (<optional return> result)
 *              &horizconf (<optional return> result)
 *              mincountv (min number of up + down; use 0 for default)
 *              mincounth (min number of left + right; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See "Measuring document image skew and orientation"
 *          Dan S. Bloomberg, Gary E. Kopec and Lakshmi Dasari
 *          IS&T/SPIE EI'95, Conference 2422: Document Recognition II
 *          pp 302-316, Feb 6-7, 1995, San Jose, CA
 *      (2) Uses rasterop implementation of HMT.
 */
l_int32
pixPageFlipDetect(PIX        *pixs,
                  l_float32  *vertconf,
                  l_float32  *horizconf,
                  l_int32     mincountv,
                  l_int32     mincounth)
{
PIX       *pixt0, *pixt1, *pixt2, *pixt3, *pixt4;
SEL       *sel1, *sel2, *sel3, *sel4;
l_int32    count1, count2, count3, count4;
l_float32  nup, ndown, nleft, nright;

    PROCNAME("pixPageFlipDetect");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not 1 bpp", procName, 1);
    if (mincountv == 0)
        mincountv = DEFAULT_MIN_COUNT_UP_DOWN;
    if (mincounth == 0)
        mincounth = DEFAULT_MIN_COUNT_LEFT_RIGHT;

    pixt0 = pixMorphSequence(pixs, "d20.1", 0);

    sel1 = selCreateFromString(textsel1, 5, 6, NULL);
    sel2 = selCreateFromString(textsel2, 5, 6, NULL);
    sel3 = selCreateFromString(textsel3, 5, 6, NULL);
    sel4 = selCreateFromString(textsel4, 5, 6, NULL);

    pixt1 = pixHMT(NULL, pixt0, sel1);
    pixCountPixels(pixt1, &count1, NULL);
    pixDestroy(&pixt1);

    pixt2 = pixHMT(NULL, pixt0, sel2);
    pixCountPixels(pixt2, &count2, NULL);
    pixDestroy(&pixt2);

    pixt3 = pixHMT(NULL, pixt0, sel3);
    pixCountPixels(pixt3, &count3, NULL);
    pixDestroy(&pixt3);

    pixt4 = pixHMT(NULL, pixt0, sel4);
    pixCountPixels(pixt4, &count4, NULL);
    pixDestroy(&pixt4);

    pixDestroy(&pixt0);
    selDestroy(&sel1);
    selDestroy(&sel2);
    selDestroy(&sel3);
    selDestroy(&sel4);

    nleft = (l_float32)count1;
    nright = (l_float32)count2;
    nup = (l_float32)(count1 + count2);
    ndown = (l_float32)(count3 + count4);

    if (vertconf) {
        *vertconf = 0.0;
        if (nup + ndown > mincountv)
            *vertconf = 2. * ((nup - ndown) / sqrt(nup + ndown));
    }

    if (horizconf) {
        *horizconf = 0.0;
        if (nleft + nright > mincounth)
            *horizconf = 2. * ((nleft - nright) / sqrt(nleft + nright));
    }

    return 0;
}


/*----------------------------------------------------------------*
 *         DWA implementation. Identical results, but faster      *
 *----------------------------------------------------------------*/
/*!
 *  pixPageFlipDetectDWA()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &vertconf (<optional return> result)
 *              &horizconf (<optional return> result)
 *              mincountv (min number of up + down; use 0 for default)
 *              mincounth (min number of left + right; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Same interface as for pixPageFlipDetect().
 *      (2) Uses auto-gen'd code for the Sels defined at the
 *          top of this file, with some renaming of functions.
 *          The auto-gen'd code is in fliphmtgen.c, and can
 *          be generated by a simple executable; see prog/flipselgen.c.
 *      (3) This runs about 2.5 times faster than the pixPageFlipDetect().
 */
l_int32
pixPageFlipDetectDWA(PIX        *pixs,
                     l_float32  *vertconf,
                     l_float32  *horizconf,
                     l_int32     mincountv,
                     l_int32     mincounth)
{
PIX       *pixtr, *pixt0, *pixt1, *pixt2, *pixt3, *pixt4;
l_int32    count1, count2, count3, count4;
l_float32  nup, ndown, nleft, nright;

    PROCNAME("pixPageFlipDetectDWA");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not 1 bpp", procName, 1);
    if (mincountv == 0)
        mincountv = DEFAULT_MIN_COUNT_UP_DOWN;
    if (mincounth == 0)
        mincounth = DEFAULT_MIN_COUNT_LEFT_RIGHT;

    pixt0 = pixAddBorderGeneral(pixs, ADDED_BORDER, ADDED_BORDER,
                                ADDED_BORDER, ADDED_BORDER, 0);
    pixFMorphopGen_1(pixt0, pixt0, MORPH_DILATION, "sel_20h");

    pixt1 = pixFlipFHMTGen(NULL, pixt0, "flipsel1");
    pixtr = pixRemoveBorderGeneral(pixt1, ADDED_BORDER, ADDED_BORDER,
                                   ADDED_BORDER, ADDED_BORDER);
    pixDestroy(&pixt1);
    pixCountPixels(pixtr, &count1, NULL);
    pixDestroy(&pixtr);

    pixt2 = pixFlipFHMTGen(NULL, pixt0, "flipsel2");
    pixtr = pixRemoveBorderGeneral(pixt2, ADDED_BORDER, ADDED_BORDER,
                                   ADDED_BORDER, ADDED_BORDER);
    pixDestroy(&pixt2);
    pixCountPixels(pixtr, &count2, NULL);
    pixDestroy(&pixtr);

    pixt3 = pixFlipFHMTGen(NULL, pixt0, "flipsel3");
    pixtr = pixRemoveBorderGeneral(pixt3, ADDED_BORDER, ADDED_BORDER,
                                   ADDED_BORDER, ADDED_BORDER);
    pixDestroy(&pixt3);
    pixCountPixels(pixtr, &count3, NULL);
    pixDestroy(&pixtr);

    pixt4 = pixFlipFHMTGen(NULL, pixt0, "flipsel4");
    pixtr = pixRemoveBorderGeneral(pixt4, ADDED_BORDER, ADDED_BORDER,
                                   ADDED_BORDER, ADDED_BORDER);
    pixDestroy(&pixt4);
    pixCountPixels(pixtr, &count4, NULL);
    pixDestroy(&pixtr);

    pixDestroy(&pixt0);

    nleft = (l_float32)count1;
    nright = (l_float32)count2;
    nup = (l_float32)(count1 + count2);
    ndown = (l_float32)(count3 + count4);

    if (vertconf) {
        *vertconf = 0.0;
        if (nup + ndown > mincountv)
            *vertconf = 2. * ((nup - ndown) / sqrt(nup + ndown));
    }

    if (horizconf) {
        *horizconf = 0.0;
        if (nleft + nright > mincounth)
            *horizconf = 2. * ((nleft - nright) / sqrt(nleft + nright));
    }

    return 0;
}


