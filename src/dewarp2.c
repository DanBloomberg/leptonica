/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*!
 * \file dewarp2.c
 * <pre>
 *
 *    Build the page disparity model
 *
 *      Build page disparity model
 *          l_int32            dewarpBuildPageModel()
 *          l_int32            dewarpFindVertDisparity()
 *          l_int32            dewarpFindHorizDisparity()
 *          PTAA              *dewarpGetTextlineCenters()
 *          static PTA        *dewarpGetMeanVerticals()
 *          PTAA              *dewarpRemoveShortLines()
 *          static l_int32     dewarpGetLineEndpoints()
 *          static l_int32     dewarpFindLongLines()
 *          static l_int32     dewarpIsLineCoverageValid()
 *          static l_int32     dewarpQuadraticLSF()
 *
 *      Build the line disparity model
 *          l_int32            dewarpBuildLineModel()
 *
 *      Query model status
 *          l_int32            dewarpaModelStatus()
 *
 *      Rendering helpers
 *          static l_int32     pixRenderMidYs()
 *          static l_int32     pixRenderHorizEndPoints
 * </pre>
 */

#include <math.h>
#include "allheaders.h"

static PTA *dewarpGetMeanVerticals(PIX *pixs, l_int32 x, l_int32 y);
static l_int32 dewarpGetLineEndpoints(l_int32 h, PTAA *ptaa, PTA **pptal,
                                      PTA **pptar);
static l_int32 dewarpFindLongLines(PTA *ptal, PTA *ptar, l_float32 minfract,
                                   PTA **pptald, PTA **pptard);
static l_int32 dewarpIsLineCoverageValid(PTAA *ptaa2, l_int32 h,
                                         l_int32 *ptopline, l_int32 *pbotline);
static l_int32 dewarpQuadraticLSF(PTA *ptad, l_float32 *pa, l_float32 *pb,
                                  l_float32 *pc, l_float32 *pmederr);
static l_int32 pixRenderMidYs(PIX *pixs, NUMA *namidys, l_int32 linew);
static l_int32 pixRenderHorizEndPoints(PIX *pixs, PTA *ptal, PTA *ptar,
                                       l_uint32 color);


#ifndef  NO_CONSOLE_IO
#define  DEBUG_TEXTLINE_CENTERS    0   /* set this to 1 for debuging */
#define  DEBUG_SHORT_LINES         0   /* ditto */
#else
#define  DEBUG_TEXTLINE_CENTERS    0   /* always must be 0 */
#define  DEBUG_SHORT_LINES         0   /* ditto */
#endif  /* !NO_CONSOLE_IO */

    /* Special parameter values */
static const l_float32   MIN_RATIO_LINES_TO_HEIGHT = 0.45f;


/*----------------------------------------------------------------------*
 *                      Build page disparity model                      *
 *----------------------------------------------------------------------*/
/*!
 * \brief   dewarpBuildPageModel()
 *
 * \param[in]    dew
 * \param[in]    debugfile use NULL to skip writing this
 * \return  0 if OK, 1 if unable to build the model or on error
 *
 * <pre>
 * Notes:
 *      (1) This is the basic function that builds the horizontal and
 *          vertical disparity arrays, which allow determination of the
 *          src pixel in the input image corresponding to each
 *          dest pixel in the dewarped image.
 *      (2) Sets vsuccess = 1 if the vertical disparity array builds.
 *          Always attempts to build the horizontal disparity array,
 *          even if it will not be requested (useboth == 0).
 *          Sets hsuccess = 1 if horizontal disparity builds.
 *      (3) The method is as follows:
 *          (a) Estimate the points along the centers of all the
 *              long textlines.  If there are too few lines, no
 *              disparity models are built.
 *          (b) From the vertical deviation of the lines, estimate
 *              the vertical disparity.
 *          (c) From the ends of the lines, estimate the horizontal
 *              disparity, assuming that the text is made of lines
 *              that are close to left and right justified.
 *          (d) One can also compute an additional contribution to the
 *              horizontal disparity, inferred from slopes of the top
 *              and bottom lines.  We do not do this.
 *      (4) In more detail for the vertical disparity:
 *          (a) Fit a LS quadratic to center locations along each line.
 *              This smooths the curves.
 *          (b) Sample each curve at a regular interval, find the y-value
 *              of the mid-point on each curve, and subtract the sampled
 *              curve value from this value.  This is the vertical
 *              disparity at sampled points along each curve.
 *          (c) Fit a LS quadratic to each set of vertically aligned
 *              disparity samples.  This smooths the disparity values
 *              in the vertical direction.  Then resample at the same
 *              regular interval.  We now have a regular grid of smoothed
 *              vertical disparity valuels.
 *      (5) Once the sampled vertical disparity array is found, it can be
 *          interpolated to get a full resolution vertical disparity map.
 *          This can be applied directly to the src image pixels
 *          to dewarp the image in the vertical direction, making
 *          all textlines horizontal.  Likewise, the horizontal
 *          disparity array is used to left- and right-align the
 *          longest textlines.
 * </pre>
 */
l_int32
dewarpBuildPageModel(L_DEWARP    *dew,
                     const char  *debugfile)
{
l_int32  linecount, topline, botline, ret;
PIX     *pixs, *pix1, *pix2, *pix3;
PTA     *pta;
PTAA    *ptaa1, *ptaa2;

    PROCNAME("dewarpBuildPageModel");

    if (!dew)
        return ERROR_INT("dew not defined", procName, 1);

    dew->debug = (debugfile) ? 1 : 0;
    dew->vsuccess = dew->hsuccess = 0;
    pixs = dew->pixs;
    if (debugfile) {
        lept_rmdir("lept/dewmod");  /* erase previous images */
        lept_mkdir("lept/dewmod");
        pixDisplayWithTitle(pixs, 0, 0, "pixs", 1);
        pixWrite("/tmp/lept/dewmod/0010.png", pixs, IFF_PNG);
    }

        /* Make initial estimate of centers of textlines */
    ptaa1 = dewarpGetTextlineCenters(pixs, debugfile || DEBUG_TEXTLINE_CENTERS);
    if (!ptaa1) {
        L_WARNING("textline centers not found; model not built\n", procName);
        return 1;
    }
    if (debugfile) {
        pix1 = pixConvertTo32(pixs);
        pta = generatePtaFilledCircle(1);
        pix2 = pixGenerateFromPta(pta, 5, 5);
        pix3 = pixDisplayPtaaPattern(NULL, pix1, ptaa1, pix2, 2, 2);
        pixWrite("/tmp/lept/dewmod/0020.png", pix3, IFF_PNG);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pix3);
        ptaDestroy(&pta);
    }

        /* Remove all lines that are not at least 0.8 times the length
         * of the longest line. */
    ptaa2 = dewarpRemoveShortLines(pixs, ptaa1, 0.8,
                                   debugfile || DEBUG_SHORT_LINES);
    if (debugfile) {
        pix1 = pixConvertTo32(pixs);
        pta = generatePtaFilledCircle(1);
        pix2 = pixGenerateFromPta(pta, 5, 5);
        pix3 = pixDisplayPtaaPattern(NULL, pix1, ptaa2, pix2, 2, 2);
        pixWrite("/tmp/lept/dewmod/0030.png", pix3, IFF_PNG);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pix3);
        ptaDestroy(&pta);
    }
    ptaaDestroy(&ptaa1);

        /* Verify that there are sufficient "long" lines */
    linecount = ptaaGetCount(ptaa2);
    if (linecount < dew->minlines) {
        ptaaDestroy(&ptaa2);
        L_WARNING("linecount %d < min req'd number of lines (%d) for model\n",
                  procName, linecount, dew->minlines);
        return 1;
    }

        /* Verify that the lines have a reasonable coverage of the
         * vertical extent of the image foreground. */
    if (dewarpIsLineCoverageValid(ptaa2, pixGetHeight(pixs),
                                  &topline, &botline) == FALSE) {
        ptaaDestroy(&ptaa2);
        L_WARNING("invalid line coverage: [%d ... %d] in height %d\n",
                  procName, topline, botline, pixGetHeight(pixs));
        return 1;
    }

        /* Get the sampled vertical disparity from the textline centers.
         * The disparity array will push pixels vertically so that each
         * textline is flat and centered at the y-position of the mid-point. */
    if (dewarpFindVertDisparity(dew, ptaa2, 0) != 0) {
        L_WARNING("vertical disparity not built\n", procName);
        ptaaDestroy(&ptaa2);
        return 1;
    }

        /* Get the sampled horizontal disparity from the left and right
         * edges of the text.  The disparity array will expand the image
         * linearly outward to align the text edges vertically.
         * Do this even if useboth == 0; we still calculate it even
         * if we don't plan to use it. */
    if ((ret = dewarpFindHorizDisparity(dew, ptaa2)) == 0)
        L_INFO("hsuccess = 1\n", procName);

        /* Debug output */
    if (debugfile) {
        dewarpPopulateFullRes(dew, NULL, 0, 0);
        pix1 = fpixRenderContours(dew->fullvdispar, 3.0, 0.15);
        pixWrite("/tmp/lept/dewmod/0060.png", pix1, IFF_PNG);
        pixDisplay(pix1, 1000, 0);
        pixDestroy(&pix1);
        if (ret == 0) {
            pix1 = fpixRenderContours(dew->fullhdispar, 3.0, 0.15);
            pixWrite("/tmp/lept/dewmod/0070.png", pix1, IFF_PNG);
            pixDisplay(pix1, 1000, 0);
            pixDestroy(&pix1);
        }
        convertFilesToPdf("/tmp/lept/dewmod", NULL, 135, 1.0, 0, 0,
                          "Dewarp Build Model", debugfile);
        fprintf(stderr, "pdf file: %s\n", debugfile);
    }

    ptaaDestroy(&ptaa2);
    return 0;
}


