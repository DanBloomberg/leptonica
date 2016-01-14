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



#ifndef MORPH_H_INCLUDED
#define MORPH_H_INCLUDED

/* 
 *  morph.h
 */

/*
 *  Two types of boundary condition for erosion.
 *  The global variable MORPH_BC takes on one of these two values.
 *  See notes in morph.c for usage.
 */
enum {
    SYMMETRIC_MORPH_BC = 0,
    ASYMMETRIC_MORPH_BC = 1
};

/*
 *  Use in structuring elements
 */
enum {
    SEL_DONT_CARE  = 0,
    SEL_HIT        = 1,
    SEL_MISS       = 2
};

/*
 *  Use in granulometry
 */
enum {
    L_RUN_OFF = 0,
    L_RUN_ON  = 1
};

/*
 *  Use in grayscale morphology, granulometry and composable Sels
 */
enum {
    L_HORIZ      = 1,
    L_VERT       = 2
};

enum {
    L_MORPH_DILATE    = 1,
    L_MORPH_ERODE     = 2,
    L_MORPH_OPEN      = 3,
    L_MORPH_CLOSE     = 4
};

/*
 *  Standard size of border added around images for special processing
 */
static const l_int32  ADDED_BORDER = 32;   /* pixels, not bits */

/*
 *  Use in rescaling grayscale images
 */
enum {
    L_LINEAR_SCALE  = 1,
    L_LOG_SCALE     = 2
};

/*
 *  Use in grayscale tophat
 */
enum {
    L_TOPHAT_WHITE = 0,
    L_TOPHAT_BLACK = 1
};

/*
 *  Use in arithmetic on 2 grayscale images or on 2 numas
 */
enum {
    L_ARITH_ADD       = 1,
    L_ARITH_SUBTRACT  = 2,
    L_ARITH_MULTIPLY  = 3,   /* on numas only               */
    L_ARITH_DIVIDE    = 4    /* on numas only               */
};

/*
 *  Use in grayscale min/max
 */
enum {
    L_CHOOSE_MIN = 1,
    L_CHOOSE_MAX = 2
};

/*
 *  Use in comparison of two images
 */
enum {
    L_COMPARE_XOR = 1,
    L_COMPARE_SUBTRACT = 2,
    L_COMPARE_ABS_DIFF = 3
};

/*
 *  Use in determining color difference from gray
 */
enum {
    L_MAX_DIFF_FROM_AVERAGE_2 = 1,
    L_MAX_MIN_DIFF_FROM_2 = 2
};


/*-------------------------------------------------------------------------*
 *                                Sel and Sel array                          *
 *-------------------------------------------------------------------------*/
#define  SEL_VERSION_NUMBER    1

struct Sel
{
    l_int32       sy;          /* sel height                               */
    l_int32       sx;          /* sel width                                */
    l_int32       cy;          /* y location of sel origin                 */
    l_int32       cx;          /* x location of sel origin                 */
    l_int32     **data;        /* {0,1,2}; data[i][j] in [row][col] order  */
    char         *name;        /* used to find sel by name                 */
};
typedef struct Sel SEL;

struct Sela
{
    l_int32          n;         /* number of sel actually stored           */
    l_int32          nalloc;    /* size of allocated ptr array             */
    struct Sel     **sel;       /* sel ptr array                           */
};
typedef struct Sela SELA;


#endif  /* MORPH_H_INCLUDED */
