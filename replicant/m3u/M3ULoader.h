#pragma once
#include "playlist/ifc_playlistloader.h"
#include "metadata/ifc_metadata.h"
#include "nx/nxuri.h"

class M3ULoader : public ifc_playlistloader/* TODO: , public ifc_metadata*/
{
public:
	M3ULoader();
	~M3ULoader();
private:
	/* ifc_playlistloader */
	int WASABICALL PlaylistLoader_Load(nx_uri_t filename, cb_playlistloader *playlist);

	/* ifc_metadata - FOR PLAYLIST INFO, NOT FILE INFO! */
#if 0 // TODO
	int WASABICALL Metadata_GetField(int field, int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, int index, double *value);
#endif

	nx_string_t filename;
};
