#pragma once
#include "foundation/dispatch.h"

// {E7D03E08-E337-4E97-BB22-A690EB8B10D2}
static const GUID audiotrackpro_settings_interface_guid = 
{ 0xe7d03e08, 0xe337, 0x4e97, { 0xbb, 0x22, 0xa6, 0x90, 0xeb, 0x8b, 0x10, 0xd2 } };


class ifc_audiotrackpro_settings : public Wasabi2::Dispatchable
{
protected:
	ifc_audiotrackpro_settings() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_audiotrackpro_settings() {}
public:
	static GUID GetInterfaceGUID() { return audiotrackpro_settings_interface_guid; }

	/* returns NErr_True or NErr_False */
	int GetCrossfadeStatus() { return AudioTrackProSettings_GetCrossfadeStatus(); }
	int GetCrossfadeTime(double *seconds) { return AudioTrackProSettings_GetCrossfadeTime(seconds); }
private:
	virtual int WASABICALL AudioTrackProSettings_GetCrossfadeStatus()=0;
	virtual int WASABICALL AudioTrackProSettings_GetCrossfadeTime(double *seconds)=0;

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};