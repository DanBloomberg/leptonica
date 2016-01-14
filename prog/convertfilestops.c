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
 * convertfilestops.c
 *
 *    Converts all files in the given directory with matching substring
 *    to a level 2 compressed PostScript file, at the specified resolution.
 *
 *    Decreasing the resolution will cause the image to be rendered
 *    larger, and v.v.
 *
 *    Note: this program only runs under Unix; it will not compile under cygwin.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char           *dirin, *substr, *fileout;
l_int32         res;
static char     mainName[] = "convertfilestops";

    if (argc != 5)
	exit(ERROR_INT(" Syntax:  convertfilestops dirin substr res fileout",
                       mainName, 1));

    dirin = argv[1];
    substr = argv[2];
    res = atoi(argv[3]);
    fileout = argv[4];

    return convertFilesToPS(dirin, substr, res, fileout);
}


