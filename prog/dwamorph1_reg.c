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
 * dwamorph1_reg.c
 *
 *   Implements full regression test, including autogen of code,
 *   compilation, and running the result.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

main(int    argc,
     char **argv)
{
char        *filename;
char         buf[256];
SELA        *sela;
static char  mainName[] = "dwamorph1_reg";

    if (argc != 2)
	exit(ERROR_INT(" Syntax:  dwamorph1_reg filename", mainName, 1));
    filename = argv[1];

        /* Generate the linear sel dwa code */
    sela = selaAddDwaLinear(NULL);
    if (fmorphautogen(sela, 3, "dwalinear"))
        exit(1);
    selaDestroy(&sela);

#if 0
        /* Build dwamorph2_reg, linking in that dwa code */
    system("make dwamorph2_reg;");

        /* Run dwamorph2_reg to test the code */
    sprintf(buf, "dwamorph2_reg %s;", filename);
    system(buf);
#endif

#if 1
        /* Build dwamorph3_reg, linking in that dwa code */
    system("make dwamorph3_reg;");
#endif

#if 0
        /* Run dwamorph3_reg to test the code */
    sprintf(buf, "dwamorph3_reg %s;", filename);
    system(buf);
#endif
    
    exit(0);
}

