#include "api.h"
#include "android-jni.h"
#include "JNIMetadataEditor.h"
#include "JNIData.h"
#include "metadata/metadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "foundation/export.h"
#include "foundation/error.h"
#include <android/log.h>
#include <assert.h>
#include <unistd.h>

extern JavaVM *g_jvm;

// JNI Class reference cache globals
static jclass javaMetadataEditorClass;
static jmethodID javaMetadataEditorClassConstructor;
static jfieldID javaMetadataEditorClassMetadataToken;

jobject JNIMetadataEditorCreate(JNIEnv *env, ifc_metadata_editor *metadata_editor)
{
	if(!javaMetadataEditorClass)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditorCreate] Dint have a clazz = %x", javaMetadataEditorClass);
		return 0;
	}

	if(!javaMetadataEditorClassConstructor) 
		return 0;

	jobject obj = env->NewObject(javaMetadataEditorClass, javaMetadataEditorClassConstructor);
	if(!obj)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditorCreate] MetadataEditor obj = %x", obj);		
		return 0;
	}	

	env->SetIntField(obj, javaMetadataEditorClassMetadataToken, (jint)metadata_editor);
	metadata_editor->Retain();

	__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditor] creating metadataEditor Token = '%x'", (jint)metadata_editor);

	return obj;
}

static void JNICALL JNINativeClassInit(JNIEnv *env, jclass cls)
{
	// Cache the metadata java class and its members
	javaMetadataEditorClass = (jclass)env->NewGlobalRef(cls);
	javaMetadataEditorClassConstructor = env->GetMethodID(javaMetadataEditorClass, "<init>", "()V");
	javaMetadataEditorClassMetadataToken = env->GetFieldID(javaMetadataEditorClass, "metadataEditorToken", "I");
}

// Modifies metadata field string value that is existing based off of a key and integer index
static jint JNICALL JNINativeSetFieldIndex(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jint jindex, jstring jvalue)
{
	ifc_metadata_editor *metadata_editor = (ifc_metadata_editor *)metadata_editor_token;
	int key = (int)jkey;
	int index = (int)jindex;

	nx_string_t nx_value = 0;
	
	int ret = NXStringCreateWithJString(env, jvalue, &nx_value);
	if (ret == NErr_Success || ret == NErr_NullPointer)
	{
		assert(metadata_editor != 0);	

		if (metadata_editor != 0 && key >= 0) 
		{
			int ret = metadata_editor->SetField(key, index, nx_value);
			if (ret == NErr_Success)
			{
				if (nx_value != 0)
					NXStringRelease(nx_value);
				return NErr_Success;
			}
		}

		if (nx_value != 0)
			NXStringRelease(nx_value);
		return ret;
	}
}

static jint JNICALL JNINativeSetField(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jstring jvalue)
{
	return JNINativeSetFieldIndex(env, obj, metadata_editor_token, jkey, (jint)0, jvalue);
}

static jint JNICALL JNINativeAddField(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jstring jvalue)
{
	// TODO: Add another value into the metadata field for another index, or index 0 if it is the first value
	return NErr_NotImplemented;
}


// Modifies metadata field integer value that is existing based off of a key and integer index
static jint JNICALL JNINativeSetIntegerIndex(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jint jindex, jint jvalue)
{
		// TODO: Need to add / edit an existing field based off of an integer index
	ifc_metadata_editor *metadata_editor = (ifc_metadata_editor *)metadata_editor_token;
	int key = (int)jkey;
	int index = (int)jindex;
	int64_t value = (int64_t)jvalue;
	
	assert(metadata_editor != 0);	

	if (metadata_editor != 0 && key >= 0) 
	{
		int ret = metadata_editor->SetInteger(key, index, value);
		if (ret == NErr_Success)
		{
			return NErr_Success;
		}
		return ret;
	}

	return NErr_Error;
}

static jint JNICALL JNINativeSetInteger(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jint jvalue)
{
	return JNINativeSetIntegerIndex(env, obj, metadata_editor_token, jkey, (jint)0, jvalue);
}

static jint JNICALL JNINativeAddInteger(JNIEnv *env, jobject obj, jint metadata_editor_token, jint jkey, jint jvalue)
{
	// TODO: Add another value into the metadata field for another index, or index 0 if it is the first value
	return NErr_NotImplemented;
}

