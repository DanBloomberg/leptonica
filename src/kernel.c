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
 *  kernel.c
 *
 *      Basic operations on kernels for image convolution
 *
 *         Create/destroy
 *            L_KERNEL   *kernelCreate()
 *            void        kernelDestroy()
 *
 *         Accessors:
 *            l_int32     kernelGetElement()
 *            l_int32     kernelSetElement()
 *            l_int32     kernelGetParameters()
 *            l_int32     kernelSetOrigin()
 *            l_int32     kernelGetNorm()
 *
 *         Serialized I/O
 *            L_KERNEL   *kernelRead()
 *            L_KERNEL   *kernelReadStream()
 *            l_int32     kernelWrite()
 *            l_int32     kernelWriteStream()
 *       
 *         Making a kernel from a compiled string
 *            L_KERNEL   *kernelCreateFromString()
 *
 *         Making a kernel from a simple file format
 *            L_KERNEL   *kernelCreateFromFile()
 *
 *         Making a kernel from a Pix
 *            L_KERNEL   *kernelCreateFromPix()
 *
 *         Display a kernel in a pix
 *            PIX        *kernelDisplayInPix()
 *
 *         Parse string to extract ints
 *            NUMA       *parseStringForInts()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


/*------------------------------------------------------------------------*
 *                           Create / Destroy                             *
 *------------------------------------------------------------------------*/
/*!
 *  kernelCreate()
 *
 *      Input:  height, width
 *      Return: kernel, or null on error
 *
 *  Notes:
 *      (1) kernelCreate() initializes all values to 0.
 *      (2) After this call, (cy,cx) and nonzero data values must be
 *          assigned.
 */
L_KERNEL *
kernelCreate(l_int32  height,
             l_int32  width)
{
L_KERNEL  *kel;

    PROCNAME("kernelCreate");

    if ((kel = (L_KERNEL *)CALLOC(1, sizeof(L_KERNEL))) == NULL)
        return (L_KERNEL *)ERROR_PTR("kel not made", procName, NULL);
    kel->sy = height;
    kel->sx = width;
    if ((kel->data = create2dIntArray(height, width)) == NULL)
        return (L_KERNEL *)ERROR_PTR("data not allocated", procName, NULL);

    return kel;
}


/*!
 *  kernelDestroy()
 *
 *      Input:  &kel (<to be nulled>)
 *      Return: void
 */
void
kernelDestroy(L_KERNEL  **pkel)
{
l_int32    i;
L_KERNEL  *kel;

    PROCNAME("kernelDestroy");

    if (pkel == NULL)  {
        L_WARNING("ptr address is NULL!", procName);
        return;
    }
    if ((kel = *pkel) == NULL)
        return;

    for (i = 0; i < kel->sy; i++)
        FREE(kel->data[i]);
    FREE(kel->data);
    FREE(kel);

    *pkel = NULL;
    return;
}


/*----------------------------------------------------------------------*
 *                               Accessors                              *
 *----------------------------------------------------------------------*/
/*!
 *  kernelGetElement()
 *
 *      Input:  kel
 *              row
 *              col
 *              &val
 *      Return: 0 if OK; 1 on error
 */
l_int32
kernelGetElement(L_KERNEL  *kel,
                 l_int32    row,
                 l_int32    col,
                 l_int32   *pval)
{
    PROCNAME("kernelGetElement");

    if (!kel)
        return ERROR_INT("kernel not defined", procName, 1);
    if (row < 0 || row >= kel->sy)
        return ERROR_INT("kernel row out of bounds", procName, 1);
    if (col < 0 || col >= kel->sx)
        return ERROR_INT("kernel col out of bounds", procName, 1);

    *pval = kel->data[row][col];
    return 0;
}


/*!
 *  kernelSetElement()
 *
 *      Input:  kernel
 *              row
 *              col
 *              val
 *      Return: 0 if OK; 1 on error
 */
l_int32
kernelSetElement(L_KERNEL  *kel,
                 l_int32    row,
                 l_int32    col,
                 l_int32    val)
{
    PROCNAME("kernelSetElement");

    if (!kel)
        return ERROR_INT("kel not defined", procName, 1);
    if (val < -255 || val > 255)
        return ERROR_INT("invalid kernel element type", procName, 1);
    if (row < 0 || row >= kel->sy)
        return ERROR_INT("kernel row out of bounds", procName, 1);
    if (col < 0 || col >= kel->sx)
        return ERROR_INT("kernel col out of bounds", procName, 1);

    kel->data[row][col] = val;
    return 0;
}


