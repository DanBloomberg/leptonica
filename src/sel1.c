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
 *  sel1.c
 *
 *      Basic ops on Sels and Selas
 *
 *         Create/destroy/copy:
 *            SELA      *selaCreate()
 *            void       selaDestroy()
 *            SEL       *selCreate()
 *            void       selDestroy()
 *            SEL       *selCopy()
 *            SEL       *selCreateBrick()
 *
 *         Helper proc:
 *            l_int32  **create2dIntArray()
 *
 *         Extension of sela:
 *            SELA      *selaAddSel()
 *            l_int32    selaExtendArray()
 *
 *         Accessors:
 *            l_int32    selaGetCount()
 *            SEL       *selaGetSel()
 *            char      *selGetName()
 *            l_int32    selaFindSelByName()
 *            l_int32    selGetElement()
 *            l_int32    selSetElement()
 *            l_int32    selGetParameters()
 *            char      *selaGetBrickName()
 *
 *         Max extent vals for erosion and hmt:
 *            l_int32    selFindMaxTranslations()
 *
 *         Write/read & visualization:
 *            l_int32    selaWrite()
 *            l_int32    selaWriteStream()
 *            l_int32    selWriteStream()
 *            l_int32    selaRead()
 *            l_int32    selaReadStream()
 *            l_int32    selReadStream()
 *       
 *         Building custom SELs:
 *            SEL       *selCreateFromString()
 *            void       selPrintToString()     [for debugging]
 *
 *     Usage note:
 *        Consistency, being the hobgoblin of small minds,
 *        is adhered to here in the dimensioning and accessing of sels.
 *        Everything is done in standard matrix (row, column) order.
 *        We have 3 functions that make sels:
 *             selCreate(), with input (h, w, [name])
 *             selCreateBrick(), with input (h, w, cy, cx, val)
 *             selCreateFromString() with input (text, h, w, [name])
 *        When we set specific elements in a sel, we likewise use
 *        (row, col) ordering:
 *             selSetElement(), with input (row, col, type)
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"

    /* MS VC++ can't handle array initialization with static consts ! */
#define L_BUF_SIZE      256

static const l_int32  INITIAL_PTR_ARRAYSIZE = 50;  /* n'import quoi */
static const l_int32  MANY_SELS = 1000;


/*------------------------------------------------------------------------*
 *                      Create / Destroy / Copy                           *
 *------------------------------------------------------------------------*/
/*!
 *  selaCreate()
 *
 *      Input:  n (initial number of sel ptrs; use 0 for default)
 *      Return: sela, or null on error
 */
SELA *
selaCreate(l_int32  n)
{
SELA  *sela;

    PROCNAME("selaCreate");

    if (n <= 0)
	n = INITIAL_PTR_ARRAYSIZE;
    if (n > MANY_SELS)
	L_WARNING_INT("%d sels", procName, n);

    if ((sela = (SELA *)CALLOC(1, sizeof(SELA))) == NULL)
	return (SELA *)ERROR_PTR("sela not made", procName, NULL);

    sela->nalloc = n;
    sela->n = 0;

	/* make array of se ptrs */
    if ((sela->sel = (SEL **)CALLOC(n, sizeof(SEL *))) == NULL)
	return (SELA *)ERROR_PTR("sel ptrs not made", procName, NULL);

    return sela;
}


/*!
 *  selaDestroy()
 *
 *      Input:  &sela (<to be nulled>)
 *      Return: void
 */
void
selaDestroy(SELA  **psela)
{
SELA    *sela;
l_int32  i;

    if (!psela) return;
    if ((sela = *psela) == NULL)
	return;

    for (i = 0; i < sela->n; i++)
	selDestroy(&sela->sel[i]);
    FREE(sela->sel);
    FREE(sela);
    *psela = NULL;
    return;
}


