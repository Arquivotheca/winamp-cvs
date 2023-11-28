#pragma once
#include <jni.h>
#include "metadata/ifc_metadata_editor.h"

jobject JNIMetadataEditorCreate(JNIEnv *env, ifc_metadata_editor *metadata);

class JNIMetadataEditor
{
public:	
	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
};