/*!
 * \brief   dewarpFindVertDisparity()
 *
 * \param[in]    dew
 * \param[in]    ptaa unsmoothed lines, not vertically ordered
 * \param[in]    rotflag 0 if using dew->pixs; 1 if rotated by 90 degrees cw
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This starts with points along the centers of textlines.
 *          It does quadratic fitting (and smoothing), first along the
 *          lines and then in the vertical direction, to generate
 *          the sampled vertical disparity map.  This can then be
 *          interpolated to full resolution and used to remove
 *          the vertical line warping.
 *      (2) Use %rotflag == 1 if you are dewarping vertical lines, as
 *          is done in dewarpBuildLineModel().  The usual case is for
 *          %rotflag == 0.
 *      (3) Note that this builds a vertical disparity model (VDM), but
 *          does not check it against constraints for validity.
 *          Constraint checking is done after building the models,
 *          and before inserting reference models.
 *      (4) This sets the vsuccess flag to 1 on success.
 *      (5) Pix debug output goes to /tmp/dewvert/ for collection into
 *          a pdf.  Non-pix debug output goes to /tmp.
 * </pre>
 */
l_int32
dewarpFindVertDisparity(L_DEWARP  *dew,
                        PTAA      *ptaa,
                        l_int32    rotflag)
{
l_int32     i, j, nlines, npts, nx, ny, sampling;
l_float32   c0, c1, c2, x, y, midy, val, medval, medvar, minval, maxval;
l_float32  *famidys;
NUMA       *nax, *nafit, *nacurve0, *nacurve1, *nacurves;
NUMA       *namidy, *namidys, *namidysi;
PIX        *pix1, *pix2, *pixcirc, *pixdb;
PTA        *pta, *ptad, *ptacirc;
PTAA       *ptaa0, *ptaa1, *ptaa2, *ptaa3, *ptaa4, *ptaa5, *ptaat;
FPIX       *fpix;

    PROCNAME("dewarpFindVertDisparity");

    if (!dew)
        return ERROR_INT("dew not defined", procName, 1);
    dew->vsuccess = 0;
    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);

    if (dew->debug) L_INFO("finding vertical disparity\n", procName);

        /* Do quadratic fit to smooth each line.  A single quadratic
         * over the entire width of the line appears to be sufficient.
         * Quartics tend to overfit to noise.  Each line is thus
         * represented by three coefficients: y(x) = c2 * x^2 + c1 * x + c0.
         * Using the coefficients, sample each fitted curve uniformly
         * across the full width of the image.  The result is in ptaa0.  */
    sampling = dew->sampling;
    nx = (rotflag) ? dew->ny : dew->nx;
    ny = (rotflag) ? dew->nx : dew->ny;
    nlines = ptaaGetCount(ptaa);
    dew->nlines = nlines;
    ptaa0 = ptaaCreate(nlines);
    nacurve0 = numaCreate(nlines);  /* stores curvature coeff c2 */
    pixdb = (rotflag) ? pixRotateOrth(dew->pixs, 1) : pixClone(dew->pixs);
    for (i = 0; i < nlines; i++) {  /* for each line */
        pta = ptaaGetPta(ptaa, i, L_CLONE);
        ptaGetQuadraticLSF(pta, &c2, &c1, &c0, NULL);
        numaAddNumber(nacurve0, c2);
        ptad = ptaCreate(nx);
        for (j = 0; j < nx; j++) {  /* uniformly sampled in x */
             x = j * sampling;
             applyQuadraticFit(c2, c1, c0, x, &y);
             ptaAddPt(ptad, x, y);
        }
        ptaaAddPta(ptaa0, ptad, L_INSERT);
        ptaDestroy(&pta);
    }
    if (dew->debug) {
        lept_mkdir("lept/dewarp");
        lept_mkdir("lept/dewdebug");
        lept_mkdir("lept/dewmod");
        ptaat = ptaaCreate(nlines);
        for (i = 0; i < nlines; i++) {
            pta = ptaaGetPta(ptaa, i, L_CLONE);
            ptaGetArrays(pta, &nax, NULL);
            ptaGetQuadraticLSF(pta, NULL, NULL, NULL, &nafit);
            ptad = ptaCreateFromNuma(nax, nafit);
            ptaaAddPta(ptaat, ptad, L_INSERT);
            ptaDestroy(&pta);
            numaDestroy(&nax);
            numaDestroy(&nafit);
        }
        pix1 = pixConvertTo32(pixdb);
        pta = generatePtaFilledCircle(1);
        pixcirc = pixGenerateFromPta(pta, 5, 5);
        pix2 = pixDisplayPtaaPattern(NULL, pix1, ptaat, pixcirc, 2, 2);
        pixWrite("/tmp/lept/dewmod/0041.png", pix2, IFF_PNG);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        ptaDestroy(&pta);
        pixDestroy(&pixcirc);
        ptaaDestroy(&ptaat);
    }

        /* Remove lines with outlier curvatures.
         * Note that this is just looking for internal consistency in
         * the line curvatures.  It is not rejecting lines based on
         * the magnitude of the curvature.  That is done when constraints
         * are applied for valid models. */
    numaGetMedianVariation(nacurve0, &medval, &medvar);
    L_INFO("\nPage %d\n", procName, dew->pageno);
    L_INFO("Pass 1: Curvature: medval = %f, medvar = %f\n",
           procName, medval, medvar);
    ptaa1 = ptaaCreate(nlines);
    nacurve1 = numaCreate(nlines);
    for (i = 0; i < nlines; i++) {  /* for each line */
        numaGetFValue(nacurve0, i, &val);
        if (L_ABS(val - medval) > 7.0 * medvar)  /* TODO: reduce to ~ 3.0 */
            continue;
        pta = ptaaGetPta(ptaa0, i, L_CLONE);
        ptaaAddPta(ptaa1, pta, L_INSERT);
        numaAddNumber(nacurve1, val);
    }
    nlines = ptaaGetCount(ptaa1);
    numaDestroy(&nacurve0);

        /* Save the min and max curvature (in micro-units) */
    numaGetMin(nacurve1, &minval, NULL);
    numaGetMax(nacurve1, &maxval, NULL);
    dew->mincurv = lept_roundftoi(1000000. * minval);
    dew->maxcurv = lept_roundftoi(1000000. * maxval);
    L_INFO("Pass 2: Min/max curvature = (%d, %d)\n", procName,
           dew->mincurv, dew->maxcurv);

        /* Find and save the y values at the mid-points in each curve.
         * If the slope is zero anywhere, it will typically be here. */
    namidy = numaCreate(nlines);
    for (i = 0; i < nlines; i++) {
        pta = ptaaGetPta(ptaa1, i, L_CLONE);
        npts = ptaGetCount(pta);
        ptaGetPt(pta, npts / 2, NULL, &midy);
        numaAddNumber(namidy, midy);
        ptaDestroy(&pta);
    }

        /* Sort the lines in ptaa1c by their vertical position, going down */
    namidysi = numaGetSortIndex(namidy, L_SORT_INCREASING);
    namidys = numaSortByIndex(namidy, namidysi);
    nacurves = numaSortByIndex(nacurve1, namidysi);
    numaDestroy(&dew->namidys);  /* in case previously made */
    numaDestroy(&dew->nacurves);
    dew->namidys = namidys;
    dew->nacurves = nacurves;
    ptaa2 = ptaaSortByIndex(ptaa1, namidysi);
    numaDestroy(&namidy);
    numaDestroy(&nacurve1);
    numaDestroy(&namidysi);
    if (dew->debug) {
        numaWrite("/tmp/lept/dewdebug/midys.na", namidys);
        numaWrite("/tmp/lept/dewdebug/curves.na", nacurves);
        pix1 = pixConvertTo32(pixdb);
        ptacirc = generatePtaFilledCircle(5);
        pixcirc = pixGenerateFromPta(ptacirc, 11, 11);
        srand(3);
        pixDisplayPtaaPattern(pix1, pix1, ptaa2, pixcirc, 5, 5);
        srand(3);  /* use the same colors for text and reference lines */
        pixRenderMidYs(pix1, namidys, 2);
        pix2 = (rotflag) ? pixRotateOrth(pix1, 3) : pixClone(pix1);
        pixWrite("/tmp/lept/dewmod/0042.png", pix2, IFF_PNG);
        pixDisplay(pix2, 0, 0);
        ptaDestroy(&ptacirc);
        pixDestroy(&pixcirc);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
    }
    pixDestroy(&pixdb);

        /* Convert the sampled points in ptaa2 to a sampled disparity with
         * with respect to the y value at the mid point in the curve.
         * The disparity is the distance the point needs to move;
         * plus is downward.  */
    ptaa3 = ptaaCreate(nlines);
    for (i = 0; i < nlines; i++) {
        pta = ptaaGetPta(ptaa2, i, L_CLONE);
        numaGetFValue(namidys, i, &midy);
        ptad = ptaCreate(nx);
        for (j = 0; j < nx; j++) {
            ptaGetPt(pta, j, &x, &y);
            ptaAddPt(ptad, x, midy - y);
        }
        ptaaAddPta(ptaa3, ptad, L_INSERT);
        ptaDestroy(&pta);
    }
    if (dew->debug) {
        ptaaWrite("/tmp/lept/dewdebug/ptaa3.ptaa", ptaa3, 0);
    }

        /* Generate ptaa4 by taking vertical 'columns' from ptaa3.
         * We want to fit the vertical disparity on the column to the
         * vertical position of the line, which we call 'y' here and
         * obtain from namidys.  So each pta in ptaa4 is the set of
         * vertical disparities down a column of points.  The columns
         * in ptaa4 are equally spaced in x. */
    ptaa4 = ptaaCreate(nx);
    famidys = numaGetFArray(namidys, L_NOCOPY);
    for (j = 0; j < nx; j++) {
        pta = ptaCreate(nlines);
        for (i = 0; i < nlines; i++) {
            y = famidys[i];
            ptaaGetPt(ptaa3, i, j, NULL, &val);  /* disparity value */
            ptaAddPt(pta, y, val);
        }
        ptaaAddPta(ptaa4, pta, L_INSERT);
    }
    if (dew->debug) {
        ptaaWrite("/tmp/lept/dewdebug/ptaa4.ptaa", ptaa4, 0);
    }

        /* Do quadratic fit vertically on each of the pixel columns
         * in ptaa4, for the vertical displacement (which identifies the
         * src pixel(s) for each dest pixel) as a function of y (the
         * y value of the mid-points for each line).  Then generate
         * ptaa5 by sampling the fitted vertical displacement on a
         * regular grid in the vertical direction.  Each pta in ptaa5
         * gives the vertical displacement for regularly sampled y values
         * at a fixed x. */
    ptaa5 = ptaaCreate(nx);  /* uniformly sampled across full height of image */
    for (j = 0; j < nx; j++) {  /* for each column */
        pta = ptaaGetPta(ptaa4, j, L_CLONE);
        ptaGetQuadraticLSF(pta, &c2, &c1, &c0, NULL);
        ptad = ptaCreate(ny);
        for (i = 0; i < ny; i++) {  /* uniformly sampled in y */
             y = i * sampling;
             applyQuadraticFit(c2, c1, c0, y, &val);
             ptaAddPt(ptad, y, val);
        }
        ptaaAddPta(ptaa5, ptad, L_INSERT);
        ptaDestroy(&pta);
    }
    if (dew->debug) {
        ptaaWrite("/tmp/lept/dewdebug/ptaa5.ptaa", ptaa5, 0);
        convertFilesToPdf("/tmp/lept/dewmod", "004", 135, 1.0, 0, 0,
                          "Dewarp Vert Disparity",
                          "/tmp/lept/dewarp/vert_disparity.pdf");
        fprintf(stderr, "pdf file: /tmp/lept/dewarp/vert_disparity.pdf\n");
    }

        /* Save the result in a fpix at the specified subsampling  */
    fpix = fpixCreate(nx, ny);
    for (i = 0; i < ny; i++) {
        for (j = 0; j < nx; j++) {
            ptaaGetPt(ptaa5, j, i, NULL, &val);
            fpixSetPixel(fpix, j, i, val);
        }
    }
    dew->sampvdispar = fpix;
    dew->vsuccess = 1;

    ptaaDestroy(&ptaa0);
    ptaaDestroy(&ptaa1);
    ptaaDestroy(&ptaa2);
    ptaaDestroy(&ptaa3);
    ptaaDestroy(&ptaa4);
    ptaaDestroy(&ptaa5);
    return 0;
}


