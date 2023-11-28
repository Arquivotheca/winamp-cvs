#include "JNIIPlaylistLoader.h"
#include <android/log.h>
#include "foundation/export.h"

extern JavaVM *g_jvm;
static jclass jni_class;
static jmethodID on_file_method;

JNIIPlaylistLoader::JNIIPlaylistLoader(JNIEnv *env, jobject obj)
{

}

static void JNICALL Java_com_nullsoft_replicant_IPlaylistLoader_nativeClassInit(JNIEnv * env, jclass cls)
{
	jni_class = (jclass)env->NewGlobalRef(cls);
	on_file_method = env->GetMethodID(jni_class, "onFile", "(Ljava/lang/String;)Z");
}

JNINativeMethod JNIIPlaylistLoader::jni_methods[] =
{
	{ "nativeClassInit", "()V", (void *) Java_com_nullsoft_replicant_IPlaylistLoader_nativeClassInit },
};

size_t JNIIPlaylistLoader::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIIPlaylistLoader::jni_classname = "com/nullsoft/replicant/PlaylistManager";
