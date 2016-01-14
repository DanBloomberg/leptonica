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

#ifndef  ARRAY_H
#define  ARRAY_H

    /* Flags for parsing and splitting strings into an sarray */
enum {
    WORD_SUBSTRING = 1,  /* split strings on any white space */
    LINE_SUBSTRING = 2,  /* split strings on newline */
};


    /* Array of number arrays */
struct Numaa
{
    l_int32          nalloc;    /* size of allocated ptr array         */
    l_int32          n;         /* number of Numa saved                */
    struct Numa    **numa;      /* array of Numa                       */
};
typedef struct Numaa  NUMAA;


    /* Number array: an array of floats */
struct Numa
{
    l_int32          nalloc;    /* size of allocated number array      */
    l_int32          n;         /* number of numbers saved             */
    l_int32          refcount;  /* reference count (1 if no clones)    */
    l_float32       *array;     /* number array                        */
};
typedef struct Numa  NUMA;


    /* Sparse 2-dimensional array of number arrays */
struct Numa2d
{
    l_int32          nrows;      /* number of rows allocated for ptr array  */
    l_int32          ncols;      /* number of cols allocated for ptr array  */
    l_int32          initsize;   /* initial size of each numa that is made  */
    struct Numa   ***numa;       /* 2D array of Numa                        */
};
typedef struct Numa2d  NUMA2D;


    /* A hash table of Numas */
struct NumaHash
{
    l_int32          nbuckets;
    l_int32          initsize;   /* initial size of each numa that is made */
    struct Numa    **numa;
};
typedef struct NumaHash NUMAHASH;


    /* String array: an array of C strings */
struct Sarray
{
    l_int32          nalloc;    /* size of allocated ptr array         */
    l_int32          n;         /* number of strings allocated         */
    char           **array;     /* string array                        */
};
typedef struct Sarray SARRAY;

#endif  /* ARRAY_H */
