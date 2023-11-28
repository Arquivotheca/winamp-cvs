#include "api.h"
#include "JNIMediaServer.h"
#include "nswasabi/ReferenceCounted.h"

static jclass media_server_class;

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	media_server_class = (jclass)env->NewGlobalRef(cls);
}

static void JNICALL JNINativeSetIPv4Address(JNIEnv *env, jclass cls, jint ip)
{
	if (!REPLICANT_API_MEDIASERVER)
		WASABI2_API_SVC->GetService(&REPLICANT_API_MEDIASERVER);

	if (REPLICANT_API_MEDIASERVER)
	{
		REPLICANT_API_MEDIASERVER->SetIPv4Address(ip);
	}
}

static void JNICALL JNINativeSetDestinationDirectory(JNIEnv *env, jclass cls, jstring jdirectory)
{
	if (!REPLICANT_API_MEDIASERVER)
		WASABI2_API_SVC->GetService(&REPLICANT_API_MEDIASERVER);

	if (REPLICANT_API_MEDIASERVER)
	{
		ReferenceCountedNXURI directory;
		if (NXURICreateWithJString(env, jdirectory, &directory) == NErr_Success)
	{
		REPLICANT_API_MEDIASERVER->SetDestinationDirectory(directory);
	}
	}
}

static void JNICALL JNINativeStart(JNIEnv * env, jclass cls)
{
	if (!REPLICANT_API_MEDIASERVER)
		WASABI2_API_SVC->GetService(&REPLICANT_API_MEDIASERVER);

	if (REPLICANT_API_MEDIASERVER)
	{
		REPLICANT_API_MEDIASERVER->Start();
	}
}

static void JNICALL JNINativeStop(JNIEnv * env, jclass cls)
{
	if (!REPLICANT_API_MEDIASERVER)
		WASABI2_API_SVC->GetService(&REPLICANT_API_MEDIASERVER);

	if (REPLICANT_API_MEDIASERVER)
	{
		REPLICANT_API_MEDIASERVER->Stop();
	}
}


JNINativeMethod JNIMediaServer::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeSetIPv4Address", "(I)V", (void *) JNINativeSetIPv4Address },
	{ "nativeSetDestinationDirectory", "(Ljava/lang/String;)V", (void *) JNINativeSetDestinationDirectory },
	{ "nativeStart", "()V", (void *) JNINativeStart },
	{ "nativeStop", "()V", (void *) JNINativeStop },
};
	

size_t JNIMediaServer::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIMediaServer::jni_classname = "com/nullsoft/replicant/MediaServer";
