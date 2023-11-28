#pragma once
#include <jni.h>
#include "Replicant/playback/Player.h"
#include "player/cb_playerevents.h"
#include <semaphore.h>
#include "PlaybackParameters.h"

class JNIMediaPlayer : public cb_playerevents
{
public:
	JNIMediaPlayer(JNIEnv *env, jobject obj);
	
	Player player;
	PlaybackParameters playback_parameters;
	sem_t prepare_semaphore;
	volatile bool prepare_synchronous;

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
private:
	jobject java_player;
	
	/* player event callbacks */
	void WASABICALL PlayerEvents_OnLengthChanged(double new_length);
	void WASABICALL PlayerEvents_OnPositionChanged(double new_position);
	void WASABICALL PlayerEvents_OnMetadataChanged(ifc_metadata *metadata);
	void WASABICALL PlayerEvents_OnEqualizerChanged(ifc_equalizer *equalizer);
	void WASABICALL PlayerEvents_OnLoaded(nx_uri_t filename);
	void WASABICALL PlayerEvents_OnInitialized();
	void WASABICALL PlayerEvents_OnError(int error_code);
	void WASABICALL PlayerEvents_OnEndOfFile();
	void WASABICALL PlayerEvents_OnSeekComplete(int error_code, double new_position);
	void WASABICALL PlayerEvents_OnSeekable(int is_seekable);
	void WASABICALL PlayerEvents_OnBuffering(int percent);
	void WASABICALL PlayerEvents_OnStopped();
	void WASABICALL PlayerEvents_OnReady();
	void WASABICALL PlayerEvents_OnClosed();
	void WASABICALL PlayerEvents_OnBitrateChanged(double new_bitrate);
};
