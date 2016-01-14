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
 *   boxfunc1.c
 *
 *      Box geometry
 *           l_int32   boxContains()
 *           l_int32   boxIntersects()
 *           BOXA     *boxaContainedInBox()
 *           BOXA     *boxaIntersectsBox()
 *           BOXA     *boxaClipToBox()
 *           BOX      *boxOverlapRegion()
 *           BOX      *boxBoundingRegion()
 *           l_int32   boxOverlapFraction()
 *           l_int32   boxContainsPt()
 *           BOX      *boxaGetNearestToPt()
 *           l_int32   boxIntersectByLine()
 *           l_int32   boxGetCentroid()
 *           BOX      *boxClipToRectangle()
 *           void      boxResizeOneSide()
 *           BOX      *boxAdjustSides()
 *
 *      Boxa combination
 *           l_int32   boxaJoin()
 *
 *      Other boxa functions
 *           l_int32   boxaGetExtent()
 *           l_int32   boxaSizeRange()
 *           BOXA     *boxaSelectBySize()
 *           NUMA     *boxaMakeSizeIndicator()
 *           BOXA     *boxaSelectWithIndicator()
 *           BOXA     *boxaPermutePseudorandom()
 *           BOXA     *boxaPermuteRandom()
 *           l_int32   boxaSwapBoxes()
 *           PTA      *boxaConvertToPta()
 *           BOXA     *ptaConvertToBoxa()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


/*---------------------------------------------------------------------*
 *                             Box geometry                            *
 *---------------------------------------------------------------------*/
/*!
 *  boxContains()
 *
 *      Input:  box1, box2
 *              &result (<return> 1 if box2 is entirely contained within
 *                       box1, and 0 otherwise)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxContains(BOX     *box1,
            BOX     *box2,
            l_int32 *presult)
{
    PROCNAME("boxContains");

    if (!box1 || !box2)
        return ERROR_INT("box1 and box2 not both defined", procName, 1);

    if ((box1->x <= box2->x) &&
        (box1->y <= box2->y) &&
        (box1->x + box1->w >= box2->x + box2->w) &&
        (box1->y + box1->h >= box2->y + box2->h))
        *presult = 1;
    else
        *presult = 0;
    return 0;
}
                


/*!
 *  boxIntersects()
 *
 *      Input:  box1, box2
 *              &result (<return> 1 if any part of box2 is contained
 *                      in box1, and 0 otherwise)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxIntersects(BOX      *box1,
              BOX      *box2,
              l_int32  *presult)
{
l_int32  left1, left2, top1, top2, right1, right2, bot1, bot2;

    PROCNAME("boxIntersects");

    if (!box1 || !box2)
        return ERROR_INT("box1 and box2 not both defined", procName, 1);

    left1 = box1->x;
    left2 = box2->x;
    top1 = box1->y;
    top2 = box2->y;
    right1 = box1->x + box1->w - 1;
    bot1 = box1->y + box1->h - 1;
    right2 = box2->x + box2->w - 1;
    bot2 = box2->y + box2->h - 1;
    if ((bot2 >= top1) && (bot1 >= top2) &&
         (right1 >= left2) && (right2 >= left1))
        *presult = 1;
    else
        *presult = 0;
    return 0;
}
                

/*!
 *  boxaContainedInBox()
 *
 *      Input:  boxas
 *              box (for containment)
 *      Return: boxad (boxa with all boxes in boxas that are
 *                     entirely contained in box), or null on error
 *
 *  Notes:
 *      (1) All boxes in boxa that are entirely outside box are removed.
 */
BOXA *
boxaContainedInBox(BOXA  *boxas,
                   BOX   *box)
{
l_int32  i, n, val;
BOX     *boxt;
BOXA    *boxad;

    PROCNAME("boxaContainedInBox");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!box)
        return (BOXA *)ERROR_PTR("box not defined", procName, NULL);
    if ((n = boxaGetCount(boxas)) == 0)
        return boxaCreate(1);  /* empty */

    boxad = boxaCreate(0);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxas, i, L_CLONE);
        boxContains(box, boxt, &val);
        if (val == 1)
            boxaAddBox(boxad, boxt, L_COPY);
        boxDestroy(&boxt);  /* destroy the clone */
    }

    return boxad;
}


/*!
 *  boxaIntersectsBox()
 *
 *      Input:  boxas
 *              box (for intersecting)
 *      Return  boxad (boxa with all boxes in boxas that intersect box),
 *                     or null on error
 *
 *  Notes:
 *      (1) All boxes in boxa that intersect with box (i.e., are completely
 *          or partially contained in box) are retained.
 */
