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
 * ioformats.c
 *
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *    This is the primary Leptonica regression test for
 *    read/write I/O to standard image files (png, jpeg, etc.)
 *    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *    This tests reading and writing of images in different formats
 *    It should work properly on input images of any depth, with
 *    and without colormaps.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define   BMP_FILE      "test1.bmp"
#define   FILE_1BPP     "feyn.tif"
#define   FILE_2BPP     "weasel2.4g.png"
#define   FILE_4BPP     "weasel-16c.png"
#define   FILE_8BPP_1   "dreyfus8.png"
#define   FILE_8BPP_2   "weasel-240c.png"
#define   FILE_8BPP_3   "test8.jpg"
#define   FILE_32BPP    "marge.jpg"


main(int    argc,
     char **argv)
{
static char  mainName[] = "ioformats";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  ioformats", mainName, 1));

    fprintf(stderr, "Test bmp 1 bpp file:\n");
    ioFormatTest(BMP_FILE);
    fprintf(stderr, "\nTest other 1 bpp file:\n");
    ioFormatTest(FILE_1BPP);
    fprintf(stderr, "\nTest 2 bpp file:\n");
    ioFormatTest(FILE_2BPP);
    fprintf(stderr, "\nTest 4 bpp file:\n");
    ioFormatTest(FILE_4BPP);
    fprintf(stderr, "\nTest 8 bpp grayscale file with cmap:\n");
    ioFormatTest(FILE_8BPP_1);
    fprintf(stderr, "\nTest 8 bpp color file with cmap:\n");
    ioFormatTest(FILE_8BPP_2);
    fprintf(stderr, "\nTest 8 bpp file without cmap:\n");
    ioFormatTest(FILE_8BPP_3);
    fprintf(stderr, "\nTest 32 bpp file:\n");
    ioFormatTest(FILE_32BPP);

    exit(0);
}

