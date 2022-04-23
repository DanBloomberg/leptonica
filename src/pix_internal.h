/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_PIX_INTERNAL_H
#define  LEPTONICA_PIX_INTERNAL_H

#include "allheaders.h"
#include <stdatomic.h>

/*! Basic rectangle */
struct Box
{
    l_int32            x;           /*!< left coordinate                   */
    l_int32            y;           /*!< top coordinate                    */
    l_int32            w;           /*!< box width                         */
    l_int32            h;           /*!< box height                        */
    atomic_int         refcount;    /*!< reference count (1 if no clones)  */
};

/*! Array of Box */
struct Boxa
{
    l_int32            n;           /*!< number of box in ptr array        */
    l_int32            nalloc;      /*!< number of box ptrs allocated      */
    atomic_int         refcount;    /*!< reference count (1 if no clones)  */
    struct Box       **box;         /*!< box ptr array                     */
};

/* The 'special' field is by default 0, but it can hold integers
 * that direct non-default actions, e.g., in png and jpeg I/O. */

/*! Basic Pix */
struct Pix
{
    l_uint32             w;         /*!< width in pixels                   */
    l_uint32             h;         /*!< height in pixels                  */
    l_uint32             d;         /*!< depth in bits (bpp)               */
    l_uint32             spp;       /*!< number of samples per pixel       */
    l_uint32             wpl;       /*!< 32-bit words/line                 */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    atomic_int           refcount;  /*!< reference count (1 if no clones)  */
    l_int32              informat;  /*!< input file format, IFF_*          */
    l_int32              special;   /*!< special instructions for I/O, etc */
    char                *text;      /*!< text string associated with pix   */
    struct PixColormap  *colormap;  /*!< colormap (may be null)            */
    l_uint32            *data;      /*!< the image data                    */
};

/*  Serialization for primary data structures */
#define  PIXAA_VERSION_NUMBER      2  /*!< Version for Pixaa serialization */
#define  PIXA_VERSION_NUMBER       2  /*!< Version for Pixa serialization  */
#define  BOXA_VERSION_NUMBER       2  /*!< Version for Boxa serialization  */
#define  BOXAA_VERSION_NUMBER      3  /*!< Version for Boxaa serialization */

/*! Array of pix */
struct Pixa
{
    l_int32             n;          /*!< number of Pix in ptr array        */
    l_int32             nalloc;     /*!< number of Pix ptrs allocated      */
    atomic_int          refcount;   /*!< reference count (1 if no clones)  */
    struct Pix        **pix;        /*!< the array of ptrs to pix          */
    struct Boxa        *boxa;       /*!< array of boxes                    */
};

#define  FPIX_VERSION_NUMBER      2 /*!< Version for FPix serialization    */

/*! Pix with float array */
struct FPix
{
    l_int32              w;         /*!< width in pixels                   */
    l_int32              h;         /*!< height in pixels                  */
    l_int32              wpl;       /*!< 32-bit words/line                 */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    atomic_int           refcount;  /*!< reference count (1 if no clones)  */
    l_float32           *data;      /*!< the float image data              */
};

/*! Array of FPix */
struct FPixa
{
    l_int32             n;          /*!< number of fpix in ptr array       */
    l_int32             nalloc;     /*!< number of fpix ptrs allocated     */
    atomic_int          refcount;   /*!< reference count (1 if no clones)  */
    struct FPix       **fpix;       /*!< the array of ptrs to fpix         */
};

#define  DPIX_VERSION_NUMBER      2 /*!< Version for DPix serialization    */

/*! Pix with double array */
struct DPix
{
    l_int32              w;         /*!< width in pixels                   */
    l_int32              h;         /*!< height in pixels                  */
    l_int32              wpl;       /*!< 32-bit words/line                 */
    atomic_int           refcount;  /*!< reference count (1 if no clones)  */
    l_int32              xres;      /*!< image res (ppi) in x direction    */
                                    /*!< (use 0 if unknown)                */
    l_int32              yres;      /*!< image res (ppi) in y direction    */
                                    /*!< (use 0 if unknown)                */
    l_float64           *data;      /*!< the double image data             */
};

#define  PTA_VERSION_NUMBER      1  /*!< Version for Pta serialization     */

/*! Array of points */
struct Pta
{
    l_int32            n;           /*!< actual number of pts              */
    l_int32            nalloc;      /*!< size of allocated arrays          */
    atomic_int         refcount;    /*!< reference count (1 if no clones)  */
    l_float32         *x, *y;       /*!< arrays of floats                  */
};

#endif  /* LEPTONICA_PIX_INTERNAL_H */
