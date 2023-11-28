#include "android-jni.h"
#include "JNIMediaPlayer.h"

#include <android/log.h>
#include "foundation/export.h"
#include "JNIMetadata.h"

extern JavaVM *g_jvm;

struct JNIMediaPlayerCallback
{
	jfieldID listener_field;
	jmethodID listener_method;
	void Init(JNIEnv *env, jclass media_player_class, const char *field_name, const char *class_name, const char *class_signature, const char *method_name, const char *method_signature)
	{
		listener_field = env->GetFieldID(media_player_class, field_name, class_signature);
		if (!listener_field)
			__android_log_print(ANDROID_LOG_ERROR,"libreplicant","Cannot find %s field!", field_name);

		jclass prepared_class = env->FindClass(class_name);
		if (prepared_class)
			listener_method = env->GetMethodID(prepared_class, method_name, method_signature);
		else 
			__android_log_print(ANDROID_LOG_ERROR,"libreplicant","Cannot find %s class!", class_name);
	}
};

/* cached java methodIDs and stuff */

static JNIMediaPlayerCallback metadatachanged_listener, buffering_listener;
static jclass media_player_class;
static jmethodID playerEventPreparedMethod,
	playerEventReadyMethod,
	playerEventSeekableMethod,
	playerEventLengthChangedMethod,
	playerEventPositionChangedMethod,
	playerEventSeekCompleteMethod,
	playerEventCompleteMethod,
	playerEventErrorMethod,
	playerEventStopMethod,
	playerEventClosedMethod;


JNIMediaPlayer::JNIMediaPlayer(JNIEnv *env, jobject obj)
{
	prepare_synchronous=false;
	sem_init(&prepare_semaphore, 0, 0);

	java_player=env->NewGlobalRef(obj);
}


void JNIMediaPlayer::PlayerEvents_OnLengthChanged(double new_length)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jint java_length_milliseconds = (int)( new_length * 1000 );
		env->CallVoidMethod(java_player, playerEventLengthChangedMethod, java_length_milliseconds);
	}
}

void JNIMediaPlayer::PlayerEvents_OnPositionChanged(double new_position)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jint java_position_milliseconds = (int)( new_position * 1000 );
		env->CallVoidMethod(java_player, playerEventPositionChangedMethod, java_position_milliseconds);
	}
}

void JNIMediaPlayer::PlayerEvents_OnMetadataChanged(ifc_metadata *metadata)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (metadata)
	{
		if (env)
		{
			jobject callback_object = env->GetObjectField(java_player, metadatachanged_listener.listener_field);
			if (callback_object)
			{
				jobject java_metadata = JNIMetadataCreate(env, metadata);
				env->CallVoidMethod(callback_object, metadatachanged_listener.listener_method, java_player, java_metadata);
				env->DeleteLocalRef(callback_object);
				if (java_metadata)
					env->DeleteLocalRef(java_metadata);
			}
		}
	}
}

void JNIMediaPlayer::PlayerEvents_OnEqualizerChanged(ifc_equalizer *equalizer)
{
}

void JNIMediaPlayer::PlayerEvents_OnLoaded(nx_uri_t filename)
{
	if (prepare_synchronous)
	{
		sem_post(&prepare_semaphore);
		prepare_synchronous=false;
	}
	JNIEnv *env = JNIGetThreadEnvironment();
	if (env)
	{
		env->CallVoidMethod(java_player, playerEventPreparedMethod);
	}
}

void JNIMediaPlayer::PlayerEvents_OnInitialized()
{
	JNIEnv *env = JNIGetThreadEnvironment();
}

void JNIMediaPlayer::PlayerEvents_OnError(int error_code)
{
	if (prepare_synchronous)
	{
		sem_post(&prepare_semaphore);
		prepare_synchronous=false;
	}
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventErrorMethod, 1, (jint)error_code);
	}
}

void JNIMediaPlayer::PlayerEvents_OnEndOfFile()
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventCompleteMethod);
	}	
}

void JNIMediaPlayer::PlayerEvents_OnSeekComplete(int error_code, double new_position)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventSeekCompleteMethod);
	}
}

void JNIMediaPlayer::PlayerEvents_OnSeekable(int is_seekable)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jboolean java_is_seekable = (bool)( is_seekable );
		env->CallVoidMethod(java_player, playerEventSeekableMethod, java_is_seekable);
	}
}

void JNIMediaPlayer::PlayerEvents_OnBuffering(int percent)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jint java_percent = (jint)percent;
		jobject callback_object = env->GetObjectField(java_player, buffering_listener.listener_field);
		if (callback_object)
		{
			env->CallVoidMethod(callback_object, buffering_listener.listener_method, java_player, java_percent);
			env->DeleteLocalRef(callback_object);
		}
	}	
}

void JNIMediaPlayer::PlayerEvents_OnStopped()
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventStopMethod);
	}
}

void JNIMediaPlayer::PlayerEvents_OnReady()
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventReadyMethod);
	}
}

void JNIMediaPlayer::PlayerEvents_OnClosed()
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		env->CallVoidMethod(java_player, playerEventClosedMethod);
		//__android_log_print(ANDROID_LOG_ERROR,"libreplicant","[JNIMediaPlayer] Called OnClosedEvent......");
	}
}

