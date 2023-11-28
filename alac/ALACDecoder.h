/* copyright 2006 Ben Allison */
#ifndef NULLSOFT_ALAC_DECODER_H
#define NULLSOFT_ALAC_DECODER_H

#include "../in_mp4/mpeg4audio.h"
#include "decomp.h"

class ALACDecoder : public MP4AudioDecoder
{
public:
	ALACDecoder() : alac(0)
	{
	}
	int OpenMP4(MP4FileHandle mp4_file, MP4TrackId mp4_track, size_t output_bits, size_t maxChannels, bool useFloat);
	int GetCurrentBitrate(unsigned int *bitrate);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	void Flush();
	void Close();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int OutputFrameSize(size_t *frameSize);
	const char *GetCodecInfoString();
	int CanHandleCodec(const char *codecName);
	int SetGain(float gain);
private:
	alac_file *alac;
	int channels;
	int bps;
	int output_bits;
	bool use_rg;
	float rg;
protected:
	RECVS_DISPATCH;
};
#endif