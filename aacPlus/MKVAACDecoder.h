#pragma once
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include "aacplusdectypes.h"
#include "../nsmkv/Tracks.h"

class MKVAACDecoder : public ifc_mkvaudiodecoder
{
public:
	static MKVAACDecoder *Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_mkvaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();

private:
	MKVAACDecoder(HANDLE_AACPLUSDEC_DECODER handle, AACPLUSDEC_STREAMPROPERTIES *streamProperties, unsigned int bps, bool floating_point);
	
	/* internal implementation */

	/* data */
	HANDLE_AACPLUSDEC_DECODER handle;
	AACPLUSDEC_STREAMPROPERTIES *streamProperties;
	unsigned int bps;
	bool floating_point;
	size_t preDelay;

};