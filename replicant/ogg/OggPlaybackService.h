#pragma once
#include "file/svc_fileplayback.h"
#include "nswasabi/ServiceName.h"

// {46484A41-219C-4069-9BA2-CF8994DC11C4}
static const GUID ogg_fileplayback_guid = 
{ 0x46484a41, 0x219c, 0x4069, { 0x9b, 0xa2, 0xcf, 0x89, 0x94, 0xdc, 0x11, 0xc4 } };

class OggPlaybackService : public svc_fileplayback
{
public:
	WASABI_SERVICE_NAME("Ogg File Playback");
	WASABI_SERVICE_GUID(ogg_fileplayback_guid);
	
	ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
};