BOXA *
boxaIntersectsBox(BOXA  *boxas,
                  BOX   *box)
{
l_int32  i, n, val;
BOX     *boxt;
BOXA    *boxad;

    PROCNAME("boxaIntersectsBox");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!box)
        return (BOXA *)ERROR_PTR("box not defined", procName, NULL);
    if ((n = boxaGetCount(boxas)) == 0)
        return boxaCreate(1);  /* empty */

    boxad = boxaCreate(0);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxas, i, L_CLONE);
        boxIntersects(box, boxt, &val);
        if (val == 1)
            boxaAddBox(boxad, boxt, L_COPY);
        boxDestroy(&boxt);  /* destroy the clone */
    }

    return boxad;
}


/*!
 *  boxaClipToBox()
 *
 *      Input:  boxas
 *              box (for clipping)
 *      Return  boxad (boxa with boxes in boxas clipped to box),
 *                     or null on error
 *
 *  Notes:
 *      (1) All boxes in boxa not intersecting with box are removed, and
 *          the remaining boxes are clipped to box.
 */
BOXA *
boxaClipToBox(BOXA  *boxas,
              BOX   *box)
{
l_int32  i, n;
BOX     *boxt, *boxo;
BOXA    *boxad;

    PROCNAME("boxaClipToBox");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!box)
        return (BOXA *)ERROR_PTR("box not defined", procName, NULL);
    if ((n = boxaGetCount(boxas)) == 0)
        return boxaCreate(1);  /* empty */

    boxad = boxaCreate(0);
    for (i = 0; i < n; i++) {
        boxt = boxaGetBox(boxas, i, L_CLONE);
        if ((boxo = boxOverlapRegion(box, boxt)) != NULL)
            boxaAddBox(boxad, boxo, L_INSERT);
        boxDestroy(&boxt);
    }

    return boxad;
}


/*!
 *  boxOverlapRegion()
 *
 *      Input:  box1, box2 (two boxes)
 *      Return: box (of overlap region between input boxes),
 *              or null if no overlap or on error
 */
BOX *
boxOverlapRegion(BOX  *box1,
                 BOX  *box2)
{
l_int32  x, y, w, h, left1, left2, top1, top2, right1, right2, bot1, bot2;

    PROCNAME("boxOverlapRegion");

    if (!box1)
        return (BOX *)ERROR_PTR("box1 not defined", procName, NULL);
    if (!box2)
        return (BOX *)ERROR_PTR("box2 not defined", procName, NULL);

    left1 = box1->x;
    left2 = box2->x;
    top1 = box1->y;
    top2 = box2->y;
    right1 = box1->x + box1->w - 1;
    bot1 = box1->y + box1->h - 1;
    right2 = box2->x + box2->w - 1;
    bot2 = box2->y + box2->h - 1;
    if ((bot2 < top1) || (bot1 < top2) ||
         (right1 < left2) || (right2 < left1))
        return NULL;

    x = (left1 > left2) ? left1 : left2;
    y = (top1 > top2) ? top1 : top2;
    w = L_MIN(right1 - x + 1, right2 - x + 1);
    h = L_MIN(bot1 - y + 1, bot2 - y + 1);
    return boxCreate(x, y, w, h);
}


/*!
 *  boxBoundingRegion()
 *
 *      Input:  box1, box2 (two boxes)
 *      Return: box (of bounding region containing the input boxes),
 *              or null on error
 */
BOX *
boxBoundingRegion(BOX  *box1,
                  BOX  *box2)
{
l_int32  left, top, right1, right2, right, bot1, bot2, bot;

    PROCNAME("boxBoundingRegion");

    if (!box1)
        return (BOX *)ERROR_PTR("box1 not defined", procName, NULL);
    if (!box2)
        return (BOX *)ERROR_PTR("box2 not defined", procName, NULL);

    left = L_MIN(box1->x, box2->x);
    top = L_MIN(box1->y, box2->y);
    right1 = box1->x + box1->w - 1;
    right2 = box2->x + box2->w - 1;
    right = L_MAX(right1, right2);
    bot1 = box1->y + box1->h - 1;
    bot2 = box2->y + box2->h - 1;
    bot = L_MAX(bot1, bot2);
    return boxCreate(left, top, right - left + 1, bot - top + 1);
}


/*!
 *  boxOverlapFraction()
 *
 *      Input:  box1, box2 (two boxes)
 *              &fract (<return> the fraction of box2 overlapped by box1)
 *      Return: 0 if OK, 1 on error.
 *
 *  Notes:
 *      (1) The result depends on the order of the input boxes,
 *          because the overlap is taken as a fraction of box2.
 */
