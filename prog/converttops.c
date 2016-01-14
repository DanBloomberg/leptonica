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
 * converttops.c
 *
 *   Syntax: converttops filein fileout [level]
 *
 *      where level = {1,2} and 2 is the default
 *
 *   This program converts any of the supported images (png,
 *   jpeg, tiff, bmp, png) into a PS file that prints at the
 *   highest resolution possible that fills the page.
 *   Images with colormaps are converted to grayscale or rgb.
 *   The output PS file is either uncompressed (level 1) or
 *   compressed (level2). 
 *
 *   Details of imaging call sequence for level 1:
 *      convertToPSEmbed()
 *         --> pixWritePSEmbed()
 *            --> pixWriteStreamPS()
 *               --> pixConvertForPSWrap() ...
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char        *filein, *fileout;
char         error_msg[] = " ps level = {1,2}; level 2 is default";
l_int32      level;
PIX         *pix, *pixs;
static char  mainName[] = "converttops";

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Syntax: converttops filein fileout [level]\n");
        fprintf(stderr, "%s\n", error_msg);
        return 1;
    }

    filein = argv[1];
    fileout = argv[2];

    level = 2;
    if (argc == 4) {
        level = atoi(argv[3]);
	if (level != 1 && level != 2) {
	    L_WARNING("ps level must be 1 or 2; setting to 2", mainName);
	    level = 2;
	}
    }

    convertToPSEmbed(filein, fileout, level);
    return 0;
}

