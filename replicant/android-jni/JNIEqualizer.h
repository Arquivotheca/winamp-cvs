#pragma once
#include <jni.h>
#include <stddef.h>


class JNIEqualizer
{
public:
	JNIEqualizer(JNIEnv *env, jobject obj);

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
private:

};
