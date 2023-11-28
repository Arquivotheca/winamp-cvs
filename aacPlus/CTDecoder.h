#ifndef NULLSOFT_IN_MP4_CT_DECODER_H
#define NULLSOFT_IN_MP4_CT_DECODER_H

#include "../in_mp4/mpeg4audio.h"
#include "aacplusdectypes.h"

class CTDecoder : public MP4AudioDecoder
{
public:
	CTDecoder();
	int Open();
	int OpenEx(size_t bits, size_t maxChannels, bool useFloat);
	void Close();
	int GetCurrentBitrate(unsigned int *bitrate);
	int AudioSpecificConfiguration(void *buffer, size_t buffer_size); // reads ASC block from mp4 file
	void Flush();
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample);
	int GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *useFloat);
	int DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes); 
	int OutputFrameSize(size_t *frameSize);
	int CanHandleCodec(const char *codecName);
	int CanHandleType(uint8_t type);
	int CanHandleMPEG4Type(uint8_t type);
	int SetGain(float _gain) { gain=_gain; return MP4_SUCCESS; }
private:
	HANDLE_AACPLUSDEC_DECODER handle;
	AACPLUSDEC_STREAMPROPERTIES *streamProperties; 
	size_t preDelay;
	unsigned int bitsPerSample;
	size_t maxChannels;
	bool isFloat;
	float gain;
protected:
	RECVS_DISPATCH;
};
#endif