void JNIMediaPlayer::PlayerEvents_OnBitrateChanged(double new_bitrate)
{
}
/* ==== */

static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
	media_player_class = (jclass)env->NewGlobalRef(cls);

	metadatachanged_listener.Init(env, media_player_class, "metadataChangedListener", "com/nullsoft/replicant/MediaPlayer$OnMetadataChangedListener",
		"Lcom/nullsoft/replicant/MediaPlayer$OnMetadataChangedListener;", 
		"onMetadataChanged", "(Lcom/nullsoft/replicant/MediaPlayer;Lcom/nullsoft/replicant/Metadata;)V");

	buffering_listener.Init(env, media_player_class, "bufferingListener", "com/nullsoft/replicant/MediaPlayer$OnBufferingUpdateListener",
		"Lcom/nullsoft/replicant/MediaPlayer$OnBufferingUpdateListener;", 
		"onBufferingUpdate", "(Lcom/nullsoft/replicant/MediaPlayer;I)V");

	playerEventPreparedMethod = env->GetMethodID(media_player_class, "playerEventPrepared", "()V");
	playerEventReadyMethod = env->GetMethodID(media_player_class, "playerEventReady", "()V");
	playerEventSeekableMethod = env->GetMethodID(media_player_class, "playerEventSeekable", "(Z)V");
	playerEventLengthChangedMethod = env->GetMethodID(media_player_class, "playerEventLengthChanged", "(I)V");
	playerEventPositionChangedMethod = env->GetMethodID(media_player_class, "playerEventPositionChanged", "(I)V");
	playerEventSeekCompleteMethod = env->GetMethodID(media_player_class, "playerEventSeekComplete", "()V");
	playerEventCompleteMethod =  env->GetMethodID(media_player_class, "playerEventComplete", "()V");
	playerEventErrorMethod =   env->GetMethodID(media_player_class, "playerEventError", "(II)V");
	playerEventStopMethod =   env->GetMethodID(media_player_class, "playerEventStop", "()V");
	playerEventClosedMethod = env->GetMethodID(media_player_class, "playerEventClosed", "()V");
}

static jint JNICALL JNINativeCreate(JNIEnv * env, jobject obj)
{
	JNIMediaPlayer *jni_player = new JNIMediaPlayer(env, obj);
	if (!jni_player)
		return 0;
	jni_player->player.RegisterForEvents(jni_player);
	int ret = jni_player->player.Init();
	if (ret != NErr_Success)
	{
		delete jni_player;
		// TODO: throw exception
		return 0;
	}	
	jni_player->player.SetPlaybackParameters(&jni_player->playback_parameters);
	return (jint)jni_player;
}

static void JNICALL JNINativePrepareAsync(JNIEnv * env, jobject obj, jint token, jstring jfilename)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	nx_uri_t nx_filename;
	ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		jni_player->prepare_synchronous=false;
		ret = jni_player->player.Load(nx_filename);
		NXURIRelease(nx_filename);
	}

	JNIThrowExceptionForNError(env, ret);
}

static void JNICALL JNINativePrepare(JNIEnv * env, jobject obj, jint token, jstring jfilename)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;

	jni_player->prepare_synchronous=true;
	nx_uri_t nx_filename;
	ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		ret = jni_player->player.Load(nx_filename);
		NXURIRelease(nx_filename);

		if (ret == NErr_Success)
		{
			sem_wait(&jni_player->prepare_semaphore);
		}
	}

	JNIThrowExceptionForNError(env, ret);
}

static void JNICALL JNINativePlay(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.Start();
}

static void JNICALL JNINativePause(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.Pause();
}


static void JNICALL JNINativeStop(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.Stop();
}

static void JNICALL JNINativeSeek(JNIEnv * env, jobject obj, jint token, jint milliseconds)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.SeekSeconds((double)milliseconds / 1000.0);
}

static void JNICALL JNINativeSeekPercent(JNIEnv * env, jobject obj, jint token, jdouble percent)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.SeekPercent( (double)percent );
}

static void JNICALL JNINativeReset(JNIEnv * env, jobject obj, jint token)
{
	int ret;
	JNIMediaPlayer *jni_player = (JNIMediaPlayer *)token;
	jni_player->player.Reset();
} 

static void JNICALL JNINativeCrash(JNIEnv *env, jobject obj)
{
	*(int *)0=0;
}

JNINativeMethod JNIMediaPlayer::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeCreate", "()I", (void *) JNINativeCreate },
	{ "nativePrepareAsync", "(ILjava/lang/String;)V", (void *) JNINativePrepareAsync },
	{ "nativePrepare", "(ILjava/lang/String;)V", (void *) JNINativePrepare },
	{ "nativePlay", "(I)V", (void *) JNINativePlay },
	{ "nativePause", "(I)V", (void *) JNINativePause },
	{ "nativeStop", "(I)V", (void *) JNINativeStop },
	{ "nativeSeek", "(II)V", (void *) JNINativeSeek },
	{ "nativeSeekPercent", "(ID)V", (void *) JNINativeSeekPercent },
	{ "nativeReset", "(I)V", (void *) JNINativeReset },
	{ "nativeCrash", "()V", (void *) JNINativeCrash },
};

size_t JNIMediaPlayer::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIMediaPlayer::jni_classname = "com/nullsoft/replicant/MediaPlayer";
