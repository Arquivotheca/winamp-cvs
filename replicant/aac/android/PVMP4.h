#pragma once

#include "IAACDecoder.h"
#include <pvmp4audiodecoder_api.h>


int AACDecoderLibraryInit();

class PVAACDecoder : public IAACDecoder
{
public:
	PVAACDecoder();
	~PVAACDecoder();
	int Initialize(int desired_channels=2);

	/* IAACDecoder implementation */
	int Decode(const void *input_buffer, size_t input_buffer_length, int16_t *output_buffer, size_t *samples_decoded);
	int Configure(const void *asc, size_t asc_length);
	void Reset();
	int FillParameters(nsaudio::Parameters *parameters);

private:
	tPVMP4AudioDecoderExternal decoder;
	void *decoder_memory;
	size_t decoder_memory_size;
	void *decoder_config;
	size_t decoder_config_size;
};

typedef uint32_t (*GETMEMFUNC)(void);
typedef int (*AACIPPFUNC)(void *, void *);
typedef void (*AACPFUNC)(void *);

extern GETMEMFUNC __AACDecoderGetMemoryRequirements;
extern AACIPPFUNC __AACDecoderInit, __AACDecoderDecode, __AACDecoderConfig;
extern AACPFUNC __AACDecoderReset;

