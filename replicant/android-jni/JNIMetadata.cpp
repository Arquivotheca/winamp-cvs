#include "api.h"
#include "android-jni.h"
#include "JNIMetadata.h"
#include "JNIData.h"
#include "metadata/metadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "foundation/export.h"
#include "foundation/error.h"
#include "http/svc_http_demuxer.h"
#include <android/log.h>
#include <assert.h>
#include <unistd.h>

extern JavaVM *g_jvm;

// JNI Class reference cache globals
static jclass javaMetadataClass;
static jmethodID javaMetadataClassConstructor;
static jfieldID javaMetadataClassMetadataToken;

jobject JNIMetadataCreate(JNIEnv *env, ifc_metadata *metadata)
{
	if(!javaMetadataClass)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataCreate] Dint have a clazz = %x", javaMetadataClass);
		return 0;
	}

	if(!javaMetadataClassConstructor) 
		return 0;

	jobject obj = env->NewObject(javaMetadataClass, javaMetadataClassConstructor);
	if(!obj)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataCreate] metadata obj = %x", obj);		
		return 0;
	}	

	env->SetIntField(obj, javaMetadataClassMetadataToken, (jint)metadata);
	metadata->Retain();

	//__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataCreate] metadata Token = '%x'", (jint)metadata);

	return obj;
}

static void JNICALL JNINativeClassInit(JNIEnv *env, jclass cls)
{
	// Cache the metadata java class and its members
	javaMetadataClass = (jclass)env->NewGlobalRef(cls);
	javaMetadataClassConstructor = env->GetMethodID(javaMetadataClass, "<init>", "()V");
	javaMetadataClassMetadataToken = env->GetFieldID(javaMetadataClass, "metadataToken", "I");
}

static jint JNICALL JNINativeRegisterField(JNIEnv *env, jclass cls, jstring jkey)
{
	int ret;
	nx_string_t field;
	ret = NXStringCreateWithJString(env, jkey, &field);
	if (ret == NErr_Success)
	{
		int metadata_key;
		ret = REPLICANT_API_METADATA->RegisterField(field, &metadata_key);
		if (ret != NErr_Success)
			return MetadataKeys::UNKNOWN;
		NXStringRelease(field);
		return (jint)metadata_key;		
	}
	else
	{
		JNIThrowExceptionForNError(env, ret);
		return -1;
	}
}

static jstring JNICALL JNINativeGetFieldIndex(JNIEnv *env, jobject obj, jint metadata_token, jint jkey, jint jindex)
{
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	int key = (int)jkey;
	int index = (int)jindex;

	assert(metadata != 0);	

	if (metadata != 0 && key >= 0) 
	{
		ReferenceCountedNXString value;
		int ret = metadata->GetField(key, index, &value);
		if (ret == NErr_Success && value)
		{
			jstring jvalue;
			int ret = NXStringCreateJString(env, value, &jvalue);
			if (ret != NErr_Success)
				return 0;
			return jvalue;
		}
	}
	return 0;
}

static jstring JNICALL JNINativeGetField(JNIEnv *env, jobject obj, jint metadata_token, jint jkey)
{
	return JNINativeGetFieldIndex(env, obj, metadata_token, jkey, (jint)0);
}

static jint JNICALL JNINativeGetIntegerIndex(JNIEnv *env, jobject obj, jint metadata_token, jint jkey, jint jindex)
{
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	int key = (int)jkey;
	int index = (int)jindex;

	assert(metadata != 0);	

	if (metadata != 0 && key >= 0) 
	{
		int64_t value = 0;
		int ret = metadata->GetInteger(key, index, &value);
		if (ret == NErr_Success/* && value*/)		// Don't need to check for value since 0 is legitimate value
		{
			jint jvalue = (jint)value;
			return jvalue;
		}
	}
	return 0;
}

static jint JNICALL JNINativeGetInteger(JNIEnv *env, jobject obj, jint metadata_token, jint jkey)
{
	return JNINativeGetIntegerIndex(env, obj, metadata_token, jkey, (jint)0);
}