/*!
 * \brief   dewarpFindHorizDisparity()
 *
 * \param[in]    dew
 * \param[in]    ptaa unsmoothed lines, not vertically ordered
 * \return  0 if OK, 1 if vertical disparity array is no built or on error
 *
 * <pre>
 * Notes:
 *      (1) This builds a horizontal disparity model (HDM), but
 *          does not check it against constraints for validity.
 *          Constraint checking is done at rendering time.
 *      (2) This is not required for a successful model; only the vertical
 *          disparity is required.  This will not be called if the
 *          function to build the vertical disparity fails.
 *      (3) This sets the hsuccess flag to 1 on success.
 *      (4) Internally in ptal1, ptar1, ptal2, ptar2: x and y are reversed,
 *          so the 'y' value is horizontal distance across the image width.
 *      (5) Debug output goes to /tmp/lept/dewmod/ for collection into a pdf.
 * </pre>
 */
l_int32
dewarpFindHorizDisparity(L_DEWARP  *dew,
                         PTAA      *ptaa)
{
l_int32    i, j, w, h, nx, ny, sampling, ret;
l_float32  c0, c1, cl0, cl1, cl2, cr0, cr1, cr2;
l_float32  x, y, ymin, ymax, refl, refr;
l_float32  val, mederr;
NUMA      *nald, *nard;
PIX       *pix1;
PTA       *ptal1, *ptar1;  /* left/right end points of lines; initial */
PTA       *ptal2, *ptar2;  /* left/right end points; after filtering */
PTA       *ptal3, *ptar3;  /* left/right end points; long lines only */
PTA       *ptal4, *ptar4;  /* left and right block, fitted, uniform spacing */
PTA       *pta, *ptat, *pta1, *pta2;
PTAA      *ptaah;
FPIX      *fpix;

    PROCNAME("dewarpFindHorizDisparity");

    if (!dew)
        return ERROR_INT("dew not defined", procName, 1);
    dew->hsuccess = 0;
    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 1);

    lept_mkdir("lept/dewdebug");
    lept_mkdir("lept/dewarp");
    if (dew->debug) L_INFO("finding horizontal disparity\n", procName);

        /* Get the endpoints of the lines */
    h = pixGetHeight(dew->pixs);
    ret = dewarpGetLineEndpoints(h, ptaa, &ptal1, &ptar1);
    if (ret) {
        L_INFO("Horiz disparity not built\n", procName);
        return 1;
    }
    if (dew->debug) {
        ptaWrite("/tmp/lept/dewdebug/endpts_left1.pta", ptal1, 1);
        ptaWrite("/tmp/lept/dewdebug/endpts_right1.pta", ptar1, 1);
    }

        /* Filter the points by x-location to prevent 2-column images
         * from getting confused about left and right endpoints. We
         * require valid left points to not be farther than
         *     0.15 * (remaining distance to the right edge of the image)
         * to the right of the leftmost endpoint, and similarly for
         * the right endpoints. (Note: x and y are reversed in the pta.) */
    w = pixGetWidth(dew->pixs);
    ptaGetMinMax(ptal1, NULL, &ymin, NULL, NULL);
    ptal2 = ptaSelectByValue(ptal1, 0, ymin + 0.15 * (w - ymin),
                             L_SELECT_YVAL, L_SELECT_IF_LT);
    ptaGetMinMax(ptar1, NULL, NULL, NULL, &ymax);
    ptar2 = ptaSelectByValue(ptar1, 0, 0.85 * ymax, L_SELECT_YVAL,
                             L_SELECT_IF_GT);
    ptaDestroy(&ptal1);
    ptaDestroy(&ptar1);
    if (dew->debug) {
        ptaWrite("/tmp/lept/dewdebug/endpts_left2.pta", ptal2, 1);
        ptaWrite("/tmp/lept/dewdebug/endpts_right2.pta", ptar2, 1);
    }

        /* Do a quadratic fit to the left and right endpoints of the
         * longest lines.  Each line is represented by 3 coefficients:
         *     x(y) = c2 * y^2 + c1 * y + c0.
         * Using the coefficients, sample each fitted curve uniformly
         * along the full height of the image. */
    sampling = dew->sampling;
    nx = dew->nx;
    ny = dew->ny;

        /* Find the top and bottom set of long lines, defined by being
         * at least 0.95 of the length of the longest line in each set.
         * Quit if there are not at least 3 lines in each set. */
    ptal3 = ptar3 = NULL;  /* end points of longest lines */
    ret = dewarpFindLongLines(ptal2, ptar2, 0.95, &ptal3, &ptar3);
    if (ret) {
        L_INFO("Horiz disparity not built\n", procName);
        ptaDestroy(&ptal2);
        ptaDestroy(&ptar2);
        return 1;
    }

        /* Fit the left side, using quadratic LSF on the set of long
         * lines.  It is not necessary to use the noisy LSF fit
         * function, because we've removed outlier end points by
         * selecting the long lines.  Then uniformly sample along
         * this fitted curve. */
    dewarpQuadraticLSF(ptal3, &cl2, &cl1, &cl0, &mederr);
    dew->leftslope = lept_roundftoi(1000. * cl1);  /* milli-units */
    dew->leftcurv = lept_roundftoi(1000000. * cl2);  /* micro-units */
    L_INFO("Left quad LSF median error = %5.2f\n", procName,  mederr);
    L_INFO("Left edge slope = %d\n", procName, dew->leftslope);
    L_INFO("Left edge curvature = %d\n", procName, dew->leftcurv);
    ptal4 = ptaCreate(ny);
    for (i = 0; i < ny; i++) {  /* uniformly sampled in y */
        y = i * sampling;
        applyQuadraticFit(cl2, cl1, cl0, y, &x);
        ptaAddPt(ptal4, x, y);
    }

        /* Fit the right side in the same way. */
    dewarpQuadraticLSF(ptar3, &cr2, &cr1, &cr0, &mederr);
    dew->rightslope = lept_roundftoi(1000.0 * cr1);  /* milli-units */
    dew->rightcurv = lept_roundftoi(1000000. * cr2);  /* micro-units */
    L_INFO("Right quad LSF median error = %5.2f\n", procName,  mederr);
    L_INFO("Right edge slope = %d\n", procName, dew->rightslope);
    L_INFO("Right edge curvature = %d\n", procName, dew->rightcurv);
    ptar4 = ptaCreate(ny);
    for (i = 0; i < ny; i++) {  /* uniformly sampled in y */
        y = i * sampling;
        applyQuadraticFit(cr2, cr1, cr0, y, &x);
        ptaAddPt(ptar4, x, y);
    }

    if (dew->debug) {
        PTA  *ptalft, *ptarft;
        h = pixGetHeight(dew->pixs);
        pta1 = ptaCreate(h);
        pta2 = ptaCreate(h);
        for (i = 0; i < h; i++) {
            applyQuadraticFit(cl2, cl1, cl0, i, &x);
            ptaAddPt(pta1, x, i);
            applyQuadraticFit(cr2, cr1, cr0, i, &x);
            ptaAddPt(pta2, x, i);
        }
        pix1 = pixDisplayPta(NULL, dew->pixs, pta1);
        pixDisplayPta(pix1, pix1, pta2);
        pixRenderHorizEndPoints(pix1, ptal3, ptar3, 0xff000000);
        pixDisplay(pix1, 600, 800);
        pixWrite("/tmp/lept/dewmod/0051.png", pix1, IFF_PNG);
        pixDestroy(&pix1);

        pix1 = pixDisplayPta(NULL, dew->pixs, pta1);
        pixDisplayPta(pix1, pix1, pta2);
        ptalft = ptaTranspose(ptal4);
        ptarft = ptaTranspose(ptar4);
        pixRenderHorizEndPoints(pix1, ptalft, ptarft, 0x0000ff00);
        pixDisplay(pix1, 800, 800);
        pixWrite("/tmp/lept/dewmod/0052.png", pix1, IFF_PNG);
        convertFilesToPdf("/tmp/lept/dewmod", "005", 135, 1.0, 0, 0,
                          "Dewarp Horiz Disparity",
                          "/tmp/lept/dewarp/horiz_disparity.pdf");
        fprintf(stderr, "pdf file: /tmp/lept/dewarp/horiz_disparity.pdf\n");
        pixDestroy(&pix1);
        ptaDestroy(&pta1);
        ptaDestroy(&pta2);
        ptaDestroy(&ptalft);
        ptaDestroy(&ptarft);
    }

        /* Find the x value at the midpoints (in y) of the two vertical lines,
         * ptal4 and ptar4.  These are the reference values for each of the
         * lines.  Then use the difference between the these midpoint
         * values and the actual x coordinates of the lines to represent
         * the horizontal disparity (nald, nard) on the vertical lines
         * for the sampled y values. */
    ptaGetPt(ptal4, ny / 2, &refl, NULL);
    ptaGetPt(ptar4, ny / 2, &refr, NULL);
    nald = numaCreate(ny);
    nard = numaCreate(ny);
    for (i = 0; i < ny; i++) {
        ptaGetPt(ptal4, i, &x, NULL);
        numaAddNumber(nald, refl - x);
        ptaGetPt(ptar4, i, &x, NULL);
        numaAddNumber(nard, refr - x);
    }

        /* Now for each pair of sampled values of the two lines (at the
         * same value of y), do a linear interpolation to generate
         * the horizontal disparity on all sampled points between them.  */
    ptaah = ptaaCreate(ny);
    for (i = 0; i < ny; i++) {
        pta = ptaCreate(2);
        numaGetFValue(nald, i, &val);
        ptaAddPt(pta, refl, val);
        numaGetFValue(nard, i, &val);
        ptaAddPt(pta, refr, val);
        ptaGetLinearLSF(pta, &c1, &c0, NULL);  /* horiz disparity along line */
        ptat = ptaCreate(nx);
        for (j = 0; j < nx; j++) {
            x = j * sampling;
            applyLinearFit(c1, c0, x, &val);
            ptaAddPt(ptat, x, val);
        }
        ptaaAddPta(ptaah, ptat, L_INSERT);
        ptaDestroy(&pta);
    }
    numaDestroy(&nald);
    numaDestroy(&nard);

        /* Save the result in a fpix at the specified subsampling  */
    fpix = fpixCreate(nx, ny);
    for (i = 0; i < ny; i++) {
        for (j = 0; j < nx; j++) {
            ptaaGetPt(ptaah, i, j, NULL, &val);
            fpixSetPixel(fpix, j, i, val);
        }
    }
    dew->samphdispar = fpix;
    dew->hsuccess = 1;

    ptaDestroy(&ptal2);
    ptaDestroy(&ptar2);
    ptaDestroy(&ptal3);
    ptaDestroy(&ptar3);
    ptaDestroy(&ptal4);
    ptaDestroy(&ptar4);
    ptaaDestroy(&ptaah);
    return 0;
}


