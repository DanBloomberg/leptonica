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
 *      Page orientation detection (pure rotation by 90 degree increments):
 *
 *          l_int32      pixOrientDetect()
 *          l_int32      pixUpDownDetect()
 *
 *          l_int32      pixOrientDetectDwa()
 *          l_int32      pixUpDownDetectDwa()
 *
 *      Page mirror detection (flip 180 degrees about line in plane of image):
 *
 *          l_int32      pixMirrorDetect()
 *          l_int32      pixMirrorDetectDwa()
 *
 *  ===================================================================    
 *
 *  Page transformation detection:
 *
 *  Once a page is deskewed, there are 8 possible states that it
 *  can be in, shown symbolically below.  Suppose state 0 is correct.
 *      
 *      0: correct   1         2         3
 *      +------+   +------+   +------+   +------+      
 *      | **** |   | *    |   | **** |   |    * |       
 *      | *    |   | *    |   |    * |   |    * |      
 *      | *    |   | **** |   |    * |   | **** |      
 *      +------+   +------+   +------+   +------+      
 *
 *          4         5          6          7
 *      +-----+    +-----+    +-----+    +-----+   
 *      | *** |    |   * |    | *** |    | *   |   
 *      |   * |    |   * |    | *   |    | *   |   
 *      |   * |    |   * |    | *   |    | *   |   
 *      |   * |    | *** |    | *   |    | *** |   
 *      +-----+    +-----+    +-----+    +-----+   
 *
 *  Each of the other seven can be derived from state 0 by applying some
 *  combination of a 90 degree clockwise rotation, a flip about
 *  a horizontal line, and a flip about a vertical line,
 *  all abbreviated as:
 *      R = Rotation (about a line perpendicular to the image)
 *      H = Horizontal flip (about a vertical line in the plane of the image)
 *      V = Vertical flip (about a horizontal line in the plane of the image)
 *
 *  We get these transformations:
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
 *  Note that in four of these, the sum of H and V is 1 (odd).
 *  For these four, we have a change in parity (handedness) of
 *  the image, and the transformation cannot be performed by
 *  rotation about a vertical line out of the page.   Under
 *  rotation R, the set of 8 transformations decomposes into
 *  two subgroups linking {0, 3, 4, 7} and {1, 2, 5, 6} independently.
 *
 *  pixOrientDetect*() tests for a pure rotation (0, 90, 180, 270 degrees).
 *  It doesn't change parity.
 *
 *  pixMirrorDetect*() tests for a horizontal flip about the vertical axis.
 *  It changes parity.
 *
 *  The landscape/portrait rotation can be detected in two ways:
 *
 *    (1) Compute the deskew confidence for an image segment,
 *        both as is and rotated 90 degrees  (see skew.c).
 *
 *    (2) Compute the ascender/descender signal for the image,
 *        both as is and rotated 90 degrees  (implemented here).
 *
 *  The ascender/descender signal is useful for determining text
 *  orientation in Roman alphabets because the incidence of letters
 *  with straight-line ascenders (b, d, h, k, l, <t>) outnumber
 *  those with descenders (<g>, p, q).  The letters <t> and <g>
 *  will respond variably to the filter, depending on the type face.
 *
 *  What about the mirror image situations?  These aren't common
 *  unless you're dealing with film, or some strange situation.
 *  But you can reliably test if the image has undergone a
 *  parity-changing flip once about some axis in the plane
 *  of the image, using pixMirrorDetect*().  This works by
 *  counting the number of characters with ascenders that
 *  stick out to the left and right of the ascender.  Characters
 *  that are not mirror flipped are more likely to extend to the
 *  right (b, h, k) than to the left (d).  Of course, that is for
 *  text that is rightside-up.  So before you apply the mirror
 *  test, it is necessary to insure that the text has the ascenders
 *  going up, and not down or to the left or right!
 *
 *  The set of operations you actually use depends on your prior knowledge:
 *
 *  (1) If the page is known to be either rightside-up or upside-down, use
 *      either pixOrientDetect() with pleftconf = NULL, or pixUpDownDetect().
 *
 *  (2) If any of the four orientations are possible, use pixOrientDetect()
 *
 *  (3) If the text is horizontal and rightside-up, the only remaining
 *      degree of freedom is a left-right mirror flip.
 *      Use pixMirrorDetect().
 *
 *  We summarize the full orientation and mirror flip detection process:
 *
 *  (1) First determine which of the four 90 degree rotations
 *      causes the text to be rightside-up.  This can be done
 *      with either skew confidence or the pixOrientDetect*()
 *      signals.  For the latter, see the table for pixOrientDetect().
 *
 *  (2) Then, with ascenders pointing up, apply pixMirrorDetect*().
 *      In the normal situation the confidence confidence will be
 *      large and positive.  However, if mirror flipped, the
 *      confidence will be large and negative.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "allheaders.h"

    /* Sels for pixOrientDetect() and pixMirrorDetect() */
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

