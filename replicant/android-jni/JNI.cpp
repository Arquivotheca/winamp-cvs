#include <jni.h>
#include "android-jni.h"
#include "JNIMediaPlayer.h"
#include "JNIMediaServer.h"
#include "JNIEqualizer.h"
#include "JNIPlaybackParameters.h"
#include "JNIReplicant.h"
#include "JNIData.h"
#include "JNIMetadata.h"
#include "JNIMetadataEditor.h"
#include "JNIPlaylistManager.h"
#include "JNIIPlaylistLoader.h"
#include "JNIAutoTagTrack.h"
#include "JNIAutoTagAlbum.h"
#include "JNIArtwork.h"
#include "JNICloudManager.h"
#include "foundation/export.h"
#include <android/log.h>

JavaVM *g_jvm = NULL; /* TODO: move this into some sort of api_replicant */
static pthread_key_t jni_thread_key;

static void JNIThreadDestructor(void *void_env)
{
	if (void_env)
	{
//		JNIEnv *env = (void *)void_env;
		//DetachCurrentThread(g_jvm);
		g_jvm->DetachCurrentThread();
	}
}

JNIEnv *JNIGetThreadEnvironment()
{
	void *void_env = pthread_getspecific(jni_thread_key);
	if (void_env)
		return (JNIEnv *)void_env;

	JNIEnv *thread_environment;
	g_jvm->AttachCurrentThread(&thread_environment, 0);
	pthread_setspecific(jni_thread_key, thread_environment);
	return thread_environment;
}

void JNIThrowException(JNIEnv * env, const char *exception_class)
{
	jclass newExcCls;
	env->ExceptionDescribe();
	env->ExceptionClear();
	newExcCls = env->FindClass(exception_class);

	if (newExcCls == NULL) 
	{
		/* Unable to find the exception class, give up. */
		return;
	}
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Exception] %s", exception_class);
	env->ThrowNew(newExcCls, "thrown from C code");
}

void JNIThrowExceptionForNError(JNIEnv * env, int ret)
{
	switch(ret)
	{
	case NErr_Success:
		break;
	case NErr_OutOfMemory:
		JNIThrowException(env, "java/lang/OutOfMemoryError");
		break;
	case NErr_NullPointer:
		JNIThrowException(env, "java/lang/NullPointerException");
		break;
	case NErr_Empty:
		JNIThrowException(env, "java/lang/IllegalArgumentException");
		break;
	case NErr_FileNotFound:
		JNIThrowException(env, "java/io/FileNotFoundException");
		break;
	default:
		JNIThrowException(env, "java/lang/Exception");
		break;
	}
}

template <class T>
static int RegisterMethods(JNIEnv *env)
{
	jclass clazz = env->FindClass(T::jni_classname);
	if(!clazz) 
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Init] unable to load class %s", T::jni_classname);
		return -1;
	}

	if(env->RegisterNatives(clazz, T::jni_methods, T::jni_methods_count) < 0) 
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Init] error registering functions for class %s", T::jni_classname);
		env->DeleteLocalRef(clazz);
		return -1;
	}

	env->DeleteLocalRef(clazz);
	return 0;
}

extern "C" DLLEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env=0;
	g_jvm = vm;

	if(g_jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) 
	{
		return -1;
	}

	pthread_key_create(&jni_thread_key, JNIThreadDestructor);
	RegisterMethods<JNIMediaPlayer>(env);
	RegisterMethods<JNIEqualizer>(env);
	RegisterMethods<JNIPlaybackParameters>(env);
	RegisterMethods<JNIData>(env);
	RegisterMethods<JNIMetadata>(env);
	RegisterMethods<JNIMetadataEditor>(env);
	RegisterMethods<JNIReplicant>(env);
	RegisterMethods<JNIIPlaylistLoader>(env);
	RegisterMethods<JNIPlaylistManager>(env);
	RegisterMethods<JNIAutoTagTrack>(env);
	RegisterMethods<JNIAutoTagAlbum>(env);
	RegisterMethods<JNIArtwork>(env);
	RegisterMethods<JNICloudManager>(env);
	RegisterMethods<JNIMediaServer>(env);

	return JNI_VERSION_1_6;
}

extern "C" DLLEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	g_jvm = NULL;
}
