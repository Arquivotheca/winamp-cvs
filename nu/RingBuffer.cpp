/*
 *  RingBuffer.cpp
 *  simple_mp3_playback
 *
 *  Created by Ben Allison on 11/10/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
 *
 */

#include "RingBuffer.h"
#include <bfc/platform/types.h>
#include <bfc/platform/minmax.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

// TODO: benski> move this into 
#ifdef _WIN64
#include <windows.h>
inline static void atomic_add(volatile size_t *val, size_t add)
{
	InterlockedExchangeAdd64((volatile LONGLONG *)val, (LONGLONG)add);
}

inline static void atomic_sub(volatile size_t *val, size_t sub)
{
	InterlockedExchangeAdd64((volatile LONGLONG *)val, -((LONGLONG)sub));
}
#elif defined(_WIN32)
#include <windows.h>
inline static void atomic_add(volatile size_t *val, size_t add)
{
	InterlockedExchangeAdd((volatile LONG *)val, (LONG)add);
}

inline static void atomic_sub(volatile size_t *val, size_t sub)
{
	InterlockedExchangeAdd((volatile LONG *)val, -((LONG)sub));
}

inline static void atomic_write(volatile size_t *dest, size_t src)
{
	InterlockedExchange((volatile LONG *)dest, (LONG)src);
}

inline static void atomic_read(size_t *dest, size_t src)
{
	InterlockedExchange((volatile LONG *)dest, (LONG)src);
	// TODO: on Itanium - InterlockedExchangeAcquire((volatile LONG *)dest, (LONG)src);
}

#elif defined(__APPLE__)

#include <libkern/OSAtomic.h>
#ifdef __LP64__
inline static void atomic_add(volatile size_t *val, size_t add)
{
	OSAtomicAdd64(add, (volatile int64_t *)val);
}

inline static void atomic_sub(volatile size_t *val, size_t sub)
{
	OSAtomicAdd64(-((int64_t)sub), (volatile int64_t *)val);
}
#else
inline static void atomic_add(volatile size_t *val, size_t add)
{
	OSAtomicAdd32(add, (volatile int32_t *)val);
}

inline static void atomic_sub(volatile size_t *val, size_t sub)
{
	OSAtomicAdd32(-((int32_t)sub), (volatile int32_t *)val);
}
#endif

#elif defined(__linux__)
#include <ext/atomicity.h>
inline static void atomic_add(volatile size_t *val, size_t add)
{
	__gnu_cxx::__atomic_add((volatile _Atomic_word *)val, add);
}

inline static void atomic_sub(volatile size_t *val, size_t sub)
{
	__gnu_cxx::__atomic_add((volatile _Atomic_word *)val, -((intptr_t)sub));
}
#endif

RingBuffer::RingBuffer()
{
	ringBuffer=0;
	ringBufferSize=0;
	ringBufferUsed=0;
	ringWritePosition=0;
	ringReadPosition=0;
}

RingBuffer::~RingBuffer()
{
	free(ringBuffer);
	ringBuffer=0;
}

void RingBuffer::Reset()
{
	free(ringBuffer);
	ringBuffer=0;
}

bool RingBuffer::reserve(size_t bytes)
{
	Reset();
	ringBufferSize=bytes;
	ringBuffer = (char *)malloc(ringBufferSize);
	if (!ringBuffer) return false;
	clear();
	return true;
}

bool RingBuffer::empty() const
{
	return (ringBufferUsed==0);
}

size_t RingBuffer::read(void *dest, size_t len)
{
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier
	size_t toCopy;
	atomic_read(&toCopy, ringBufferUsed); // memory fence so we get the latest value 
	if (toCopy > len) toCopy = len;

	size_t copied=0;
	len-=toCopy;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	atomic_sub(&ringBufferUsed, read1);
	toCopy-=read1;
	out = (int8_t *)out+read1;

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
		copied+=toCopy;
		ringReadPosition+=toCopy;
		atomic_sub(&ringBufferUsed, toCopy);
		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}

	return copied;
}

size_t RingBuffer::at(size_t offset, void *dest, size_t len) const
{
	size_t toCopy;
	atomic_read(&toCopy, ringBufferUsed); // memory fence so we get the latest value

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	/* --- do a "dummy read" to deal with the offset request --- */
	size_t dummy_end = ringBufferSize-(ringReadPosition-ringBuffer);

	offset = MIN(toCopy, offset);
	size_t read0 = MIN(dummy_end, offset);
	ringReadPosition+=read0;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read0;
	offset-=read0;

	// do second-half read (wraparound)
	if (offset)
	{
		ringReadPosition+=offset;
		toCopy-=offset;
	}

  // dummy read done

	/* --- set up destination buffer and copy size --- */
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	if (toCopy > len) toCopy=len;
	size_t copied=0;

	/* --- read to the end of the ring buffer --- */
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	out = (int8_t *)out+read1;

	/* --- see if we still have more to read after wrapping around --- */
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
		copied+=toCopy;
		ringReadPosition+=toCopy;
	}

	return copied;
}