#ifndef NO_CONSOLE_IO
#define  DEBUG_ORIENT    0
#define  DEBUG_UPDOWN    0
#define  DEBUG_MIRROR    0
#endif   /* ~NO_CONSOLE_IO */


/*----------------------------------------------------------------*
 *         Orientation detection (four 90 degree angles)          *
 *                      Rasterop implementation                   *
 *----------------------------------------------------------------*/
/*!
 *  pixOrientDetect()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &upconf (<optional return> ; may be null)
 *              &leftconf (<optional return> ; may be null)
 *              mincount (min number of up + down; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See "Measuring document image skew and orientation"
 *          Dan S. Bloomberg, Gary E. Kopec and Lakshmi Dasari
 *          IS&T/SPIE EI'95, Conference 2422: Document Recognition II
 *          pp 302-316, Feb 6-7, 1995, San Jose, CA
 *      (2) upconf is the normalized difference between up ascenders
 *          and down ascenders.  The image is analyzed without rotation
 *          for being rightside-up or upside-down.  Set &upconf to null
 *          to skip this operation.
 *      (3) leftconf is the normalized difference between up ascenders
 *          and down ascenders in the image after it has been
 *          rotated 90 degrees clockwise.  With that rotation, ascenders
 *          projecting to the left in the source image will project up
 *          in the rotated image.  We compute this by rotating 90 degrees
 *          clockwise and testing for up and down ascenders.  Set
 *          &leftconf to null to skip this operation.
 *      (4) Note that upconf and leftconf are not linear measures of
 *          confidence, e.g., in a range between 0 and 100.  They
 *          measure how far you are out on the tail of a (presumably)
 *          normal distribution.  For example, a confidence of 10 means
 *          that it is nearly certain that the difference did not
 *          happen at random.  However, these values must be interpreted
 *          cautiously, taking into consideration the estimated prior
 *          for a particular orientation or mirror flip.   The up-down
 *          signal is very strong if applied to text with ascenders
 *          up and down, and relatively weak for text at 90 degrees,
 *          but even at 90 degrees, the difference can look significant.
 *          For example, suppose the ascenders are oriented horizontally,
 *          but the test is done vertically.  Then upconf can be < -6.0,
 *          suggesting the text may be upside-down.  However, if instead
 *          the test were done horizontally, leftconf will be very much
 *          larger (in absolute value), giving the correct orientation.
 *      (5) If you compute both upconf and leftconf, and there is
 *          sufficient signal, the following table determines the
 *          cw angle necessary to rotate pixs so that the text is 
 *          rightside-up:
 *             0 deg :           upconf >> 1,    abs(upconf) >> abs(leftconf)
 *             90 deg :          leftconf >> 1,  abs(leftconf) >> abs(upconf)
 *             180 deg :         upconf << -1,   abs(upconf) >> abs(leftconf)
 *             270 deg :         leftconf << -1, abs(leftconf) >> abs(upconf)
 *      (6) Uses rasterop implementation of HMT.
 */
