#ifndef NULLSOFT_ML_RG_PROCESS_H
#define NULLSOFT_ML_RG_PROCESS_H

#include "obj_replaygain.h"

//this class is meant for use as a service

class ProcessReplayGain : public obj_replaygain
{
public:
	ProcessReplayGain() : context(0), albumPeak(0)
	{}
	int Open(int mode);
	int ProcessTrack(const wchar_t *filename);
	int Write();
	void Close();
protected:
	RECVS_DISPATCH;

	void *context;
	int mode;
	float albumPeak;
	RGWorkAlbum queue;
};



#endif