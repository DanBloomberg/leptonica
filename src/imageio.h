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


#ifndef  IMAGEIO_H_INCLUDED
#define  IMAGEIO_H_INCLUDED

/* ------------------ image file format types -------------- */
/*  
 *  The IFF_DEFAULT flags attempts to write the file out in the
 *  same file format that the pix was read from.  If the pix
 *  was not read from file, the IFF_DEFAULT file format is
 *  chosen to be IFF_PNG for d <= 4 and IFF_JFIF_JPEG for d >= 8.
 */
enum {
    IFF_UNKNOWN        = 0,
    IFF_BMP            = 1,
    IFF_JFIF_JPEG      = 2,
    IFF_PNG            = 3,
    IFF_TIFF           = 4,
    IFF_TIFF_PACKBITS  = 5,
    IFF_TIFF_G3        = 6,
    IFF_TIFF_G4        = 7,
    IFF_PNM            = 8,
    IFF_PS             = 9,
    IFF_DEFAULT        = 10
};


/* ------------------ format header ids --------------- */
enum {
    BMP_ID             = 0x4d42,
    TIFF_BIGEND_ID     = 0x4d4d,     /* MM - for 'motorola' */
    TIFF_LITTLEEND_ID  = 0x4949      /* II - for 'intel' */
};

/* ------------------ format header ids --------------- */
enum {
    L_HINT_GRAY = 1,  /* only want grayscale information */
};

#endif  /* IMAGEIO_H_INCLUDED */
