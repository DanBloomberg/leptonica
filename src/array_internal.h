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

#ifndef  LEPTONICA_ARRAY_INTERNAL_H
#define  LEPTONICA_ARRAY_INTERNAL_H

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_ATOMICS__)
#include <stdatomic.h>
#elif !defined(atomic_int)
#define atomic_int int
#endif
#include "allheaders.h"

/*! Byte array (analogous to C++ "string") */
struct L_Bytea
{
    size_t           nalloc;    /*!< number of bytes allocated in data array  */
    size_t           size;      /*!< number of bytes presently used           */
    atomic_int       refcount;  /*!< reference count (1 if no clones)         */
    l_uint8         *data;      /*!< data array                               */
};

/*! Dna version for serialization */
#define  DNA_VERSION_NUMBER     1

    /*! Double number array: an array of doubles */
struct L_Dna
{
    l_int32          nalloc;    /*!< size of allocated number array      */
    l_int32          n;         /*!< number of numbers saved             */
    atomic_int       refcount;  /*!< reference count (1 if no clones)    */
    l_float64        startx;    /*!< x value assigned to array[0]        */
    l_float64        delx;      /*!< change in x value as i --> i + 1    */
    l_float64       *array;     /*!< number array                        */
};

/*! Numa version for serialization */
#define  NUMA_VERSION_NUMBER     1

/*! Number array: an array of floats */
struct Numa
{
    l_int32          nalloc;    /*!< size of allocated number array      */
    l_int32          n;         /*!< number of numbers saved             */
    atomic_int       refcount;  /*!< reference count (1 if no clones)    */
    l_float32        startx;    /*!< x value assigned to array[0]        */
    l_float32        delx;      /*!< change in x value as i --> i + 1    */
    l_float32       *array;     /*!< number array                        */
};

/*! Sarray version for serialization */
#define  SARRAY_VERSION_NUMBER     1

/*! String array: an array of C strings */
struct Sarray
{
    l_int32          nalloc;    /*!< size of allocated ptr array         */
    l_int32          n;         /*!< number of strings allocated         */
    atomic_int       refcount;  /*!< reference count (1 if no clones)    */
    char           **array;     /*!< string array                        */
};

#endif  /* LEPTONICA_ARRAY_INTERNAL_H */