/*!
 * \brief   dewarpGetTextlineCenters()
 *
 * \param[in]    pixs 1 bpp
 * \param[in]    debugflag 1 for debug output
 * \return  ptaa of center values of textlines
 *
 * <pre>
 * Notes:
 *      (1) This in general does not have a point for each value
 *          of x, because there will be gaps between words.
 *          It doesn't matter because we will fit a quadratic to the
 *          points that we do have.
 * </pre>
 */
PTAA *
dewarpGetTextlineCenters(PIX     *pixs,
                         l_int32  debugflag)
{
char      buf[64];
l_int32   i, w, h, bx, by, nsegs, csize1, csize2;
BOXA     *boxa;
PIX      *pix1, *pix2;
PIXA     *pixa1, *pixa2;
PTA      *pta;
PTAA     *ptaa;

    PROCNAME("dewarpGetTextlineCenters");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PTAA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    pixGetDimensions(pixs, &w, &h, NULL);

    if (debugflag) L_INFO("finding text line centers\n", procName);

        /* Filter to solidify the text lines within the x-height region,
         * and to remove most of the ascenders and descenders.
         * We start with a small vertical opening to remove noise beyond
         * the line that can cause an error in the line end points.
         * The small closing (csize1) is used to bridge the gaps between
         * letters.  The large closing (csize2) bridges the gaps between
         * words; using 1/30 of the page width usually suffices. */
    csize1 = L_MAX(15, w / 80);
    csize2 = L_MAX(40, w / 30);
    snprintf(buf, sizeof(buf), "o1.3 + c%d.1 + o%d.1 + c%d.1",
             csize1, csize1, csize2);
    pix1 = pixMorphSequence(pixs, buf, 0);

        /* Remove the components (e.g., embedded images) that have
         * long vertical runs (>= 50 pixels).  You can't use bounding
         * boxes because connected component b.b. of lines can be quite
         * tall due to slope and curvature.  */
    pix2 = pixMorphSequence(pix1, "e1.50", 0);  /* seed */
    pixSeedfillBinary(pix2, pix2, pix1, 8);  /* tall components */
    pixXor(pix2, pix2, pix1);  /* remove tall */

    if (debugflag) {
        lept_mkdir("lept/dewmod");
        pixWrite("/tmp/lept/dewmod/0011.tif", pix1, IFF_TIFF_G4);
        pixDisplayWithTitle(pix1, 0, 600, "pix1", 1);
        pixWrite("/tmp/lept/dewmod/0012.tif", pix2, IFF_TIFF_G4);
        pixDisplayWithTitle(pix2, 0, 800, "pix2", 1);
    }
    pixDestroy(&pix1);

        /* Get the 8-connected components ... */
    boxa = pixConnComp(pix2, &pixa1, 8);
    pixDestroy(&pix2);
    boxaDestroy(&boxa);
    if (pixaGetCount(pixa1) == 0) {
        pixaDestroy(&pixa1);
        return NULL;
    }

        /* ... and remove the short width and very short height c.c */
    pixa2 = pixaSelectBySize(pixa1, 100, 4, L_SELECT_IF_BOTH,
                                   L_SELECT_IF_GT, NULL);
    if ((nsegs = pixaGetCount(pixa2)) == 0) {
        pixaDestroy(&pixa1);
        pixaDestroy(&pixa2);
        return NULL;
    }
    if (debugflag) {
        pix2 = pixaDisplay(pixa2, w, h);
        pixWrite("/tmp/lept/dewmod/0013.tif", pix2, IFF_TIFF_G4);
        pixDisplayWithTitle(pix2, 0, 1000, "pix2", 1);
        pixDestroy(&pix2);
    }

        /* For each c.c., get the weighted center of each vertical column.
         * The result is a set of points going approximately through
         * the center of the x-height part of the text line.  */
    ptaa = ptaaCreate(nsegs);
    for (i = 0; i < nsegs; i++) {
        pixaGetBoxGeometry(pixa2, i, &bx, &by, NULL, NULL);
        pix2 = pixaGetPix(pixa2, i, L_CLONE);
        pta = dewarpGetMeanVerticals(pix2, bx, by);
        ptaaAddPta(ptaa, pta, L_INSERT);
        pixDestroy(&pix2);
    }
    if (debugflag) {
        pix1 = pixCreateTemplate(pixs);
        pix2 = pixDisplayPtaa(pix1, ptaa);
        pixWrite("/tmp/lept/dewmod/0014.tif", pix2, IFF_PNG);
        pixDisplayWithTitle(pix2, 0, 1200, "pix3", 1);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
    }

    pixaDestroy(&pixa1);
    pixaDestroy(&pixa2);
    return ptaa;
}


