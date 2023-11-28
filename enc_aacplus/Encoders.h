#ifndef NULLSOFT_ENC_AACPLUS_ENCODERSH
#define NULLSOFT_ENC_AACPLUS_ENCODERSH


#include "main.h"
#include "MP4Writer.h"
#include "../nsv/enc_if.h"
#include "../aacPlus/aacPlusEnc.h"


void Populate(aacPlusEncOutputFormat *format, AACplusConfig *cfg);

class AudioCoderCommon : public AudioCoder
{
public:
	AudioCoderCommon(int nch, int srate, int bps, bitstreamFormat _bitstream, sbrSignallingMode _signallingMode);
	void PrepareToFinish();
	virtual void Finish(const wchar_t *filename) {}
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
	virtual ~AudioCoderCommon();
	int GetLastError() { return m_err; };
protected:

	void ConfigureBitstream();
	void ConfigureEncoder(bool allowPNS=false);
	int m_err;
	int finished;
	aacPlusEncHandle m_handle;
	bitstreamFormat bitstream;
	sbrSignallingMode signallingMode;
	bool first;
	unsigned char *padding;
	int paddingSize;
	int frameSize;
};

class AudioCoderAACPlus : public AudioCoderCommon
{
public:
	AudioCoderAACPlus(int nch, int srate, int bps, AACplusConfig *cfg);
};

class AudioCoderAAC : public AudioCoderCommon
{
public:
	AudioCoderAAC(int nch, int srate, int bps, AACplusConfig *cfg);

};

class AudioCoderAACPlusHighBitrate : public AudioCoderCommon
{
public:
	AudioCoderAACPlusHighBitrate(int nch, int srate, int bps, AACplusConfig *cfg);
};

class MP4Coder : public AudioCoderCommon
{
public:
	MP4Coder(int nch, int srate, int bps, bitstreamFormat _bitstream, sbrSignallingMode _signallingMode);
	void Start();
	void Finish(const wchar_t *filename);
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
protected:
	MP4Writer mp4Writer;
	unsigned int bytesThisSample;
	int padding, postpadding, totalSamples;
	int framesPerSample;
	bool resampling;
};

class MP4CoderAACPlus : public MP4Coder
{
public:
	MP4CoderAACPlus(int nch, int srate, int bps, AACplusConfig *cfg);
};

class MP4CoderAAC : public MP4Coder
{
public:
	MP4CoderAAC(int nch, int srate, int bps, AACplusConfig *cfg);

};

class MP4CoderAACPlusHighBitrate : public MP4Coder
{
public:
	MP4CoderAACPlusHighBitrate(int nch, int srate, int bps, AACplusConfig *cfg);
};

#endif