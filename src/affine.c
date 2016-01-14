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
 *  affine.c
 *
 *      Affine image transformation using a sequence of 
 *      shear/scale/translation
 *           PIX      *pixAffineSequential()
 *
 *      Affine (3-pt) image transformation using a sampled
 *      (to nearest integer) transform on each dest point
 *           PIX      *pixAffineSampled()
 *
 *      Affine (3-pt) image transformation using interpolation 
 *      (or area mapping) for anti-aliasing images that are
 *      2, 4, or 8 bpp gray, or colormapped, or 32 bpp RGB
 *           PIX      *pixAffineInterpolated()
 *           PIX      *pixAffineInterpolatedColor()
 *           PIX      *pixAffineInterpolatedGray()
 *           void      affineInterpolatedColorLow()
 *           void      affineInterpolatedGrayLow()
 *
 *      Affine coordinate transformation
 *           l_int32   affineXformCoeffs()
 *           l_int32   affineXformSampled()
 *           l_int32   affineXformInterpolated()
 *
 *      Gauss-jordan linear equation solver
 *           l_int32   gaussjordan()
 *
 *      An affine transform is a transform on an image from one
 *      coordinate space to another.  One can define a coordinate
 *      space by the location of the origin, the orientation of x and
 *      y axes, and the unit scaling along each axis.  An affine
 *      transform is a general linear transformation (or warping)
 *      from the first coordinate space to the second.
 *
 *      In the general case, we define the affine transform using
 *      two sets of three (noncollinear) points in a plane.  One set
 *      corresponds to the input (src) coordinate space; the other to the 
 *      transformed (dest) coordinate space.  Each point in the
 *      src corresponds to one of the points in the dest.  With two
 *      sets of three points, we get a set of 6 equations in 6 unknowns
 *      that specifies the mapping between the coordinate spaces.
 *
 *      For the special case where we perform an arbitrary affine transform
 *      by a sequence of transforms (scaling, translation, shear,
 *      rotation), we use a specific set of three points in each
 *      of the coordinate spaces: the first point is the origin,
 *      the second gives the orientation and scaling of the x axis
 *      relative to the origin, and the third gives the orientation
 *      and scaling of the y axis relative to the origin.
 *      correspond to a new origin and new x and y axes.
 *
 *      There are two different things we can demand: an
 *      affine coordinate transformation and an affine image
 *      transformation.
 *
 *      For the former, we ask for the coordinate value (x',y')
 *      in the transformed space for any point (x,y) in the original
 *      space.  To do this, it is most convenient to express the affine
 *      coordinate transformation using an LU decomposition of
 *      a set of six linear equations that express the six coordinates
 *      of the three points in the transformed space as a function of
 *      the six coordinates in the original space.  We can then
 *      do an affine image transformation, point by point, by
 *      back-substituting to get the new coordinates for every pixel
 *      in the image. 
 *
 *      This 'pointwise' transformation can be done either by sampling
 *      and picking a single pixel in the src to replicate into the dest,
 *      or by interpolating (or averaging) over four src pixels to
 *      determine the value of the dest pixel.  The first method is
 *      implemented by pixAffineSampled() and the second method by
 *      pixAffineInterpolated().  The interpolated method can only
 *      be used for images with more than 1 bpp, but for these, the
 *      image quality is significantly better than the sampled method.
 *
 *      Alternatively, the affine image transformation can be performed
 *      directly as a sequence of translations, shears and scalings,
 *      without computing directly the affine coordinate transformation.
 *      We have at our disposal (1) translations (using rasterop),
 *      (2) horizontal and vertical shear about any horizontal and vertical
 *      line, respectively, and (3) non-isotropic scaling by two
 *      arbitrary x and y scaling factors.  We also have rotation
 *      about an arbitrary point, but this is equivalent to a set 
 *      of three shears and we do not need to use it.
 *
 *      A typical application might be to align two images, which
 *      may be scaled, rotated and translated versions of each other.
 *      Through some pre-processing, three corresponding points are
 *      located in each of the two images.  One of the images is
 *      then to be (affine) transformed to align with the other.
 *
 *      Suppose that we are tranforming image 1 to correspond to image 2.
 *      We have a set of three points, describing the coordinate space
 *      embedded in image 1, and we need to transform image 1 until
 *      those three points exactly correspond to the new coordinate space
 *      defined by the second set of three points.  In our image
 *      matching application, the latter set of three points was
 *      found to be the corresponding points in image 2.
 *
 *      This can be done in a pointwise fashion.  Compute the
 *      (linear) affine transformation, given by the 2 sets of
 *      three corresponding points, in a form:
 *
 *          x' = ax + by + c
 *          y' = dx + ey + f
 *
 *      where the six coefficients have been computed.
 *      It is best to do this "backwards," where the image is to
 *      be transformed from (x',y') --> (x,y).  Then for every
 *      point (x,y) in the destination image, compute the corresponding
 *      point (x',y') in the source image.  For example, for 1 bpp images,
 *      if the pixel nearest (x',y') in the source is ON, then turn
 *      on the pixel at (x,y) in the dest.  The reason we do this
 *      "backwards" is because if we went the other way and iterated
 *      over the source image, because of integer sampling we can
 *      miss some destination pixels, and this would show up as a
 *      regular pattern of artifacts.
 *
 *      For binary images, it is usually more efficient to do such
 *      transformations by a sequence of word parallel operations.
 *      Shear and translation can be done in-place and word parallel;
 *      arbitrary scaling is mostly pixel-wise.
 *
 *      The most elegant way I can think of to do such a sequential
 *      implementation is to imagine that we're going to transform
 *      BOTH images until they're aligned.  (We don't really want
 *      to transform both, because in fact we may only have one image
 *      that is undergoing a general affine transformation.)
 *
 *      An important constraint is that we have shear operations
 *      about an arbitrary horizontal or vertical line, but always
 *      parallel to the x or y axis.  If we continue to pretend that
 *      we have an unprimed coordinate space embedded in image 1 and
 *      a primed coordinate space embedded in image 2, we imagine
 *      (a) transforming image 1 by horizontal and vertical shears about
 *      point 1 to align points 3 and 2 along the y and x axes,
 *      respectively, and (b) transforming image 2 by horizontal and
 *      vertical shears about point 1' to align points 3' and 2' along
 *      the y and x axes.  Then we scale image 1 so that the distances
 *      from 1 to 2 and from 1 to 3 are equal to the distances in
 *      image 2 from 1' to 2' and from 1' to 3'.  This scaling operation
 *      leaves the true image origin, at (0,0) invariant, and will in
 *      general translate point 1.  The original points 1 and 1' will
 *      typically not coincide in any event, so we must translate
 *      the origin of image 1, at its current point 1, to the origin
 *      of image 2 at 1'.  The images should now be aligned.  But
 *      because we never really transformed image 2 (and image 2 may
 *      not even exist), we now perform  on image 1 the reverse of
 *      the shear transforms that we imagined doing on image 2;
 *      namely, the negative vertical shear followed by the negative
 *      horizontal shear.  Image 1 should now have its transformed
 *      unprimed coordinates aligned with the original primed
 *      coordinates.  In all this, it is only necessary to keep track
 *      of the shear angles and translations of points during the shears.
 *      What has been accomplished is a general affine transformation
 *      on image 1.
 *
 *      Having described all this, if you are going to use an
 *      affine transformation in an application, this is what you
 *      need to know:
 *
 *          (1) You should NEVER use the sequential method, because
 *              the image quality for 1 bpp text is much poorer
 *              (even though it is about 2x faster than the pointwise sampled
 *              method), and for images with depth greater than 1, it is
 *              nearly 20x slower than the pointwise sampled method
 *              and over 10x slower than the pointwise interpolated method!
 *              The sequential method is given here for purely
 *              pedagogical reasons.
 *
 *          (2) For 1 bpp images, use the pointwise sampled function
 *              pixAffineSampled().  For all other images, the best
 *              quality results result from using the pointwise
 *              interpolated function pixAffineInterpolated(); the
 *              cost is less than a doubling of the computation time
 *              with respect to the sampled function.  If you use 
 *              interpolation on colormapped images, the colormap will
 *              be removed, resulting in either a grayscale or color
 *              image, depending on the values in the colormap.
 *              If you want to retain the colormap, use pixAffineSampled().
 *
 *      Typical relative timing of pointwise transforms (sampled = 1.0):
 *      8 bpp:   sampled        1.0
 *               interpolated   1.6
 *      32 bpp:  sampled        1.0
 *               interpolated   1.8
 *      Additionally, the computation time/pixel is nearly the same
 *      for 8 bpp and 32 bpp, for both sampled and interpolated.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "allheaders.h"

    /* gauss-jordan elimination */
