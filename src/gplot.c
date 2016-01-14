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
 *  gplot.c
 *                     
 *     Basic plotting functions
 *          GPLOT      *gplotCreate()
 *          l_int32     gplotAddPlot()
 *          l_int32     gplotMakeOutput()
 *          void        gplotDestroy()
 *
 *     Quick and dirty plots
 *          l_int32     gplotSimple1()
 *          l_int32     gplotSimple2()
 *          l_int32     gplotSimpleN()
 *
 *     Plotting from an input text file
 *          l_int32     gplotFromFile()
 *
 *
 *     Utility for programmatic plotting using gnuplot 7.3.2 or later
 *     Enabled:
 *         - output to png (color), ps (mono), x11 (color), latex (mono)
 *         - optional title for graph
 *         - optional x and y axis labels
 *         - multiple plots on one frame
 *         - optional title for each plot on the frame
 *         - choice of 5 plot styles for each plot
 *         - choice of 2 plot modes, either using one input array
 *           (Y vs index) or two input arrays (Y vs X).  This
 *           choice is made implicitly depending on the number of
 *           input arrays.
 *
 *     Usage:
 *         gplotCreate() to set up for the first plot
 *         gplotAddPlot() for each additional plot on the frame
 *         gplotMakeOutput() to run gnuplot and make all output files
 *         gplotDestroy() to clean up
 *
 *     Example of use:
 *         gplot = gplotCreate(natheta, nascore1, "tempskew",
 *                  GPLOT_PNG, GPLOT_LINES, "Skew score vs angle",
 *		   "plot 1", "angle (deg)", "score");
 *         gplotAddPlot(gplot, natheta, nascore2, GPLOT_POINTS, "plot 2");
 *         gplotMakeOutput(gplot);
 *         gplotDestroy(&gplot);
 *
 *     Note for output to GPLOT_LATEX:
 *         This creates latex output of the plot, named <rootname>.tex.
 *         It needs to be placed in a latex file <latexname>.tex
 *         that precedes the plot output with, at a minimum:
 *           \documentclass{article}
 *           \begin{document}
 *         and ends with
 *           \end{document}
 *         You can then generate a dvi file <latexname>.dvi using
 *           latex <latexname>.tex
 *         and a PostScript file <psname>.ps from that using
 *           dvips -o <psname>.ps <latexname>.dvi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

#if defined(__MINGW32__) || defined(_WIN32)
#define snprintf _snprintf
#endif

    /* MS VC++ can't handle array initialization with static consts ! */
#define L_BUF_SIZE      512
#define MAX_NUM_GPLOTS  40

const char  *gplotstylenames[] = {"with lines",
                                  "with points",
				  "with impulses",
                                  "with linespoints",
				  "with dots"};
const char  *gplotfilestyles[] = {"LINES",
                                  "POINTS",
				  "IMPULSES", 
                                  "LINESPOINTS",
				  "DOTS"};
const char  *gplotfileoutputs[] = {"PNG",
                                   "PS",
				   "EPS",
				   "X11",
                                   "LATEX"};


/*-----------------------------------------------------------------*
 *                       Basic Plotting Functions                  *
 *-----------------------------------------------------------------*/
/*!
 *  gplotCreate()
 *
 *      Input:  nax (<optional> numa: set to null for Y_VS_I;
 *                   required for Y_VS_X)
 *              nay (numa: required for both Y_VS_I and Y_VS_X)
 *              rootname (root for all output files)
 *              outformat (GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11,
 *                         GPLOT_LATEX)
 *              plotstyle (GPLOT_LINES, GPLOT_POINTS, GPLOT_IMPULSES,
 *                         GPLOT_LINESPOINTS, GPLOT_DOTS)
 *              title  (<optional> overall title)
 *              plottitle  (<optional> title for individual plot)
 *              xlabel (<optional> x axis label)
 *              ylabel (<optional> y axis label)
 *      Return: gplot, or null on error
 *
 *  There are 2 options for (x,y) values:
 *      o  To plot an array vs the index, use nay and set nax = NULL
 *      o  To plot one array vs another, use both nax and nay
 *  This function assigns plotmode value Y_VS_I to the first
 *  and Y_VS_X to the second.
 */
