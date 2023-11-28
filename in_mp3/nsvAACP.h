#include "../nsv/dec_if.h"
#include "../aacPlus/api_aacplusdecoder.h"

class EAACP_Decoder : public IAudioDecoder
{
public:
	EAACP_Decoder();
	~EAACP_Decoder();
	int decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8]);
	void flush();
	bool OK();

private:
	void FillOutputFormat(unsigned int out_fmt[8]);
	void CopyToOutput(void *out, int *out_len);

private:
	api_aacplusdecoder *aacPlus;
	
	AACPLUSDEC_BITSTREAMBUFFERINFO bitbufInfo;
	AACPLUSDEC_AUDIOBUFFERINFO audiobufInfo;

	unsigned char pcm_buf[65536*2];

	int pcm_buf_used;

	int readpos;

	unsigned int cbvalid;

};
