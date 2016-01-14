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
 * alltiff2ps.c
 *
 *    Converts all g4 compressed tiff files in a directory to a single
 *    PostScript file, at the specified resolution.  Decreasing the
 *    resolution will cause the image to be rendered larger, and v.v.
 *
 *    Note: this program is Unix only; it will not compile under cygwin.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char           *dirin, *fileout, *fname, *fullname;
l_int32         res, i, index, nfiles, tifffound, retval, format;
SARRAY         *safiles;
FILE           *fp;
static char     mainName[] = "alltiff2ps";

    if (argc != 4)
	exit(ERROR_INT(" Syntax:  alltiff2ps dirin res fileout", mainName, 1));

    dirin = argv[1];
    res = atoi(argv[2]);
    fileout = argv[3];

	/* capture the filenames in the input directory; ignore directories */
    if ((safiles = getFilenamesInDirectory(dirin)) == NULL)
	exit(ERROR_INT("safiles not made", mainName, 1));

    nfiles = sarrayGetCount(safiles);
    tifffound = FALSE;
    for (i = 0, index = 0; i < nfiles; i++) {
        fname = sarrayGetString(safiles, i, 0);
	fullname = genPathname(dirin, fname);
        fp = fopen(fullname, "r");
	if (!fp)
	    continue;
	format = findFileFormat(fp);
	fclose(fp);
	if (format != IFF_TIFF)
	    continue;
	if (!tifffound) {
            retval = convertTiffG4ToPS(fullname, fileout, "w", 0, 0, res, 1.0,
			      index + 1, FALSE, TRUE);
            if (retval == 0) {
	        tifffound = TRUE;
		index++;
            }
	}
	else {
            retval = convertTiffG4ToPS(fullname, fileout, "a", 0, 0, res, 1.0,
			      index + 1, FALSE, TRUE);
            if (retval == 0) {
		index++;
            }
        }
	FREE((void *)fullname);
    }

    sarrayDestroy(&safiles);
    return 0;
}

