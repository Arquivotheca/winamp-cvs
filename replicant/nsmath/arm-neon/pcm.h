#pragma once
#ifdef __cplusplus
extern "C" {
#endif
	
	void nsmath_pcm_Convert_F32_S16_NEON(int16_t *destination, const float *source, size_t sample_count);
	void nsmath_pcm_Convert_F32_S16_x4_NEON(int16_t *destination, const float *source, size_t sample_count);
	void nsmath_pcm_Convert_F32_S16_x32_NEON(int16_t *destination, const float *source, size_t sample_count);
	
#ifdef __cplusplus
}
#endif
