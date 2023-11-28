#include "android-jni.h"
#include "nswasabi/ReferenceCounted.h"
#include "JNIAutoTagAlbum.h"
#include "JNIMetadata.h"
#include "gracenote/ifc_gracenote_results.h"
#include <new>

#include <android/log.h>
#include "foundation/export.h"

// Globals to java
extern JavaVM *g_jvm;

api_gracenote *REPLICANT_API_GRACENOTE = 0;

// Java class
static jclass autotag_class = 0;

JNIGracenoteCallback *gn_callback;

  /////////////////////////////////////////////////////////////////////////
 // JNIAutoTagAlbum - JNI support methods
///////////////////////////////////////////////////////////////////////////

static void JNICALL JNINativeClassInitAlbum(JNIEnv * env, jclass cls)
{
	autotag_class = (jclass)env->NewGlobalRef(cls);
	
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","JNIAutoTag: Got reference to event callback methods autoTagEventOnStatusChangedMethod: '%x'", autoTagEventOnStatusChangedMethod);
}


static jint JNICALL JNINativeSetupGracenote(JNIEnv * env, jclass cls)
{
	if (!gn_callback)
		gn_callback = new (std::nothrow) JNIGracenoteCallback();
	
	//api_syscb *WASABI2_API_SYSCB = 0;
	
	if (!REPLICANT_API_GRACENOTE)
		WASABI2_API_SVC->GetService(&REPLICANT_API_GRACENOTE);

	if (REPLICANT_API_GRACENOTE)
	{
		
		

		// See if we have the System callbacks API
		/*if (!WASABI2_API_SYSCB)
		{
			WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] WASABI2_API_SYSCB service retreived. '%x'.", WASABI2_API_SYSCB);
		}	 */

		if (WASABI2_API_SYSCB)
		{			
			//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] WASABI2_API_SYSCB service found. '%x'.", WASABI2_API_SYSCB);
			WASABI2_API_SYSCB->RegisterCallback((Gracenote::SystemCallback *)(gn_callback));
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] WASABI2_API_SYSCB Callback registered. '%x'", gn_callback);

			// Wiring up the static callbacks to Java so that clients can be aware of setup status and completion
			gn_callback->autoTagEventOnSetupGracenoteStatusMethod = env->GetStaticMethodID(autotag_class, "onSetupGracenoteStatus", "(I)V");
			gn_callback->autoTagEventOnSetupGracenoteErrorMethod = env->GetStaticMethodID(autotag_class, "onSetupGracenoteError", "(II)V");


		}
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] WASABI2_API_SYSCB service NOT FOUND.", WASABI2_API_SYSCB);
		
		int ret = gracenote_api->Initialize();

		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSetupGracenote initialized. '%x'.", REPLICANT_API_GRACENOTE);
		
		return (jint)ret;
	}
	
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSetupGracenote ERROR, cannot get gracenote api service '%x'.", REPLICANT_API_GRACENOTE);
	
	return (jint)NErr_Error;
}

static jint JNICALL JNINativeCreateAlbum(JNIEnv * env, jobject obj)
{
	JNIAutoTagAlbum *jni_autotag = new (std::nothrow) JNIAutoTagAlbum(env, obj);
	if (!jni_autotag)
		return 0;

	// Setting up the java callbacks
	jni_autotag->autoTagEventOnStatusChangedMethod = env->GetMethodID(autotag_class, "onStatusChange", "(Ljava/lang/String;I)V");
	jni_autotag->autoTagEventOnResultMethod = env->GetMethodID(autotag_class, "onResult", "(Lcom/nullsoft/replicant/Metadata;)V");

	
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Created JNI autotag album object jni_autotag '%x'.", jni_autotag);
	
	if (jni_autotag->InitializeGracenoteAPI() == NErr_Success)
	{
		//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] gracenote_api is initialized.");
		REPLICANT_API_GRACENOTE->CreateAutoTag_Album(&jni_autotag->autotagger, jni_autotag, 0);
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Created new ALBUM query object %x", &jni_autotag->autotagger);


	}
	else
	{
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] gracenote_api initialization failed.");
	}
	
	return (jint)jni_autotag;
}



