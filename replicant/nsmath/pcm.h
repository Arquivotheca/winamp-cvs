#pragma once
#include "foundation/types.h"
#ifdef __cplusplus
#include "audio/parameters.h"
#endif

#ifdef _WIN32
#ifdef NSMATH_EXPORTS
#define NSMATH_API __declspec(dllexport)
#else
#define NSMATH_API __declspec(dllimport)
#endif
#else
#define NSMATH_API __attribute__ ((visibility("default")))
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* ConverterGain converts a single interleaved stream from one type to another, without applyin gain */
typedef void (*Converter)(void *destination, const void *source, size_t sample_count);
/* ConverterGain converts a single interleaved stream from one type to another, with floating point gain applied */
typedef void (*ConverterGain)(void *destination, const void *source, size_t sample_count, float gain);
/* ConverterDecimate converts a single interleaved stream from one type to another, applying decimation (right-shift) or padding (left-shift) */
typedef void (*ConverterShift)(void *destination, const void *source, size_t sample_count, unsigned int decimation_bits);

/* Interleaver converts a group of non-interleaved streams from one type to another, without applying gain */
typedef void (*Interleaver)(void *destination, const void **source, size_t channels, size_t sample_count);
/* InterleaverGain converts a group of non-interleaved streams from one type to another, applying floating point gain */
typedef void (*InterleaverGain)(void *destination, const void **source, size_t channels, size_t sample_count, float gain);
/* InterleaverDecimate converts a group of non-interleaved streams from one type to another, applying decimation (right-shift) or padding (left-shift) */
typedef void (*InterleaverShift)(void *destination, const void **source, size_t channels, size_t sample_count, unsigned int decimation_bits);


typedef struct nsmath_pcm_interleaver_s
{
	Interleaver interleave;
	InterleaverGain interleave_gain;
	InterleaverShift interleave_shift;
	float gain;
	unsigned int shift_bits;
} nsmath_pcm_interleaver_s, *nsmath_pcm_interleaver_t;

typedef struct nsmath_pcm_converter_s
{
	Converter convert;
	ConverterGain convert_gain;
	ConverterShift convert_shift;
	float gain;
	unsigned int shift_bits;
} nsmath_pcm_converter_s, *nsmath_pcm_converter_t;

#ifdef __cplusplus
/* Although this function takes nsaudio::Parameters, it will not performance sample rate conversion, nor channel conversion (although it will perform channel reassignment WHEN IMPLEMENTED) */
NSMATH_API int nsmath_pcm_CreateInterleaver(nsmath_pcm_interleaver_t interleaver, const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters, double optional_gain); 
NSMATH_API int nsmath_pcm_CreateConverter(nsmath_pcm_converter_t converter, const nsaudio::Parameters *source_parameters, const nsaudio::Parameters *destination_parameters, double optional_gain); 
#endif

#ifdef __cplusplus
}
#endif
