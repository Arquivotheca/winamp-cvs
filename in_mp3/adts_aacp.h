#if 0
#ifndef NULLSOFT_IN_MP3_ADTS_AACP_H
#define NULLSOFT_IN_MP3_ADTS_AACP_H

#include "../aacPlus/api_aacplusdecoder.h"
#include "adts.h"


class ADTS_AACP : public adts
{
public:
	ADTS_AACP();
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

	AACPLUSDEC_STREAMPROPERTIES *pDesc;
	AACPLUSDEC_EXPERTSETTINGS *pConf;
	AACPLUSDEC_PROGRAMPROPERTIES *checkit;
	AACPLUSDEC_BITSTREAMBUFFERINFO bitbufInfo;
	AACPLUSDEC_AUDIOBUFFERINFO audiobufInfo;
	AACPLUSDEC_DATASTREAMBUFFERINFO datastreamInfo;
	AncillaryData dataStreamBuffer;
	api_aacplusdecoder *decoder;
	unsigned char buffer[8192];
	int readpos;
	size_t inputread;
	int outputFrameSize;
	int bitsPerSample;
	int channels;
	float gain;
	bool allowRG;
	bool useFloat;
	size_t predelay;
};

#endif

#endif