static jint JNICALL JNINativeAddTrackAlbum(JNIEnv * env, jobject obj, jint token, jstring jfilename)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)token;

	nx_uri_t nx_filename;
	
	int ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		ret = jni_autotag->AddTrack(nx_filename);
		if (ret == NErr_Success)
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeAddTrackAlbum, track '%s' added successfully.", nx_filename->string);

		NXURIRelease(nx_filename);

		return ret;
	}

	JNIThrowExceptionForNError(env, ret);

	return (jint)ret;
}

static jint JNICALL JNINativeAddSimpleAlbum(JNIEnv * env, jobject obj, jint token, jstring jartist, jstring jtitle)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)token;

	if (!REPLICANT_API_GRACENOTE || !jni_autotag->autotagger)
	{
		return NErr_FailedCreate;
	}

	ReferenceCountedNXString nx_artist, nx_title;
	int ret = NXStringCreateWithJString(env, jartist, &nx_artist);
	if (ret != NErr_Success)
	{
		JNIThrowExceptionForNError(env, ret);
		return ret;
	}

	ret = NXStringCreateWithJString(env, jtitle, &nx_title);
	if (ret != NErr_Success)
	{
		JNIThrowExceptionForNError(env, ret);
		return ret;
	}
		
	ret = jni_autotag->AddSimple(nx_artist, nx_title);
	return ret;	
}

static jint JNICALL JNINativeRunQueryAlbum(JNIEnv * env, jobject obj, jint token, jint jquery_flags)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)token;

	int query_flags = (int)jquery_flags;
	
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Begin running Gracenote query with flags '%x'", query_flags);
	int ret = jni_autotag->autotagger->Run(query_flags);
	
	//JNIThrowExceptionForNError(env, ret);

	return (jint)ret;
}

// This is the support method for the static call to save Album Art, this is preferred current method over the instance saves, JNINativeSaveAllAlbum, and JNINativeSaveTrackAlbum
static jint JNICALL JNINativeSaveResultsAlbum(JNIEnv * env, jobject obj, jint jresult_token, jint jflags)
{
	int ret;
	int flags = (int)jflags;
	
	ifc_gracenote_results *result = (ifc_gracenote_results *)jresult_token;

	if (!REPLICANT_API_GRACENOTE)
		WASABI2_API_SVC->GetService(&REPLICANT_API_GRACENOTE);

	if (REPLICANT_API_GRACENOTE)
	{
		/*if ( (flags & GracenoteAPI::SAVE_NO_METADATA) == GracenoteAPI::SAVE_NO_METADATA )
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveResultsAlbum NOT saving any Metadata because of 'SAVE_NO_METADATA' flag.");
		else if ( (flags & GracenoteAPI::SAVE_NO_COVER_ART) != GracenoteAPI::SAVE_NO_COVER_ART )	
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveResultsAlbum NOT saving any Album Art because of 'SAVE_NO_COVER_ART' flag.");
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveResultsAlbum attempting to save both Metadata & Album Art.");
		*/
		ret = REPLICANT_API_GRACENOTE->SaveAlbumResults(result, flags);

		if (ret == NErr_Success)
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveResultsAlbum successfull.");
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveResultsAlbum FAILED!");
	}
	return (jint)ret;
}

// This is the support method for the static call to save Album Art, this is preferred current method over the instance saves, JNINativeSaveAllAlbum, and JNINativeSaveTrackAlbum
static jint JNICALL JNINativeSaveSingleResultAlbum(JNIEnv * env, jobject obj, jint jresult_token, jint jflags, jstring jfilename)
{
	int ret;
	int flags = (int)jflags;
	nx_uri_t nx_filename;

	ifc_gracenote_results *result = (ifc_gracenote_results *)jresult_token;

	ret = NXURICreateWithJString(env, jfilename, &nx_filename);
	if (ret == NErr_Success)
	{
		if (!REPLICANT_API_GRACENOTE)
			WASABI2_API_SVC->GetService(&REPLICANT_API_GRACENOTE);

		if (REPLICANT_API_GRACENOTE)
		{
			ret = REPLICANT_API_GRACENOTE->SaveTrackResults(nx_filename, result, flags);

			if (ret == NErr_Success)
				__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveSingleResultAlbum SUCCESSFUL for track '%s'.", nx_filename->string);
			else
				__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveSingleResultAlbum FAILED for track '%s'!", nx_filename->string);
		}

		NXURIRelease(nx_filename);

		return (jint)ret;
	}
}

