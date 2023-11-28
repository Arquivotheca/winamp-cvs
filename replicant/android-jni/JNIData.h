#pragma once
#include <jni.h>
#include "nx/nxdata.h"

jobject JNIDataCreate(JNIEnv *env, nx_data_t data);

class JNIData
{
public:	
	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
};