// Function returns sub metadata fields... eg: Track metadata within album metadata objects.
static jobject JNICALL JNINativeGetMetadata(JNIEnv *env, jobject obj, jint metadata_token, jint jkey, jint jindex)
{
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	
	int index = (int)jindex;
	int key = (int)jkey;

	// TODO: Actually implement the use of the passed in key here
	
	ifc_metadata *sub_metadata;
	//if (metadata->GetMetadata(MetadataKeys::TRACK, index, &sub_metadata) == NErr_Success)
	if (metadata->GetMetadata(jkey, index, &sub_metadata) == NErr_Success)
	{
		jobject jni_metadata = JNIMetadataCreate(env, sub_metadata);
		sub_metadata->Release();
		return jni_metadata;
	}
	else
		return 0;
}

static jobject JNICALL JNINativeGetAlbumArtData(JNIEnv *env, jobject obj, jint metadata_token, jint jimage_quality)
{
	int ret;			// Error return codes
	
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	
	int image_quality = (int)jimage_quality;

	artwork_t artwork;

	ret = metadata->GetArtwork(MetadataKeys::ALBUM, image_quality, &artwork);

	if (ret == NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNINativeGetAlbumArtData] metadata->GetArtData: SUCCESS");
		jobject jni_image_data = JNIDataCreate(env, artwork.data);
		return jni_image_data;
	}
	else if (ret == NErr_Unknown)
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNINativeGetAlbumArtData] metadata->GetArtData: ERROR UNKNOWN");
	else if (ret == NErr_Empty)
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNINativeGetAlbumArtData] metadata->GetArtData: ERROR EMPTY");
	else
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNINativeGetAlbumArtData] metadata->GetArtData: Other ERROR code '%d'", ret);


	return 0;
}

static jint JNICALL JNINativeRetain(JNIEnv *env, jobject obj, jint metadata_token)
{
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	
	if (metadata)
	{
		metadata->Retain();
		return metadata_token;
	}
	return NErr_NullPointer;
}

static jint JNICALL JNINativeRelease(JNIEnv *env, jobject obj, jint metadata_token)
{
	ifc_metadata *metadata = (ifc_metadata *)metadata_token;
	
	if (metadata)
	{
		assert(metadata);
		int ret = metadata->Release();
		if (ret == 0)
		{
			//__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadata] JNINativeRelease: Metadata object '%x' successfully released.", metadata);
		}
		else
			__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadata] JNINativeRelease: Metadata object '%x' released, but '%d' references still remain.", metadata, ret);
		
		return (jint)NErr_Success;
	}
	return 0;
}

static jobject JNICALL JNINativeOpen(JNIEnv *env, jclass cls, jstring jfilename)
{
	int ret;
	nx_uri_t filename;
	ret = NXURICreateWithJString(env, jfilename, &filename);
	if (ret == NErr_Success)
	{
		ReferenceCountedPointer<ifc_metadata> metadata;
		ret = REPLICANT_API_METADATA->CreateMetadata(&metadata, filename);
		if (ret == NErr_Success)
			return JNIMetadataCreate(env, metadata);
			
		return NULL;
	}
	else
	{
		JNIThrowExceptionForNError(env, ret);
		return 0;
	}
}

static jboolean JNICALL JNINativeIsSupportedFilename(JNIEnv * env, jobject obj, jstring jfilename)
{
	int supported = -1;
	nx_uri_t nx_filename;
	
	if (!REPLICANT_API_METADATA)
		return JNI_FALSE;
	
	int ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		supported = REPLICANT_API_METADATA->SupportedFilename(nx_filename);

		NXURIRelease(nx_filename);
		
		if (supported == NErr_True)
		{
			//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIMetadata] JNINativeIsSupportedMetadata, '%s' is 'TRUE'", nx_filename->string);
			return JNI_TRUE;
		}
		else if (supported == NErr_False)
		{
			//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIMetadata] JNINativeIsSupportedMetadata, '%s' is 'FALSE'", nx_filename->string);
			return JNI_FALSE;
		}
	}

	JNIThrowExceptionForNError(env, ret);

	return ret;
}