/*!
 * \brief   dewarpGetMeanVerticals()
 *
 * \param[in]    pixs 1 bpp, single c.c.
 * \param[in]    x,y location of UL corner of pixs with respect to page image
 * \return  pta (mean y-values in component for each x-value,
 *                   both translated by (x,y
 */
static PTA *
dewarpGetMeanVerticals(PIX     *pixs,
                       l_int32  x,
                       l_int32  y)
{
l_int32    w, h, i, j, wpl, sum, count;
l_uint32  *line, *data;
PTA       *pta;

    PROCNAME("pixGetMeanVerticals");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PTA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);

    pixGetDimensions(pixs, &w, &h, NULL);
    pta = ptaCreate(w);
    data = pixGetData(pixs);
    wpl = pixGetWpl(pixs);
    for (j = 0; j < w; j++) {
        line = data;
        sum = count = 0;
        for (i = 0; i < h; i++) {
            if (GET_DATA_BIT(line, j) == 1) {
                sum += i;
                count += 1;
            }
            line += wpl;
        }
        if (count == 0) continue;
        ptaAddPt(pta, x + j, y + (sum / count));
    }

    return pta;
}


/*!
 * \brief   dewarpRemoveShortLines()
 *
 * \param[in]    pixs 1 bpp
 * \param[in]    ptaas input lines
 * \param[in]    fract minimum fraction of longest line to keep
 * \param[in]    debugflag
 * \return  ptaad containing only lines of sufficient length,
 *                     or NULL on error
 */
PTAA *
dewarpRemoveShortLines(PIX       *pixs,
                       PTAA      *ptaas,
                       l_float32  fract,
                       l_int32    debugflag)
{
l_int32    w, n, i, index, maxlen, len;
l_float32  minx, maxx;
NUMA      *na, *naindex;
PIX       *pix1, *pix2;
PTA       *pta;
PTAA      *ptaad;

    PROCNAME("dewarpRemoveShortLines");

    if (!pixs || pixGetDepth(pixs) != 1)
        return (PTAA *)ERROR_PTR("pixs undefined or not 1 bpp", procName, NULL);
    if (!ptaas)
        return (PTAA *)ERROR_PTR("ptaas undefined", procName, NULL);

    pixGetDimensions(pixs, &w, NULL, NULL);
    n = ptaaGetCount(ptaas);
    ptaad = ptaaCreate(n);
    na = numaCreate(n);
    for (i = 0; i < n; i++) {
        pta = ptaaGetPta(ptaas, i, L_CLONE);
        ptaGetRange(pta, &minx, &maxx, NULL, NULL);
        numaAddNumber(na, maxx - minx + 1);
        ptaDestroy(&pta);
    }

        /* Sort by length and find all that are long enough */
    naindex = numaGetSortIndex(na, L_SORT_DECREASING);
    numaGetIValue(naindex, 0, &index);
    numaGetIValue(na, index, &maxlen);
    if (maxlen < 0.5 * w)
        L_WARNING("lines are relatively short\n", procName);
    pta = ptaaGetPta(ptaas, index, L_CLONE);
    ptaaAddPta(ptaad, pta, L_INSERT);
    for (i = 1; i < n; i++) {
        numaGetIValue(naindex, i, &index);
        numaGetIValue(na, index, &len);
        if (len < fract * maxlen) break;
        pta = ptaaGetPta(ptaas, index, L_CLONE);
        ptaaAddPta(ptaad, pta, L_INSERT);
    }

    if (debugflag) {
        pix1 = pixCopy(NULL, pixs);
        pix2 = pixDisplayPtaa(pix1, ptaad);
        pixDisplayWithTitle(pix2, 0, 200, "pix4", 1);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
    }

    numaDestroy(&na);
    numaDestroy(&naindex);
    return ptaad;
}


