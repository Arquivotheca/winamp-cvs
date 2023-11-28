//
//  FLACDecoderPull.h
//  flac
//
//  Created by Ben Allison on 1/12/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once
#include "audio/ifc_audio_decoder_callback.h"
#include "FLAC/all.h"
#include "audio/ifc_audioout.h"
#include "FLACMetadata.h"
#include "nswasabi/MetadataChain.h"
#include "FLACFileCallbacks.h"

class FLACDecoderCallback : public ifc_audio_decoder_callback, private MetadataChain<FLACMetadata>
{
public:
	FLACDecoderCallback();
	~FLACDecoderCallback();
	int Initialize(nx_uri_t filename, nx_file_t file, FLAC__StreamDecoder *decoder, int flags, nsaudio::Parameters *parameters, ifc_metadata *parent_metadata);
	
private:
	int WASABICALL AudioDecoderCallback_GetMetadata(ifc_metadata **metadata);
	int WASABICALL AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_GetFrameSize(size_t *frame_size);
	FLAC__StreamDecoder *decoder;
	int flags;
	bool aborted;
	nx_file_t file;
	ifc_audio_decoder_callback::callback *callback;
	static FLAC__StreamDecoderWriteStatus FLACOnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
	static void FLACOnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
	FLACClientData client_data;
};