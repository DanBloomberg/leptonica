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
 *  colorquant.c
 *                     
 *      One-pass color quantization from 24 bit full color,
 *      with fixed partitioning and 256 colors only
 *          PIX            *pixColorQuant1Pass()
 *
 *      Two-pass octree color quantization from 24 bit full color,
 *      with adaptive tree and variable number of colors
 *          PIX            *pixOctreeColorQuant()
 *
 *        which calls
 *          CQCELL       ***octreeGenerateAndPrune()
 *          PIX            *pixOctreeQuantizePixels()
 *
 *        which calls
 *          l_int32         octreeFindColorCell()
 *          
 *      Helper cqcell functions
 *          CQCELL       ***cqcellTreeCreate()
 *          void            cqcellTreeDestroy()
 *
 *      Helper index functions
 *          l_int32         makeRGBToIndexTables()
 *          void            getRGBFromOctcube()
 *          l_int32         getOctcubeIndices()
 *          void            getOctcubeIndexFromRGB()
 *          l_int32         octcubeGetCount()
 *
 *      Adaptive octree quantization to 4 and 8 bpp with colormap
 *          PIX            *pixOctreeQuant()
 *
 *      Fixed partition octcube quantization to arbitrary depth
 *          PIX            *pixFixedOctcubeQuant()
 *          PIX            *pixFixedOctcubeQuantRGB()
 *          PIX            *pixFixedOctcubeQuantCmap()
 *          PIX            *pixOctcubeQuantMixed()
 *          NUMA           *pixOctcubeHistogram()
 *          
 *      Color quantize RGB image using existing colormap
 *          PIX            *pixOctcubeQuantFromCmap()
 *          PIX            *pixOctcubeQuantFromCmapLUT()
 *
 *      Get filled octcube table from colormap
 *          l_int32        *pixcmapToOctcubeLUT()
 *
 *      Strip out unused elements in colormap
 *          l_int32         pixRemoveUnusedColors()
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

    /* in pixOctreeColorQuant() */
static const l_float32  SUBSAMPLE_FACTOR = 0.25;


#ifndef   NO_CONSOLE_IO
#define   DEBUG_OCTINDEX     0
#define   DEBUG_CMAP         0
#define   PRINT_STATISTICS   0
#endif   /* ~NO_CONSOLE_IO */


/*------------------------------------------------------------------*
 *                 Simple octree color quantization                 *
 *------------------------------------------------------------------*/
/*!
 *  pixColorQuant1Pass()
 *
 *      Input:  pixs  (32 bpp; 24-bit color)
 *              ditherflag  (1 for dithering; 0 for no dithering)
 *      Return: pixd (8 bit with colormap), or null on error
 *
 *  This simple 1-pass color quantization works by breaking the
 *  color space into 256 pieces, with 3 bits quantized for each of
 *  red and green, and 2 bits quantized for blue.  We shortchange
 *  blue because the eye is least sensitive to blue.  This
 *  division of the color space is into two levels of octrees,
 *  followed by a further division by 4 (not 8), where both
 *  blue octrees have been combined in the third level. 
 *
 *  The color map is generated from the 256 color centers by
 *  taking the representative color to be the center of the
 *  cell volume.  This gives a maximum error in the red and
 *  green values of 16 levels, and a maximum error in the
 *  blue sample of 32 levels. 
 *
 *  Each pixel in the 24-bit color image is placed in its containing
 *  cell, given by the relevant MSbits of the red, green and blue
 *  samples.  An error-diffusion dithering is performed on each
 *  color sample to give the appearance of good average local color.
 *  Dithering is required; without it, the contouring and visible
 *  color errors are very bad.
 *
 *  I originally implemented this algorithm in two passes,
 *  where the first pass was used to compute the weighted average
 *  of each sample in each pre-allocated region of color space.
 *  The idea was to use these centroids in the dithering algorithm
 *  of the second pass, to reduce the average error that was
 *  being dithered.  However, with dithering, there is
 *  virtually no difference, so there is no reason to make the
 *  first pass.  Consequently, this 1-pass version just assigns
 *  the pixels to the centers of the pre-allocated cells.
 *  We use dithering to spread the difference between the sample
 *  value and the location of the center of the cell.  For speed
 *  and simplicity, we use integer dithering and propagate only
 *  to the right, down, and diagonally down-right, with ratios
 *  3/8, 3/8 and 1/4, respectively.  The results should be nearly
 *  as good, and a bit faster, with propagation only to the right
 *  and down.
 * 
 *  The algorithm is very fast, because there is no search,
 *  only fast generation of the cell index for each pixel.
 *  We use a simple mapping from the three 8 bit rgb samples
 *  to the 8 bit cell index; namely, (r7 r6 r5 g7 g6 g5 b7 b6).
 *  This is not in an octcube format, but it doesn't matter.
 *  There are no storage requirements.  We could keep a
 *  running average of the center of each sample in each
 *  cluster, rather than using the center of the cell, but
 *  this is just extra work, esp. with dithering.
 *
 *  The method implemented here is very simple and fast, and
 *  gives surprisingly good results with dithering.
 *  However, without dithering, the loss of color accuracy is
 *  evident in regions that are very light or that have subtle
 *  blending of colors.
 */
