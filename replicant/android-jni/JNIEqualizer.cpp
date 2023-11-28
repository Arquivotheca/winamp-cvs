#include "JNIEqualizer.h"

#include <android/log.h>
#include "foundation/export.h"
#include "JNIMediaPlayer.h"

extern JavaVM *g_jvm;

JNIEqualizer::JNIEqualizer(JNIEnv *env, jobject obj)
{
}

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
}

static jdouble JNICALL JNINativeGetBand(JNIEnv * env, jobject obj, jint token, jint jBand)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	unsigned int band = (unsigned int)jBand;
	
	double db = jni_player->player.GetEQBand(band);

	return (jdouble)db;
}

static void JNICALL JNINativeSetBand(JNIEnv * env, jobject obj, jint token, jint jBand, jdouble jDb)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	unsigned int band = (unsigned int)jBand;
	double db = (double)jDb; 

	jni_player->player.SetEQBand(band, db);
}

static jdouble JNICALL JNINativeGetPreamp(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	double db = jni_player->player.GetEQPreamp();

	return (jdouble)db;
}


static void JNICALL JNINativeSetPreamp(JNIEnv * env, jobject obj, jint token, jdouble jDb)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	double db = (double)jDb; 

	/*ret = */jni_player->player.SetEQPreamp(db);

}

static jint JNICALL JNINativeGetEQState(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	if (jni_player->player.GetEQState())
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

static void JNICALL JNINativeSetEQState(JNIEnv * env, jobject obj, jint token, jboolean joff_on)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	
	if (joff_on == JNI_TRUE)
		jni_player->player.SetEQState(1);
	else if (joff_on == JNI_FALSE)
		jni_player->player.SetEQState(0);

	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "EQ State Changed to :'%d'", joff_on);
	
}

JNINativeMethod JNIEqualizer::jni_methods[] =
{
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeGetBand", "(II)D", (void *) JNINativeGetBand },
	{ "nativeSetBand", "(IID)V", (void *) JNINativeSetBand },
	{ "nativeGetPreamp", "(I)D", (void *) JNINativeGetPreamp },
	{ "nativeSetPreamp", "(ID)V", (void *) JNINativeSetPreamp },
	{ "nativeGetEQState", "(I)Z", (void *) JNINativeGetEQState },
	{ "nativeSetEQState", "(IZ)V", (void *) JNINativeSetEQState },
};

size_t JNIEqualizer::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIEqualizer::jni_classname = "com/nullsoft/replicant/Equalizer";
