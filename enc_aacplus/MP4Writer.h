#ifndef NULLSOFT_ENC_AACPLUS_MP4WRITERH
#define NULLSOFT_ENC_AACPLUS_MP4WRITERH

#include "main.h"
#include <mp4.h>

class MP4Writer
{
public:
	MP4Writer();
	~MP4Writer();
	
	void AddAudioTrack(aacPlusEncOutputFormat *format, bool backwardsCompatible);
	void WriteGaps(unsigned int pregap, unsigned int postgap, unsigned int totalSamples);
	void WriteASC(void *buf, size_t size);
	void Write(void *buf, size_t size, MP4Duration duration);
	void CloseTo(const wchar_t *filename);

	bool OK() { return true; }

	MP4TrackId mp4Track;
	MP4FileHandle mp4File;
	wchar_t tempfile[MAX_PATH];
};

#endif