size_t RingBuffer::peek(void *dest, size_t len) const
{
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	size_t toCopy;
	atomic_read(&toCopy, ringBufferUsed); // memory fence so we get the latest value 

	if (toCopy > len) toCopy=len;
	size_t copied=0;

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	out = (int8_t *)out+read1;

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
		copied+=toCopy;
		ringReadPosition+=toCopy;
	}

	return copied;
}

size_t RingBuffer::advance(size_t len)
{
	size_t toCopy;
	atomic_read(&toCopy, ringBufferUsed); // memory fence so we get the latest value 

	if (toCopy>len) toCopy=len;
	size_t copied=0;
	len-=toCopy;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	atomic_sub(&ringBufferUsed, read1);

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		copied+=toCopy;
		ringReadPosition+=toCopy;
		atomic_sub(&ringBufferUsed, toCopy);

		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}

	return copied;
}

size_t RingBuffer::avail() const
{
	size_t used;
	atomic_read(&used, ringBufferUsed); // memory fence so we get the latest value 
	return ringBufferSize - used;
}

size_t RingBuffer::write(const void *buffer, size_t bytes)
{
	size_t used;
	atomic_read(&used, ringBufferUsed); // memory fence so we get the latest value 

	size_t avail = ringBufferSize - used;
	bytes = MIN(avail, bytes);

	// write to the end of the ring buffer
	size_t end = ringBufferSize-(ringWritePosition-ringBuffer);
	size_t copied=0;
	size_t write1 = MIN(end, bytes);
	memcpy(ringWritePosition, buffer, write1);
	copied+=write1;
	ringWritePosition+=write1;
	if (ringWritePosition == ringBuffer + ringBufferSize)
		ringWritePosition=ringBuffer;

	// update positions
	atomic_add(&ringBufferUsed, write1);
	bytes-=write1;
	buffer = (const int8_t *)buffer+write1;

	// see if we still have more to write after wrapping around
	if (bytes)
	{
		memcpy(ringWritePosition, buffer, bytes);
		copied+=bytes;
		ringWritePosition+=bytes;
		atomic_add(&ringBufferUsed, bytes);
		if (ringWritePosition == ringBuffer + ringBufferSize)
			ringWritePosition=ringBuffer;
	}

	return copied;
}

size_t RingBuffer::drain(Drainer *drainer, size_t max_bytes)
{
		// read to the end of the ring buffer
	size_t used;
	atomic_read(&used, ringBufferUsed); // memory fence so we get the latest value 

	size_t bytes = used;
	bytes = MIN(bytes, max_bytes);
	size_t copied=0;
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);
	size_t drain1 = MIN(end, bytes);

	if (!drain1)
		return 0;

	size_t read1 = drainer->Write(ringReadPosition, drain1);
	if (read1 == 0)
		return 0;

	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

		// update positions
	atomic_sub(&ringBufferUsed, read1);
	bytes-=read1;

	// see if we still have more to read after wrapping around
	if (drain1 == read1 && bytes)
	{
		size_t read2 =  drainer->Write(ringReadPosition, bytes);
		copied+=read2;
		ringReadPosition+=read2;
		atomic_sub(&ringBufferUsed, read2);
		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}
	return copied;
}

size_t RingBuffer::fill(Filler *filler, size_t max_bytes)
{
	// write to the end of the ring buffer
	size_t used;
	atomic_read(&used, ringBufferUsed); // memory fence so we get the latest value 

	size_t bytes = ringBufferSize - used;
	bytes = MIN(bytes, max_bytes);
	size_t copied=0;
	size_t end = ringBufferSize-(ringWritePosition-ringBuffer);
	size_t fill1 = MIN(end, bytes);

	if (!fill1)
		return 0;

	size_t write1 = filler->Read(ringWritePosition, fill1);
	if (write1 == 0)
		return 0;
	copied+=write1;
	ringWritePosition+=write1;
	if (ringWritePosition == ringBuffer + ringBufferSize)
		ringWritePosition=ringBuffer;

	// update positions
	atomic_add(&ringBufferUsed, write1);
	bytes-=write1;

	// see if we still have more to write after wrapping around
	if (fill1 == write1 && bytes)
	{
		size_t write2 =  filler->Read(ringWritePosition, bytes);
		copied+=write2;
		ringWritePosition+=write2;
		atomic_add(&ringBufferUsed, write2);
		if (ringWritePosition == ringBuffer + ringBufferSize)
			ringWritePosition=ringBuffer;
	}
	return copied;
}

size_t RingBuffer::size() const
{
	size_t used;
	atomic_read(&used, ringBufferUsed); // memory fence so we get the latest value 
	return used;
}

void RingBuffer::clear()
{
	atomic_write(&ringBufferUsed, 0);
	ringWritePosition=ringBuffer;
	ringReadPosition=ringBuffer;
}

void *RingBuffer::LockBuffer()
{
	return ringBuffer;
}

void RingBuffer::UnlockBuffer(size_t written)
{
	ringWritePosition = ringBuffer+written;
	atomic_write(&ringBufferUsed, written);
}
