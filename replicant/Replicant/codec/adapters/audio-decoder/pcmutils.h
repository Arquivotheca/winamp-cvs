#pragma once
#include "foundation/types.h"
#include "nsmath/pcm.h"
/* TODO: move to nsmath */

#ifdef __cplusplus
extern "C" {
#endif

	/* ConverterGain */
void Codec_Float32_To_Int16_Clip(int16_t *destination_buffer, const float *source_buffer, size_t count, float gain);
void Codec_Int16_To_Float32(float *destination_buffer, const int16_t *source_buffer, size_t count, float gain);

/* Interleaver */
void Codec_Int32_To_Int16_Interleave(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel);

/* InterleaverGain */
void Codec_Int32_To_Int16_Interleave_Gain(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain);
void Codec_Int32_To_Float32_Interleave(float *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain);
void Codec_Int16_To_Float32_Interleave(float *destination_buffer, const int16_t **source_buffer, size_t channels, size_t count_per_channel, float gain);

/* InterleaverPad */
void Codec_Int32_To_Int16_Interleave_Pad(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits);

/* InterleaverDecimate */
void Codec_Int32_To_Int16_Interleave_Decimate(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits);

#ifdef __cplusplus
}
#endif