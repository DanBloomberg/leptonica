/*====================================================================*
-  Copyright (C) 2017 Milner Technologies, Inc.  This content is a
-  component of leptonica and is provided under the terms of the
-  leptonica license.
-
-  Redistribution and use in source and binary forms, with or without
-  modification, are permitted provided that the following conditions
-  are met:
-  1. Redistributions of source code must retain the above copyright
-     notice, this list of conditions and the following disclaimer.
-  2. Redistributions in binary form must reproduce the leptonica
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
*/#include <string.h>
#include <assert.h>
#include "allheaders.h"
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
	MemIOData* thing;
	PROCNAME("memio_png_write_data");

	thing = (MemIOData*)png_get_io_ptr(png_ptr);
	if (thing->m_Last->m_Buffer == 0)
	{
		if (len > BUFFER_SIZE)
		{
			thing->m_Last->m_Buffer = new char[len];
			memcpy(thing->m_Last->m_Buffer, data, len);
			thing->m_Last->m_Size = thing->m_Last->m_Count = len;
			return;
		}

		thing->m_Last->m_Buffer = new char[BUFFER_SIZE];
		thing->m_Last->m_Size = BUFFER_SIZE;
	}

	int written = 0;
	while (written < len)
	{
		if (thing->m_Last->m_Count == thing->m_Last->m_Size)
		{
			MemIOData* next = new MemIOData;
			next->m_Buffer = 0;
			next->m_Count = 0;
			next->m_Size = 0;
			next->m_Next = 0;
			next->m_Last = next;

			thing->m_Last->m_Next = next;
			thing->m_Last = next;

			thing->m_Last->m_Buffer = new char[BUFFER_SIZE];
			thing->m_Last->m_Size = BUFFER_SIZE;
			thing->m_Last->m_Count = 0;
		}
			
		int remainingSpace = thing->m_Last->m_Size - thing->m_Last->m_Count;
		int remainingToWrite = len - written;
		if (remainingSpace < remainingToWrite)
		{
			memcpy(thing->m_Last->m_Buffer + thing->m_Last->m_Count, data + written, remainingSpace);
			written += remainingSpace;

			thing->m_Last->m_Count += remainingSpace;
		}
		else
		{
			memcpy(thing->m_Last->m_Buffer + thing->m_Last->m_Count, data + written, remainingToWrite);
			written += remainingToWrite;
			thing->m_Last->m_Count += remainingToWrite;
		}
	}
}


/*
* <pre>
* Consolidate write buffers into a single buffer in the head of the linked list.
* </pre>
*/
void
memio_png_flush(MemIOData* thing)
{
	PROCNAME("memio_png_write_data");
	
	/* if the data was contained in one buffer just give the buffer to the user. */
	if (thing->m_Next == 0)
	{
		return;
	}

	/* consolidate multiple buffers into one new one. */

	/* add the buffer sizes together. */
	int amount = thing->m_Count;
	MemIOData* buffer = thing->m_Next;
	while (buffer != 0)
	{
		amount += buffer->m_Count;
		buffer = buffer->m_Next;
	}

	/* copy data to a new buffer. */
	char* data = new char[amount];
	memcpy(data, thing->m_Buffer, thing->m_Count);
	int copied = thing->m_Count;

	delete[] thing->m_Buffer;
	thing->m_Buffer = 0;
	/* don't delete original "thing" because we don't control it. */

	buffer = thing->m_Next;
	while (buffer != 0 && copied < amount)
	{
		memcpy(data + copied, buffer->m_Buffer, buffer->m_Count);
		copied += buffer->m_Count;

		MemIOData* old = buffer;
		buffer = buffer->m_Next;

		delete[] old->m_Buffer;
		delete old;
	}
	
	assert(copied == amount);

	thing->m_Buffer = data;
	thing->m_Count = copied;
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

	MemIOData* thing = (MemIOData*)png_get_io_ptr(png_ptr);

	memcpy(outBytes, thing->m_Buffer + thing->m_Count, byteCountToRead);
	thing->m_Count += byteCountToRead;
}