/*!
 *  selCreate()
 *
 *      Input:  height, width
 *              name (<optional> sel name; can be null)
 *      Return: sel, or null on error
 *
 *  Note: selCreate() initializes all values to 0.
 *        After this call, (cy,cx) and nonzero data values must be
 *        assigned.  If a text name is not assigned here, it will
 *        be needed later when the sel is put into a sela.
 */
SEL *
selCreate(l_int32      height,
	  l_int32      width,
	  const char  *name)
{
SEL  *sel;

    PROCNAME("selCreate");

    if ((sel = (SEL *)CALLOC(1, sizeof(SEL))) == NULL)
	return (SEL *)ERROR_PTR("sel not made", procName, NULL);
    if (name)
	sel->name = stringNew(name);
    sel->sy = height;
    sel->sx = width;
    if ((sel->data = create2dIntArray(height, width)) == NULL)
	return (SEL *)ERROR_PTR("data not allocated", procName, NULL);

    return sel;
}


/*!
 *  selDestroy()
 *
 *      Input:  &sel (<to be nulled>)
 *      Return: void
 */
void
selDestroy(SEL  **psel)
{
l_int32  i;
SEL     *sel;

    PROCNAME("selDestroy");

    if (psel == NULL)  {
	L_WARNING("ptr address is NULL!", procName);
	return;
    }
    if ((sel = *psel) == NULL)
	return;

    for (i = 0; i < sel->sy; i++)
	FREE(sel->data[i]);
    FREE(sel->data);
    if (sel->name)
	FREE(sel->name);
    FREE(sel);

    *psel = NULL;
    return;
}


/*!
 *  selCopy() 
 *
 *      Input:  sel
 *      Return: a copy of the sel, or null on error
 */
SEL *
selCopy(SEL  *sel)
{
l_int32  sx, sy, cx, cy, i, j;
SEL     *csel;

    PROCNAME("selCopy");

    if (!sel)
	return (SEL *)ERROR_PTR("sel not defined", procName, NULL);

    if ((csel = (SEL *)CALLOC(1, sizeof(SEL))) == NULL)
	return (SEL *)ERROR_PTR("csel not made", procName, NULL);

    selGetParameters(sel, &sy, &sx, &cy, &cx);
    csel->sy = sy;
    csel->sx = sx;
    csel->cy = cy;
    csel->cx = cx;

    if ((csel->data = create2dIntArray(sy, sx)) == NULL)
	return (SEL *)ERROR_PTR("sel data not made", procName, NULL);

    for (i = 0; i < sy; i++)
	for (j = 0; j < sx; j++)
	    csel->data[i][j] = sel->data[i][j];

    csel->name = stringNew(sel->name);

    return csel;
}



/*!
 *  selCreateBrick()
 *
 *      Input:  height, width
 *              cy, cx  (center, relative to UL corner at 0,0)
 *              type  (SEL_HIT, SEL_MISS, or SEL_DONT_CARE)
 *      Return: sel, or null on error
 *
 *  Action: a "brick" is a rectangular array of either hits,
 *          misses, or don't-cares.
 */
SEL *
selCreateBrick(l_int32  h,
	       l_int32  w,
	       l_int32  cy,
	       l_int32  cx,
	       l_int32  type)
{
l_int32  i, j;
SEL     *sel;

    PROCNAME("selCreateBrick");

    if (h <= 0 || w <= 0)
	return (SEL *)ERROR_PTR("h and w must both be > 0", procName, NULL);
    if (type != SEL_HIT && type != SEL_MISS && type != SEL_DONT_CARE)
	return (SEL *)ERROR_PTR("invalid sel element type", procName, NULL);

    if ((sel = selCreate(h, w, NULL)) == NULL)
	return (SEL *)ERROR_PTR("sel not made", procName, NULL);
    sel->cy = cy;
    sel->cx = cx;
    for (i = 0; i < h; i++)
	for (j = 0; j < w; j++)
	    sel->data[i][j] = type;

    return sel;
}


