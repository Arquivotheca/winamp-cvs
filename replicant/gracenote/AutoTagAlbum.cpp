#include "main.h"
#include "api.h"
#include "AutoTagAlbum.h"
#include "Gracenote.h"
#include "MusicID_File_Populate.h"
#include "MetadataGDO.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include <new>
#include <stdio.h>

#ifdef  __ANDROID__
#include <android/log.h>
#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
#define ANDROID_LOG_DEBUG 2
static void __android_log_print(int, const char *, const char *, ...)
{
}
#endif


//#define GN_AUTOTAG_USE_RESULT_CALLBACK

static const char *gn_status_string(gnsdk_musicidfile_callback_status_t status)
{
	static char temp_status[64];
	switch(status)
	{
	case gnsdk_musicidfile_status_fileinfo_processing_begin:
		return "File Info Processing Begin";
	case gnsdk_musicidfile_status_fileinfo_query:
		return "File Info Query";

	case gnsdk_musicidfile_status_fileinfo_processing_complete:
		return "File Info Processing Complete";

	case gnsdk_musicidfile_status_fileinfo_processing_error:
		return "File Info Processing Error";

	case gnsdk_musicidfile_status_error:
		return "Error";
	default:
		return 0;
	}
}

gnsdk_void_t AutoTagAlbum::gn_callback_status(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_musicidfile_callback_status_t status, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagAlbum *autotag_album = (AutoTagAlbum *)userdata;
	if (autotag_album->callback)
	{
		int code;
		switch(status)
		{
		case gnsdk_musicidfile_status_fileinfo_processing_begin:
			code = ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_READING;
			break;
		case gnsdk_musicidfile_status_fileinfo_query:
			code = ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ANALYZING;
			break;
		case gnsdk_musicidfile_status_fileinfo_processing_complete:
			code = ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_DONE;
			break;
		default:
			return;
		}

		/* extract filename from the fileinfo object */
		gnsdk_cstr_t filename_utf8;
		gnsdk_error_t gn_error = gnsdk_musicidfile_fileinfo_metadata_get(fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME, &filename_utf8, 0);
		if (gn_error == GNSDK_SUCCESS)
		{
			/* convert the utf8 string from gracenote into an nx_uri_t */
			ReferenceCountedNXURI filename;
			int ret = NXURICreateWithUTF8(&filename, filename_utf8);
			if (ret == NErr_Success)
			{
				autotag_album->callback->OnStatus(filename, code);
			}
		}
	}
}

gnsdk_void_t AutoTagAlbum::gn_callback_get_fingerprint(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagAlbum *autotag_album = (AutoTagAlbum *)userdata;
	/* extract filename from the fileinfo object */
	gnsdk_cstr_t filename_utf8;
	gnsdk_error_t gn_error = gnsdk_musicidfile_fileinfo_metadata_get(fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME, &filename_utf8, 0);
	if (gn_error == GNSDK_SUCCESS)
	{
		/* convert the utf8 string from gracenote into an nx_uri_t */
		ReferenceCountedNXURI filename;
		int ret = NXURICreateWithUTF8(&filename, filename_utf8);
		if (ret == NErr_Success)
		{
			MusicID_File_Fingerprint(filename, fileinfo);
		}
	}
}

gnsdk_void_t AutoTagAlbum::gn_callback_get_metadata(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
		AutoTagAlbum *autotag_album = (AutoTagAlbum *)userdata;
	/* extract filename from the fileinfo object */
	gnsdk_cstr_t filename_utf8;
	gnsdk_error_t gn_error = gnsdk_musicidfile_fileinfo_metadata_get(fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME, &filename_utf8, 0);
	if (gn_error == GNSDK_SUCCESS)
	{
		/* convert the utf8 string from gracenote into an nx_uri_t */
		ReferenceCountedNXURI filename;
		int ret = NXURICreateWithUTF8(&filename, filename_utf8);
		if (ret == NErr_Success)
		{
			MusicID_File_Metadata(filename, fileinfo);
		}
	}	
}

