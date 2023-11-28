//
//  MP4Decoder.cpp
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#include "api.h"
#include "main.h"
#include "MP4Decoder.h"
#include "MP4DecoderCallback.h"
#include "MP4FileObject.h"
#include "svc_mp4decoder.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>
#include "mp4.h"

static ifc_mp4audiodecoder *GetAudioDecoder(MP4FileHandle mp4_file, MP4TrackId mp4_track, ifc_mp4file *mp4_file_object)
{
	GUID mp4_decoder_guid = svc_mp4decoder::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(mp4_decoder_guid, n++))
	{
		svc_mp4decoder *l = (svc_mp4decoder*)sf->GetInterface();
		if (l)
		{
			ifc_mp4audiodecoder *decoder=0;
			int ret = l->CreateAudioDecoder(mp4_file_object, mp4_track, &decoder);
			l->Release();


			if (ret == NErr_Success && decoder)
				return decoder;
		}
	}
	return 0;
}

int MP4Decoder::FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags)
{
	/* check extension before we check the parameters */
	if (IsMyExtension(filename, EXTENSION_FOR_AUDIO_DECODE))
	{
		/* TODO: validate parameters */

		// open file
		MP4FileHandle mp4_file = MP4ReadFile(filename, file);
		if (!mp4_file)
			return NErr_Malformed;

		MetadataChain<MP4FileObject> *mp4_file_object = new (std::nothrow) ReferenceCounted<MetadataChain<MP4FileObject> >;
		if (!mp4_file_object)
			return NErr_OutOfMemory;

		mp4_file_object->Initialize(filename, mp4_file);
		mp4_file_object->SetParentMetadata(parent_metadata);

		ifc_mp4audiodecoder *audio_decoder=0;
		uint32_t num_audio_tracks = MP4GetNumberOfTracks(mp4_file, MP4_AUDIO_TRACK_TYPE);
		for (uint32_t track_num=0;track_num < num_audio_tracks && !audio_decoder;track_num++)
		{
			MP4TrackId audio_track = MP4FindTrackId(mp4_file, track_num, MP4_AUDIO_TRACK_TYPE);
			/* TODO: verify that this isn't a 'dependent' track, e.g. SLS portion of HD-AAC, stored in tref.dpnd */
			if (audio_track != MP4_INVALID_TRACK_ID)
				audio_decoder = GetAudioDecoder(mp4_file, audio_track, mp4_file_object);
		}

		if (!audio_decoder)
		{
			mp4_file_object->ifc_mp4file::Release();
			return NErr_NoMatchingImplementation;
		}

		/* create our object */
		MP4DecoderCallback *mp4_decoder_callback = new (std::nothrow) ReferenceCounted<MP4DecoderCallback>();
		if (!mp4_decoder_callback)
		{
			audio_decoder->Release();
			mp4_file_object->ifc_mp4file::Release();
			return NErr_OutOfMemory;
		}

		int ret = mp4_decoder_callback->Initialize(mp4_file, audio_decoder, flags, parameters, mp4_file_object);
		audio_decoder->Release();
		mp4_file_object->ifc_mp4file::Release();
		if (ret != NErr_Success)
		{
			mp4_decoder_callback->ifc_audio_decoder_callback::Release();			
			return ret;
		}

		*decoder = mp4_decoder_callback;
		return NErr_Success;
	}
	return NErr_False;

}