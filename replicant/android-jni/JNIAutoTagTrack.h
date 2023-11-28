#pragma once
#include <jni.h>
#include <semaphore.h>
#include "nx/nxuri.h"
#include "foundation/error.h"

#include "api.h"
#include "gracenote/ifc_gracenote_autotag_track.h"

  /////////////////////////////////////////////////////////////////////////
 // JNIAutoTagTrack - For performing track api gracenote queries 
///////////////////////////////////////////////////////////////////////////
class JNIAutoTagTrack : public ifc_gracenote_autotag_callback
{
public:
	JNIAutoTagTrack(JNIEnv *env, jobject obj);

	ifc_gracenote_autotag_track *autotagger;

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;

	// Java Callback methods
	jmethodID autoTagEventOnStatusChangedMethod;
	jmethodID autoTagEventOnResultMethod;

	NError RunQuery(nx_uri_t nx_filename);
	NError RunQuerySimple(nx_string_t artist, nx_string_t title);

	NError InitializeGracenoteAPI(void);
	NError Save(void);
private:
	jobject java_autotag;

	JNIEnv *thread_environment;
	JNIEnv *AttachToThread();
	
	/* AutoTag callbacks */
	int WASABICALL AutoTagCallback_OnStatus(nx_uri_t filename, int status);
	int WASABICALL AutoTagCallback_OnResults(ifc_gracenote_results *results);	
};