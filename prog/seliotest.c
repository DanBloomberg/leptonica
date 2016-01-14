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
 * seliotest.c
 *
 *    Runs a number of tests on reading and writing of Sels
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
char           *str1, *str2;
l_int32         nbytes1, nbytes2;
PIX            *pix;
SELA           *sela1, *sela2;
static char     mainName[] = "seliotest";

    if (argc != 1)
	return ERROR_INT(" Syntax:  seliotest", mainName, 1);

        /* selaRead() / selaWrite()  */
    sela1 = selaAddBasic(NULL);
    selaWrite("junkout1", sela1);
    sela2 = selaRead("junkout1");
    selaWrite("junkout2", sela2);
    str1 = (char *)arrayRead("junkout1", &nbytes1);
    str2 = (char *)arrayRead("junkout2", &nbytes2);
    if (nbytes1 == nbytes2 && !strcmp(str1, str2))
        fprintf(stderr, "Success:  selaRead() / selaWrite()\n");
    else
        fprintf(stderr, "Failure:  selaRead() / selaWrite()\n");
    FREE(str1);
    FREE(str2);
    selaDestroy(&sela1);
    selaDestroy(&sela2);
    
	/* Create from file and display result */
    sela1 = selaCreateFromFile("flipsels.txt");
    pix = selaDisplayInPix(sela1, 31, 3, 15, 4);
    pixDisplay(pix, 100, 100);
    selaDestroy(&sela1);
    pixDestroy(&pix);

    return 0;
}