GPLOT  *
gplotCreate(NUMA        *nax,
            NUMA        *nay,
	    const char  *rootname,
	    l_int32      outformat,
	    l_int32      plotstyle,
	    const char  *title,
	    const char  *plottitle,
	    const char  *xlabel,
	    const char  *ylabel)
{
char       buf[L_BUF_SIZE];
char      *dataname, *cmdname, *outname;
l_int32    n, i, plotmode;
l_float32  valx, valy;
GPLOT     *gplot;
FILE      *fp;

    PROCNAME("gplotCreate");

    if (!nay)
	return (GPLOT *)ERROR_PTR("nay not defined", procName, NULL);
    if (!rootname)
	return (GPLOT *)ERROR_PTR("rootname not defined", procName, NULL);
    if (outformat != GPLOT_PNG && outformat != GPLOT_PS &&
        outformat != GPLOT_EPS && outformat != GPLOT_X11 &&
        outformat != GPLOT_LATEX)
	return (GPLOT *)ERROR_PTR("outformat invalid", procName, NULL);
    if (plotstyle != GPLOT_LINES && plotstyle != GPLOT_POINTS &&
        plotstyle != GPLOT_IMPULSES && plotstyle != GPLOT_LINESPOINTS &&
	plotstyle != GPLOT_DOTS)
	return (GPLOT *)ERROR_PTR("plotstyle invalid", procName, NULL);

    n = numaGetCount(nay);
    if (nax) {
	if (n != numaGetCount(nax))
	    return (GPLOT *)ERROR_PTR("nax and nay sizes differ",
		    procName, NULL);
    }

    if ((gplot = (GPLOT *)CALLOC(1, sizeof(GPLOT))) == NULL)
	return (GPLOT *)ERROR_PTR("gplot not made", procName, NULL);

	/* save outformat, rootname, cmdname, outname */
    gplot->outformat = outformat;
    gplot->rootname = stringNew(rootname);
    snprintf(buf, L_BUF_SIZE, "%s.cmd", rootname);
    cmdname = stringNew(buf);
    gplot->cmdname = cmdname;
    if (outformat == GPLOT_PNG)
	snprintf(buf, L_BUF_SIZE, "%s.png", rootname);
    else if (outformat == GPLOT_PS)
	snprintf(buf, L_BUF_SIZE, "%s.ps", rootname);
    else if (outformat == GPLOT_EPS)
	snprintf(buf, L_BUF_SIZE, "%s.eps", rootname);
    else if (outformat == GPLOT_LATEX)
	snprintf(buf, L_BUF_SIZE, "%s.tex", rootname);
    else  /* outformat == GPLOT_X11 */
	sprintf(buf, "");
    outname = stringNew(buf);
    gplot->outname = outname;

	/* get plot mode */
    if (!nax)
	plotmode = Y_VS_I;
    else
	plotmode = Y_VS_X;
    gplot->plotmode = plotmode;

	/* gen data name and write data to data file */
    snprintf(buf, L_BUF_SIZE, "%s.data.1", rootname);
    dataname = stringNew(buf);
    gplot->nplots = 1;
    if ((fp = fopen(dataname, "w")) == NULL)
	return (GPLOT *)ERROR_PTR("data stream not opened", procName, NULL);
    if (plotmode == Y_VS_I) {
	for (i = 0; i < n; i++) {
	    numaGetFValue(nay, i, &valy);
	    fprintf(fp, "%f  %f\n", (l_float32)i, valy);
	}
    }
    else {   /* plotmode == Y_VS_X */
	for (i = 0; i < n; i++) {
	    numaGetFValue(nax, i, &valx);
	    numaGetFValue(nay, i, &valy);
	    fprintf(fp, "%f  %f\n", valx, valy);
	}
    }
    fclose(fp);

	/* write instructions to command file */
    if ((fp = fopen(cmdname, "w")) == NULL)
	return (GPLOT *)ERROR_PTR("cmd stream not opened", procName, NULL);
    if (title) {   /* set title */
	snprintf(buf, L_BUF_SIZE, "set title '%s'\n", title);
	fprintf(fp, buf);
    }
    if (xlabel) {   /* set xlabel */
	snprintf(buf, L_BUF_SIZE, "set xlabel '%s'\n", xlabel);
	fprintf(fp, buf);
    }
    if (ylabel) {   /* set ylabel */
	snprintf(buf, L_BUF_SIZE, "set ylabel '%s'\n", ylabel);
	fprintf(fp, buf);
    }
    if (outformat == GPLOT_PNG)    /* set terminal type and output */
	snprintf(buf, L_BUF_SIZE, "set terminal png; set output '%s'\n", outname);
    else if (outformat == GPLOT_PS)
	snprintf(buf, L_BUF_SIZE, "set terminal postscript; set output '%s'\n",
                 outname);
    else if (outformat == GPLOT_EPS)
	snprintf(buf, L_BUF_SIZE,
                "set terminal postscript eps; set output '%s'\n", outname);
    else if (outformat == GPLOT_LATEX)
	snprintf(buf, L_BUF_SIZE, "set terminal latex; set output '%s'\n",
                 outname);
    else  /* outformat == GPLOT_X11 */
	snprintf(buf, L_BUF_SIZE, "set terminal x11\n");
    fprintf(fp, buf);
    if (plottitle)
	snprintf(buf, L_BUF_SIZE, "plot '%s' title '%s' %s",
	        dataname, plottitle, gplotstylenames[plotstyle]);
    else
	snprintf(buf, L_BUF_SIZE,
                 "plot '%s' %s", dataname, gplotstylenames[plotstyle]);
    fprintf(fp, buf);
    fclose(fp);
	    
    FREE((void *)dataname);
    return gplot;
}


