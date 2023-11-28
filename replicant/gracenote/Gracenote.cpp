#include "main.h"
#include "api.h"
#include "Gracenote.h"
#include "MetadataGDO.h"
#include "nx/nxonce.h"
#include "nx/nxfile.h"
#include "AutoTagTrack.h"
#include "AutoTagAlbum.h"
#include "nswasabi/ReferenceCounted.h"
#include "GracenoteSysCallback.h"
#include <new>
#include "foundation/error.h"
#include "nswasabi/AutoCharNX.h"
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


static int BuildDataPath(nx_uri_t *full_path, const char *filename)
{
	int ret;
	nx_uri_t data_path=0;
	ret = WASABI2_API_APP->GetDataPath(&data_path);
	if (ret != NErr_Success)
		return ret;

	ReferenceCountedNXString gn_user_filename;
	ret = NXStringCreateWithUTF8(&gn_user_filename, filename);
	if (ret != NErr_Success)
		return ret;
	ReferenceCountedNXURI gn_user_filename_uri;
	ret = NXURICreateWithNXString(&gn_user_filename_uri, gn_user_filename);
	if (ret != NErr_Success)
		return ret;

	return NXURICreateWithPath(full_path, gn_user_filename_uri, data_path);
}

// use free() to release the data, on NErr_Success
static int ReadData(FILE *f, gnsdk_char_t **out_data)
{
	// get the length of the file
	fseek(f, 0, SEEK_END);
	size_t length = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);

	length++;
	if (length == 0)
		return NErr_IntegerOverflow;

	gnsdk_char_t *data = (gnsdk_char_t *)malloc(length);
	fgets(data, length, f);
	*out_data=data;
	return NErr_Success;
}

GracenoteAPI::GracenoteAPI()
{
	NXOnceInit(&gracenote_manager_once);
	NXOnceInit(&gracenote_user_once);
	NXOnceInit(&gracenote_dsp_once);
	NXOnceInit(&gracenote_link_once);
	NXOnceInit(&gracenote_musicid_file_once);

	gracenote_manager_handle=0;
	gracenote_manager_error = NErr_Empty;
	gracenote_locale_handle=0;
	gracenote_user_handle=0;
	gracenote_user_error=NErr_Empty;
}

static const char *gracenote_license_data = 
	"-- BEGIN LICENSE v1.0 0D4E76AD --\n"
	"licensee: Gracenote, Inc.\n"
	"name: Winamp Mobile and Play GNSDK License 011812 \n"
	"start_date: 2012-01-18\n"
	"\n"
	"client_id: 5081088\n"
	"musicid_file: enabled\n"
	"musicid_search: enabled\n"
	"musicid: enabled\n"
	"\n"
	"-- SIGNATURE 0D4E76AD --\n"
	"lAADAgAfAYHHad8X0O3HA/Jz6WX5OBn0huUxtO/gBTwHehcuxAAfAR/WO22UIm3LxR9DoveB9tVYJx2kHslFRW9yKNnnOQ==\n"
	"-- END LICENSE 0D4E76AD --\n";

int GracenoteAPI::ManagerOnce(nx_once_t once, void *_me, void **unused)
{
	GracenoteAPI *me = (GracenoteAPI *)_me;
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_MANAGER_LOADING);
	gnsdk_error_t gn_error = gnsdk_manager_initialize(&me->gracenote_manager_handle, gracenote_license_data, GNSDK_MANAGER_LICENSEDATA_NULLTERMSTRING);	
	if (gn_error != 0)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_MANAGER_LOADING, NErr_FailedCreate);
		me->gracenote_manager_error = NErr_FailedCreate;
		me->gracenote_manager_handle=0; /* just in case it got trashed */
	}
	else
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_MANAGER_LOADED);
		me->gracenote_manager_error = NErr_Success;
	}
	return 1;
}

int GracenoteAPI::GetManager(gnsdk_manager_handle_t *manager_handle)
{
	NXOnce(&gracenote_manager_once, ManagerOnce, (GracenoteAPI *)this);
	*manager_handle = gracenote_manager_handle;
	return gracenote_manager_error;
}

/* ======================================================== */

