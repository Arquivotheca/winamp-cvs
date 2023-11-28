#pragma once
#include "file/svc_fileplayback.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {0645753A-6D0F-4D82-9221-30232BE134F5}
static const GUID flac_fileplayback_guid = 
{ 0x645753a, 0x6d0f, 0x4d82, { 0x92, 0x21, 0x30, 0x23, 0x2b, 0xe1, 0x34, 0xf5 } };


class FLACPlaybackService : public svc_fileplayback
{
public:
	WASABI_SERVICE_NAME("FLAC File Playback");
	static GUID GetServiceGUID() { return flac_fileplayback_guid; }
	ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
};