/*!
 *  gplotAddPlot()
 *
 *      Input:  gplot
 *              nax (<optional> numa)
 *              nay (required numa)
 *              plotstyle (GPLOT_LINES, GPLOT_POINTS, GPLOT_IMPULSES,
 *                         GPLOT_LINESPOINTS, GPLOT_DOTS)
 *              plottitle (<optional> title for individual plot)
 *      Return: 0 if OK; 1 on error
 *
 *  Action: this uses gplot and the new arrays to add a plot
 *          to the output, by writing a new data file and 
 *          appending the appropriate plot commands to the
 *          command file.  If nax is not defined, nay is
 *          plotted against the array index.
 */
l_int32
gplotAddPlot(GPLOT       *gplot,
             NUMA        *nax,
             NUMA        *nay,
	     l_int32      plotstyle,
	     const char  *plottitle)
{
char       buf[L_BUF_SIZE];
char      *dataname;
l_int32    n, i, nplots;
l_float32  valx, valy;
FILE      *fp;

    PROCNAME("gplotAddPlot");

    if (!gplot)
	return ERROR_INT("gplot not defined", procName, 1);
    if (!nay)
	return ERROR_INT("nay not defined", procName, 1);

    n = numaGetCount(nay);
    if (nax) {
	if (n != numaGetCount(nax))
	    return ERROR_INT("nax and nay sizes differ", procName, 1);
    }

    nplots = gplot->nplots + 1;
    gplot->nplots = nplots;
    snprintf(buf, L_BUF_SIZE, "%s.data.%d", gplot->rootname, nplots);
    dataname = stringNew(buf);
    if ((fp = fopen(dataname, "w")) == NULL)
	return ERROR_INT("data stream not opened", procName, 1);

    if (!nax) {  /* use mode Y_VS_I */
	for (i = 0; i < n; i++) {
	    numaGetFValue(nay, i, &valy);
	    fprintf(fp, "%f  %f\n", (l_float32)i, valy);
	}
    }
    else {   /* use mode Y_VS_X */
	for (i = 0; i < n; i++) {
	    numaGetFValue(nax, i, &valx);
	    numaGetFValue(nay, i, &valy);
	    fprintf(fp, "%f  %f\n", valx, valy);
	}
    }
    fclose(fp);

    if ((fp = fopen(gplot->cmdname, "a")) == NULL)
	return ERROR_INT("cmd stream not opened", procName, 1);
    if (plottitle)
	snprintf(buf, L_BUF_SIZE, ", \\\n '%s' title '%s' %s",
	        dataname, plottitle, gplotstylenames[plotstyle]);
    else
	snprintf(buf, L_BUF_SIZE,
                ", \\\n '%s' %s", dataname, gplotstylenames[plotstyle]);
    fprintf(fp, buf);
    fclose(fp);

    FREE((void *)dataname);
    return 0;
}