l_int32
boxOverlapFraction(BOX        *box1,
                   BOX        *box2,
                   l_float32  *pfract)
{
l_int32  w2, h2, w, h;
BOX     *boxo;

    PROCNAME("boxOverlapFraction");

    if (!pfract)
        return ERROR_INT("&fract not defined", procName, 1);
    *pfract = 0.0;
    if (!box1)
        return ERROR_INT("box1 not defined", procName, 1);
    if (!box2)
        return ERROR_INT("box2 not defined", procName, 1);

    if ((boxo = boxOverlapRegion(box1, box2)) == NULL)  /* no overlap */
        return 0;

    boxGetGeometry(box2, NULL, NULL, &w2, &h2);
    boxGetGeometry(boxo, NULL, NULL, &w, &h);
    *pfract = (l_float32)(w * h) / (l_float32)(w2 * h2);
    boxDestroy(&boxo);
    return 0;
}


/*!
 *  boxContainsPt()
 *
 *      Input:  box
 *              x, y (a point)
 *              &contains (<return> 1 if box contains point; 0 otherwise)
 *      Return: 0 if OK, 1 on error.
 */
l_int32
boxContainsPt(BOX       *box,
              l_float32  x,
              l_float32  y,
              l_int32   *pcontains)
{
l_int32  bx, by, bw, bh;

    PROCNAME("boxContainsPt");

    if (!pcontains)
        return ERROR_INT("&contains not defined", procName, 1);
    *pcontains = 0;
    if (!box)
        return ERROR_INT("&box not defined", procName, 1);
    boxGetGeometry(box, &bx, &by, &bw, &bh);
    if (x >= bx && x < bx + bw && y >= by && y < by + bh)
        *pcontains = 1;
    return 0;
}


/*!
 *  boxaGetNearestToPt()
 *
 *      Input:  boxa
 *              x, y  (point)
 *      Return  box (box with centroid closest to the given point [x,y]),
 *              or NULL if no boxes in boxa)
 *
 *  Notes:
 *      (1) Uses euclidean distance between centroid and point.
 */
BOX *
boxaGetNearestToPt(BOXA    *boxa,
                   l_int32  x,
                   l_int32  y)
{
l_int32    i, n, cx, cy, minindex;
l_float32  delx, dely, dist, mindist;
BOX       *box;

    PROCNAME("boxaGetNearestToPt");

    if (!boxa)
        return (BOX *)ERROR_PTR("boxa not defined", procName, NULL);
    if ((n = boxaGetCount(boxa)) == 0)
        return (BOX *)ERROR_PTR("n = 0", procName, NULL);
    
    mindist = 1000000000.;
    minindex = 0;
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        boxGetCentroid(box, &cx, &cy);
        delx = (l_float32)(cx - x);
        dely = (l_float32)(cy - y);
        dist = delx * delx + dely * dely;
        if (dist < mindist) {
            minindex = i;
            mindist = dist;
        }
        boxDestroy(&box);
    }

    return boxaGetBox(boxa, minindex, L_COPY);
}


/*!
 *  boxGetCentroid()
 *
 *      Input:  box
 *              &x, &y (<return> location of center of box)
 *      Return  0 if OK, 1 on error
 */
l_int32
boxGetCentroid(BOX      *box,
                l_int32  *px,
                l_int32  *py)
{
l_int32  x, y, w, h;

    PROCNAME("boxGetCentroid");

    if (!px || !py)
        return ERROR_INT("&x, &y not both defined", procName, 1);
    *px = *py = 0;
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    boxGetGeometry(box, &x, &y, &w, &h);
    *px = x + w / 2;
    *py = y + h / 2;

    return 0;
}


/*!
 *  boxIntersectByLine()
 *
 *      Input:  box
 *              x, y (point that line goes through)
 *              slope (of line)
 *              (&x1, &y1) (<return> 1st point of intersection with box)
 *              (&x2, &y2) (<return> 2nd point of intersection with box)
 *              &n (<return> number of points of intersection)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) If the intersection is at only one point (a corner), the
 *          coordinates are returned in (x1, y1).
 *      (2) Represent a vertical line by one with a large but finite slope.
 */
