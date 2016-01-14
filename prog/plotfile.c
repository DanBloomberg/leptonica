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
 * plotfile.c
 *
 *	      plotfile  filein  rootout  outputtype
 *
 *                filein: input data file (e.g., gplotdata.example)
 *                rootout: root name of generated gnuplot data and cmd files
 *                outputtype: one of {PNG, PS, EPS, X11}
 *
 *     plotfile is a program that allows you to use gnuplot with
 *     input from a single data file, in a simplified format.
 *
 *     See the file gplotdata.example to see how to enter data for
 *     multiple plots in different plot styles.
 *  
 *     The format specifications for the input file are as follows:
 *
 *     ------------------
 *     Header Information
 *     ------------------
 *
 *     All lines starting with '%' are comments and are ignored
 *     All blank lines are ignored
 *     The header information is optional.  It must be followed
 *     by a line beginning with '&' to signal the beginning of the data.
 *
 *     If header plotinfo lines exist, the first three must be
 *            title
 *            xaxis label
 *            yaxis label
 *     These all default to null strings.
 *
 *     Optionally after that, the individual plots can be
 *     labelled and the drawing method specified.  This is
 *     done by specifying:
 *            number of plots
 *            title for plot 1
 *            drawing style for plot 1
 *            title for plot 1
 *            drawing style for plot 1
 *            ...
 *
 *     Default for this is to have the individual plots untitled
 *     and to use LINES for each plot.  The drawing styles allowed
 *     must be chosen from the set:
 *            {LINES, POINTS, IMPULSES, LINESPOINTS, DOTS}
 *
 *     ------------------
 *     Data Information
 *     ------------------
 *
 *     Use '&' at the beginning of a line as a control character.
 *     The first '&' signals that the data is to start
 *     Subsequent '&' are separators between data for different plots. 
 *     This way, multiple plots can be drawn on the same output graph.
 *
 *     All blank lines or lines starting with '%' are ignored.
 *
 *     Data in file can have either one or two numbers per line.
 *     If there is one number, it is assumed to be the "y" value,
 *     and the "x" value is taken to be the line number, starting with 1.
 *
 *     All characters other than the digits 0-9, '+', '-' and '.' are ignored,
 *     (with the exception that '%' and '&' have a special meaning when
 *     at the beginning of a line).
 */


#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein, *rootout, *outputtype;
static char  mainName[] = "plotfile";

    if (argc != 4)
	exit(ERROR_INT(" Syntax:  plotfile filein rootout outputtype",
	     mainName, 1));

    filein = argv[1];
    rootout = argv[2];
    outputtype = argv[3];

    gplotFromFile(filein, rootout, outputtype);

    exit(0);
}