/*!
 *  create2dIntArray()
 *
 *      Input:  sy (rows == height)
 *              sx (columns == width)
 *      Return: doubly indexed array (i.e., an array of sy row pointers,
 *              each of which points to an array of sx ints)
 *
 *  Note: the array[sy][sx] is indexed in standard
 *        "matrix notation" with the row index first.
 */
l_int32 **
create2dIntArray(l_int32  sy,
                 l_int32  sx)
{
l_int32    i;
l_int32  **array;

    PROCNAME("create2dIntArray");

    if ((array = (l_int32 **)CALLOC(sy, sizeof(l_int32 *))) == NULL)
	return (l_int32 **)ERROR_PTR("ptr array not made", procName, NULL);

    for (i = 0; i < sy; i++) {
	if ((array[i] = (l_int32 *)CALLOC(sx, sizeof(l_int32))) == NULL)
	    return (l_int32 **)ERROR_PTR("array not made", procName, NULL);
    }

    return array;
}



/*------------------------------------------------------------------------*
 *                           Extension of sela                            *
 *------------------------------------------------------------------------*/
/*!
 *  selaAddSel()
 *
 *      Input:  sela
 *              sel to be added
 *              selname (ignored if already defined in sel;
 *                       req'd in sel when added to a sela)
 *              copyflag (for sel: 0 inserts, 1 copies)
 *      Return: 0 if OK; 1 on error
 *
 *  Action:  Adds sel to arrays, making a copy if flagged.
 *           Copies the name to the sel if necessary.
 *           Increments the sel count.
 */
l_int32
selaAddSel(SELA        *sela,
	   SEL         *sel,
	   const char  *selname,
	   l_int32      copyflag)
{
l_int32  n;
SEL     *csel;

    PROCNAME("selaAddSel");

    if (!sela)
	return ERROR_INT("sela not defined", procName, 1);
    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    if (!sel->name && !selname)
	return ERROR_INT("added sel must have name", procName, 1);

    if (copyflag == TRUE) {
	if ((csel = selCopy(sel)) == NULL)
	    return ERROR_INT("csel not made", procName, 1);
    }
    else   /* copyflag is false; insert directly */
	csel = sel;

    if (csel->name == NULL)
	csel->name = stringNew(selname);

    n = selaGetCount(sela);
    if (n >= sela->nalloc)
	selaExtendArray(sela);
    sela->sel[n] = csel;
    sela->n++;

    return 0;
}
    

/*!
 *  selaExtendArray()
 *
 *      Input:  sela
 *      Return: 0 if OK; 1 on error
 *
 *  Action: doubles the ptr array; copies the old ptr addresses
 *          into the new array; updates ptr array size
 */
l_int32
selaExtendArray(SELA  *sela)
{
    PROCNAME("selaExtendArray");

    if (!sela)
        return ERROR_INT("sela not defined", procName, 1);
    
    if ((sela->sel = (SEL **)reallocNew((void **)&sela->sel,
                              sizeof(l_intptr_t) * sela->nalloc,
                              2 * sizeof(l_intptr_t) * sela->nalloc)) == NULL)
	    return ERROR_INT("new ptr array not returned", procName, 1);

    sela->nalloc = 2 * sela->nalloc;
    return 0;
}



/*----------------------------------------------------------------------*
 *                               Accessors                              *
 *----------------------------------------------------------------------*/
/*!
 *  selaGetCount()
 *
 *      Input:  sela
 *      Return: count, or 0 on error
 */
l_int32
selaGetCount(SELA  *sela)
{
    PROCNAME("selaGetCount");

    if (!sela)
	return ERROR_INT("sela not defined", procName, 0);

    return sela->n;
}


/*!
 *  selaGetSel()
 *
 *      Input:  sela
 *              index of sel to be retrieved (not copied)
 *      Return: sel, or null on error
 *
 *  Notes:
 *      (1) This returns a ptr to the sel, not a copy, so the caller
 *          must not destroy it!
 */