int GracenoteAPI::UserOnce(nx_once_t once, void *_me, void **unused)
{
	GracenoteAPI *me = (GracenoteAPI *)_me;
	ReferenceCountedNXURI gn_user_data_path, gn_locale_data_path;
	gnsdk_error_t gn_error;


	gnsdk_manager_handle_t manager=0;
	int ret = me->GetManager(&manager);
	if (ret != NErr_Success)
	{
		me->gracenote_user_error=ret;
		return 1;
	}

	// Enable gracenote logging
	//nx_uri_t gn_log_uri;
    //if (BuildDataPath(&gn_log_uri, "gnsdk_replicant.log") == NErr_Success)
    //{
	//	AutoCharUTF8 utf8(gn_log_uri);
	    //gn_error = gnsdk_manager_logging_enable(utf8, GNSDK_LOG_PKG_ALL, GNSDK_LOG_LEVEL_ALL, GNSDK_LOG_OPTION_ALL, 0, GNSDK_TRUE);
        // TODO: Put this back to a real path, just for debugging purposes for now ^^^
	//	gn_error = gnsdk_manager_logging_enable("/mnt/sdcard/Winamp/gnsdk_replicant.log", GNSDK_LOG_PKG_ALL, GNSDK_LOG_LEVEL_ALL, GNSDK_LOG_OPTION_ALL, 0, GNSDK_TRUE);
    //    NXURIRelease(gn_log_uri);
    //}

	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_USER_LOADING);

	/* try to user data read from file */
	if (BuildDataPath(&gn_user_data_path, "gnuser.dat") == NErr_Success)
	{
		FILE *f = NXFile_fopen(gn_user_data_path, nx_file_FILE_read_binary);
		if (f)
		{
			gnsdk_char_t *serialized_string=0;
			ret = ReadData(f, &serialized_string);
			fclose(f);
			if (ret == NErr_Success)
			{
				gn_error = gnsdk_manager_user_deserialize(serialized_string, &me->gracenote_user_handle);

				if (gn_error != GNSDK_SUCCESS)
					me->gracenote_user_handle = 0; // just in case

				free(serialized_string);
			}
		}
	}

	/* register, if we have to */
	if (!me->gracenote_user_handle)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_USER_REGISTRATION);
		gn_error = gnsdk_manager_user_register_new("5081088", "B0FDDA2538AE63B2BA9BC89097C48CBD", "1", &me->gracenote_user_handle);
		if (gn_error != GNSDK_SUCCESS)
		{
			me->gracenote_user_handle=0; /* just in case it got trashed */
			me->gracenote_user_error = gn_error;
			WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_USER_REGISTRATION, NErr_Error);
			return 1;
		}

		if (gn_user_data_path)
		{
			gnsdk_str_t serialized_user_string=0;
			gn_error = gnsdk_manager_user_serialize(me->gracenote_user_handle, &serialized_user_string);
			if (gn_error == GNSDK_SUCCESS)
			{
				FILE *f = NXFile_fopen(gn_user_data_path, nx_file_FILE_write_binary);
				if (f)
				{
					fputs(serialized_user_string, f);
					fclose(f);
				}
				gnsdk_manager_string_free(serialized_user_string);
			}
		}
	}

	me->gracenote_user_error = NErr_Success;
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_USER_LOADED);

	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LOCALE_LOADING);
	/* try to locale data read from file */
	if (BuildDataPath(&gn_locale_data_path, "gnlocale.dat") == NErr_Success)
	{
		FILE *f = NXFile_fopen(gn_locale_data_path, nx_file_FILE_read_binary);
		if (f)
		{
			gnsdk_char_t *serialized_string=0;
			ret = ReadData(f, &serialized_string);
			fclose(f);
			if (ret == NErr_Success)
			{
				gn_error = gnsdk_manager_locale_deserialize(serialized_string, &me->gracenote_locale_handle);

				if (gn_error != GNSDK_SUCCESS)
					me->gracenote_locale_handle = 0; // just in case

				free(serialized_string);
			}
		}
	}

	/* download locale data, if we have to */
	if (!me->gracenote_locale_handle)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LOCALE_DOWNLOADING);
		gn_error = gnsdk_manager_locale_load(GNSDK_LOCALE_GROUP_MUSIC, GNSDK_LANG_ENGLISH, GNSDK_REGION_DEFAULT, GNSDK_DESCRIPTOR_DETAILED, me->gracenote_user_handle, 0, 0, &me->gracenote_locale_handle);
		if (gn_error != GNSDK_SUCCESS)
		{
			WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_LOCALE_DOWNLOADING, NErr_Error);
			me->gracenote_locale_handle=0; /* just in case it got trashed */
		}
		else if (gn_locale_data_path)
		{
			gnsdk_str_t serialized_locale_string=0;
			gn_error = gnsdk_manager_locale_serialize(me->gracenote_locale_handle, &serialized_locale_string);
			if (gn_error == GNSDK_SUCCESS)
			{
				size_t n = strlen(serialized_locale_string);
				FILE *f = NXFile_fopen(gn_locale_data_path, nx_file_FILE_write_binary);
				if (f)
				{
					fputs(serialized_locale_string, f);
					fclose(f);
				}
				gnsdk_manager_string_free(serialized_locale_string);
			}
		}
	}

	if (me->gracenote_locale_handle)
	{
		gnsdk_manager_locale_set_group_default(me->gracenote_locale_handle);
	}

	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LOCALE_LOADED);
	return 1;
}

