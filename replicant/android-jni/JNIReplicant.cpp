#include "api.h"
#include "android-jni.h"
#include "component/android/ComponentManager.h"
#include <android/log.h>
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "JNIReplicant.h"
#include "Application.h"
#include "nx/cpufeatures/cpu-features.h"
#include "Replicant/Replicant.h"
#include "nswasabi/ReferenceCounted.h"
#include <stdio.h>
#include "Wasabi/Wasabi.h"

api_metadata *REPLICANT_API_METADATA=0;
api_artwork *REPLICANT_API_ARTWORK=0;
api_mediaserver *REPLICANT_API_MEDIASERVER=0;

AndroidAPI android_api;
ComponentManager component_manager;
Application application;

static SingletonServiceFactory<AndroidAPI, api_android> android_factory;
static SingletonServiceFactory<Application, api_application> application_factory;


static void JNICALL JNINativeClassInit(JNIEnv * env, jclass cls)
{
}

static void JNICALL JNINativeInit(JNIEnv * env, jobject obj)
{
	
	application.Init();
	int error_code=Wasabi_Init();
	if (error_code != NErr_Success)
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "Wasabi failed to load (error code %d)", error_code);
		// TODO: throw exception
	}
	else 
	{
		application_factory.Register(WASABI2_API_SVC, WASABI2_API_APP);	
		android_factory.Register(WASABI2_API_SVC, WASABI2_API_ANDROID);
		Replicant_Common_Initialize(WASABI2_API_SVC);
		Replicant_Playback_Initialize(WASABI2_API_SVC);
		Replicant_Codec_Initialize(WASABI2_API_SVC);
		Replicant_Metadata_Initialize(WASABI2_API_SVC);
		WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
		component_manager.SetServiceAPI(WASABI2_API_SVC);
		WASABI2_API_SVC->GetService(&REPLICANT_API_ARTWORK);
	}
}

static void JNICALL JNINativeSetTempPath(JNIEnv *env, jobject obj, jstring jtemp_path)
{
	ReferenceCountedNXString temp_path;
	if (NXStringCreateWithJString(env, jtemp_path, &temp_path) == NErr_Success)
	{
		ReferenceCountedNXURI temp_path_uri;
		if (NXURICreateWithNXString(&temp_path_uri, temp_path) == NErr_Success)
		{
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "using %s as the temp path", temp_path_uri->string);
			NXURISetTempPath(temp_path_uri);
		}
	}
}

static void JNICALL JNINativeLoadComponents(JNIEnv *env, jobject obj, jstring jdirectory)
{
	nx_uri_t nx_directory;
	int ret = NXURICreateWithJString(env, jdirectory, &nx_directory);
	if (ret == NErr_Success)
	{		
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "Loading components from %s", nx_directory->string);
		component_manager.AddDirectory(nx_directory);
		NXURIRelease(nx_directory);
		component_manager.Load();
	}
	else
	{
		JNIThrowExceptionForNError(env, ret);
	}
}

static void JNICALL JNINativeSetSDK(JNIEnv *env, jobject obj, jint sdk_version, jstring jrelease)
{
	ReferenceCountedNXString release;
	if (NXStringCreateWithJString(env, jrelease, &release) == NErr_Success)
	{
		application.SetSDK(sdk_version, release);
	}
}

static void JNICALL JNINativeSetDeviceID(JNIEnv *env, jobject obj, jstring jdevid)
{
	ReferenceCountedNXString devid;
	if (NXStringCreateWithJString(env, jdevid, &devid) == NErr_Success)
	{
		application.SetDeviceID(devid);
	}
}

static void JNICALL JNINativeSetApplication(JNIEnv * env, jobject obj, jstring jappname, jstring jshortname, jstring jversion)
{
	int ret;
	ReferenceCountedNXString appname, shortname, version;
	ret = NXStringCreateWithJString(env, jappname, &appname);
	if (ret == NErr_Success)
	{
		ret = NXStringCreateWithJString(env, jshortname, &shortname);
		if (ret == NErr_Success)
		{
			ret = NXStringCreateWithJString(env, jversion, &version);
			if (ret == NErr_Success)
			{
				application.SetApplication(appname, shortname, version);
				return ;
			}
		}
	}
	JNIThrowExceptionForNError(env, ret);
}