l_int32
boxIntersectByLine(BOX       *box,
                   l_int32    x,
                   l_int32    y,
                   l_float32  slope,
                   l_int32   *px1,
                   l_int32   *py1,
                   l_int32   *px2,
                   l_int32   *py2,
                   l_int32   *pn)
{
l_int32    bx, by, bw, bh, xp, yp, xt, yt, i, n;
l_float32  invslope;
PTA       *pta;

    PROCNAME("boxIntersectByLine");

    if (!px1 || !py1 || !px2 || !py2)
        return ERROR_INT("&x1, &y1, &x2, &y2 not all defined", procName, 1);
    *px1 = *py1 = *px2 = *py2 = 0;
    if (!pn)
        return ERROR_INT("&n not defined", procName, 1);
    *pn = 0;
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    boxGetGeometry(box, &bx, &by, &bw, &bh);

    if (slope == 0.0) {
        if (y >= by && y < by + bh) {
            *py1 = *py2 = y; 
            *px1 = bx;
            *px2 = bx + bw - 1;
        }
        return 0;
    }

    if (slope > 1000000.0) {
        if (x >= bx && x < bx + bw) {
            *px1 = *px2 = x; 
            *py1 = by;
            *py2 = by + bh - 1;
        }
        return 0;
    }

        /* Intersection with top and bottom lines of box */
    pta = ptaCreate(2);
    invslope = 1.0 / slope;
    xp = (l_int32)(x + invslope * (y - by));
    if (xp >= bx && xp < bx + bw)
        ptaAddPt(pta, xp, by); 
    xp = (l_int32)(x + invslope * (y - by - bh + 1));
    if (xp >= bx && xp < bx + bw)
        ptaAddPt(pta, xp, by + bh - 1); 

        /* Intersection with left and right lines of box */
    yp = (l_int32)(y + slope * (x - bx));
    if (yp >= by && yp < by + bh)
        ptaAddPt(pta, bx, yp); 
    yp = (l_int32)(y + slope * (x - bx - bw + 1));
    if (yp >= by && yp < by + bh)
        ptaAddPt(pta, bx + bw - 1, yp); 

        /* There is a maximum of 2 unique points; remove duplicates.  */
    n = ptaGetCount(pta);
    if (n > 0) {
        ptaGetIPt(pta, 0, px1, py1);  /* accept the first one */
	*pn = 1;
    }
    for (i = 1; i < n; i++) {
        ptaGetIPt(pta, i, &xt, &yt);
        if ((*px1 != xt) || (*py1 != yt)) {
            *px2 = xt;
            *py2 = yt;
            *pn = 2;
            break;
        }
    }

    ptaDestroy(&pta);
    return 0;
}


/*!
 *  boxClipToRectangle()
 *
 *      Input:  box
 *              wi, hi (rectangle representing image)
 *      Return: part of box within given rectangle, or NULL on error
 *              or if box is entirely outside the rectangle
 *
 *  Note: the rectangle is assumed to go from (0,0) to (wi - 1, hi - 1)
 */
BOX *
boxClipToRectangle(BOX     *box,
                   l_int32  wi,
                   l_int32  hi)
{
BOX  *boxd;

    PROCNAME("boxClipToRectangle");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);
    if (box->x >= wi || box->y >= hi ||
        box->x + box->w <= 0 || box->y + box->h <= 0)
        return (BOX *)ERROR_PTR("box outside rectangle", procName, NULL);

    boxd = boxCopy(box);
    if (boxd->x < 0) {
        boxd->w += boxd->x;
        boxd->x = 0;
    }
    if (boxd->y < 0) {
        boxd->h += boxd->y;
        boxd->y = 0;
    }
    if (boxd->x + boxd->w > wi)
        boxd->w = wi - boxd->x;
    if (boxd->y + boxd->h > hi)
        boxd->h = hi - boxd->y;
    return boxd;
}


/*!
 *  boxResizeOneSide()
 *
 *      Input:  box (to be resized on one side)
 *              loc (new location of the side that is changing)
 *              sideflag (L_FROM_LEFT, etc., indicating the side that moves)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxResizeOneSide(BOX     *box,
                 l_int32  loc,
                 l_int32  sideflag)
{
l_int32  x, y, w, h;

    PROCNAME("boxResizeOneSide");

    if (!box)
        return ERROR_INT("box not defined", procName, 1);

    boxGetGeometry(box, &x, &y, &w, &h);
    if (sideflag == L_FROM_LEFT)
        boxSetGeometry(box, loc, -1, w + x - loc, -1);
    else if (sideflag == L_FROM_RIGHT)
        boxSetGeometry(box, -1, -1, loc - x + 1, -1);
    else if (sideflag == L_FROM_TOP)
        boxSetGeometry(box, -1, loc, -1, h + y - loc);
    else if (sideflag == L_FROM_BOTTOM)
        boxSetGeometry(box, -1, -1, -1, loc - y + 1);
    return 0;
}


/*!
 *  boxAdjustSides()
 *
 *      Input:  box (to be resized on one side)
 *              delleft, delright, deltop, delbot (changes in location of
 *                                                 each side)
 *      Return: boxd, or null on error if boxd has 0 width or height
 *
 *  Notes:
 *      (1) New box dimensions are cropped at left and top to x >= 0 and y >= 0.
 *      (2) For example, to expand the box by 20 pixels on each side, use
 *             boxAdjustSides(box, -20, 20, -20, 20);
 */
