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
 *  tiffiostub.c
 *
 *     Stubs for tiffio.c functions
 */

PIX * pixReadTiff ( const char *filename, l_int32 n )
{
    PROCNAME("pixReadTiff");
    return (PIX * )ERROR_PTR("function not present", procName, NULL);
}

PIX * pixReadStreamTiff ( FILE *fp, l_int32 n )
{
    PROCNAME("pixReadStreamTiff");
    return (PIX * )ERROR_PTR("function not present", procName, NULL);
}

l_int32 pixWriteTiffCustom ( const char *filename, PIX *pix, l_int32 comptype, const char *modestring, NUMA *natags, SARRAY *savals, SARRAY *satypes, NUMA *nasizes )
{
    PROCNAME("pixWriteTiffCustom");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWriteTiff ( const char *filename, PIX *pix, l_int32 comptype, const char *modestring )
{
    PROCNAME("pixWriteTiff");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWriteStreamTiff ( FILE *fp, PIX *pix, l_int32 comptype )
{
    PROCNAME("pixWriteStreamTiff");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWriteToTiffStream ( TIFF *tif, PIX *pix, l_int32 comptype, NUMA *natags, SARRAY *savals, SARRAY *satypes, NUMA *nasizes )
{
    PROCNAME("pixWriteToTiffStream");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 fprintTiffInfo ( FILE *fpout, const char *tiffile )
{
    PROCNAME("fprintTiffInfo");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 tiffGetCount ( FILE *fp, l_int32 *pn )
{
    PROCNAME("tiffGetCount");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 readHeaderTiff ( const char *filename, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbps, l_int32 *pspp, l_int32 *pres, l_int32 *pcmap )
{
    PROCNAME("readHeaderTiff");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 freadHeaderTiff ( FILE *fp, l_int32 *pwidth, l_int32 *pheight, l_int32 *pbps, l_int32 *pspp, l_int32 *pres, l_int32 *pcmap )
{
    PROCNAME("freadHeaderTiff");
    return ERROR_INT("function not present", procName, 1);
}