static jboolean JNICALL JNINativeIsSupportedCPU(JNIEnv * env, jobject obj)
{
	uint64_t features = android_getCpuFeatures();
	if (features & (ANDROID_CPU_ARM_FEATURE_VFP|ANDROID_CPU_ARM_FEATURE_VFPv3))
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

/* simple helper function for converting from a UTF string to a GUID */
static GUID GUIDFromCString(const char *source) 
{
	GUID guid;
	int Data1, Data2, Data3;
	int Data4[8];

	//{ 0x1b3ca60c, 0xda98, 0x4826, { 0xb4, 0xa9, 0xd7, 0x97, 0x48, 0xa5, 0xfd, 0x73 } };
	//sscanf( source, " { 0x%08x, 0x%04x, 0x%04x, { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x } } ; ",
	
	// {2E9CE2F8-E26D-4629-A3FF-5DF619136B2C}
	sscanf( source, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
	
		&Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
		Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7 );

	// Cross assign all the values
	guid.Data1 = Data1;
	guid.Data2 = Data2;
	guid.Data3 = Data3;
	guid.Data4[0] = Data4[0];
	guid.Data4[1] = Data4[1];
	guid.Data4[2] = Data4[2];
	guid.Data4[3] = Data4[3];
	guid.Data4[4] = Data4[4];
	guid.Data4[5] = Data4[5];
	guid.Data4[6] = Data4[6];
	guid.Data4[7] = Data4[7];

	return guid;
}

static void JNICALL JNINativeSetPermission(JNIEnv * env, jobject obj, jstring jguid)
{
	/* in theory, we could avoid the malloc that GetStringUTFChars does if we were willing to write a utf-16 GUID parser, but it's not worth it */
	jboolean is_copy;
	const char *guid_string = env->GetStringUTFChars(jguid, &is_copy);
	
	if (guid_string)
	{
		GUID permission_guid = GUIDFromCString(guid_string);
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Features] ENABLING permission: '%s'", guid_string);
		env->ReleaseStringUTFChars(jguid, guid_string);

		application.SetPermission(permission_guid);
	}
}

static void JNICALL JNINativeRemovePermission(JNIEnv * env, jobject obj, jstring jguid)
{
	jboolean is_copy;
	const char *guid_string = env->GetStringUTFChars(jguid, &is_copy);
	
	if (guid_string)
	{
		GUID permission_guid = GUIDFromCString(guid_string);
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Features] DISABLING (remove) permission: '%s'", guid_string);
		env->ReleaseStringUTFChars(jguid, guid_string);

		application.RemovePermission(permission_guid);
	}
}

static jboolean JNICALL JNINativeGetFeature(JNIEnv * env, jobject obj, jstring jguid)
{
	/* in theory, we could avoid the malloc that GetStringUTFChars does if we were willing to write a utf-16 GUID parser, but it's not worth it */
	jboolean is_copy;
	const char *guid_string = env->GetStringUTFChars(jguid, &is_copy);
	if (!guid_string)
		return JNI_FALSE;

	GUID feature_guid = GUIDFromCString(guid_string);
	env->ReleaseStringUTFChars(jguid, guid_string);

	if (application.GetFeature(feature_guid) == NErr_True)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

static void JNICALL JNINativeSetDataPath(JNIEnv *env, jobject obj, jstring jdirectory)
{
	nx_uri_t nx_directory;
	int ret = NXURICreateWithJString(env, jdirectory, &nx_directory);
	if (ret == NErr_Success)
	{		
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "setting data path to %s", nx_directory->string);
		application.SetDataPath(nx_directory);
		NXURIRelease(nx_directory);
	}
	else
	{
		JNIThrowExceptionForNError(env, ret);
	}
}

static void JNICALL JNINativeEnableAllPermissions(JNIEnv * env, jobject obj)
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Permissions] enabling ALL permissions");
	application.EnableAllPermissions();	
}

static void JNICALL JNINativeClearPermissions(JNIEnv * env, jobject obj)
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Permissions] clearing ALL permissions");
	application.ClearPermissions();	
}

static void JNICALL JNINativeNotifyPermissions(JNIEnv * env, jobject obj)
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Permissions] notifying components about permissions update");
	application.NotifyPermissions(WASABI2_API_SYSCB);	
	application.DumpPermissions();
}

JNINativeMethod JNIReplicant::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeInit", "()V", (void *) JNINativeInit },
	{ "nativeSetTempPath", "(Ljava/lang/String;)V", (void *) JNINativeSetTempPath },
	{ "nativeLoadComponents", "(Ljava/lang/String;)V", (void *) JNINativeLoadComponents },
	{ "nativeSetSDK", "(ILjava/lang/String;)V", (void *) JNINativeSetSDK },
	{ "nativeSetApplication", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void *) JNINativeSetApplication },
	{ "nativeIsSupportedCPU", "()Z", (void *)JNINativeIsSupportedCPU },
	{ "nativeSetPermission", "(Ljava/lang/String;)V", (void *)JNINativeSetPermission},
	{ "nativeRemovePermission", "(Ljava/lang/String;)V", (void *)JNINativeRemovePermission},
	{ "nativeGetFeature", "(Ljava/lang/String;)Z", (void *)JNINativeGetFeature},
	{ "nativeSetDataPath", "(Ljava/lang/String;)V", (void *)JNINativeSetDataPath},
	{ "nativeEnableAllPermissions", "()V", (void *)JNINativeEnableAllPermissions},
	{ "nativeClearPermissions", "()V", (void *)JNINativeClearPermissions},
	{ "nativeNotifyPermissions", "()V", (void *)JNINativeNotifyPermissions},
	{ "nativeSetDeviceID", "(Ljava/lang/String;)V", (void *) JNINativeSetDeviceID },
};

size_t JNIReplicant::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char *JNIReplicant::jni_classname = "com/nullsoft/replicant/Replicant";
