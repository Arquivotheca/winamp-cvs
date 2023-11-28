#pragma once

#include "foundation/types.h"
#include "audio/parameters.h"

class IAACDecoder
{
public:
	virtual ~IAACDecoder(){}
	virtual int Decode(const void *input_buffer, size_t input_buffer_length, int16_t *output_buffer, size_t *samples_decoded)=0;
	virtual int Configure(const void *asc, size_t asc_length)=0;
	virtual void Reset()=0;
	virtual int FillParameters(nsaudio::Parameters *parameters)=0;
};

enum
{
	NO_AAC,
	FDK_AAC,
	PV_AAC,
};

enum
{
	TYPE_ADTS_AAC,
	TYPE_RAW_AAC,
};

extern int aaclib_kind;

int AACDecoder_Create(IAACDecoder **decoder, int transport);