SEL *
selaGetSel(SELA    *sela,
	   l_int32  i)
{
    PROCNAME("selaGetSel");

    if (!sela)
	return (SEL *)ERROR_PTR("sela not defined", procName, NULL);

    if (i < 0 || i >= sela->n)
	return (SEL *)ERROR_PTR("invalid index", procName, NULL);

    return sela->sel[i];
}


/*!
 *  selGetName()
 *
 *      Input:  sel
 *      Return: sel name (not copied), or null if no name or on error
 */
char *
selGetName(SEL  *sel)
{
    PROCNAME("selGetName");

    if (!sel)
	return (char *)ERROR_PTR("sel not defined", procName, NULL);

    return sel->name;
}


/*!
 *  selaFindSelByName()
 *
 *      Input:  sela
 *              sel name
 *              &index (<optional, return>)
 *              &sel  (<optional, return> sel (not a copy))
 *      Return: 0 if OK; 1 on error
 */
l_int32
selaFindSelByName(SELA        *sela, 
	          const char  *name,
		  l_int32     *pindex,
		  SEL        **psel)
{
l_int32  i, n;
char    *sname;
SEL     *sel;

    PROCNAME("selaFindSelByName");

    if (pindex) *pindex = -1;
    if (psel) *psel = NULL;

    if (!sela)
	return ERROR_INT("sela not defined", procName, 1);

    n = selaGetCount(sela);
    for (i = 0; i < n; i++)
    {
	if ((sel = selaGetSel(sela, i)) == NULL) {
	    L_WARNING("missing sel", procName);
	    continue;
	}
	    
	sname = selGetName(sel);
	if (sname && (!strcmp(name, sname))) {
            if (pindex)
                *pindex = i;
	    if (psel)
                *psel = sel;
	    return 0;
	}
    }
    
    return 1;
}


/*!
 *  selGetElement()
 *
 *      Input:  sel
 *              row
 *              col
 *              &type  (<return> SEL_HIT, SEL_MISS, SEL_DONT_CARE)
 *      Return: 0 if OK; 1 on error
 */
l_int32
selGetElement(SEL      *sel,
              l_int32   row,
	      l_int32   col,
	      l_int32  *ptype)
{
    PROCNAME("selGetElement");

    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    if (row < 0 || row >= sel->sy)
	return ERROR_INT("sel row out of bounds", procName, 1);
    if (col < 0 || col >= sel->sx)
	return ERROR_INT("sel col out of bounds", procName, 1);

    *ptype = sel->data[row][col];
    return 0;
}


/*!
 *  selSetElement()
 *
 *      Input:  sel
 *              row
 *              col
 *              type  (SEL_HIT, SEL_MISS, SEL_DONT_CARE)
 *      Return: 0 if OK; 1 on error
 */
l_int32
selSetElement(SEL     *sel,
              l_int32  row,
	      l_int32  col,
	      l_int32  type)
{
    PROCNAME("selSetElement");

    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    if (type != SEL_HIT && type != SEL_MISS && type != SEL_DONT_CARE)
	return ERROR_INT("invalid sel element type", procName, 1);
    if (row < 0 || row >= sel->sy)
	return ERROR_INT("sel row out of bounds", procName, 1);
    if (col < 0 || col >= sel->sx)
	return ERROR_INT("sel col out of bounds", procName, 1);

    sel->data[row][col] = type;
    return 0;
}


/*!
 *  selGetParameters()
 *
 *      Input:  sel
 *              &sy, &sx, &cy, &cx (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
selGetParameters(SEL      *sel,
                 l_int32  *psy,
                 l_int32  *psx,
                 l_int32  *pcy,
                 l_int32  *pcx)
{
    PROCNAME("selSetElement");

    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    if (psy) *psy = sel->sy; 
    if (psx) *psx = sel->sx; 
    if (pcy) *pcy = sel->cy; 
    if (pcx) *pcx = sel->cx; 
    return 0;
}


/*!
 *  selaGetBrickName()
 *
 *      Input:  sela
 *              hsize, vsize (of brick sel)
 *      Return: sel name (new string), or null if no name or on error
 */
