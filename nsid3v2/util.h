#pragma once
#include <bfc/platform/types.h>

namespace ID3v2
{
	namespace Util
	{	
		/* Call this with a value you read straight from a file.  It will deal with endianness issues */
		uint32_t Int28To32(uint32_t val);
#if defined(LITTLE_ENDIAN) || defined(_M_IX86) || defined(_M_X64)
		inline uint32_t UInt32RawToUInt32(uint32_t val)
		{
			return ((((uint32_t)(val) & 0xff000000) >> 24) | \
				(((uint32_t)(val) & 0x00ff0000) >> 8) |
				(((uint32_t)(val) & 0x0000ff00) << 8) |
				(((uint32_t)(val) & 0x000000ff) << 24));
		}
#elif defined(BIG_ENDIAN)
		inline uint32_t UInt32RawToUInt32(uint32_t val)
		{
			return val;
		}
#else
#error neither BIG_ENDIAN nor LITTLE_ENDIAN defined!
#endif

		// returns input bytes used
		size_t UnsynchroniseTo(void *output, const void *input, size_t bytes);

		// returns number of real bytes required to read 'bytes' data
		size_t UnsynchronisedSize(const void *data, size_t bytes);

		// returns output bytes used
		size_t SynchroniseTo(void *output, const void *input, size_t bytes);

		// returns number of bytes required to store synchronized version of 'data' (bytes long)
		size_t SynchronisedSize(const void *data, size_t bytes);
	}
}