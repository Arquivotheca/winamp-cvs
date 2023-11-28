//
//  FLACDecoder.cpp
//  flac
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//

#include "FLACDecoder.h"
#include "main.h"
#include "FLACDecoderCallback.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include <new>

ns_error_t FLACDecoder::FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags)
{
	/* check extension before we check the parameters */
	if (NXPathMatchExtension(filename, flac_extension) == NErr_Success)
	{
		/* TODO: validate parameters */
		
		// create decoder object
		FLAC__StreamDecoder *flac_decoder = FLAC__stream_decoder_new();
		if (!decoder)
			return NErr_OutOfMemory; // uhh ohh
		
		/* create our object */
		FLACDecoderCallback *flac_decoder_callback = new (std::nothrow) ReferenceCounted<FLACDecoderCallback>();
		if (!flac_decoder_callback)
		{
			FLAC__stream_decoder_delete(flac_decoder);
			return NErr_OutOfMemory;
		}
		
		int ret = flac_decoder_callback->Initialize(filename, file, flac_decoder, flags, parameters, parent_metadata);
		if (ret != NErr_Success)
		{
			flac_decoder_callback->ifc_audio_decoder_callback::Release();
			return ret;
		}
		
		*decoder = flac_decoder_callback;		
		return NErr_Success;
	}
	return NErr_False;
	
}