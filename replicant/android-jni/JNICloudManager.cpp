#include "android-jni.h"
#include "JNICloudManager.h"
#include "decode/svc_raw_media_reader.h"	
#include "decode/ifc_raw_media_reader.h"
#include <android/log.h>
#include "foundation/export.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include "sha1.h"

extern JavaVM *g_jvm;
api_cloud *REPLICANT_API_CLOUD = 0;

struct JNICloudManagerCallback
{
	jfieldID listener_field;
	jmethodID listener_method;
	void Init(JNIEnv *env, jclass cloud_manager_class, const char *field_name, const char *class_name, const char *class_signature, const char *method_name, const char *method_signature)
	{
		listener_field = env->GetFieldID(cloud_manager_class, field_name, class_signature);
		if (!listener_field)
			__android_log_print(ANDROID_LOG_ERROR,"libreplicant","[CloudManager] Cannot find %s field!", field_name);

		jclass prepared_class = env->FindClass(class_name);
		if (prepared_class)
			listener_method = env->GetMethodID(prepared_class, method_name, method_signature);
		else 
			__android_log_print(ANDROID_LOG_ERROR,"libreplicant","[CloudManager] Cannot find %s class!", class_name);
	}
};

/* cached java methodIDs */

static JNICloudManagerCallback pull_listener;
static jclass cloud_manager_class;
//static jmethodID cloudEventPullMethod;
static jmethodID
	cloudEventOnRevisionMethod,
	cloudEventOnUnauthorizedMethod,
	cloudEventOnFirstPullMethod;


JNICloudManager::JNICloudManager(JNIEnv *env, jobject obj)
{
	java_cloud_manager=env->NewGlobalRef(obj);
}


/*void JNICloudManager::CloudEvents_Pull(int result)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jint jresult = result;
		env->CallVoidMethod(java_cloud_manager, cloudEventPullMethod, jresult);
	}
} */

int JNICloudManager::InitializeDB(nx_string_t device_token)
{
	REPLICANT_API_CLOUD->CreateDatabaseConnection(&db_connection, device_token);

	attributes.device_token = NXStringRetain(device_token);

	db_connection->BeginTransaction();
	db_connection->Devices_Find(attributes.device_token, &attributes.device_id, 0);
	db_connection->Attribute_Add("artist", &attributes.artist);
	db_connection->Attribute_Add("album", &attributes.album);
	db_connection->Attribute_Add("trackno", &attributes.trackno);
	db_connection->Attribute_Add("albumartist", &attributes.albumartist);
	db_connection->Attribute_Add("bpm", &attributes.bpm);
	db_connection->Attribute_Add("category", &attributes.category);
	db_connection->Attribute_Add("comment", &attributes.comment);
	db_connection->Attribute_Add("composer", &attributes.composer);
	db_connection->Attribute_Add("director", &attributes.director);
	db_connection->Attribute_Add("disc", &attributes.disc);
	db_connection->Attribute_Add("discs", &attributes.discs);
	db_connection->Attribute_Add("genre", &attributes.genre);
	db_connection->Attribute_Add("producer", &attributes.producer);
	db_connection->Attribute_Add("publisher", &attributes.publisher);
	db_connection->Attribute_Add("tracks", &attributes.tracks);
	db_connection->Attribute_Add("year", &attributes.year);	
	db_connection->Attribute_Add("albumgain", &attributes.albumgain);
	db_connection->Attribute_Add("trackgain", &attributes.trackgain);
	db_connection->Attribute_Add("rating", &attributes.rating);
	db_connection->Attribute_Add("type", &attributes.type);
	db_connection->Attribute_Add("lossless", &attributes.lossless);

	ReferenceCountedNXString mime_type;
	if (NXStringCreateWithUTF8(&mime_type, "audio/mpeg") == NErr_Success)
		db_connection->MIME_SetPlayable(mime_type, 1, 1);
	if (NXStringCreateWithUTF8(&mime_type, "audio/flac") == NErr_Success)
		db_connection->MIME_SetPlayable(mime_type, 1, 1);
	if (NXStringCreateWithUTF8(&mime_type, "audio/mp4") == NErr_Success)
		db_connection->MIME_SetPlayable(mime_type, 1, 1);

	if (NXStringCreateWithUTF8(&mime_type, "audio/ogg") == NErr_Success)
		db_connection->MIME_SetPlayable(mime_type, 1, 0);
	if (NXStringCreateWithUTF8(&mime_type, "audio/wav") == NErr_Success)
		db_connection->MIME_SetPlayable(mime_type, 1, 0);
	db_connection->Commit();
}