#define  SWAP(a,b)   {temp = (a); (a) = (b); (b) = temp;}

#ifndef  NO_CONSOLE_IO
#define  DEBUG     0
#endif  /* ~NO_CONSOLE_IO */


/*-------------------------------------------------------------*
 *              Sequential affine image transformation         *
 *-------------------------------------------------------------*/
/*!
 *  pixAffineSequential()
 *
 *      Input:  pixs
 *              ptad  (3 pts of final coordinate space)
 *              ptas  (3 pts of initial coordinate space)
 *              bw    (pixels of additional border width during computation)
 *              bh    (pixels of additional border height during computation)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) The 3 pts must not be collinear.
 *      (2) The 3 pts must be given in this order:
 *            origin
 *            a location along the x-axis
 *            a location along the y-axis.
 *      (3)
 *      - This is about 3x faster on 1 bpp images than pixAffineSampled(),
 *        but the results on text are inferior.
 *      - You must guess how much border must be added so that no
 *        pixels are lost in the transformations from src to
 *        dest coordinate space.  (This can be calculated but it
 *        is a lot of work!)  For coordinate spaces that are nearly
 *        at right angles, on a 300 ppi scanned page, the addition
 *        of 1000 pixels on each side is usually sufficient.
 */
PIX *
pixAffineSequential(PIX     *pixs,
                    PTA     *ptad,
                    PTA     *ptas,
                    l_int32  bw,
                    l_int32  bh)
{
l_int32    x1, y1, x2, y2, x3, y3;    /* ptas */
l_int32    x1p, y1p, x2p, y2p, x3p, y3p;   /* ptad */
l_int32    x1sc, y1sc;  /* scaled origin */
l_float32  x2s, x2sp, scalex, scaley;
l_float32  th3, th3p, ph2, ph2p;
l_float32  rad2deg;
PIX       *pixt1, *pixt2, *pixd;

    PROCNAME("pixAffineSequential");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);

    if (ptaGetCount(ptas) != 3)
        return (PIX *)ERROR_PTR("ptas count not 3", procName, NULL);
    if (ptaGetCount(ptad) != 3)
        return (PIX *)ERROR_PTR("ptad count not 3", procName, NULL);
    ptaGetIPt(ptas, 0, &x1, &y1);
    ptaGetIPt(ptas, 1, &x2, &y2);
    ptaGetIPt(ptas, 2, &x3, &y3);
    ptaGetIPt(ptad, 0, &x1p, &y1p);
    ptaGetIPt(ptad, 1, &x2p, &y2p);
    ptaGetIPt(ptad, 2, &x3p, &y3p);

    rad2deg = 180. / 3.1415926535;

    if (y1 == y3)
        return (PIX *)ERROR_PTR("y1 == y3!", procName, NULL);
    if (y1p == y3p)
        return (PIX *)ERROR_PTR("y1p == y3p!", procName, NULL);
        
    if (bw != 0 || bh != 0) {
            /* resize all points and add border to pixs */
        x1 = x1 + bw;
        y1 = y1 + bh;
        x2 = x2 + bw;
        y2 = y2 + bh;
        x3 = x3 + bw;
        y3 = y3 + bh;
        x1p = x1p + bw;
        y1p = y1p + bh;
        x2p = x2p + bw;
        y2p = y2p + bh;
        x3p = x3p + bw;
        y3p = y3p + bh;

        if ((pixt1 = pixAddBorderGeneral(pixs, bw, bw, bh, bh, 0)) == NULL)
            return (PIX *)ERROR_PTR("pixt1 not made", procName, NULL);
    }
    else
        pixt1 = pixClone(pixs);

    /*-------------------------------------------------------------*
        The horizontal shear is done to move the 3rd point to the
        y axis.  This moves the 2nd point either towards or away
        from the y axis, depending on whether it is above or below
        the x axis.  That motion must be computed so that we know
        the angle of vertical shear to use to get the 2nd point
        on the x axis.  We must also know the x coordinate of the
        2nd point in order to compute how much scaling is required
        to match points on the axis.
     *-------------------------------------------------------------*/

        /* Shear angles required to put src points on x and y axes */
    th3 = atan2((l_float64)(x1 - x3), (l_float64)(y1 - y3));
    x2s = (l_float32)(x2 - ((l_float32)(y1 - y2) * (x3 - x1)) / (y1 - y3));
    if (x2s == (l_float32)x1)
        return (PIX *)ERROR_PTR("x2s == x1!", procName, NULL);
    ph2 = atan2((l_float64)(y1 - y2), (l_float64)(x2s - x1));

        /* Shear angles required to put dest points on x and y axes.
         * Use the negative of these values to instead move the
         * src points from the axes to the actual dest position.
         * These values are also needed to scale the image. */
    th3p = atan2((l_float64)(x1p - x3p), (l_float64)(y1p - y3p));
    x2sp = (l_float32)(x2p - ((l_float32)(y1p - y2p) * (x3p - x1p)) / (y1p - y3p));
    if (x2sp == (l_float32)x1p)
        return (PIX *)ERROR_PTR("x2sp == x1p!", procName, NULL);
    ph2p = atan2((l_float64)(y1p - y2p), (l_float64)(x2sp - x1p));

        /* Shear image to first put src point 3 on the y axis,
         * and then to put src point 2 on the x axis */
    pixHShearIP(pixt1, y1, th3, L_BRING_IN_WHITE);
    pixVShearIP(pixt1, x1, ph2, L_BRING_IN_WHITE);

        /* Scale image to match dest scale.  The dest scale
         * is calculated above from the angles th3p and ph2p
         * that would be required to move the dest points to
         * the x and y axes. */
    scalex = (l_float32)(x2sp - x1p) / (x2s - x1);
    scaley = (l_float32)(y3p - y1p) / (y3 - y1);
    if ((pixt2 = pixScale(pixt1, scalex, scaley)) == NULL)
        return (PIX *)ERROR_PTR("pixt2 not made", procName, NULL);

