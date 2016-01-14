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
 *  fpix1.c
 *
 *    This file has basic constructors, destructors and field accessors
 *    for FPix
 *
 *    Create/copy/destroy
 *          FPIX          *fpixCreate()
 *          FPIX          *fpixCreateTemplate()
 *          FPIX          *fpixClone()
 *          FPIX          *fpixCopy()
 *          l_int32        fpixResizeImageData()
 *          void           fpixDestroy()
 *
 *    FPix accessors
 *          l_int32        fpixGetDimensions()
 *          l_int32        fpixSetDimensions()
 *          l_int32        fpixGetWpl()
 *          l_int32        fpixSetWpl()
 *          l_int32        fpixGetRefcount()
 *          l_int32        fpixChangeRefcount()
 *          l_int32        fpixGetResolution()
 *          l_int32        fpixSetResolution()
 *          l_int32        fpixCopyResolution()
 *          l_float32     *fpixGetData()
 *          l_int32        fpixSetData()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


/*--------------------------------------------------------------------*
 *                     FPix Create/copy/destroy                       *
 *--------------------------------------------------------------------*/
/*!
 *  fpixCreate()
 *
 *      Input:  width, height
 *      Return: fpixd (with data allocated and initialized to 0),
 *                     or null on error
 *
 *  Notes:
 *      (1) Makes a FPix of specified size, with the data array
 *          allocated and initialized to 0.
 */
FPIX *
fpixCreate(l_int32  width,
           l_int32  height)
{
l_float32  *data;
FPIX       *fpixd;

    PROCNAME("fpixCreate");

    if (width <= 0)
        return (FPIX *)ERROR_PTR("width must be > 0", procName, NULL);
    if (height <= 0)
        return (FPIX *)ERROR_PTR("height must be > 0", procName, NULL);

    if ((fpixd = (FPIX *)CALLOC(1, sizeof(FPIX))) == NULL)
        return (FPIX *)ERROR_PTR("CALLOC fail for fpixd", procName, NULL);
    fpixSetDimensions(fpixd, width, height);
    fpixSetWpl(fpixd, width);
    fpixd->refcount = 1;

    data = (l_float32 *)CALLOC(4 * width * height, sizeof(l_float32));
    if (!data)
        return (FPIX *)ERROR_PTR("CALLOC fail for data", procName, NULL);
    fpixSetData(fpixd, data);

    return fpixd;
}


/*!
 *  fpixCreateTemplate()
 *
 *      Input:  fpixs
 *      Return: fpixd, or null on error
 *
 *  Notes:
 *      (1) Makes a FPix of the same size as the input FPix, with the
 *          data array allocated and initialized to 0.
 *      (2) Copies the resolution.
 */
FPIX *
fpixCreateTemplate(FPIX  *fpixs)
{
l_int32  w, h;
FPIX    *fpixd;

    PROCNAME("fpixCreateTemplate");

    if (!fpixs)
        return (FPIX *)ERROR_PTR("fpixs not defined", procName, NULL);

    fpixGetDimensions(fpixs, &w, &h);
    fpixd = fpixCreate(w, h);
    fpixCopyResolution(fpixd, fpixs);
    return fpixd;
}


/*!
 *  fpixClone()
 *
 *      Input:  fpix
 *      Return: same fpix (ptr), or null on error
 *
 *  Notes:
 *      (1) See pixClone() for definition and usage.
 */
FPIX *
fpixClone(FPIX  *fpix)
{
    PROCNAME("fpixClone");

    if (!fpix)
        return (FPIX *)ERROR_PTR("fpix not defined", procName, NULL);
    fpixChangeRefcount(fpix, 1);

    return fpix;
}


/*!
 *  fpixCopy()
 *
 *      Input:  fpixd (<optional>; can be null, or equal to fpixs,
 *                    or different from fpixs)
 *              fpixs
 *      Return: fpixd, or null on error
 *
 *  Notes:
 *      (1) There are three cases:
 *            (a) fpixd == null  (makes a new fpix; refcount = 1)
 *            (b) fpixd == fpixs  (no-op)
 *            (c) fpixd != fpixs  (data copy; no change in refcount)
 *          If the refcount of fpixd > 1, case (c) will side-effect
 *          these handles.
 *      (2) The general pattern of use is:
 *             fpixd = fpixCopy(fpixd, fpixs);
 *          This will work for all three cases.
 *          For clarity when the case is known, you can use:
 *            (a) fpixd = fpixCopy(NULL, fpixs);
 *            (c) fpixCopy(fpixd, fpixs);
 *      (3) For case (c), we check if fpixs and fpixd are the same size.
 *          If so, the data is copied directly.
 *          Otherwise, the data is reallocated to the correct size
 *          and the copy proceeds.  The refcount of fpixd is unchanged.
 *      (4) This operation, like all others that may involve a pre-existing
 *          fpixd, will side-effect any existing clones of fpixd.
 */
FPIX *
fpixCopy(FPIX  *fpixd,   /* can be null */
         FPIX  *fpixs)
{
l_int32     w, h, bytes;
l_float32  *datas, *datad;

    PROCNAME("fpixCopy");

    if (!fpixs)
        return (FPIX *)ERROR_PTR("fpixs not defined", procName, NULL);
    if (fpixs == fpixd)
        return fpixd;

        /* Total bytes in image data */
    fpixGetDimensions(fpixs, &w, &h);
    bytes = 4 * w * h;

        /* If we're making a new fpix ... */
    if (!fpixd) {
        if ((fpixd = fpixCreateTemplate(fpixs)) == NULL)
            return (FPIX *)ERROR_PTR("fpixd not made", procName, NULL);
        datas = fpixGetData(fpixs);
        datad = fpixGetData(fpixd);
        memcpy((char *)datad, (char *)datas, bytes);
        return fpixd;
    }

        /* Reallocate image data if sizes are different */
    fpixResizeImageData(fpixd, fpixs);

        /* Copy data */
    fpixCopyResolution(fpixd, fpixs);
    datas = fpixGetData(fpixs);
    datad = fpixGetData(fpixd);
    memcpy((char*)datad, (char*)datas, bytes);
    return fpixd;
}


