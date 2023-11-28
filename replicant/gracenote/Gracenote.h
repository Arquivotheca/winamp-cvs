#pragma once
#include "gnsdk.h"
#include "api_gracenote.h"
#include "nswasabi/ServiceName.h"
#include "nx/nxonce.h"
#include "nx/nxthread.h"
#include "ifc_gracenote_autotag_track.h"
#include "ifc_gracenote_autotag_album.h"

class GracenoteAPI : public api_gracenote
{
public:
	GracenoteAPI();
	WASABI_SERVICE_NAME("Gracenote API");
	int GetManager(gnsdk_manager_handle_t *manager_handle);
	int GetUser(gnsdk_user_handle_t *user_handle);
	int LinkCreate(gnsdk_link_query_handle_t *link_handle);

private:
	gnsdk_manager_handle_t gracenote_manager_handle;
	gnsdk_locale_handle_t gracenote_locale_handle;
	int gracenote_manager_error;
	
	gnsdk_user_handle_t gracenote_user_handle;
	int gracenote_user_error;

	
	nx_once_value_t gracenote_manager_once;
	nx_once_value_t gracenote_user_once;
	nx_once_value_t gracenote_dsp_once;
	nx_once_value_t gracenote_link_once;
	nx_once_value_t gracenote_musicid_file_once;

	static int NX_ONCE_API ManagerOnce(nx_once_t once, void *me, void **unused);
	static int NX_ONCE_API UserOnce(nx_once_t once, void *me, void **unused);
	static int NX_ONCE_API DSPOnce(nx_once_t once, void *me, void **unused);
	static int NX_ONCE_API LinkOnce(nx_once_t once, void *me, void **unused);
	static int NX_ONCE_API MusicIDFileOnce(nx_once_t once, void *me, void **unused);
	
	static nx_thread_return_t NXTHREADCALL InitializeThreadProcedure(nx_thread_parameter_t parameter);

	int WASABICALL Gracenote_CreateAutoTag_Track(ifc_gracenote_autotag_track **autotag_track, ifc_gracenote_autotag_callback *callback, int flags);
	int WASABICALL Gracenote_CreateAutoTag_Album(ifc_gracenote_autotag_album **autotag_album, ifc_gracenote_autotag_callback *callback, int flags);
	int WASABICALL Gracenote_SaveAlbumResults(ifc_gracenote_results *results, int flags);
	int WASABICALL Gracenote_SaveTrackResults(nx_uri_t filename, ifc_gracenote_results *results, int flags);
	int WASABICALL Gracenote_Initialize();
	int WASABICALL Gracenote_GetVersion(nx_string_t *version);
	int WASABICALL Gracenote_GetBuildDate(nx_string_t *build_date);
};