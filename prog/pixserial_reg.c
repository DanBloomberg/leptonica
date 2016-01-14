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
 * pixserial_reg.c
 *
 *    Tests the fast (uncompressed) serialization of pix to a string
 *    in memory and the deserialization back to a pix.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

    /* Use this set */
static l_int32  nfiles = 10;
static const char  *filename[] = {
                         "feyn.tif",         /* 1 bpp */
                         "dreyfus2.png",     /* 2 bpp cmapped */
                         "dreyfus4.png",     /* 4 bpp cmapped */
                         "weasel4.16c.png",  /* 4 bpp cmapped */
                         "dreyfus8.png",     /* 8 bpp cmapped */
                         "weasel8.240c.png", /* 8 bpp cmapped */
                         "karen8.jpg",       /* 8 bpp, not cmapped */
                         "test16.tif",       /* 8 bpp, not cmapped */
                         "marge.jpg",        /* rgb */
                         "test24.jpg"        /* rgb */
                            };

main(int    argc,
     char **argv)
{
l_int32      i, nbytes, errorfound, same;
l_uint32    *data32, *data32r;
PIX         *pixs, *pixd;
static char  mainName[] = "pixserial_reg";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  pixserial_reg", mainName, 1));

    errorfound = FALSE;
    for (i = 0; i < nfiles; i++) {
        pixs = pixRead(filename[i]);
        pixSerializeToMemory(pixs, &data32, &nbytes);
            /* Just for fun, write and read back from file */
        arrayWrite("/tmp/junkarray", "w", data32, nbytes);
        data32r = (l_uint32 *)arrayRead("/tmp/junkarray", &nbytes); 
        pixd = pixDeserializeFromMemory(data32r, nbytes);
        pixEqual(pixs, pixd, &same);
        if (same)
            L_INFO_INT("success for image %d", mainName, i);
        else {
            L_INFO_STRING("FAILURE for image %s", mainName, filename[i]);
            errorfound = TRUE;
        }
        pixDestroy(&pixs);
        pixDestroy(&pixd);
        FREE(data32);
        FREE(data32r);
    }
    if (errorfound == TRUE)
        fprintf(stderr, "***********\nERROR FOUND\n***********\n");
    else
        fprintf(stderr, "******\nALL OK\n******\n");
    
        /* Now do timing */
    for (i = 0; i < nfiles; i++) {
        pixs = pixRead(filename[i]);
        startTimer();
        pixSerializeToMemory(pixs, &data32, &nbytes);
        pixd = pixDeserializeFromMemory(data32, nbytes);
        fprintf(stderr, "Time for %s: %7.3f sec\n", filename[i], stopTimer());
        FREE(data32);
        pixDestroy(&pixs);
        pixDestroy(&pixd);
    }

    return 0;
}