BOX *
boxAdjustSides(BOX     *box,
               l_int32  delleft,
               l_int32  delright,
               l_int32  deltop,
               l_int32  delbot)
{
l_int32  x, y, w, h, xl, xr, yt, yb, wnew, hnew;

    PROCNAME("boxAdjustSides");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);

    boxGetGeometry(box, &x, &y, &w, &h);
    xl = L_MAX(0, x + delleft);
    yt = L_MAX(0, y + deltop);
    xr = x + w + delright;  /* one pixel beyond right edge */
    yb = y + h + delbot;    /* one pixel below bottom edge */
    wnew = xr - xl;
    hnew = yb - yt;
    
    if (wnew < 1 || hnew < 1)
        return (BOX *)ERROR_PTR("boxd has 0 area", procName, NULL);

    return boxCreate(xl, yt, wnew, hnew);
}


/*----------------------------------------------------------------------*
 *                          Boxa Combination                            *
 *----------------------------------------------------------------------*/
/*!
 *  boxaJoin()
 *
 *      Input:  boxad  (dest boxa; add to this one)
 *              boxas  (source boxa; add from this one)
 *              istart  (starting index in nas)
 *              iend  (ending index in nas; use 0 to cat all)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This appends a clone of each indicated box in boxas to boxad
 *      (2) istart < 0 is taken to mean 'read from the start' (istart = 0)
 *      (3) iend <= 0 means 'read to the end'
 */
l_int32
boxaJoin(BOXA    *boxad,
         BOXA    *boxas,
         l_int32  istart,
         l_int32  iend)
{
l_int32  ns, i;
BOX     *box;

    PROCNAME("boxaJoin");

    if (!boxad)
        return ERROR_INT("boxad not defined", procName, 1);
    if (!boxas)
        return ERROR_INT("boxas not defined", procName, 1);
    ns = boxaGetCount(boxas);
    if (istart < 0)
        istart = 0;
    if (istart >= ns)
        return ERROR_INT("istart out of bounds", procName, 1);
    if (iend <= 0)
        iend = ns - 1;
    if (iend >= ns)
        return ERROR_INT("iend out of bounds", procName, 1);
    if (istart > iend)
        return ERROR_INT("istart > iend; nothing to add", procName, 1);

    for (i = istart; i <= iend; i++) {
        box = boxaGetBox(boxas, i, L_CLONE);
        boxaAddBox(boxad, box, L_INSERT);
    }

    return 0;
}


/*---------------------------------------------------------------------*
 *                        Other Boxa functions                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaGetExtent()
 *
 *      Input:  boxa
 *              &w  (<optional return> width)
 *              &h  (<optional return> height)
 *              &box (<optional return>, minimum box containing all boxes
 *                    in boxa)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned w and h are the minimum size image
 *          that would contain all boxes untranslated.
 */
l_int32
boxaGetExtent(BOXA     *boxa,
              l_int32  *pw,
              l_int32  *ph,
              BOX     **pbox)
{
l_int32  i, n, x, y, w, h, xmax, ymax, xmin, ymin;

    PROCNAME("boxaGetExtent");

    if (!pw && !ph && !pbox)
        return ERROR_INT("no ptrs defined", procName, 1);
    if (pbox) *pbox = NULL;
    if (pw) *pw = 0;
    if (ph) *ph = 0;
    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);

    n = boxaGetCount(boxa);
    if (n == 0)
        return ERROR_INT("no boxes in boxa", procName, 1);

    xmax = ymax = 0;
    xmin = ymin = 100000000;
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
        xmin = L_MIN(xmin, x);
        ymin = L_MIN(ymin, y);
        xmax = L_MAX(xmax, x + w);
        ymax = L_MAX(ymax, y + h);
    }
    if (pw) *pw = xmax;
    if (ph) *ph = ymax;
    if (pbox)
      *pbox = boxCreate(xmin, ymin, xmax - xmin, ymax - ymin);

    return 0;
}


/*!
 *  boxaSizeRange()
 *
 *      Input:  boxa
 *              &minw, &minh, &maxw, &maxh (<optional return> range of
 *                                          dimensions of box in the array)
 *      Return: 0 if OK, 1 on error
 */
l_int32  
boxaSizeRange(BOXA     *boxa,
              l_int32  *pminw,
              l_int32  *pminh,
              l_int32  *pmaxw,
              l_int32  *pmaxh)
{
l_int32  minw, minh, maxw, maxh, i, n, w, h;

    PROCNAME("boxaSizeRange");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    if (!pminw && !pmaxw && !pminh && !pmaxh)
        return ERROR_INT("no data can be returned", procName, 1);
    
    minw = minh = 100000000;
    maxw = maxh = 0;
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, NULL, NULL, &w, &h);
        if (w < minw)
            minw = w;
        if (h < minh)
            minh = h;
        if (w > maxw)
            maxw = w;
        if (h > maxh)
            maxh = h;
    }

    if (pminw) *pminw = minw;
    if (pminh) *pminh = minh;
    if (pmaxw) *pmaxw = maxw;
    if (pmaxh) *pmaxh = maxh;

    return 0;
}


