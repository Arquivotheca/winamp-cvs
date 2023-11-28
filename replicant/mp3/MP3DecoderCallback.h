//
//  FLACDecoderPull.h
//  flac
//
//  Created by Ben Allison on 1/12/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once
#include "audio/ifc_audio_decoder_callback.h"
#include "nsmp3/mpgadecoder.h"
#include "mp3/giofile_crt.h"
#include "audio/parameters.h"
#include "nu/PtrDeque.h"
#include "nswasabi/MetadataChain.h"

class MP3DecoderCallback : public ifc_audio_decoder_callback
{
public:
	MP3DecoderCallback();
	~MP3DecoderCallback();
	int Initialize(MetadataChain<GioFile> *gio_file, CMpgaDecoder *decoder, int flags, nsaudio::Parameters *parameters);
	
private:
	int WASABICALL AudioDecoderCallback_GetMetadata(ifc_metadata **metadata);
	int WASABICALL AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_GetFrameSize(size_t *frame_size);
	CMpgaDecoder *decoder;
	MetadataChain<GioFile> *gio_file;
	int flags;
	size_t frame_size;
	size_t channels;
	size_t pregap, postgap;
	bool done;
	size_t samples_per_frame;

	class MP3Buffer : public nu::PtrDequeNode
	{
	public:
		float decode_buffer[1152*2];
		size_t valid_bytes;
	};

	nu::PtrDeque<MP3Buffer> buffers;
	nu::PtrDeque<MP3Buffer> filled_buffers;

	int DecodeNextFrame(bool first);
};