/*!
 *  fpixResizeImageData()
 *
 *      Input:  fpixd, fpixs
 *      Return: 0 if OK, 1 on error
 */
l_int32
fpixResizeImageData(FPIX  *fpixd,
                    FPIX  *fpixs)
{
l_int32     ws, hs, wd, hd, bytes;
l_float32  *data;

    PROCNAME("fpixResizeImageData");

    if (!fpixs)
        return ERROR_INT("fpixs not defined", procName, 1);
    if (!fpixd)
        return ERROR_INT("fpixd not defined", procName, 1);

    fpixGetDimensions(fpixs, &ws, &hs);
    fpixGetDimensions(fpixd, &wd, &hd);
    if (ws == wd && hs == hd)  /* nothing to do */
        return 0;

    fpixSetDimensions(fpixd, ws, hs);
    fpixSetWpl(fpixd, ws);
    bytes = 4 * ws * hs;
    data = fpixGetData(fpixd);
    if (data) FREE(data);
    if ((data = (l_float32 *)MALLOC(bytes)) == NULL)
        return ERROR_INT("MALLOC fail for data", procName, 1);
    fpixSetData(fpixd, data);
    return 0;
}


/*!
 *  fpixDestroy()
 *
 *      Input:  &fpix <will be nulled>
 *      Return: void
 *
 *  Notes:
 *      (1) Decrements the ref count and, if 0, destroys the fpix.
 *      (2) Always nulls the input ptr.
 */
void
fpixDestroy(FPIX  **pfpix)
{
l_float32  *data;
FPIX       *fpix;

    PROCNAME("fpixDestroy");

    if (!pfpix) {
        L_WARNING("ptr address is null!", procName);
        return;
    }

    if ((fpix = *pfpix) == NULL)
        return;

        /* Decrement the ref count.  If it is 0, destroy the fpix. */
    fpixChangeRefcount(fpix, -1);
    if (fpixGetRefcount(fpix) <= 0) {
        if ((data = fpixGetData(fpix)) != NULL)
            FREE(data);
        FREE(fpix);
    }

    *pfpix = NULL;
    return;
}


/*--------------------------------------------------------------------*
 *                                Accessors                           *
 *--------------------------------------------------------------------*/
/*!
 *  fpixGetDimensions()
 *
 *      Input:  fpix
 *              &w, &h (<optional return>; each can be null)
 *      Return: 0 if OK, 1 on error
 */
l_int32
fpixGetDimensions(FPIX     *fpix,
                  l_int32  *pw,
                  l_int32  *ph)
{
    PROCNAME("fpixGetDimensions");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);
    if (pw) *pw = fpix->w;
    if (ph) *ph = fpix->h;
    return 0;
}


/*!
 *  fpixSetDimensions()
 *
 *      Input:  fpix
 *              w, h
 *      Return: 0 if OK, 1 on error
 */
l_int32
fpixSetDimensions(FPIX     *fpix,
                  l_int32   w,
                  l_int32   h)
{
    PROCNAME("fpixSetDimensions");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);
    fpix->w = w;
    fpix->h = h;
    return 0;
}


l_int32
fpixGetWpl(FPIX  *fpix)
{
    PROCNAME("fpixGetWpl");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);
    return fpix->wpl;
}


l_int32
fpixSetWpl(FPIX    *fpix,
           l_int32  wpl)
{
    PROCNAME("fpixSetWpl");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);

    fpix->wpl = wpl;
    return 0;
}


l_int32
fpixGetRefcount(FPIX  *fpix)
{
    PROCNAME("fpixGetRefcount");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, UNDEF);
    return fpix->refcount;
}


l_int32
fpixChangeRefcount(FPIX    *fpix,
                   l_int32  delta)
{
    PROCNAME("fpixChangeRefcount");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);

    fpix->refcount += delta;
    return 0;
}


l_int32
fpixGetResolution(FPIX     *fpix,
                  l_int32  *pxres,
                  l_int32  *pyres)
{
    PROCNAME("fpixGetResolution");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);
    if (pxres) *pxres = fpix->xres;
    if (pyres) *pyres = fpix->yres;
    return 0;
}


l_int32
fpixSetResolution(FPIX    *fpix,
                  l_int32  xres,
                  l_int32  yres)
{
    PROCNAME("fpixSetResolution");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);

    fpix->xres = xres;
    fpix->yres = yres;
    return 0;
}


l_int32
fpixCopyResolution(FPIX  *fpixd,
                   FPIX  *fpixs)
{
l_int32  xres, yres;
    PROCNAME("fpixCopyResolution");

    if (!fpixs || !fpixd)
        return ERROR_INT("pixs and pixd not both defined", procName, 1);

    fpixGetResolution(fpixs, &xres, &yres);
    fpixSetResolution(fpixd, xres, yres);
    return 0;
}


l_float32 *
fpixGetData(FPIX  *fpix)
{
    PROCNAME("fpixGetData");

    if (!fpix)
        return (l_float32 *)ERROR_PTR("fpix not defined", procName, NULL);
    return fpix->data;
}


l_int32
fpixSetData(FPIX       *fpix,
            l_float32  *data)
{
    PROCNAME("fpixSetData");

    if (!fpix)
        return ERROR_INT("fpix not defined", procName, 1);

    fpix->data = data;
    return 0;
}


