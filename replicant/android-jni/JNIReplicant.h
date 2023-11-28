#pragma once
#include <jni.h>

class JNIReplicant
{
public:	
	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
};