/*!
 *  kernelGetParameters()
 *
 *      Input:  kernel
 *              &sy, &sx, &cy, &cx (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
kernelGetParameters(L_KERNEL  *kel,
                    l_int32   *psy,
                    l_int32   *psx,
                    l_int32   *pcy,
                    l_int32   *pcx)
{
    PROCNAME("kernelGetParameters");

    if (!kel)
        return ERROR_INT("kernel not defined", procName, 1);
    if (psy) *psy = kel->sy; 
    if (psx) *psx = kel->sx; 
    if (pcy) *pcy = kel->cy; 
    if (pcx) *pcx = kel->cx; 
    return 0;
}


/*!
 *  kernelSetOrigin()
 *
 *      Input:  kernel
 *              cy, cx
 *      Return: 0 if OK; 1 on error
 */
l_int32
kernelSetOrigin(L_KERNEL  *kel,
                l_int32    cy,
                l_int32    cx)
{
    PROCNAME("kernelSetOrigin");

    if (!kel)
        return ERROR_INT("kel not defined", procName, 1);
    kel->cy = cy;
    kel->cx = cx;
    return 0;
}


/*!
 *  kernelGetNorm()
 *
 *      Input:  kernel
 *              &norm (<return> multiplicative normalizing factor)
 *      Return: 0 if OK, 1 on error
 */
l_int32
kernelGetNorm(L_KERNEL   *kel,
              l_float32  *pnorm)
{
l_int32  sx, sy, i, j, sum;

    PROCNAME("kernelGetNorm");

    if (!pnorm)
        return ERROR_INT("&norm not defined", procName, 1);
    *pnorm = 1.0;
    if (!kel)
        return ERROR_INT("kernel not defined", procName, 1);

    kernelGetParameters(kel, &sy, &sx, NULL, NULL);
    sum = 0;
    for (i = 0; i < sy; i++) {
        for (j = 0; j < sx; j++) {
            sum += kel->data[i][j];
        }
    }
    *pnorm = 1. / (l_float32)sum;

    return 0;
}


/*----------------------------------------------------------------------*
 *                            Kernel serialized I/O                     *
 *----------------------------------------------------------------------*/
/*!
 *  kernelRead()
 *
 *      Input:  filename
 *      Return: kernel, or null on error
 */
L_KERNEL *
kernelRead(const char  *fname)
{
FILE      *fp;
L_KERNEL  *kel;

    PROCNAME("kernelRead");

    if (!fname)
        return (L_KERNEL *)ERROR_PTR("fname not defined", procName, NULL);

    if ((fp = fopen(fname, "rb")) == NULL)
        return (L_KERNEL *)ERROR_PTR("stream not opened", procName, NULL);
    if ((kel = kernelReadStream(fp)) == NULL)
        return (L_KERNEL *)ERROR_PTR("kel not returned", procName, NULL);
    fclose(fp);

    return kel;
}


/*!
 *  kernelReadStream()
 *
 *      Input:  stream
 *      Return: kernel, or null on error
 */
L_KERNEL *
kernelReadStream(FILE  *fp)
{
l_int32    sy, sx, cy, cx, i, j, ret, version;
L_KERNEL  *kel;

    PROCNAME("kernelReadStream");

    if (!fp)
        return (L_KERNEL *)ERROR_PTR("stream not defined", procName, NULL);

    ret = fscanf(fp, "  Kernel Version %d\n", &version);
    if (ret != 1)
        return (L_KERNEL *)ERROR_PTR("not a kernel file", procName, NULL);
    if (version != KERNEL_VERSION_NUMBER)
        return (L_KERNEL *)ERROR_PTR("invalid kernel version", procName, NULL);

    if (fscanf(fp, "  sy = %d, sx = %d, cy = %d, cx = %d\n",
            &sy, &sx, &cy, &cx) != 4)
        return (L_KERNEL *)ERROR_PTR("dimensions not read", procName, NULL);

    if ((kel = kernelCreate(sy, sx)) == NULL)
        return (L_KERNEL *)ERROR_PTR("kel not made", procName, NULL);
    kernelSetOrigin(kel, cy, cx);

    for (i = 0; i < sy; i++) {
        fscanf(fp, "    ");
        for (j = 0; j < sx; j++)
            fscanf(fp, "%5d", &kel->data[i][j]);
        fscanf(fp, "\n");
    }
    fscanf(fp, "\n");

    return kel;
}


