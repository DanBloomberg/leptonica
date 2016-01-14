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

#ifndef COLORQUANT_H
#define COLORQUANT_H


/*
 *  This data structure is used for pixOctreeColorQuant(),
 *  a color octree that adjusts to the color distribution
 *  in the image that is being quantized.  The best settings
 *  are with CQ_NLEVELS = 6 and DITHERING set on.
 *
 *  Notes:  (1) the CTE (color table entry) index is sequentially
 *              assigned as the tree is pruned back
 *          (2) if 'bleaf' == 1, all pixels in that cube have been
 *              assigned to one or more CTEs.  But note that if
 *              all 8 subcubes have 'bleaf' == 1, it will have no
 *              pixels left for assignment and will not be a CTE.
 *          (3) 'nleaves', the number of leaves contained at the next
 *              lower level is some number between 0 and 8, inclusive.
 *              If it is zero, it means that all colors within this cube
 *              are part of a single growing cluster that has not yet
 *              been set aside as a leaf.  If 'nleaves' > 0, 'bleaf'
 *              will be set to 1 and all pixels not assigned to leaves
 *              at lower levels will be assigned to a CTE here.
 *              (However, as described above, if all pixels are already
 *              assigned, we set 'bleaf' = 1 but do not create a CTE
 *              at this level.)
 *          (4) To keep the maximum color error to a minimum, we
 *              prune the tree back to level 2, and require that
 *              all 64 level 2 cells are CTEs.
 *
 */
struct ColorQuantCell
{
    l_int32     rc, gc, bc;   /* center values                              */
    l_int32     n;            /* number of samples in this cell             */
    l_int32     index;        /* CTE (color table entry) index              */
    l_int32     nleaves;      /* # of leaves contained at next lower level  */
    l_int32     bleaf;        /* boolean: 0 if not a leaf, 1 if so          */
};
typedef struct ColorQuantCell    CQCELL;


    /* for pixOctreeColorQuant */
static const l_int32  CQ_NLEVELS = 5;   /* only 4, 5 and 6 are allowed */
static const l_int32  CQ_RESERVED_COLORS = 64;  /* to allow for level 2 */
                                                /* remainder CTEs */


/*
 *  This data structure is used for pixOctreeQuant(),
 *  a color octree that adjusts in a simple way to the to the color
 *  distribution in the image that is being quantized.  It outputs
 *  colormapped images, either 4 bpp or 8 bpp, depending on the
 *  max number of colors and the compression desired. 
 *
 *  The number of samples is saved as a float in the first location,
 *  because this is required to use it as the key that orders the
 *  cells in the heap.
 */
struct OctcubeQuantCell
{
    l_float32  n;                  /* number of samples in this cell       */
    l_int32    octindex;           /* octcube index                        */
    l_int32    rcum, gcum, bcum;   /* cumulative values                    */
    l_int32    rval, gval, bval;   /* average values                       */
};
typedef struct OctcubeQuantCell    OQCELL;


#endif  /* COLORQUANT_H */


