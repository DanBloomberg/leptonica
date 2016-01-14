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
 *     gplot.h
 *
 *          Data structures used to generate gnuplot files
 *
 */

#ifndef GPLOT_H_INCLUDED
#define GPLOT_H_INCLUDED

enum GPLOT_MODE  {Y_VS_I = 1, Y_VS_X};

#define  NUM_GPLOT_STYLES      5
enum GPLOT_STYLE  {GPLOT_LINES, GPLOT_POINTS, GPLOT_IMPULSES,
                   GPLOT_LINESPOINTS, GPLOT_DOTS};

#define  NUM_GPLOT_OUTPUTS     5
enum GPLOT_OUTPUT  {GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11, GPLOT_LATEX};

extern const char  *gplotstylenames[];  /* used in gnuplot cmd file */
extern const char  *gplotfilestyles[];  /* used in simple file input */
extern const char  *gplotfileoutputs[]; /* used in simple file input */


struct GPlot
{
    char          *rootname;   /* for cmd, data, output         */
    char          *cmdname;    /* command file name             */
    char          *outname;    /* output file name              */
    l_int32        plotmode;   /* GPLOT_MODE values             */
    l_int32        outformat;  /* GPLOT_OUTPUT values           */
    l_int32        nplots;     /* current number of plots       */
    char          *title;      /* optional                      */
    char          *xlabel;     /* optional x axis label         */
    char          *ylabel;     /* optional y axis label         */
};
typedef struct GPlot  GPLOT;


#endif /* GPLOT_H_INCLUDED */