int GracenoteAPI::GetUser(gnsdk_user_handle_t *user_handle)
{
	NXOnce(&gracenote_manager_once, ManagerOnce, (GracenoteAPI *)this);
	NXOnce(&gracenote_user_once, UserOnce, (GracenoteAPI *)this);
	*user_handle = gracenote_user_handle;
	return gracenote_user_error;
}

int GracenoteAPI::DSPOnce(nx_once_t once, void *_me, void **unused)
{
	GracenoteAPI *me = (GracenoteAPI *)_me;
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_DSP_LOADING);

	gnsdk_manager_handle_t manager=0;
	int ret = me->GetManager(&manager);
	if (ret != NErr_Success)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_DSP_LOADING, NErr_Error);
		return 1;
	}

	gnsdk_error_t gn_err = gnsdk_dsp_initialize(manager);
	if (gn_err == GNSDK_SUCCESS)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_DSP_LOADED);
	}
	else
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_DSP_LOADING, NErr_Error);
	}
	return 1;
}

int GracenoteAPI::LinkOnce(nx_once_t once, void *_me, void **unused)
{
	GracenoteAPI *me = (GracenoteAPI *)_me;
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LINK_LOADING);

	gnsdk_manager_handle_t manager=0;
	int ret = me->GetManager(&manager);
	if (ret != NErr_Success)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_LINK_LOADING, NErr_Error);
		return 1;
	}

	gnsdk_error_t gn_err = gnsdk_link_initialize(manager);
	if (gn_err == GNSDK_SUCCESS)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LINK_LOADED);
	}
	else
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_LINK_LOADING, NErr_Error);
	}
	return 1;
}

int GracenoteAPI::MusicIDFileOnce(nx_once_t once, void *_me, void **unused)
{
	GracenoteAPI *me = (GracenoteAPI *)_me;
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_MUSICID_FILE_LOADING);

	gnsdk_manager_handle_t manager=0;
	int ret = me->GetManager(&manager);
	if (ret != NErr_Success)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_MUSICID_FILE_LOADING, NErr_Error);
		return 1;
	}


	gnsdk_error_t gn_err = gnsdk_musicidfile_initialize(manager);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: gnsdk_musicidfile_initialize, gn_err: '%d'", gn_err);
	if (gn_err == GNSDK_SUCCESS)
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_MUSICID_FILE_LOADED);
	}
	else
	{
		WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_error, Gracenote::SystemCallback::STATUS_GRACENOTE_MUSICID_FILE_LOADING, NErr_Error);
	}
	return 1;
}

