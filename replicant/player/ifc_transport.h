#pragma once
#include "foundation/dispatch.h"

class NOVTABLE ifc_transport : public Wasabi2::Dispatchable
{
protected:
	ifc_transport() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_transport() {}
public:

	int Play() { return Transport_Play(); }
	int Stop() { return Transport_Stop(); }
	int Pause() { return Transport_Pause(); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL Transport_Play()=0;
	virtual int WASABICALL Transport_Stop()=0;
	virtual int WASABICALL Transport_Pause()=0;
};

