#pragma once
#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void Float32_To_Int16_Clip(int16_t *destination_buffer, const float *source_buffer, size_t count);

void Int16_To_Float32(float *destination_buffer, const int16_t *source_buffer, size_t count, float gain);
void Int32_To_Float32(float *destination_buffer, const int32_t *source_buffer, size_t count, float gain);
void Float32_To_Float32(float *destination_buffer, const float *source_buffer, size_t count, float gain);
void Float64_To_Float32(float *destination_buffer, const double *source_buffer, size_t count, float gain);

void Int32_To_Float32_Deinterleave(float *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain);
void Int16_To_Float32_Deinterleave(float *destination_buffer, const int16_t **source_buffer, size_t channels, size_t count_per_channel, float gain);
void Float32_To_Float32_Deinterleave(float *destination_buffer, const float **source_buffer, size_t channels, size_t count_per_channel, float gain);
void Float64_To_Float32_Deinterleave(float *destination_buffer, const double **source_buffer, size_t channels, size_t count_per_channel, float gain);

typedef void (*ConverterFunc)(float *destination_buffer, const void *source_buffer, size_t count, float gain);
typedef void (*DeinterleaverFunc)(float *destination_buffer, const void **source_buffer, size_t channels, size_t count_per_channel, float gain);
#ifdef __cplusplus
}
#endif