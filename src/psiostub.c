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
 *  psiostub.c
 *
 *     Stubs for psio.c functions
 */

l_int32 convertToPSEmbed ( const char *filein, const char *fileout, l_int32 level )
{
    PROCNAME("convertToPSEmbed");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWritePSEmbed ( const char *filein, const char *fileout )
{
    PROCNAME("pixWritePSEmbed");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 pixWriteStreamPS ( FILE *fp, PIX *pix, BOX *box, l_int32 res, l_float32 scale )
{
    PROCNAME("pixWriteStreamPS");
    return ERROR_INT("function not present", procName, 1);
}

char * pixWriteStringPS ( PIX *pixs, BOX *box, l_int32 res, l_float32 scale )
{
    PROCNAME("pixWriteStringPS");
    return (char * )ERROR_PTR("function not present", procName, NULL);
}

void getScaledParametersPS ( BOX *box, l_int32 wpix, l_int32 hpix, l_int32 res, l_float32 scale, l_float32 *pxpt, l_float32 *pypt, l_float32 *pwpt, l_float32 *phpt )
{
    PROCNAME("getScaledParametersPS");
    ERROR_VOID("function not present", procName);
    return;
}

void convertByteToHexAscii ( l_uint8 byteval, char *pnib1, char *pnib2 )
{
    PROCNAME("convertByteToHexAscii");
    ERROR_VOID("function not present", procName);
    return;
}

l_int32 convertJpegToPSEmbed ( const char *filein, const char *fileout )
{
    PROCNAME("convertJpegToPSEmbed");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertJpegToPS ( const char *filein, const char *fileout, const char *operation, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 endpage )
{
    PROCNAME("convertJpegToPS");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertJpegToPSString ( const char *filein, char **poutstr, l_int32 *pnbytes, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 endpage )
{
    PROCNAME("convertJpegToPSString");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 extractJpegDataFromFile ( const char *filein, l_uint8 **pdata, l_int32 *pnbytes, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp )
{
    PROCNAME("extractJpegDataFromFile");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 extractJpegDataFromArray ( const void *data, l_int32 nbytes, l_int32 *pw, l_int32 *ph, l_int32 *pbps, l_int32 *pspp )
{
    PROCNAME("extractJpegDataFromArray");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertTiffG4ToPSEmbed ( const char *filein, const char *fileout )
{
    PROCNAME("convertTiffG4ToPSEmbed");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertTiffG4ToPS ( const char *filein, const char *fileout, const char *operation, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 mask, l_int32 endpage )
{
    PROCNAME("convertTiffG4ToPS");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertTiffG4ToPSString ( const char *filein, char **poutstr, l_int32 *pnbytes, l_int32 x, l_int32 y, l_int32 res, l_float32 scale, l_int32 pageno, l_int32 mask, l_int32 endpage )
{
    PROCNAME("convertTiffG4ToPSString");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 extractTiffG4DataFromFile ( const char *filein, l_uint8 **pdata, l_int32 *pnbytes, l_int32 *pw, l_int32 *ph, l_int32 *pminisblack )
{
    PROCNAME("extractTiffG4DataFromFile");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 convertTiffMultipageToPS ( const char *filein, const char *fileout, const char *tempfile, l_float32 fillfract )
{
    PROCNAME("convertTiffMultipageToPS");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 getResLetterPage ( l_int32 w, l_int32 h, l_float32 fillfract )
{
    PROCNAME("getResLetterPage");
    return ERROR_INT("function not present", procName, 1);
}

l_int32 getResA4Page ( l_int32 w, l_int32 h, l_float32 fillfract )
{
    PROCNAME("getResA4Page");
    return ERROR_INT("function not present", procName, 1);
}

char * encodeAscii85 ( l_uint8 *inarray, l_int32 insize, l_int32 *poutsize )
{
    PROCNAME("encodeAscii85");
    return (char * )ERROR_PTR("function not present", procName, NULL);
}

l_int32 convertChunkToAscii85 ( l_uint8 *inarray, l_int32 insize, l_int32 *pindex, char *outbuf, l_int32 *pnbout )
{
    PROCNAME("convertChunkToAscii85");
    return ERROR_INT("function not present", procName, 1);
}

l_uint8 * decodeAscii85 ( char *ina, l_int32 insize, l_int32 *poutsize )
{
    PROCNAME("decodeAscii85");
    return (l_uint8 * )ERROR_PTR("function not present", procName, NULL);
}

