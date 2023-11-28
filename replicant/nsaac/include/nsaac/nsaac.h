#pragma once
#include "foundation/types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct nsaac_decoder_object_t *nsaac_decoder_t;
typedef struct nsaac_frameinfo_struct_t
{
	unsigned int channels;
	unsigned int frame_size;
	unsigned int sample_rate;
	unsigned int channel_mode;
	const float *output_data;
} nsaac_frameinfo_value_t, *nsaac_frameinfo_t;

int nsaac_decoder_create(nsaac_decoder_t *decoder, int max_channels);
void nsaac_decoder_destroy(nsaac_decoder_t decoder);
int nsaac_decoder_init_from_adts(nsaac_decoder_t decoder, const void *adts_data, size_t data_size);
int nsaac_decoder_decode_frame(nsaac_decoder_t decoder, nsaac_frameinfo_t frame_info, const void *data, size_t data_size);

#ifdef __cplusplus
}
#endif