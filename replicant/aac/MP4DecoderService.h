#pragma once
#include "mp4/svc_mp4decoder.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {3BE7D72D-D229-4D1E-8D2B-82579D965974}
static const GUID mp4_aac_decoder_service_guid = 
{ 0x3be7d72d, 0xd229, 0x4d1e, { 0x8d, 0x2b, 0x82, 0x57, 0x9d, 0x96, 0x59, 0x74 } };

class MP4DecoderService : public svc_mp4decoder
{
public:
	WASABI_SERVICE_NAME("MP4 AAC Decoder");
	static GUID GetServiceGUID() { return mp4_aac_decoder_service_guid; }

	/* svc_mp4decoder implementation */
	int WASABICALL MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder);
};