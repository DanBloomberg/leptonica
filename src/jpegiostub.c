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

#include <stdio.h>
#include "allheaders.h"

/*
 *  jpegiostub.c
 *
 *     Stubs for jpegio.c functions
 */

PIX * pixReadJpeg ( const char *filename, l_int32 cmflag, l_int32 reduction, l_int32 *pnwarn )
{
    PROCNAME("pixReadJpeg");
    return (PIX * )ERROR_PTR("function not present", procName, NULL);
}

PIX * pixReadStreamJpeg ( FILE *fp, l_int32 cmflag, l_int32 reduction, l_int32 *pnwarn, l_int32 hint )
{
    PROCNAME("pixReadStreamJpeg");
    return (PIX * )ERROR_PTR("function not present", procName, NULL);
}

l_int32 pixWriteJpeg ( const char *filename, PIX *pix, l_int32 quality, l_int32 progressive )
{
    PROCNAME("pixWriteJpeg");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWriteStreamJpeg ( FILE *fp, PIX *pix, l_int32 quality, l_int32 progressive )
{
    PROCNAME("pixWriteStreamJpeg");
    return ERROR_INT("function not present", procName, 1);
}

