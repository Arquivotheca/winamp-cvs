#include "JNIPlaylistManager.h"
#include <android/log.h>
#include "foundation/export.h"
#include "nx/nxstring.h"
#include "foundation/error.h"

extern JavaVM *g_jvm;

JNIPlaylistManager::JNIPlaylistManager(JNIEnv *env, jobject obj)
{
}

static void JNICALL Java_com_nullsoft_replicant_PlaylistManager_nativeClassInit(JNIEnv * env, jclass cls)
{
}


static jint JNICALL Java_com_nullsoft_replicant_PlaylistManager_nativeLoad(JNIEnv * env, jclass cls, jstring jfilename, jobject loader)
{
	int ret;
	nx_string_t nx_filename;
	ret = NXStringCreateWithJString(env, jfilename, &nx_filename);
	if (ret != NErr_Success)
	{
	}
	return (jint)NErr_Success;
}


JNINativeMethod JNIPlaylistManager::jni_methods[] =
{
	{ "nativeClassInit", "()V", (void *) Java_com_nullsoft_replicant_PlaylistManager_nativeClassInit },
	{ "nativeLoad", "(Ljava/lang/String;Lcom/nullsoft/replicant/IPlaylistLoader;)I", (void *) Java_com_nullsoft_replicant_PlaylistManager_nativeLoad },
};

size_t JNIPlaylistManager::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIPlaylistManager::jni_classname = "com/nullsoft/replicant/PlaylistManager";