static ns_error_t ComputeMediaHash(nx_uri_t filename, nx_string_t *mediahash)
{
	uint8_t sha1_hash[20];
	ifc_serviceFactory *sf;
	size_t i=0;
	while (sf = WASABI2_API_SVC->EnumService(svc_raw_media_reader::GetServiceType(), i++))
	{
		svc_raw_media_reader *reader_service = (svc_raw_media_reader *)sf->GetInterface();
		if (reader_service)
		{
			ifc_raw_media_reader *reader=0;
			int ret = reader_service->CreateRawMediaReader(&reader, filename, 1); // TODO use pass appropriate.  possibly add helper method api_decode::CreateRawMediaReader that handles it
			reader_service->Release();
			if (ret == NErr_Success)
			{
				SHA1_CTX sha1;
				uint8_t buffer[65536];
				size_t bytes_read;
				SHA1Init(&sha1);
				while (reader->Read(buffer, 65536, &bytes_read) == NErr_Success)
				{
					SHA1Update(&sha1, buffer, bytes_read);
				}

				reader->Release();
				SHA1Final(sha1_hash, &sha1);
				char temp[41];
				sprintf(temp, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
					sha1_hash[0], sha1_hash[1], sha1_hash[2], sha1_hash[3], 
					sha1_hash[4], sha1_hash[5], sha1_hash[6], sha1_hash[7], 
					sha1_hash[8], sha1_hash[9], sha1_hash[10], sha1_hash[11], 
					sha1_hash[12], sha1_hash[13], sha1_hash[14], sha1_hash[15],
					sha1_hash[16], sha1_hash[17], sha1_hash[18], sha1_hash[19]);

				return NXStringCreateWithUTF8(mediahash, temp);
			}
		}
	}
	return NErr_Error;
}

class MediaHashMetadata : public ifc_metadata
{
public:
	MediaHashMetadata(nx_string_t mediahash);
	~MediaHashMetadata();
	static int MetadataKey_CloudMediaHash;
private:
	/* ifc_metadata implementation */
	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }

private:
	nx_string_t mediahash;
};

int MediaHashMetadata::MetadataKey_CloudMediaHash=-1;
MediaHashMetadata::MediaHashMetadata(nx_string_t mediahash)
{
	this->mediahash = NXStringRetain(mediahash);
}

MediaHashMetadata::~MediaHashMetadata()
{
	NXStringRelease(mediahash);
}

ns_error_t MediaHashMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (field == MetadataKey_CloudMediaHash)
	{
		if (mediahash)
		{
			*value = NXStringRetain(mediahash);
			return NErr_Success;
		}
		else
		{
			return NErr_Empty;
		}
	}
	return NErr_Unknown;
}

