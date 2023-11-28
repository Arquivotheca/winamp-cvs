#include "api.h"
#include "MD5.h"
#include "android-jni.h"
#include "JNIData.h"
#include "foundation/export.h"
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nx.h"
#include <android/log.h>
#include <assert.h>
#include <unistd.h>

extern JavaVM *g_jvm;

// JNI Class reference cache globals
static jclass javaDataClass;
static jmethodID javaDataClassConstructor;
static jfieldID javaDataClassDataToken;
static jfieldID javaDataClassSize;

jobject JNIDataCreate(JNIEnv *env, nx_data_t data)
{
	assert(data);
	
	if(!javaDataClass)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIDataCreate] Didn't have a clazz = %x", javaDataClass);
		return 0;
	}

	if(!javaDataClassConstructor) 
		return 0;

	jobject obj = env->NewObject(javaDataClass, javaDataClassConstructor);
	if(!obj)
	{
		__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIDataCreate] data obj = %x", obj);		
		return 0;
	}	

	nx_data_t retained_data = NXDataRetain(data);
	env->SetIntField(obj, javaDataClassDataToken, (jint)retained_data);
	env->SetIntField(obj, javaDataClassSize, (jint)NXDataSize(retained_data));

	//__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIDataCreate] data Token = '%x'", (jint)retained_data);

	return obj;
}

static void JNICALL JNINativeClassInit(JNIEnv *env, jclass cls)
{
	// Cache the data java class and its members
	javaDataClass = (jclass)env->NewGlobalRef(cls);
	javaDataClassConstructor = env->GetMethodID(javaDataClass, "<init>", "()V");
	javaDataClassDataToken = env->GetFieldID(javaDataClass, "dataToken", "I");
	javaDataClassSize = env->GetFieldID(javaDataClass, "dataSize", "I");
	__android_log_print(ANDROID_LOG_INFO,"libreplicant","[JNIData] JNINativeClassInit complete.");
}

static void JNICALL JNINativeRelease(JNIEnv *env, jobject obj, jint data_token)
{
	nx_data_t data = (nx_data_t)data_token;
	assert(data);
	NXDataRelease(data);
}

static jint JNICALL JNIRead(JNIEnv *env, jobject obj, jbyteArray output, jint data_token, jint offset, jint position, jint length)
{
	nx_data_t data = (nx_data_t)data_token;
	assert(data);
	const uint8_t *pointer = (const uint8_t *)NXDataPointer(data);
	env->SetByteArrayRegion(output, offset, length, (jbyte *)pointer + position);
	return length;
}

static jint JNICALL JNIReadByte(JNIEnv *env, jobject obj, jint data_token, jint position)
{
	nx_data_t data = (nx_data_t)data_token;
	const uint8_t *pointer = (const uint8_t *)NXDataPointer(data);
	return (jint)pointer[position];
}

static jstring JNICALL JNIGetHash(JNIEnv *env, jobject obj, jint data_token)
{
	nx_data_t data = (nx_data_t)data_token;
	if (data)
	{
		const void *bytes;
		size_t length;
		int ret = NXDataGet(data, &bytes, &length);
		if (ret == NErr_Empty)
		{
			uint8_t md5_hash[16];
			MD5_CTX md5;
			MD5Init(&md5);
			MD5Final(md5_hash, &md5);

			char md5_string[33];
			sprintf(md5_string, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
				md5_hash[0], md5_hash[1], md5_hash[2], md5_hash[3], 
				md5_hash[4], md5_hash[5], md5_hash[6], md5_hash[7], 
				md5_hash[8], md5_hash[9], md5_hash[10], md5_hash[11], 
				md5_hash[12], md5_hash[13], md5_hash[14], md5_hash[15]);

			nx_string_t md5_nx;
			if (NXStringCreateWithUTF8(&md5_nx, md5_string) == NErr_Success)
			{
				jstring jvalue;
				if (NXStringCreateJString(env, md5_nx, &jvalue) != NErr_Success)
					jvalue=0;
				NXStringRelease(md5_nx);
				return jvalue;
			}
		}
		else if (ret == NErr_Success)
		{
			uint8_t md5_hash[16];
			MD5_CTX md5;
			MD5Init(&md5);
			MD5Update(&md5, (const uint8_t *)bytes, length);
			MD5Final(md5_hash, &md5);

			char md5_string[33];
			sprintf(md5_string, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
				md5_hash[0], md5_hash[1], md5_hash[2], md5_hash[3], 
				md5_hash[4], md5_hash[5], md5_hash[6], md5_hash[7], 
				md5_hash[8], md5_hash[9], md5_hash[10], md5_hash[11], 
				md5_hash[12], md5_hash[13], md5_hash[14], md5_hash[15]);

			nx_string_t md5_nx;
			if (NXStringCreateWithUTF8(&md5_nx, md5_string) == NErr_Success)
			{
				jstring jvalue;
				if (NXStringCreateJString(env, md5_nx, &jvalue) != NErr_Success)
					jvalue=0;
				NXStringRelease(md5_nx);
				return jvalue;
			}
		}
	}
	return (jstring)0;
}

static jstring JNICALL JNIGetMIME(JNIEnv *env, jobject obj, jint data_token)
{
	nx_data_t data = (nx_data_t)data_token;
	if (data)
	{
		ReferenceCountedNXString mime_type;
		if (NXDataGetMIME(data, &mime_type) == NErr_Success)
		{
			jstring jvalue;
			if (NXStringCreateJString(env, mime_type, &jvalue) != NErr_Success)
				jvalue=0;
			return jvalue;
		}
	}
	return (jstring)0;
}

static jstring JNICALL JNIGetSourceURI(JNIEnv *env, jobject obj, jint data_token)
{
	nx_data_t data = (nx_data_t)data_token;
	if (data)
	{
		ReferenceCountedNXURI source_uri;
		if (NXDataGetSourceURI(data, &source_uri) == NErr_Success)
		{
			ReferenceCountedNXString nx_source;
			if (NXURIGetNXString(&nx_source, source_uri) == NErr_Success)
			{
				jstring jvalue;
				if (NXStringCreateJString(env, nx_source, &jvalue) != NErr_Success)
					jvalue=0;
				return jvalue;
			}
		}
	}
	return (jstring)0;
}

static jlong JNICALL JNIGetSourceModifiedTime(JNIEnv *env, jobject obj, jint data_token)
{
	nx_data_t data = (nx_data_t)data_token;
	if (data)
	{
		nx_file_stat_t file_stats;
		if (NXDataGetSourceStat(data, &file_stats) == NErr_Success)
		{
			return (jlong)file_stats->modified_time;
		}
	}
	return (jlong)0;
}

JNINativeMethod JNIData::jni_methods[] = {
	{ "nativeClassInit", "()V", (void *) JNINativeClassInit },
	{ "nativeRelease", "(I)V", (void *) JNINativeRelease },
	{ "nativeRead", "([BIIII)I", (void *) JNIRead },
	{ "nativeReadByte", "(II)I", (void *) JNIReadByte },
	{ "nativeGetHash", "(I)Ljava/lang/String;", (void *) JNIGetHash },
	{ "nativeGetMIME", "(I)Ljava/lang/String;", (void *) JNIGetMIME },
	{ "nativeGetSourceURI", "(I)Ljava/lang/String;", (void *) JNIGetSourceURI },
	{ "nativeGetSourceModifiedTime", "(I)J", (void *) JNIGetSourceModifiedTime },

};
size_t JNIData::jni_methods_count = sizeof(jni_methods) / sizeof(jni_methods[0]);
const char * JNIData::jni_classname = "com/nullsoft/replicant/Data";