/*!
 * \brief   dewarpGetLineEndpoints()
 *
 * \param[in]    h height of pixs
 * \param[in]    ptaa lines
 * \param[out]   pptal left end points of each line
 * \param[out]   pptar right end points of each line
 * \return  0 if OK, 1 on error.
 *
 * <pre>
 * Notes:
 *      (1) We require that the set of end points extends over 45% of the
 *          height of the input image, to insure good coverage and
 *          avoid extrapolating the curvature too far beyond the
 *          actual textlines.  Large extrapolations are particularly
 *          dangerous if used as a reference model.
 *      (2) For fitting the endpoints, x = f(y), we transpose x and y.
 *          Thus all these ptas have x and y swapped!
 * </pre>
 */
static l_int32
dewarpGetLineEndpoints(l_int32  h,
                       PTAA    *ptaa,
                       PTA    **pptal,
                       PTA    **pptar)
{
l_int32    i, n, npt, x, y;
l_float32  miny, maxy, ratio;
PTA       *pta, *ptal, *ptar;

    PROCNAME("dewarpGetLineEndpoints");

    if (!pptal || !pptar)
        return ERROR_INT("&ptal and &ptar not both defined", procName, 1);
    *pptal = *pptar = NULL;
    if (!ptaa)
        return ERROR_INT("ptaa undefined", procName, 1);

    n = ptaaGetCount(ptaa);
    ptal = ptaCreate(n);
    ptar = ptaCreate(n);
    for (i = 0; i < n; i++) {
        pta = ptaaGetPta(ptaa, i, L_CLONE);
        ptaGetIPt(pta, 0, &x, &y);
        ptaAddPt(ptal, y, x);
        npt = ptaGetCount(pta);
        ptaGetIPt(pta, npt - 1, &x, &y);
        ptaAddPt(ptar, y, x);
        ptaDestroy(&pta);
    }

        /* Use the min and max of the y value on the left side. */
    ptaGetRange(ptal, &miny, &maxy, NULL, NULL);
    ratio = (maxy - miny) / (l_float32)h;
    if (ratio < MIN_RATIO_LINES_TO_HEIGHT) {
        L_INFO("ratio lines to height, %f, too small\n", procName, ratio);
        ptaDestroy(&ptal);
        ptaDestroy(&ptar);
        return 1;
    }

    *pptal = ptal;
    *pptar = ptar;
    return 0;
}


/*!
 * \brief   dewarpFindLongLines()
 *
 * \param[in]    ptal left end points of lines
 * \param[in]    ptar right end points of lines
 * \param[in]    minfract minimum allowed fraction of longest line
 * \param[out]   pptald left end points of longest lines
 * \param[out]   pptard right end points of longest lines
 * \return  0 if OK, 1 on error or if there aren't enough long lines
 *
 * <pre>
 * Notes:
 *      (1) We do the following:
 *         (a) Sort the lines from top to bottom, and divide equally
 *             into Top and Bottom sets.
 *         (b) For each set, select the lines that are at least %minfract
 *             of the length of the longest line in the set.
 *             Typically choose %minfract around 0.95.
 *         (c) Accumulate the left and right end points from both
 *             sets into the two returned ptas.
 * </pre>
 */
static l_int32
dewarpFindLongLines(PTA       *ptal,
                    PTA       *ptar,
                    l_float32  minfract,
                    PTA      **pptald,
                    PTA      **pptard)
{
l_int32    i, n, ntop, nt, nb;
l_float32  xl, xr, yl, yr, len, maxtoplen, maxbotlen, tbratio;
NUMA      *nalen, *naindex;
PTA       *ptals, *ptars, *ptald, *ptard;

    PROCNAME("dewarpFindLongLines");

    if (!pptald || !pptard)
        return ERROR_INT("&ptald and &ptard are not both defined", procName, 1);
    *pptald = *pptard = NULL;
    if (!ptal || !ptar)
        return ERROR_INT("ptal and ptar are not both defined", procName, 1);
    if (minfract < 0.8 || minfract > 1.0)
        return ERROR_INT("typ minfract is in [0.90 - 0.95]", procName, 1);

        /* Sort from top to bottom, remembering that x <--> y in the pta */
    n = ptaGetCount(ptal);
    ptaGetSortIndex(ptal, L_SORT_BY_X, L_SORT_INCREASING, &naindex);
    ptals = ptaSortByIndex(ptal, naindex);
    ptars = ptaSortByIndex(ptar, naindex);
    numaDestroy(&naindex);

    ptald = ptaCreate(n);  /* output of long lines */
    ptard = ptaCreate(n);  /* ditto */

        /* Find all lines in the top half that are within typically
         * about 5 percent of the length of the longest line in that set. */
    ntop = n / 2;
    nalen = numaCreate(n / 2);  /* lengths of top lines */
    for (i = 0; i < ntop; i++) {
        ptaGetPt(ptals, i, NULL, &xl);
        ptaGetPt(ptars, i, NULL, &xr);
        numaAddNumber(nalen, xr - xl);
    }
    numaGetMax(nalen, &maxtoplen, NULL);
    L_INFO("Top: maxtoplen = %8.3f\n", procName, maxtoplen);
    for (i = 0; i < ntop; i++) {
        numaGetFValue(nalen, i, &len);
        if (len >= minfract * maxtoplen) {
            ptaGetPt(ptals, i, &yl, &xl);
            ptaAddPt(ptald, yl, xl);
            ptaGetPt(ptars, i, &yr, &xr);
            ptaAddPt(ptard, yr, xr);
        }
    }
    numaDestroy(&nalen);

    nt = ptaGetCount(ptald);
    if (nt < 3) {
        L_INFO("too few long lines at top: %d\n", procName, nt);
        ptaDestroy(&ptals);
        ptaDestroy(&ptars);
        ptaDestroy(&ptald);
        ptaDestroy(&ptard);
        return 1;
    }

        /* Find all lines in the bottom half that are within 8 percent
         * of the length of the longest line in that set. */
    nalen = numaCreate(0);  /* lengths of bottom lines */
    for (i = ntop; i < n; i++) {
        ptaGetPt(ptals, i, NULL, &xl);
        ptaGetPt(ptars, i, NULL, &xr);
        numaAddNumber(nalen, xr - xl);
    }
    numaGetMax(nalen, &maxbotlen, NULL);
    L_INFO("Bottom: maxbotlen = %8.3f\n", procName, maxbotlen);
    for (i = 0; i < n - ntop; i++) {
        numaGetFValue(nalen, i, &len);
        if (len >= minfract * maxbotlen) {
            ptaGetPt(ptals, ntop + i, &yl, &xl);
            ptaAddPt(ptald, yl, xl);
            ptaGetPt(ptars, ntop + i, &yr, &xr);
            ptaAddPt(ptard, yr, xr);
        }
    }
    numaDestroy(&nalen);
    ptaDestroy(&ptals);
    ptaDestroy(&ptars);

        /* Impose another condition: the top and bottom max lengths must
         * be within 15% of each other. */
    tbratio = (maxtoplen >= maxbotlen) ?  maxbotlen / maxtoplen :
        maxtoplen / maxbotlen;
    nb = ptaGetCount(ptald) - nt;
    if (nb < 3 || tbratio < 0.85) {
        if (nb < 3) L_INFO("too few long lines at bottom: %d\n", procName, nb);
        if (tbratio < 0.85) L_INFO("big length diff: ratio = %4.2f\n",
                                   procName, tbratio);
        ptaDestroy(&ptald);
        ptaDestroy(&ptard);
        return 1;
    } else {
        *pptald = ptald;
        *pptard = ptard;
    }
    return 0;
}


