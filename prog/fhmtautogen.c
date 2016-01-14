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
 * fhmtautogen.c
 *
 *    This program was used to generate the two files:
 *         fhmtgen.1.c
 *         fhmtgenlow.1.c
 *    using INDEX = 1.
 *    It shows how to generate dwa code for hit-miss transforms
 *    from a SELA.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

#define  INDEX     1


main(int    argc,
     char **argv)
{
SELA        *sela;
static char  mainName[] = "fhmtautogen";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  fhmtautogen", mainName, 1));

    sela = selaAddHitMiss(NULL);

    if (fhmtautogen(sela, INDEX))
	exit(1);

    exit(0);
}

