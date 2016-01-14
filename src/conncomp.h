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
 *   conncomp.h
 *
 *        The struct FillSeg is used by the Heckbert seedfill
 *        algorithm to hold information about image segments
 *        that are waiting to be investigated.  We use two
 *        Stacks, one to hold the FillSegs in use, and an
 *        auxiliary Stack as a reservoir to hold FillSegs
 *        for re-use.
 */

#ifndef CONNCOMP_H_INCLUDED
#define CONNCOMP_H_INCLUDED


struct FillSeg
{
    l_int32    xleft;    /* left edge of run */
    l_int32    xright;   /* right edge of run */
    l_int32    y;        /* run y  */
    l_int32    dy;       /* parent segment direction: 1 above, -1 below) */
};
typedef struct FillSeg    FILLSEG;


enum {
    L_HORIZONTAL_RUNS = 0,   /* determine horizontal runs */
    L_VERTICAL_RUNS = 1      /* determine vertical runs */
};


#endif /* CONNCOMP_H_INCLUDED */
