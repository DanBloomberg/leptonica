/*====================================================================*
-  Copyright (C) 2017 Milner Technologies, Inc.  This content is a
-  component of Leptonica and is provided under the terms of the
-  leptonica license.
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
* \file memio.c
* <pre>
* libpng read/write callback replacements for performing memory I/O.
* </pre>

*/#ifdef  HAVE_CONFIG_H`
#include "config_auto.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>

#include "allheaders.h"

/* --------------------------------------------*/
#if  HAVE_LIBPNG   /* defined in environ.h */
/* --------------------------------------------*/

#include "memio.h"

/* 
* <pre>Buffer allocation size.  Set this small enough to avoid consuming unnecessary memory,
* but larger than an average image. </pre>
*/
#define BUFFER_SIZE 8192

/*
* <pre>
* libpng callback for writing an image into a linked list of memory buffers.
* </pre>
*/
void
memio_png_write_data(png_structp png_ptr, png_bytep data, png_size_t len)
{
	MEMIODATA* thing;
	MEMIODATA* last;
	int written = 0;

	PROCNAME("memio_png_write_data");

	thing = (struct MemIOData*)png_get_io_ptr(png_ptr);
	last = (struct MemIOData*)thing->m_Last;
	if (last->m_Buffer == 0)
	{
		if (len > BUFFER_SIZE)
		{
#ifdef __cplusplus
			last->m_Buffer = new char[len];
#else
			last->m_Buffer = LEPT_MALLOC(len);
#endif  /* __cplusplus */
			memcpy(last->m_Buffer, data, len);
			last->m_Size = last->m_Count = len;
			return;
		}

#ifdef __cplusplus
		last->m_Buffer = new char[BUFFER_SIZE];
#else
		last->m_Buffer = LEPT_MALLOC(BUFFER_SIZE);
#endif  /* __cplusplus */
		last->m_Size = BUFFER_SIZE;
	}

	while (written < len)
	{
		if (last->m_Count == last->m_Size)
		{
#ifdef __cplusplus
			MEMIODATA* next = new MEMIODATA;
#else
			MEMIODATA* next = LEPT_MALLOC(sizeof(MEMIODATA));
#endif  /* __cplusplus */
			next->m_Next = 0;
			next->m_Count = 0;
			next->m_Last = next;

			last->m_Next = next;
			last = thing->m_Last = next;

#ifdef __cplusplus
			last->m_Buffer = new char[BUFFER_SIZE];
#else
			last->m_Buffer = LEPT_MALLOC(BUFFER_SIZE);
#endif  /* __cplusplus */
			last->m_Size = BUFFER_SIZE;
		}
		
		/* following paragraph localizes vars only. */
		{
			int remainingSpace = last->m_Size - last->m_Count;
			int remainingToWrite = len - written;
			if (remainingSpace < remainingToWrite)
			{
				memcpy(last->m_Buffer + last->m_Count, data + written, remainingSpace);
				written += remainingSpace;

				last->m_Count += remainingSpace;
			}
			else
			{
				memcpy(last->m_Buffer + last->m_Count, data + written, remainingToWrite);
				written += remainingToWrite;
				last->m_Count += remainingToWrite;
			}
		}
	}
}


/*
* <pre>
* Consolidate write buffers into a single buffer in the head of the linked list.
* </pre>
*/
void
memio_png_flush(MEMIODATA* pthing)
{
	int amount = 0;
	MEMIODATA* buffer = 0;
	int copied = 0;
	char* data = 0;

	PROCNAME("memio_png_write_data");
	
	/* if the data was contained in one buffer just give the buffer to the user. */
	if (pthing->m_Next == 0)
	{
		return;
	}

	/* consolidate multiple buffers into one new one. */

	/* add the buffer sizes together. */
	amount = pthing->m_Count;
	buffer = pthing->m_Next;
	while (buffer != 0)
	{
		amount += buffer->m_Count;
		buffer = buffer->m_Next;
	}

	/* copy data to a new buffer. */
#ifdef __cplusplus
	data = new char[amount];
#else
	data = LEPT_MALLOC(amount);
#endif  /* __cplusplus */
	memcpy(data, pthing->m_Buffer, pthing->m_Count);
	copied = pthing->m_Count;

#ifdef __cplusplus
	delete[] pthing->m_Buffer;
#else
	LEPT_FREE(pthing->m_Buffer);
#endif  /* __cplusplus */
	pthing->m_Buffer = 0;
	/* don't delete original "thing" because we don't control it. */

	buffer = pthing->m_Next;
	pthing->m_Next = 0;
	while (buffer != 0 && copied < amount)
	{
		MEMIODATA* old;
		memcpy(data + copied, buffer->m_Buffer, buffer->m_Count);
		copied += buffer->m_Count;

		old = buffer;
		buffer = buffer->m_Next;

#ifdef __cplusplus
		delete[] old->m_Buffer;
		delete old;
#else
		LEPT_FREE(old->m_Buffer);
		LEPT_FREE(old);
#endif  /* __cplusplus */
	}
	
	/*
	assert(copied == amount);
	*/

	pthing->m_Buffer = data;
	pthing->m_Count = copied;
	pthing->m_Size = amount;
	return;
}


/*
* <pre>
* libpng callback that reads an image from a single memory buffer.
* </pre>
*/
void 
memio_png_read_data(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
{
	PROCNAME("memio_png_read_data");

	MEMIODATA* thing = (MEMIODATA*)png_get_io_ptr(png_ptr);

	memcpy(outBytes, thing->m_Buffer + thing->m_Count, byteCountToRead);
	thing->m_Count += byteCountToRead;
}

void
memio_free(MEMIODATA* pthing)
{
	MEMIODATA* buffer;

	PROCNAME("memio_free");
	
	if (pthing->m_Buffer != 0)
	{
#ifdef __cplusplus
		delete[] pthing->m_Buffer;
#else
		LEPT_FREE(pthing->m_Buffer);
#endif  /* __cplusplus */
	}

	pthing->m_Buffer = 0;
	buffer = pthing->m_Next;
	while (buffer != 0)
	{
		MEMIODATA* old = buffer;
		buffer = buffer->m_Next;

#ifdef __cplusplus
		if (old->m_Buffer != 0)
		{
			delete[] old->m_Buffer;
		}

		delete old;
#else
		if (old->m_Buffer != 0)
		{
			LEPT_FREE(old->m_Buffer);
		}

		LEPT_FREE(old);
#endif  /* __cplusplus */
	}
}

/* --------------------------------------------*/
#endif  /* HAVE_LIBPNG */
/* --------------------------------------------*/