// !!!!!!!!!!!!!!!!!!
// !!! DEPRECATED !!!
// !!!!!!!!!!!!!!!!!!
// This is the instance level saving method when not calling the static save of AutoTagAlbum
static jint JNICALL JNINativeSaveAllAlbum(JNIEnv * env, jobject obj, jint token, jint jresult_token, jint jflags)
{
	int ret;
	int flags = (int)jflags;
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)token;

	ifc_gracenote_results *result = (ifc_gracenote_results *)jresult_token;
	
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Calling JNINativeSaveAllAlbum.");
	ret = jni_autotag->autotagger->SaveAll(result, flags);

	if (ret == NErr_Success)
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveAllAlbum successfull.");
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveAllAlbum FAILED!");
	
	return (jint)ret;
}

// !!!!!!!!!!!!!!!!!!
// !!! DEPRECATED !!!
// !!!!!!!!!!!!!!!!!!
// This is the instance level saving method when not calling the static save of AutoTagAlbum
static jint JNICALL JNINativeSaveTrackAlbum(JNIEnv * env, jobject obj, jint token, jint jresult_token, jint jflags)
{
	int flags = (int)jflags;
	int ret;
	//int track_index = (int)jtrack_index;

	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)token;

	ifc_gracenote_results *result = (ifc_gracenote_results *)jresult_token;

	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Calling JNINativeSaveTrackAlbum.");
	ret = jni_autotag->autotagger->SaveTrack(result, flags);

	if (ret == NErr_Success)
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveTrackAlbum successfull.");
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] JNINativeSaveTrackAlbum FAILED!");


	return (jint)ret;
}

// This is used to lock the AuotTagAlbum object so that it is not cleaned up on the native side, it can be then be recreated from scratch with a token in java later
static jint JNICALL JNINativeLockAlbum(JNIEnv *env, jobject obj, jint autotag_token)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)autotag_token;
	
	if (jni_autotag)
	{
		return (jint)jni_autotag->Lock();
	}
	return (jint)NErr_NullPointer;
}

// Used to unlock a lock on an AutoTagAlbum object
static jint JNICALL JNINativeUnlockAlbum(JNIEnv *env, jobject obj, jint autotag_token)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)autotag_token;
	
	if (jni_autotag)
	{
		return (jint)jni_autotag->Unlock();
	}
	return (jint)NErr_NullPointer;
}


// This method needs to be called manually from java for us to shut down the use of the autotagger
static jint JNICALL JNINativeShutdownAlbum(JNIEnv * env, jobject obj, jint autotag_token)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)autotag_token;

	if (jni_autotag)
	{
		return (jint)jni_autotag->Shutdown(env, obj);
	}
	return (jint)NErr_NullPointer;
}

// Method is called when Java grabage collector calls finalize
// Return's 0 on successful delete
// Returns the autotag_token back if we retain it successfully
static jint JNICALL JNINativeReleaseAlbum(JNIEnv *env, jobject obj, jint autotag_token)
{
	JNIAutoTagAlbum *jni_autotag = (JNIAutoTagAlbum *)autotag_token;
	
	if (jni_autotag)
	{
		if (jni_autotag->GetLock() == true)
		{
			__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIAutoTagAlbum] JNINativeRelease: AutoTagAlbum object '%x' not released due to free lock.", jni_autotag);
			// Somebody better unlock us later, otherwise we will leak
			return autotag_token;
		}
		else
		{
			// Clean up and dergister the system callbacks for the static system call
			if (gn_callback)
			{
				WASABI2_API_SYSCB->UnregisterCallback((Gracenote::SystemCallback *)(gn_callback));
				delete gn_callback;
				gn_callback = 0;
			}
			
			assert(jni_autotag);
			delete jni_autotag;
			__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIAutoTagAlbum] JNINativeRelease: AutoTagAlbum object '%x' successfully released & freed.", jni_autotag);
			return (jint)NErr_Success;
		}
	}
	return 0;
}


  /////////////////////////////////////////////////////////////////////////
 // JNIGracenoteCallback - Class to handle static callbacks to java on gracenote status
