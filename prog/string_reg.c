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
 * string_reg.c
 *
 *    This tests several sarray functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allheaders.h"


main(int    argc,
     char **argv)
{
l_int32      nbytesin, nbytesout;
char        *infile, *outfile, *instring, *outstring;
SARRAY      *sa1, *sa2, *sa3, *sa4, *sa5;
char         buf[256];
static char  mainName[] = "string_reg";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  string_reg infile", mainName, 1));

    infile = argv[1];
    instring = (char *)arrayRead(infile, &nbytesin);

    sa1 = sarrayCreateWordsFromString(instring);
    sa2 = sarrayCreateLinesFromString(instring, 0);
    sa3 = sarrayCreateLinesFromString(instring, 1);

    outstring = sarrayToString(sa1, 0);
    nbytesout = strlen(outstring);
    arrayWrite("junkout1", "w", outstring, nbytesout);
    FREE(outstring);

    outstring = sarrayToString(sa1, 1);
    nbytesout = strlen(outstring);
    arrayWrite("junkout2", "w", outstring, nbytesout);
    FREE(outstring);

    outstring = sarrayToString(sa2, 0);
    nbytesout = strlen(outstring);
    arrayWrite("junkout3", "w", outstring, nbytesout);
    FREE(outstring);

    outstring = sarrayToString(sa2, 1);
    nbytesout = strlen(outstring);
    arrayWrite("junkout4", "w", outstring, nbytesout);
    FREE(outstring);

    outstring = sarrayToString(sa3, 0);
    nbytesout = strlen(outstring);
    arrayWrite("junkout5", "w", outstring, nbytesout);
    FREE(outstring);

    outstring = sarrayToString(sa3, 1);
    nbytesout = strlen(outstring);
    arrayWrite("junkout6", "w", outstring, nbytesout);
    FREE(outstring);
    sprintf(buf, "diff -s junkout6 %s", infile);
    system(buf);

	/* write/read/write; compare junkout5 with junkout6 */
    sarrayWrite("junkout7", sa2);
    sarrayWrite("junkout8", sa3);
    sa4 = sarrayRead("junkout8");
    sarrayWrite("junkout9", sa4);
    sa5 = sarrayRead("junkout9");
    system("diff -s junkout8 junkout9");

    sarrayDestroy(&sa1);
    sarrayDestroy(&sa2);
    sarrayDestroy(&sa3);
    sarrayDestroy(&sa4);
    sarrayDestroy(&sa5);
    FREE(instring);

    return 0;
}