#if  DEBUG
    fprintf(stderr, "th3 = %5.1f deg, ph2 = %5.1f deg\n",
            rad2deg * th3, rad2deg * ph2);
    fprintf(stderr, "th3' = %5.1f deg, ph2' = %5.1f deg\n",
            rad2deg * th3p, rad2deg * ph2p);
    fprintf(stderr, "scalex = %6.3f, scaley = %6.3f\n", scalex, scaley);
#endif  /* DEBUG */

    /*-------------------------------------------------------------*
        Scaling moves the 1st src point, which is the origin. 
        It must now be moved again to coincide with the origin
        (1st point) of the dest.  After this is done, the 2nd
        and 3rd points must be sheared back to the original
        positions of the 2nd and 3rd dest points.  We use the
        negative of the angles that were previously computed
        for shearing those points in the dest image to x and y
        axes, and take the shears in reverse order as well.
     *-------------------------------------------------------------*/
        /* Shift image to match dest origin. */
    x1sc = (l_int32)(scalex * x1 + 0.5);   /* x comp of origin after scaling */
    y1sc = (l_int32)(scaley * y1 + 0.5);   /* y comp of origin after scaling */
    pixRasteropIP(pixt2, x1p - x1sc, y1p - y1sc, L_BRING_IN_WHITE);

        /* Shear image to take points 2 and 3 off the axis and
         * put them in the original dest position */
    pixVShearIP(pixt2, x1p, -ph2p, L_BRING_IN_WHITE);
    pixHShearIP(pixt2, y1p, -th3p, L_BRING_IN_WHITE);

    if (bw != 0 || bh != 0) {
        if ((pixd = pixRemoveBorderGeneral(pixt2, bw, bw, bh, bh)) == NULL)
            return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    }
    else
        pixd = pixClone(pixt2);

    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    return pixd;
}