/*!
 *  boxaSelectBySize()
 *
 *      Input:  boxas
 *              width, height (threshold dimensions)
 *              type (L_SELECT_WIDTH, L_SELECT_HEIGHT,
 *                    L_SELECT_IF_EITHER, L_SELECT_IF_BOTH)
 *              relation (L_SELECT_IF_LT, L_SELECT_IF_GT,
 *                        L_SELECT_IF_LTE, L_SELECT_IF_GTE)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: boxad (filtered set), or null on error
 *
 *  Notes:
 *      (1) The args specify constraints on the size of the
 *          components that are kept.
 *      (2) Uses box clones in the new boxa.
 *      (3) If the selection type is L_SELECT_WIDTH, the input
 *          height is ignored, and v.v.
 *      (4) To keep small components, use relation = L_SELECT_IF_LT or
 *          L_SELECT_IF_LTE.
 *          To keep large components, use relation = L_SELECT_IF_GT or
 *          L_SELECT_IF_GTE.
 */
BOXA *
boxaSelectBySize(BOXA     *boxas,
                 l_int32   width,
                 l_int32   height,
                 l_int32   type,
                 l_int32   relation,
                 l_int32  *pchanged)
{
BOXA  *boxad;
NUMA  *na;

    PROCNAME("boxaSelectBySize");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (type != L_SELECT_WIDTH && type != L_SELECT_HEIGHT &&
        type != L_SELECT_IF_EITHER && type != L_SELECT_IF_BOTH)
        return (BOXA *)ERROR_PTR("invalid type", procName, NULL);
    if (relation != L_SELECT_IF_LT && relation != L_SELECT_IF_GT && 
        relation != L_SELECT_IF_LTE && relation != L_SELECT_IF_GTE)
        return (BOXA *)ERROR_PTR("invalid relation", procName, NULL);
    if (pchanged) *pchanged = FALSE;
    
        /* Compute the indicator array for saving components */
    na = boxaMakeSizeIndicator(boxas, width, height, type, relation);

        /* Filter to get output */
    boxad = boxaSelectWithIndicator(boxas, na, pchanged);

    numaDestroy(&na);
    return boxad;
}


/*!
 *  boxaMakeSizeIndicator()
 *
 *      Input:  boxa
 *              width, height (threshold dimensions)
 *              type (L_SELECT_WIDTH, L_SELECT_HEIGHT,
 *                    L_SELECT_IF_EITHER, L_SELECT_IF_BOTH)
 *              relation (L_SELECT_IF_LT, L_SELECT_IF_GT,
 *                        L_SELECT_IF_LTE, L_SELECT_IF_GTE)
 *      Return: na (indicator array), or null on error
 *
 *  Notes:
 *      (1) The args specify constraints on the size of the
 *          components that are kept.
 *      (2) If the selection type is L_SELECT_WIDTH, the input
 *          height is ignored, and v.v.
 *      (3) To keep small components, use relation = L_SELECT_IF_LT or
 *          L_SELECT_IF_LTE.
 *          To keep large components, use relation = L_SELECT_IF_GT or
 *          L_SELECT_IF_GTE.
 */
NUMA *
boxaMakeSizeIndicator(BOXA     *boxa,
                      l_int32   width,
                      l_int32   height,
                      l_int32   type,
                      l_int32   relation)
{
l_int32  i, n, w, h, ival;
NUMA    *na;

    PROCNAME("boxaMakeSizeIndicator");

    if (!boxa)
        return (NUMA *)ERROR_PTR("boxa not defined", procName, NULL);
    if (type != L_SELECT_WIDTH && type != L_SELECT_HEIGHT &&
        type != L_SELECT_IF_EITHER && type != L_SELECT_IF_BOTH)
        return (NUMA *)ERROR_PTR("invalid type", procName, NULL);
    if (relation != L_SELECT_IF_LT && relation != L_SELECT_IF_GT && 
        relation != L_SELECT_IF_LTE && relation != L_SELECT_IF_GTE)
        return (NUMA *)ERROR_PTR("invalid relation", procName, NULL);

    n = boxaGetCount(boxa);
    na = numaCreate(n);
    for (i = 0; i < n; i++) {
        ival = 0;
        boxaGetBoxGeometry(boxa, i, NULL, NULL, &w, &h);
        switch (type)
        {
        case L_SELECT_WIDTH:
            if ((relation == L_SELECT_IF_LT && w < width) ||
                (relation == L_SELECT_IF_GT && w > width) ||
                (relation == L_SELECT_IF_LTE && w <= width) ||
                (relation == L_SELECT_IF_GTE && w >= width))
                ival = 1;
            break;
        case L_SELECT_HEIGHT:
            if ((relation == L_SELECT_IF_LT && h < height) ||
                (relation == L_SELECT_IF_GT && h > height) ||
                (relation == L_SELECT_IF_LTE && h <= height) ||
                (relation == L_SELECT_IF_GTE && h >= height))
                ival = 1;
            break;
        case L_SELECT_IF_EITHER:
            if (((relation == L_SELECT_IF_LT) && (w < width || h < height)) ||
                ((relation == L_SELECT_IF_GT) && (w > width || h > height)) ||
               ((relation == L_SELECT_IF_LTE) && (w <= width || h <= height)) ||
                ((relation == L_SELECT_IF_GTE) && (w >= width || h >= height)))
                    ival = 1;
            break;
        case L_SELECT_IF_BOTH:
            if (((relation == L_SELECT_IF_LT) && (w < width && h < height)) ||
                ((relation == L_SELECT_IF_GT) && (w > width && h > height)) ||
               ((relation == L_SELECT_IF_LTE) && (w <= width && h <= height)) ||
                ((relation == L_SELECT_IF_GTE) && (w >= width && h >= height)))
                    ival = 1;
            break;
        default:
            L_WARNING("can't get here!", procName);
        }
        numaAddNumber(na, ival);
    }

    return na;
}


