#pragma once
#include "foundation/types.h"
#include "mp3/MPEGHeader.h"
class CVbriHeader
{
public: 
	CVbriHeader();
	~CVbriHeader();

	int readVbriHeader(const MPEGHeader &frame, const uint8_t *Hbuffer, size_t buffer_length);

	uint64_t GetSeekPoint(double percent) const;
	double GetLengthSeconds() const;
	uint64_t GetSamples() const;
	uint32_t GetFrames() const;

	uint64_t seekPointByTime(double EntryTimeInSeconds) const; 
protected:
	int readFromBuffer(const uint8_t *HBuffer, int length);

	int h_id;
	int SampleRate;
	uint32_t VbriStreamBytes;
	uint32_t VbriStreamFrames;
	uint32_t VbriTableSize;
	uint32_t VbriEntryFrames;
	int *VbriTable;
	int encoderDelay;
	int position;
	int samples_per_frame;
};

