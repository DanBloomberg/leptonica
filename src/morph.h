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
 *  use in structuring elements
 */
enum {
    SEL_DONT_CARE  = 0,
    SEL_HIT        = 1,
    SEL_MISS       = 2
};

/*
 *  use in granulometry
 */
enum {
    L_RUN_OFF = 0,
    L_RUN_ON  = 1
};

/*
 *  use in grayscale morphology and granulometry
 */
enum {
    L_HORIZ      = 1,
    L_VERT       = 2
};

enum {
    MORPH_DILATION   = 1,
    MORPH_EROSION    = 2
};

/*
 * standard size of border added around images for special processing
 */
static const l_int32  ADDED_BORDER = 32;   /* pixels, not bits */

/*
 *  use in rescaling grayscale images
 */
enum {
    L_LINEAR_SCALE  = 1,
    L_LOG_SCALE     = 2
};

/*
 *  use in grayscale tophat
 */
enum {
    TOPHAT_WHITE = 0,
    TOPHAT_BLACK = 1
};

/*
 *  use in grayscale arithmetic
 */
enum {
    ARITH_ADD       = 1,
    ARITH_SUBTRACT  = 2
};

/*
 *  use in grayscale min/max
 */
enum {
    L_CHOOSE_MIN = 1,
    L_CHOOSE_MAX = 2
};


/*-------------------------------------------------------------------------*
 *	                        Sel and Sel array                          *
 *-------------------------------------------------------------------------*/
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