static jint JNICALL JNINativeRetain(JNIEnv *env, jobject obj, jint metadata_editor_token)
{
	ifc_metadata_editor *metadata_editor = (ifc_metadata_editor *)metadata_editor_token;
	
	if (metadata_editor)
	{
		metadata_editor->Retain();
		return metadata_editor_token;
	}
	return NErr_NullPointer;
}

static jint JNICALL JNINativeRelease(JNIEnv *env, jobject obj, jint metadata_editor_token)
{
	ifc_metadata_editor *metadata_editor = (ifc_metadata_editor *)metadata_editor_token;
	
	if (metadata_editor)
	{
		assert(metadata_editor);
		int ret = metadata_editor->Release();
		if (ret == 0)
		{
			//__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadata] JNINativeRelease: Metadata object '%x' successfully released.", metadata);
		}
		else
			__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditor] JNINativeRelease: Metadata Editor object '%x' released, but '%d' references still remain.", metadata_editor, ret);
		
		return (jint)NErr_Success;
	}
	return 0;
}

static int GetMetadataEditor(nx_uri_t filename, ifc_metadata_editor **out_metadata_editor)
{
	ifc_metadata_editor *metadata_editor=0;
	GUID metadata_guid = svc_metadata::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(metadata_guid);
	for (size_t i=0; i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(metadata_guid,i);
		if (sf)
		{	
			svc_metadata * l = (svc_metadata*)sf->GetInterface();
			if (l)
			{
				int ret = l->CreateMetadataEditor(filename, &metadata_editor);
				l->Release();
				if (ret == NErr_Success)
				{
					*out_metadata_editor = metadata_editor;
					return NErr_Success;
				}
				else if (ret != NErr_False)
					return ret;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

static jobject JNICALL JNINativeOpenEditor(JNIEnv *env, jclass cls, jstring jfilename)
{
	int ret;
	nx_uri_t filename;
	ret = NXURICreateWithJString(env, jfilename, &filename);
	if (ret == NErr_Success)
	{
		ReferenceCountedPointer<ifc_metadata_editor> metadata_editor;
		ret = GetMetadataEditor(filename, &metadata_editor);
		if (ret == NErr_Success)
			return JNIMetadataEditorCreate(env, metadata_editor);
		
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditor] Couldnt create metadata_editor from filename '%s'", filename->string);

		return NULL;
	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIMetadataEditor] Couldnt create nx_uri_t from java filename");
		JNIThrowExceptionForNError(env, ret);
		return NULL;
	}
}

static jint JNICALL JNINativeSaveEditor(JNIEnv *env, jobject obj, jint metadata_editor_token)
{
	ifc_metadata_editor *metadata_editor = (ifc_metadata_editor *)metadata_editor_token;

	if (metadata_editor)
	{
		metadata_editor->Save();
	}
}

//////////////////////////////////////////////////////////////////////////
// JNIMetadata object methods
//////////////////////////////////////////////////////////////////////////
JNINativeMethod JNIMetadataEditor::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeSetField", "(IILjava/lang/String;)I", (void *) JNINativeSetField },
	{ "nativeSetFieldIndex", "(IIILjava/lang/String;)I", (void *) JNINativeSetFieldIndex },
	{ "nativeSetInteger", "(III)I", (void *) JNINativeSetInteger },
	{ "nativeSetIntegerIndex", "(IIII)I", (void *) JNINativeSetIntegerIndex },
	//{ "nativeSetMetadata", "(III)Ljava/lang/Object;", (void *) JNINativeSetMetadata },
	//{ "nativeSetAlbumArtData", "(II)Ljava/lang/Object;", (void *) JNINativeSetAlbumArtData },
	{ "nativeOpenEditor", "(Ljava/lang/String;)Ljava/lang/Object;", (void *) JNINativeOpenEditor },
	{ "nativeSaveEditor", "(I)I", (void *) JNINativeSaveEditor },
	{ "nativeRetain", "(I)I", (void *) JNINativeRetain },
	{ "nativeRelease", "(I)I", (void *) JNINativeRelease },
};
size_t JNIMetadataEditor::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIMetadataEditor::jni_classname = "com/nullsoft/replicant/MetadataEditor";