#pragma once
#include <jni.h>
#include <stddef.h>

/* this exists mainly to get ensure a class reference to IPlaylistLoader */
class JNIIPlaylistLoader
{
public:
	JNIIPlaylistLoader(JNIEnv *env, jobject obj);

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
private:
	
};
