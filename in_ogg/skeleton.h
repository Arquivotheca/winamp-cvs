#pragma once
#include <ogg/ogg.h>
#include <bfc/platform/types.h>

class OggSkeleton
{
public:
	OggSkeleton(const ogg_packet *packet);
	static bool IsSkeletonPacket(const ogg_packet *packet);
	
private:
	uint16_t version_major, version_minor;
	uint64_t presentation_numerator, presentation_denominator;
	uint64_t basetime_numerator, basetime_denominator;
	
};