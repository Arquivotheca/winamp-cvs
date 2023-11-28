#ifndef NULLSOFT_IN_MP3_ADTS_MP2_H
#define NULLSOFT_IN_MP3_ADTS_MP2_H

#include "adts.h"
#include "api.h"
#include "config.h"
#ifndef NO_MP3SURROUND
#include "../mp3/bccDecLinklib/include/bccDecLink.h" // Binaural Cue Coding (aka mp3 surround)
#endif

class ADTS_MP2 : public adts
{
public:
	ADTS_MP2();
	int Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool allowRG, bool _useFloat = false);
	bool Open(ifc_mpeg_stream_reader *file);
	void Close();
	void GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate);
	void CalculateFrameSize(int *frameSize);
	void Flush(ifc_mpeg_stream_reader *file);
	size_t GetCurrentBitrate();
	size_t GetDecoderDelay();
	int Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate);
	int Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut);
	int GetLayer();
	void Release();
private:
	CMpgaDecoder *decoder;
	int outputFrameSize;
	size_t bitsPerSample;
	double gain;
	bool allowRG;
	bool useFloat;

	int channels;
	int sampleRate;
	unsigned int decoderDelay;
	unsigned int endcut;

#ifndef NO_MP3SURROUND
	float delayline[1152*2];
	int delaylineSize;
	bool lineFilled;
	SADEC_HANDLE saDecHandle;
	SA_DEC_MODE saMode;
#endif
	
	
};


#endif
