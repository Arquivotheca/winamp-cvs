#include "nsmath.h"
#include <cpu-features.h>
#include "nsmath/arm-neon/pcm.h"
#include "nsmath/pcm.h"
#include "nsmath/armv6-vfp/pcm.h"
static int nsmath_initialized;

int nsmath_init()
{
	if (!nsmath_initialized)
	{
		uint64_t features = android_getCpuFeatures();
#ifdef __ARM_ARCH_7A__
		if (features & ANDROID_CPU_ARM_FEATURE_NEON)
		{
			nsmath_pcm_Convert_F32_S16 = nsmath_pcm_Convert_F32_S16_NEON;
			nsmath_pcm_Convert_F32_S16_x4 = nsmath_pcm_Convert_F32_S16_x4_NEON;
			nsmath_initialized=1;
			return 0;
		}
		else 
		{
			/* no NEON, but we can assume VFP */
			nsmath_pcm_Convert_F32_S16 = nsmath_pcm_Convert_F32_S16_armv6_VFP;
			nsmath_pcm_Convert_F32_S16_x4 = nsmath_pcm_Convert_F32_S16_armv6_VFP;
			nsmath_initialized=1;
			return 0;
			
		}
#endif

		nsmath_pcm_Convert_F32_S16 = nsmath_pcm_Convert_F32_S16_C;
		nsmath_pcm_Convert_F32_S16_x4 = nsmath_pcm_Convert_F32_S16_C;

		nsmath_initialized=1;
	}
	return 0;
}
