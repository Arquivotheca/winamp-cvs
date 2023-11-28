#include "JNIPlaybackParameters.h"

#include <android/log.h>
#include "foundation/export.h"
#include "JNIMediaPlayer.h"

extern JavaVM *g_jvm;

JNIPlaybackParameters::JNIPlaybackParameters(JNIEnv *env, jobject obj)
{
}

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","Initializing PlaybackParameters");
}

static void JNICALL JNINativeReplayGain_SetFlags(JNIEnv * env, jobject obj, jint token, jint jnew_flags)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	int new_flags = (int)jnew_flags;
	
	jni_player->playback_parameters.ReplayGain_SetFlags(new_flags);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting flags '%d'", new_flags);
}

static jint JNICALL JNINativeReplayGain_GetFlags(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jint)jni_player->playback_parameters.ReplayGain_GetFlags();
}

static void JNICALL JNINativeReplayGain_SetState(JNIEnv * env, jobject obj, jint token, jboolean jnew_state)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	if (jnew_state == JNI_TRUE)
	{
		jni_player->playback_parameters.ReplayGain_SetState(true);
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Enabled.");
	}
	else
	{
		jni_player->playback_parameters.ReplayGain_SetState(false);
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Disabled.");
	}
}

static jboolean JNICALL JNINativeReplayGain_GetState(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	if (jni_player->playback_parameters.ReplayGain_GetState())
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

static void JNICALL JNINativeReplayGain_SetMode(JNIEnv * env, jobject obj, jint token, jint jmode)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	int mode = (int)jmode;
	
	jni_player->playback_parameters.ReplayGain_SetMode(mode);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting mode '%d'.", mode);
}

static jint JNICALL JNINativeReplayGain_GetMode(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jint)jni_player->playback_parameters.ReplayGain_GetMode();
}

static void JNICALL JNINativeReplayGain_SetClippingPrevention(JNIEnv * env, jobject obj, jint token, jboolean jnew_mode)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	bool new_mode = (bool)jnew_mode;
	
	jni_player->playback_parameters.ReplayGain_SetClippingPrevention(new_mode);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting clipping prevention '%d'.", new_mode);
}

static jboolean JNICALL JNINativeReplayGain_GetClippingPrevention(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jboolean)jni_player->playback_parameters.ReplayGain_GetClippingPrevention();
}  

static void JNICALL JNINativeReplayGain_SetAuto(JNIEnv * env, jobject obj, jint token, jboolean jauto_replaygain) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	bool auto_replaygain = (bool)jauto_replaygain;
	
	jni_player->playback_parameters.ReplayGain_SetAuto(auto_replaygain);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting AUTO mode '%d'.", auto_replaygain);
}
static jboolean JNICALL JNINativeReplayGain_GetAuto(JNIEnv * env, jobject obj, jint token)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jboolean)jni_player->playback_parameters.ReplayGain_GetAuto();
}

static void JNICALL JNINativeReplayGain_SetDefaultGain(JNIEnv * env, jobject obj, jint token, jdouble jnew_gain)
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	double new_gain = (double)jnew_gain;
	
	jni_player->playback_parameters.ReplayGain_SetDefaultGain(new_gain);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting Default Gain '%f'.", new_gain);
}

static jdouble JNICALL JNINativeReplayGain_GetDefaultGain(JNIEnv * env, jobject obj, jint token) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jdouble)jni_player->playback_parameters.ReplayGain_GetDefaultGain();
}

static void JNICALL JNINativeReplayGain_SetPreamp(JNIEnv * env, jobject obj, jint token, jdouble jnew_preamp) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	double new_preamp = (double)jnew_preamp;
	
	jni_player->playback_parameters.ReplayGain_SetPreamp(new_preamp);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] ReplayGain - Setting Preamp '%f'.", new_preamp);
}
static jdouble JNICALL JNINativeReplayGain_GetPreamp(JNIEnv * env, jobject obj, jint token) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jdouble)jni_player->playback_parameters.ReplayGain_GetPreamp();
}

// Crossfade methods
static void JNICALL JNINativeCrossfade_SetStatus(JNIEnv * env, jobject obj, jint token, jint jstatus) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	int status = (int)jstatus;
	
	jni_player->playback_parameters.Crossfade_SetStatus(status);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] Crossfade - Setting status '%d'.", status);
}

static jint JNICALL JNINativeCrossfade_GetStatus(JNIEnv * env, jobject obj, jint token) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jint)jni_player->playback_parameters.Crossfade_GetStatus();
}

static void JNICALL JNINativeCrossfade_SetTime(JNIEnv * env, jobject obj, jint token, jdouble jnew_time) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	double new_time = (double)jnew_time;
	
	jni_player->playback_parameters.Crossfade_SetTime(new_time);
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[Playback Parameters] Crossfade - Setting time '%f'.", new_time);
}

static jdouble JNICALL JNINativeCrossfade_GetTime(JNIEnv * env, jobject obj, jint token) 
{
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	return (jdouble)jni_player->playback_parameters.Crossfade_GetTime();
}



JNINativeMethod JNIPlaybackParameters::jni_methods[] =
{
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeReplayGain_SetFlags", "(II)V", (void *) JNINativeReplayGain_SetFlags },
	{ "nativeReplayGain_GetFlags", "(I)I", (void *) JNINativeReplayGain_GetFlags },
	{ "nativeReplayGain_SetState", "(IZ)V", (void *) JNINativeReplayGain_SetState},
	{ "nativeReplayGain_GetState", "(I)Z", (void *) JNINativeReplayGain_GetState },
	{ "nativeReplayGain_SetMode", "(II)V", (void *) JNINativeReplayGain_SetMode },
	{ "nativeReplayGain_GetMode", "(I)I", (void *) JNINativeReplayGain_GetMode  },
	{ "nativeReplayGain_SetClippingPrevention", "(IZ)V", (void *) JNINativeReplayGain_SetClippingPrevention  },
	{ "nativeReplayGain_GetClippingPrevention", "(I)Z", (void *) JNINativeReplayGain_GetClippingPrevention  },
	{ "nativeReplayGain_SetAuto", "(IZ)V", (void *) JNINativeReplayGain_SetAuto  },
	{ "nativeReplayGain_GetAuto", "(I)Z", (void *) JNINativeReplayGain_GetAuto  },
	{ "nativeReplayGain_SetDefaultGain", "(ID)V", (void *) JNINativeReplayGain_SetDefaultGain  },
	{ "nativeReplayGain_GetDefaultGain", "(I)D", (void *) JNINativeReplayGain_GetDefaultGain  },
	{ "nativeReplayGain_SetPreamp", "(ID)V", (void *) JNINativeReplayGain_SetPreamp  },
	{ "nativeReplayGain_GetPreamp", "(I)D", (void *) JNINativeReplayGain_GetPreamp  },
	
	{ "nativeCrossfade_SetStatus", "(II)V", (void *) JNINativeCrossfade_SetStatus  },
	{ "nativeCrossfade_GetStatus", "(I)I", (void *) JNINativeCrossfade_GetStatus  },
	{ "nativeCrossfade_SetTime", "(ID)V", (void *) JNINativeCrossfade_SetTime  },
	{ "nativeCrossfade_GetTime", "(I)D", (void *) JNINativeCrossfade_GetTime  },
};

size_t JNIPlaybackParameters::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIPlaybackParameters::jni_classname = "com/nullsoft/replicant/PlaybackParameters";
