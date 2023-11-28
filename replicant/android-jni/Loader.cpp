#include <jni.h>
#include "nx/cpufeatures/cpu-features.h"

extern "C" jstring JNICALL Java_com_nullsoft_replicant_Loader_getPlatformName(JNIEnv * env, jclass cls)
{
	AndroidCpuFamily cpu_family = android_getCpuFamily();

	if (cpu_family == ANDROID_CPU_FAMILY_ARM)
	{
		uint64_t features = android_getCpuFeatures();
		if (features & ANDROID_CPU_ARM_FEATURE_ARMv7)
		{
			return env->NewStringUTF("ARMv7");
		}
		/* TODO
		else if (features & ANDROID_CPU_ARM_FEATURE_ARMv6)
		{
			return env->NewStringUTF("ARMv5");
		}
		*/
		else
		{
			// assume ARMv5
			return env->NewStringUTF("ARMv5");
		}
	}
	else if (cpu_family == ANDROID_CPU_FAMILY_X86)
	{
		return env->NewStringUTF("x86");
	}
	
	return (jstring)0;
}
