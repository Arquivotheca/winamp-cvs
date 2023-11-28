#pragma once
/* copyright 2006 David Hammerton, Ben Allison */

#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct alac_decoder_s *alac_decoder_t;
	typedef int32_t **alac_buffer_t;

	int alac_create(alac_decoder_t *decoder);
	int alac_configure(alac_decoder_t alac, const void *configuration_data, size_t configuration_length);
	int alac_get_information(alac_decoder_t alac, uint32_t *sample_rate, uint8_t *channels, uint8_t *bps);
	int alac_decode(alac_decoder_t alac, const void *inbuffer, size_t inbuffer_length, alac_buffer_t * const outbuffer, size_t *frames);
	
	void alac_destroy(alac_decoder_t alac);

#ifdef __cplusplus
}
#endif