/*!
 * \brief   dewarpIsLineCoverageValid()
 *
 * \param[in]    ptaa of validated lines
 * \param[in]    h height of pix
 * \param[out]   ptopline location of top line
 * \param[out]   pbotline location of bottom line
 * \return  1 if coverage is valid, 0 if not or on error.
 *
 * <pre>
 * Notes:
 *      (1) The criterion for valid coverage is:
 *          (a) there must be lines in both halves (top and bottom)
 *              of the image.
 *          (b) the coverage must be at least 40% of the image height
 * </pre>
 */
static l_int32
dewarpIsLineCoverageValid(PTAA     *ptaa,
                          l_int32   h,
                          l_int32  *ptopline,
                          l_int32  *pbotline)
{
l_int32    i, n, both_halves;
l_float32  top, bot, y, fraction;

    PROCNAME("dewarpIsLineCoverageValid");

    if (!ptaa)
        return ERROR_INT("ptaa not defined", procName, 0);
    if ((n = ptaaGetCount(ptaa)) == 0)
        return ERROR_INT("ptaa empty", procName, 0);
    if (h <= 0)
        return ERROR_INT("invalid h", procName, 0);
    if (!ptopline || !pbotline)
        return ERROR_INT("&topline and &botline not defined", procName, 0);

    top = 100000.0;
    bot = 0.0;
    for (i = 0; i < n; i++) {
        ptaaGetPt(ptaa, i, 0, NULL, &y);
        if (y < top) top = y;
        if (y > bot) bot = y;
    }
    *ptopline = (l_int32)top;
    *pbotline = (l_int32)bot;
    both_halves = top < 0.5 * h && bot > 0.5 * h;
    fraction = (bot - top) / h;
    if (both_halves && fraction > 0.40)
        return 1;
    return 0;
}


/*!
 * \brief   dewarpQuadraticLSF()
 *
 * \param[in]    ptad left or right end points of longest lines
 * \param[out]   pa  coeff a of LSF: y = ax^2 + bx + c
 * \param[out]   pb  coeff b of LSF: y = ax^2 + bx + c
 * \param[out]   pc  coeff c of LSF: y = ax^2 + bx + c
 * \param[out]   pmederr [optional] median error
 * \return  0 if OK, 1 on error.
 *
 * <pre>
 * Notes:
 *      (1) This is used for finding the left or right sides of
 *          the text block, computed as a quadratic curve.
 *          Only the longest lines are input, so there are
 *          no outliers.
 *      (2) The ptas for the end points all have x and y swapped.
 * </pre>
 */
static l_int32
dewarpQuadraticLSF(PTA        *ptad,
                   l_float32  *pa,
                   l_float32  *pb,
                   l_float32  *pc,
                   l_float32  *pmederr)
{
l_int32    i, n;
l_float32  x, y, xp, c0, c1, c2;
NUMA      *naerr;

    PROCNAME("dewarpQuadraticLSF");

    if (pmederr) *pmederr = 0.0;
    if (!pa || !pb || !pc)
        return ERROR_INT("not all ptrs are defined", procName, 1);
    *pa = *pb = *pc = 0.0;
    if (!ptad)
        return ERROR_INT("ptad not defined", procName, 1);

        /* Fit to the longest lines */
    ptaGetQuadraticLSF(ptad, &c2, &c1, &c0, NULL);
    *pa = c2;
    *pb = c1;
    *pc = c0;

        /* Optionally, find the median error */
    if (pmederr) {
        n = ptaGetCount(ptad);
        naerr = numaCreate(n);
        for (i = 0; i < n; i++) {
            ptaGetPt(ptad, i, &y, &xp);
            applyQuadraticFit(c2, c1, c0, y, &x);
            numaAddNumber(naerr, L_ABS(x - xp));
        }
        numaGetMedian(naerr, pmederr);
        numaDestroy(&naerr);
    }
    return 0;
}


/*----------------------------------------------------------------------*
 *                      Build line disparity model                     *
 *----------------------------------------------------------------------*/
/*!
 * \brief   dewarpBuildLineModel()
 *
 * \param[in]    dew
 * \param[in]    opensize size of opening to remove perpendicular lines
 * \param[in]    debugfile use NULL to skip writing this
 * \return  0 if OK, 1 if unable to build the model or on error
 *
 * <pre>
 * Notes:
 *      (1) This builds the horizontal and vertical disparity arrays
 *          for an input of ruled lines, typically for calibration.
 *          In book scanning, you could lay the ruled paper over a page.
 *          Then for that page and several below it, you can use the
 *          disparity correction of the line model to dewarp the pages.
 *      (2) The dew has been initialized with the image of ruled lines.
 *          These lines must be continuous, but we do a small amount
 *          of pre-processing here to insure that.
 *      (3) %opensize is typically about 8.  It must be larger than
 *          the thickness of the lines to be extracted.  This is the
 *          default value, which is applied if %opensize \< 3.
 *      (4) Sets vsuccess = 1 and hsuccess = 1 if the vertical and/or
 *          horizontal disparity arrays build.
 *      (5) Similar to dewarpBuildPageModel(), except here the vertical
 *          and horizontal disparity arrays are both built from ruled lines.
 *          See notes there.
 * </pre>
 */