///////////////////////////////////////////////////////////////////////////
JNIGracenoteCallback::JNIGracenoteCallback()
{
	autoTagEventOnSetupGracenoteStatusMethod = 0;
	autoTagEventOnSetupGracenoteErrorMethod = 0;
}

int JNIGracenoteCallback::GracenoteSystemCallback_OnStatus(int status_message)
{
	//__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIGracenoteCallback] Callback OnStatus issued. '%x'", status_message);

	JNIEnv *env = JNIGetThreadEnvironment();
	
	if (env)
	{
		if (this->autoTagEventOnSetupGracenoteStatusMethod)
		{
			jint jstatus = status_message;
			env->CallStaticVoidMethod(autotag_class, this->autoTagEventOnSetupGracenoteStatusMethod, jstatus);
		}
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIGracenoteCallback] autoTagEventOnSetupGracenoteStatusMethod is NULL.");

		// Make sure that when we get the last status of 'LOADED' that we detach from the JVM because this thread will die and cause a deadd00d crash otherwise
		//if (status_message == STATUS_GRACENOTE_LOADED)
			//DetachFromThread();
	}
	
	return NErr_Success;
}

int  JNIGracenoteCallback::GracenoteSystemCallback_OnError(int status_message, ns_error_t error_code)
{
	JNIEnv *env = JNIGetThreadEnvironment();
	
	if (env)
	{
		if (this->autoTagEventOnSetupGracenoteErrorMethod)
		{
			jint jstatus = status_message;
			jint jerror = error_code;
			env->CallStaticVoidMethod(autotag_class, this->autoTagEventOnSetupGracenoteErrorMethod, jstatus, jerror);
		}
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIGracenoteCallback] autoTagEventOnSetupGracenoteErrorMethod is NULL.");
	}
	
	return NErr_Success;
}


///////////////////////////////////////////////////////////////////////////
// JNIAutoTagAlbum Member Methods
///////////////////////////////////////////////////////////////////////////

JNIAutoTagAlbum::JNIAutoTagAlbum(JNIEnv *env, jobject obj)
{
	java_autotag=env->NewGlobalRef(obj);

	autotagger = 0;
	gracenote_api = 0;
	free_lock = false;

	autoTagEventOnStatusChangedMethod = 0;
	autoTagEventOnResultMethod = 0;

}

NError JNIAutoTagAlbum::Lock(void)
{
	free_lock = true;
	return NErr_Success;
}

NError JNIAutoTagAlbum::Unlock(void)
{
	free_lock = false;
	return NErr_Success;
}

bool JNIAutoTagAlbum::GetLock(void)
{
	return free_lock;
}

// DEPRECATED
/*NError JNIAutoTagAlbum::RunQuery(int query_flags)
{
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Begin running Gracenote query for all tracks added to ALBUM with flags '%x'");

	autotagger->Run(query_flags);

	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Query complete for all tracks added to ALBUM.");

	return NErr_Success;
}  */


NError JNIAutoTagAlbum::InitializeGracenoteAPI(void)
{
	if (gracenote_api == 0)
	{
		WASABI2_API_SVC->GetService(&gracenote_api);

		if (gracenote_api)
		{
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Successfully created gracenote_api '%x'.", gracenote_api);
			
			// Initialize the gracenote API for a new user
			
			return NErr_Success;
		}
		else
		{
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Failed to create gracenote_api. '%x'", gracenote_api);
			return NErr_Error;
		}
	}
	else
		__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Already have a gracenote_api. '%x'", gracenote_api);
	return NErr_Success;
}

NError JNIAutoTagAlbum::AddTrack(nx_uri_t nx_filename)
{
	int ret = autotagger->Add(nx_filename);

	return ret;
}

ns_error_t JNIAutoTagAlbum::AddSimple(nx_string_t artist, nx_string_t title)
{
	return autotagger->AddSimple(artist, title);
}