/*!
 *  gplotMakeOutput()
 *
 *      Input:  gplot
 *      Return: 0 if OK; 1 on error
 *
 *  Action: this uses gplot and the new arrays to add a plot
 *          to the output, by writing a new data file and appending
 *          the appropriate plot commands to the command file.
 */
l_int32
gplotMakeOutput(GPLOT  *gplot)
{
char  buf[L_BUF_SIZE];

    PROCNAME("gplotMakeOutput");

    if (!gplot)
	return ERROR_INT("gplot not defined", procName, 1);

    if (gplot->outformat != GPLOT_X11)
	snprintf(buf, L_BUF_SIZE, "gnuplot %s &", gplot->cmdname);
    else
	snprintf(buf, L_BUF_SIZE,
                 "gnuplot -persist -geometry +10+10 %s &", gplot->cmdname);
    system(buf);
    return 0;
}


/*!
 *   gplotDestroy()
 *
 *        Input: &gplot (<to be nulled>)
 *        Return: void
 */
void
gplotDestroy(GPLOT  **pgplot)
{
GPLOT  *gplot;

    PROCNAME("gplotDestroy");

    if (pgplot == NULL) {
	L_WARNING("ptr address is null!", procName);
	return;
    }

    if ((gplot = *pgplot) == NULL)	
	return;

    FREE((void *)gplot->rootname);
    FREE((void *)gplot->cmdname);
    FREE((void *)gplot->outname);
    if (gplot->title)
	FREE((void *)gplot->title);
    if (gplot->xlabel)
	FREE((void *)gplot->xlabel);
    if (gplot->ylabel)
	FREE((void *)gplot->ylabel);

    FREE((void *)gplot);
    *pgplot = NULL;
    return;
}


/*-----------------------------------------------------------------*
 *                       Quick and Dirty Plots                     *
 *-----------------------------------------------------------------*/
/*!
 *  gplotSimple1()
 *
 *      Input:  na   (numa; we plot Y_VS_I)
 *              outformat (GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11,
 *                         GPLOT_LATEX)
 *              outroot (root of output files)
 *              title  (<optional>, can be NULL)
 *      Return: 0 if OK, 1 on error
 *
 *  This is a simple line plot of a numa, where the array value
 *  is plotted vs the array index.  You can put a title on the
 *  plot if you want.  The plot is generated in the specified
 *  output format.
 *
 *  If you are generating more than 1 of these, be sure the outroot
 *  strings are different so that you don't overwrite the output files!
 */
l_int32
gplotSimple1(NUMA        *na,
	     l_int32      outformat,
	     const char  *outroot,
	     const char  *title)
{
GPLOT  *gplot;

    PROCNAME("gplotSimple1");

    if (!na)
	return ERROR_INT("na not defined", procName, 1);
    if (outformat != GPLOT_PNG && outformat != GPLOT_PS &&
        outformat != GPLOT_EPS && outformat != GPLOT_X11 &&
        outformat != GPLOT_LATEX)
	return ERROR_INT("invalid outformat", procName, 1);
    if (!outroot)
	return ERROR_INT("outroot not specified", procName, 1);

    if ((gplot = gplotCreate(NULL, na, outroot, outformat, GPLOT_LINES,
                        title, NULL, NULL, NULL)) == NULL)
	return ERROR_INT("gplot not made", procName, 1);
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    return 0;
}


/*!
 *  gplotSimple2()
 *
 *      Input:  na1 (numa; we plot Y_VS_I)
 *              na2 (ditto)
 *              outformat (GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11,
 *                         GPLOT_LATEX)
 *              outroot (root of output files)
 *              title  (<optional>)
 *      Return: 0 if OK, 1 on error
 *
 *  This is a simple line plot of two numa, where the array values
 *  are each plotted vs the array index.  You can put a title on the
 *  plot if you want.  The plot is generated in the specified
 *  output format.
 *
 *  If you are generating more than 1 of these, be sure the outroot
 *  strings are different so that you don't overwrite the output files!
 */
