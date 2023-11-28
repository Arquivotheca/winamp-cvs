#include "api.h"
#include "AutoTagTrack.h"
#include "Gracenote.h"
#include "MusicID_File_Populate.h"
#include "nswasabi/AutoCharNX.h"
#include "MetadataGDO.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

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

gnsdk_void_t AutoTagTrack::gn_callback_status(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_musicidfile_callback_status_t status, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
		if (status == gnsdk_musicidfile_status_fileinfo_processing_complete && autotag_track->callback)
	{
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
				autotag_track->callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_DONE);
			}
		}
	}
}

gnsdk_void_t AutoTagTrack::gn_callback_get_fingerprint(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
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
			if (autotag_track->callback)
				autotag_track->callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ANALYZING);
			MusicID_File_Fingerprint(filename, fileinfo);
		}
	}
}

gnsdk_void_t AutoTagTrack::gn_callback_get_metadata(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file, gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
	/* extract filename from the fileinfo object */
	if (autotag_track->user_metadata)
	{
		/* TODO: use user-provided metadata if present */
	}
	
		gnsdk_cstr_t filename_utf8;
		gnsdk_error_t gn_error = gnsdk_musicidfile_fileinfo_metadata_get(fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME, &filename_utf8, 0);
		if (gn_error == GNSDK_SUCCESS)
		{
		
			/* convert the utf8 string from gracenote into an nx_uri_t */
			ReferenceCountedNXURI filename;
			int ret = NXURICreateWithUTF8(&filename, filename_utf8);
			if (ret == NErr_Success)
			{
				if (autotag_track->callback)
					autotag_track->callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_READING);
				MusicID_File_Metadata(filename, fileinfo);
			}
		}

}

static gnsdk_void_t gn_callback_result_available(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_gdo_handle_t album_response, gnsdk_uint32_t current_album,	gnsdk_uint32_t total_albums, gnsdk_bool_t* p_abort)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
}

static gnsdk_void_t gn_callback_result_not_found(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_musicidfile_fileinfo_handle_t fileinfo, gnsdk_uint32_t current_file,	gnsdk_uint32_t total_files, gnsdk_bool_t* p_abort)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
}

static gnsdk_void_t gn_callback_musicid_complete(const gnsdk_void_t* userdata, gnsdk_musicidfile_query_handle_t musicidfile_query_handle, gnsdk_error_t musicidfile_complete_error)
{
	AutoTagTrack *autotag_track = (AutoTagTrack *)userdata;
}

static gnsdk_musicidfile_callbacks_t gn_callbacks =
{
	AutoTagTrack::gn_callback_status,
	AutoTagTrack::gn_callback_get_fingerprint,
	AutoTagTrack::gn_callback_get_metadata,
	gn_callback_result_available,
	gn_callback_result_not_found,
	gn_callback_musicid_complete,
};

/* --------------- */
AutoTagTrack::AutoTagTrack()
{
	callback=0;
	filename=0;
	gn_query=0;
	gn_user=0;
	user_metadata=0;
}

AutoTagTrack::~AutoTagTrack()
{
	NXURIRelease(filename);

	if (gn_query)
	{
		gnsdk_musicidfile_query_release(gn_query);
	}

	if (callback)
		callback->Release();

	if (user_metadata)
		user_metadata->Release();
	/* TODO: release gn_user once we make Gracenote_GetUser do proper reference counting */
}

int AutoTagTrack::Initialize(ifc_gracenote_autotag_callback *callback)
{
	if (!callback)
		return NErr_BadParameter;

	this->callback = callback;
	callback->Retain();
	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_INITIALIZING);

	int ret = REPLICANT_API_GRACENOTE->GetUser(&gn_user);
	if (ret != NErr_Success)
		return ret;

	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: About to call gnsdk_musicidfile_query_create");
	gnsdk_error_t gn_error = gnsdk_musicidfile_query_create(gn_user, &gn_callbacks, (AutoTagTrack *)this, &gn_query);
	if (gn_error != 0)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: ERROR in gnsdk_musicidfile_query_create");
		return NErr_Error;
	}

	gnsdk_musicidfile_query_option_set(gn_query, GNSDK_MUSICIDFILE_OPTION_ENABLE_LINK_DATA , GNSDK_MUSICIDFILE_OPTION_VALUE_TRUE );

	return NErr_Success;
}

