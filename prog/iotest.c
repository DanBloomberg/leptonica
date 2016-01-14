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
 * iotest.c
 *
 *   Tests all I/O except multipage/custom tiff and PostScript, which
 *   are separately tested in mtifftest and psiotest, respectively.
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

extern const char *ImageFileFormatExtensions[];

main(int    argc,
     char **argv)
{
l_int32      w, h, d, wpl, count, i, format;
FILE        *fp;
PIX         *pix, *pixt, *pixt2;
PIXCMAP     *cmap;
char        *filein, *fileout;
static char  mainName[] = "iotest";

    if (argc != 3)
	exit(ERROR_INT(" Syntax:  iotest filein fileout", mainName, 1));

    filein = argv[1];
    fileout = argv[2];

#if 1
    if ((pix = pixRead(filein)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));
#else
    if ((pix = pixReadJpeg(filein, 0, 4, NULL)) == NULL)
	exit(ERROR_INT("pix not made", mainName, 1));
#endif

#if 1
    w = pixGetWidth(pix);
    h = pixGetHeight(pix);
    d = pixGetDepth(pix);
    wpl = pixGetWpl(pix);
    fprintf(stderr, "w = %d, h = %d, d = %d, wpl = %d\n", w, h, d, wpl);
    if (pixGetColormap(pix)) {
	fprintf(stderr, "colormap exists\n");
	pixcmapWriteStream(stderr, pixGetColormap(pix));
	fp = fopen("junkcmap", "w");
	pixcmapWriteStream(fp, pixGetColormap(pix));
	fclose(fp);
	fp = fopen("junkcmap", "r");
	cmap = pixcmapReadStream(fp);
	fclose(fp);
	fp = fopen("junkcmap1", "w");
	pixcmapWriteStream(fp, cmap);
	fclose(fp);
	pixcmapDestroy(&cmap);
    }
    else
	fprintf(stderr, "no colormap\n");
    format = pixGetInputFormat(pix);
    fprintf(stderr, "Input format extension: %s\n",
            ImageFileFormatExtensions[format]);
#endif

#if 1
    if (d == 1) {
        pixCountPixels(pix, &count, NULL);
	fprintf(stderr, "pixel ratio ON/OFF = %6.3f\n",
          (l_float32)count / (l_float32)(pixGetWidth(pix) * pixGetHeight(pix)));
    }
#endif

#if 0   /* test tiff header reader */
{ l_int32 w, h, bps, spp, res, cmap;
    if (readHeaderTiff(filein, &w, &h, &bps, &spp, &res, &cmap) == 0)
        fprintf(stderr,
        "w = %d, h = %d, bps = %d, spp = %d, res = %d, cmap = %d\n",
        w, h, bps, spp, res, cmap);
}
#endif

#if 0
    pixWrite(fileout, pix, IFF_BMP);
#elif 0
    pixWrite(fileout, pix, IFF_PNG);
#elif 0
    pixWrite(fileout, pix, IFF_TIFF);
    fprintTiffInfo(stderr, fileout);
#elif 0
    pixWrite(fileout, pix, IFF_TIFF_PACKBITS);
    fprintTiffInfo(stderr, fileout);
#elif 0
    pixWrite(fileout, pix, IFF_TIFF_G3);
    fprintTiffInfo(stderr, fileout);
#elif 0
    pixWrite(fileout, pix, IFF_TIFF_G4);
    fprintTiffInfo(stderr, fileout);
#elif 0
    pixWrite(fileout, pix, IFF_JFIF_JPEG);
#elif 0
    pixWriteJpeg(fileout, pix, 75, 0);
#elif 0
    pixWrite(fileout, pix, IFF_PNM);
#elif 0
    pixWrite(fileout, pix, IFF_PS);
#endif 

#if 1
    d = pixGetDepth(pix);
    if (d == 8 || d == 32)
        pixWriteJpeg(fileout, pix, 75, 0);
    else
	pixWrite(fileout, pix, IFF_PNG);
#endif

#if 0    /* use on 2, 4 or 8 bpp colormapped images */
    pixt = pixRemoveColormap(pix, REMOVE_CMAP_BASED_ON_SRC);
    pixt2 = pixConvertRGBToColormap(pixt, 5, &nerrors);
    fprintf(stderr, "nerrors = %d\n", nerrors);
    pixWrite(fileout, pixt2, IFF_PNG);
    pixDestroy(&pixt);
    pixDestroy(&pixt2);
#endif

    pixDestroy(&pix);

    exit(0);
}

