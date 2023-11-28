//
//  MP3Decoder.cpp
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#include "api.h"
#include "main.h"
#include "MP3Decoder.h"
#include "MP3DecoderCallback.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>


int MP3Decoder::FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags)
{
	/* check extension before we check the parameters */
	if (IsMyExtension(filename))
	{
		/* TODO: validate parameters */		
		ReferenceCountedObject<MetadataChain<GioFile> > giofile;
		if (!giofile)
			return NErr_OutOfMemory;

		// open file
		int ret = giofile->Open(filename, file);
		if (ret != NErr_Success)
		{
			return ret;
		}

		giofile->SetParentMetadata(parent_metadata);

		// create decoder object
		CMpgaDecoder *mpeg = 0;
		if (flags & svc_decode::FLAG_VALIDATION)
			mpeg = new CMpgaDecoder(MPEGAUDIO_CRCCHECK_ON);
		else
			mpeg = new CMpgaDecoder(MPEGAUDIO_CRCCHECK_OFF);

		if (!mpeg)
		{
			return NErr_OutOfMemory;
		}
		
		/* create our object */
		MP3DecoderCallback *mp3_decoder_callback = new (std::nothrow) ReferenceCounted<MP3DecoderCallback>();
		if (!mp3_decoder_callback)
		{
			delete mpeg;
			return NErr_OutOfMemory;
		}
		
		ret = mp3_decoder_callback->Initialize(giofile, mpeg, flags, parameters);
		if (ret != NErr_Success)
		{
			mp3_decoder_callback->Release();
			return ret;
		}
		
		*decoder = mp3_decoder_callback;
		return NErr_Success;
	}
	return NErr_False;
	
}
