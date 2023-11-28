#pragma once
#include "playlist/svc_playlisthandler.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {8D031378-4209-4bfe-AC94-03C57C896214}
static const GUID m3u_playlist_handler_guid = 
{ 0x8d031378, 0x4209, 0x4bfe, { 0xac, 0x94, 0x3, 0xc5, 0x7c, 0x89, 0x62, 0x14 } };

class M3UHandler : public svc_playlisthandler
{
public:
	M3UHandler();
	WASABI_SERVICE_NAME("M3U Playlist Handler");
	static GUID GetServiceGUID() { return m3u_playlist_handler_guid; }

private:
	nx_string_t WASABICALL PlaylistHandler_EnumerateExtensions(size_t n);
	int WASABICALL PlaylistHandler_SupportedFilename(nx_uri_t filename);
	int WASABICALL PlaylistHandler_CreateLoader(nx_uri_t filename, ifc_playlistloader **loader);
	
};