#pragma once
#include "mp4/svc_mp4decoder.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {3F73F340-1CD7-4694-8893-3DF9E87005AE}
static const GUID mp4_mp3_decoder_service_guid = 
{ 0x3f73f340, 0x1cd7, 0x4694, { 0x88, 0x93, 0x3d, 0xf9, 0xe8, 0x70, 0x5, 0xae } };

class MP4DecoderService : public svc_mp4decoder
{
public:
	WASABI_SERVICE_NAME("MP3-in-MP4 Decoder");
	
	static GUID GetServiceGUID() { return mp4_mp3_decoder_service_guid; }

	/* svc_mp4decoder implementation */
	int WASABICALL MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder);
};