void JNICloudManager::CalculateMediaHashes()
{
	if (MediaHashMetadata::MetadataKey_CloudMediaHash == -1)
	{
		ReferenceCountedNXString metadata;
		if (NXStringCreateWithUTF8(&metadata, "cloud/mediahash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MediaHashMetadata::MetadataKey_CloudMediaHash);
	}

	int *ids=0;
	size_t num_ids=0;
	if (db_connection->IDMap_Get_MediaHash_Null(&ids, &num_ids) == NErr_Success)
	{
		for (size_t i=0;i<num_ids;i++)
		{
			int internal_id = ids[i];
			ReferenceCountedNXURI filepath;
			if (db_connection->IDMap_Get_Filepath(internal_id, &filepath) == NErr_Success)
			{
				ReferenceCountedNXString media_hash, meta_hash, id_hash;
				if (ComputeMediaHash(filepath, &media_hash) == NErr_Success)
				{
					MediaHashMetadata metadata(media_hash);
					db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);					

					// do an announce every 32 tracks.  we can tune this
					if ((i & 31) == 0)
					{
						//if (cloud_client)
						//	cloud_client->Flush();
					}
				}
				else
				{
					// flag file to be ignored from further re-processing attempts
					db_connection->IDMap_SetIgnore(internal_id);
				}
			}
		}
		free(ids);
	}
}

/* ========================================= Callback Events ================================================== */
void JNICloudManager::CloudEvents_OnRevision(ifc_cloudclient *client, int64_t revision, int from_reset)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	__android_log_print(ANDROID_LOG_ERROR,"libreplicant","[CloudManager] CloudEvents_OnRevision");
	
	if (env)
	{
		jint jClientToken = (jint)( client );
		jlong jRevision = (jlong)( revision );
		jint jFrom_reset = (jint)( from_reset );
		env->CallVoidMethod(java_cloud_manager, cloudEventOnRevisionMethod, jClientToken, jRevision, jFrom_reset);
	}
}

void JNICloudManager::CloudEvents_OnUnauthorized(ifc_cloudclient *client)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	__android_log_print(ANDROID_LOG_ERROR,"libreplicant","[CloudManager] CloudEvents_OnUnauthorized");
	
	if (env)
	{
		jint jClientToken = (jint)( client );
		env->CallVoidMethod(java_cloud_manager, cloudEventOnUnauthorizedMethod, jClientToken);
	}
}

void JNICloudManager::CloudEvents_OnAction(ifc_cloudclient *client, nx_string_t action, nx_string_t message)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant","[CloudManager] CloudEvents_OnAction (%s) - '%s'", action->string, message->string );
	__android_log_print(ANDROID_LOG_DEBUG, "libreplicant","[CloudManager] CloudEvents_OnAction (%s) - '%s'", AutoCharPrintfUTF8(action), AutoCharPrintfUTF8(message) );
	
}

void JNICloudManager::CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	__android_log_print(ANDROID_LOG_DEBUG, "libreplicant","[CloudManager] CloudEvents_OnFirstPull forced=(%s)", (forced) ? "true" : "false" );
	
	if (env)
	{
		jint jClientToken = (jint)( client );
		jboolean jForced = (jboolean)( forced );
		env->CallVoidMethod(java_cloud_manager, cloudEventOnFirstPullMethod, jClientToken, jForced);
	}
}

void JNICloudManager::CloudEvents_OnPlaylistUpload(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid, int entry, PlaylistEntry* item)
{
	ReferenceCountedNXString metahash;
	if (db_connection->PlaylistMap_GetMetahash(uuid, entry, &metahash) == NErr_Success)
	{
		item->metahash = NXStringRetain(metahash);
	}
}

/* ====================================== JNI statics ===================================================== */

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	cloud_manager_class = (jclass)env->NewGlobalRef(cls);

	//cloudEventPullMethod = env->GetMethodID(cloud_manager_class, "cloudEventPull", "(I)V");
	cloudEventOnRevisionMethod = env->GetMethodID(cloud_manager_class, "cloudEventOnRevision", "(IJI)V");
	cloudEventOnUnauthorizedMethod = env->GetMethodID(cloud_manager_class, "cloudEventOnUnauthorized", "(I)V");
	cloudEventOnFirstPullMethod = env->GetMethodID(cloud_manager_class, "cloudEventOnFirstPull", "(IZ)V");
}

