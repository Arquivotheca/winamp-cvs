#pragma once
#include <jni.h>
//#include <semaphore.h>
#include "nx/nxuri.h"
#include "foundation/error.h"

#include "api.h"

  /////////////////////////////////////////////////////////////////////////
 // JNIArtwork - For retreiving artwork related to a track on various levels
///////////////////////////////////////////////////////////////////////////
//class JNIAutoTagAlbum : public ifc_gracenote_autotag_callback
class JNIArtwork
{
public:
	//JNIArtwork(JNIEnv *env, jobject obj);

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;


private:
	//JNIEnv *thread_environment;
	//JNIEnv *AttachToThread();

};