/*!
 *  boxaSelectWithIndicator()
 *
 *      Input:  boxas
 *              na (indicator numa)
 *              &changed (<optional return> 1 if changed; 0 if clone returned)
 *      Return: boxad, or null on error
 *
 *  Notes:
 *      (1) Returns a boxa clone if no components are removed.
 *      (2) Uses box clones in the new boxa.
 *      (3) The indicator numa has values 0 (ignore) and 1 (accept).
 */
BOXA *
boxaSelectWithIndicator(BOXA     *boxas,
                        NUMA     *na,
                        l_int32  *pchanged)
{
l_int32  i, n, ival, nsave;
BOX     *box;
BOXA    *boxad;

    PROCNAME("boxaSelectWithIndicator");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!na)
        return (BOXA *)ERROR_PTR("na not defined", procName, NULL);

    nsave = 0;
    n = numaGetCount(na);
    for (i = 0; i < n; i++) {
        numaGetIValue(na, i, &ival);
        if (ival == 1) nsave++;
    }

    if (nsave == n) {
        if (pchanged) *pchanged = FALSE;
        return boxaCopy(boxas, L_CLONE);
    }
    if (pchanged) *pchanged = TRUE;
    boxad = boxaCreate(nsave);
    for (i = 0; i < n; i++) {
        numaGetIValue(na, i, &ival);
        if (ival == 0) continue;
        box = boxaGetBox(boxas, i, L_CLONE);
        boxaAddBox(boxad, box, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxaPermutePseudorandom()
 *
 *      Input:  boxas (input boxa)
 *      Return: boxad (with boxes permuted), or null on error
 *
 *  Notes:
 *      (1) This does a pseudorandom in-place permutation of the boxes.
 *      (2) The result is guaranteed not to have any boxes in their
 *          original position, but it is not very random.  If you
 *          need randomness, use boxaPermuteRandom().
 */
BOXA *
boxaPermutePseudorandom(BOXA  *boxas)
{
l_int32  n;
NUMA    *na;
BOXA    *boxad;

    PROCNAME("boxaPermutePseudorandom");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxa not defined", procName, NULL);

    n = boxaGetCount(boxas);
    na = numaPseudorandomSequence(n, 0);
    boxad = boxaSortByIndex(boxas, na);
    numaDestroy(&na);
    return boxad;
}


/*!
 *  boxaPermuteRandom()
 *
 *      Input:  boxad (<optional> can be null or equal to boxas)
 *              boxas (input boxa)
 *      Return: boxad (with boxes permuted), or null on error
 *
 *  Notes:
 *      (1) If boxad is null, make a copy of boxas and permute the copy.
 *          Otherwise, boxad must be equal to boxas, and the operation
 *          is done in-place.
 *      (2) This does a random in-place permutation of the boxes,
 *          by swapping each box in turn with a random box.  The
 *          result is almost guaranteed not to have any boxes in their
 *          original position.
 *      (3) MSVC rand() has MAX_RAND = 2^15 - 1, so it will not do
 *          a proper permutation is the number of boxes exceeds this.
 */
BOXA *
boxaPermuteRandom(BOXA  *boxad,
                  BOXA  *boxas)
{
l_int32  i, n, index;

    PROCNAME("boxaPermuteRandom");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxa not defined", procName, NULL);
    if (boxad && (boxad != boxas))
        return (BOXA *)ERROR_PTR("boxad defined but in-place", procName, NULL);

    if (!boxad)
        boxad = boxaCopy(boxas, L_COPY);
    n = boxaGetCount(boxad);
    index = (l_uint32)rand() % n;
    index = L_MAX(1, index);
    boxaSwapBoxes(boxad, 0, index);
    for (i = 1; i < n; i++) {
        index = (l_uint32)rand() % n;
        if (index == i) index--;
        boxaSwapBoxes(boxad, i, index);
    }

    return boxad;
}


/*!
 *  boxaSwapBoxes()
 *
 *      Input:  boxa
 *              i, j (two indices of boxes, that are to be swapped)
 *      Return: 0 if OK, 1 on error
 */
l_int32
boxaSwapBoxes(BOXA    *boxa,
              l_int32  i,
              l_int32  j)
{
l_int32  n;
BOX     *box;

    PROCNAME("boxaSwapBoxes");

    if (!boxa)
        return ERROR_INT("boxa not defined", procName, 1);
    n = boxaGetCount(boxa);
    if (i < 0 || i >= n)
        return ERROR_INT("i invalid", procName, 1);
    if (j < 0 || j >= n)
        return ERROR_INT("j invalid", procName, 1);
    if (i == j)
        return ERROR_INT("i == j", procName, 1);

    box = boxa->box[i];
    boxa->box[i] = boxa->box[j];
    boxa->box[j] = box;
    return 0;
}


/*!
 *  boxaConvertToPta()
 *
 *      Input:  boxa 
 *              ncorners (2 or 4 for the representation of each box)
 *      Return: pta (with @ncorners points for each box in the boxa),
 *                   or null on error
 *
 *  Notes:
 *      (1) If ncorners == 2, we select the UL and LR corners.
 *          Otherwise we save all 4 corners in this order: UL, UR, LL, LR.
 */
PTA *
boxaConvertToPta(BOXA    *boxa,
                 l_int32  ncorners)
{
l_int32  i, n, x, y, w, h;
PTA     *pta;

    PROCNAME("boxaConvertToPta");

    if (!boxa)
        return (PTA *)ERROR_PTR("boxa not defined", procName, NULL);
    if (ncorners != 2 && ncorners != 4)
        return (PTA *)ERROR_PTR("ncorners not 2 or 4", procName, NULL);

    n = boxaGetCount(boxa);
    if ((pta = ptaCreate(n)) == NULL)
        return (PTA *)ERROR_PTR("pta not made", procName, NULL);
    for (i = 0; i < n; i++) {
        boxaGetBoxGeometry(boxa, i, &x, &y, &w, &h);
        ptaAddPt(pta, x, y);
        if (ncorners == 2) 
            ptaAddPt(pta, x + w - 1, y + h - 1);
        else {
            ptaAddPt(pta, x + w - 1, y);
            ptaAddPt(pta, x, y + h - 1);
            ptaAddPt(pta, x + w - 1, y + h - 1);
        }
    }

    return pta;
}


/*!
 *  ptaConvertToBoxa()
 *
 *      Input:  pta
 *              ncorners (2 or 4 for the representation of each box)
 *      Return: boxa (with one box for each 2 or 4 points in the pta),
 *                    or null on error
 *
 *  Notes:
 *      (1) For 2 corners, the order of the 2 points is UL, LR.
 *          For 4 corners, the order of points is UL, UR, LL, LR.
 *      (2) Each derived box is the minimum szie containing all corners.
 */
BOXA *
ptaConvertToBoxa(PTA     *pta,
                 l_int32  ncorners)
{
l_int32  i, n, nbox, x1, y1, x2, y2, x3, y3, x4, y4, x, y, xmax, ymax;
BOX     *box;
BOXA    *boxa;

    PROCNAME("ptaConvertToBoxa");

    if (!pta)
        return (BOXA *)ERROR_PTR("pta not defined", procName, NULL);
    if (ncorners != 2 && ncorners != 4)
        return (BOXA *)ERROR_PTR("ncorners not 2 or 4", procName, NULL);
    n = ptaGetCount(pta);
    if (n % ncorners != 0)
        return (BOXA *)ERROR_PTR("size % ncorners != 0", procName, NULL);
    nbox = n / ncorners;
    if ((boxa = boxaCreate(nbox)) == NULL)
        return (BOXA *)ERROR_PTR("boxa not made", procName, NULL);
    for (i = 0; i < n; i += ncorners) {
        ptaGetIPt(pta, i, &x1, &y1);
        ptaGetIPt(pta, i + 1, &x2, &y2);
        if (ncorners == 2) {
            box = boxCreate(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
            boxaAddBox(boxa, box, L_INSERT);
            continue;
        }
        ptaGetIPt(pta, i + 2, &x3, &y3);
        ptaGetIPt(pta, i + 3, &x4, &y4);
        x = L_MIN(x1, x3);
        y = L_MIN(y1, y2);
        xmax = L_MAX(x2, x4);
        ymax = L_MAX(y3, y4);
        box = boxCreate(x, y, xmax - x + 1, ymax - y + 1);
        boxaAddBox(boxa, box, L_INSERT);
    }

    return boxa;
}