static jboolean JNICALL JNINativeIsSupportedMimetype(JNIEnv * env, jobject obj, jstring jmimetype)
{
	int supported = -1;
	nx_string_t nx_mimetype;
	
	if (!REPLICANT_API_METADATA)
		return JNI_FALSE;
	
	int ret = NXStringCreateWithJString(env, jmimetype, &nx_mimetype);
	if (ret == NErr_Success)
	{
		GUID http_demuxer_guid = svc_http_demuxer::GetServiceType(); 
		ifc_serviceFactory *sf; 
		size_t n = 0; 
		while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++)) 
		{ 
			svc_http_demuxer *l = (svc_http_demuxer*)sf->GetInterface(); 
			if (l) 
			{ 
				const char *this_accept; 
				size_t i=0; 
				while (this_accept=l->EnumerateAcceptedTypes(i++)) 
				{ 
					int comp = NXStringKeywordCompareWithCString(nx_mimetype, this_accept);

					if ( comp == 0)
						supported = NErr_True;
					else
						supported = (supported == NErr_True) ? NErr_True : NErr_False;

				} 
				l->Release(); 
			} 
		} 

		NXStringRelease(nx_mimetype);
		
		if (supported == NErr_True)
		{
			//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIMetadata] JNINativeIsSupportedMimetype, '%s' is 'TRUE'", nx_mimetype->string);
			return JNI_TRUE;
		}
		else if (supported == NErr_False)
		{
			//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIMetadata] JNINativeIsSupportedMimetype, '%s' is 'FALSE'", nx_mimetype->string);
			return JNI_FALSE;
		}
	}

	JNIThrowExceptionForNError(env, ret);

	return ret;
}

static jstring JNICALL JNINativeGetGenre(JNIEnv * env, jclass cls, jint jgenre_id)
{
	uint8_t genre_id = (uint8_t)jgenre_id;
	nx_string_t nx_genre = 0;
	jstring jgenre_string = 0;
	
	if (!REPLICANT_API_METADATA)
		return 0;
	
	ns_error_t ret = REPLICANT_API_METADATA->GetGenre(genre_id, &nx_genre);

	if (ret == NErr_Success && nx_genre)
	{
		NXStringRetain(nx_genre);
		ns_error_t ret = NXStringCreateJString(env, nx_genre, &jgenre_string);
		if (ret != NErr_Success)
			return 0;
		NXStringRelease(nx_genre);
	}
	return jgenre_string;
}

//////////////////////////////////////////////////////////////////////////
// JNIMetadata object methods
//////////////////////////////////////////////////////////////////////////
JNINativeMethod JNIMetadata::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeGetField", "(II)Ljava/lang/String;", (void *) JNINativeGetField },
	{ "nativeGetFieldIndex", "(III)Ljava/lang/String;", (void *) JNINativeGetFieldIndex },
	{ "nativeGetInteger", "(II)I", (void *) JNINativeGetInteger },
	{ "nativeGetIntegerIndex", "(III)I", (void *) JNINativeGetIntegerIndex },
	{ "nativeGetMetadata", "(III)Ljava/lang/Object;", (void *) JNINativeGetMetadata },
	{ "nativeGetAlbumArtData", "(II)Ljava/lang/Object;", (void *) JNINativeGetAlbumArtData },
	{ "nativeRegisterField", "(Ljava/lang/String;)I", (void *) JNINativeRegisterField },
	{ "nativeOpen", "(Ljava/lang/String;)Ljava/lang/Object;", (void *) JNINativeOpen },
	{ "nativeIsSupportedFilename", "(Ljava/lang/String;)Z", (void *)JNINativeIsSupportedFilename},
	{ "nativeIsSupportedMimetype", "(Ljava/lang/String;)Z", (void *)JNINativeIsSupportedMimetype},
	{ "nativeGetGenre", "(I)Ljava/lang/String;", (void *)JNINativeGetGenre},
	{ "nativeRetain", "(I)I", (void *) JNINativeRetain },
	{ "nativeRelease", "(I)I", (void *) JNINativeRelease },
};
size_t JNIMetadata::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIMetadata::jni_classname = "com/nullsoft/replicant/Metadata";