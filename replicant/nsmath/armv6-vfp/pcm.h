#pragma once
#ifdef __cplusplus
extern "C" {
#endif
	
	void nsmath_pcm_Convert_F32_S16_armv6_VFP(int16_t *destination, const float *source, size_t sample_count);
	
#ifdef __cplusplus
}
#endif
