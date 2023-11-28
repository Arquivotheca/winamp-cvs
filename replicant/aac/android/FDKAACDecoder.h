#pragma once
#include "fdk-aac.h"
#include "IAACDecoder.h"

extern AACDECODER_CLOSE __aacDecoder_Close;
extern AACDECODER_CONFIGRAW __aacDecoder_ConfigRaw;
extern AACDECODER_DECODEFRAME __aacDecoder_DecodeFrame;
extern AACDECODER_FILL __aacDecoder_Fill;
extern AACDECODER_GETSTREAMINFO __aacDecoder_GetStreamInfo;
extern AACDECODER_OPEN __aacDecoder_Open;
extern AACDECODER_SETPARAM __aacDecoder_SetParam;

class FDKAACDecoder : public IAACDecoder
{
public:
	FDKAACDecoder();
	~FDKAACDecoder();
	int Initialize(int transport);
	
	int Decode(const void *input_buffer, size_t input_buffer_length, int16_t *output_buffer, size_t *samples_decoded);
	int Configure(const void *asc, size_t asc_length);
	void Reset();
	int FillParameters(nsaudio::Parameters *parameters);

private:
	HANDLE_FDKAACDECODER decoder;
	FDK_CStreamInfo *stream_info;
	bool need_reset;
};
