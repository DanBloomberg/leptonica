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


#ifndef ENVIRON_H_INCLUDED
#define ENVIRON_H_INCLUDED

#if !defined(USE_PSTDINT)
#include <stdint.h>
#else
#include "pstdint.h"
#endif

typedef intptr_t l_intptr_t;
typedef uintptr_t l_uintptr_t;


/*--------------------------------------------------------------------*
 *                          Built-in types                            *
 *--------------------------------------------------------------------*/
typedef signed char                l_int8;
typedef unsigned char              l_uint8;
typedef short                   l_int16;
typedef unsigned short          l_uint16;
typedef int                     l_int32;
typedef unsigned int            l_uint32;
typedef float                        l_float32;
typedef double                        l_float64;


/*------------------------------------------------------------------------*
 *                            Standard macros                             *
 *------------------------------------------------------------------------*/
#ifndef L_MIN
#define L_MIN(x,y)   (((x) < (y)) ? (x) : (y))
#endif

#ifndef L_MAX
#define L_MAX(x,y)   (((x) > (y)) ? (x) : (y))
#endif

#ifndef L_ABS
#define L_ABS(x)     (((x) < 0) ? (-1 * (x)) : (x))
#endif

#ifndef L_SIGN
#define L_SIGN(x)    (((x) < 0) ? -1 : 1)
#endif

#ifndef UNDEF
#define UNDEF        -1
#endif

#ifndef NULL
#define NULL          0
#endif

#ifndef TRUE
#define TRUE          1
#endif

#ifndef FALSE
#define FALSE         0
#endif


/*--------------------------------------------------------------------*
 *         Use environ variables within compiler invocation           *
 *--------------------------------------------------------------------*/
/*
 *  To control conditional compilation, one of two variables
 *
 *       L_LITTLE_ENDIAN  (e.g., for Intel X86)
 *       L_BIG_ENDIAN     (e.g., for Sun SPARC, Mac Power PC)
 *
 *  is defined when the GCC compiler is invoked.
 *
 *  N.B.  All code should compile properly for both hardware
 *        architectures.  However, it has only been thoroughly
 *        tested on X86 hardware.
 */


/*------------------------------------------------------------------------*
 *                   Simple search state variables                        *
 *------------------------------------------------------------------------*/
enum {
    L_NOT_FOUND = 0,
    L_FOUND = 1
};


/*------------------------------------------------------------------------*
 *                      Standard memory allocation                        *
 *------------------------------------------------------------------------*/
#define MALLOC(blocksize)           malloc(blocksize)
#define CALLOC(numelem, elemsize)   calloc(numelem, elemsize)
#define REALLOC(ptr, blocksize)     realloc(ptr, blocksize)
#define FREE(ptr)                   free(ptr)


/*------------------------------------------------------------------------*
 *         Control printing of error, warning, and info messages         *
 *                                                                        *
 *      (Use -DNO_CONSOLE_IO on compiler line to prevent text output)     *
 *------------------------------------------------------------------------*/
#ifdef  NO_CONSOLE_IO

#define PROCNAME(name)
#define ERROR_PTR(a,b,c)            ((void *)(c))
#define ERROR_INT(a,b,c)            ((l_int32)(c))
#define ERROR_FLOAT(a,b,c)          ((l_float32)(c))
#define ERROR_VOID(a,b)
#define L_WARNING(a,b)
#define L_WARNING_INT(a,b,c)
#define L_INFO(a,b)
#define L_INFO_INT(a,b,c)
#define L_INFO_INT2(a,b,c,d)
#define L_INFO_FLOAT(a,b,c)
#define L_INFO_FLOAT2(a,b,c,d)

#else

#define PROCNAME(name)              static const char procName[] = name
#define ERROR_PTR(a,b,c)            l_errorPtr((a),(b),(c))
#define ERROR_INT(a,b,c)            l_errorInt((a),(b),(c))
#define ERROR_FLOAT(a,b,c)          l_errorFloat((a),(b),(c))
#define ERROR_VOID(a,b)             l_errorVoid((a),(b))
#define L_WARNING(a,b)              l_warning((a),(b))
#define L_WARNING_INT(a,b,c)        l_warningInt((a),(b),(c))
#define L_INFO(a,b)                 l_info((a),(b))
#define L_INFO_INT(a,b,c)           l_infoInt((a),(b),(c))
#define L_INFO_INT2(a,b,c,d)        l_infoInt2((a),(b),(c),(d))
#define L_INFO_FLOAT(a,b,c)         l_infoFloat((a),(b),(c))
#define L_INFO_FLOAT2(a,b,c,d)      l_infoFloat2((a),(b),(c),(d))

#endif  /* NO_CONSOLE_IO */


#endif /* ENVIRON_H_INCLUDED */