l_int32
pixOrientDetect(PIX        *pixs,
                l_float32  *pupconf,
                l_float32  *pleftconf,
                l_int32     mincount)
{
PIX       *pixt;

    PROCNAME("pixOrientDetect");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not 1 bpp", procName, 1);
    if (!pupconf && !pleftconf)
        return ERROR_INT("nothing to do", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_UP_DOWN;

    if (pupconf)
        pixUpDownDetect(pixs, pupconf, mincount);
    if (pleftconf) {
        pixt = pixRotate90(pixs, 1);
        pixUpDownDetect(pixt, pleftconf, mincount);
        pixDestroy(&pixt);
    }

#if DEBUG_ORIENT
        /* This uses two thresholds -- 10.0 on the magnitude of the
         * larger confidence, and 2.5 on the ratio between the
         * larger and smaller |confidence| -- to determine the
         * orientation in the general case */
    if (pupconf && pleftconf)
        fprintf(stderr, "upconf = %f, leftconf = %f\n", *pupconf, *pleftconf);
    if (*pupconf > 10.0 && (L_ABS(*pupconf) > 2.5 * L_ABS(*pleftconf)))
        fprintf(stderr, "Text is rightside-up\n");
    else if (*pleftconf > 10.0 &&
             (L_ABS(*pleftconf) > 2.5 * L_ABS(*pupconf)))
        fprintf(stderr, "Text is rotated 90 deg ccw\n");
    else if (*pupconf < -10.0 && (L_ABS(*pupconf) > 2.5 * L_ABS(*pleftconf)))
        fprintf(stderr, "Text is upside-down\n");
    else if (*pleftconf < -10.0 &&
             (L_ABS(*pleftconf) > 2.5 * L_ABS(*pupconf)))
        fprintf(stderr, "Text is rotated 90 deg ccw\n");
    else
        fprintf(stderr, "Confidence is low; no determination is made\n");
#endif   /* DEBUG_ORIENT */

    return 0;
}


/*!
 *  pixUpDownDetect()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &conf (<return> confidence that text is rightside-up)
 *              mincount (min number of up + down; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) See pixOrientDetect() for details.
 *      (2) conf is the normalized difference between the number of
 *          detected up and down ascenders, assuming that the text
 *          is either rightside-up or upside-down and not rotated
 *          at a 90 degree angle.
 */
l_int32
pixUpDownDetect(PIX        *pixs,
                l_float32  *pconf,
                l_int32     mincount)
{
l_int32    count1, count2, count3, count4;
l_float32  nup, ndown;
PIX       *pixt0, *pixt1, *pixt2, *pixt3, *pixt4;
SEL       *sel1, *sel2, *sel3, *sel4;

    PROCNAME("pixUpDownDetect");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pconf)
        return ERROR_INT("&conf not defined", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_UP_DOWN;

    sel1 = selCreateFromString(textsel1, 5, 6, NULL);
    sel2 = selCreateFromString(textsel2, 5, 6, NULL);
    sel3 = selCreateFromString(textsel3, 5, 6, NULL);
    sel4 = selCreateFromString(textsel4, 5, 6, NULL);

    pixt0 = pixMorphSequence(pixs, "d20.1", 0);

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

    nup = (l_float32)(count1 + count2);
    ndown = (l_float32)(count3 + count4);

    *pconf = 0.0;
    if (nup + ndown > mincount)
        *pconf = 2. * ((nup - ndown) / sqrt(nup + ndown));

#if DEBUG_UPDOWN
    fprintf(stderr, "nup = %f, ndown = %f\n", nup, ndown);
    if (*pconf > 10.0)
        fprintf(stderr, "Text is rightside-up\n");
    if (*pconf < -10.0)
        fprintf(stderr, "Text is upside-down\n");
#endif   /* DEBUG_UPDOWN */

    return 0;
}


/*----------------------------------------------------------------*
 *         Orientation detection (four 90 degree angles)          *
 *                         DWA implementation                     *
 *----------------------------------------------------------------*/
/*!
 *  pixOrientDetectDwa()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &upconf (<optional return> ; may be null)
 *              &leftconf (<optional return> ; may be null)
 *              mincount (min number of up + down; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) Same interface as for pixOrientDetect().  See notes
 *          there for usage.
 *      (2) Uses auto-gen'd code for the Sels defined at the
 *          top of this file, with some renaming of functions.
 *          The auto-gen'd code is in fliphmtgen.c, and can
 *          be generated by a simple executable; see prog/flipselgen.c.
 *      (3) This runs about 2.5 times faster than the pixOrientDetect().
 */
l_int32
pixOrientDetectDwa(PIX        *pixs,
                   l_float32  *pupconf,
                   l_float32  *pleftconf,
                   l_int32     mincount)
{
PIX       *pixt;

    PROCNAME("pixOrientDetectDwa");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (pixGetDepth(pixs) != 1)
        return ERROR_INT("pixs not 1 bpp", procName, 1);
    if (!pupconf && !pleftconf)
        return ERROR_INT("nothing to do", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_UP_DOWN;

    if (pupconf)
        pixUpDownDetectDwa(pixs, pupconf, mincount);
    if (pleftconf) {
        pixt = pixRotate90(pixs, 1);
        pixUpDownDetectDwa(pixt, pleftconf, mincount);
        pixDestroy(&pixt);
    }

    return 0;
}


/*!
 *  pixUpDownDetectDwa()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &conf (<return> confidence that text is rightside-up)
 *              mincount (min number of up + down; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) DWA version of pixUpDownDetect(); see pixOrientDetect().
 *      (2) conf is the normalized difference between the number of
 *          detected up and down ascenders, assuming that the text
 *          is either rightside-up or upside-down and not rotated
 *          at a 90 degree angle.
 */
l_int32
pixUpDownDetectDwa(PIX        *pixs,
                   l_float32  *pconf,
                   l_int32     mincount)
{
l_int32    count1, count2, count3, count4;
l_float32  nup, ndown;
PIX       *pixt0, *pixt1, *pixt2, *pixt3, *pixt4;

    PROCNAME("pixUpDownDetectDwa");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pconf)
        return ERROR_INT("&conf not defined", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_UP_DOWN;

    pixt0 = pixAddBorderGeneral(pixs, ADDED_BORDER, ADDED_BORDER,
                                ADDED_BORDER, ADDED_BORDER, 0);
    pixFMorphopGen_1(pixt0, pixt0, L_MORPH_DILATE, "sel_20h");

    pixt1 = pixFlipFHMTGen(NULL, pixt0, "flipsel1");
    pixCountPixels(pixt1, &count1, NULL);
    pixDestroy(&pixt1);

    pixt2 = pixFlipFHMTGen(NULL, pixt0, "flipsel2");
    pixCountPixels(pixt2, &count2, NULL);
    pixDestroy(&pixt2);

    pixt3 = pixFlipFHMTGen(NULL, pixt0, "flipsel3");
    pixCountPixels(pixt3, &count3, NULL);
    pixDestroy(&pixt3);

    pixt4 = pixFlipFHMTGen(NULL, pixt0, "flipsel4");
    pixCountPixels(pixt4, &count4, NULL);
    pixDestroy(&pixt4);

    pixDestroy(&pixt0);
    nup = (l_float32)(count1 + count2);
    ndown = (l_float32)(count3 + count4);

    *pconf = 0.0;
    if (nup + ndown > mincount)
        *pconf = 2. * ((nup - ndown) / sqrt(nup + ndown));

    return 0;
}


/*----------------------------------------------------------------*
 *                     Left-right mirror detection                *
 *                       Rasterop implementation                  *
 *----------------------------------------------------------------*/
/*!
 *  pixMirrorDetect()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &conf (<return> confidence that text is not LR mirror reversed)
 *              mincount (min number of left + right; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) For this test, it is necessary that the text is horizontally
 *          oriented, with ascenders going up.
 *      (2) conf is the normalized difference between the number of
 *          right and left facing characters with ascenders.
 *          Left-facing are {d}; right-facing are {b, h, k}.
 *      (3) A large positive conf value indicates normal text, whereas
 *          a large negative conf value means the page is mirror reversed.
 */
l_int32
pixMirrorDetect(PIX        *pixs,
                l_float32  *pconf,
                l_int32     mincount)
{
l_int32    count1, count2;
l_float32  nleft, nright;
PIX       *pixt0, *pixt1, *pixt2;
SEL       *sel1, *sel2;

    PROCNAME("pixMirrorDetect");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pconf)
        return ERROR_INT("&conf not defined", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_LEFT_RIGHT;

    sel1 = selCreateFromString(textsel1, 5, 6, NULL);
    sel2 = selCreateFromString(textsel2, 5, 6, NULL);

        /* Small vertical closing to fill holes in characters */
    pixt0 = pixMorphSequence(pixs, "c1.6", 0);

    pixt1 = pixHMT(NULL, pixt0, sel1);
    pixCountPixels(pixt1, &count1, NULL);  /* right-facing */
    pixDestroy(&pixt1);

    pixt2 = pixHMT(NULL, pixt0, sel2);
    pixCountPixels(pixt2, &count2, NULL);  /* left-facing */
    pixDestroy(&pixt2);

    nright = (l_float32)count1;
    nleft = (l_float32)count2;
    pixDestroy(&pixt0);
    selDestroy(&sel1);
    selDestroy(&sel2);

    *pconf = 0.0;
    if (nright + nleft > mincount)
        *pconf = 2. * ((nright - nleft) / sqrt(nright + nleft));

#if DEBUG_MIRROR
    fprintf(stderr, "nright = %f, nleft = %f\n", nright, nleft);
    if (*pconf > 10.0)
        fprintf(stderr, "Text is not mirror reversed\n");
    if (*pconf < -10.0)
        fprintf(stderr, "Text is mirror reversed\n");
#endif  /* DEBUG_MIRROR */

    return 0;
}


/*----------------------------------------------------------------*
 *                     Left-right mirror detection                *
 *                          DWA implementation                    *
 *----------------------------------------------------------------*/
/*!
 *  pixMirrorDetectDwa()
 *
 *      Input:  pixs (1 bpp, deskewed, English text)
 *              &conf (<return> confidence that text is not LR mirror reversed)
 *              mincount (min number of left + right; use 0 for default)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) We assume the text is horizontally oriented, with 
 *          ascenders going up.
 *      (2) conf is the normalized difference between right and left
 *          facing characters with ascenders.  Left-facing are {d};
 *          right-facing are {b, h, k}.
 */
l_int32
pixMirrorDetectDwa(PIX        *pixs,
                   l_float32  *pconf,
                   l_int32     mincount)
{
l_int32    count1, count2;
l_float32  nleft, nright;
PIX       *pixt, *pixt0, *pixt1, *pixt2;

    PROCNAME("pixMirrorDetectDwa");

    if (!pixs)
        return ERROR_INT("pixs not defined", procName, 1);
    if (!pconf)
        return ERROR_INT("&conf not defined", procName, 1);
    if (mincount == 0)
        mincount = DEFAULT_MIN_COUNT_LEFT_RIGHT;

    pixt = pixAddBorderGeneral(pixs, ADDED_BORDER, ADDED_BORDER,
                                ADDED_BORDER, ADDED_BORDER, 0);
    pixt0 = pixMorphSequenceDwa(pixt, "c1.6", 0);
    pixDestroy(&pixt);

    pixt1 = pixFlipFHMTGen(NULL, pixt0, "flipsel1");
    pixCountPixels(pixt1, &count1, NULL);  /* right-facing */
    pixDestroy(&pixt1);

    pixt2 = pixFlipFHMTGen(NULL, pixt0, "flipsel2");
    pixCountPixels(pixt2, &count2, NULL);  /* left-facing */
    pixDestroy(&pixt2);

    pixDestroy(&pixt0);
    nright = (l_float32)count1;
    nleft = (l_float32)count2;

    *pconf = 0.0;
    if (nright + nleft > mincount)
        *pconf = 2. * ((nright - nleft) / sqrt(nright + nleft));

    return 0;
}


