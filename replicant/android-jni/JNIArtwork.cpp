#include "api.h"
#include "android-jni.h"
#include <new>
#include "nx/nxdata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/Benchmark.h"

#include <android/log.h>
#include "foundation/export.h"

#include "JNIArtwork.h"
#include "JNIData.h"

// Globals to java
extern JavaVM *g_jvm;


static jclass artworkinfo_class = 0;
static jmethodID artworkinfo_setinfo_method = 0;

/////////////////////////////////////////////////////////////////////////
// JNIArtwork - JNI entry methods
///////////////////////////////////////////////////////////////////////////

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	artworkinfo_class = (jclass)env->FindClass("com/nullsoft/replicant/Artwork/ArtworkInfo");
	artworkinfo_setinfo_method = env->GetMethodID(artworkinfo_class, "setInfo", "(Ljava/lang/Object;IIJ)V");
}

static int JNICALL JNIGetArtData(JNIEnv * env, jobject obj, jstring jfilename, jint jfield, jobject jinfo)
{
	artwork_t artwork;

	if (!REPLICANT_API_ARTWORK)
		return NErr_NotImplemented;

	if (!artworkinfo_setinfo_method)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIArtwork] artworkinfo_setinfo_method is NULL.");
		return NErr_Error;
	}

	// The parameters for the lookup
	ReferenceCountedNXURI nx_filename;
	int field = (int)jfield;

	int ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret != NErr_Success)
		return ret;

	static uint64_t get_artwork_bm_start = 0, get_artwork_bm_end = 0, get_artwork_bm_diff = 0;
	nx_time_unix_64_t file_modified=0;
	
	// BENCHMARKING
	//get_artwork_bm_start = Benchmark();																					
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIArtwork] retreiving art for file: '%s', field: '%d'", nx_filename->string, field);
	ret = REPLICANT_API_ARTWORK->GetArtwork(nx_filename, field, &artwork, DATA_FLAG_ALL, &file_modified);						
	//get_artwork_bm_end = Benchmark();																						
	//get_artwork_bm_diff = (get_artwork_bm_end - get_artwork_bm_start) / 1000000;											
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Benchmark] GetArtwork() millis: %llu, for '%s', having art '%p'", get_artwork_bm_diff, nx_filename->string, &artwork);

	if (ret == NErr_Success)
	{
		// Build out all the parameters into a java object and pass them all into ArtworkInfo.setInfo
		jobject jimage_data = JNIDataCreate(env, artwork.data);

		jint jwidth = (jint)artwork.width;
		jint jheight = (jint)artwork.height;
		jlong jfile_modified = (jlong)file_modified;
		
		env->CallVoidMethod(jinfo, artworkinfo_setinfo_method, jimage_data, jwidth, jheight, jfile_modified);
		return NErr_Success;
	}
	else
	{
		jlong jfile_modified = (jlong)file_modified;
		
		env->CallVoidMethod(jinfo, artworkinfo_setinfo_method, (jobject)0, (jlong)0, (jlong)0, jfile_modified);
		return (jint)ret;
	}
	
}

static jint JNICALL JNICheckArtData(JNIEnv *env, jobject obj, jstring jfilename, jlong file_modified, jstring source_uri, jlong source_modified)
{
	int ret;

	nx_file_stat_s stat_buffer;
	if (file_modified)
	{
		ReferenceCountedNXURI nx_filename;

		ret = NXURICreateWithJString(env, jfilename, &nx_filename);
		if (ret != NErr_Success)
			return ret;

		ret = NXFile_stat(nx_filename, &stat_buffer);
		if (ret != NErr_Success)
			return ret;
	
		if (stat_buffer.modified_time != file_modified)
			return NErr_False;
	}

	if (source_modified)
	{
		ReferenceCountedNXURI nx_source;
	
		ret = NXURICreateWithJString(env, source_uri, &nx_source);
		if (ret != NErr_Success)
			return ret;

		ret = NXFile_stat(nx_source, &stat_buffer);
		if (ret != NErr_Success)
			return ret;
	
		if (stat_buffer.modified_time != source_modified)
			return NErr_False;
	}

	return NErr_True;
}

///////////////////////////////////////////////////////////////////////////
// JNIArtwork Member Methods
///////////////////////////////////////////////////////////////////////////
// JNI Methods
JNINativeMethod JNIArtwork::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	//{ "nativeGetArtData", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;IILjava/lang/String;I)Ljava/lang/Object;", (void *) JNIGetArtData },
	{ "nativeGetArtData", "(Ljava/lang/String;ILjava/lang/Object;)I", (void *) JNIGetArtData },
	{ "nativeCheckArtData", "(Ljava/lang/String;JLjava/lang/String;J)I", (void *) JNICheckArtData },
};

size_t JNIArtwork::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIArtwork::jni_classname = "com/nullsoft/replicant/Artwork/Artwork";
