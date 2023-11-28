#pragma once
#include "foundation/types.h"
#ifdef __cplusplus
extern "C" {
#endif
	
/* ConverterGain */
void armv5_vfp_nsmath_pcm_Convert_F32_S16(int16_t *destination, const float *source, size_t sample_count, float gain);
void armv5_vfp_nsmath_pcm_Convert_F32_S24(int16_t *destination, const float *source, size_t sample_count, float gain);

/* Interleaver */
void armv5_vfp_nsmath_pcm_Convert_S32_S16_stereo(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_mono(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel);
void armv5_vfp_nsmath_pcm_Convert_S32_S16(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel);
void armv5_vfp_nsmath_pcm_Convert_S32_S24(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel);

/* InterleaverGain */
void armv5_vfp_nsmath_pcm_Convert_S32_S16_gain_stereo(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_gain_mono(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_gain(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);
void armv5_vfp_nsmath_pcm_Convert_S32_S24_gain(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);

void armv5_vfp_nsmath_pcm_Convert_S32_F32_stereo(float *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);
void armv5_vfp_nsmath_pcm_Convert_S32_F32_mono(float *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);
void armv5_vfp_nsmath_pcm_Convert_S32_F32(float *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain);

/* InterleaverDecimate */
void armv5_vfp_nsmath_pcm_Convert_S32_S16_stereo_decimate8(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_stereo_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_shift8(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);

void armv5_vfp_nsmath_pcm_Convert_S32_S16_mono_decimate8(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_mono_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);
void armv5_vfp_nsmath_pcm_Convert_S32_S16_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);
void armv5_vfp_nsmath_pcm_Convert_S32_S24_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits);

/* InterleaverPad */
void armv5_vfp_nsmath_pcm_Convert_S32_S24_pad(uint8_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int padding_bits);

#ifdef __cplusplus
}
#endif
