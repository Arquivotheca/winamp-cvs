#pragma once
#include "ogg/svc_oggdecoder.h"
#include "ogg/ifc_oggaudiodecoder.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>

class OggDecoderService : public svc_oggdecoder
{
public:
private:
	ns_error_t WASABICALL OggDecoder_CreateAudioDecoder(ogg_packet *packet, ifc_oggaudiodecoder **audio_decoder);
};

class OggVorbisDecoder : public ifc_oggaudiodecoder
{
public:
	OggVorbisDecoder();

private:
	vorbis_info info;
	vorbis_comment comment;
};