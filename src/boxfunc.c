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
 *   boxfunc.c
 *
 *   Functions whose primary task is to operate on box, boxa
 *   and boxaa, typically creating new structs or extracting
 *   information not contained in the struct fields.
 *
 *      Box geometry
 *           l_int32   boxContains()
 *           l_int32   boxIntersects()
 *           BOXA     *boxaContainedInBox()
 *           BOXA     *boxaIntersectsBox()
 *           BOX      *boxOverlapRegion()
 *           BOX      *boxBoundingRegion()
 *           l_int32   boxOverlapFraction()
 *           l_int32   boxContainsPt()
 *           BOX      *boxaGetNearestToPt()
 *           l_int32   boxIntersectByLine()
 *           l_int32   boxGetCentroid()
 *           BOX      *boxClipToRectangle()
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
 *
 *      Boxa/Box transform (shift, scale) and orthogonal rotation
 *           BOXA     *boxaTransform()
 *           BOX      *boxTransform()
 *           BOXA     *boxaTransformOrdered()
 *           BOX      *boxTransformOrdered()
 *           BOXA     *boxaRotateOrth()
 *           BOX      *boxRotateOrth()
 *
 *      Boxa sort
 *           BOXA     *boxaSort()
 *           BOXA     *boxaSortByIndex()
 *           BOXAA    *boxaSort2d()
 *           BOXAA    *boxaSort2dByIndex()
 *
 *      Other boxaa functions
 *           l_int32   boxaaGetExtent()
 *           BOXA     *boxaaFlattenToBoxa()
 *           l_int32   boxaaAlignBox()
 *
 *      Boxa/Boxaa painting into pix
 *           PIX      *pixMaskConnComp()
 *           PIX      *pixMaskBoxa()
 *           PIX      *pixPaintBoxa()
 *           PIX      *pixPaintBoxaRandom()
 *           PIX      *pixDrawBoxa()
 *           PIX      *pixDrawBoxaRandom()
 *           PIX      *boxaaDisplay() 
 *
 *  See summary in pixPaintBoxa() of various ways to paint and draw
 *  boxes on images.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
 *      Input:  boxas, box
 *      Return: boxad (boxa with all boxes in boxas that are
 *                     entirely contained in box; or NULL if no boxes
 *                     in boxas are contained within box, or on error)
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
        return (BOXA *)ERROR_PTR("no boxes in boxas", procName, NULL);

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
 *      Input:  boxas, box
 *      Return  boxad (boxa with all boxes in boxas that intersect box;
 *                     or NULL if no boxes intersect the box, or on error)
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
        return (BOXA *)ERROR_PTR("no boxes in boxas", procName, NULL);

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
    index = (rand() >> 16) % n;
    index = L_MAX(1, index);
    boxaSwapBoxes(boxad, 0, index);
    for (i = 1; i < n; i++) {
        index = (rand() >> 16) % n;
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


/*---------------------------------------------------------------------*
 *      Boxa/Box transform (shift, scale) and orthogonal rotation      *
 *---------------------------------------------------------------------*/
/*!
 *  boxaTransform()
 * 
 *      Input:  boxa
 *              shiftx, shifty
 *              scalex, scaley
 *      Return: boxad, or null on error
 *
 *  Notes:
 *      (1) This is a very simple function that first shifts, then scales.
 */
BOXA *
boxaTransform(BOXA      *boxas,
              l_int32    shiftx,
              l_int32    shifty,
              l_float32  scalex,
              l_float32  scaley)
{
l_int32  i, n;
BOX     *boxs, *boxd;
BOXA    *boxad;

    PROCNAME("boxaTransform");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    n = boxaGetCount(boxas);
    if ((boxad = boxaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("boxad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if ((boxs = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("boxs not found", procName, NULL);
        boxd = boxTransform(boxs, shiftx, shifty, scalex, scaley);
        boxDestroy(&boxs);
        boxaAddBox(boxad, boxd, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxTransform()
 * 
 *      Input:  box
 *              shiftx, shifty
 *              scalex, scaley
 *      Return: boxd, or null on error
 *
 *  Notes:
 *      (1) This is a very simple function that first shifts, then scales.
 */
BOX *
boxTransform(BOX       *box,
             l_int32    shiftx,
             l_int32    shifty,
             l_float32  scalex,
             l_float32  scaley)
{
    PROCNAME("boxTransform");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);
    return boxCreate((l_int32)(scalex * (box->x + shiftx) + 0.5),
                     (l_int32)(scaley * (box->y + shifty) + 0.5),
                     (l_int32)(L_MAX(1.0, scalex * box->w + 0.5)),
                     (l_int32)(L_MAX(1.0, scaley * box->h + 0.5)));
}


/*!
 *  boxaTransformOrdered()
 * 
 *      Input:  boxa
 *              shiftx, shifty 
 *              scalex, scaley
 *              xcen, ycen (center of rotation)
 *              angle (in radians; clockwise is positive)
 *              order (one of 6 combinations: L_TR_SC_RO, ...)
 *      Return: boxd, or null on error
 *
 *  Notes:
 *      (1) This allows a sequence of linear transforms on each box.
 *          the transforms are from the affine set, composed of
 *          shift, scaling and rotation, and the order of the
 *          transforms is specified.
 *      (2) See boxTransformOrdered() for usage and implementation details.
 */
BOXA *
boxaTransformOrdered(BOXA      *boxas,
                     l_int32    shiftx,
                     l_int32    shifty,
                     l_float32  scalex,
                     l_float32  scaley,
                     l_int32    xcen,
                     l_int32    ycen,
                     l_float32  angle,
                     l_int32    order)
{
l_int32  i, n;
BOX     *boxs, *boxd;
BOXA    *boxad;

    PROCNAME("boxaTransformOrdered");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    n = boxaGetCount(boxas);
    if ((boxad = boxaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("boxad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if ((boxs = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("boxs not found", procName, NULL);
        boxd = boxTransformOrdered(boxs, shiftx, shifty, scalex, scaley,
                                   xcen, ycen, angle, order);
        boxDestroy(&boxs);
        boxaAddBox(boxad, boxd, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxTransformOrdered()
 * 
 *      Input:  boxs
 *              shiftx, shifty 
 *              scalex, scaley
 *              xcen, ycen (center of rotation)
 *              angle (in radians; clockwise is positive)
 *              order (one of 6 combinations: L_TR_SC_RO, ...)
 *      Return: boxd, or null on error
 *
 *  Notes:
 *      (1) This allows a sequence of linear transforms, composed of
 *          shift, scaling and rotation, where the order of the
 *          transforms is specified.
 *      (2) The rotation is taken about a point specified by (xcen, ycen).
 *          Let the components of the vector from the center of rotation
 *          to the box center be (xdif, ydif):
 *            xdif = (bx + 0.5 * bw) - xcen
 *            ydif = (by + 0.5 * bh) - ycen
 *          Then the box center after rotation has new components:
 *            bxcen = xcen + xdif * cosa - ydif * sina
 *            bycen = ycen + ydif * cosa + xdif * sina
 *          where cosa and sina are the cos and sin of the angle,
 *          and the enclosing box for the rotated box has size:
 *            rw = |bw * cosa| + |bh * sina|
 *            rh = |bh * cosa| + |bw * sina|
 *          where bw and bh are the unrotated width and height.
 *          Then the box UL corner (rx, ry) is
 *            rx = bxcen - 0.5 * rw
 *            ry = bycen - 0.5 * rh
 *      (3) The center of rotation specified by args @xcen and @ycen
 *          is the point before any translation or scaling.  If the
 *          rotation is not the first operation, the actual center
 *          of rotation must be computed by doing the same translation
 *          and/or scaling that is done for the UL corner of the box
 *          before the rotation operation.  See the code below for
 *          details.  Why do we do this?  It's easier to specify the
 *          center location before transforming.  For example, if you
 *          want TR_SC_RO where the rotation is about the center of
 *          an "image region" containing the box, after translation
 *          and scaling, the center used for rotation is still in
 *          the center of the (translated and scaled) image region.
 */
BOX *
boxTransformOrdered(BOX       *boxs,
                    l_int32    shiftx,
                    l_int32    shifty,
                    l_float32  scalex,
                    l_float32  scaley,
                    l_int32    xcen,
                    l_int32    ycen,
                    l_float32  angle,
                    l_int32    order)
{
l_int32    bx, by, bw, bh, tx, ty, tw, th;
l_int32    xcent, ycent;  /* transformed center of rotation */
l_float32  sina, cosa, xdif, ydif, rx, ry, rw, rh;
BOX       *boxd;

    PROCNAME("boxTransformOrdered");

    if (!boxs)
        return (BOX *)ERROR_PTR("boxs not defined", procName, NULL);
    if (order != L_TR_SC_RO && order != L_SC_RO_TR && order != L_RO_TR_SC &&
        order != L_TR_RO_SC && order != L_RO_SC_TR && order != L_SC_TR_RO)
        return (BOX *)ERROR_PTR("order invalid", procName, NULL);

    boxGetGeometry(boxs, &bx, &by, &bw, &bh);
    if (angle != 0.0) {
        sina = sin(angle);
        cosa = cos(angle);
    }

    if (order == L_TR_SC_RO) {
        tx = (l_int32)(scalex * (bx + shiftx) + 0.5);
        ty = (l_int32)(scaley * (by + shifty) + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * bw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * bh + 0.5));
        xcent = (l_int32)(scalex * (xcen + shiftx) + 0.5);
        ycent = (l_int32)(scaley * (ycen + shifty) + 0.5);
        if (angle == 0.0)
            boxd = boxCreate(tx, ty, tw, th);
        else {
            xdif = tx + 0.5 * tw - xcent;
            ydif = ty + 0.5 * th - ycent;
            rw = L_ABS(tw * cosa) + L_ABS(th * sina);
            rh = L_ABS(th * cosa) + L_ABS(tw * sina);
            rx = xcent + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycent + ydif * cosa - xdif * sina - 0.5 * rh;
            boxd = boxCreate((l_int32)rx, (l_int32)ry, (l_int32)rw,
                             (l_int32)rh);
        }
    }
    else if (order == L_SC_TR_RO) {
        tx = (l_int32)(scalex * bx + shiftx + 0.5);
        ty = (l_int32)(scaley * by + shifty + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * bw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * bh + 0.5));
        xcent = (l_int32)(scalex * xcen + shiftx + 0.5);
        ycent = (l_int32)(scaley * ycen + shifty + 0.5);
        if (angle == 0.0)
            boxd = boxCreate(tx, ty, tw, th);
        else {
            xdif = tx + 0.5 * tw - xcent;
            ydif = ty + 0.5 * th - ycent;
            rw = L_ABS(tw * cosa) + L_ABS(th * sina);
            rh = L_ABS(th * cosa) + L_ABS(tw * sina);
            rx = xcent + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycent + ydif * cosa - xdif * sina - 0.5 * rh;
            boxd = boxCreate((l_int32)rx, (l_int32)ry, (l_int32)rw,
                             (l_int32)rh);
        }
    }
    else if (order == L_RO_TR_SC) {
        if (angle == 0.0) {
            rx = bx;
            ry = by;
            rw = bw;
            rh = bh;
        }
        else {
            xdif = bx + 0.5 * bw - xcen;
            ydif = by + 0.5 * bh - ycen;
            rw = L_ABS(bw * cosa) + L_ABS(bh * sina);
            rh = L_ABS(bh * cosa) + L_ABS(bw * sina);
            rx = xcen + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycen + ydif * cosa - xdif * sina - 0.5 * rh;
        }
        tx = (l_int32)(scalex * (rx + shiftx) + 0.5);
        ty = (l_int32)(scaley * (ry + shifty) + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * rw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * rh + 0.5));
        boxd = boxCreate(tx, ty, tw, th);
    }
    else if (order == L_RO_SC_TR) {
        if (angle == 0.0) {
            rx = bx;
            ry = by;
            rw = bw;
            rh = bh;
        }
        else {
            xdif = bx + 0.5 * bw - xcen;
            ydif = by + 0.5 * bh - ycen;
            rw = L_ABS(bw * cosa) + L_ABS(bh * sina);
            rh = L_ABS(bh * cosa) + L_ABS(bw * sina);
            rx = xcen + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycen + ydif * cosa - xdif * sina - 0.5 * rh;
        }
        tx = (l_int32)(scalex * rx + shiftx + 0.5);
        ty = (l_int32)(scaley * ry + shifty + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * rw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * rh + 0.5));
        boxd = boxCreate(tx, ty, tw, th);
    }
    else if (order == L_TR_RO_SC) {
        tx = bx + shiftx;
        ty = by + shifty;
        xcent = xcen + shiftx;
        ycent = ycen + shifty;
        if (angle == 0.0) {
            rx = tx;
            ry = ty;
            rw = bw;
            rh = bh;
        }
        else {
            xdif = tx + 0.5 * bw - xcent;
            ydif = ty + 0.5 * bh - ycent;
            rw = L_ABS(bw * cosa) + L_ABS(bh * sina);
            rh = L_ABS(bh * cosa) + L_ABS(bw * sina);
            rx = xcent + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycent + ydif * cosa - xdif * sina - 0.5 * rh;
        }
        tx = (l_int32)(scalex * rx + 0.5);
        ty = (l_int32)(scaley * ry + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * rw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * rh + 0.5));
        boxd = boxCreate(tx, ty, tw, th);
    }
    else {  /* order == L_SC_RO_TR) */
        tx = (l_int32)(scalex * bx + 0.5);
        ty = (l_int32)(scaley * by + 0.5);
        tw = (l_int32)(L_MAX(1.0, scalex * bw + 0.5));
        th = (l_int32)(L_MAX(1.0, scaley * bh + 0.5));
        xcent = (l_int32)(scalex * xcen + 0.5);
        ycent = (l_int32)(scaley * ycen + 0.5);
        if (angle == 0.0) {
            rx = tx;
            ry = ty;
            rw = tw;
            rh = th;
        }
        else {
            xdif = tx + 0.5 * tw - xcent;
            ydif = ty + 0.5 * th - ycent;
            rw = L_ABS(tw * cosa) + L_ABS(th * sina);
            rh = L_ABS(th * cosa) + L_ABS(tw * sina);
            rx = xcent + xdif * cosa + ydif * sina - 0.5 * rw;
            ry = ycent + ydif * cosa - xdif * sina - 0.5 * rh;
        }
        tx = (l_int32)(rx + shiftx + 0.5);
        ty = (l_int32)(ry + shifty + 0.5);
        tw = (l_int32)(rw + 0.5);
        th = (l_int32)(rh + 0.5);
        boxd = boxCreate(tx, ty, tw, th);
    }

    return boxd;
}


/*!
 *  boxaRotateOrth()
 * 
 *      Input:  boxa
 *              w, h (of image in which the boxa is embedded)
 *              rotation (0 = noop, 1 = 90 deg, 2 = 180 deg, 3 = 270 deg;
 *                        all rotations are clockwise)
 *      Return: boxad, or null on error
 *
 *  Notes:
 *      (1) See boxRotateOrth() for details.
 */
BOXA *
boxaRotateOrth(BOXA    *boxas,
               l_int32  w,
               l_int32  h,
               l_int32  rotation)
{
l_int32  i, n;
BOX     *boxs, *boxd;
BOXA    *boxad;

    PROCNAME("boxaRotateOrth");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (rotation == 0)
        return boxaCopy(boxas, L_COPY);
    if (rotation < 1 || rotation > 3)
        return (BOXA *)ERROR_PTR("rotation not in {0,1,2,3}", procName, NULL);

    n = boxaGetCount(boxas);
    if ((boxad = boxaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("boxad not made", procName, NULL);
    for (i = 0; i < n; i++) {
        if ((boxs = boxaGetBox(boxas, i, L_CLONE)) == NULL)
            return (BOXA *)ERROR_PTR("boxs not found", procName, NULL);
        boxd = boxRotateOrth(boxs, w, h, rotation);
        boxDestroy(&boxs);
        boxaAddBox(boxad, boxd, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxRotateOrth()
 * 
 *      Input:  box
 *              w, h (of image in which the box is embedded)
 *              rotation (0 = noop, 1 = 90 deg, 2 = 180 deg, 3 = 270 deg;
 *                        all rotations are clockwise)
 *      Return: boxd, or null on error
 *
 *  Notes:
 *      (1) Rotate the image with the embedded box by the specified amount.
 *      (2) After rotation, the rotated box is always measured with
 *          respect to the UL corner of the image.
 */
BOX *
boxRotateOrth(BOX     *box,
              l_int32  w,
              l_int32  h,
              l_int32  rotation)
{
l_int32  bx, by, bw, bh, xdist, ydist;

    PROCNAME("boxRotateOrth");

    if (!box)
        return (BOX *)ERROR_PTR("box not defined", procName, NULL);
    if (rotation == 0)
        return boxCopy(box);
    if (rotation < 1 || rotation > 3)
        return (BOX *)ERROR_PTR("rotation not in {0,1,2,3}", procName, NULL);
    boxGetGeometry(box, &bx, &by, &bw, &bh);
    ydist = h - by - bh;  /* below box */
    xdist = w - bx - bw;  /* to right of box */
    if (rotation == 1)   /* 90 deg cw */
        return boxCreate(ydist, bx, bh, bw);
    else if (rotation == 2)  /* 180 deg cw */
        return boxCreate(xdist, ydist, bw, bh);
    else  /*  rotation == 3, 270 deg cw */
        return boxCreate(by, xdist, bh, bw);
}


/*---------------------------------------------------------------------*
 *                              Boxa sort                              *
 *---------------------------------------------------------------------*/
/*!
 *  boxaSort()
 * 
 *      Input:  boxa
 *              sorttype (L_SORT_BY_X, L_SORT_BY_Y, L_SORT_BY_WIDTH,
 *                        L_SORT_BY_HEIGHT, L_SORT_BY_MIN_DIMENSION,
 *                        L_SORT_BY_MAX_DIMENSION, L_SORT_BY_PERIMETER,
 *                        L_SORT_BY_AREA)
 *              sortorder  (L_SORT_INCREASING, L_SORT_DECREASING)
 *              &naindex (<optional return> index of sorted order into
 *                        original array)
 *      Return: boxad (sorted version of boxas), or null on error
 */
BOXA *
boxaSort(BOXA    *boxas,
         l_int32  sorttype,
         l_int32  sortorder,
         NUMA   **pnaindex)
{
l_int32  i, n, size;
BOX     *box;
BOXA    *boxad;
NUMA    *na, *naindex;

    PROCNAME("boxaSort");

    if (pnaindex) *pnaindex = NULL;
    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (sorttype != L_SORT_BY_X && sorttype != L_SORT_BY_Y && 
        sorttype != L_SORT_BY_WIDTH && sorttype != L_SORT_BY_HEIGHT &&
        sorttype != L_SORT_BY_MIN_DIMENSION &&
        sorttype != L_SORT_BY_MAX_DIMENSION &&
        sorttype != L_SORT_BY_PERIMETER && sorttype != L_SORT_BY_AREA)
        return (BOXA *)ERROR_PTR("invalid sort type", procName, NULL);
    if (sortorder != L_SORT_INCREASING && sortorder != L_SORT_DECREASING)
        return (BOXA *)ERROR_PTR("invalid sort order", procName, NULL);

        /* Build up numa of specific data */
    n = boxaGetCount(boxas);
    if ((na = numaCreate(n)) == NULL)
        return (BOXA *)ERROR_PTR("na not made", procName, NULL);
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxas, i, L_CLONE);
        if (!box)
          return (BOXA *)ERROR_PTR("box not found", procName, NULL);
        switch (sorttype)
        {
        case L_SORT_BY_X:
            numaAddNumber(na, box->x);
            break;
        case L_SORT_BY_Y:
            numaAddNumber(na, box->y);
            break;
        case L_SORT_BY_WIDTH:
            numaAddNumber(na, box->w);
            break;
        case L_SORT_BY_HEIGHT:
            numaAddNumber(na, box->h);
            break;
        case L_SORT_BY_MIN_DIMENSION:
            size = L_MIN(box->w, box->h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_MAX_DIMENSION:
            size = L_MAX(box->w, box->h);
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_PERIMETER:
            size = box->w + box->h;
            numaAddNumber(na, size);
            break;
        case L_SORT_BY_AREA:
            size = box->w * box->h;
            numaAddNumber(na, size);
            break;
        default:
            L_WARNING("invalid sort type", procName);
        }
        boxDestroy(&box);
    }

        /* Get the sort index for data array */
    if ((naindex = numaGetSortIndex(na, sortorder)) == NULL)
        return (BOXA *)ERROR_PTR("naindex not made", procName, NULL);

        /* Build up sorted boxa using sort index */
    boxad = boxaSortByIndex(boxas, naindex);

    if (pnaindex)
        *pnaindex = naindex;
    else
        numaDestroy(&naindex);
    numaDestroy(&na);
    return boxad;
}


/*!
 *  boxaSortByIndex()
 * 
 *      Input:  boxas
 *              naindex (na that maps from the new boxa to the input boxa)
 *      Return: boxad (sorted), or null on error
 */
BOXA *
boxaSortByIndex(BOXA  *boxas,
                NUMA  *naindex)
{
l_int32  i, n, index;
BOX     *box;
BOXA    *boxad;

    PROCNAME("boxaSortByIndex");

    if (!boxas)
        return (BOXA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!naindex)
        return (BOXA *)ERROR_PTR("naindex not defined", procName, NULL);

    n = boxaGetCount(boxas);
    boxad = boxaCreate(n);
    for (i = 0; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        box = boxaGetBox(boxas, index, L_COPY);
        boxaAddBox(boxad, box, L_INSERT);
    }

    return boxad;
}


/*!
 *  boxaSort2d()
 * 
 *      Input:  boxas
 *              &naa (<optional return> numaa with sorted indices
 *                    whose values are the indices of the input array)
 *              delta1 (min overlap that permits aggregation of a box
 *                      onto a boxa of horizontally-aligned boxes; pass 1)
 *              delta2 (min overlap that permits aggregation of a box
 *                      onto a boxa of horizontally-aligned boxes; pass 2)
 *              minh1 (components less than this height either join an
 *                     existing boxa or are set aside for pass 2)
 *      Return: boxaa (2d sorted version of boxa), or null on error
 *
 *  Notes:
 *      (1) The final result is a sort where the 'fast scan' direction is
 *          left to right, and the 'slow scan' direction is from top
 *          to bottom.  Each boxa in the boxaa represents a sorted set
 *          of boxes from left to right.
 *      (2) Two passes are used to aggregate the boxas, which can corresond
 *          to characters or words in a line of text.  In pass 1, only
 *          taller components, which correspond to xheight or larger,
 *          are permitted to start a new boxa, whereas in pass 2,
 *          the remaining vertically-challenged components are allowed
 *          to join an existing boxa or start a new one.
 *      (3) If delta1 < 0, the first pass allows aggregation when
 *          boxes in the same boxa do not overlap vertically.
 *          The distance by which they can miss and still be aggregated
 *          is the absolute value |delta1|.   Similar for delta2 on
 *          the second pass.
 *      (4) On the first pass, any component of height less than minh1
 *          cannot start a new boxa; it's put aside for later insertion.
 *      (5) On the second pass, any small component that doesn't align
 *          with an existing boxa can start a new one.
 *      (6) This can be used to identify lines of text from
 *          character or word bounding boxes.
 */
BOXAA *
boxaSort2d(BOXA    *boxas,
           NUMAA  **pnaad,
           l_int32  delta1,
           l_int32  delta2,
           l_int32  minh1)
{
l_int32  i, index, h, nt, ne, n, m, ival;
BOX     *box;
BOXA    *boxa, *boxae, *boxan, *boxat1, *boxat2, *boxav, *boxavs;
BOXAA   *baa, *baad;
NUMA    *naindex, *nae, *nan, *nah, *nav, *nat1, *nat2, *nad;
NUMAA   *naa, *naad;

    PROCNAME("boxaSort2d");

    if (pnaad) *pnaad = NULL;
    if (!boxas)
        return (BOXAA *)ERROR_PTR("boxas not defined", procName, NULL);

        /* Sort from left to right */
    if ((boxa = boxaSort(boxas, L_SORT_BY_X, L_SORT_INCREASING, &naindex))
                    == NULL)
        return (BOXAA *)ERROR_PTR("boxa not made", procName, NULL);

        /* First pass: assign taller boxes to boxa by row */
    nt = boxaGetCount(boxa);
    baa = boxaaCreate(0);
    naa = numaaCreate(0);
    boxae = boxaCreate(0);  /* save small height boxes here */
    nae = numaCreate(0);  /* keep track of small height boxes */
    for (i = 0; i < nt; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        boxGetGeometry(box, NULL, NULL, NULL, &h);
        if (h < minh1) {  /* save for 2nd pass */
            boxaAddBox(boxae, box, L_INSERT);
            numaAddNumber(nae, i);
        }
        else {
            n = boxaaGetCount(baa);
            boxaaAlignBox(baa, box, delta1, &index);
            if (index < n) {  /* append to an existing boxa */
                boxaaAddBox(baa, index, box, L_INSERT);
            }
            else {  /* doesn't align, need new boxa */
                boxan = boxaCreate(0);
                boxaAddBox(boxan, box, L_INSERT);
                boxaaAddBoxa(baa, boxan, L_INSERT);
                nan = numaCreate(0);
                numaaAddNuma(naa, nan, L_INSERT);
            }
            numaGetIValue(naindex, i, &ival);
            numaaAddNumber(naa, index, ival);
        }
    }
    boxaDestroy(&boxa);
    numaDestroy(&naindex);

        /* Second pass: feed in small height boxes;
         * TODO: this correctly, using local y position! */
    ne = boxaGetCount(boxae);
    for (i = 0; i < ne; i++) {
        box = boxaGetBox(boxae, i, L_CLONE);
        n = boxaaGetCount(baa);
        boxaaAlignBox(baa, box, delta2, &index);
        if (index < n) {  /* append to an existing boxa */
            boxaaAddBox(baa, index, box, L_INSERT);
        }
        else {  /* doesn't align, need new boxa */
            boxan = boxaCreate(0);
            boxaAddBox(boxan, box, L_INSERT);
            boxaaAddBoxa(baa, boxan, L_INSERT);
            nan = numaCreate(0);
            numaaAddNuma(naa, nan, L_INSERT);
        }
        numaGetIValue(nae, i, &ival);  /* location in original boxas */
        numaaAddNumber(naa, index, ival);
    }

        /* Sort each boxa in the boxaa */
    m = boxaaGetCount(baa);
    for (i = 0; i < m; i++) {
        boxat1 = boxaaGetBoxa(baa, i, L_CLONE);
        boxat2 = boxaSort(boxat1, L_SORT_BY_X, L_SORT_INCREASING, &nah);
        boxaaReplaceBoxa(baa, i, boxat2);
        nat1 = numaaGetNuma(naa, i, L_CLONE);
        nat2 = numaSortByIndex(nat1, nah);
        numaaReplaceNuma(naa, i, nat2);
        boxaDestroy(&boxat1);
        numaDestroy(&nat1);
        numaDestroy(&nah);
    }

        /* Sort boxa vertically within boxaa, using the first box
         * in each boxa. */
    m = boxaaGetCount(baa);
    boxav = boxaCreate(m);  /* holds first box in each boxa in baa */
    naad = numaaCreate(m);
    if (pnaad)
        *pnaad = naad;
    baad = boxaaCreate(m);
    for (i = 0; i < m; i++) {
        boxat1 = boxaaGetBoxa(baa, i, L_CLONE);
        box = boxaGetBox(boxat1, 0, L_CLONE); 
        boxaAddBox(boxav, box, L_INSERT);
        boxaDestroy(&boxat1);
    }
    boxavs = boxaSort(boxav, L_SORT_BY_Y, L_SORT_INCREASING, &nav);
    for (i = 0; i < m; i++) {
        numaGetIValue(nav, i, &index);
        boxa = boxaaGetBoxa(baa, index, L_CLONE);
        boxaaAddBoxa(baad, boxa, L_INSERT);
        nad = numaaGetNuma(naa, index, L_CLONE);
        numaaAddNuma(naad, nad, L_INSERT);
    }

/*    fprintf(stderr, "box count = %d, numaa count = %d\n", nt,
            numaaGetNumberCount(naad)); */

    boxaaDestroy(&baa);
    boxaDestroy(&boxav);
    boxaDestroy(&boxavs);
    boxaDestroy(&boxae);
    numaDestroy(&nav);
    numaDestroy(&nae);
    numaaDestroy(&naa);
    if (!pnaad)
        numaaDestroy(&naad);

    return baad;
}


/*!
 *  boxaSort2dByIndex()
 * 
 *      Input:  boxas
 *              naa (numaa that maps from the new baa to the input boxa)
 *      Return: baa (sorted boxaa), or null on error
 */
BOXAA *
boxaSort2dByIndex(BOXA   *boxas,
                  NUMAA  *naa)
{
l_int32  ntot, boxtot, i, j, n, nn, index;
BOX     *box;
BOXA    *boxa;
BOXAA   *baa;
NUMA    *na;

    PROCNAME("boxaSort2dByIndex");

    if (!boxas)
        return (BOXAA *)ERROR_PTR("boxas not defined", procName, NULL);
    if (!naa)
        return (BOXAA *)ERROR_PTR("naindex not defined", procName, NULL);

        /* Check counts */
    ntot = numaaGetNumberCount(naa);
    boxtot = boxaGetCount(boxas);
    if (ntot != boxtot)
        return (BOXAA *)ERROR_PTR("element count mismatch", procName, NULL);

    n = numaaGetCount(naa);
    baa = boxaaCreate(n);
    for (i = 0; i < n; i++) {
        na = numaaGetNuma(naa, i, L_CLONE);
        nn = numaGetCount(na);
        boxa = boxaCreate(nn);
        for (j = 0; j < nn; j++) {
            numaGetIValue(na, i, &index);
            box = boxaGetBox(boxas, index, L_COPY);
            boxaAddBox(boxa, box, L_INSERT);
        }
        boxaaAddBoxa(baa, boxa, L_INSERT);
        numaDestroy(&na);
    }

    return baa;
}


/*---------------------------------------------------------------------*
 *                        Other Boxaa functions                        *
 *---------------------------------------------------------------------*/
/*!
 *  boxaaGetExtent()
 *
 *      Input:  boxaa
 *              &w  (<optional return> width)
 *              &h  (<optional return> height)
 *              &box (<optional return>, minimum box containing all boxa
 *                    in boxaa)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The returned w and h are the minimum size image
 *          that would contain all boxes untranslated.
 */
l_int32
boxaaGetExtent(BOXAA    *boxaa,
               l_int32  *pw,
               l_int32  *ph,
               BOX     **pbox)
{
l_int32  i, j, n, m, x, y, w, h, xmax, ymax, xmin, ymin;
BOXA    *boxa;

    PROCNAME("boxaaGetExtent");

    if (!pw && !ph && !pbox)
        return ERROR_INT("no ptrs defined", procName, 1);
    if (pbox) *pbox = NULL;
    if (pw) *pw = 0;
    if (ph) *ph = 0;
    if (!boxaa)
        return ERROR_INT("boxaa not defined", procName, 1);

    n = boxaaGetCount(boxaa);
    if (n == 0)
        return ERROR_INT("no boxa in boxaa", procName, 1);

    xmax = ymax = 0;
    xmin = ymin = 100000000;
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(boxaa, i, L_CLONE);
        m = boxaGetCount(boxa);
        for (j = 0; j < m; j++) {
            boxaGetBoxGeometry(boxa, j, &x, &y, &w, &h);
            xmin = L_MIN(xmin, x);
            ymin = L_MIN(ymin, y);
            xmax = L_MAX(xmax, x + w);
            ymax = L_MAX(ymax, y + h);
        }
    }
    if (pw) *pw = xmax;
    if (ph) *ph = ymax;
    if (pbox)
      *pbox = boxCreate(xmin, ymin, xmax - xmin, ymax - ymin);

    return 0;
}


/*!
 *  boxaaFlattenToBoxa()
 *
 *      Input:  boxaa
 *              &naindex  (<optional return> the boxa index in the boxaa)
 *              copyflag  (L_COPY or L_CLONE)
 *      Return: boxa, or null on error
 *
 *  Notes:
 *      (1) This 'flattens' the boxaa to a boxa, taking the boxes in
 *          order in the first boxa, then the second, etc.
 *      (2) If &naindex is defined, we generate a Numa that gives, for
 *          each box in the boxaa, the index of the boxa to which it belongs.
 */
BOXA *
boxaaFlattenToBoxa(BOXAA   *baa,
                   NUMA   **pnaindex,
                   l_int32  copyflag)
{
l_int32  i, j, m, n;
BOXA    *boxa, *boxat;
BOX     *box;
NUMA    *naindex;

    PROCNAME("boxaaFlattenToBoxa");

    if (pnaindex) *pnaindex = NULL;
    if (!baa)
        return (BOXA *)ERROR_PTR("baa not defined", procName, NULL);
    if (copyflag != L_COPY && copyflag != L_CLONE)
        return (BOXA *)ERROR_PTR("invalid copyflag", procName, NULL);
    if (pnaindex) {
        naindex = numaCreate(0);
        *pnaindex = naindex;
    }

    n = boxaaGetCount(baa);
    boxa = boxaCreate(n);
    for (i = 0; i < n; i++) {
        boxat = boxaaGetBoxa(baa, i, L_CLONE);
        m = boxaGetCount(boxat);
        for (j = 0; j < m; j++) {
            box = boxaGetBox(boxat, j, copyflag);
            boxaAddBox(boxa, box, L_INSERT);
            if (pnaindex)
                numaAddNumber(naindex, i);  /* save 'row' number */
        }
        boxaDestroy(&boxat);
    }

    return boxa;
}


/*!
 *  boxaaAlignBox()
 * 
 *      Input:  boxaa
 *              box (to be aligned with the last of one of the boxa
 *                   in boxaa, if possible)
 *              delta (amount by which consecutive components can miss
 *                     in overlap and still be included in the array)
 *              &index (of boxa with best overlap, or if none match,
 *                      this is the index of the next boxa to be generated)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is not greedy; it finds the boxa whose last box has
 *          the biggest overlap with the input box.
 */
l_int32
boxaaAlignBox(BOXAA    *baa,
              BOX      *box,
              l_int32   delta,
              l_int32  *pindex)
{
l_int32  i, n, m, y, yt, h, ht, ovlp, maxovlp, maxindex;
BOXA    *boxa;

    PROCNAME("boxaaAlignBox");

    if (!baa)
        return ERROR_INT("baa not defined", procName, 1);
    if (!box)
        return ERROR_INT("box not defined", procName, 1);
    if (!pindex)
        return ERROR_INT("&index not defined", procName, 1);

    n = boxaaGetCount(baa);
    boxGetGeometry(box, NULL, &y, NULL, &h);
    maxovlp = -10000000;
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(baa, i, L_CLONE);
        if ((m = boxaGetCount(boxa)) == 0) {
            L_WARNING("no boxes in boxa", procName);
            continue;
        }
        boxaGetBoxGeometry(boxa, m - 1, NULL, &yt, NULL, &ht);  /* last one */
        boxaDestroy(&boxa);

            /* Overlap < 0 means the components do not overlap vertically */
        if (yt >= y)
            ovlp = y + h - 1 - yt;
        else
            ovlp = yt + ht - 1 - y;
        if (ovlp > maxovlp) {
            maxovlp = ovlp;
            maxindex = i;
        }
    }

    if (maxovlp + delta >= 0)
        *pindex = maxindex;
    else
        *pindex = n;
    return 0;
}


/*---------------------------------------------------------------------*
 *                     Boxa/Boxaa painting into Pix                    *
 *---------------------------------------------------------------------*/
/*!
 *  pixMaskConnComp()
 *
 *      Input:  pixs (1 bpp)
 *              connectivity (4 or 8)
 *              &boxa (<optional return> bounding boxes of c.c.)
 *      Return: pixd (1 bpp mask over the c.c.), or null on error
 *
 *  Notes:
 *      (1) This generates a mask image with ON pixels over the
 *          b.b. of the c.c. in pixs.  If there are no ON pixels in pixs,
 *          pixd will also have no ON pixels.
 */
PIX *
pixMaskConnComp(PIX     *pixs,
                l_int32  connectivity,
                BOXA   **pboxa)
{
BOXA  *boxa;
PIX   *pixd;

    PROCNAME("pixMaskConnComp");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (connectivity != 4 && connectivity != 8)
        return (PIX *)ERROR_PTR("connectivity not 4 or 8", procName, NULL);

    boxa = pixConnComp(pixs, NULL, connectivity);
    pixd = pixCreateTemplate(pixs);
    if (boxaGetCount(boxa) != 0)
        pixMaskBoxa(pixd, pixd, boxa, L_SET_PIXELS);
    if (pboxa)
        *pboxa = boxa;
    else
        boxaDestroy(&boxa);
    return pixd;
}


/*!
 *  pixMaskBoxa()
 *
 *      Input:  pixd (<optional> may be null)
 *              pixs (any depth; not cmapped)
 *              boxa (of boxes, to paint)
 *              op (L_SET_PIXELS, L_CLEAR_PIXELS, L_FLIP_PIXELS)
 *      Return: pixd (with masking op over the boxes), or null on error
 *
 *  Notes:
 *      (1) This can be used with:
 *              pixd = NULL  (makes a new pixd)
 *              pixd = pixs  (in-place)
 *      (2) If pixd == NULL, this first makes a copy of pixs, and then
 *          bit-twiddles over the boxes.  Otherwise, it operates directly
 *          on pixs.
 *      (3) This simple function is typically used with 1 bpp images.
 *          It uses the 1-image rasterop function, rasteropUniLow(),
 *          to set, clear or flip the pixels in pixd.  
 *      (4) If you want to generate a 1 bpp mask of ON pixels from the boxes
 *          in a Boxa, in a pix of size (w,h):
 *              pix = pixCreate(w, h, 1);
 *              pixMaskBoxa(pix, pix, boxa, L_SET_PIXELS);
 */
PIX *
pixMaskBoxa(PIX     *pixd,
            PIX     *pixs,
            BOXA    *boxa,
            l_int32  op)
{
l_int32  i, n, x, y, w, h;
BOX     *box;

    PROCNAME("pixMaskBoxa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetColormap(pixs))
        return (PIX *)ERROR_PTR("pixs is cmapped", procName, NULL);
    if (pixd && (pixd != pixs))
        return (PIX *)ERROR_PTR("if pixd, must be in-place", procName, NULL);
    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (op != L_SET_PIXELS && op != L_CLEAR_PIXELS && op != L_FLIP_PIXELS)
        return (PIX *)ERROR_PTR("invalid op", procName, NULL);

    pixd = pixCopy(pixd, pixs);
    if ((n = boxaGetCount(boxa)) == 0) {
        L_WARNING("no boxes to mask", procName);
        return pixd;
    }

    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        boxGetGeometry(box, &x, &y, &w, &h);
        if (op == L_SET_PIXELS)
            pixRasterop(pixd, x, y, w, h, PIX_SET, NULL, 0, 0);
        else if (op == L_CLEAR_PIXELS)
            pixRasterop(pixd, x, y, w, h, PIX_CLR, NULL, 0, 0);
        else  /* op == L_FLIP_PIXELS */
            pixRasterop(pixd, x, y, w, h, PIX_NOT(PIX_DST), NULL, 0, 0);
        boxDestroy(&box);
    }

    return pixd;
}


/*!
 *  pixPaintBoxa()
 *
 *      Input:  pixs (any depth, can be cmapped)
 *              boxa (of boxes, to paint)
 *              val (rgba color to paint)
 *      Return: pixd (with painted boxes), or null on error
 *
 *  Notes:
 *      (1) If pixs is 1 bpp or is colormapped, it is converted to 8 bpp
 *          and the boxa is painted using a colormap; otherwise,
 *          it is converted to 32 bpp rgb.
 *      (2) There are several ways to display a box on an image:
 *            * Paint it as a solid color
 *            * Draw the outline
 *            * Blend the outline or region with the existing image
 *          We provide painting and drawing here; blending is in blend.c.
 *          When painting or drawing, the result can be either a 
 *          cmapped image or an rgb image.  The dest will be cmapped
 *          if the src is either 1 bpp or has a cmap that is not full.
 *          To force RGB output, use pixConvertTo8(pixs, FALSE)
 *          before calling any of these paint and draw functions.
 */
PIX *
pixPaintBoxa(PIX      *pixs,
             BOXA     *boxa,
             l_uint32  val)
{
l_int32   i, n, d, rval, gval, bval, newindex;
l_int32   mapvacancy;   /* true only if cmap and not full */
BOX      *box;
PIX      *pixd;
PIXCMAP  *cmap;

    PROCNAME("pixPaintBoxa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);

    if ((n = boxaGetCount(boxa)) == 0) {
        L_WARNING("no boxes to paint; returning a copy", procName);
        return pixCopy(NULL, pixs);
    }

    mapvacancy = FALSE;
    if ((cmap = pixGetColormap(pixs)) != NULL) {
        if (pixcmapGetCount(cmap) < 256)
            mapvacancy = TRUE;
    }
    if (pixGetDepth(pixs) == 1 || mapvacancy)
        pixd = pixConvertTo8(pixs, TRUE);
    else
        pixd = pixConvertTo32(pixs);
    if (!pixd)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    d = pixGetDepth(pixd);
    if (d == 8) {  /* colormapped */
        cmap = pixGetColormap(pixd);
        extractRGBValues(val, &rval, &gval, &bval);
        if (pixcmapAddNewColor(cmap, rval, gval, bval, &newindex))
            return (PIX *)ERROR_PTR("cmap full; can't add", procName, NULL);
    }

    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        if (d == 8)
            pixSetInRectArbitrary(pixd, box, newindex);
        else
            pixSetInRectArbitrary(pixd, box, val);
        boxDestroy(&box);
    }

    return pixd;
}


/*!
 *  pixPaintBoxaRandom()
 *
 *      Input:  pixs (any depth, can be cmapped)
 *              boxa (of boxes, to paint)
 *      Return: pixd (with painted boxes), or null on error
 *
 *  Notes:
 *      (1) If pixs is 1 bpp, we paint the boxa using a colormap;
 *          otherwise, we convert to 32 bpp.
 *      (2) We use up to 254 different colors for painting the regions.
 *      (3) If boxes overlap, the later ones paint over earlier ones.
 */
PIX *
pixPaintBoxaRandom(PIX   *pixs,
                   BOXA  *boxa)
{
l_int32   i, n, d, rval, gval, bval, index;
l_uint32  val;
BOX      *box;
PIX      *pixd;
PIXCMAP  *cmap;

    PROCNAME("pixPaintBoxaRandom");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);

    if ((n = boxaGetCount(boxa)) == 0) {
        L_WARNING("no boxes to paint; returning a copy", procName);
        return pixCopy(NULL, pixs);
    }

    if (pixGetDepth(pixs) == 1)
        pixd = pixConvert1To8(NULL, pixs, 255, 0);
    else
        pixd = pixConvertTo32(pixs);
    if (!pixd)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    cmap = pixcmapCreateRandom(8);
    d = pixGetDepth(pixd);
    if (d == 8)  /* colormapped */
        pixSetColormap(pixd, cmap);

    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        index = (i % 254) + 1;
        if (d == 8)
            pixSetInRectArbitrary(pixd, box, index);
        else {  /* d == 32 */
            pixcmapGetColor(cmap, index, &rval, &gval, &bval);
            composeRGBPixel(rval, gval, bval, &val);
            pixSetInRectArbitrary(pixd, box, val);
        }
        boxDestroy(&box);
    }

    if (d == 32)
        pixcmapDestroy(&cmap);
    return pixd;
}


/*!
 *  pixDrawBoxa()
 *
 *      Input:  pixs (any depth, can be cmapped)
 *              boxa (of boxes, to draw)
 *              width (of lines)
 *              val (rgba color to draw)
 *      Return: pixd (with outlines of boxes added), or null on error
 *
 *  Notes:
 *      (1) If pixs is 1 bpp or is colormapped, it is converted to 8 bpp
 *          and the boxa is drawn using a colormap; otherwise,
 *          it is converted to 32 bpp rgb.
 */
PIX *
pixDrawBoxa(PIX      *pixs,
            BOXA     *boxa,
            l_int32   width,
            l_uint32  val)
{
l_int32   rval, gval, bval, newindex;
l_int32   mapvacancy;   /* true only if cmap and not full */
PIX      *pixd;
PIXCMAP  *cmap;

    PROCNAME("pixDrawBoxa");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (width < 1)
        return (PIX *)ERROR_PTR("width must be >= 1", procName, NULL);

    if (boxaGetCount(boxa) == 0) {
        L_WARNING("no boxes to draw; returning a copy", procName);
        return pixCopy(NULL, pixs);
    }

    mapvacancy = FALSE;
    if ((cmap = pixGetColormap(pixs)) != NULL) {
        if (pixcmapGetCount(cmap) < 256)
            mapvacancy = TRUE;
    }
    if (pixGetDepth(pixs) == 1 || mapvacancy)
        pixd = pixConvertTo8(pixs, TRUE);
    else
        pixd = pixConvertTo32(pixs);
    if (!pixd)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

    extractRGBValues(val, &rval, &gval, &bval);
    if (pixGetDepth(pixd) == 8) {  /* colormapped */
        cmap = pixGetColormap(pixd);
        pixcmapAddNewColor(cmap, rval, gval, bval, &newindex);
    }

    pixRenderBoxaArb(pixd, boxa, width, rval, gval, bval);
    return pixd;
}


/*!
 *  pixDrawBoxaRandom()
 *
 *      Input:  pixs (any depth, can be cmapped)
 *              boxa (of boxes, to draw)
 *              width (thickness of line)
 *      Return: pixd (with box outlines drawn), or null on error
 *
 *  Notes:
 *      (1) If pixs is 1 bpp, we draw the boxa using a colormap;
 *          otherwise, we convert to 32 bpp.
 *      (2) We use up to 254 different colors for drawing the boxes.
 *      (3) If boxes overlap, the later ones draw over earlier ones.
 */
PIX *
pixDrawBoxaRandom(PIX     *pixs,
                  BOXA    *boxa,
                  l_int32  width)
{
l_int32   i, n, rval, gval, bval, index;
BOX      *box;
PIX      *pixd;
PIXCMAP  *cmap;
PTAA     *ptaa;

    PROCNAME("pixDrawBoxaRandom");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (width < 1)
        return (PIX *)ERROR_PTR("width must be >= 1", procName, NULL);

    if ((n = boxaGetCount(boxa)) == 0) {
        L_WARNING("no boxes to draw; returning a copy", procName);
        return pixCopy(NULL, pixs);
    }

        /* Input depth = 1 bpp; generate cmapped output */
    if (pixGetDepth(pixs) == 1) {
        ptaa = generatePtaaBoxa(boxa);
        pixd = pixRenderRandomCmapPtaa(pixs, ptaa, width, 1);
        ptaaDestroy(&ptaa);
        return pixd;
    }

        /* Generate rgb output */
    pixd = pixConvertTo32(pixs);
    cmap = pixcmapCreateRandom(8);
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        index = (i % 254) + 1;
        pixcmapGetColor(cmap, index, &rval, &gval, &bval);
        pixRenderBoxArb(pixd, box, width, rval, gval, bval);
        boxDestroy(&box);
    }
    pixcmapDestroy(&cmap);
    return pixd;
}


/*!
 *  boxaaDisplay()
 *
 *      Input:  boxaa
 *              linewba (line width to display boxa)
 *              linewb (line width to display box)
 *              colorba (color to display boxa)
 *              colorb (color to display box)
 *              w (of pix; use 0 if determined by boxaa)
 *              h (of pix; use 0 if determined by boxaa)
 *      Return: 0 if OK, 1 on error
 */
PIX *
boxaaDisplay(BOXAA    *boxaa,
             l_int32   linewba,
             l_int32   linewb,
             l_uint32  colorba,
             l_uint32  colorb,
             l_int32   w,
             l_int32   h)
{
l_int32   i, j, n, m, rbox, gbox, bbox, rboxa, gboxa, bboxa;
BOX      *box;
BOXA     *boxa;
PIX      *pix;
PIXCMAP  *cmap;

    PROCNAME("boxaaDisplay");

    if (!boxaa)
        return (PIX *)ERROR_PTR("boxaa not defined", procName, NULL);
    if (w == 0 || h == 0) 
        boxaaGetExtent(boxaa, &w, &h, NULL);

    pix = pixCreate(w, h, 8);
    cmap = pixcmapCreate(8);
    pixSetColormap(pix, cmap);
    extractRGBValues(colorb, &rbox, &gbox, &bbox);
    extractRGBValues(colorba, &rboxa, &gboxa, &bboxa);
    pixcmapAddColor(cmap, 255, 255, 255);
    pixcmapAddColor(cmap, rbox, gbox, bbox);
    pixcmapAddColor(cmap, rboxa, gboxa, bboxa);
    
    n = boxaaGetCount(boxaa);
    for (i = 0; i < n; i++) {
        boxa = boxaaGetBoxa(boxaa, i, L_CLONE);   
        boxaGetExtent(boxa, NULL, NULL, &box);
        pixRenderBoxArb(pix, box, linewba, rboxa, gboxa, bboxa);
        boxDestroy(&box);
        m = boxaGetCount(boxa);
        for (j = 0; j < m; j++) {
            box = boxaGetBox(boxa, j, L_CLONE);
            pixRenderBoxArb(pix, box, linewb, rbox, gbox, bbox);
            boxDestroy(&box);
        }
        boxaDestroy(&boxa);
    }

    return pix;
}