int AutoTagTrack::AutoTag_Track_Run(nx_uri_t filename)
{
	this->filename = NXURIRetain(filename);
	gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo=0; 
	gnsdk_error_t gn_error=gnsdk_musicidfile_query_fileinfo_create(gn_query, "autotag_track", 0, 0, &gn_fileinfo);			                                                              
	if (gn_error != 0)
		return NErr_Error;

	callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ADDING);

	int ret = MusicID_File_Populate(gn_fileinfo, filename);
	if (ret != NErr_Success)
	{
		return ret;
	}

	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_QUERYING);


	gn_error = gnsdk_musicidfile_query_do_trackid(gn_query, GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL);
	gnsdk_gdo_handle_t gdo;

	if (gnsdk_musicidfile_fileinfo_get_response_gdo(gn_fileinfo, &gdo) == GNSDK_SUCCESS)
	{
		gnsdk_uint32_t count;
		if (gnsdk_manager_gdo_child_count(gdo, GNSDK_GDO_CHILD_ALBUM, &count) == GNSDK_SUCCESS)
		{
			for (gnsdk_uint32_t i=0;i<count;i++)
			{
				gnsdk_gdo_handle_t album_gdo;
				if (gnsdk_manager_gdo_child_get(gdo, GNSDK_GDO_CHILD_ALBUM, i+1, &album_gdo) == GNSDK_SUCCESS)
				{
					MetadataGDO_TrackMatch *metadata = new (std::nothrow) ReferenceCounted<MetadataGDO_TrackMatch>;
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
	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_DONE);
	return NErr_Success;
}

int AutoTagTrack::AutoTag_Track_Run_Simple(nx_string_t artist, nx_string_t title)
{
	gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo=0; 
	gnsdk_error_t gn_error=gnsdk_musicidfile_query_fileinfo_create(gn_query, "autotag_track", 0, 0, &gn_fileinfo);			                                                              
	if (gn_error != 0)
		return NErr_Error;

	callback->OnStatus(filename, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_FILE_ADDING);

	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_QUERYING);

	AutoCharUTF8 utf8;
	utf8.Set(artist);
	gnsdk_musicidfile_fileinfo_metadata_set(gn_fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKARTIST, utf8);
	utf8.Set(title);
	gnsdk_musicidfile_fileinfo_metadata_set(gn_fileinfo, GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKTITLE, utf8);

	gn_error = gnsdk_musicidfile_query_do_trackid(gn_query, GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE);
	gnsdk_gdo_handle_t gdo;

	if (gnsdk_musicidfile_fileinfo_get_response_gdo(gn_fileinfo, &gdo) == GNSDK_SUCCESS)
	{
		gnsdk_uint32_t count;
		if (gnsdk_manager_gdo_child_count(gdo, GNSDK_GDO_CHILD_ALBUM, &count) == GNSDK_SUCCESS)
		{
			for (gnsdk_uint32_t i=0;i<count;i++)
			{
				gnsdk_gdo_handle_t album_gdo;
				if (gnsdk_manager_gdo_child_get(gdo, GNSDK_GDO_CHILD_ALBUM, i+1, &album_gdo) == GNSDK_SUCCESS)
				{
					MetadataGDO_TrackMatch *metadata = new (std::nothrow) ReferenceCounted<MetadataGDO_TrackMatch>;
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
	callback->OnStatus(0, ifc_gracenote_autotag_callback::STATUS_AUTOTAG_DONE);
	return NErr_Success;
}

int AutoTagTrack::AutoTag_Track_Save(ifc_gracenote_results *results, int flags)
{
	if (!results)
		return NErr_BadParameter;

	MetadataGDO_TrackMatch *metadata = (MetadataGDO_TrackMatch *)results;
	return metadata->SaveTo(filename, flags);
}