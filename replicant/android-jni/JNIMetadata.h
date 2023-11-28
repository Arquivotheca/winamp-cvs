#pragma once
#include <jni.h>
#include "metadata/ifc_metadata.h"

jobject JNIMetadataCreate(JNIEnv *env, ifc_metadata *metadata);

class JNIMetadata
{
public:	
	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
};