/*-------------------------------------------------------------*
 *               Sampled affine image transformation           *
 *-------------------------------------------------------------*/
/*!
 *  pixAffineSampled()
 *
 *      Input:  pixs (all depths)
 *              ptad  (3 pts of final coordinate space)
 *              ptas  (3 pts of initial coordinate space)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Brings in either black or white pixels from the boundary.
 *      (2) Retains colormap, which you can do for a sampled transform..
 *      (3) The 3 points must not be collinear.
 *      (4) The order of the 3 points is arbitrary; however, to compare
 *          with the sequential transform they must be in these locations
 *          and in this order: origin, x-axis, y-axis.
 *      (5) For 1 bpp images, this has much better quality results
 *          than pixAffineSequential(), particularly for text.
 *          It is about 3x slower, but does not require additional
 *          border pixels.  The poor quality of pixAffineSequential()
 *          is due to repeated quantized transforms.  It is strongly
 *          recommended that pixAffineSampled() be used for 1 bpp images.
 *      (6) For 8 or 32 bpp, much better quality is obtained by the
 *          somewhat slower pixAffineInterpolated().  See that function
 *          for relative timings between sampled and interpolated.
 *      (7) To repeat, use of the sequential transform,
 *          pixAffineSequential(), for any images, is discouraged.
 */
PIX *
pixAffineSampled(PIX     *pixs,
                 PTA     *ptad,
                 PTA     *ptas,
                 l_int32  incolor)
{
l_int32     i, j, w, h, d, x, y, wpls, wpld, color, cmapindex;
l_uint32    val;
l_float32  *vc;
l_uint32   *datas, *datad, *lines, *lined;
PIX        *pixd;
PIXCMAP    *cmap;

    PROCNAME("pixAffineSampled");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor", procName, NULL);
    if (ptaGetCount(ptas) != 3)
        return (PIX *)ERROR_PTR("ptas count not 3", procName, NULL);
    if (ptaGetCount(ptad) != 3)
        return (PIX *)ERROR_PTR("ptad count not 3", procName, NULL);

        /* Get backwards transform from dest to src */
    affineXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);
    d = pixGetDepth(pixs);

        /* Init all dest pixels to color to be brought in from outside */
    if ((cmap = pixGetColormap(pixs)) != NULL) {
        if (incolor == L_BRING_IN_WHITE)
            color = 1;
        else
            color = 0;
        pixcmapAddBlackOrWhite(cmap, color, &cmapindex);
        pixSetAllArbitrary(pixd, cmapindex);
    }
    else {
        if ((d == 1 && incolor == L_BRING_IN_WHITE) ||
            (d > 1 && incolor == L_BRING_IN_BLACK))
            pixClearAll(pixd);
        else
            pixSetAll(pixd);
    }

        /* Scan over dest pixels */
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
            affineXformSampled(vc, j, i, &x, &y);
            if (x < 0 || y < 0 || x >=w || y >= h)
                continue;
            if (d == 1) {
                lines = datas + y * wpls;
                if (GET_DATA_BIT(lines, x))
                    SET_DATA_BIT(lined, j);
            }
            else if (d == 8) {
                lines = datas + y * wpls;
                val = GET_DATA_BYTE(lines, x);
                SET_DATA_BYTE(lined, j, val);
            }
            else if (d == 32) {
                lines = datas + y * wpls;
                lined[j] = lines[x];
            }
            else {  /* all other depths */
                pixGetPixel(pixs, x, y, &val);
                pixSetPixel(pixd, j, i, val);
            }
        }
    }

    FREE(vc);
    return pixd;
}


