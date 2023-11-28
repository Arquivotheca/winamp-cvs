#include "skeleton.h"
#include <string.h> // for memcmp

bool OggSkeleton::IsSkeletonPacket(const ogg_packet *packet)
{
	if (packet && packet->packet && packet->bytes >= 64)
	{
		if (!memcmp(packet->packet, "fishead", 8))
			return true;
	}
	return false;
}


OggSkeleton::OggSkeleton(const ogg_packet *packet)
{

	version_major = *(uint16_t *)(packet->packet + 8);
	version_minor = *(uint16_t *)(packet->packet + 10);
	presentation_numerator = *(uint64_t *)(packet->packet + 12);
	presentation_denominator = *(uint64_t *)(packet->packet + 20);
	basetime_numerator = *(uint64_t *)(packet->packet + 28);
	basetime_denominator = *(uint64_t *)(packet->packet + 36);
// TODO: read UTC	

}