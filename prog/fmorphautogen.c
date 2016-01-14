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
 * fmorphautogen.c
 *
 *    This program was used to generate the two files:
 *         fmorphgen.1.c
 *         fmorphgenlow.1.c
 *    using INDEX = 1.
 *    It shows how to generate dwa code from a SELA.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  INDEX     1


main(int    argc,
     char **argv)
{
SELA        *sela;
static char  mainName[] = "fmorphautogen";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  fmorphautogen", mainName, 1));

    sela = selaAddBasic(NULL);

    if (fmorphautogen(sela, INDEX))
	exit(1);

    exit(0);
}