/*-------------------------------------------------------------*
 *           Interpolated affine image transformation          *
 *-------------------------------------------------------------*/
/*!
 *  pixAffineInterpolated()
 *
 *      Input:  pixs (2, 4, 8 bpp gray or colormapped, or 32 bpp RGB)
 *              ptad  (3 pts of final coordinate space)
 *              ptas  (3 pts of initial coordinate space)
 *              incolor (L_BRING_IN_WHITE, L_BRING_IN_BLACK)
 *      Return: pixd, or null on error
 *
 *  Notes:
 *      (1) Brings in either black or white pixels from the boundary
 *      (2) Removes any existing colormap, if necessary, before transforming
 */
PIX *
pixAffineInterpolated(PIX     *pixs,
                      PTA     *ptad,
                      PTA     *ptas,
                      l_int32  incolor)
{
l_int32   d;
l_uint32  colorval;
PIX      *pixt1, *pixt2, *pixd;

    PROCNAME("pixAffineInterpolated");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (pixGetDepth(pixs) == 1)
        return (PIX *)ERROR_PTR("pixs is 1 bpp", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (incolor != L_BRING_IN_WHITE && incolor != L_BRING_IN_BLACK)
        return (PIX *)ERROR_PTR("invalid incolor", procName, NULL);
    if (ptaGetCount(ptas) != 3)
        return (PIX *)ERROR_PTR("ptas count not 3", procName, NULL);
    if (ptaGetCount(ptad) != 3)
        return (PIX *)ERROR_PTR("ptad count not 3", procName, NULL);

        /* Remove cmap if it exists, and unpack to 8 bpp if necessary */
    pixt1 = pixRemoveColormap(pixs, REMOVE_CMAP_BASED_ON_SRC);
    d = pixGetDepth(pixt1);
    if (d < 8)
        pixt2 = pixConvertTo8(pixt1, FALSE);
    else
        pixt2 = pixClone(pixt1);
    d = pixGetDepth(pixt2);

        /* Compute actual color to bring in from edges */
    colorval = 0;
    if (incolor == L_BRING_IN_WHITE) {
        if (d == 8)
            colorval = 255;
        else  /* d == 32 */
            colorval = 0xffffff00;
    }
    
    if (d == 8)
        pixd = pixAffineInterpolatedGray(pixt2, ptad, ptas, colorval);
    else  /* d == 32 */
        pixd = pixAffineInterpolatedColor(pixt2, ptad, ptas, colorval);
    pixDestroy(&pixt1);
    pixDestroy(&pixt2);
    return pixd;
}


/*!
 *  pixAffineInterpolatedColor()
 *
 *      Input:  pixs (32 bpp)
 *              ptad  (3 pts of final coordinate space)
 *              ptas  (3 pts of initial coordinate space)
 *              colorval (e.g., 0 to bring in BLACK, 0xffffff00 for WHITE)
 *      Return: pixd, or null on error
 *
 *  *** Warning: implicit assumption about RGB component ordering ***
 */
PIX *
pixAffineInterpolatedColor(PIX      *pixs,
                           PTA      *ptad,
                           PTA      *ptas,
                           l_uint32  colorval)
{
l_int32     w, h, wpls, wpld;
l_uint32   *datas, *datad;
l_float32  *vc;
PIX        *pixd;

    PROCNAME("pixAffineInterpolatedColor");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (pixGetDepth(pixs) != 32)
        return (PIX *)ERROR_PTR("pixs must be 32 bpp", procName, NULL);
    if (ptaGetCount(ptas) != 3)
        return (PIX *)ERROR_PTR("ptas count not 3", procName, NULL);
    if (ptaGetCount(ptad) != 3)
        return (PIX *)ERROR_PTR("ptad count not 3", procName, NULL);

        /* Get backwards transform from dest to src */
    affineXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    pixSetAllArbitrary(pixd, colorval);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    affineInterpolatedColorLow(datad, w, h, wpld, datas, wpls, vc);
    FREE(vc);

    return pixd;
}


/*!
 *  pixAffineInterpolatedGray()
 *
 *      Input:  pixs (8 bpp)
 *              ptad  (3 pts of final coordinate space)
 *              ptas  (3 pts of initial coordinate space)
 *              grayval (0 to bring in BLACK, 255 for WHITE)
 *      Return: pixd, or null on error
 */
PIX *
pixAffineInterpolatedGray(PIX     *pixs,
                          PTA     *ptad,
                          PTA     *ptas,
                          l_uint8  grayval)
{
l_int32     w, h, wpls, wpld;
l_float32  *vc;
l_uint32   *datas, *datad;
PIX        *pixd;

    PROCNAME("pixAffineInterpolatedGray");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (!ptas)
        return (PIX *)ERROR_PTR("ptas not defined", procName, NULL);
    if (!ptad)
        return (PIX *)ERROR_PTR("ptad not defined", procName, NULL);
    if (pixGetDepth(pixs) != 8)
        return (PIX *)ERROR_PTR("pixs must be 8 bpp", procName, NULL);
    if (ptaGetCount(ptas) != 3)
        return (PIX *)ERROR_PTR("ptas count not 3", procName, NULL);
    if (ptaGetCount(ptad) != 3)
        return (PIX *)ERROR_PTR("ptad count not 3", procName, NULL);

        /* Get backwards transform from dest to src */
    affineXformCoeffs(ptad, ptas, &vc);

    w = pixGetWidth(pixs);
    h = pixGetHeight(pixs);
    datas = pixGetData(pixs);
    wpls = pixGetWpl(pixs);
    pixd = pixCreateTemplate(pixs);
    pixSetAllArbitrary(pixd, grayval);
    datad = pixGetData(pixd);
    wpld = pixGetWpl(pixd);

    affineInterpolatedGrayLow(datad, w, h, wpld, datas, wpls, vc);
    FREE(vc);

    return pixd;
}


/*!
 *  affineInterpolatedColorLow()
 */
void
affineInterpolatedColorLow(l_uint32   *datad,
                           l_int32     w,
                           l_int32     h,
                           l_int32     wpld,
                           l_uint32   *datas,
                           l_int32     wpls,
                           l_float32  *vc)
{
l_int32    i, j, x, y, xf, yf, wm2, hm2;
l_int32    rval, gval, bval;
l_uint32   word00, word01, word10, word11, val;
l_uint32  *lines, *lined;

        /* Iterate over destination pixels */
    wm2 = w - 2;
    hm2 = h - 2;
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
                /* Compute src pixel and fraction corresponding to (i,j) */
            affineXformInterpolated(vc, j, i, &x, &y, &xf, &yf);

                /* Skip if off the edge; omit x = 0 and y = 0 because
                 * xf and yf can be < 0, in which case overflow is
                 * possible for val, and black pixels can be rendered
                 * on pixels at the src boundaries. */
            if (x < 1 || y < 1 || x > wm2 || y > hm2)
                continue;

#if  DEBUG
            if (xf < 0 || yf < 0)
                fprintf(stderr, "x = %d, y = %d, xf = %d, yf = %d\n",
                     x, y, xf, yf);
#endif  /* DEBUG */

                /* Do area weighting (eqiv. to linear interpolation) */
            lines = datas + y * wpls;
            word00 = *(lines + x);
            word10 = *(lines + x + 1);
            word01 = *(lines + wpls + x);
            word11 = *(lines + wpls + x + 1);
            rval = ((16 - xf) * (16 - yf) * ((word00 >> L_RED_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_RED_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_RED_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_RED_SHIFT) & 0xff) + 128) / 256;
            gval = ((16 - xf) * (16 - yf) * ((word00 >> L_GREEN_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_GREEN_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_GREEN_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_GREEN_SHIFT) & 0xff) + 128) / 256;
            bval = ((16 - xf) * (16 - yf) * ((word00 >> L_BLUE_SHIFT) & 0xff) +
                xf * (16 - yf) * ((word10 >> L_BLUE_SHIFT) & 0xff) +
                (16 - xf) * yf * ((word01 >> L_BLUE_SHIFT) & 0xff) +
                xf * yf * ((word11 >> L_BLUE_SHIFT) & 0xff) + 128) / 256;
            val = (rval << L_RED_SHIFT) | (gval << L_GREEN_SHIFT) |
                  (bval << L_BLUE_SHIFT);
            *(lined + j) = val;
        }
    }

    return;
}


