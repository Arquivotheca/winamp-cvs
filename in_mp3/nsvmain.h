#pragma once
#include "../mp3/mp3dec/mpgadecoder.h"
#include "../nsv/dec_if.h"

class MP3_Decoder : public IAudioDecoder
{
public:
	MP3_Decoder() : mp3_dec(0, 0, 0) { fused = 0; pcm_buf_used = 0; }
	~MP3_Decoder() { };
	int decode(void *in, int in_len,
	           void *out, int *out_len,
	           unsigned int out_fmt[8]);
	void flush() { fused = 0; pcm_buf_used = 0; mp3_dec.Reset(); }
private:
	CMpgaDecoder mp3_dec;
	float pcm_buf[1152*2*2];
	int pcm_buf_used;
	int pcm_offs;
	int fused;
};
