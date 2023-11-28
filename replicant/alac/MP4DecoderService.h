#pragma once
#include "foundation/types.h"
#include "mp4/svc_mp4decoder.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {A8E5BDCE-B914-469E-B257-F751FA64CAC1}
static const GUID mp4_alac_decoder_service_guid = 
{ 0xa8e5bdce, 0xb914, 0x469e, { 0xb2, 0x57, 0xf7, 0x51, 0xfa, 0x64, 0xca, 0xc1 } };


class MP4DecoderService : public svc_mp4decoder
{
public:
	WASABI_SERVICE_NAME("ALAC MP4 Decoder");
	static GUID GetServiceGUID() { return mp4_alac_decoder_service_guid; }

	/* svc_mp4decoder implementation */
	int WASABICALL MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder);
};