/*!
 *  affineInterpolatedGrayLow()
 */
void
affineInterpolatedGrayLow(l_uint32   *datad,
                          l_int32     w,
                          l_int32     h,
                          l_int32     wpld,
                          l_uint32   *datas,
                          l_int32     wpls,
                          l_float32  *vc)
{
l_int32     i, j, x, y, xf, yf, wm2, hm2;
l_int32     v00, v01, v10, v11;
l_uint8     val;
l_uint32   *lines, *lined;

        /* Iterate over destination pixels */
    wm2 = w - 2;
    hm2 = h - 2;
    for (i = 0; i < h; i++) {
        lined = datad + i * wpld;
        for (j = 0; j < w; j++) {
                /* Compute src pixel and fraction corresponding to (i,j) */
            affineXformInterpolated(vc, j, i, &x, &y, &xf, &yf);

                /* Skip if off the edge; omit x = 0 and y = 0 because
                 * xf and yf can be < 0, in which case overflow is
                 * possible for val, and black pixels can be rendered
                 * on pixels at the src boundaries. */
            if (x < 1 || y < 1 || x > wm2 || y > hm2)
                continue;

#if  DEBUG
            if (xf < 0 || yf < 0)
                fprintf(stderr, "x = %d, y = %d, xf = %d, yf = %d\n",
                     x, y, xf, yf);
#endif  /* DEBUG */

                /* Do area weighting (eqiv. to linear interpolation) */
            lines = datas + y * wpls;
            v00 = (16 - xf) * (16 - yf) * GET_DATA_BYTE(lines, x);
            v10 = xf * (16 - yf) * GET_DATA_BYTE(lines, x + 1);
            v01 = (16 - xf) * yf * GET_DATA_BYTE(lines + wpls, x);
            v11 = xf * yf * GET_DATA_BYTE(lines + wpls, x + 1);
            val = (l_uint8)((v00 + v01 + v10 + v11 + 128) / 256);
            SET_DATA_BYTE(lined, j, val);
        }
    }

    return;
}