char *
selaGetBrickName(SELA    *sela,
		 l_int32  hsize,
		 l_int32  vsize)
{
l_int32  i, nsels, sx, sy;
SEL     *sel;

    PROCNAME("selaGetBrickName");

    if (!sela)
	return (char *)ERROR_PTR("sela not defined", procName, NULL);

    nsels = selaGetCount(sela);
    for (i = 0; i < nsels; i++) {
        sel = selaGetSel(sela, i);
	selGetParameters(sel, &sy, &sx, NULL, NULL);
	if (hsize == sx && vsize == sy)
            return stringNew(selGetName(sel));
    }

    return NULL;
}



/*----------------------------------------------------------------------*
 *                Max translations for erosion and hmt                  *
 *----------------------------------------------------------------------*/
/*!
 *  selFindMaxTranslations()
 *
 *      Input:  sel
 *              &xp, &yp, &xn, &yn  (<return> max shifts)
 *      Return: 0 if OK; 1 on error
 *
 *  Note: these are the maximum shifts for the erosion operation.
 *        For example, when j < cx, the shift of the image
 *        is +x to the cx.  This is a positive xp shift.
 */
l_int32
selFindMaxTranslations(SEL      *sel,
                       l_int32  *pxp,
                       l_int32  *pyp,
                       l_int32  *pxn,
                       l_int32  *pyn)
{
l_int32  sx, sy, cx, cy, i, j;
l_int32  maxxp, maxyp, maxxn, maxyn;

    PROCNAME("selaFindMaxTranslations");

    if (!pxp || !pyp || !pxn || !pyn)
	return ERROR_INT("&xp (etc) defined", procName, 1);
    *pxp = *pyp = *pxn = *pyn = 0;
    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    selGetParameters(sel, &sy, &sx, &cy, &cx);

    maxxp = maxyp = maxxn = maxyn = 0;
    for (i = 0; i < sy; i++) {
	for (j = 0; j < sx; j++) {
	    if (sel->data[i][j] == 1) {
		maxxp = L_MAX(maxxp, cx - j);
		maxyp = L_MAX(maxyp, cy - i);
		maxxn = L_MAX(maxxn, j - cx);
		maxyn = L_MAX(maxyn, i - cy);
	    }
	}
    }

    *pxp = maxxp;
    *pyp = maxyp;
    *pxn = maxxn;
    *pyn = maxyn;

    return 0;
}



/*----------------------------------------------------------------------*
 *                    I/O and visualizing Sela & Sel                    *
 *----------------------------------------------------------------------*/
/*!
 *  selaWrite()
 *
 *      Input:  filename
 *              sela
 *      Return: 0 if OK, 1 on error
 */
l_int32
selaWrite(const char  *fname,
	  SELA        *sela)
{
FILE  *fp;

    PROCNAME("selaWrite");

    if (!fname)
	return ERROR_INT("fname not defined", procName, 1);
    if (!sela)
	return ERROR_INT("sela not defined", procName, 1);

    if ((fp = fopen(fname, "wb")) == NULL)
	return ERROR_INT("stream not opened", procName, 1);
    selaWriteStream(fp, sela);
    fclose(fp);

    return 0;
}


/*!
 *  selaWriteStream()
 *
 *      Input:  stream
 *              sela
 *      Return: 0 if OK, 1 on error
 */
l_int32
selaWriteStream(FILE  *fp,
	        SELA  *sela)
{
l_int32  i, n;
SEL     *sel;

    PROCNAME("selaWriteStream");

    if (!fp)
	return ERROR_INT("stream not defined", procName, 1);
    if (!sela)
	return ERROR_INT("sela not defined", procName, 1);

    n = selaGetCount(sela);
    fprintf(fp, "sel array: number of sels = %d\n\n", n);
    for (i = 0; i < n; i++)
    {
	if ((sel = selaGetSel(sela, i)) == NULL)
	    continue;
	selWriteStream(fp, sel);
    }
    return 0;
}


