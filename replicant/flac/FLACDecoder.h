//
//  FLACDecoder.h
//  flac
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once

#include "file/svc_filedecode.h"
#include "nswasabi/ServiceName.h"

// {B498E62F-397B-49DC-8177-B735241151A0}
static const GUID flac_decoder_guid = 
{ 0xb498e62f, 0x397b, 0x49dc, { 0x81, 0x77, 0xb7, 0x35, 0x24, 0x11, 0x51, 0xa0 } };


class FLACDecoder : public svc_filedecode
{
public:
	WASABI_SERVICE_NAME("FLAC File Decoder");
	WASABI_SERVICE_GUID(flac_decoder_guid);
	
private:
	ns_error_t WASABICALL FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags);
};