/*-------------------------------------------------------------*
 *                 Affine coordinate transformation            *
 *-------------------------------------------------------------*/
/*!
 *  affineXformCoeffs()
 *
 *      Input:  ptas  (source 3 points; unprimed)
 *              ptad  (transformed 3 points; primed)
 *              &vc   (<return> vector of coefficients of transform)
 *      Return: 0 if OK; 1 on error
 *
 *  We have a set of six equations, describing the affine
 *  transformation that takes 3 points (ptas) into 3 other
 *  points (ptad).  These equations are:
 *
 *          x1' = c[0]*x1 + c[1]*y1 + c[2]
 *          y1' = c[3]*x1 + c[4]*y1 + c[5]
 *          x2' = c[0]*x2 + c[1]*y2 + c[2]
 *          y2' = c[3]*x2 + c[4]*y2 + c[5]
 *          x3' = c[0]*x3 + c[1]*y3 + c[2]
 *          y3' = c[3]*x3 + c[4]*y3 + c[5]
 *    
 *  This can be represented as
 *
 *          AC = B
 *
 *  where B and C are column vectors
 *    
 *          B = [ x1' y1' x2' y2' x3' y3' ]
 *          C = [ c[0] c[1] c[2] c[3] c[4] c[5] c[6] ]
 *    
 *  and A is the 6x6 matrix
 *
 *          x1   y1   1   0    0    0
 *           0    0   0   x1   y1   1
 *          x2   y2   1   0    0    0
 *           0    0   0   x2   y2   1
 *          x3   y3   1   0    0    0
 *           0    0   0   x3   y3   1
 *
 *  These six equations are solved here for the coefficients C.
 *
 *  These six coefficients can then be used to find the dest
 *  point (x',y') corresponding to any src point (x,y), according
 *  to the equations
 *
 *           x' = c[0]x + c[1]y + c[2]
 *           y' = c[3]x + c[4]y + c[5]
 *
 *  that are implemented in affineXform().
 */
l_int32
affineXformCoeffs(PTA         *ptas,
                  PTA         *ptad,
                  l_float32  **pvc)
{
l_int32     i;
l_float32   x1, y1, x2, y2, x3, y3;
l_float32  *b;   /* rhs vector of primed coords X'; coeffs returned in *pvc */
l_float32  *a[6];  /* 6x6 matrix A  */

    PROCNAME("affineXformCoeffs");

    if (!ptas)
        return ERROR_INT("ptas not defined", procName, 1);
    if (!ptad)
        return ERROR_INT("ptad not defined", procName, 1);
    if (!pvc)
        return ERROR_INT("&vc not defined", procName, 1);
        
    if ((b = (l_float32 *)CALLOC(6, sizeof(l_float32))) == NULL)
        return ERROR_INT("b not made", procName, 1);
    *pvc = b;

    ptaGetPt(ptas, 0, &x1, &y1);
    ptaGetPt(ptas, 1, &x2, &y2);
    ptaGetPt(ptas, 2, &x3, &y3);
    ptaGetPt(ptad, 0, &b[0], &b[1]);
    ptaGetPt(ptad, 1, &b[2], &b[3]);
    ptaGetPt(ptad, 2, &b[4], &b[5]);

    for (i = 0; i < 6; i++)
        if ((a[i] = (l_float32 *)CALLOC(6, sizeof(l_float32))) == NULL)
            return ERROR_INT("a[i] not made", procName, 1);

    a[0][0] = x1;
    a[0][1] = y1;
    a[0][2] = 1.;
    a[1][3] = x1;
    a[1][4] = y1;
    a[1][5] = 1.;
    a[2][0] = x2;
    a[2][1] = y2;
    a[2][2] = 1.;
    a[3][3] = x2;
    a[3][4] = y2;
    a[3][5] = 1.;
    a[4][0] = x3;
    a[4][1] = y3;
    a[4][2] = 1.;
    a[5][3] = x3;
    a[5][4] = y3;
    a[5][5] = 1.;

    gaussjordan(a, b, 6);

    for (i = 0; i < 6; i++)
        FREE(a[i]);

    return 0;
}