static jint JNICALL JNINativeCreate(JNIEnv * env, jobject obj)
{
	int ret = -1;
	JNICloudManager *jni_cloud_manager = new JNICloudManager(env, obj);
	if (!jni_cloud_manager)
		return 0;
	//jni_cloud_manager->cloud_api.RegisterForEvents(jni_cloud_manager);
	//int ret = jni_cloud_manager->cloud_api.Init();
	/*if (ret != NErr_Success)
	{
	delete jni_cloud_manager;
	// TODO: throw exception
	return 0;
	}	  */

	return (jint)jni_cloud_manager;
}

static bool GetCloudAPI(int dev_mode)
{
	if (!REPLICANT_API_CLOUD)
	{
		WASABI2_API_SVC->GetService(&REPLICANT_API_CLOUD);
		if (REPLICANT_API_CLOUD)
			REPLICANT_API_CLOUD->SetDevMode(dev_mode);
	}

	if (REPLICANT_API_CLOUD)
		return true;

	return false;
}

static bool GetCloudAPI()
{
	// Defaulting to USE_DEVELOPMENT_API, but by this time we should have been initialized to something!
	GetCloudAPI(USE_DEVELOPMENT_API);
}

static jint JNICALL JNINativeSetCloudAPI(JNIEnv * env, jobject obj, jint jdev_mode )
{
	int dev_mode = (int) jdev_mode;

	return GetCloudAPI(dev_mode);
}

static jint JNICALL JNINativeInitializeDB(JNIEnv * env, jobject obj, jint token, jstring jdevice_token )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)token;

	ReferenceCountedNXString nx_device_token;
	ret = NXStringCreateWithJString(env, jdevice_token, &nx_device_token);		// Get the device name
	if (ret != NErr_Success) 
		return NErr_Error;

	if (jni_cloud_manager && GetCloudAPI())
	{
		ret = jni_cloud_manager->InitializeDB(nx_device_token);
	}

	__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] Initializing native cloud DB completed...");
	return NErr_Success;
}

static jint JNICALL JNINativeInitializeCloud(JNIEnv * env, jobject obj, jint token, jstring jdevice_token, jstring juser_name, jstring jauth_token, jstring jprovider)
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)token;

	if ( jni_cloud_manager )
	{
		if (GetCloudAPI())
		{
			__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] Initializing native CloudClient ...");

			nx_string_t nx_device_token;
			nx_string_t nx_user_name;
			nx_string_t nx_auth_token;
			nx_string_t nx_provider;

			int ret = NXStringCreateWithJString(env, jdevice_token, &nx_device_token);		// Get the device name
			if (ret != NErr_Success) return NErr_Error;

			ret = NXStringCreateWithJString(env, juser_name, &nx_user_name);				// Get the user name
			if (ret != NErr_Success) return NErr_Error;

			ret = NXStringCreateWithJString(env, jauth_token, &nx_auth_token);				// Get the auth token
			if (ret != NErr_Success) return NErr_Error;

			ret = NXStringCreateWithJString(env, jprovider, &nx_provider);					// Get the provider
			if (ret != NErr_Success) return NErr_Error;

			REPLICANT_API_CLOUD->SetCredentials(nx_user_name, nx_auth_token, nx_provider);

			if (REPLICANT_API_CLOUD->CreateCloudClient(nx_device_token, &jni_cloud_manager->cloud_client) == NErr_Success)
			{
				// Register cloud_manager for callbacks from the cloud client
				ret = jni_cloud_manager->cloud_client->RegisterCallback(jni_cloud_manager);
				
				if (ret == NErr_Success)
					__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] SUCCESS registered for cloud callbacks\n");
				else
					__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] FAILED to register for cloud callbacks\n");

				//jni_cloud_manager->cloud_client->DevicesList();
				//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "CLOUD TEST: successfully pulled devices list\n");
				//jni_cloud_manager->cloud_client->Pull();
				//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "CLOUD TEST: successfully pulled song list data\n");

				//NXSleep(100000);
				//cloud_client->Release();

			}

			//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] ... CloudClient Completed initialization.");


			NXStringRelease(nx_user_name);
			NXStringRelease(nx_auth_token);
			NXStringRelease(nx_provider);
			NXStringRelease(nx_device_token);

		}
		return NErr_Success;
	}
	else
		return NErr_Error;
	return 0;
}

