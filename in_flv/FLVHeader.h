#ifndef NULLSOFT_FLVHEADER_H
#define NULLSOFT_FLVHEADER_H

#include <bfc/platform/types.h>

class FLVHeader
{
public:
	bool Read(uint8_t *data, size_t size); // size must be >=9, returns "true" if this was a valid header

	// attributes, consider these read-only
	uint8_t version;
	bool hasAudio, hasVideo;
	uint32_t headerSize;	
};

#endif