gnsdk_void_t AutoTagAlbum::gn_callback_result_available(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_gdo_handle_t album_response, gnsdk_uint32_t current_album,	gnsdk_uint32_t total_albums, gnsdk_bool_t* p_abort)
{
	AutoTagAlbum *autotag_album = (AutoTagAlbum *)userdata;
#ifdef GN_AUTOTAG_USE_RESULT_CALLBACK
	if (autotag_album->callback)
	{
		gnsdk_uint32_t count;
		if (gnsdk_manager_gdo_child_count(album_response, GNSDK_GDO_CHILD_ALBUM, &count) == GNSDK_SUCCESS)
		{
			for (gnsdk_uint32_t i=0;i<count;i++)
			{
				gnsdk_gdo_handle_t album_gdo;
				if (gnsdk_manager_gdo_child_get(album_response, GNSDK_GDO_CHILD_ALBUM, i+1, &album_gdo) == GNSDK_SUCCESS)
				{
					MetadataGDO_AlbumMatch *metadata = new (std::nothrow) ReferenceCounted<MetadataGDO_AlbumMatch>;
					if (metadata)
					{
						if (metadata->Initialize(album_gdo) == NErr_Success)
						{
							autotag_album->callback->OnResults(metadata);
						}
						metadata->Release();
					}
					gnsdk_manager_gdo_release(album_gdo);
				}
			}			
		}	
	}
#endif
}

gnsdk_void_t gn_callback_result_not_found(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file,	gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagAlbum *autotag_track = (AutoTagAlbum *)userdata;
}

gnsdk_void_t gn_callback_musicid_complete(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_error_t musicidfile_complete_error)
{
	AutoTagAlbum *autotag_track = (AutoTagAlbum *)userdata;
}

static gnsdk_musicidfile_callbacks_t gn_callbacks =
{
	AutoTagAlbum::gn_callback_status,
	AutoTagAlbum::gn_callback_get_fingerprint,
	AutoTagAlbum::gn_callback_get_metadata,
	AutoTagAlbum::gn_callback_result_available,
	gn_callback_result_not_found,
	gn_callback_musicid_complete,
};

/* --------------- */
AutoTagAlbum::AutoTagAlbum()
{
	callback=0;
	gn_query=0;
	gn_user=0;
	}

AutoTagAlbum::~AutoTagAlbum()
{
	if (gn_query)
	{
		gnsdk_musicidfile_query_release(gn_query);
	}

	if (callback)
		callback->Release();

	/* TODO: release gn_user once we make Gracenote_GetUser do proper reference counting */
}

int AutoTagAlbum::Initialize(ifc_gracenote_autotag_callback *callback)
{
	if (!callback)
		return NErr_BadParameter;

	this->callback = callback;
	callback->Retain();
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","AutoTagAlbum: callback->OnStatus(ifc_gracenote_autotag_callback::STATUS_AUTOTAG_INITIALIZING);");
	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_INITIALIZING);
	
	int ret = REPLICANT_API_GRACENOTE->GetUser(&gn_user);
	if (ret != NErr_Success)
		return ret;

	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","AutoTagAlbum: About to call gnsdk_musicidfile_query_create");
	gnsdk_error_t gn_error = gnsdk_musicidfile_query_create(gn_user, &gn_callbacks, (AutoTagAlbum *)this, &gn_query);
	if (gn_error != 0)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","AutoTagAlbum: ERROR in gnsdk_musicidfile_query_create");
		return NErr_Error;
	}

	gnsdk_musicidfile_query_option_set(gn_query, GNSDK_MUSICIDFILE_OPTION_ENABLE_LINK_DATA, GNSDK_MUSICIDFILE_OPTION_VALUE_TRUE);

	return NErr_Success;
}

int AutoTagAlbum::AutoTag_Album_Add(nx_uri_t filename)
{
	size_t index = files.size();
	char temp[64];
	sprintf(temp, "%u", index);

	gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo=0; 
	gnsdk_error_t gn_error=gnsdk_musicidfile_query_fileinfo_create(gn_query, temp, 0, 0, &gn_fileinfo);							                                                              
	if (gn_error != 0)
		return NErr_Error;

	callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ADDING);

	int ret = MusicID_File_Populate(gn_fileinfo, filename);
	if (ret != NErr_Success)
		return ret;

	files.push_back(gn_fileinfo);

	return NErr_Success;
}

int AutoTagAlbum::AutoTag_Album_AddSimple(nx_string_t artist, nx_string_t title)
{
	size_t index = files.size();
	char temp[64];
	sprintf(temp, "%u", index);

	gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo=0; 
	gnsdk_error_t gn_error=gnsdk_musicidfile_query_fileinfo_create(gn_query, temp, 0, 0, &gn_fileinfo);							                                                              
	if (gn_error != 0)
		return NErr_Error;

	//callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ADDING);

	AutoCharUTF8 utf8;
	utf8.Set(artist);
	gnsdk_musicidfile_fileinfo_metadata_set(gn_fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKARTIST, utf8);
	utf8.Set(title);
	gnsdk_musicidfile_fileinfo_metadata_set(gn_fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKTITLE, utf8);

	//int ret = MusicID_File_Populate(gn_fileinfo, filename);
	//if (ret != NErr_Success)
	//return ret;

	files.push_back(gn_fileinfo);

	return NErr_Success;
}

