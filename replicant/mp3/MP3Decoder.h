//
//  MP3Decoder.h
//
//  Created by Ben Allison on 1/17/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
#pragma once

#include "file/svc_filedecode.h"
#include "nswasabi/ServiceName.h"

// {2BD05D2D-64B7-41C1-B039-9922326994C5}
static const GUID mp3_file_decode_guid = 
{ 0x2bd05d2d, 0x64b7, 0x41c1, { 0xb0, 0x39, 0x99, 0x22, 0x32, 0x69, 0x94, 0xc5 } };



class MP3Decoder : public svc_filedecode
{
public:
	WASABI_SERVICE_NAME("MP3 File Decode");
	WASABI_SERVICE_GUID(mp3_file_decode_guid);
	
private:
	ns_error_t WASABICALL FileDecodeService_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags);
};