static jint JNICALL JNINativeBeginTransaction(JNIEnv * env, jobject obj, jint jcloud_token )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		jni_cloud_manager->db_connection->BeginTransaction();
		//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] Native BEGIN TRANSACTION...");
		return NErr_Success;
	}
	return NErr_Error;
}

static jint JNICALL JNINativeCommit(JNIEnv * env, jobject obj, jint jcloud_token )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		jni_cloud_manager->db_connection->Commit();
		//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] ...Native COMMIT TRANSACTION");
		return NErr_Success;
	}
	return NErr_Error;
}


static jint JNICALL JNINativeAddMedia(JNIEnv * env, jobject obj, jint jcloud_token, jstring juri, jint jmetadata_token)
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		int internal_id;
		//JNIMetadata *jni_metadata = (JNIMetadata *)jmetadata_token;
		int metadata_token = (int)jmetadata_token;
		ifc_metadata *metadata = (ifc_metadata *)metadata_token;

		nx_uri_t nx_uri;
		ret = NXURICreateWithJString(env, juri, &nx_uri);		// get the filename uri

		if (ret != NErr_Success) return NErr_Error;

		// TODO: detect for add vs. an update (likely for optimization reasons that check should be done in JAVA and a seperate method provided)
		jni_cloud_manager->db_connection->BeginTransaction();
		jni_cloud_manager->db_connection->Media_Add(nx_uri, metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL, &internal_id);
		jni_cloud_manager->db_connection->Commit();
		// Not returning media_id because currently the media scanner does not care
		// if it is ever made aware then this can be return paramater
		
		
		// TODO benski> this is not at all where I want to do this.  putting this here to test it
		jni_cloud_manager->CalculateMediaHashes();

		NXURIRelease(nx_uri);

		return NErr_Success;
	}
	return NErr_Error;
}

static jint JNICALL JNINativeUpdateMedia(JNIEnv * env, jobject obj, jint jcloud_token, jint jmedia_id, jint jmetadata_token)
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		int media_id = (int)jmedia_id;
		int metadata_token = (int)jmetadata_token;
		ifc_metadata *metadata = (ifc_metadata *)metadata_token;

		jni_cloud_manager->db_connection->BeginTransaction();
		jni_cloud_manager->db_connection->Media_Update(media_id, metadata, ifc_clouddb::DIRTY_FULL|ifc_clouddb::DIRTY_LOCAL);
		jni_cloud_manager->db_connection->Commit();

		return NErr_Success;
	}
	return NErr_Error;
}

static jint JNICALL JNINativeDeleteMedia(JNIEnv * env, jobject obj, jint jcloud_token, jint jmedia_id )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		int media_id = (int)jmedia_id;

		// Surround with external begin transaction and commit to ensure that batch deletes are done efficiently
		jni_cloud_manager->db_connection->IDMap_Delete(media_id);

		return NErr_Success;
	}
	return NErr_Error;
}


#pragma region ImportMetadata
class ImportMetadata : public ifc_metadata
{
public:
	ImportMetadata();
	~ImportMetadata();

	int Initialize(JNIEnv * env, nx_uri_t uri, jstring jtitle, jstring jartist, jstring jalbum, jstring jgenre, jstring jcomposer, jstring jmime_type, jint jtrack, jint jyear, jlong jduration, jlong jsize, jlong jfiletime, jlong date_added, jlong playcount, jlong lastplayed);

	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data);
private:
	nx_uri_t uri;
	nx_string_t title;
	nx_string_t artist;
	nx_string_t album;
	nx_string_t genre;
	nx_string_t composer;
	int64_t track;
	nx_string_t year;
	double duration;
	int64_t filesize;
	int64_t filetime;
	int64_t date_added;		int date_added_field;
	int64_t playcount;		int playcount_field;
	int64_t lastplayed;		int lastplayed_field;
	nx_string_t mimetype;
};

