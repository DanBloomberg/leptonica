/*====================================================================*
-  Copyright (C) 2017 Milner Technologies, Inc.  This content is a
-  component of leptonica and is provided under the terms of the
-  Leptonica license.
-
-  Redistribution and use in source and binary forms, with or without
-  modification, are permitted provided that the following conditions
-  are met:
-  1. Redistributions of source code must retain the above copyright
-     notice, this list of conditions and the following disclaimer.
-  2. Redistributions in binary form must reproduce the Leptonica
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

/*!
* \file memio.h
* <pre>
* libpng read/write callback replacements for performing memory I/O.
* </pre>
*/

#ifndef  LEPTONICA_MEMIO_H
#define  LEPTONICA_MEMIO_H

/* --------------------------------------------*/
#if  HAVE_LIBPNG   /* defined in environ.h */
/* --------------------------------------------*/

#include <png.h>

/*
* <pre>
* A node in a linked list of memory buffers that hold I/O content.
* </pre>
*/
struct MemIOData
{
	/*
	* <pre>Pointer to this node's I/O content.</pre>
	*/
	char* m_Buffer;

	/*
	* <pre>The number of I/O content bytes read or written to m_Buffer.</pre>
	*/
	int m_Count;

	/*
	* <pre>The allocated size of m_Buffer.</pre>
	*/
	int m_Size;

	/*
	* <pre>Pointer to the next node in the list.  Zero if this is the last node.</pre>
	*/
	struct MemIOData* m_Next;

	/*
	* <pre>Pointer to the last node in the linked list.  The last node is where new
	* content is written.</pre>
	*/
    struct MemIOData* m_Last;
};

typedef struct MemIOData MEMIODATA;

#ifndef NO_PROTOS

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void memio_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
void memio_png_flush(MEMIODATA* pthing);
void memio_png_read_data(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);
void memio_free(MEMIODATA* pthing);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* NO_PROTOS */

/* --------------------------------------------*/
#endif  /* HAVE_LIBPNG */
/* --------------------------------------------*/

#endif