/*!
 *  kernelWrite()
 *
 *      Input:  fname (output file)
 *              kernel
 *      Return: 0 if OK, 1 on error
 */
l_int32
kernelWrite(const char  *fname,
            L_KERNEL    *kel)
{
FILE  *fp;

    PROCNAME("kernelWrite");

    if (!fname)
        return ERROR_INT("fname not defined", procName, 1);
    if (!kel)
        return ERROR_INT("kel not defined", procName, 1);

    if ((fp = fopen(fname, "wb")) == NULL)
        return ERROR_INT("stream not opened", procName, 1);
    kernelWriteStream(fp, kel);
    fclose(fp);

    return 0;
}


/*!
 *  kernelWriteStream()
 *
 *      Input:  stream
 *              kel
 *      Return: 0 if OK, 1 on error
 */
l_int32
kernelWriteStream(FILE      *fp,
                  L_KERNEL  *kel)
{
l_int32  sx, sy, cx, cy, i, j;

    PROCNAME("kernelWriteStream");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!kel)
        return ERROR_INT("kel not defined", procName, 1);
    kernelGetParameters(kel, &sy, &sx, &cy, &cx);

    fprintf(fp, "  Kernel Version %d\n", KERNEL_VERSION_NUMBER);
    fprintf(fp, "  sy = %d, sx = %d, cy = %d, cx = %d\n", sy, sx, cy, cx);
    for (i = 0; i < sy; i++) {
        fprintf(fp, "    ");
        for (j = 0; j < sx; j++)
            fprintf(fp, "%5d", kel->data[i][j]);
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    return 0;
}


/*----------------------------------------------------------------------*
 *                 Making a kernel from a compiled strings              *
 *----------------------------------------------------------------------*/
/*!
 *  kernelCreateFromString()
 *
 *      Input:  height, width
 *              cy, cx   (origin)
 *              data
 *      Return: kernel of the given size, or null on error
 *
 *  Notes:
 *      (1) The data is an array of chars, in row-major order, where
 *          each char is an integer in the range [-255 ... 255].
 *      (2) The only other formatting limitation is that you must
 *          leave space between the last number in each row and
 *          the double-quote.  If possible, it's also nice to have each
 *          line in the string represent a line in the kernel; e.g.,
 *              static const char *kdata =
 *                  " 20   50   20 "
 *                  " 70  140   70 "
 *                  " 20   50   20 ";
 */
L_KERNEL *
kernelCreateFromString(l_int32      h,
                       l_int32      w,
                       l_int32      cy,
                       l_int32      cx,
                       const char  *kdata)
{
l_int32    n, i, j, index, val;
L_KERNEL  *kel;
NUMA      *na;

    PROCNAME("kernelCreateFromString");

    if (h < 1)
        return (L_KERNEL *)ERROR_PTR("height must be > 0", procName, NULL);
    if (w < 1)
        return (L_KERNEL *)ERROR_PTR("width must be > 0", procName, NULL);
    if (cy < 0 || cy >= h)
        return (L_KERNEL *)ERROR_PTR("cy invalid", procName, NULL);
    if (cx < 0 || cx >= w)
        return (L_KERNEL *)ERROR_PTR("cx invalid", procName, NULL);
    
    kel = kernelCreate(h, w);
    kernelSetOrigin(kel, cy, cx);
    na = parseStringForInts(kdata, " \t\n");
    n = numaGetCount(na);
    if (n != w * h) {
        numaDestroy(&na);
	fprintf(stderr, "w = %d, h = %d, num ints = %d\n", w, h, n);
        return (L_KERNEL *)ERROR_PTR("invalid integer data", procName, NULL);
    }

    index = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            numaGetIValue(na, index, &val);
            kernelSetElement(kel, i, j, val);
	    index++;
        }
    }

    numaDestroy(&na);
    return kel;
}


/*----------------------------------------------------------------------*
 *                Making a kernel from a simple file format             *
 *----------------------------------------------------------------------*/
