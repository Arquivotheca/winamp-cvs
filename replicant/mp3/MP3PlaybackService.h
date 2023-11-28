#pragma once
#include "file/svc_fileplayback.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {30A579F7-5479-4E20-AD99-D1CE1B2F46DF}
static const GUID mp3_file_playback_guid = 
{ 0x30a579f7, 0x5479, 0x4e20, { 0xad, 0x99, 0xd1, 0xce, 0x1b, 0x2f, 0x46, 0xdf } };

class MP3PlaybackService : public svc_fileplayback
{
public:
	WASABI_SERVICE_NAME("MP3 File Playback");
	WASABI_SERVICE_GUID(mp3_file_playback_guid);
	ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
};