NError JNIAutoTagAlbum::Shutdown(JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Shutting down. '%x'", java_autotag);
	env->DeleteGlobalRef(java_autotag);

	// ToDo: Do some more error checking here
	return NErr_Success;
}

// Callbacks
int JNIAutoTagAlbum::AutoTagCallback_OnStatus(nx_uri_t filename, int status)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		
		if (java_autotag && autoTagEventOnStatusChangedMethod)
		{
			/*if (filename)
				__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Calling autoTagEventOnStatusChangedMethod, with status: '%x' and filename '%s'", status, filename->string);
			else
				__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] Calling autoTagEventOnStatusChangedMethod, with status: '%x' and filename IS NULL", status);
			*/
			jstring jfilename = 0;
			jint jstatus = (jint)status;
			nx_string_t nx_filename = 0;
			int ret = 0;
			
			if (filename != 0)
			{
				int ret = NXURIGetNXString(&nx_filename, filename);
				if (ret == NErr_Success)
				{
					if (nx_filename != 0)
					{
						ret = NXStringCreateJString(env, nx_filename, &jfilename);
						NXStringRelease(nx_filename);
						if (ret != NErr_Success)
						{
							return NErr_Error;
						}
					}

				}
				else
				{
					return NErr_Error;
				}
			}	 

			env->CallVoidMethod(java_autotag, this->autoTagEventOnStatusChangedMethod, jfilename, jstatus);
			return NErr_Success;
		}
		else
			__android_log_print(ANDROID_LOG_DEBUG,"libreplicant","[JNIAutoTagAlbum] java_autotag or autoTagEventOnStatusChangedMethod is NULL.");

		return NErr_Error;
	}
}

int JNIAutoTagAlbum::AutoTagCallback_OnResults(ifc_gracenote_results *results)
{
	JNIEnv *env = JNIGetThreadEnvironment();

	if (env)
	{
		jobject java_metadata_result = JNIMetadataCreate(env, results);

		env->CallVoidMethod(java_autotag, this->autoTagEventOnResultMethod, java_metadata_result);
		
		if (java_metadata_result)
			env->DeleteLocalRef(java_metadata_result);
	}
}

// JNI Methods
JNINativeMethod JNIAutoTagAlbum::jni_methods[] = {
	{ "nativeClassInitAlbum", "()V", (void *) JNINativeClassInitAlbum },
	{ "nativeCreateAlbum", "()I", (void *) JNINativeCreateAlbum },
	{ "nativeSetupGracenote", "()I", (void *) JNINativeSetupGracenote },
	{ "nativeAddTrackAlbum", "(ILjava/lang/String;)I", (void *) JNINativeAddTrackAlbum },
	{ "nativeRunQueryAlbum", "(II)I", (void *) JNINativeRunQueryAlbum },
	{ "nativeSaveResultsAlbum", "(II)I", (void *) JNINativeSaveResultsAlbum },
	{ "nativeSaveSingleResultAlbum", "(IILjava/lang/String;)I", (void *) JNINativeSaveSingleResultAlbum },
	{ "nativeSaveTrackAlbum", "(III)I", (void *) JNINativeSaveTrackAlbum },
	{ "nativeSaveAllAlbum", "(III)I", (void *) JNINativeSaveAllAlbum },
	{ "nativeLockAlbum", "(I)I", (void *) JNINativeLockAlbum },
	{ "nativeUnlockAlbum", "(I)I", (void *) JNINativeUnlockAlbum },
	{ "nativeReleaseAlbum", "(I)I", (void *) JNINativeReleaseAlbum },
	{ "nativeShutdownAlbum", "(I)I", (void *) JNINativeShutdownAlbum },
	{ "nativeAddTrackSimple", "(ILjava/lang/String;Ljava/lang/String;)I", (void *) JNINativeAddSimpleAlbum },
};

size_t JNIAutoTagAlbum::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIAutoTagAlbum::jni_classname = "com/nullsoft/replicant/gracenote/AutoTagAlbum";


///////////////////////////////////////////////////////////////////////////
// Code graveyard
///////////////////////////////////////////////////////////////////////////