PIX *
pixColorQuant1Pass(PIX     *pixs,
                   l_int32  ditherflag)
{
l_uint8    rval, gval, bval;
l_uint8   *bufu8r, *bufu8g, *bufu8b;
l_int32   *buf1r, *buf1g, *buf1b, *buf2r, *buf2g, *buf2b;
l_int32    val1, val2, val3, dif;
l_uint8    index;
l_int32    w, h, wpls, wpld, i, j, cindex;
l_uint32  *ppixel;
l_uint32  *datas, *datad, *lines, *lined;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixColorQuant1Pass");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);

        /* Find the centers of the 256 cells, each of which represents
         * the 3 MSBits of the red and green components, and the
         * 2 MSBits of the blue component.  This gives a mapping
         * from a "cube index" to the rgb values.  Save all 256
         * rgb values of these centers in a colormap.
         * For example, to get the red color of the cell center,
         * you take the 3 MSBits of to the index and add the
         * offset to the center of the cell, which is 0x10. */
    cmap = pixcmapCreate(8);
    for (cindex = 0; cindex < 256; cindex++) {
        rval = (cindex & 0xe0) | 0x10;
        gval = ((cindex << 3) & 0xe0) | 0x10;
        bval = ((cindex << 6) & 0xc0) | 0x20;
        pixcmapAddColor(cmap, rval, gval, bval);
    }

        /* Make output 8 bpp palette image */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    if ((pixd = pixCreate(w, h, 8)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixSetColormap(pixd, cmap);
    pixCopyResolution(pixd, pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

        /* Set dest pix values to colortable indices */
    if (ditherflag == 0) {   /* no dithering */
        for (i = 0; i < h; i++) {
            lines = datas + i * wpls;
            lined = datad + i * wpld;
            for (j = 0; j < w; j++) {
                ppixel = lines + j;
                rval = GET_DATA_BYTE(ppixel, COLOR_RED);
                gval = GET_DATA_BYTE(ppixel, COLOR_GREEN);
                bval = GET_DATA_BYTE(ppixel, COLOR_BLUE);
                index = (rval & 0xe0) | ((gval >> 3) & 0x1c) | (bval >> 6);
                SET_DATA_BYTE(lined, j, index);
            }
        }
    }
    else {  /* ditherflag == 1 */
        bufu8r = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        bufu8g = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        bufu8b = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        buf1r = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf1g = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf1b = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2r = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2g = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2b = (l_int32 *)CALLOC(w, sizeof(l_int32));
        if (!bufu8r || !bufu8g || !bufu8b)
            return (PIX *)ERROR_PTR("uint8 mono line buf not made",
                procName, NULL);
        if (!buf1r || !buf1g || !buf1b || !buf2r || !buf2g || !buf2b)
            return (PIX *)ERROR_PTR("mono line buf not made", procName, NULL);

            /* Start by priming buf2; line 1 is above line 2 */
        pixGetRGBLine(pixs, 0, bufu8r, bufu8g, bufu8b);
        for (j = 0; j < w; j++) {
            buf2r[j] = 64 * bufu8r[j];
            buf2g[j] = 64 * bufu8g[j];
            buf2b[j] = 64 * bufu8b[j];
        }

        for (i = 0; i < h - 1; i++) {
                /* Swap data 2 --> 1, and read in new line 2 */
            memcpy(buf1r, buf2r, 4 * w);
            memcpy(buf1g, buf2g, 4 * w);
            memcpy(buf1b, buf2b, 4 * w);
            pixGetRGBLine(pixs, i + 1, bufu8r, bufu8g, bufu8b);
            for (j = 0; j < w; j++) {
                buf2r[j] = 64 * bufu8r[j];
                buf2g[j] = 64 * bufu8g[j];
                buf2b[j] = 64 * bufu8b[j];
            }

                /* Dither */
            lined = datad + i * wpld;
            for (j = 0; j < w - 1; j++) {
                rval = buf1r[j] / 64;
                gval = buf1g[j] / 64;
                bval = buf1b[j] / 64;
                index = (rval & 0xe0) | ((gval >> 3) & 0x1c) | (bval >> 6);
                SET_DATA_BYTE(lined, j, index);

                dif = buf1r[j] / 8 - 8 * ((rval | 0x10) & 0xf0);
                if (dif != 0) {
                    val1 = buf1r[j + 1] + 3 * dif;
                    val2 = buf2r[j] + 3 * dif;
                    val3 = buf2r[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1r[j + 1] = L_MIN(16383, val1);
                        buf2r[j] = L_MIN(16383, val2);
                        buf2r[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1r[j + 1] = L_MAX(0, val1);
                        buf2r[j] = L_MAX(0, val2);
                        buf2r[j + 1] = L_MAX(0, val3);
                    }
                }

                dif = buf1g[j] / 8 - 8 * ((gval | 0x10) & 0xf0);
                if (dif != 0) {
                    val1 = buf1g[j + 1] + 3 * dif;
                    val2 = buf2g[j] + 3 * dif;
                    val3 = buf2g[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1g[j + 1] = L_MIN(16383, val1);
                        buf2g[j] = L_MIN(16383, val2);
                        buf2g[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1g[j + 1] = L_MAX(0, val1);
                        buf2g[j] = L_MAX(0, val2);
                        buf2g[j + 1] = L_MAX(0, val3);
                    }
                }

                dif = buf1b[j] / 8 - 8 * ((bval | 0x20) & 0xe0);
                if (dif != 0) {
                    val1 = buf1b[j + 1] + 3 * dif;
                    val2 = buf2b[j] + 3 * dif;
                    val3 = buf2b[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1b[j + 1] = L_MIN(16383, val1);
                        buf2b[j] = L_MIN(16383, val2);
                        buf2b[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1b[j + 1] = L_MAX(0, val1);
                        buf2b[j] = L_MAX(0, val2);
                        buf2b[j + 1] = L_MAX(0, val3);
                    }
                }
            }

                /* Get last pixel in row; no downward propagation */
            rval = buf1r[w - 1] / 64;
            gval = buf1g[w - 1] / 64;
            bval = buf1b[w - 1] / 64;
            index = (rval & 0xe0) | ((gval >> 3) & 0x1c) | (bval >> 6);
            SET_DATA_BYTE(lined, w - 1, index);
        }

            /* Get last row of pixels; no leftward propagation */
        lined = datad + (h - 1) * wpld;
        for (j = 0; j < w; j++) {
            rval = buf2r[j] / 64;
            gval = buf2g[j] / 64;
            bval = buf2b[j] / 64;
            index = (rval & 0xe0) | ((gval >> 3) & 0x1c) | (bval >> 6);
            SET_DATA_BYTE(lined, j, index);
        }

        FREE(bufu8r);
        FREE(bufu8g);
        FREE(bufu8b);
        FREE(buf1r);
        FREE(buf1g);
        FREE(buf1b);
        FREE(buf2r);
        FREE(buf2g);
        FREE(buf2b);
    }

    return pixd;
}


/*------------------------------------------------------------------*
 *                 Better octree color quantization                 *
 *------------------------------------------------------------------*/
/*!
 *  pixOctreeColorQuant()
 *
 *      Input:  pixs  (32 bpp; 24-bit color)
 *              colors  (in colormap; some number in range [32 ... 256];
 *                      the actual number of colors used will be smaller)
 *              ditherflag  (1 to dither, 0 otherwise)
 *      Return: pixd (8 bit with colormap), or null on error
 *
 *  I found one description in the literature of octree color
 *  quantization, using progressive truncation of the octree,
 *  by M. Gervautz and W. Purgathofer in Graphics Gems, pp.
 *  287-293, ed. A. Glassner, Academic Press, 1990.
 *  Rather than setting up a fixed partitioning of the color
 *  space ab initio, as we do here, they allow the octree to be
 *  progressively truncated as new pixels are added.  They
 *  need to set up some data structures that are traversed
 *  with the addition of each 24 bit pixel, in order to decide
 *  either (1) in which cluster (sub-branch of the octree) to put
 *  the pixel, or (2) whether to truncate the octree further
 *  to place the pixel in an existing cluster, or (3) which
 *  two existing clusters should be merged so that the pixel
 *  can be left to start a truncated leaf of the octree.  Such dynamic
 *  truncation is considerably more complicated, and Gervautz et
 *  al. did not explain how they did it in anywhere near the
 *  detail required to check their implementation.
 *
 *  The simple method in pixColorQuant1Pass() is very
 *  fast, and with dithering the results are good, but you
 *  can do better if the color clusters are selected adaptively
 *  from the image.  We want a method that makes much better
 *  use of color samples in regions of color space with high
 *  pixel density, while also fairly representing small numbers
 *  of color pixels in low density regions.  This will require
 *  two passes through the image: the first for generating the pruned
 *  tree of color cubes and the second for computing the index
 *  into the color table for each pixel.  We perform the first
 *  pass on a subsampled image, because we do not need to use
 *  all the pixels in the image to generate the tree.  Subsampling
 *  down to 0.25 (1/16 of the pixels) makes the program run
 *  about 1.3 times faster.
 *
 *  Instead of dividing the color space into 256 equal-sized
 *  regions, we initially divide it into 2^12 or 2^15 or 2^18
 *  equal-sized octcubes.  Suppose we choose to use 2^18 octcubes.  
 *  This gives us 6 octree levels.  We then prune back,
 *  starting from level 6.  For every cube at level 6, there
 *  are 8 cubes at level 5.  Call the operation of putting a
 *  cube aside as a color table entry (CTE) a "saving." 
 *  We use a (in general) level-dependent threshold, and save
 *  those level 6 cubes that are above threshold.
 *  The rest are combined into the containing level 5 cube.
 *  If between 1 and 7 level 6 cubes within a level 5
 *  cube have been saved by thresholding, then the remaining
 *  level 6 cubes in that level 5 cube are automatically
 *  saved as well, without applying a threshold.  This greatly
 *  simplifies both the description of the CTEs and the later
 *  classification of each pixel as belonging to a CTE.
 *  This procedure is iterated through every cube, starting at
 *  level 5, and then 4, 3, and 2, successively.  The result is that
 *  each CTE contains the entirety of a set of from 1 to 7 cubes
 *  from a given level that all belong to a single cube at the
 *  level above.   We classify the CTEs in terms of the
 *  condition in which they are made as either being "threshold"
 *  or "residual."  They are "threshold" CTEs if no subcubes
 *  are CTEs (that is, they contain every pixel within the cube)
 *  and the number of pixels exceeds the threshold for making 
 *  a CTE.  They are "residual" CTEs if at least one but not more
 *  than 7 of the subcubes have already been determined to be CTEs;
 *  this happens automatically -- no threshold is applied.
 *  If all 8 subcubes are determined to be CTEs, the cube is
 *  marked as having all pixels accounted for ('bleaf' = 1) but
 *  is not saved as a CTE.
 *
 *  We stop the pruning at level 2, at which there are 64
 *  sub-cubes.  Any pixels not already claimed in a CTE are
 *  put in these cubes.  
 *
 *  As the cubes are saved as color samples in the color table,
 *  the number of remaining pixels P and the number of
 *  remaining colors in the color table N are recomputed,
 *  along with the average number of pixels P/N (ppc) to go in
 *  each of the remaining colors.  This running average number is
 *  used to set the threshold at the current level.
 *
 *  Because we are going to very small cubes at levels 6 or 5,
 *  and will dither the colors for errors, it is not necessary 
 *  to compute the color center of each cluster; we can simply
 *  use the center of the cube.  This gives us a minimax error
 *  condition: the maximum error is half the width of the 
 *  level 2 cubes -- 32 color values out of 256 -- for each color
 *  sample.  In practice, most of the pixels will be very much
 *  closer to the center of their cells.  And with dithering,
 *  the average pixel color in a small region will be closer still.
 *  Thus with the octree quantizer, we are able to capture
 *  regions of high color pdf (probability density function) in small
 *  but accurate CTEs, and to have only a small number of pixels
 *  that end up a significant distance (with a guaranteed maximum)
 *  from their true color.
 *
 *  How should the threshold factor vary?  Threshold factors
 *  are required for levels 2, 3, 4 and 5 in the pruning stage.
 *  The threshold for level 5 is actually applied to cubes at
 *  level 6, etc.  From various experiments, it appears that
 *  the results do not vary appreciably for threshold values near 1.0.
 *  If you want more colors in smaller cubes, the threshold
 *  factors can be set lower than 1.0 for cubes at levels 4 and 5. 
 *  However, if the factor is set much lower than 1.0 for
 *  levels 2 and 3, we can easily run out of colors.
 *  We put aside 64 colors in the calculation of the threshold
 *  values, because we must have 64 color centers at level 2,
 *  that will have very few pixels in most of them.
 *  If we reduce the factor for level 5 to 0.4, this will
 *  generate many level 6 CTEs, and consequently
 *  many residual cells will be formed up from those leaves,
 *  resulting in the possibility of running out of colors.
 *  Remember, the residual CTEs are mandatory, and are formed
 *  without using the threshold, regardless of the number of
 *  pixels that are absorbed.
 *      
 *  The implementation logically has four parts:
 *
 *       (1) accumulation into small, fixed cells
 *       (2) pruning back into selected CTE cubes
 *       (3) organizing the CTEs for fast search to find
 *           the CTE to which any image pixel belongs
 *       (4) doing a second scan to code the image pixels by CTE
 *
 *  Step (1) is straightforward; we use 2^15 cells.
 *
 *  We've already discussed how the pruning step (2) will be performed.
 *
 *  Steps (3) and (4) are related, in that the organization
 *  used by step (3) determines how the search actually
 *  takes place for each pixel in step (4).
 *
 *  There are many ways to do step (3).  Let's explore a few.
 *
 *  (a) The simplest is to order the cubes from highest occupancy
 *      to lowest, and traverse the list looking for the deepest
 *      match.  To make this more efficient, so that we know when
 *      to stop looking, any cube that has separate CTE subcubes
 *      would be marked as such, so that we know when we hit a 
 *      true leaf.
 *
 *  (b) Alternatively, we can order the cubes by highest
 *      occupancy separately each level, and work upward,
 *      starting at level 5, so that when we find a match we
 *      know that it will be correct. 
 *
 *  (c) Another approach would be to order the cubes by
 *      "address" and use a hash table to find the cube
 *      corresponding to a pixel color.  I don't know how to
 *      do this with a variable length address, as each CTE
 *      will have 3*n bits, where n is the level.
 *
 *  (d) Another approach entirely is to put the CTE cubes into
 *      a tree, in such a way that starting from the root, and
 *      using 3 bits of address at a time, the correct branch of
 *      each octree can be taken until a leaf is found.  Because
 *      a given cube can be both a leaf and also have branches
 *      going to sub-cubes, the search stops only when no
 *      marked subcubes have addresses that match the given pixel.
 *
 *      In the tree method, we can start with a dense infrastructure,
 *      and place the leaves corresponding to the N colors
 *      in the tree, or we can grow from the root only those
 *      branches that end directly on leaves.
 *
 *  What we do here is to take approach (d), and implement the tree
 *  "virtually", as a set of arrays, one array for each level
 *  of the tree.   Initially we start at level 5, an array with
 *  2^15 cubes, each with 8 subcubes.  We then build nodes at
 *  levels closer to the root; at level 4 there are 2^12 nodes
 *  each with 8 subcubes; etc.  Using these arrays has
 *  several advantages:
 *
 *     -  We don't need to keep track of links between cubes
 *        and subcubes, because we can use the canonical
 *        addressing on the cell arrays directly to determine
 *        which nodes are parent cubes and which are sub-cubes.
 *
 *     -  We can prune directly on this tree
 *
 *     -  We can navigate the pruned tree quickly to classify
 *        each pixel in the image.
 *      
 *  Canonical addressing guarantees that the i-th node at level k
 *  has 8 subnodes given by the 8*i ... 8*i+7 nodes at level k+1.
 *
 *  The pruning step works as follows.  We go from the lowest
 *  level up.  At each level, the threshold is found from the
 *  product of a factor near 1.0 and the ratio of unmarked pixels
 *  to remaining colors (minus the 64).  We march through
 *  the space, sequentially considering a cube and its 8 subcubes.
 *  We first check those subcubes that are not already
 *  marked as CTE to see if any are above threshold, and if so,
 *  generate a CTE and mark them as such.
 *  We then determine if any of the subcubes have been marked.
 *  If so, and there are subcubes that are not marked,
 *  we generate a CTE for the cube from the remaining unmarked
 *  subcubes; this is mandatory and does not depend on how many
 *  pixels are in the set of subcubes.  If none of the subcubes
 *  are marked, we aggregate their pixels into the cube
 *  containing them, but do not mark it as a CTE; that
 *  will be determined when iterating through the next level up.
 *
 *  When all the pixels in a cube are accounted for in one or more
 *  colors, we set the boolean 'bleaf' to true.  This is the
 *  flag used to mark the cubes in the pruning step.  If a cube
 *  is marked, and all 8 subcubes are marked, then it is not
 *  itself given a CTE because all pixels have already been
 *  accounted for.
 *
 *  Note that the pruning of the tree and labelling of the CTEs
 *  (step 2) accomplishes step 3 implicitly, because the marked
 *  and pruned tree is ready for use in labelling each pixel
 *  in step 4.  We now, for every pixel in the image, traverse
 *  the tree from the root, looking for the lowest cube that is a leaf.
 *  At each level we have a cube and subcube.  If we reach a subcube
 *  leaf that is marked 0, we know that the color is stored in the
 *  cube above, and we've found the CTE.  Otherwise, the subcube
 *  leaf is marked 1.  If we're at the last level, we've reached
 *  the final leaf and must use it.  Otherwise, continue the
 *  process at the next level down.
 */
PIX *
pixOctreeColorQuant(PIX      *pixs,
                    l_int32   colors,
                    l_int32   ditherflag)
{
CQCELL  ***cqcaa;
PIX       *pixd, *pixsub;
PIXCMAP   *cmap;

    PROCNAME("pixOctreeColorQuant");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (colors < 128 || colors > 256)
        return (PIX *)ERROR_PTR("colors must be in [128, 256]", procName, NULL);

        /* Subsample to speed up the first pass */
    if ((pixsub = pixScaleBySampling(pixs, SUBSAMPLE_FACTOR, SUBSAMPLE_FACTOR))
            == NULL)
        return (PIX *)ERROR_PTR("pixsub not made", procName, NULL);

        /* Make the pruned octree */
    cqcaa = octreeGenerateAndPrune(pixsub, colors, CQ_RESERVED_COLORS, &cmap);
    if (!cqcaa)
        return (PIX *)ERROR_PTR("tree not made", procName, NULL);
    L_INFO_INT(" Colors requested = %d", procName, colors);
    L_INFO_INT(" Actual colors = %d", procName, cmap->n);

        /* Traverse tree from root, looking for lowest cube
         * that is a leaf, and set dest pix value to its 
         * colortable index */
    if ((pixd = pixOctreeQuantizePixels(pixs, cqcaa, ditherflag)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);

        /* Attach colormap and copy res */
    pixSetColormap(pixd, cmap);
    pixCopyResolution(pixd, pixs);

    cqcellTreeDestroy(&cqcaa);
    pixDestroy(&pixsub);
    return pixd;
}


/*!
 *  octreeGenerateAndPrune()
 *
 *      Input:  pixs
 *              number of colors to use (between 128 and 256)
 *              number of reserved colors
 *              &cmap  (made and returned)
 *      Return: octree, colormap and number of colors used, or null
 *              on error
 *
 *  Notes:
 *      (1) The number of colors in the cmap may differ from the number
 *          of colors requested, but it will not be larger than 256
 */
CQCELL ***
octreeGenerateAndPrune(PIX       *pixs,
                       l_int32    colors,
                       l_int32    reservedcolors,
                       PIXCMAP  **pcmap)
{
l_int32    rval, gval, bval;
l_int32    level, ncells, octindex;
l_int32    w, h, wpls;
l_int32    i, j, isub;
l_int32    npix;  /* number of remaining pixels to be assigned */
l_int32    ncolor; /* number of remaining color cells to be used */
l_int32    ppc;  /* ave number of pixels left for each color cell */
l_int32    rv, gv, bv;
l_float32  thresholdFactor[] = {0.01, 0.01, 1.0, 1.0, 1.0, 1.0};
l_float32  thresh;  /* factor of ppc for this level */
l_uint32  *ppixel;
l_uint32  *datas, *lines;
l_uint32  *rtab, *gtab, *btab;
CQCELL  ***cqcaa;   /* one array for each octree level */
CQCELL   **cqca, **cqcasub;
CQCELL    *cqc, *cqcsub;
PIXCMAP   *cmap;
NUMA      *nat;  /* accumulates levels for threshold cells */
NUMA      *nar;  /* accumulates levels for residual cells */

    PROCNAME("octreeGenerateAndPrune");

    if (!pixs)
        return (CQCELL ***)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (CQCELL ***)ERROR_PTR("pixs must be 32 bpp", procName, NULL);
    if (colors < 128 || colors > 256)
        return (CQCELL ***)ERROR_PTR("colors not in [128,256]", procName, NULL);
    if (!pcmap)
        return (CQCELL ***)ERROR_PTR("&cmap not defined", procName, NULL);

        /* Make the canonical index tables */
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, CQ_NLEVELS))
        return (CQCELL ***)ERROR_PTR("tables not made", procName, NULL);

    if ((cqcaa = cqcellTreeCreate()) == NULL)
        return (CQCELL ***)ERROR_PTR("cqcaa not made", procName, NULL);

        /* Generate an 8 bpp cmap (max size 256) */
    cmap = pixcmapCreate(8);
    *pcmap = cmap;

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    npix = w * h;  /* initialize to all pixels */
    ncolor = colors - reservedcolors;  /* init to (almost) all colors */
    ppc = npix / ncolor;
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);

        /* Accumulate the centers of each cluster at level CQ_NLEVELS */
    ncells = 1 << (3 * CQ_NLEVELS);
    cqca = cqcaa[CQ_NLEVELS];
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        for (j = 0; j < w; j++) {
            ppixel = lines + j;
            rval = GET_DATA_BYTE(ppixel, COLOR_RED);
            gval = GET_DATA_BYTE(ppixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(ppixel, COLOR_BLUE);
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            cqc = cqca[octindex];
            cqc->n++;
        }
    }

        /* Arrays for storing statistics */
    if ((nat = numaCreate(0)) == NULL)
        return (CQCELL ***)ERROR_PTR("nat not made", procName, NULL);
    if ((nar = numaCreate(0)) == NULL)
        return (CQCELL ***)ERROR_PTR("nar not made", procName, NULL);

        /* Prune back from the lowest level and generate the colormap */
    for (level = CQ_NLEVELS - 1; level >= 2; level--) {
        thresh = thresholdFactor[level];
        cqca = cqcaa[level];
        cqcasub = cqcaa[level + 1];
        ncells = 1 << (3 * level);
        for (i = 0; i < ncells; i++) {  /* i is octindex at level */
            cqc = cqca[i];
            for (j = 0; j < 8; j++) {  /* check all subnodes */
                isub = 8 * i + j;   /* isub is octindex at level+1 */
                cqcsub = cqcasub[isub];
                if (cqcsub->bleaf == 1) {  /* already a leaf? */
                    cqc->nleaves++;   /* count the subcube leaves */
                    continue;
                }
                if (cqcsub->n >= thresh * ppc) {  /* make it a true leaf? */
                    cqcsub->bleaf = 1;
                    if (cmap->n < 256) {
                        cqcsub->index = cmap->n;  /* assign the color index */
                        getRGBFromOctcube(isub, level + 1, &rv, &gv, &bv); 
                        pixcmapAddColor(cmap, rv, gv, bv);
#if 1   /* save values */
                        cqcsub->rc = rv;
                        cqcsub->gc = gv;
                        cqcsub->bc = bv;
#endif
                    }
                    else {
                        L_WARNING("possibly assigned pixels to wrong color",
                                procName);
                        cqcsub->index = 255;  /* assign to the last; bad */
                        pixcmapGetColor(cmap, 255, &rval, &gval, &bval);
                        cqcsub->rc = rval;
                        cqcsub->gc = gval;
                        cqcsub->bc = bval;
                    }
                    cqc->nleaves++;
                    npix -= cqcsub->n;
                    ncolor--;
                    if (ncolor > 0)
                        ppc = npix / ncolor;
                    else if (ncolor + reservedcolors > 0)
                        ppc = npix / (ncolor + reservedcolors);
                    else
                        ppc = 1000000;  /* make it big */
                    numaAddNumber(nat, level + 1);

#if  DEBUG_CMAP
    fprintf(stderr, "Exceeds threshold: colors used = %d, colors remaining = %d\n",
                     cmap->n, ncolor + reservedcolors);
    fprintf(stderr, "  cell with %d pixels, npix = %d, ppc = %d\n",
                     cqcsub->n, npix, ppc);
    fprintf(stderr, "  index = %d, level = %d, subindex = %d\n",
                     i, level, j);
    fprintf(stderr, "  rv = %d, gv = %d, bv = %d\n", rv, gv, bv);
#endif  /* DEBUG_CMAP */

                }
            }
            if (cqc->nleaves > 0 || level == 2) { /* make the cube a leaf now */
                cqc->bleaf = 1;
                if (cqc->nleaves < 8) {  /* residual CTE cube: acquire the
                                          * remaining pixels */
                    for (j = 0; j < 8; j++) {  /* check all subnodes */
                        isub = 8 * i + j;
                        cqcsub = cqcasub[isub];
                        if (cqcsub->bleaf == 0)  /* absorb */
                            cqc->n += cqcsub->n;
                    }
                    if (cmap->n < 256) {
                        cqc->index = cmap->n;  /* assign the color index */
                        getRGBFromOctcube(i, level, &rv, &gv, &bv); 
                        pixcmapAddColor(cmap, rv, gv, bv);
#if 1   /* save values */
                        cqc->rc = rv;
                        cqc->gc = gv;
                        cqc->bc = bv;
#endif
                    }
                    else {
                        L_WARNING("possibly assigned pixels to wrong color",
                                procName);
                        cqc->index = 255;  /* assign to the last */
                        pixcmapGetColor(cmap, 255, &rval, &gval, &bval);
                        cqc->rc = rval;
                        cqc->gc = gval;
                        cqc->bc = bval;
                    }
                    npix -= cqc->n;
                    ncolor--;
                    if (ncolor > 0)
                        ppc = npix / ncolor;
                    else if (ncolor + reservedcolors > 0)
                        ppc = npix / (ncolor + reservedcolors);
                    else
                        ppc = 1000000;  /* make it big */
                    numaAddNumber(nar, level);

#if  DEBUG_CMAP
    fprintf(stderr, "By remainder: colors used = %d, colors remaining = %d\n",
                     cmap->n, ncolor + reservedcolors);
    fprintf(stderr, "  cell with %d pixels, npix = %d, ppc = %d\n",
                     cqc->n, npix, ppc);
    fprintf(stderr, "  index = %d, level = %d\n", i, level);
    fprintf(stderr, "  rv = %d, gv = %d, bv = %d\n", rv, gv, bv);
#endif  /* DEBUG_CMAP */

                }
            }
            else {  /* absorb all the subpixels but don't make it a leaf */
                for (j = 0; j < 8; j++) {  /* absorb from all subnodes */
                    isub = 8 * i + j;
                    cqcsub = cqcasub[isub];
                    cqc->n += cqcsub->n;
                }
            }
        }
    }

#if  PRINT_STATISTICS
{
l_int32    tc[] = {0, 0, 0, 0, 0, 0, 0};
l_int32    rc[] = {0, 0, 0, 0, 0, 0, 0};
l_int32    nt, nr, ival;

    nt = numaGetCount(nat);
    nr = numaGetCount(nar);
    for (i = 0; i < nt; i++) {
        numaGetIValue(nat, i, &ival);
        tc[ival]++;
    }
    for (i = 0; i < nr; i++) {
        numaGetIValue(nar, i, &ival);
        rc[ival]++;
    }
    fprintf(stderr, " Threshold cells formed: %d\n", nt);
    for (i = 1; i < CQ_NLEVELS + 1; i++)
        fprintf(stderr, "   level %d:  %d\n", i, tc[i]);
    fprintf(stderr, "\n Residual cells formed: %d\n", nr);
    for (i = 0; i < CQ_NLEVELS ; i++)
        fprintf(stderr, "   level %d:  %d\n", i, rc[i]);
}
#endif  /* PRINT_STATISTICS */

    numaDestroy(&nat);
    numaDestroy(&nar);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);

    return cqcaa;
}


/*!
 *  pixOctreeQuantizePixels()
 *
 *      Input:  pixs (32 bpp)
 *              octree in array format
 *              ditherflag (1 for dithering, 0 for no dithering)
 *      Return: pixd or null on error
 *
 *  Notes:
 *      (1) This routine doesn't need to use the CTEs (colormap
 *          table entries) because the color indices are embedded
 *          in the octree.  Thus, the calling program must make
 *          and attach the colormap to pixd after it is returned.
 *      (2) Dithering is performed in integers, effectively rounding
 *          to 1/8 sample increment.  The data in the integer buffers is
 *          64 times the sample values.  The 'dif' is 8 times the
 *          sample values, and this spread, multiplied by 8, to the
 *          integer buffers.  Because the dif is truncated to an
 *          integer, the dither is accurate to 1/8 of a sample increment,
 *          or 1/2048 of the color range.
 */
PIX *
pixOctreeQuantizePixels(PIX       *pixs,
                        CQCELL  ***cqcaa,
                        l_int32    ditherflag)
{
l_uint8    rval, gval, bval;
l_uint8   *bufu8r, *bufu8g, *bufu8b;
l_int32   *buf1r, *buf1g, *buf1b, *buf2r, *buf2g, *buf2b;
l_int32    octindex, index;
l_int32    val1, val2, val3, dif;
l_int32    w, h, wpls, wpld, i, j;
l_int32    rc, gc, bc;
l_uint32  *rtab, *gtab, *btab;
l_uint32  *ppixel;
l_uint32  *datas, *datad, *lines, *lined;
PIX       *pixd;

    PROCNAME("pixOctreeQuantizePixels");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs must be 32 bpp", procName, NULL);
    if (!cqcaa)
        return (PIX *)ERROR_PTR("cqcaa not defined", procName, NULL);

        /* Make the canonical index tables */
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, CQ_NLEVELS))
        return (PIX *)ERROR_PTR("tables not made", procName, NULL);

        /* Make output 8 bpp palette image */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    if ((pixd = pixCreate(w, h, 8)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

        /* Traverse tree from root, looking for lowest cube
         * that is a leaf, and set dest pix to its 
         * colortable index value.  The results are far
         * better when dithering to get a more accurate
         * average color.  */
    if (ditherflag == 0) {    /* no dithering */
        for (i = 0; i < h; i++) {
            lines = datas + i * wpls;
            lined = datad + i * wpld;
            for (j = 0; j < w; j++) {
                ppixel = lines + j;
                rval = GET_DATA_BYTE(ppixel, COLOR_RED);
                gval = GET_DATA_BYTE(ppixel, COLOR_GREEN);
                bval = GET_DATA_BYTE(ppixel, COLOR_BLUE);
                octindex = rtab[rval] | gtab[gval] | btab[bval];
                octreeFindColorCell(octindex, cqcaa, &index, &rc, &gc, &bc);
                SET_DATA_BYTE(lined, j, index);
            }
        }
    }
    else {  /* Dither */
        bufu8r = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        bufu8g = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        bufu8b = (l_uint8 *)CALLOC(w, sizeof(l_uint8));
        buf1r = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf1g = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf1b = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2r = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2g = (l_int32 *)CALLOC(w, sizeof(l_int32));
        buf2b = (l_int32 *)CALLOC(w, sizeof(l_int32));
        if (!bufu8r || !bufu8g || !bufu8b)
            return (PIX *)ERROR_PTR("uint8 mono line buf not made",
                procName, NULL);
        if (!buf1r || !buf1g || !buf1b || !buf2r || !buf2g || !buf2b)
            return (PIX *)ERROR_PTR("mono line buf not made", procName, NULL);

            /* Start by priming buf2; line 1 is above line 2 */
        pixGetRGBLine(pixs, 0, bufu8r, bufu8g, bufu8b);
        for (j = 0; j < w; j++) {
            buf2r[j] = 64 * bufu8r[j];
            buf2g[j] = 64 * bufu8g[j];
            buf2b[j] = 64 * bufu8b[j];
        }

        for (i = 0; i < h - 1; i++) {
                /* Swap data 2 --> 1, and read in new line 2 */
            memcpy(buf1r, buf2r, 4 * w);
            memcpy(buf1g, buf2g, 4 * w);
            memcpy(buf1b, buf2b, 4 * w);
            pixGetRGBLine(pixs, i + 1, bufu8r, bufu8g, bufu8b);
            for (j = 0; j < w; j++) {
                buf2r[j] = 64 * bufu8r[j];
                buf2g[j] = 64 * bufu8g[j];
                buf2b[j] = 64 * bufu8b[j];
            }

                /* Dither */
            lined = datad + i * wpld;
            for (j = 0; j < w - 1; j++) {
                rval = buf1r[j] / 64;
                gval = buf1g[j] / 64;
                bval = buf1b[j] / 64;
                octindex = rtab[rval] | gtab[gval] | btab[bval];
                octreeFindColorCell(octindex, cqcaa, &index, &rc, &gc, &bc);
                SET_DATA_BYTE(lined, j, index);

                dif = buf1r[j] / 8 - 8 * rc;
                if (dif != 0) {
                    val1 = buf1r[j + 1] + 3 * dif;
                    val2 = buf2r[j] + 3 * dif;
                    val3 = buf2r[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1r[j + 1] = L_MIN(16383, val1);
                        buf2r[j] = L_MIN(16383, val2);
                        buf2r[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1r[j + 1] = L_MAX(0, val1);
                        buf2r[j] = L_MAX(0, val2);
                        buf2r[j + 1] = L_MAX(0, val3);
                    }
                }

                dif = buf1g[j] / 8 - 8 * gc;
                if (dif != 0) {
                    val1 = buf1g[j + 1] + 3 * dif;
                    val2 = buf2g[j] + 3 * dif;
                    val3 = buf2g[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1g[j + 1] = L_MIN(16383, val1);
                        buf2g[j] = L_MIN(16383, val2);
                        buf2g[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1g[j + 1] = L_MAX(0, val1);
                        buf2g[j] = L_MAX(0, val2);
                        buf2g[j + 1] = L_MAX(0, val3);
                    }
                }

                dif = buf1b[j] / 8 - 8 * bc;
                if (dif != 0) {
                    val1 = buf1b[j + 1] + 3 * dif;
                    val2 = buf2b[j] + 3 * dif;
                    val3 = buf2b[j + 1] + 2 * dif;
                    if (dif > 0) {
                        buf1b[j + 1] = L_MIN(16383, val1);
                        buf2b[j] = L_MIN(16383, val2);
                        buf2b[j + 1] = L_MIN(16383, val3);
                    }
                    else if (dif < 0) {
                        buf1b[j + 1] = L_MAX(0, val1);
                        buf2b[j] = L_MAX(0, val2);
                        buf2b[j + 1] = L_MAX(0, val3);
                    }
                }
            }

                /* Get last pixel in row; no downward propagation */
            rval = buf1r[w - 1] / 64;
            gval = buf1g[w - 1] / 64;
            bval = buf1b[w - 1] / 64;
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            octreeFindColorCell(octindex, cqcaa, &index, &rc, &gc, &bc);
            SET_DATA_BYTE(lined, w - 1, index);
        }

            /* Get last row of pixels; no leftward propagation */
        lined = datad + (h - 1) * wpld;
        for (j = 0; j < w; j++) {
            rval = buf2r[j] / 64;
            gval = buf2g[j] / 64;
            bval = buf2b[j] / 64;
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            octreeFindColorCell(octindex, cqcaa, &index, &rc, &gc, &bc);
            SET_DATA_BYTE(lined, j, index);
        }

        FREE(bufu8r);
        FREE(bufu8g);
        FREE(bufu8b);
        FREE(buf1r);
        FREE(buf1g);
        FREE(buf1b);
        FREE(buf2r);
        FREE(buf2g);
        FREE(buf2b);
    }

    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return pixd;
}


/*!
 *  octreeFindColorCell()
 *
 *      Input:  octindex 
 *              cqcaa
 *              &index   (<return> index of CTE; returned to set pixel value)
 *              &rval    (<return> of CTE)
 *              &gval    (<return> of CTE)
 *              &bval    (<return> of CTE)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      (1) As this is in inner loop, we don't check input pointers!
 *      (2) This traverses from the root (well, actually from level 2,
 *          because the level 2 cubes are the largest CTE cubes),
 *          and finds the index number of the cell and the color values,
 *          which can be used either directly or in a (Floyd-Steinberg)
 *          error-diffusion dithering algorithm.
 */
l_int32
octreeFindColorCell(l_int32    octindex,
                    CQCELL  ***cqcaa,
                    l_int32   *pindex,
                    l_int32   *prval,
                    l_int32   *pgval,
                    l_int32   *pbval)
{
l_int32  level;
l_int32  baseindex, subindex;
CQCELL  *cqc, *cqcsub;

        /* Use rgb values stored in the cubes; a little faster */
    for (level = 2; level < CQ_NLEVELS; level++) {
        getOctcubeIndices(octindex, level, &baseindex, &subindex);
        cqc = cqcaa[level][baseindex];
        cqcsub = cqcaa[level + 1][subindex];
        if (cqcsub->bleaf == 0) {  /* use cell at level above */
            *pindex = cqc->index;
            *prval = cqc->rc;
            *pgval = cqc->gc;
            *pbval = cqc->bc;
            break;
        }
        else if (level == CQ_NLEVELS - 1) {  /* reached the bottom */
           *pindex = cqcsub->index;
           *prval = cqcsub->rc;
           *pgval = cqcsub->gc;
           *pbval = cqcsub->bc;
            break;
        }
    }

#if 0
        /* Generate rgb values for each cube on the fly; slower */
    for (level = 2; level < CQ_NLEVELS; level++) {
        l_int32  rv, gv, bv;
        getOctcubeIndices(octindex, level, &baseindex, &subindex);
        cqc = cqcaa[level][baseindex];
        cqcsub = cqcaa[level + 1][subindex];
        if (cqcsub->bleaf == 0) {  /* use cell at level above */
            getRGBFromOctcube(baseindex, level, &rv, &gv, &bv);
            *pindex = cqc->index;
            *prval = rv;
            *pgval = gv;
            *pbval = bv;
            break;
        }
        else if (level == CQ_NLEVELS - 1) {  /* reached the bottom */
            getRGBFromOctcube(subindex, level + 1, &rv, &gv, &bv);
           *pindex = cqcsub->index;
            *prval = rv;
            *pgval = gv;
            *pbval = bv;
            break;
        }
    }
#endif

    return 0;
}



/*------------------------------------------------------------------*
 *                      Helper cqcell functions                     *
 *------------------------------------------------------------------*/
/*!
 *  cqcellTreeCreate()
 *
 *      Input:  none
 *      Return: cqcell array tree
 */
CQCELL ***
cqcellTreeCreate(void)
{
l_int32    level, ncells, i;
CQCELL  ***cqcaa;  
CQCELL   **cqca;   /* one array for each octree level */

    PROCNAME("cqcellTreeCreate");

        /* Make array of accumulation cell arrays from levels 1 to 5 */
    if ((cqcaa = (CQCELL ***)CALLOC(CQ_NLEVELS + 1, sizeof(CQCELL **))) == NULL)
        return (CQCELL ***)ERROR_PTR("cqcaa not made", procName, NULL);
    for (level = 0; level <= CQ_NLEVELS; level++) {
        ncells = 1 << (3 * level);
        if ((cqca = (CQCELL **)CALLOC(ncells, sizeof(CQCELL *))) == NULL)
            return (CQCELL ***)ERROR_PTR("cqca not made", procName, NULL);
        cqcaa[level] = cqca;
        for (i = 0; i < ncells; i++) {
            if ((cqca[i] = (CQCELL *)CALLOC(1, sizeof(CQCELL))) == NULL)
                return (CQCELL ***)ERROR_PTR("cqc not made", procName, NULL);
        }
    }

    return cqcaa;
}


/*!
 *  cqcellTreeDestroy()
 *
 *      Input:  &cqcaa (<to be nulled>
 *      Return: void
 */
void
cqcellTreeDestroy(CQCELL  ****pcqcaa)
{
l_int32    level, ncells, i;
CQCELL  ***cqcaa;
CQCELL   **cqca;

    PROCNAME("cqcellTreeDestroy");

    if (pcqcaa == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }

    if ((cqcaa = *pcqcaa) == NULL)
        return;

    for (level = 0; level <= CQ_NLEVELS; level++) {
        cqca = cqcaa[level];
        ncells = 1 << (3 * level);
        for (i = 0; i < ncells; i++)
            FREE(cqca[i]);
        FREE(cqca);
    }
    FREE(cqcaa);
    *pcqcaa = NULL;

    return;
}



/*------------------------------------------------------------------*
 *                       Helper index functions                     *
 *------------------------------------------------------------------*/
/*!
 *  makeRGBToIndexTables()
 *
 *      Input:  &rtab, &gtab, &btab  (<return> tables)
 *              cqlevels (can be 1, 2, 3, 4, 5 or 6)
 *      Return: 0 if OK; 1 on error
 *
 *  Set up tables.  e.g., for cqlevels = 5, we need an integer 0 < i < 2^15:
 *      rtab = (0  i7  0   0  i6  0   0  i5  0   0   i4  0   0   i3  0   0)
 *      gtab = (0  0   i7  0   0  i6  0   0  i5  0   0   i4  0   0   i3  0)
 *      btab = (0  0   0   i7  0  0   i6  0  0   i5  0   0   i4  0   0   i3)
 *
 *  The tables are then used to map from rbg --> index as follows:
 *      index = (0  r7  g7  b7  r6  g6  b6  r5  g5  b5  r4  g4  b4  r3  g3  b3)
 *
 *    e.g., for cqlevels = 4, we map to
 *      index = (0  0   0   0   r7  g7  b7  r6  g6  b6  r5  g5  b5  r4  g4  b4)
 *
 *  This may look a bit strange.  The notation 'r7' means the MSBit of
 *  the r value (which has 8 bits, going down from r7 to r0).  
 *  Keep in mind that r7 is actually the r component bit for level 1 of
 *  the octtree.  Level 1 is composed of 8 octcubes, represented by
 *  the bits (r7 g7 b7), which divide the entire color space into
 *  8 cubes.  At level 2, each of these 8 octcubes is further divided into
 *  8 cubes, each labeled by the second most significant bits (r6 g6 b6)
 *  of the rgb color.
 */
l_int32
makeRGBToIndexTables(l_uint32  **prtab,
                     l_uint32  **pgtab,
                     l_uint32  **pbtab,
                     l_int32     cqlevels)
{
l_int32    i;
l_uint32  *rtab, *gtab, *btab;

    PROCNAME("makeRGBToIndexTables");

    if (cqlevels < 1 || cqlevels > 6)
        return ERROR_INT("cqlevels must be in {1,...6}", procName, 1);

    if (!prtab || !pgtab || !pbtab)
        return ERROR_INT("&*tab not defined", procName, 1);
    if ((rtab = (l_uint32 *)CALLOC(256, sizeof(l_uint32))) == NULL)
        return ERROR_INT("rtab not made", procName, 1);
    if ((gtab = (l_uint32 *)CALLOC(256, sizeof(l_uint32))) == NULL)
        return ERROR_INT("gtab not made", procName, 1);
    if ((btab = (l_uint32 *)CALLOC(256, sizeof(l_uint32))) == NULL)
        return ERROR_INT("btab not made", procName, 1);
    *prtab = rtab;
    *pgtab = gtab;
    *pbtab = btab;
        
    switch (cqlevels)
    {
    case 1:
        for (i = 0; i < 256; i++) {
            rtab[i] = (i >> 5) & 0x0004;
            gtab[i] = (i >> 6) & 0x0002;
            btab[i] = (i >> 7);
        }
        break;
    case 2:
        for (i = 0; i < 256; i++) {
            rtab[i] = ((i >> 2) & 0x0020) | ((i >> 4) & 0x0004);
            gtab[i] = ((i >> 3) & 0x0010) | ((i >> 5) & 0x0002);
            btab[i] = ((i >> 4) & 0x0008) | ((i >> 6) & 0x0001);
        }
        break;
    case 3:
        for (i = 0; i < 256; i++) {
            rtab[i] = ((i << 1) & 0x0100) | ((i >> 1) & 0x0020) |
                      ((i >> 3) & 0x0004);
            gtab[i] = (i & 0x0080) | ((i >> 2) & 0x0010) |
                      ((i >> 4) & 0x0002);
            btab[i] = ((i >> 1) & 0x0040) | ((i >> 3) & 0x0008) |
                      ((i >> 5) & 0x0001);
        }
        break;
    case 4:
        for (i = 0; i < 256; i++) {
            rtab[i] = ((i << 4) & 0x0800) | ((i << 2) & 0x0100) |
                      (i & 0x0020) | ((i >> 2) & 0x0004);
            gtab[i] = ((i << 3) & 0x0400) | ((i << 1) & 0x0080) |
                      ((i >> 1) & 0x0010) | ((i >> 3) & 0x0002);
            btab[i] = ((i << 2) & 0x0200) | (i & 0x0040) |
                      ((i >> 2) & 0x0008) | ((i >> 4) & 0x0001);
        }
        break;
    case 5:
        for (i = 0; i < 256; i++) {
            rtab[i] = ((i << 7) & 0x4000) | ((i << 5) & 0x0800) |
                      ((i << 3) & 0x0100) | ((i << 1) & 0x0020) |
                      ((i >> 1) & 0x0004);
            gtab[i] = ((i << 6) & 0x2000) | ((i << 4) & 0x0400) |
                      ((i << 2) & 0x0080) | (i & 0x0010) |
                      ((i >> 2) & 0x0002);
            btab[i] = ((i << 5) & 0x1000) | ((i << 3) & 0x0200) |
                      ((i << 1) & 0x0040) | ((i >> 1) & 0x0008) |
                      ((i >> 3) & 0x0001);
        }
        break;
    case 6:
        for (i = 0; i < 256; i++) {
            rtab[i] = ((i << 10) & 0x20000) | ((i << 8) & 0x4000) |
                      ((i << 6) & 0x0800) | ((i << 4) & 0x0100) |
                      ((i << 2) & 0x0020) | (i & 0x0004);
            gtab[i] = ((i << 9) & 0x10000) | ((i << 7) & 0x2000) |
                      ((i << 5) & 0x0400) | ((i << 3) & 0x0080) |
                      ((i << 1) & 0x0010) | ((i >> 1) & 0x0002);
            btab[i] = ((i << 8) & 0x8000) | ((i << 6) & 0x1000) |
                      ((i << 4) & 0x0200) | ((i << 2) & 0x0040) |
                      (i & 0x0008) | ((i >> 2) & 0x0001);
        }
        break;
    default:
        ERROR_INT("cqlevels not in [1...6]", procName, 1);
        break;
    }

    return 0;
}


/*!
 *  getRGBFromOctcube()
 *
 *      Input:  octcube index
 *              level (at which index is expressed)
 *              &rval  (<return> r val of this cube)
 *              &gval  (<return> g val of this cube)
 *              &bval  (<return> b val of this cube)
 *      Return: void
 *
 *  Notes:
 *      (1) We can consider all octcube indices to represent a
 *          specific point in color space: namely, the location
 *          of the 'upper-left' corner of the cube, where indices
 *          increase down and to the right.  The upper left corner
 *          of the color space is then 00000.... 
 *      (2) The 'rgbindex' is a 24-bit representation of the location,
 *          in octcube notation, at the center of the octcube.
 *          To get to the center of an octcube, you choose the 111
 *          octcube at the next lower level.
 *      (3) For example, if the octcube index = 110101 (binary),
 *          which is a level 2 expression, then the rgbindex
 *          is the 24-bit representation of 110101111 (at level 3);
 *          namely, 000110101111000000000000.  The number is padded
 *          with 3 leading 0s (because the representation uses
 *          only 21 bits) and 12 trailing 0s (the default for
 *          levels 4-7, which are contained within each of the level3
 *          octcubes.  Then the rgb values for the center of the
 *          octcube are: rval = 11100000, gval = 10100000, bval = 01100000
 */
void
getRGBFromOctcube(l_int32   cubeindex,
                  l_int32   level,
                  l_int32  *prval,
                  l_int32  *pgval,
                  l_int32  *pbval)
{
l_int32  rgbindex;

        /* Bring to format in 21 bits: (r7 g7 b7 r6 g6 b6 ...) */
        /* This is valid for levels from 0 to 6 */
    rgbindex = cubeindex << (3 * (7 - level));  /* upper corner of cube */
    rgbindex |= (0x7 << (3 * (6 - level)));   /* index to center of cube */

        /* Extract separate pieces */
    *prval = ((rgbindex >> 13) & 0x80) |
             ((rgbindex >> 11) & 0x40) |
             ((rgbindex >> 9) & 0x20) |
             ((rgbindex >> 7) & 0x10) |
             ((rgbindex >> 5) & 0x08) |
             ((rgbindex >> 3) & 0x04) |
             ((rgbindex >> 1) & 0x02);
    *pgval = ((rgbindex >> 12) & 0x80) |
             ((rgbindex >> 10) & 0x40) |
             ((rgbindex >> 8) & 0x20) |
             ((rgbindex >> 6) & 0x10) |
             ((rgbindex >> 4) & 0x08) |
             ((rgbindex >> 2) & 0x04) |
             (rgbindex & 0x02);
    *pbval = ((rgbindex >> 11) & 0x80) |
             ((rgbindex >> 9) & 0x40) |
             ((rgbindex >> 7) & 0x20) |
             ((rgbindex >> 5) & 0x10) |
             ((rgbindex >> 3) & 0x08) |
             ((rgbindex >> 1) & 0x04) |
             ((rgbindex << 1) & 0x02);

    return;
}

    
/*!
 *  getOctcubeIndices()
 *
 *     Input:  rgbindex
 *             octree level (0, 1, 2, 3, 4, 5)
 *             &octcube base index (<return> index at the octree level)
 *             &octcube sub index (<return> index at the next lower level)
 *     Return: 0 if OK, 1 on error
 *
 *  for CQ_NLEVELS = 6, the full RGB index is in the form:
 *     index = (0[13] 0 r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4 r3 g3 b3 r2 g2 b2)
 *  for CQ_NLEVELS = 5, the full RGB index is in the form:
 *     index = (0[16] 0 r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4 r3 g3 b3)
 *  for CQ_NLEVELS = 4, the full RGB index is in the form:
 *     index = (0[19] 0 r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4)
 *
 *  The base index is the index of the octcube at the level given,
 *  whereas the sub index is the index at the next level down.
 *
 *  For level 0: base index = 0
 *               sub index is the 3 bit number (r7 g7 b7)
 *  For level 1: base index = (r7 g7 b7)
 *               sub index = (r7 g7 b7 r6 g6 b6)
 *  For level 2: base index = (r7 g7 b7 r6 g6 b6)
 *               sub index = (r7 g7 b7 r6 g6 b6 r5 g5 b5)
 *  For level 3: base index = (r7 g7 b7 r6 g6 b6 r5 g5 b5)
 *               sub index = (r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4)
 *  For level 4: base index = (r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4)
 *               sub index = (r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4 r3 g3 b3)
 *  For level 5: base index = (r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4 r3 g3 b3)
 *               sub index = (r7 g7 b7 r6 g6 b6 r5 g5 b5 r4 g4 b4 r3 g3 b3
 *                            r2 g2 b2)
 */
l_int32
getOctcubeIndices(l_int32   rgbindex,
                  l_int32   level,
                  l_int32  *pbindex,
                  l_int32  *psindex)
{
    PROCNAME("getOctcubeIndex");

    if (level < 0 || level > CQ_NLEVELS - 1)
        return ERROR_INT("level must be in e.g., [0 ... 5]", procName, 1);
    if (!pbindex)
        return ERROR_INT("&bindex not defined", procName, 1);
    if (!psindex)
        return ERROR_INT("&sindex not defined", procName, 1);

    *pbindex = rgbindex >> (3 * (CQ_NLEVELS - level));
    *psindex = rgbindex >> (3 * (CQ_NLEVELS - 1 - level));
    return 0;
}


/*!
 *  getOctcubeIndexFromRGB()
 *
 *      Input:  rval, gval, bval
 *              rtab, gtab, btab  (generated with makeRGBToIndexTables())
 *              &index (<return>)
 *      Return: void
 *
 *  Note: no error checking!
 */
void
getOctcubeIndexFromRGB(l_int32    rval,
                       l_int32    gval,
                       l_int32    bval,
                       l_uint32  *rtab,
                       l_uint32  *gtab,
                       l_uint32  *btab,
                       l_uint32  *pindex)
{
    *pindex = rtab[rval] | gtab[gval] | btab[bval];
    return;
}


/*!
 *  octcubeGetCount()
 *
 *      Input:  level (valid values are in [1,...6]; there are 2^level
 *                     cubes along each side of the rgb cube)
 *              &size (<return> 2^(3 * level) cubes in the entire rgb cube)
 *      Return:  0 if OK, 1 on error.  Caller must check!
 *
 *         level:   1        2        3        4        5        6
 *         size:    8       64       512     4098     32784   262272
 */
l_int32
octcubeGetCount(l_int32   level,
                l_int32  *psize)
{
    PROCNAME("octcubeGetCount");

    if (!psize)
        return ERROR_INT("&size not defined", procName, 1);
    if (level < 1 || level > 6)
        return ERROR_INT("invalid level", procName, 1);

    *psize = 1 << (3 * level);
    return 0;
}


/*---------------------------------------------------------------------------*
 *         Adaptive octree quantization to 4 and 8 bpp with colormap         *
 *---------------------------------------------------------------------------*/
/*!
 *  pixOctreeQuant()
 *
 *      Input:  pixs (32 bpp rgb)
 *              maxcolors (8 to 256; the actual number of colors used
 *                         may be less than this)
 *              subsample (factor for computing color distribution;
 *                         use 0 for default)
 *      Return: pixd (4 or 8 bpp, colormapped), or null on error
 *
 *  pixOctreeColorQuant() is very flexible in terms of the relative
 *  depth of different cubes of the octree.   By contrast, this function,
 *  pixOctreeQuant() is also adaptive, but it supports octcube
 *  leaves at only two depths: a smaller depth that guarantees
 *  full coverage of the color space and octcubes at one level
 *  deeper for more accurate colors.  Its main virutes are simplicity
 *  and speed, which are both derived from the natural indexing of
 *  the octcubes from the RGB values.
 *
 *  Before describing pixOctreeQuant(), consider an even simpler approach
 *  for 4 bpp with either 8 or 16 colors.  With 8 colors, you simply go to
 *  level 1 octcubes and use the average color found in each cube.  For 16
 *  colors, you find which of the three colors has the largest variance at
 *  the second level, and use two indices for that color.  The result
 *  is quite poor, because (1) some of the cubes are nearly empty and
 *  (2) you don't get much color differentiation for the extra 8 colors.
 *  Trust me, this method may be simple, but it isn't worth anything.
 *
 *  In pixOctreeQuant(), we generate colormapped images at
 *  either 4 bpp or 8 bpp.  For 4 bpp, we have a minimum of 8 colors
 *  for the level 1 octcubes, plus up to 8 additional colors that
 *  are determined from the level 2 popularity.  If the number of colors
 *  is between 8 and 16, the output is a 4 bpp image.  If the number of
 *  colors is greater than 16, the output is a 8 bpp image.
 *
 *  We use a priority queue, implemented with a heap, to select the
 *  requisite number of most populated octcubes at the deepest level
 *  (level 2 for 64 or fewer colors; level 3 for more than 64 colors).
 *  These are combined with one color for each octcube one level above,
 *  which is used to span the color space of octcubes that were not
 *  included at the deeper level.
 *
 *  If the deepest level is 2, we combine the popular level 2 octcubes
 *  (out of a total of 64) with the 8 level 1 octcubes.  If the deepest
 *  level is 3, we combine the popular level 3 octcubes (out of a
 *  total 512) with the 64 level 2 octcubes that span the color space.
 *  In the latter case, we require a minimum of 64 colors for the level 2
 *  octcubes, plus up to 192 additional colors determined from level 3
 *  popularity.
 *
 *  The parameter 'maxlevel' is the deepest octcube level that is used.
 *  The implementation also uses two LUTs, which are employed in
 *  two successive traversals of the dest image.  The first maps
 *  from the src octindex at 'maxlevel' to the color table index,
 *  which is the value that is stored in the 4 or 8 bpp dest pixel.
 *  The second LUT maps from that colormap value in the dest to a
 *  new colormap value for a minimum sized colormap, stored back in
 *  the dest.  It is used to remove any color map entries that
 *  correspond to color space regions that have no pixels in the
 *  source image.  These regions can be either from the higher level
 *  (e.g., level 1 for 4 bpp), or from octcubes at 'maxlevel' that
 *  are unoccupied.  This remapping results in the minimum number
 *  of colors used according to the constraints induced by the
 *  input 'maxcolors'.  We also compute the average R, G and B color
 *  values in each region of the color space represented by a
 *  colormap entry, and store them in the colormap.
 *
 *  The maximum number of colors is input, which determines the
 *  following properties of the dest image and octcube regions used:
 *
 *     Number of colors      dest image depth      maxlevel
 *     ----------------      ----------------      --------
 *       8 to 16                  4 bpp               2
 *       17 to 64                 8 bpp               2
 *       65 to 256                8 bpp               3
 *
 *  It may turn out that the number of extra colors, beyond the
 *  minimum (8 and 64 for maxlevel 2 and 3, respectively), is larger
 *  than the actual number of occupied cubes at these levels
 *  In that case, all the pixels are contained in this
 *  subset of cubes at maxlevel, and no colormap colors are needed
 *  to represent the remainder pixels one level above.  Thus, for
 *  example, in use one often finds that the pixels in an image
 *  occupy less than 192 octcubes at level 3, so they can be represented
 *  by a colormap for octcubes at level 3 only.
 */     
PIX *
pixOctreeQuant(PIX     *pixs,
               l_int32  maxcolors,
               l_int32  subsample)
{
l_int32    w, h, minside, bpp, wpls, wpld, i, j, actualcolors;
l_int32    rval, gval, bval, nbase, nextra, maxlevel, ncubes, val;
l_int32   *lut1, *lut2;
l_uint32   index;
l_uint32  *lines, *lined, *datas, *datad, *pspixel;
l_uint32  *rtab, *gtab, *btab;
OQCELL    *oqc;
OQCELL   **oqca;
PHEAP     *ph;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixOctreeQuant");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    minside = L_MIN(w, h);
    if (subsample <= 0) {
       subsample = L_MAX(1, minside / 200);
    }

    if (maxcolors >= 8 && maxcolors <= 16) {
        bpp = 4;
        pixd = pixCreate(w, h, bpp);
        maxlevel = 2;
        ncubes = 64;   /* 2^6 */
        nbase = 8;
        nextra = maxcolors - nbase;
    }
    else if (maxcolors < 64) {
        bpp = 8;
        pixd = pixCreate(w, h, bpp);
        maxlevel = 2;
        ncubes = 64;  /* 2^6 */
        nbase = 8;
        nextra = maxcolors - nbase;
    }
    else if (maxcolors >= 64 && maxcolors <= 256) {
        bpp = 8;
        pixd = pixCreate(w, h, bpp);
        maxlevel = 3;
        ncubes = 512;  /* 2^9 */
        nbase = 64;
        nextra = maxcolors - nbase;
    }
    else
        return (PIX *)ERROR_PTR("maxcolors not in {8...256}", procName, NULL);

    pixCopyResolution(pixd, pixs);

        /*----------------------------------------------------------*
         * If we're using the minimum number of colors, it is       *
         * much simpler.  We just use 'nbase' octcubes.             *
         * For this case, we don't eliminate any extra colors.      *
         *----------------------------------------------------------*/
    if (nextra == 0) {
            /* prepare the OctcubeQuantCell array */
        if ((oqca = (OQCELL **)CALLOC(nbase, sizeof(OQCELL *))) == NULL)
            return (PIX *)ERROR_PTR("oqca not made", procName, NULL);
        for (i = 0; i < nbase; i++) {
            oqca[i] = (OQCELL *)CALLOC(1, sizeof(OQCELL));
            oqca[i]->n = 0.0;
        }

        makeRGBToIndexTables(&rtab, &gtab, &btab, maxlevel - 1);

            /* Go through the entire image, gathering statistics and
             * assigning pixels to their quantized value */
        datad = pixGetData(pixd);
        wpld = pixGetWpl(pixd);
        for (i = 0; i < h; i++) {
            lines = datas + i * wpls;
            lined = datad + i * wpld;
            for (j = 0; j < w; j++) {
                pspixel = lines + j;
                rval = GET_DATA_BYTE(pspixel, COLOR_RED);
                gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
                bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
                getOctcubeIndexFromRGB(rval, gval, bval,
                                       rtab, gtab, btab, &index);
/*                fprintf(stderr, "rval = %d, gval = %d, bval = %d, index = %d\n",
                        rval, gval, bval, index); */
                switch (bpp) {
                case 4:
                    SET_DATA_QBIT(lined, j, index);
                    break;
                case 8:
                    SET_DATA_BYTE(lined, j, index);
                    break;
                default:
                    return (PIX *)ERROR_PTR("bpp not 4 or 8!", procName, NULL);
                    break;
                }
                oqca[index]->n += 1.0;
                oqca[index]->rcum += rval;
                oqca[index]->gcum += gval;
                oqca[index]->bcum += bval;
            }
        }

            /* Compute average color values in each octcube, and
             * generate colormap */
        cmap = pixcmapCreate(bpp);
        pixSetColormap(pixd, cmap);
        for (i = 0; i < nbase; i++) {
            oqc = oqca[i];
            if (oqc->n != 0) {
                oqc->rval = (l_int32)(oqc->rcum / oqc->n);
                oqc->gval = (l_int32)(oqc->gcum / oqc->n);
                oqc->bval = (l_int32)(oqc->bcum / oqc->n);
            }
            else 
                getRGBFromOctcube(i, maxlevel - 1, &oqc->rval,
                                  &oqc->gval, &oqc->bval);
            pixcmapAddColor(cmap, oqc->rval, oqc->gval, oqc->bval);
        }
/*        pixcmapWriteStream(stderr, cmap); */

        for (i = 0; i < nbase; i++)
            FREE(oqca[i]);
        FREE(oqca);
        FREE(rtab);
        FREE(gtab);
        FREE(btab);
        return pixd;
    }
            
        /*------------------------------------------------------------*
         * General case: we will use colors in octcubes at maxlevel.  *
         * We also remove any colors that are not populated from      *
         * the colormap.                                              *
         *------------------------------------------------------------*/
        /* Prepare the OctcubeQuantCell array */
    if ((oqca = (OQCELL **)CALLOC(ncubes, sizeof(OQCELL *))) == NULL)
        return (PIX *)ERROR_PTR("oqca not made", procName, NULL);
    for (i = 0; i < ncubes; i++) {
        oqca[i] = (OQCELL *)CALLOC(1, sizeof(OQCELL));
        oqca[i]->n = 0.0;
    }

        /* Make the tables to map color to the octindex,
         * of which there are 'ncubes' at 'maxlevel' */
    makeRGBToIndexTables(&rtab, &gtab, &btab, maxlevel);

        /* Estimate the color distribution; we want to find the
         * most popular nextra colors at 'maxlevel' */
    for (i = 0; i < h; i += subsample) {
        lines = datas + i * wpls;
        for (j = 0; j < w; j += subsample) {
            pspixel = lines + j;
            rval = GET_DATA_BYTE(pspixel, COLOR_RED);
            gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
            getOctcubeIndexFromRGB(rval, gval, bval, rtab, gtab, btab, &index);
            oqca[index]->n += 1.0;
            oqca[index]->octindex = index;
            oqca[index]->rcum += rval;
            oqca[index]->gcum += gval;
            oqca[index]->bcum += bval;
        }
    }

        /* Transfer the OQCELL from the array, and order in a heap */
    ph = pheapCreate(512, L_SORT_DECREASING);
    for (i = 0; i < ncubes; i++)
        pheapAdd(ph, oqca[i]);
    FREE(oqca);  /* don't need this array */

        /* Prepare a new OctcubeQuantCell array, with maxcolors cells  */
    if ((oqca = (OQCELL **)CALLOC(maxcolors, sizeof(OQCELL *))) == NULL)
        return (PIX *)ERROR_PTR("oqca not made", procName, NULL);
    for (i = 0; i < nbase; i++) {  /* make nbase cells */
        oqca[i] = (OQCELL *)CALLOC(1, sizeof(OQCELL));
        oqca[i]->n = 0.0;
    }

        /* Remove the nextra most populated ones, and put them in the array */
    for (i = 0; i < nextra; i++) {
        oqc = (OQCELL *)pheapRemove(ph);
        oqc->n = 0.0;  /* reinit */
        oqc->rcum = 0;
        oqc->gcum = 0;
        oqc->bcum = 0;
        oqca[nbase + i] = oqc;  /* store it in the array */
    }

        /* Destroy the heap and its remaining contents */
    pheapDestroy(&ph, TRUE);

        /* Generate a lookup table from octindex at maxlevel
         * to color table index */
    if ((lut1 = (l_int32 *)CALLOC(ncubes, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("lut1 not made", procName, NULL);
    for (i = 0; i < nextra; i++)
        lut1[oqca[nbase + i]->octindex] = nbase + i;
    for (index = 0; index < ncubes; index++) {
        if (lut1[index] == 0)  /* not one of the extras; need to assign */
            lut1[index] = index >> 3;  /* remove the least significant bits */
/*        fprintf(stderr, "lut1[%d] = %d\n", index, lut1[index]); */
    }
        
        /* Go through the entire image, gathering statistics and
         * assigning pixels to their quantized value */
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            pspixel = lines + j;
            rval = GET_DATA_BYTE(pspixel, COLOR_RED);
            gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
            getOctcubeIndexFromRGB(rval, gval, bval, rtab, gtab, btab, &index);
/*            fprintf(stderr, "rval = %d, gval = %d, bval = %d, index = %d\n",
                    rval, gval, bval, index); */
            val = lut1[index];
            switch (bpp) {
            case 4:
                SET_DATA_QBIT(lined, j, val);
                break;
            case 8:
                SET_DATA_BYTE(lined, j, val);
                break;
            default:
                return (PIX *)ERROR_PTR("bpp not 4 or 8!", procName, NULL);
                break;
            }
            oqca[val]->n += 1.0;
            oqca[val]->rcum += rval;
            oqca[val]->gcum += gval;
            oqca[val]->bcum += bval;
        }
    }
    
        /* Compute averages, set up a colormap, and make a second
         * lut that converts from the color values currently in
         * the image to a minimal set */
    if ((lut2 = (l_int32 *)CALLOC(ncubes, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("lut2 not made", procName, NULL);
    cmap = pixcmapCreate(bpp);
    pixSetColormap(pixd, cmap);
    for (i = 0, index = 0; i < maxcolors; i++) {
        oqc = oqca[i];
        lut2[i] = index;
        if (oqc->n == 0)  /* no occupancy; don't bump up index */
            continue;
        oqc->rval = (l_int32)(oqc->rcum / oqc->n);
        oqc->gval = (l_int32)(oqc->gcum / oqc->n);
        oqc->bval = (l_int32)(oqc->bcum / oqc->n);
        pixcmapAddColor(cmap, oqc->rval, oqc->gval, oqc->bval);
        index++;
    }
/*    pixcmapWriteStream(stderr, cmap); */
    actualcolors = pixcmapGetCount(cmap);
/*    fprintf(stderr, "Number of different colors = %d\n", actualcolors); */

        /* Last time through the image; use the lookup table to
         * remap the pixel value to the minimal colormap */
    if (actualcolors < maxcolors) {
        for (i = 0; i < h; i++) {
            lined = datad + i * wpld;
            for (j = 0; j < w; j++) {
                switch (bpp) {
                case 4:
                    val = GET_DATA_QBIT(lined, j);
                    SET_DATA_QBIT(lined, j, lut2[val]);
                    break;
                case 8:
                    val = GET_DATA_BYTE(lined, j);
                    SET_DATA_BYTE(lined, j, lut2[val]);
                    break;
                }
            }
        }
    }

    for (i = 0; i < maxcolors; i++)
        FREE(oqca[i]);
    FREE(oqca);
    FREE(lut1);
    FREE(lut2);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return pixd;
}


/*---------------------------------------------------------------------------*
 *           Fixed partition octcube quantization and histogram              *
 *---------------------------------------------------------------------------*/
/*!
 *  pixFixedOctcubeQuant()
 *
 *      Input:  pixs (32 bpp rgb)
 *              level (significant bits for each of RGB)
 *      Return: pixd (quantized to octcube) or null on error
 *
 *  Notes:
 *      (1) This first tries to make a colormapped pixd.
 *      (2) If this fails because there are too many colors,
 *          it makes an rgb pixd with colors quantized to centers
 *          of the octcubes at the specified levels.
 *      (3) Often level 3 (512 octcubes) will succeed because not more
 *          than half of them are occupied with 1 or more pixels.
 */     
PIX *
pixFixedOctcubeQuant(PIX     *pixs,
                     l_int32  level)
{
PIX       *pixd;

    PROCNAME("pixFixedOctcubeQuant");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (level < 1 || level > 6)
        return (PIX *)ERROR_PTR("level not in {1,...6}", procName, NULL);

    if ((pixd = pixFixedOctcubeQuantCmap(pixs, level)))
        return pixd;
    else
        return pixFixedOctcubeQuantRGB(pixs, level);
}


/*!
 *  pixFixedOctcubeQuantRGB()
 *
 *      Input:  pixs (32 bpp rgb)
 *              level (significant bits for each of RGB)
 *      Return: pixd (quantized to octcube), or null on error
 *
 *  Notes:
 *      (1) pixel values are taken at the center of each octcube,
 *          not as an average of the pixels in that octcube.
 */     
PIX *
pixFixedOctcubeQuantRGB(PIX     *pixs,
                        l_int32  level)
{
l_int32    w, h, wpls, wpld, i, j;
l_int32    rval, gval, bval;
l_uint32   octindex;
l_uint32  *rtab, *gtab, *btab;
l_uint32  *lines, *lined, *datas, *datad, *pspixel, *pdpixel;
PIX       *pixd;

    PROCNAME("pixFixedOctcubeQuantRGB");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (level < 1 || level > 6)
        return (PIX *)ERROR_PTR("level not in {1,...6}", procName, NULL);

    if (makeRGBToIndexTables(&rtab, &gtab, &btab, level))
        return (PIX *)ERROR_PTR("tables not made", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    pixd = pixCreate(w, h, 32);
    pixCopyResolution(pixd, pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);

    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            pspixel = lines + j;
            pdpixel = lined + j;
            rval = GET_DATA_BYTE(pspixel, COLOR_RED);
            gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            getRGBFromOctcube(octindex, level, &rval, &gval, &bval);
            composeRGBPixel(rval, gval, bval, pdpixel);
        }
    }

    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return pixd;
}


/*!
 *  pixFixedOctcubeQuantCmap()
 *
 *      Input:  pixs (32 bpp rgb)
 *              level (significant bits for each of RGB; valid in [1...6])
 *      Return: pixd (quantized to octcube) or null on error
 *
 *  Notes:
 *      (1) Generates a colormapped image, where the colormap table values
 *          are the averages of all pixels that are found in the octcube.
 *      (2) This fails if there are more than 256 colors (i.e., more
 *          than 256 occupied octcubes).
 *      (3) The depth of the result, which is either 2, 4 or 8 bpp,
 *          is the minimum required to hold the number of colors that
 *          are found.
 */     
PIX *
pixFixedOctcubeQuantCmap(PIX     *pixs,
                         l_int32  level)
{
l_int32    w, h, wpls, wpld, i, j, depth, size, ncolors, index;
l_int32    rval, gval, bval;
l_int32   *carray, *rarray, *garray, *barray;
l_uint32   octindex;
l_uint32  *rtab, *gtab, *btab;
l_uint32  *lines, *lined, *datas, *datad, *pspixel;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixFixedOctcubeQuantCmap");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);

    if (octcubeGetCount(level, &size))  /* array size = 2 ** (3 * level) */
        return (PIX *)ERROR_PTR("size not returned", procName, NULL);
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, level))
        return (PIX *)ERROR_PTR("tables not made", procName, NULL);

    if ((carray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("carray not made", procName, NULL);
    if ((rarray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("rarray not made", procName, NULL);
    if ((garray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("garray not made", procName, NULL);
    if ((barray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("barray not made", procName, NULL);

        /* Find the number of different colors */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = NULL;
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        for (j = 0; j < w; j++) {
            pspixel = lines + j;
            rval = GET_DATA_BYTE(pspixel, COLOR_RED);
            gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            carray[octindex]++;
            rarray[octindex] += rval;
            garray[octindex] += gval;
            barray[octindex] += bval;
        }
    }
    for (i = 0, ncolors = 0; i < size; i++) {
        if (carray[i] > 0)
            ncolors++;
    }
    if (ncolors > 256) {
        L_WARNING_INT("%d colors found; more than 256", procName, ncolors);
        goto array_cleanup;
    }

    if (ncolors <= 4)
        depth = 2;
    else if (ncolors <= 16)
        depth = 4;
    else
        depth = 8;

        /* Average the colors in each bin and add to colormap table;
         * then use carray to hold the colormap index + 1  */
    cmap = pixcmapCreate(depth);
    for (i = 0, index = 0; i < size; i++) {
        if (carray[i] > 0) {
            rarray[i] /= carray[i];
            garray[i] /= carray[i];
            barray[i] /= carray[i];
            pixcmapAddColor(cmap, rarray[i], garray[i], barray[i]);
            carray[i] = index + 1;  /* to avoid storing 0 */
            index++;
        }
    }

    pixd = pixCreate(w, h, depth);
    pixSetColormap(pixd, cmap);
    pixCopyResolution(pixd, pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            pspixel = lines + j;
            rval = GET_DATA_BYTE(pspixel, COLOR_RED);
            gval = GET_DATA_BYTE(pspixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(pspixel, COLOR_BLUE);
            octindex = rtab[rval] | gtab[gval] | btab[bval];
            switch (depth) 
            {
            case 2:
                SET_DATA_DIBIT(lined, j, carray[octindex] - 1);
                break;
            case 4:
                SET_DATA_QBIT(lined, j, carray[octindex] - 1);
                break;
            case 8:
                SET_DATA_BYTE(lined, j, carray[octindex] - 1);
                break;
            default:
                L_WARNING("shouldn't get here", procName);
            }
        }
    }

array_cleanup:
    FREE(carray);
    FREE(rarray);
    FREE(garray);
    FREE(barray);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return pixd;
}


/*!
 *  pixOctcubeQuantMixed()
 *
 *      Input:  pixs (32 bpp rgb)
 *              depth (of output pix)
 *              graylevels (grayscale)
 *              delta (threshold for deciding if a pix is color or grayscale)
 *      Return: pixd (quantized to octcube and gray levels) or null on error
 *
 *  Notes:
 *      (1) Generates a colormapped image, where the colormap table values
 *          have two components: octcube values representing pixels with
 *          color content, and grayscale values for the rest.
 *      (2) The threshold (delta) is the maximum allowable difference of
 *          the max abs value of | r - g |, | r - b | and | g - b |.
 *      (3) The octcube values are the averages of all pixels that are
 *          found in the octcube, and that are far enough from gray to
 *          be considered color.  This can roughly be visualized as all
 *          the points in the rgb color cube that are not within a "cylinder"
 *          of diameter approximately 'delta' along the main diagonal.
 *      (4) We want to guarantee full coverage of the rgb color space; thus,
 *          if the output depth is 4, the octlevel is 1 (2 x 2 x 2 = 8 cubes)
 *          and if the output depth is 8, the octlevel is 2 (4 x 4 x 4
 *          = 64 cubes).
 *      (5) Consequently, we have the following constraint on the number
 *          of allowed gray levels: for 4 bpp, 8; for 8 bpp, 192.
 */     
PIX *
pixOctcubeQuantMixed(PIX     *pixs,
                     l_int32  depth,
                     l_int32  graylevels,
                     l_int32  delta)
{
l_int32    w, h, wpls, wpld, i, j, size, octlevels;
l_int32    rval, gval, bval, del, val, midval;
l_int32   *carray, *rarray, *garray, *barray;
l_int32   *tabval;
l_uint32   octindex, pixel;
l_uint32  *rtab, *gtab, *btab;
l_uint32  *lines, *lined, *datas, *datad;
PIX       *pixd;
PIXCMAP   *cmap;

    PROCNAME("pixOctcubeQuantMixed");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (depth == 4) {
        octlevels = 1;
        size = 8;   /* 2 ** 3 */
        if (graylevels > 8)
            return (PIX *)ERROR_PTR("max 8 gray levels", procName, NULL);
    }
    else if (depth == 8) {
        octlevels = 2;
        size = 64;   /* 2 ** 6 */
        if (graylevels > 192)
            return (PIX *)ERROR_PTR("max 192 gray levels", procName, NULL);
    }
    else
        return (PIX *)ERROR_PTR("output depth not 4 or 8 bpp", procName, NULL);
    
        /* Make octcube index tables */
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, octlevels))
        return (PIX *)ERROR_PTR("tables not made", procName, NULL);

        /* Make octcube arrays for storing points in each cube */
    if ((carray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("carray not made", procName, NULL);
    if ((rarray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("rarray not made", procName, NULL);
    if ((garray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("garray not made", procName, NULL);
    if ((barray = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (PIX *)ERROR_PTR("barray not made", procName, NULL);

        /* Make lookup table, using computed thresholds  */
    if ((tabval = makeGrayQuantIndexTable(graylevels)) == NULL)
        return (PIX *)ERROR_PTR("tabval not made", procName, NULL);

        /* Make colormapped output pixd */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if ((pixd = pixCreate(w, h, depth)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    cmap = pixcmapCreate(depth);
    for (j = 0; j < size; j++)  /* reserve octcube colors */
        pixcmapAddColor(cmap, 1, 1, 1);  /* a color that won't be used */
    for (j = 0; j < graylevels; j++) {  /* set grayscale colors */
        val = (255 * j) / (graylevels - 1);
        pixcmapAddColor(cmap, val, val, val);
    }
    pixSetColormap(pixd, cmap);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);

        /* Go through src image: assign dest pixels to colormap values
         * and compute average colors in each occupied octcube */
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            pixel = *(lines + j);
            rval = pixel >> 24;
            gval = (pixel >> 16) & 0xff;
            bval = (pixel >> 8) & 0xff;
            if (rval > gval) {
                if (gval > bval) {   /* r > g > b */
                    del = rval - bval;
                    midval = gval;
                }
                else {
                    if (rval > bval) {  /* r > b > g */
                        del = rval - gval;
                        midval = bval;
                    }
                    else {  /* b > r > g */
                        del = bval - gval;
                        midval = rval;
                    }
                }
            }
            else  {  /* gval >= rval */
                if (rval > bval) {  /* g > r > b */
                    del = gval - bval;
                    midval = rval;
                }
                else {
                    if (gval > bval) {  /* g > b > r */
                        del = gval - rval;
                        midval = bval;
                    }
                    else {  /* b > g > r */
                        del = bval - rval;
                        midval = gval;
                    }
                }
            }
            if (del > delta) {  /* assign to color */
                octindex = rtab[rval] | gtab[gval] | btab[bval];
                carray[octindex]++;
                rarray[octindex] += rval;
                garray[octindex] += gval;
                barray[octindex] += bval;
                if (depth == 4)
                    SET_DATA_QBIT(lined, j, octindex);
                else  /* depth == 8 */
                    SET_DATA_BYTE(lined, j, octindex);
            }
            else {  /* assign to grayscale */
                val = size + tabval[midval];
                if (depth == 4)
                    SET_DATA_QBIT(lined, j, val);
                else  /* depth == 8 */
                    SET_DATA_BYTE(lined, j, val);
            }
        }
    }

        /* Average the colors in each bin and reset the colormap */
    for (i = 0; i < size; i++) {
        if (carray[i] > 0) {
            rarray[i] /= carray[i];
            garray[i] /= carray[i];
            barray[i] /= carray[i];
            pixcmapResetColor(cmap, i, rarray[i], garray[i], barray[i]);
        }
    }

    FREE(carray);
    FREE(rarray);
    FREE(garray);
    FREE(barray);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    FREE(tabval);

    return pixd;
}


/*!
 *  pixOctcubeHistogram()
 *
 *      Input:  pixs (32 bpp rgb)
 *              level (significant bits for each of RGB; valid in [1...6])
 *      Return: numa (histogram of color pixels, or null on error)
 */     
NUMA *
pixOctcubeHistogram(PIX     *pixs,
                    l_int32  level)
{
l_int32     size, i, j, w, h, wpl;
l_uint32    rval, gval, bval, octindex;
l_uint32   *rtab, *gtab, *btab;
l_uint32   *ppixel, *data, *line;
l_float32  *array;
NUMA       *na;

    PROCNAME("pixOctcubeHistogram");

    if (!pixs)
        return (NUMA *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (NUMA *)ERROR_PTR("pixs not 32 bpp", procName, NULL);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    wpl = pixGetWpl(pixs);
    data = pixGetData(pixs);

    if (octcubeGetCount(level, &size))  /* array size = 2 ** (3 * level) */
        return (NUMA *)ERROR_PTR("size not returned", procName, NULL);
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, level))
        return (NUMA *)ERROR_PTR("tables not made", procName, NULL);

    if ((na = numaCreate(size)) == NULL)
        return (NUMA *)ERROR_PTR("na not made", procName, NULL);
    na->n = size;  /* fake storage of n zeroes */
    array = na->array;  /* don't do this at home */

    for (i = 0; i < h; i++) {
        line = data + i * wpl;
        for (j = 0; j < w; j++) {
            ppixel = line + j;
            rval = GET_DATA_BYTE(ppixel, COLOR_RED);
            gval = GET_DATA_BYTE(ppixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(ppixel, COLOR_BLUE);
            octindex = rtab[rval] | gtab[gval] | btab[bval];
#if DEBUG_OCTINDEX
            if ((level == 1 && octindex > 7) ||
                (level == 2 && octindex > 63) ||
                (level == 3 && octindex > 511) ||
                (level == 4 && octindex > 4097) ||
                (level == 5 && octindex > 32783) ||
                (level == 6 && octindex > 262271)) {
                fprintf(stderr, "level = %d, octindex = %d, index error!\n",
                        level, octindex);
                continue;
            }
#endif  /* DEBUG_OCTINDEX */
              array[octindex] += 1.0;
        }
    }

    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return na;
}


/*------------------------------------------------------------------*
 *          Color quantize RGB image using existing colormap        *
 *------------------------------------------------------------------*/
/*!
 *  pixOctcubeQuantFromCmap()
 *
 *      Input:  pixs  (32 bpp rgb)
 *              cmap  (to quantize to; of dest pix)
 *              level (of octcube used for finding nearest color in cmap)
 *              metric (L_MANHATTAN_DISTANCE, L_EUCLIDEAN_DISTANCE)
 *      Return: pixd  (2, 4 or 8 bpp, colormapped), or null on error
 *
 *  Notes:
 *      (1) In typical use, we are doing an operation, such as 
 *          interpolative scaling, on a colormapped pix, where it is
 *          necessary to remove the colormap before the operation.
 *          We then want to re-quantize the RGB result using the same
 *          colormap.
 *      (2) The level is used to divide the color space into octcubes.
 *          Each input pixel is, in effect, placed at the center of an
 *          octcube at the given level, and it is mapped into the
 *          exact color (given in the colormap) that is the closest
 *          to that location.  We need to know that distance, for each color
 *          in the colormap.  The higher the level of the octtree, the smaller
 *          the octcubes in the color space, and hence the more accurately
 *          we can determine the closest color in the colormap; however,
 *          the size of the LUT, which is the total number of octcubes,
 *          increases by a factor of 8 for each increase of 1 level.
 *          The time required to acquire a level 4 mapping table, which has
 *          about 4K entries, is less than 1 msec, so that is the
 *          recommended minimum size to be used.  At that size, the 
 *          octcubes have their centers 16 units apart in each (r,g,b)
 *          direction.  If two colors are in the same octcube, the one
 *          closest to the center will always be chosen.  The maximum
 *          error for any component occurs when the correct color is
 *          at a cube corner and there is an incorrect color just inside
 *          the cube next to the opposite corner, giving an error of
 *          14 units (out of 256) for each component.   Using a level 5
 *          mapping table reduces the maximum error to 6 units.
 *      (3) Typically you should use the Euclidean metric, because the
 *          resulting voronoi cells (which are generated using the actual
 *          colormap values as seeds) are convex for Euclidean distance
 *          but not for Manhattan distance.  In terms of the octcubes,
 *          convexity of the voronoi cells means that if the 8 corners
 *          of any cube (of which the octcubes are special cases)
 *          are all within a cell, then every point in the cube will
 *          lie within the cell.
 *      (4) The depth of the output pixd is the minimum (2, 4 or 8 bpp)
 *          necessary to hold the indices in the colormap.
 *      (5) We build a mapping table from octcube to colormap index so
 *          that this function can run in a time (otherwise) independent
 *          of the number of colors in the colormap.  This avoids a
 *          brute-force search for the closest colormap color to each
 *          pixel in the image.
 *      (6) This is similar to the function pixAssignToNearestColor()
 *          used for color segmentation.
 *      (7) Except for very small images or when using level > 4,
 *          it takes very little time to generate the tables,
 *          compared to the generation of the colormapped dest pix,
 *          so one would not typically use the low-level version.
 */
PIX *
pixOctcubeQuantFromCmap(PIX      *pixs,
                        PIXCMAP  *cmap,
                        l_int32   level,
                        l_int32   metric)
{
l_int32   *cmaptab;
l_uint32  *rtab, *gtab, *btab;
PIX       *pixd;

    PROCNAME("pixOctcubeQuantFromCmap");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (!cmap)
        return (PIX *)ERROR_PTR("cmap not defined", procName, NULL);
    if (level < 1 || level > 6)
        return (PIX *)ERROR_PTR("level not in {1...6}", procName, NULL);
    if (metric != L_MANHATTAN_DISTANCE && metric != L_EUCLIDEAN_DISTANCE)
        return (PIX *)ERROR_PTR("invalid metric", procName, NULL);

        /* Set up the tables to map rgb to the nearest colormap index */
    if (makeRGBToIndexTables(&rtab, &gtab, &btab, level))
        return (PIX *)ERROR_PTR("index tables not made", procName, NULL);
    if ((cmaptab = pixcmapToOctcubeLUT(cmap, level, metric)) == NULL)
        return (PIX *)ERROR_PTR("cmaptab not made", procName, NULL);

    pixd = pixOctcubeQuantFromCmapLUT(pixs, cmap, cmaptab, rtab, gtab, btab);

    FREE(cmaptab);
    FREE(rtab);
    FREE(gtab);
    FREE(btab);
    return pixd;
}


/*!
 *  pixOctcubeQuantFromCmapLUT()
 *
 *      Input:  pixs  (32 bpp rgb)
 *              cmap (for the dest)
 *              cmaptab  (from octindex to colormap index)
 *              rtab, gtab, btab (from RGB to octindex)
 *      Return: pixd  (2, 4 or 8 bpp, colormapped), or null on error
 *
 *  Notes:
 *      (1) See the notes in the higher-level function
 *          pixOctcubeQuantFromCmap().  The octcube level for
 *          the generated octree is specified there, along with
 *          the distance metric for determining the closest
 *          color in the colormap to each octcube.
 *      (2) If the colormap, level and metric information have already
 *          been used to construct the set of mapping tables,
 *          this low-level function can be used directly (i.e.,
 *          independently of pixOctcubeQuantFromCmap()) to build
 *          a colormapped pix that uses the specified colormap.
 */
PIX *
pixOctcubeQuantFromCmapLUT(PIX       *pixs,
                           PIXCMAP   *cmap,
                           l_int32   *cmaptab,
                           l_uint32  *rtab,
                           l_uint32  *gtab,
                           l_uint32  *btab)
{
l_int32    i, j, w, h, ncolors, depth, wpls, wpld;
l_int32    rval, gval, bval, index;
l_uint32   octindex;
l_uint32  *lines, *lined, *datas, *datad;
l_uint32  *ppixel;
PIX       *pixd;
PIXCMAP   *cmapc;

    PROCNAME("pixOctcubeQuantFromCmapLUT");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs not 32 bpp", procName, NULL);
    if (!cmap)
        return (PIX *)ERROR_PTR("cmap not defined", procName, NULL);
    if (!rtab || !gtab || !btab || !cmaptab)
        return (PIX *)ERROR_PTR("tables not all defined", procName, NULL);

        /* Init dest pix (with minimum bpp depending on cmap) */
    ncolors = pixcmapGetCount(cmap);
    if (ncolors <= 4)
        depth = 2;
    else if (ncolors <= 16)
        depth = 4;
    else
        depth = 8;
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    if ((pixd = pixCreate(w, h, depth)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    cmapc = pixcmapCopy(cmap);
    pixSetColormap(pixd, cmapc);

        /* Insert the colormap index of the color nearest to the input pixel */
    datas = pixGetData(pixs);
    datad = pixGetData(pixd);
    wpls = pixGetWpl(pixs);
    wpld = pixGetWpl(pixd);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            ppixel = lines + j;
            rval = GET_DATA_BYTE(ppixel, COLOR_RED);
            gval = GET_DATA_BYTE(ppixel, COLOR_GREEN);
            bval = GET_DATA_BYTE(ppixel, COLOR_BLUE);
                /* Map from rgb to octcube index */
            getOctcubeIndexFromRGB(rval, gval, bval, rtab, gtab, btab,
                                   &octindex);
                /* Map from octcube index to nearest colormap index */
            index = cmaptab[octindex];
            if (depth == 2)
                SET_DATA_DIBIT(lined, j, index);
            else if (depth == 4)
                SET_DATA_QBIT(lined, j, index);
            else  /* depth == 8 */
                SET_DATA_BYTE(lined, j, index);
        }
    }

    return pixd;
}



/*------------------------------------------------------------------*
 *              Get filled octcube table from colormap              *
 *------------------------------------------------------------------*/
/*!
 *  pixcmapToOctcubeLUT()
 *
 *      Input:  cmap
 *              level (significant bits for each of RGB; valid in [1...6])
 *              metric (L_MANHATTAN_DISTANCE, L_EUCLIDEAN_DISTANCE)
 *      Return: tab[2**(3 * level)]
 *
 *  Notes:
 *      (1) This function is used to quickly find the colormap color
 *          that is closest to any rgb color.  It is used to assign
 *          rgb colors to an existing colormap.  It can be very expensive
 *          to search through the entire colormap for the closest color
 *          to each pixel.  Instead, we first set up this table, which is
 *          populated by the colormap index nearest to each octcube
 *          color.  Then we go through the image; for each pixel,
 *          do two table lookups: first to generate the octcube index
 *          from rgb and second to use this table to read out the
 *          colormap index.
 *      (2) Here are the actual function calls:
 *            - first make the tables that map from rgb --> octcube index
 *                     makeRGBToIndexTables()
 *            - then for each pixel:
 *                * use the tables to get the octcube index
 *                     getOctcubeIndexFromRGB()
 *                * use this table to get the nearest color in the colormap
 *                     cmap_index = tab[index]
 *      (3) Distance can be either manhattan or euclidean.
 *      (4) When this function is used within color segmentation,
 *          there are typically a small number of colors and the
 *          number of levels can be small (e.g., 3).
 */
l_int32 *
pixcmapToOctcubeLUT(PIXCMAP  *cmap,
                    l_int32   level,
                    l_int32   metric)
{
l_int32   i, k, size, ncolors, mindist, dist, mincolor;
l_int32   rval, gval, bval;  /* color at center of the octcube */
l_int32  *rmap, *gmap, *bmap;
l_int32  *tab;

    PROCNAME("pixcmapToOctcubeLUT");

    if (!cmap)
        return (l_int32 *)ERROR_PTR("cmap not defined", procName, NULL);
    if (level < 1 || level > 6)
        return (l_int32 *)ERROR_PTR("level not in {1...6}", procName, NULL);
    if (metric != L_MANHATTAN_DISTANCE && metric != L_EUCLIDEAN_DISTANCE)
        return (l_int32 *)ERROR_PTR("invalid metric", procName, NULL);

    if (octcubeGetCount(level, &size))  /* array size = 2 ** (3 * level) */
        return (l_int32 *)ERROR_PTR("size not returned", procName, NULL);
    if ((tab = (l_int32 *)CALLOC(size, sizeof(l_int32))) == NULL)
        return (l_int32 *)ERROR_PTR("tab not allocated", procName, NULL);

    ncolors = pixcmapGetCount(cmap);
    pixcmapToArrays(cmap, &rmap, &gmap, &bmap);

    for (i = 0; i < size; i++) {
        getRGBFromOctcube(i, level, &rval, &gval, &bval);
        mindist = 1000000;
        mincolor = 0;  /* irrelevant init */
        for (k = 0; k < ncolors; k++) {
            if (metric == L_MANHATTAN_DISTANCE) {
                dist = L_ABS(rval - rmap[k]) + L_ABS(gval - gmap[k]) +
                       L_ABS(bval - bmap[k]);
            }
            else {  /* L_EUCLIDEAN_DISTANCE */
                dist = (rval - rmap[k]) * (rval - rmap[k]) +
                       (gval - gmap[k]) * (gval - gmap[k]) +
                       (bval - bmap[k]) * (bval - bmap[k]);
            }
            if (dist < mindist) {
                mindist = dist;
                mincolor = k;
            }
        }
        tab[i] = mincolor;
    }

    FREE(rmap);
    FREE(gmap);
    FREE(bmap);
    return tab;
}


/*------------------------------------------------------------------*
 *               Strip out unused elements in colormap              *
 *------------------------------------------------------------------*/
/*!
 *  pixRemoveUnusedColors()
 *
 *      Input:  pixs  (colormapped)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is an in-place operation.
 *      (2) If the image doesn't have a colormap, returns without error.
 *      (3) Unusued colors are removed from the colormap, and the
 *          image pixels are re-numbered.
 */
l_int32
pixRemoveUnusedColors(PIX  *pixs)
{
l_int32     i, j, w, h, d, nc, wpls, val, newval, index, zerofound;
l_int32     rval, gval, bval;
l_uint32   *datas, *lines;
l_int32    *histo, *map1, *map2;
PIXCMAP    *cmap, *cmapd;

    PROCNAME("pixRemoveUnusedColors");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if ((cmap = pixGetColormap(pixs)) == NULL)
        return 0;

    d = pixGetDepth(pixs);
    if (d != 2 && d != 4 && d != 8)
        return ERROR_INT("d not in {2, 4, 8}", procName, 1);

        /* Find which indices are actually used */
    nc = pixcmapGetCount(cmap);
    if ((histo = (l_int32 *)CALLOC(nc, sizeof(l_int32))) == NULL)
        return ERROR_INT("histo not made", procName, 1);
    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    wpls = pixGetWpl(pixs);
    datas = pixGetData(pixs);
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        for (j = 0; j < w; j++) {
            switch (d)
            {
            case 2:
                val = GET_DATA_DIBIT(lines, j);
                break;
            case 4:
                val = GET_DATA_QBIT(lines, j);
                break;
            case 8:
                val = GET_DATA_BYTE(lines, j);
                break;
            default:
                return ERROR_INT("switch ran off end!", procName, 1);
            }
            if (val >= nc) {
                L_WARNING("cmap index out of bounds!", procName);
                continue;
            }
            histo[val]++;
        }
    }

        /* Check if there are any zeroes.  If none, quit. */
    zerofound = FALSE;
    for (i = 0; i < nc; i++) {
        if (histo[i] == 0) {
            zerofound = TRUE;
            break;
        }
    }
    if (!zerofound) {
      FREE(histo);
      return 0;
    }

        /* Generate mapping tables between indices */
    if ((map1 = (l_int32 *)CALLOC(nc, sizeof(l_int32))) == NULL)
        return ERROR_INT("map1 not made", procName, 1);
    if ((map2 = (l_int32 *)CALLOC(nc, sizeof(l_int32))) == NULL)
        return ERROR_INT("map2 not made", procName, 1);
    index = 0;
    for (i = 0; i < nc; i++) {
        if (histo[i] != 0) {
            map1[index] = i;  /* get old index from new */
            map2[i] = index;  /* get new index from old */
            index++;
        }
    }

        /* Generate new colormap and attach to pixs */
    cmapd = pixcmapCreate(d);
    for (i = 0; i < index; i++) {
        pixcmapGetColor(cmap, map1[i], &rval, &gval, &bval);
        pixcmapAddColor(cmapd, rval, gval, bval);
    }
    pixSetColormap(pixs, cmapd);

        /* Map pixel (index) values to new cmap */
    for (i = 0; i < h; i++) {
        lines = datas + i * wpls;
        for (j = 0; j < w; j++) {
            switch (d)
            {
            case 2:
                val = GET_DATA_DIBIT(lines, j);
                newval = map2[val];
                SET_DATA_DIBIT(lines, j, newval);
                break;
            case 4:
                val = GET_DATA_QBIT(lines, j);
                newval = map2[val];
                SET_DATA_QBIT(lines, j, newval);
                break;
            case 8:
                val = GET_DATA_BYTE(lines, j);
                newval = map2[val];
                SET_DATA_BYTE(lines, j, newval);
                break;
            default:
                return ERROR_INT("switch ran off end!", procName, 1);
            }
        }
    }
        
    FREE(histo);
    FREE(map1);
    FREE(map2);
    return 0;
}


