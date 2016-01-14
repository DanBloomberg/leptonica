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
 *  pngiostub.c
 *
 *     Stubs for pngio.c functions
 */

PIX * pixReadStreamPng ( FILE *fp )
{
    PROCNAME("pixReadStreamPng");
    return (PIX * )ERROR_PTR("function not present", procName, NULL);
}

l_int32 pixWriteStreamPng ( FILE *fp, PIX *pix )
{
    PROCNAME("pixWriteStreamPng");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 readHeaderPng ( const char *filename, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbpc, l_int32 *pcpp, l_int32 *pcmap )
{
    PROCNAME("readHeaderPng");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 freadHeaderPng ( FILE *fp, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbpc, l_int32 *pcpp, l_int32 *pcmap )
{
    PROCNAME("freadHeaderPng");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 sreadHeaderPng ( const l_uint8 *data, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbpc, l_int32 *pcpp, l_int32 *pcmap )
{
    PROCNAME("sreadHeaderPng");
    return ERROR_INT("function not present", procName, 1);
}