l_int32
dewarpBuildLineModel(L_DEWARP    *dew,
                     l_int32      opensize,
                     const char  *debugfile)
{
char     buf[64];
l_int32  i, j, bx, by, ret, nlines;
BOXA    *boxa;
PIX     *pixs, *pixh, *pixv, *pix, *pix1, *pix2;
PIXA    *pixa1, *pixa2;
PTA     *pta;
PTAA    *ptaa1, *ptaa2;

    PROCNAME("dewarpBuildLineModel");

    if (!dew)
        return ERROR_INT("dew not defined", procName, 1);
    if (opensize < 3) {
        L_WARNING("opensize should be >= 3; setting to 8\n", procName);
        opensize = 8;  /* default */
    }

    dew->debug = (debugfile) ? 1 : 0;
    dew->vsuccess = dew->hsuccess = 0;
    pixs = dew->pixs;
    if (debugfile) {
        lept_rmdir("lept/dewline");  /* erase previous images */
        lept_mkdir("lept/dewline");
        lept_rmdir("lept/dewmod");  /* erase previous images */
        lept_mkdir("lept/dewmod");
        lept_mkdir("lept/dewarp");
        pixDisplayWithTitle(pixs, 0, 0, "pixs", 1);
        pixWrite("/tmp/lept/dewline/001.png", pixs, IFF_PNG);
    }

        /* Extract and solidify the horizontal and vertical lines.  We use
         * the horizontal lines to derive the vertical disparity, and v.v.
         * Both disparities are computed using the vertical disparity
         * algorithm; the horizontal disparity is found from the
         * vertical lines by rotating them clockwise by 90 degrees.
         * On the first pass, we compute the horizontal disparity, from
         * the vertical lines, by rotating them by 90 degrees (so they
         * are horizontal) and computing the vertical disparity on them;
         * we rotate the resulting fpix array for the horizontal disparity
         * back by -90 degrees.  On the second pass, we compute the vertical
         * disparity from the horizontal lines in the usual fashion. */
    snprintf(buf, sizeof(buf), "d1.3 + c%d.1 + o%d.1", opensize - 2, opensize);
    pixh = pixMorphSequence(pixs, buf, 0);  /* horiz */
    snprintf(buf, sizeof(buf), "d3.1 + c1.%d + o1.%d", opensize - 2, opensize);
    pix1 = pixMorphSequence(pixs, buf, 0);  /* vert */
    pixv = pixRotateOrth(pix1, 1); /* vert rotated to horizontal */
    pixa1 = pixaCreate(2);
    pixaAddPix(pixa1, pixv, L_INSERT);  /* get horizontal disparity first */
    pixaAddPix(pixa1, pixh, L_INSERT);
    pixDestroy(&pix1);

        /*--------------------------------------------------------------*/
        /*    Process twice: first for horiz disparity, then for vert   */
        /*--------------------------------------------------------------*/
    for (i = 0; i < 2; i++) {
        pix = pixaGetPix(pixa1, i, L_CLONE);
        pixDisplay(pix, 0, 900);
        boxa = pixConnComp(pix, &pixa2, 8);
        nlines = boxaGetCount(boxa);
        boxaDestroy(&boxa);
        if (nlines < dew->minlines) {
            L_WARNING("only found %d lines\n", procName, nlines);
            pixDestroy(&pix);
            pixaDestroy(&pixa1);
            continue;
        }

            /* Identify the pixels along the skeleton of each line */
        ptaa1 = ptaaCreate(nlines);
        for (j = 0; j < nlines; j++) {
            pixaGetBoxGeometry(pixa2, j, &bx, &by, NULL, NULL);
            pix1 = pixaGetPix(pixa2, j, L_CLONE);
            pta = dewarpGetMeanVerticals(pix1, bx, by);
            ptaaAddPta(ptaa1, pta, L_INSERT);
            pixDestroy(&pix1);
        }
        pixaDestroy(&pixa2);
        if (debugfile) {
            pix1 = pixConvertTo32(pix);
            pix2 = pixDisplayPtaa(pix1, ptaa1);
            snprintf(buf, sizeof(buf), "/tmp/lept/dewline/%03d.png", 2 + 2 * i);
            pixWrite(buf, pix2, IFF_PNG);
            pixDestroy(&pix1);
            pixDestroy(&pix2);
        }

            /* Remove all lines that are not at least 0.75 times the length
             * of the longest line. */
        ptaa2 = dewarpRemoveShortLines(pix, ptaa1, 0.75, DEBUG_SHORT_LINES);
        if (debugfile) {
            pix1 = pixConvertTo32(pix);
            pix2 = pixDisplayPtaa(pix1, ptaa2);
            snprintf(buf, sizeof(buf), "/tmp/lept/dewline/%03d.png", 3 + 2 * i);
            pixWrite(buf, pix2, IFF_PNG);
            pixDestroy(&pix1);
            pixDestroy(&pix2);
        }
        ptaaDestroy(&ptaa1);
        nlines = ptaaGetCount(ptaa2);
        if (nlines < dew->minlines) {
            pixDestroy(&pix);
            ptaaDestroy(&ptaa2);
            L_WARNING("%d lines: too few to build model\n", procName, nlines);
            continue;
        }

            /* Get the sampled 'vertical' disparity from the textline
             * centers.  The disparity array will push pixels vertically
             * so that each line is flat and centered at the y-position
             * of the mid-point. */
        ret = dewarpFindVertDisparity(dew, ptaa2, 1 - i);

            /* If i == 0, move the result to the horizontal disparity,
             * rotating it back by -90 degrees. */
        if (i == 0) {  /* horizontal disparity, really */
            if (ret) {
                L_WARNING("horizontal disparity not built\n", procName);
            } else {
                L_INFO("hsuccess = 1\n", procName);
                dew->samphdispar = fpixRotateOrth(dew->sampvdispar, 3);
                fpixDestroy(&dew->sampvdispar);
                if (debugfile)
                    lept_mv("/tmp/lept/dewarp/vert_disparity.pdf",
                            "lept/dewarp", "horiz_disparity.pdf", NULL);
            }
            dew->hsuccess = dew->vsuccess;
            dew->vsuccess = 0;
        } else {  /* i == 1 */
            if (ret)
                L_WARNING("vertical disparity not built\n", procName);
            else
                L_INFO("vsuccess = 1\n", procName);
        }
        ptaaDestroy(&ptaa2);
        pixDestroy(&pix);
    }
    pixaDestroy(&pixa1);

        /* Debug output */
    if (debugfile) {
        if (dew->vsuccess == 1) {
            dewarpPopulateFullRes(dew, NULL, 0, 0);
            pix1 = fpixRenderContours(dew->fullvdispar, 3.0, 0.15);
            pixWrite("/tmp/lept/dewline/006.png", pix1, IFF_PNG);
            pixDisplay(pix1, 1000, 0);
            pixDestroy(&pix1);
        }
        if (dew->hsuccess == 1) {
            pix1 = fpixRenderContours(dew->fullhdispar, 3.0, 0.15);
            pixWrite("/tmp/lept/dewline/007.png", pix1, IFF_PNG);
            pixDisplay(pix1, 1000, 0);
            pixDestroy(&pix1);
        }
        convertFilesToPdf("/tmp/lept/dewline", NULL, 135, 1.0, 0, 0,
                          "Dewarp Build Line Model", debugfile);
        fprintf(stderr, "pdf file: %s\n", debugfile);
    }

    return 0;
}


/*----------------------------------------------------------------------*
 *                         Query model status                           *
 *----------------------------------------------------------------------*/
/*!
 * \brief   dewarpaModelStatus()
 *
 * \param[in]    dewa
 * \param[in]    pageno
 * \param[out]   pvsuccess [optional] 1 on success
 * \param[out]   phsuccess [optional] 1 on success
 * \return  0 if OK, 1 on error
 *
 * <pre>
 * Notes:
 *      (1) This tests if a model has been built, not if it is valid.
 * </pre>
 */
l_int32
dewarpaModelStatus(L_DEWARPA  *dewa,
                   l_int32     pageno,
                   l_int32    *pvsuccess,
                   l_int32    *phsuccess)
{
L_DEWARP  *dew;

    PROCNAME("dewarpaModelStatus");

    if (pvsuccess) *pvsuccess = 0;
    if (phsuccess) *phsuccess = 0;
    if (!dewa)
        return ERROR_INT("dewa not defined", procName, 1);

    if ((dew = dewarpaGetDewarp(dewa, pageno)) == NULL)
        return ERROR_INT("dew not retrieved", procName, 1);
    if (pvsuccess) *pvsuccess = dew->vsuccess;
    if (phsuccess) *phsuccess = dew->hsuccess;
    return 0;
}


/*----------------------------------------------------------------------*
 *                          Rendering helpers                           *
 *----------------------------------------------------------------------*/
/*!
 * \brief   pixRenderMidYs()
 *
 * \param[in]    pixs 32 bpp
 * \param[in]    namidys y location of reference lines for vertical disparity
 * \param[in]    linew width of rendered line; typ 2
 * \return  0 if OK, 1 on error
 */
static l_int32
pixRenderMidYs(PIX     *pixs,
               NUMA    *namidys,
               l_int32  linew)
{
l_int32   i, n, w, yval, rval, gval, bval;
PIXCMAP  *cmap;

    PROCNAME("pixRenderMidYs");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!namidys)
        return ERROR_INT("namidys not defined", procName, 1);

    w = pixGetWidth(pixs);
    n = numaGetCount(namidys);
    cmap = pixcmapCreateRandom(8, 0, 0);
    for (i = 0; i < n; i++) {
        pixcmapGetColor(cmap, i % 256, &rval, &gval, &bval);
        numaGetIValue(namidys, i, &yval);
        pixRenderLineArb(pixs, 0, yval, w, yval, linew, rval, gval, bval);
    }
    pixcmapDestroy(&cmap);
    return 0;
}


/*!
 * \brief   pixRenderHorizEndPoints()
 *
 * \param[in]    pixs 32 bpp
 * \param[in]    ptal left side line end points
 * \param[in]    ptar right side line end points
 * \param[in]    color 0xrrggbb00
 * \return  0 if OK, 1 on error
 */
static l_int32
pixRenderHorizEndPoints(PIX      *pixs,
                        PTA      *ptal,
                        PTA      *ptar,
                        l_uint32  color)
{
PIX      *pixcirc;
PTA      *ptalt, *ptart, *ptacirc;

    PROCNAME("pixRenderHorizEndPoints");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!ptal || !ptar)
        return ERROR_INT("ptal and ptar not both defined", procName, 1);

    ptacirc = generatePtaFilledCircle(5);
    pixcirc = pixGenerateFromPta(ptacirc, 11, 11);
    ptalt = ptaTranspose(ptal);
    ptart = ptaTranspose(ptar);

    pixDisplayPtaPattern(pixs, pixs, ptalt, pixcirc, 5, 5, color);
    pixDisplayPtaPattern(pixs, pixs, ptart, pixcirc, 5, 5, color);
    ptaDestroy(&ptacirc);
    ptaDestroy(&ptalt);
    ptaDestroy(&ptart);
    pixDestroy(&pixcirc);
    return 0;
}
