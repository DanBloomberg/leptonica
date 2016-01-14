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
 *  binexpand.c
 *
 *      Power of 2 expansion
 *         PIX     *pixExpandBinary()
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "allheaders.h"


/*------------------------------------------------------------------*
 *                      Power of 2 expansion                        *
 *------------------------------------------------------------------*/
/*!
 *  pixExpandBinary()
 *
 *      Input:  pixs
 *              factor
 *      Return: pixd (expanded pix), or null on error
 */
PIX *
pixExpandBinary(PIX     *pixs,
	        l_int32  factor)
{
l_int32    ws, hs, wd, hd, wpls, wpld;
l_uint32  *datas, *datad;
PIX       *pixd;

    PROCNAME("pixExpandBinary");

    if (!pixs)
        return (PIX *)ERROR_PTR("pixs not defined", procName, NULL);
    if (factor != 2 && factor != 4 && factor != 8 && factor != 16)
        return (PIX *)ERROR_PTR("factor must be in {2,4,8,16}", procName, NULL);

    if (pixGetDepth(pixs) != 1)
        return (PIX *)ERROR_PTR("pixs not binary", procName, NULL);

    ws = pixGetWidth(pixs);
    hs = pixGetHeight(pixs);
    wpls = pixGetWpl(pixs);
    datas = pixGetData(pixs);

    wd = factor * ws;
    hd = factor * hs;
    if ((pixd = pixCreate(wd, hd, 1)) == NULL)
        return (PIX *)ERROR_PTR("pixd not made", procName, NULL);
    pixCopyResolution(pixd, pixs);
    pixScaleResolution(pixd, (l_float32)factor, (l_float32)factor);
    wpld = pixGetWpl(pixd);
    datad = pixGetData(pixd);

    expandBinaryLow(datad, wd, hd, wpld, datas, ws, hs, wpls, factor);

    return pixd;
}

