#pragma once
#include "nx/nxuri.h"
#include "nx/nxthread.h"
#include "gnsdk.h"
#include "metadata/ifc_metadata.h"
#include "foundation/dispatch.h"
#include "ifc_gracenote_results.h"
#include "ifc_gracenote_autotag_track.h"
/* this one auto-tags a single track */



class AutoTagTrack : public ifc_gracenote_autotag_track
{
public:
	AutoTagTrack();
	~AutoTagTrack();

	int Initialize(ifc_gracenote_autotag_callback *callback);
	
	static gnsdk_void_t gn_callback_status(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_musicidfile_callback_status_t status, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
	static gnsdk_void_t gn_callback_get_fingerprint(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
	static gnsdk_void_t gn_callback_get_metadata(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
private:
	ifc_gracenote_autotag_callback *callback;
	nx_uri_t filename;
	ifc_metadata *user_metadata;
	gnsdk_musicidfile_query_handle_t gn_query;
	gnsdk_user_handle_t gn_user;
	

	int WASABICALL AutoTag_Track_Run(nx_uri_t filename);
	int WASABICALL AutoTag_Track_Run_Simple(nx_string_t artist, nx_string_t title);
	int WASABICALL AutoTag_Track_Save(ifc_gracenote_results *results, int flags);
};