int GracenoteAPI::Gracenote_CreateAutoTag_Track(ifc_gracenote_autotag_track **autotag_track, ifc_gracenote_autotag_callback *callback, int flags)
{
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Entering Gracenote_CreateAutoTag_Track, autotag_track: '%x', callback: '%x'", autotag_track, callback);
	gnsdk_user_handle_t user_handle;
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Getting user...");
	int ret = GetUser(&user_handle);
	if (ret != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Getting user failed.");
		return ret;
	}
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: User setup successful.");

	NXOnce(&gracenote_dsp_once, DSPOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_dsp_once '%x'", gracenote_dsp_once);

	NXOnce(&gracenote_link_once, LinkOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_link_once '%x'", gracenote_link_once);

	NXOnce(&gracenote_musicid_file_once, MusicIDFileOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_musicid_file_once '%x'", gracenote_musicid_file_once);

	AutoTagTrack *autotag = new (std::nothrow) ReferenceCounted<AutoTagTrack>;
	if (!autotag)
		return NErr_OutOfMemory;

	ret = autotag->Initialize(callback);
	if (ret != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Failed to initialize track callbacks '%d'", ret);
		autotag->Release();
		return ret;
	}
	*autotag_track = autotag;
	return NErr_Success;
}

int GracenoteAPI::Gracenote_CreateAutoTag_Album(ifc_gracenote_autotag_album **autotag_album, ifc_gracenote_autotag_callback *callback, int flags)
{
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Entering Gracenote_CreateAutoTag_Album, autotag_album: '%x', callback: '%x'", autotag_album, callback);
	gnsdk_user_handle_t user_handle;
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Getting user...");
	int ret = GetUser(&user_handle);
	if (ret != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Getting user failed.");
		return ret;
	}
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: User setup successful.");

	NXOnce(&gracenote_dsp_once, DSPOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_dsp_once '%x'", gracenote_dsp_once);

	NXOnce(&gracenote_link_once, LinkOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_link_once '%x'", gracenote_link_once);

	NXOnce(&gracenote_musicid_file_once, MusicIDFileOnce, (GracenoteAPI *)this);
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Created gracenote_musicid_file_once '%x'", gracenote_musicid_file_once);

	AutoTagAlbum *autotag = new (std::nothrow) ReferenceCounted<AutoTagAlbum>;
	if (!autotag)
		return NErr_OutOfMemory;

	ret = autotag->Initialize(callback);
	if (ret != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","GracenoteAPI: Failed to initialize album callbacks '%d'", ret);
		autotag->Release();
		return ret;
	}
	*autotag_album = autotag;
	return NErr_Success;
}

int GracenoteAPI::LinkCreate(gnsdk_link_query_handle_t *link_handle)
{
	gnsdk_user_handle_t user_handle;
	int ret = GetUser(&user_handle);
	if (ret != NErr_Success)
		return ret;

	NXOnce(&gracenote_link_once, LinkOnce, (GracenoteAPI *)this);
	gnsdk_link_query_handle_t gracenote_link_handle;
	gnsdk_error_t gn_error = gnsdk_link_query_create(user_handle, 0, 0, &gracenote_link_handle);
	if (gn_error == GNSDK_SUCCESS)
	{
		*link_handle = gracenote_link_handle;
		return NErr_Success;
	}
	return NErr_Error;	
}

int GracenoteAPI::Gracenote_SaveAlbumResults(ifc_gracenote_results *results, int flags)
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
			ret = Gracenote_SaveTrackResults(NULL, (ifc_gracenote_results *)track, flags);
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

int GracenoteAPI::Gracenote_SaveTrackResults(nx_uri_t filename, ifc_gracenote_results *results, int flags)
{
	if (!results)
		return NErr_BadParameter;

	MetadataGDO *metadata = (MetadataGDO *)results;

	if (filename == NULL)
	{
		nx_string_t nx_filename;
		int ret = results->GetField(MetadataKeys::URI, 0, &nx_filename);
		if (ret != NErr_Success)
			return ret;

		ret = NXURICreateWithNXString(&filename, nx_filename);
		if (ret != NErr_Success)
		{
			NXStringRelease(nx_filename);
			return ret;
		}

	
		ret = metadata->SaveTo(filename, flags);
		NXURIRelease(filename);
		return ret;
	}
	else
	{
		return metadata->SaveTo(filename, flags);
	}
}

nx_thread_return_t GracenoteAPI::InitializeThreadProcedure(nx_thread_parameter_t p)
{
	GracenoteAPI *gn = (GracenoteAPI *)p;
	NXOnce(&gn->gracenote_manager_once, ManagerOnce, (void *)p);
	NXOnce(&gn->gracenote_user_once, UserOnce, (void *)p);
	// TODO: check error codes before issuing this
	WASABI2_API_SYSCB->IssueCallback(Gracenote::event_type, Gracenote::on_status, Gracenote::SystemCallback::STATUS_GRACENOTE_LOADED);
	return 0;
}

int GracenoteAPI::Gracenote_Initialize()
{
	nx_thread_t gracenote_thread;
	NXThreadCreate(&gracenote_thread, InitializeThreadProcedure, (void *)(GracenoteAPI *)this);
	return NErr_Success;
}

int GracenoteAPI::Gracenote_GetVersion(nx_string_t *version)
{
	return NXStringCreateWithUTF8(version, gnsdk_manager_get_version());
}

int GracenoteAPI::Gracenote_GetBuildDate(nx_string_t *version)
{
	return NXStringCreateWithUTF8(version, gnsdk_manager_get_build_date());
}
