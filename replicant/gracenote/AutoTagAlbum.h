#pragma once
#include "nx/nxuri.h"
#include "nx/nxthread.h"
#include "gnsdk.h"
#include "metadata/ifc_metadata.h"
#include "foundation/dispatch.h"
#include "ifc_gracenote_results.h"
#include "ifc_gracenote_callback.h"
#include "nu/PtrList.h"
#include "ifc_gracenote_autotag_album.h"



class AutoTagAlbum : public ifc_gracenote_autotag_album
{
public:
	AutoTagAlbum();
	~AutoTagAlbum();

	int Initialize(ifc_gracenote_autotag_callback *callback);
	
	static gnsdk_void_t gn_callback_status(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_musicidfile_callback_status_t status, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
	static gnsdk_void_t gn_callback_get_fingerprint(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
	static gnsdk_void_t gn_callback_get_metadata(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort);
	static gnsdk_void_t gn_callback_result_available(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_gdo_handle_t album_response, gnsdk_uint32_t current_album,	gnsdk_uint32_t total_albums, gnsdk_bool_t* p_abort);
private:
	ifc_gracenote_autotag_callback *callback;
	nu::HandleList<gnsdk_musicidfile_fileinfo_handle_t> files;

	gnsdk_musicidfile_query_handle_t gn_query;
	gnsdk_user_handle_t gn_user;

	int WASABICALL AutoTag_Album_Add(nx_uri_t filename);
	int WASABICALL AutoTag_Album_Run(int query_flag);
	int WASABICALL AutoTag_Album_SaveAll(ifc_gracenote_results *results, int flags);
	int WASABICALL AutoTag_Album_SaveTrack(ifc_gracenote_results *results, int flags);
	int WASABICALL AutoTag_Album_AddSimple(nx_string_t artist, nx_string_t title);
	
	
};