int AutoTagAlbum::AutoTag_Album_Run(int query_flag)
{
	gnsdk_uint32_t flags = 0;
	gnsdk_error_t gn_error;
	gnsdk_gdo_handle_t gdo;

	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_QUERYING);

	if (query_flag == AUTOTAG_QUERY_FLAG_DEFAULT)			// The old default behavior, all results for track, and 
	{
		if (files.size() == 1)
		{
			flags = GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL;
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[AutoTagAlbum]: Enabling ALL results for gracenote query");
		}
		else
		{
			flags = GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE;
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[AutoTagAlbum]: Enabling SINGLE result for gracenote query");
		}
	}
	else if ((query_flag & AUTOTAG_QUERY_FLAG_RETURN_ALL) == AUTOTAG_QUERY_FLAG_RETURN_ALL)			// Specifically asked for all results
	{
		flags = GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL;
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[AutoTagAlbum]: Enabling ALL results for gracenote query");
	}
	else if ((query_flag & AUTOTAG_QUERY_FLAG_RETURN_SINGLE) == AUTOTAG_QUERY_FLAG_RETURN_SINGLE)	// Specifically asked for single results
	{
		flags = GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE;
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[AutoTagAlbum]: Enabling SINGLE result for gracenote query");
	}
	
	if ((query_flag & AUTOTAG_QUERY_FLAG_NO_THREADS) == AUTOTAG_QUERY_FLAG_NO_THREADS)	
	{
		flags = flags | GNSDK_MUSICIDFILE_QUERY_FLAG_NO_THREADS;
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[AutoTagAlbum]: Disabling threads in MusicID-File");
	}

	if (files.size() == 1)
		gn_error = gnsdk_musicidfile_query_do_trackid(gn_query, flags);
	else
		gn_error = gnsdk_musicidfile_query_do_albumid(gn_query, flags);


#ifndef GN_AUTOTAG_USE_RESULT_CALLBACK
	if (gnsdk_musicidfile_query_get_response_gdo(gn_query, &gdo) ==  GNSDK_SUCCESS)
	{
		gnsdk_uint32_t count;
		if (gnsdk_manager_gdo_child_count(gdo, GNSDK_GDO_CHILD_ALBUM, &count) == GNSDK_SUCCESS)
		{
			for (gnsdk_uint32_t i=0;i<count;i++)
			{
				gnsdk_gdo_handle_t album_gdo;
				if (gnsdk_manager_gdo_child_get(gdo, GNSDK_GDO_CHILD_ALBUM, i+1, &album_gdo) == GNSDK_SUCCESS)
				{
					MetadataGDO_AlbumMatch *metadata = new (std::nothrow) ReferenceCounted<MetadataGDO_AlbumMatch>;
					if (metadata)
					{
						if (metadata->Initialize(album_gdo) == NErr_Success)
						{
							callback->OnResults(metadata);
						}
						metadata->Release();
					}
					gnsdk_manager_gdo_release(album_gdo);
				}
			}
			gnsdk_manager_gdo_release(gdo);
		}	
	}
#endif
	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_DONE);
	return NErr_Success;
}

int AutoTagAlbum::AutoTag_Album_SaveAll(ifc_gracenote_results *results, int flags)
{
	if (!results)
		return NErr_BadParameter;

	MetadataGDO_AlbumMatch *metadata = (MetadataGDO_AlbumMatch *)results;
	
	for (unsigned int index=0;;index++)
	{
		ifc_metadata *track;
		int ret = metadata->GetMetadata(MetadataKey_MatchedTrack, index, &track);
		if (ret == NErr_Success)
		{
			ret = AutoTag_Album_SaveTrack((ifc_gracenote_results *)track, flags);
			track->Release();
		}
		else if (ret == NErr_EndOfEnumeration)
		{
			return NErr_Success;
		}
		else
			return ret;
	}
}

int AutoTagAlbum::AutoTag_Album_SaveTrack(ifc_gracenote_results *results, int flags)
{
	if (!results)
		return NErr_BadParameter;


	nx_string_t filename;
	int ret = results->GetField(MetadataKeys::URI, 0, &filename);
	if (ret != NErr_Success)
		return ret;

	nx_uri_t uri_filename;
	ret = NXURICreateWithNXString(&uri_filename, filename);
	if (ret != NErr_Success)
	{
		NXStringRelease(filename);
		return ret;
	}
	MetadataGDO_TrackMatch *metadata = (MetadataGDO_TrackMatch *)results;
	ret = metadata->SaveTo(uri_filename, flags);
	NXURIRelease(uri_filename);
	return ret;
}