l_int32
gplotSimple2(NUMA        *na1,
	     NUMA        *na2,
	     l_int32      outformat,
	     const char  *outroot,
	     const char  *title)
{
GPLOT  *gplot;

    PROCNAME("gplotSimple2");

    if (!na1 || !na2)
	return ERROR_INT("na1 and na2 not both defined", procName, 1);
    if (outformat != GPLOT_PNG && outformat != GPLOT_PS &&
        outformat != GPLOT_EPS && outformat != GPLOT_X11 &&
        outformat != GPLOT_LATEX)
	return ERROR_INT("invalid outformat", procName, 1);
    if (!outroot)
	return ERROR_INT("outroot not specified", procName, 1);

    if ((gplot = gplotCreate(NULL, na1, outroot, outformat, GPLOT_LINES,
                         title, NULL, NULL, NULL)) == NULL)
	return ERROR_INT("gplot not made", procName, 1);
    gplotAddPlot(gplot, NULL, na2, GPLOT_LINES, NULL);
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    return 0;
}


/*!
 *  gplotSimpleN()
 *
 *      Input:  naa (numaa; we plot Y_VS_I for each numa)
 *              outformat (GPLOT_PNG, GPLOT_PS, GPLOT_EPS, GPLOT_X11,
 *                         GPLOT_LATEX)
 *              outroot (root of output files)
 *              title (<optional>)
 *      Return: 0 if OK, 1 on error
 *
 *  This is a simple line plot of all numas in a numaa (array of numa),
 *  where for each numa the array values are each plotted vs the
 *  array index.  You can put a title on the plot if you want.
 *  The plot is generated in the specified output format.
 *
 *  If you are generating more than 1 of these, be sure the outroot
 *  strings are different so that you don't overwrite the output files!
 */
l_int32
gplotSimpleN(NUMAA       *naa,
	     l_int32      outformat,
	     const char  *outroot,
	     const char  *title)
{
l_int32  i, n;
GPLOT   *gplot;
NUMA    *na;

    PROCNAME("gplotSimpleN");

    if (!naa)
	return ERROR_INT("naa not defined", procName, 1);
    if (outformat != GPLOT_PNG && outformat != GPLOT_PS &&
        outformat != GPLOT_EPS && outformat != GPLOT_X11 &&
        outformat != GPLOT_LATEX)
	return ERROR_INT("invalid outformat", procName, 1);
    if (!outroot)
	return ERROR_INT("outroot not specified", procName, 1);

    if ((n = numaaGetCount(naa)) == 0)
	return ERROR_INT("no numa in array", procName, 1);
    for (i = 0; i < n; i++) {
        na = numaaGetNuma(naa, i, L_CLONE);
	if (i == 0) {
            gplot = gplotCreate(NULL, na, outroot, outformat, GPLOT_LINES,
                         title, NULL, NULL, NULL);
        } else {
	    gplotAddPlot(gplot, NULL, na, GPLOT_LINES, NULL);
	}
        numaDestroy(&na);
    }
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);
    return 0;
}



/*-----------------------------------------------------------------*
 *                   Plotting from an input text file              *
 *-----------------------------------------------------------------*/
/*!
 *  gplotFromFile()
 *
 *      Input:  input data filename
 *              rootname for output files
 *              output type {PNG, PS, EPS, X11}
 *      Return: 0 if OK; 1 on error
 *
 *  This routine allows you to make plots from a single text
 *  file consisting of an optional header that contains plot
 *  information followed by the data to be plotted.
 *     
 *  ------------------
 *  Header Information
 *  ------------------
 *
 *  All lines starting with '%' are comments and are ignored
 *  All blank lines are ignored
 *  The header information is optional.  It must be followed
 *  by a line beginning with '&' to signal the beginning of the data.
 *
 *  If header plotinfo lines exist, the first three must be
 *         title
 *         xaxis label
 *         yaxis label
 *  These all default to null strings.
 *
 *  Optionally after that, the individual plots can be
 *  labelled and the drawing method specified.  This is
 *  done by specifying:
 *         number of plots
 *         title for plot 1
 *         drawing style for plot 1
 *         title for plot 1
 *         drawing style for plot 1
 *         ...
 *
 *  Default for this is to have the individual plots untitled
 *  and to use LINES for each plot.  The drawing styles allowed
 *  must be chosen from the set:
 *         {LINES, POINTS, IMPULSES, LINESPOINTS, DOTS}
 *
 *  ------------------
 *  Data Information
 *  ------------------
 *
 *  Use '&' at the beginning of a line as a control character.
 *  The first '&' signals that the data is to start
 *  Subsequent '&' are separators between data for different plots. 
 *  This way, multiple plots can be drawn on the same output graph.
 *
 *  All blank lines or lines starting with '%' are ignored.
 *
 *  Data in file can have either one or two numbers per line.
 *  If there is one number, it is assumed to be the "y" value,
 *  and the "x" value is taken to be the line number, starting with 1.
 *
 *  All characters other than the digits 0-9, '+', '-' and '.' are ignored,
 *  (with the exception that '%' and '&' have a special meaning when
 *  at the beginning of a line).
 */