/*!
 *  kernelCreateFromFile()
 *
 *      Input:  filename
 *      Return: kernel, or null on error
 *
 *  Notes:
 *      (1) The file contains, in the following order:
 *           - Any number of comment lines starting with '#' are ignored
 *           - The height and width of the kernel
 *           - The y and x values of the kernel origin
 *           - The kernel data, formatted as lines of integers for
 *             the kernel values in row-major order, and with no other
 *             punctuation.  (Note: this differs from kernelCreateFromString(),
 *             where each line must begin and end with a double-quote
 *             to tell the compiler it's part of a string.)
 *           - The kernel specification ends when a blank line,
 *             a comment line, or the end of file is reached.
 *      (2) All lines must be left-justified.
 *      (3) See kernelCreateFromString() for a description of the string
 *          format for the kernel data.  As an example, here are the lines
 *          of a valid kernel description file  In the file, all lines 
 *          are left-justified:
 *                    # small 3x3 kernel
 *                    3 3
 *                    1 1
 *                    20   50   20
 *                    70  140   70
 *                    20   50   20
 */
L_KERNEL *
kernelCreateFromFile(const char  *filename)
{
char      *filestr, *line;
l_int32    nbytes, nlines, i, j, first, index, val, w, h, cx, cy, n;
NUMA      *na, *nat;
SARRAY    *sa;
L_KERNEL  *kel;

    PROCNAME("kernelCreateFromFile");

    if (!filename)
        return (L_KERNEL *)ERROR_PTR("filename not defined", procName, NULL);
    
    filestr = (char *)arrayRead(filename, &nbytes);
    sa = sarrayCreateLinesFromString(filestr, 1);
    FREE(filestr);
    nlines = sarrayGetCount(sa);

        /* Find the first data line. */
    for (i = 0; i < nlines; i++) {
        line = sarrayGetString(sa, i, L_NOCOPY);
	if (line[0] != '#') {
            first = i;
            break;
        }
    }

        /* Find the kernel dimensions and origin location. */
    line = sarrayGetString(sa, first, L_NOCOPY);
    if (sscanf(line, "%d %d", &h, &w) != 2)
        return (L_KERNEL *)ERROR_PTR("error reading h,w", procName, NULL);
    line = sarrayGetString(sa, first + 1, L_NOCOPY);
    if (sscanf(line, "%d %d", &cy, &cx) != 2)
        return (L_KERNEL *)ERROR_PTR("error reading cy,cx", procName, NULL);

        /* Extract the data.  This ends when we reach eof, or when we
	 * encounter a line of data that is either a null string or
	 * contains just a newline. */
    na = numaCreate(0);
    for (i = first + 2; i < nlines; i++) {
        line = sarrayGetString(sa, i, L_NOCOPY);
        if (line[0] == '\0' || line[0] == '\n' || line[0] == '#')
            break;
        nat = parseStringForInts(line, " \t\n");
	numaJoin(na, nat, 0, 0);
	numaDestroy(&nat);
    }
    sarrayDestroy(&sa);

    n = numaGetCount(na);
    if (n != w * h) {
        numaDestroy(&na);
	fprintf(stderr, "w = %d, h = %d, num ints = %d\n", w, h, n);
        return (L_KERNEL *)ERROR_PTR("invalid integer data", procName, NULL);
    }

    kel = kernelCreate(h, w);
    kernelSetOrigin(kel, cy, cx);
    index = 0;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            numaGetIValue(na, index, &val);
            kernelSetElement(kel, i, j, val);
	    index++;
        }
    }

    numaDestroy(&na);
    return kel;
}


/*----------------------------------------------------------------------*
 *                       Making a kernel from a Pix                     *
 *----------------------------------------------------------------------*/
/*!
 *  kernelCreateFromPix()
 *
 *      Input:  pix
 *              cy, cx (origin of kernel)
 *      Return: kernel, or null on error
 *
 *  Notes:
 *      (1) The origin must be positive and within the dimensions of the pix.
 */
L_KERNEL *
kernelCreateFromPix(PIX         *pix,
                    l_int32      cy,
                    l_int32      cx)
{
l_int32    i, j, w, h, d;
l_uint32   val;
L_KERNEL  *kel;

    PROCNAME("kernelCreateFromPix");

    if (!pix)
        return (L_KERNEL *)ERROR_PTR("pix not defined", procName, NULL);
    pixGetDimensions(pix, &w, &h, &d);
    if (d != 8)
        return (L_KERNEL *)ERROR_PTR("pix not 8 bpp", procName, NULL);
    if (cy < 0 || cx < 0 || cy >= h || cx >= w)
        return (L_KERNEL *)ERROR_PTR("(cy, cx) invalid", procName, NULL);

    kel = kernelCreate(h, w);
    kernelSetOrigin(kel, cy, cx);
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            pixGetPixel(pix, j, i, &val);
            kernelSetElement(kel, i, j, val);
        }
    }

    return kel;
}


