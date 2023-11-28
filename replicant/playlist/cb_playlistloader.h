#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "metadata/ifc_metadata.h"
#include "nx/nxstring.h"

class ifc_playlistinfo; // TODO
class cb_playlistloader : public Wasabi2::Dispatchable
{
protected:
	cb_playlistloader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_playlistloader() {}
public:
	// return NErr_True to continue enumeration, or NErr_False (or any other error) to quit
	int OnFile(ifc_metadata *info) { return PlaylistLoaderCallback_OnFile(info); }
	int OnPlaylistInfo(ifc_metadata *info);

	/* return 0 to use playlist file path as base (or just don't implement).  Add a reference before returning! */
	nx_string_t GetBasePath() { return PlaylistLoaderCallback_GetBasePath(); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	/* Note to implementors of PlaylistLoaderCallback_OnFile
	return NErr_True to continue enumeration, or NErr_False (or any other error) to quit
	two caveats about the ifc_metadata object that is passed to callbacks
	1) It only gives metadata _that is provided in the playlist data_.
	Usually this is limited to MetadataKeys::URI (for filename) and MetadataKeys::TITLE
	if you need metadata from the file (e.g. Artist, Song Title), use api_metadata
	2) The metadata object is only valid for the duration of the function call
	if you want to queue up work to another thread, you'll want to query metadata during the function call
	and pass the data you got (e.g. the nx_string_t of the filename)
	*/
	virtual int WASABICALL PlaylistLoaderCallback_OnFile(ifc_metadata *info)=0;
	virtual nx_string_t WASABICALL PlaylistLoaderCallback_GetBasePath() { return 0; }
};
