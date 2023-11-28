#include "android-jni.h"
#include "JNIAutoTagTrack.h"
#include "JNIMetadata.h"
#include "gracenote/ifc_gracenote_results.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

#include <android/log.h>
#include "foundation/export.h"

// Globals to java
extern JavaVM *g_jvm;

// Java class
static jclass autotag_class = 0;

// Callback method definitions
// ToDo: Make these members of JNIAutoTagTrack so that different callbacks can be defined per instance
// This is already implemented for JNIAutoTagAlbum

/////////////////////////////////////////////////////////////////////////
// JNIAutoTagTrack - JNI support methods
///////////////////////////////////////////////////////////////////////////
static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	autotag_class = (jclass)env->NewGlobalRef(cls);
}

static jint JNICALL JNINativeCreateTrack(JNIEnv * env, jobject obj)
{
	JNIAutoTagTrack *jni_autotag = new JNIAutoTagTrack(env, obj);
	if (!jni_autotag)
		return 0;

	// Setting up the java callbacks
	jni_autotag->autoTagEventOnStatusChangedMethod = env->GetMethodID(autotag_class, "onStatusChange", "(I)V");
	jni_autotag->autoTagEventOnResultMethod = env->GetMethodID(autotag_class, "onResult", "(Lcom/nullsoft/replicant/Metadata;)V");

	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Created JNI autotag object jni_autotag '%x'.", jni_autotag);

	if (jni_autotag->InitializeGracenoteAPI() == NErr_Success)
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: gracenote_api is initialized.");
		REPLICANT_API_GRACENOTE->CreateAutoTag_Track(&jni_autotag->autotagger, jni_autotag, 0);
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Created new track query object %x", &jni_autotag->autotagger);
	}
	else
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: gracenote_api initialization failed.");
	}

	return (jint)jni_autotag;
}



static int JNICALL JNINativeRunQueryTrack(JNIEnv * env, jobject obj, jint token, jstring jfilename)
{
	JNIAutoTagTrack *jni_autotag = (JNIAutoTagTrack *)token;

	nx_uri_t nx_filename;

	int ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		ret = jni_autotag->RunQuery(nx_filename);
		NXURIRelease(nx_filename);
		return ret;
	}

	JNIThrowExceptionForNError(env, ret);

	return ret;
}

static int JNICALL JNINativeSaveTrack(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIAutoTagTrack *jni_autotag = (JNIAutoTagTrack *)token;

	return jni_autotag->Save();
}


static int JNICALL JNINativeRunQuerySimpleTrack(JNIEnv * env, jobject obj, jint token, jstring jartist, jstring jtitle)
{
	JNIAutoTagTrack *jni_autotag = (JNIAutoTagTrack *)token;

	ReferenceCountedNXString nx_artist, nx_title;

	int ret = NXStringCreateWithJString(env, jartist, &nx_artist);
	if (ret != NErr_Success)
	{
		JNIThrowExceptionForNError(env, ret);
		return ret;
	}

	ret = NXStringCreateWithJString(env, jtitle, &nx_title);
	if (ret != NErr_Success)
	{
		JNIThrowExceptionForNError(env, ret);
		return ret;
	}

	ret = jni_autotag->RunQuerySimple(nx_artist, nx_title);
	JNIThrowExceptionForNError(env, ret);
	return ret;
}


///////////////////////////////////////////////////////////////////////////
// JNIAutoTagTrack Member Methods
///////////////////////////////////////////////////////////////////////////

JNIAutoTagTrack::JNIAutoTagTrack(JNIEnv *env, jobject obj)
{
	java_autotag=env->NewGlobalRef(obj);

	autotagger = 0;
	gracenote_api = 0;
	thread_environment = 0;
	//java_autotag = 0;
	//java_autotag=env->NewGlobalRef(obj);

	autoTagEventOnStatusChangedMethod = 0;
	autoTagEventOnResultMethod = 0;

}

JNIEnv *JNIAutoTagTrack::AttachToThread()
{
	if (!thread_environment)
	{
		g_jvm->AttachCurrentThread(&thread_environment, 0);
	}
	return thread_environment;
}

NError JNIAutoTagTrack::RunQuery(nx_uri_t nx_filename)
{
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Begin running Gracenote query for '%s'", nx_filename->string);

	autotagger->Run(nx_filename);

	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Query complete for '%s'", nx_filename->string);

	return NErr_Success;
}

NError JNIAutoTagTrack::RunQuerySimple(nx_string_t artist, nx_string_t title)
{
	autotagger->Run_Simple(artist, title);
	return NErr_Success;
}

NError JNIAutoTagTrack::InitializeGracenoteAPI(void)
{
	if (gracenote_api == 0)
	{
		WASABI2_API_SVC->GetService(&gracenote_api);

		if (gracenote_api)
		{
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Successfully created gracenote_api '%x'.", gracenote_api);
			return NErr_Success;
		}
		else
		{
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Failed to create gracenote_api. '%x'", gracenote_api);
			return NErr_Error;
		}
	}
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Already have a gracenote_api. '%x'", gracenote_api);
	return NErr_Success;
}

NError JNIAutoTagTrack::Save()
{
	// ToDo: Need to actuall have this save something
	return NErr_NotImplemented;
}

int JNIAutoTagTrack::AutoTagCallback_OnStatus(nx_uri_t filename, int status)
{
	JNIEnv *env = AttachToThread();

	if (env)
	{

		if (java_autotag && autoTagEventOnStatusChangedMethod)
		{
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Calling autoTagEventOnStatusChangedMethod, with status: '%d'", status);
			jint jstatus = status;
			//env->CallVoidMethod(java_autotag, autoTagEventOnStatusChangedMethod, jstatus);
			env->CallVoidMethod(java_autotag, this->autoTagEventOnStatusChangedMethod, jstatus);
		}
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: java_autotag or autoTagEventOnStatusChangedMethod is null.");

	}
}

int JNIAutoTagTrack::AutoTagCallback_OnResults(ifc_gracenote_results *results)
{
	JNIEnv *env = AttachToThread();

	if (env)
	{
		jobject java_metadata_result = JNIMetadataCreate(env, results);

		nx_string_t value;
		if (results->GetField(MetadataKeys::ALBUM, 0, &value) == NErr_Success)
		{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "AutoTagTrackCallback_OnResults ALBUM: '%s'", value->string);
			NXStringRelease(value);
		}

		// Call java callback with the metadata object
		env->CallVoidMethod(java_autotag, this->autoTagEventOnResultMethod, java_metadata_result);

		if (java_metadata_result)
			env->DeleteLocalRef(java_metadata_result);
	}
}


// JNI Methods
JNINativeMethod JNIAutoTagTrack::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeCreateTrack", "()I", (void *) JNINativeCreateTrack },
	{ "nativeRunQueryTrack", "(ILjava/lang/String;)I", (void *) JNINativeRunQueryTrack },
	{ "nativeSaveTrack", "(II)I", (void *) JNINativeSaveTrack },
	//{ "nativeRunQuerySimpleTrack", "(ILjava/lang/String;Ljava/lang/String;)I", (void *) JNINativeRunQuerySimpleTrack },
};

size_t JNIAutoTagTrack::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIAutoTagTrack::jni_classname = "com/nullsoft/replicant/gracenote/AutoTagTrack";