l_int32
gplotFromFile(const char  *filein,
              const char  *rootout,
	      const char  *outputtype)
{
char      *str, *linestr;
char      *title;
char      *xlabel;
char      *ylabel;
char      *plottext[MAX_NUM_GPLOTS];
l_int32    i, j, n, len, nbytes;
l_int32    nh, nplots, linesfound, iplot, plotnum, outtype, found, nread;
l_int32    plotstyle[MAX_NUM_GPLOTS];
l_float32  xval, yval;
SARRAY    *sa;
NUMA      *nax, *nay;
GPLOT     *gplot;

    PROCNAME("gplotFromFile");

    if (!filein)
	return ERROR_INT("filein not defined", procName, 1);
    if (!rootout)
	return ERROR_INT("rootout not defined", procName, 1);
    if (!outputtype)
	return ERROR_INT("outputtype not defined", procName, 1);

	/* get the output type */
    outtype = UNDEF;
    for (i = 0; i < NUM_GPLOT_OUTPUTS; i++) {
	if (strcmp(outputtype, gplotfileoutputs[i]) == 0) {
	    outtype = i;
	    break;
	}
    }
    if (outtype == UNDEF) {
	L_WARNING("outputtype not in set {PNG, PS, EPS, X11}; using PNG",
	          procName);
	outtype = GPLOT_PNG;
    }
	
	/* initialize */
    title = stringNew("");
    xlabel = stringNew("");
    ylabel = stringNew("");
    for (i = 0; i < MAX_NUM_GPLOTS; i++) {
	plottext[i] = stringNew("");
	plotstyle[i] = 0;  /* LINES */
    }

    if ((str = (char *)arrayRead(filein, &nbytes)) == NULL)
	return ERROR_INT("str not read", procName, 1);
    if ((sa = sarrayCreateLinesFromString(str, 0)) == NULL)
	return ERROR_INT("sa not made", procName, 1);
    n = sarrayGetCount(sa);

    /*-------------   parse the header info ----------------- */

	/* find the number of header lines (i.e., those 
	 * that are before the beginning of data) */
    found = FALSE;
    for (i = 0; i < n; i++) {
	linestr = sarrayGetString(sa, i, 0);
	if (linestr[0] == '&') {
	    found = TRUE;
	    break;
	}
    }
    if (found == FALSE)
	return ERROR_INT("'&' not found in data file", procName, 1);
    nh = i;  /* number of header lines found */

	/* parse the header */
    if (nh > 0)
    {
	    /* read the generic header info */
	linesfound = 0;
	nplots = 0;
	for (i = 0; i < nh; i++) {
	    linestr = sarrayGetString(sa, i, 0);
	    if (linestr[0] == '%')  /* ignore */
		continue;
	    linesfound++;
	    if (linesfound == 1)
		stringReplace(&title, linestr);
	    else if (linesfound == 2)
		stringReplace(&xlabel, linestr);
	    else if (linesfound == 3)
		stringReplace(&ylabel, linestr);
	    else if (linesfound == 4) {
	        nplots = atoi(linestr);
		break;
	    }
	}
	nread = i + 1;

	if (nplots < 0 || nplots > MAX_NUM_GPLOTS)
	    return ERROR_INT("too many plots", procName, 1);

	    /* read the header info specific to each plot */
	if (nplots > 0) {
	    linesfound = 0;
	    iplot = 0;
	    for (i = nread; i < nh; i++) {
		if (linesfound >= 2 * nplots)  /* sanity check */
		    break;
		linestr = sarrayGetString(sa, i, 0);
		if (linestr[0] == '%')  /* ignore */
		    continue;
		linesfound++;

		if (linesfound % 2 == 1) {
		    if (linesfound > 1)
			iplot++;
		    stringReplace(&plottext[iplot], linestr);
		}
		else {
		    for (j = 0; j < NUM_GPLOT_STYLES; j++) {
			if (strcmp(linestr, gplotfilestyles[j]) == 0) {
			    plotstyle[iplot] = j;
			    break;
			}
		    }
		}
	    }
	}
    }

    /*-------------   parse the data info ----------------- */

    nax = numaCreate(n);
    nay = numaCreate(n);
    plotnum = 0;
    for (i = nh + 1; i < n; i++) {
	linestr = sarrayGetString(sa, i, 1);

        if (linestr[0] == '%') {  /* ignore */
            FREE((void *)linestr);
            continue;
        }

        if (linestr[0] == '&') {   /* end of this plot data */
            if (numaGetCount(nax) == 0) { /* nothing to plot */
                FREE((void *)linestr);
                continue;
            }
            if (plotnum == 0) 
	        gplot = gplotCreate(nax, nay, rootout,
                                    outtype, plotstyle[plotnum],
				    title, plottext[plotnum], xlabel, ylabel);
            else {
                gplotAddPlot(gplot, nax, nay, plotstyle[plotnum],
		             plottext[plotnum]);
            }

#if DEBUG
            numaWriteStream(stderr, nax);
            numaWriteStream(stderr, nay);
#endif  /* DEBUG */

            numaDestroy(&nax);
            numaDestroy(&nay);
            nax = numaCreate(n);
            nay = numaCreate(n);
            plotnum++;
            FREE((void *)linestr);
            continue;
        }

            /* append the data to the number arrays */
	len = strlen(linestr);
	for (j = 0; j < len; j++) {
	    if (linestr[j] != '0' && linestr[j] != '1' &&
	        linestr[j] != '2' && linestr[j] != '3' &&
	        linestr[j] != '4' && linestr[j] != '5' &&
	        linestr[j] != '6' && linestr[j] != '7' &&
	        linestr[j] != '8' && linestr[j] != '9' &&
	        linestr[j] != '+' && linestr[j] != '-' &&
	        linestr[j] != '.')
		    linestr[j] = ' ';
	}
	if (sscanf(linestr, "%f %f", &xval, &yval) == 2) {
	    numaAddNumber(nax, xval);
	    numaAddNumber(nay, yval);
	}
	else if (sscanf(linestr, "%f", &yval) == 1) {
	    numaAddNumber(nax, i + 1);
	    numaAddNumber(nay, yval);
	}
	else {
	    L_WARNING_INT("yval not read properly in data line %d",
	                  procName, i);
        }
        FREE((void *)linestr);
    }

    if (numaGetCount(nax) > 0) {  /* make final plot */
	if (plotnum == 0) 
	    gplot = gplotCreate(nax, nay, rootout,
			        outtype, plotstyle[plotnum],
			        title, plottext[plotnum], xlabel, ylabel);
	else {
	    gplotAddPlot(gplot, nax, nay, plotstyle[plotnum],
			 plottext[plotnum]);
	}
#if DEBUG
        numaWriteStream(stderr, nax);
        numaWriteStream(stderr, nay);
#endif  /* DEBUG */
    }

    /*-----  write the cmd and data files and invoke gnuplot  ----*/

    gplotMakeOutput(gplot);
        
	/* clean up */
    FREE((void *)str);
    FREE((void *)title);
    FREE((void *)xlabel);
    FREE((void *)ylabel);
    for (i = 0; i < MAX_NUM_GPLOTS; i++)
	FREE((void *)plottext[i]);
    numaDestroy(&nax);
    numaDestroy(&nay);
    sarrayDestroy(&sa);
    gplotDestroy(&gplot);

    return 0;
}

