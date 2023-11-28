#pragma once
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

	void JNIThrowExceptionForNError(JNIEnv * env, int ret);
	void JNIThrowException(JNIEnv * env, const char *exception_class);
	JNIEnv *JNIGetThreadEnvironment();

#ifdef __cplusplus
}
#endif