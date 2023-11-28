#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "cb_playlistloader.h"

class ifc_playlistloader : public Wasabi2::Dispatchable
{
protected:
	ifc_playlistloader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_playlistloader() {}

public:
	int Load(nx_uri_t filename, cb_playlistloader *playlist) { return PlaylistLoader_Load(filename, playlist); }

	enum
	{
		DISPATCHABLE_VERSION = 0,
	};
private:
		virtual int WASABICALL PlaylistLoader_Load(nx_uri_t filename, cb_playlistloader *playlist)=0;
};