/*----------------------------------------------------------------------*
 *                     Display a kernel in a pix                        *
 *----------------------------------------------------------------------*/
/*!
 *  kernelDisplayInPix()
 *
 *      Input:  kernel
 *              size (of grid interiors; odd; minimum size of 17 is enforced)
 *              gthick (grid thickness; minimum size of 2 is enforced)
 *      Return: pix (display of kernel), or null on error
 *
 *  Notes:
 *      (1) This gives a visual representation of a kernel.
 *      (2) The origin is outlined in red.
 */
PIX *
kernelDisplayInPix(L_KERNEL     *kel,
                   l_int32       size,
                   l_int32       gthick)
{
l_int32  i, j, w, h, sx, sy, cx, cy, width, x0, y0, val;
PIX     *pixd, *pixt0, *pixt1;

    PROCNAME("kernelDisplayInPix");

    if (!kel)
        return (PIX *)ERROR_PTR("kernel not defined", procName, NULL);
    if (size < 17) {
        L_WARNING("size < 17; setting to 17", procName);
        size = 17;
    }
    if (size % 2 == 0)
        size++;
    if (gthick < 2) {
        L_WARNING("grid thickness < 2; setting to 2", procName);
        gthick = 2;
    }
    kernelGetParameters(kel, &sy, &sx, &cy, &cx);
    w = size * sx + gthick * (sx + 1);
    h = size * sy + gthick * (sy + 1);
    pixd = pixCreate(w, h, 8);

        /* Generate grid lines */
    for (i = 0; i <= sy; i++)
        pixRenderLine(pixd, 0, gthick / 2 + i * (size + gthick),
                      w - 1, gthick / 2 + i * (size + gthick),
                      gthick, L_SET_PIXELS);
    for (j = 0; j <= sx; j++)
        pixRenderLine(pixd, gthick / 2 + j * (size + gthick), 0,
                      gthick / 2 + j * (size + gthick), h - 1,
                      gthick, L_SET_PIXELS);

        /* Generate mask for each element */
    pixt0 = pixCreate(size, size, 1);
    pixSetAll(pixt0);

        /* Generate crossed lines for origin pattern */
    pixt1 = pixCreate(size, size, 1);
    width = size / 8;
    pixRenderLine(pixt1, size / 2, (l_int32)(0.12 * size),
                           size / 2, (l_int32)(0.88 * size),
                           width, L_SET_PIXELS);
    pixRenderLine(pixt1, (l_int32)(0.15 * size), size / 2,
                           (l_int32)(0.85 * size), size / 2,
                           width, L_FLIP_PIXELS);
    pixRasterop(pixt1, size / 2 - width, size / 2 - width,
                2 * width, 2 * width, PIX_NOT(PIX_DST), NULL, 0, 0);

        /* Paste the patterns in */
    y0 = gthick;
    for (i = 0; i < sy; i++) {
        x0 = gthick;
        for (j = 0; j < sx; j++) {
            kernelGetElement(kel, i, j, &val);
            pixSetMaskedGeneral(pixd, pixt0, val, x0, y0);
	    if (i == cy && j == cx)
                pixPaintThroughMask(pixd, pixt1, x0, y0, 255 - val);
            x0 += size + gthick;
        }
        y0 += size + gthick;
    }

    pixDestroy(&pixt0);
    pixDestroy(&pixt1);
    return pixd;
}


/*------------------------------------------------------------------------*
 *                     Parse string to extract ints                       *
 *------------------------------------------------------------------------*/
/*!
 *  parseStringForInts()
 *
 *      Input:  string (containing ints; not changed)
 *              seps (string of characters that can be used between ints)
 *      Return: numa (of ints found), or null on error
 */
NUMA *
parseStringForInts(const char  *str,
                   const char  *seps)
{
char    *newstr, *head, *tail;
l_int32  val;
NUMA    *na;

    PROCNAME("parseStringForInts");

    if (!str)
        return (NUMA *)ERROR_PTR("str not defined", procName, NULL);

    newstr = stringNew(str);  /* to enforce const-ness of str */
    na = numaCreate(0);
    head = strtokSafe(newstr, seps, &tail);
    val = atoi(head);
    numaAddNumber(na, val);
    FREE(head);
    while ((head = strtokSafe(NULL, seps, &tail)) != NULL) {
        val = atoi(head);
        numaAddNumber(na, val);
        FREE(head);
    }

    FREE(newstr);
    return na;
}



