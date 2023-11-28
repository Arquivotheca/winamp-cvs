#pragma once
#include <jni.h>
#include <semaphore.h>
#include "nx/nxuri.h"
#include "foundation/error.h"

#include "api.h"
#include "gracenote/ifc_gracenote_autotag_track.h"

  /////////////////////////////////////////////////////////////////////////
 // JNIAutoTagAlbum - For performing track api gracenote queries 
///////////////////////////////////////////////////////////////////////////
//class JNIAutoTagAlbum : public ifc_gracenote_autotag_callback
class JNIAutoTagAlbum : public ifc_gracenote_autotag_callback
{
public:
	JNIAutoTagAlbum(JNIEnv *env, jobject obj);

	ifc_gracenote_autotag_album *autotagger;

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;

	// Java Callback methods
	jmethodID autoTagEventOnStatusChangedMethod;
	jmethodID autoTagEventOnResultMethod;

	NError Lock(void);
	NError Unlock(void);
	bool GetLock(void);
	NError InitializeGracenoteAPI(void);
	NError AddTrack(nx_uri_t nx_filename);
	ns_error_t AddSimple(nx_string_t artist, nx_string_t title);
	NError Shutdown(JNIEnv *env, jobject obj);
	
	// DEPRECATED
	//NError RunQuery(int query_flags);
	//NError SaveTrack(ifc_gracenote_results *results, int index);
	//NError SaveAll(ifc_gracenote_results *results);

private:
	jobject java_autotag;
	
	bool free_lock;

	/* AutoTag callbacks */
	int WASABICALL AutoTagCallback_OnStatus(nx_uri_t filename, int status);
	int WASABICALL AutoTagCallback_OnResults(ifc_gracenote_results *results);	
};

class JNIGracenoteCallback : public Gracenote::SystemCallback
{
public:
	JNIGracenoteCallback();

	jmethodID autoTagEventOnSetupGracenoteStatusMethod;
	jmethodID autoTagEventOnSetupGracenoteErrorMethod;

private:
	int WASABICALL GracenoteSystemCallback_OnStatus(int status_message);
	int WASABICALL GracenoteSystemCallback_OnError(int status_message, ns_error_t error_code);
};