/*!
 *  affineXformSampled()
 *
 *      Input:  vc (vector of 6 coefficients)
 *              (x, y)  (initial point)
 *              (&xp, &yp)   (<return> transformed point)
 *      Return: 0 if OK; 1 on error
 */
l_int32
affineXformSampled(l_float32  *vc,
                   l_int32     x,
                   l_int32     y,
                   l_int32    *pxp,
                   l_int32    *pyp)
{
    PROCNAME("affineXformSampled");

    if (!vc)
        return ERROR_INT("vc not defined", procName, 1);

    *pxp = (l_int32)(vc[0] * x + vc[1] * y + vc[2] + 0.5);
    *pyp = (l_int32)(vc[3] * x + vc[4] * y + vc[5] + 0.5);

    return 0;
}


/*!
 *  affineXformInterpolated()
 *
 *      Input:  vc (vector of 6 coefficients)
 *              (x, y)  (initial point)
 *              (&xp, &yp)   (<return> transformed point)
 *              (&fxp, &fyp)   (<return> fractional transformed point)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: this does not check ptrs for returned data!
 */
l_int32
affineXformInterpolated(l_float32  *vc,
                        l_int32     x,
                        l_int32     y,
                        l_int32    *pxp,
                        l_int32    *pyp,
                        l_int32    *pfxp,
                        l_int32    *pfyp)
{
l_float32  xp, yp;

    PROCNAME("affineXformInterpolated");

    if (!vc)
        return ERROR_INT("vc not defined", procName, 1);

    xp = vc[0] * x + vc[1] * y + vc[2];
    yp = vc[3] * x + vc[4] * y + vc[5];
    *pxp = (l_int32)xp;
    *pyp = (l_int32)yp;
    *pfxp = (l_int32)(16. * (xp - *pxp));
    *pfyp = (l_int32)(16. * (yp - *pyp));

    return 0;
}



/*-------------------------------------------------------------*
 *               Gauss-jordan linear equation solver           *
 *-------------------------------------------------------------*/
/*!
 *  gaussjordan()
 *
 *      Input:   a  (n x n matrix)
 *               b  (rhs column vector)
 *               n  (dimension)
 *      Return:  0 if ok, 1 on error
 *
 *      Note side effects:
 *            (1) the matrix a is transformed to its inverse
 *            (2) the vector b is transformed to the solution X to the
 *                linear equation AX = B
 *
 *      Adapted from "Numerical Recipes in C, Second Edition", 1992
 *      pp. 36-41 (gauss-jordan elimination)
 */
l_int32
gaussjordan(l_float32  **a,
            l_float32   *b,
            l_int32      n)
{
l_int32    i, icol, irow, j, k, l, ll;
l_int32   *indexc, *indexr, *ipiv;
l_float32  big, dum, pivinv, temp;

    PROCNAME("gaussjordan");

    if (!a)
        return ERROR_INT("a not defined", procName, 1);
    if (!b)
        return ERROR_INT("b not defined", procName, 1);

    if ((indexc = (l_int32 *)CALLOC(n, sizeof(l_int32))) == NULL)
        return ERROR_INT("indexc not made", procName, 1);
    if ((indexr = (l_int32 *)CALLOC(n, sizeof(l_int32))) == NULL)
        return ERROR_INT("indexr not made", procName, 1);
    if ((ipiv = (l_int32 *)CALLOC(n, sizeof(l_int32))) == NULL)
        return ERROR_INT("ipiv not made", procName, 1);

    for (i = 0; i < n; i++) {
        big = 0.0;
        for (j = 0; j < n; j++) {
            if (ipiv[j] != 1)
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(a[j][k]) >= big) {
                            big = fabs(a[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                    else if (ipiv[k] > 1)
                        return ERROR_INT("singular matrix", procName, 1);
                }
        }
        ++(ipiv[icol]);
        
        if (irow != icol) {
            for (l = 0; l < n; l++)
                SWAP(a[irow][l], a[icol][l]);
            SWAP(b[irow], b[icol]);
        }

        indexr[i] = irow;
        indexc[i] = icol;
        if (a[icol][icol] == 0.0)
            return ERROR_INT("singular matrix", procName, 1);
        pivinv = 1.0 / a[icol][icol];
        a[icol][icol] = 1.0;
        for (l = 0; l < n; l++)
            a[icol][l] *= pivinv;
        b[icol] *= pivinv;

        for (ll = 0; ll < n; ll++) 
            if (ll != icol) {
                dum = a[ll][icol];
                a[ll][icol] = 0.0;
                for (l = 0; l < n; l++)
                    a[ll][l] -= a[icol][l] * dum;
                b[ll] -= b[icol] * dum;
            }
    }

    for (l = n - 1; l >= 0; l--) {
        if (indexr[l] != indexc[l])
            for (k = 0; k < n; k++)
                SWAP(a[k][indexr[l]], a[k][indexc[l]]);
    }

    FREE(indexr);
    FREE(indexc);
    FREE(ipiv);
    return 0;
}
