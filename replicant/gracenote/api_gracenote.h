#pragma once
#include "foundation/dispatch.h"
#include "ifc_gracenote_autotag_track.h"
#include "ifc_gracenote_autotag_album.h"
#include "service/types.h"
#include "nx/nxuri.h"
// {D354C143-26FB-41B2-9F4F-CECCF8795014}
static const GUID gracenote_api_guid = 
{ 0xd354c143, 0x26fb, 0x41b2, { 0x9f, 0x4f, 0xce, 0xcc, 0xf8, 0x79, 0x50, 0x14 } };


class api_gracenote : public Wasabi2::Dispatchable
{
protected:
	api_gracenote() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_gracenote() {}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return gracenote_api_guid; }
	/* flags are currently undefined, so pass 0 */
	int CreateAutoTag_Track(ifc_gracenote_autotag_track **autotag_track, ifc_gracenote_autotag_callback *callback, int flags) { return Gracenote_CreateAutoTag_Track(autotag_track, callback, flags); }
	int CreateAutoTag_Album(ifc_gracenote_autotag_album **autotag_album, ifc_gracenote_autotag_callback *callback, int flags) { return Gracenote_CreateAutoTag_Album(autotag_album, callback, flags); }
	/* flags can be SAVE_RESULTS_* */
	int SaveAlbumResults(ifc_gracenote_results *results, int flags) { return Gracenote_SaveAlbumResults(results, flags); }
	/* passing filename=NULL will cause this function to query the results object for the filename */
	int SaveTrackResults(nx_uri_t filename, ifc_gracenote_results *results, int flags) { return Gracenote_SaveTrackResults(filename, results, flags); }

	int Initialize() { return Gracenote_Initialize(); }

	int GetVersion(nx_string_t *version) { return Gracenote_GetVersion(version); }
	int GetBuildDate(nx_string_t *build_date) { return Gracenote_GetBuildDate(build_date); }
	enum
	{
		SAVE_RESULTS_DEFAULT = 0,
		SAVE_NO_METADATA = (1 << 0),
		SAVE_NO_COVER_ART = (1 << 1),
		SAVE_FORCE_EMBED_ART = (1 << 2), // set this flag if you want to always embed the album art
	};

	enum
	{
		DISPATCHABLE_VERSION=0,
	};

private:
	virtual int WASABICALL Gracenote_CreateAutoTag_Track(ifc_gracenote_autotag_track **autotag_track, ifc_gracenote_autotag_callback *callback, int flags)=0;	
	virtual int WASABICALL Gracenote_CreateAutoTag_Album(ifc_gracenote_autotag_album **autotag_album, ifc_gracenote_autotag_callback *callback, int flags)=0;	
	virtual int WASABICALL Gracenote_SaveAlbumResults(ifc_gracenote_results *results, int flags)=0;
	virtual int WASABICALL Gracenote_SaveTrackResults(nx_uri_t filename, ifc_gracenote_results *results, int flags)=0;
	virtual int WASABICALL Gracenote_Initialize()=0;
	virtual int WASABICALL Gracenote_GetVersion(nx_string_t *version)=0;
	virtual int WASABICALL Gracenote_GetBuildDate(nx_string_t *build_date)=0;
};