ImportMetadata::ImportMetadata()
{
	uri=0;
	title=0;
	artist=0;
	album=0;
	genre=0;
	composer=0;
	track=0;
	year=0;
	duration=0;
	filesize=0;
	filetime=0;
	date_added=0;
	playcount=0;
	lastplayed=0;
	mimetype=0;

	ReferenceCountedNXString metadata;
	if (NXStringCreateWithUTF8(&metadata, "added") == NErr_Success)
		REPLICANT_API_METADATA->RegisterField(metadata, &date_added_field);
	
	if (NXStringCreateWithUTF8(&metadata, "playcount") == NErr_Success)
		REPLICANT_API_METADATA->RegisterField(metadata, &playcount_field);
	
	if (NXStringCreateWithUTF8(&metadata, "lastplayed") == NErr_Success)
		REPLICANT_API_METADATA->RegisterField(metadata, &lastplayed_field);
}

ImportMetadata::~ImportMetadata()
{
	NXURIRelease(uri);
	NXStringRelease(title);
	NXStringRelease(artist);
	NXStringRelease(album);
	NXStringRelease(genre);
	NXStringRelease(composer);
	NXStringRelease(year);	
	NXStringRelease(mimetype);
}

ns_error_t ImportMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
	case MetadataKeys::URI:
		if (!uri)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		return NXURIGetNXString(value, uri);
	case MetadataKeys::TITLE:
		if (!title)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(title);
		return NErr_Success;
	case MetadataKeys::ARTIST:
		if (!artist)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(artist);
		return NErr_Success;
	case MetadataKeys::ALBUM:
		if (!album)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(album);
		return NErr_Success;
	case MetadataKeys::GENRE:
		if (!genre)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(genre);
		return NErr_Success;
	case MetadataKeys::COMPOSER:
		if (!composer)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(composer);
		return NErr_Success;
	case MetadataKeys::YEAR:
		if (!year)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(year);
		return NErr_Success;
	case MetadataKeys::TRACK:
		if (!track)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		return NXStringCreateWithInt64(value, track);
	case MetadataKeys::MIME_TYPE:
		if (!mimetype)
			return NErr_Empty;
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = NXStringRetain(mimetype);
		return NErr_Success;
	}
	return NErr_Unknown;
}

ns_error_t ImportMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	switch(field)
	{
	case MetadataKeys::TRACK:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = (int64_t)track;
		return NErr_Success;
	case MetadataKeys::LENGTH:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = (int64_t)duration;
		return NErr_Success;
	case MetadataKeys::FILE_SIZE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = filesize;
		return NErr_Success;
	case MetadataKeys::FILE_TIME:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = filetime;
		return NErr_Success;
	default:
		if ( field == date_added_field)
		{
			if (index > 0)
				return NErr_EndOfEnumeration;
			*value = date_added;
			return NErr_Success;
		}
		if ( field == playcount_field)
		{
			if (index > 0)
				return NErr_EndOfEnumeration;
			*value = playcount;
			return NErr_Success;
		}
		if ( field == lastplayed_field)
		{
			if (index > 0)
				return NErr_EndOfEnumeration;
			*value = lastplayed;
			return NErr_Success;
		}
	}
	return NErr_Unknown;
}

ns_error_t ImportMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	switch(field)
	{
	case MetadataKeys::LENGTH:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = duration;
		return NErr_Success;
	}
	return NErr_Unknown;
}

ns_error_t ImportMetadata::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	return NErr_Unknown;
}

ns_error_t ImportMetadata::Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)
{
	return NErr_Unknown;
}
#pragma endregion

