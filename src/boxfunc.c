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
 *
 *      Boxa/Box transform (shift, scale) and orthogonal rotation
 *           BOXA     *boxaTransform()
 *           BOX      *boxTransform()
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
 *      Boxa/Boxaa display
 *           PIX      *boxaDisplay() 
 *           PIX      *boxaaDisplay() 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 *  Note: we shift first, then scale everything
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
 *  Note: we shift first, then scale everything.
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
 *                          Boxa/Boxaa display                         *
 *---------------------------------------------------------------------*/
/*!
 *  boxaDisplay()
 *
 *      Input:  boxa
 *              linewidth
 *              w (of pix; use 0 if determined by boxa)
 *              h (of pix; use 0 if determined by boxa)
 *      Return: 0 if OK, 1 on error
 */
PIX *
boxaDisplay(BOXA    *boxa,
            l_int32  linewidth,
            l_int32  w,
            l_int32  h)
{
l_int32  i, n;
BOX     *box;
PIX     *pix;

    PROCNAME("boxaDisplay");

    if (!boxa)
        return (PIX *)ERROR_PTR("boxa not defined", procName, NULL);
    if (w == 0 || h == 0) 
        boxaGetExtent(boxa, &w, &h, NULL);

    pix = pixCreate(w, h, 1);
    n = boxaGetCount(boxa);
    for (i = 0; i < n; i++) {
        box = boxaGetBox(boxa, i, L_CLONE);
        pixRenderBox(pix, box, linewidth, L_SET_PIXELS);
        boxDestroy(&box);
    }

    return pix;
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
    rbox = GET_DATA_BYTE(&colorb, COLOR_RED);
    gbox = GET_DATA_BYTE(&colorb, COLOR_GREEN);
    bbox = GET_DATA_BYTE(&colorb, COLOR_BLUE);
    rboxa = GET_DATA_BYTE(&colorba, COLOR_RED);
    gboxa = GET_DATA_BYTE(&colorba, COLOR_GREEN);
    bboxa = GET_DATA_BYTE(&colorba, COLOR_BLUE);
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