/*!
 *  selWriteStream()
 *
 *      Input:  stream
 *              sel
 *      Return: 0 if OK, 1 on error
 */
l_int32
selWriteStream(FILE  *fp,
	       SEL   *sel)
{
l_int32  sx, sy, cx, cy, i, j;

    PROCNAME("selWriteStream");

    if (!fp)
	return ERROR_INT("stream not defined", procName, 1);
    if (!sel)
	return ERROR_INT("sel not defined", procName, 1);
    selGetParameters(sel, &sy, &sx, &cy, &cx);

    fprintf(fp, "  ------  %s  ------\n", selGetName(sel));
    fprintf(fp, "  sy = %d, sx = %d, cy = %d, cx = %d\n", sy, sx, cy, cx);
    for (i = 0; i < sy; i++) {
	fprintf(fp, "    ");
	for (j = 0; j < sx; j++)
	    fprintf(fp, "%d", sel->data[i][j]);
	fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    return 0;
}


/*!
 *  selaRead()
 *
 *      Input:  filename
 *      Return: sela, or null on error
 */
SELA  *
selaRead(const char  *fname)
{
FILE  *fp;
SELA  *sela;

    PROCNAME("selaRead");

    if (!fname)
	return (SELA *)ERROR_PTR("fname not defined", procName, NULL);

    if ((fp = fopen(fname, "rb")) == NULL)
	return (SELA *)ERROR_PTR("stream not opened", procName, NULL);
    if ((sela = selaReadStream(fp)) == NULL)
	return (SELA *)ERROR_PTR("sela not returned", procName, NULL);
    fclose(fp);

    return sela;
}


/*!
 *  selaReadStream()
 *
 *      Input:  stream
 *      Return: sela, or null on error
 */
SELA  *
selaReadStream(FILE  *fp)
{
l_int32  i, n;
SEL     *sel;
SELA    *sela;

    PROCNAME("selaReadStream");

    if (!fp)
	return (SELA *)ERROR_PTR("stream not defined", procName, NULL);

    if (fscanf(fp, "sel array: number of sels = %d\n\n", &n) != 1)
	return (SELA *)ERROR_PTR("not a sela", procName, NULL);

    if ((sela = selaCreate(n)) == NULL)
	return (SELA *)ERROR_PTR("sela not made", procName, NULL);
    sela->nalloc = n;

    for (i = 0; i < n; i++)
    {
	if ((sel = selReadStream(fp)) == NULL)
	    return (SELA *)ERROR_PTR("sel not made", procName, NULL);
	selaAddSel(sela, sel, NULL, 0);
    }

    return sela;
}


/*!
 *  selReadStream()
 *
 *      Input:  stream
 *      Return: sel, or null on error
 */
SEL  *
selReadStream(FILE  *fp)
{
char    *selname;
char     linebuf[L_BUF_SIZE];
l_int32  sy, sx, cy, cx, i, j;
SEL     *sel;

    PROCNAME("selReadStream");

    if (!fp)
	return (SEL *)ERROR_PTR("stream not defined", procName, NULL);

    fgets(linebuf, L_BUF_SIZE, fp);
    selname = stringNew(linebuf);
    sscanf(linebuf, "  ------  %s  ------", selname);

    if (fscanf(fp, "  sy = %d, sx = %d, cy = %d, cx = %d\n",
	    &sy, &sx, &cy, &cx) != 4)
	return (SEL *)ERROR_PTR("dimensions not read", procName, NULL);

    if ((sel = selCreate(sy, sx, selname)) == NULL)
	return (SEL *)ERROR_PTR("sel not made", procName, NULL);
    sel->cy = cy;
    sel->cx = cx;

    for (i = 0; i < sy; i++) {
	fscanf(fp, "    ");
	for (j = 0; j < sx; j++)
	    fscanf(fp, "%1d", &sel->data[i][j]);
	fscanf(fp, "\n");
    }
    fscanf(fp, "\n");

    FREE(selname);
    return sel;
}


/*!
 *  selCreateFromString()
 *
 *      Input:  text
 *              height, width
 *              name (<optional> sel name; can be null)
 *      Return: sel of the given size, or null on error
 *
 *  Notes:
 *      (1) The text is an array of chars (in row-major order) where
 *          each char can be one of the following:
 *             'x': hit
 *             'o': miss
 *             ' ': don't-care
 *      (2) Use an upper case char to indicate the origin of the Sel.
 *          When the origin falls on a don't-care, use 'C' as the uppecase
 *          for ' '.
 *      (3) The text can be input in a format that shows the 2D layout; e.g.,
 *              static const char *seltext = "x    "
 *                                           "x Oo "
 *                                           "x    "
 *                                           "xxxxx";
 */
SEL *
selCreateFromString(const char  *text,
                    l_int32      h,
                    l_int32      w,
                    const char  *name)
{
SEL     *sel;
l_int32  y, x;
char     ch;

    PROCNAME("selCreateFromString");

    if (h < 1)
        return (SEL *)ERROR_PTR("height must be > 0", procName, NULL);
    if (w < 1)
        return (SEL *)ERROR_PTR("width must be > 0", procName, NULL);
    
    sel = selCreate(h, w, name);

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            ch = *(text++);
            switch (ch)
            {
                case 'X':
                    sel->cx = x;
                    sel->cy = y;
                case 'x':
                    selSetElement(sel, y, x, SEL_HIT);
                    break;

                case 'O':
                    sel->cx = x;
                    sel->cy = y;
                case 'o':
                    selSetElement(sel, y, x, SEL_MISS);
                    break;

                case 'C':
                    sel->cx = x;
                    sel->cy = y;
                case ' ':
                    selSetElement(sel, y, x, SEL_DONT_CARE);
                    break;

                case '\n':
                    /* ignored */
                    continue;

                default:
                    selDestroy(&sel);
                    return (SEL *)ERROR_PTR("unknown char", procName, NULL);
            }
        }
    }

    return sel;
}