int ImportMetadata::Initialize(JNIEnv * env, nx_uri_t _uri, jstring jtitle, jstring jartist, jstring jalbum, jstring jgenre, jstring jcomposer, jstring jmime_type, jint jtrack, jint jyear, jlong jduration, jlong jsize, jlong jfiletime, jlong jdate_added, jlong jplaycount, jlong jlastplayed)
{
	int ret;

	// get the filename uri
	uri = NXURIRetain(_uri);		

	// get the title
	if (jtitle != 0)
	{
		ret = NXStringCreateWithJString(env, jtitle, &title);		
		if (ret != NErr_Success && ret != NErr_Empty)
		{
			//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) on 'title' FAILED.");
			return ret;
		}
	}

	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'title' SUCCESS.");

	// get the artist
	if (jartist != 0)
	{
		ret = NXStringCreateWithJString(env, jartist, &artist);		
		if (ret != NErr_Success && ret != NErr_Empty) return ret;		
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'artist' SUCCESS.");

	// get the album
	if (jalbum != 0)
	{
		ret = NXStringCreateWithJString(env, jalbum, &album);		
		if (ret != NErr_Success && ret != NErr_Empty) return ret;
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'album' SUCCESS.");

	// get the genre
	if (jgenre != 0)
	{
		ret = NXStringCreateWithJString(env, jgenre, &genre);		
		if (ret != NErr_Success && ret != NErr_Empty) return ret;		
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'genre' SUCCESS.");

	// get the composer
	if (jcomposer != 0)
	{
		ret = NXStringCreateWithJString(env, jcomposer, &composer);		
		//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) 'composer' = '%x'.", jcomposer);
		if (ret != NErr_Success && ret != NErr_Empty) return ret;
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'composer' SUCCESS.");

	// get the mime_type
	if (jmime_type != 0)
	{
		ret = NXStringCreateWithJString(env, jmime_type, &mimetype);		
		if (ret != NErr_Success && ret != NErr_Empty) return ret;		
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'mime_type' SUCCESS.");

	track = (int)jtrack;

	if (jyear > 0)
	{
		ret = NXStringCreateWithInt64(&year, jyear);
		if (ret != NErr_Success) return ret;		
	}
	//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) made it past 'year' SUCCESS.");

	duration = (double)jduration / 1000.0;
	filesize = jsize;
	filetime = jfiletime;
	date_added = jdate_added;
	playcount = jplaycount;
	lastplayed = jlastplayed;
	return NErr_Success;
}

static jint JNICALL JNINativeSeedMedia(JNIEnv * env, jobject obj, jint jcloud_token, jstring juri, jstring jtitle, jstring jartist, jstring jalbum, jstring jgenre, jstring jcomposer, jstring jmime_type, jint jtrack, jint jyear, jlong jduration, jlong jsize, jlong jfiletime, jlong jdate_added, jlong jplaycount, jlong jlastplayed )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)jcloud_token;

	if ( jni_cloud_manager && GetCloudAPI() )
	{
		ImportMetadata *metadata = new (std::nothrow) ReferenceCounted<ImportMetadata>;
		if (!metadata)
			return NErr_OutOfMemory;

		ReferenceCountedNXURI uri;

		int ret = NXURICreateWithJString(env, juri, &uri);		
		if (ret != NErr_Success) return ret;

		ret = metadata->Initialize(env, uri, jtitle, jartist, jalbum, jgenre, jcomposer, jmime_type, jtrack, jyear, jduration, jsize, jfiletime, jdate_added, jplaycount, jlastplayed);
		if (ret != NErr_Success)
		{
			__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata->Initialize(...) FAILED.");
			return ret;
		}


		//jni_cloud_manager->db_connection->BeginTransaction();
		int internal_id;
		jni_cloud_manager->db_connection->Media_Add(uri, metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL, &internal_id);
		//int64_t metadata_filetime;
		//metadata->Metadata_GetInteger(MetadataKeys::FILE_TIME, 0, &metadata_filetime);
		//__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] metadata filetime: '%lld' calling 'jni_cloud_manager->db_connection->Media_Add(uri, metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL, &internal_id);'", metadata_filetime);
		
		//jni_cloud_manager->db_connection->Commit();

		// Not returning media_id because currently the media scanner does not care
		// if it is ever made aware then this can be return paramater

		metadata->Release();


		return NErr_Success;
	}
	return NErr_Error;
}

static jint JNICALL JNINativeRelease(JNIEnv * env, jobject obj, jint token )
{
	int ret;
	JNICloudManager *jni_cloud_manager = (JNICloudManager *)token;

	if ( jni_cloud_manager )
	{
		if (jni_cloud_manager->cloud_client)
		{
			jni_cloud_manager->cloud_client->Release();
			__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] CloudClient '%x' Released ...", jni_cloud_manager->cloud_client);
		}
		return NErr_Success;
	}
	else
		return NErr_Error;
}


/* DEPRECATED */
/*static jint JNICALL JNINativeRunTests(JNIEnv * env, jobject obj, jint token)
{
int ret;
JNICloudManager *jni_cloud_manager = (JNICloudManager *)token;

if ( jni_cloud_manager )
{
if (!REPLICANT_API_CLOUD)
WASABI2_API_SVC->GetService(&REPLICANT_API_CLOUD);

if (REPLICANT_API_CLOUD)
{
__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] Running unit tests ...");
nx_string_t username, authtoken, provider;
//NXStringCreateWithUTF8(&username, "audiodsp");
//NXStringCreateWithUTF8(&authtoken, "A4218e52d0f12f9040");
NXStringCreateWithUTF8(&username, "gergo.spolarics");
NXStringCreateWithUTF8(&authtoken, "A0718ec2f0f024710c7");
NXStringCreateWithUTF8(&provider, "facebook");
REPLICANT_API_CLOUD->SetCredentials(username, authtoken, provider);
nx_string_t device_token;
NXStringCreateWithUTF8(&device_token, "android-test");
ifc_cloudclient *cloud_client;
if (REPLICANT_API_CLOUD->CreateCloudClient(device_token, &cloud_client) == NErr_Success)
{
__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] Successfully created Cloud Client\n");
cloud_client->DevicesList();
__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] CLOUD TEST: successfully pulled devices list\n");
cloud_client->Pull();
__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager] CLOUD TEST: successfully pulled song list data\n");
//NXSleep(100000);

//cloud_client->Release();
}
//jni_cloud_manager->cloud_api.RunTests();
__android_log_print(ANDROID_LOG_DEBUG, "libreplicant", "[CloudManager]... Completed unit tests.");
}
return NErr_Success;
}
else
return NErr_Error;
return 0;
}	 */

static void JNICALL JNIGetStreamURL(JNIEnv * env, jobject obj, jint token)
{


}

JNINativeMethod JNICloudManager::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeCreate", "()I", (void *) JNINativeCreate },
	{ "nativeSetCloudAPI", "(I)I", (void *) JNINativeSetCloudAPI },
	{ "nativeInitializeDB", "(ILjava/lang/String;)I", (void *) JNINativeInitializeDB },
	{ "nativeInitializeCloud", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void *) JNINativeInitializeCloud },
	{ "nativeBeginTransaction", "(I)I", (void *) JNINativeBeginTransaction },
	{ "nativeCommit", "(I)I", (void *) JNINativeCommit },
	{ "nativeAddMedia", "(ILjava/lang/String;I)I", (void *) JNINativeAddMedia },
	{ "nativeUpdateMedia", "(III)I", (void *) JNINativeUpdateMedia },
	{ "nativeSeedMedia", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIJJJJJJ)I", (void *) JNINativeSeedMedia },
	{ "nativeDeleteMedia", "(II)I", (void *) JNINativeDeleteMedia },
	{ "nativeRelease", "(I)I", (void *) JNINativeRelease },
	//{ "nativeRunTests", "(I)I", (void *) JNINativeRunTests },
};

size_t JNICloudManager::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNICloudManager::jni_classname = "com/nullsoft/replicant/cloud/CloudManager";