/*!
 *  selPrintToString()
 *
 *      Input:  sel
 *      Return: string (caller must free)
 *
 *  Notes:
 *      (1) This is an inverse function of selCreateFromString.
 *          It prints a textual representation of the SEL to a malloc'd
 *          string.  The format is the same as selCreateFromString
 *          except that newlines are inserted into the output
 *          between rows.
 *      (2) This is useful for debugging.  However, if you want to
 *          save some Sels in a file, put them in a Sela and write
 *          them out with selaWrite().  They can then be read in
 *          with selaRead().
 */
char *
selPrintToString(SEL  *sel)
{
char     is_center;
char    *string;
l_int32  type;
l_int32  sx, sy, cx, cy, x, y;

    PROCNAME("selPrintToString");

    if (!sel)
        return (char *)ERROR_PTR("sel not defined", procName, NULL);

    selGetParameters(sel, &sy, &sx, &cy, &cx);
    if ((string = (char *)CALLOC(1, sy * (sx + 1) + 1)) == NULL)
        return (char *)ERROR_PTR("calloc fail for string", procName, NULL);
    for (y = 0; y < sy; ++y) {
        for (x = 0; x < sx; ++x) {
            selGetElement(sel, y, x, &type);
            is_center = (x == cx && y == cy);
            switch (type) {
                case SEL_HIT:
                    *(string++) = is_center ? 'X' : 'x';
                    break;
                case SEL_MISS:
                    *(string++) = is_center ? 'O' : 'o';
                    break;
                case SEL_DONT_CARE:
                    *(string++) = is_center ? 'C' : ' ';
                    break;
            }
        }
        *(string++) = '\n';
    }

    return string;
}


