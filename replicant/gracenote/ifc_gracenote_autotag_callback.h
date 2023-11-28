#pragma once

#include "GracenoteSysCallback.h"
#include "ifc_gracenote_results.h"
#include "foundation/dispatch.h"
/* callback interface.  return anything but NErr_Success to abort the auto-tag */
class ifc_gracenote_autotag_callback : public Wasabi2::Dispatchable
{
protected:
		ifc_gracenote_autotag_callback() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_gracenote_autotag_callback() {}

public:
	enum
	{
		STATUS_AUTOTAG_INITIALIZING = 0x1000,

		STATUS_AUTOTAG_FILE_ADDING=0x1010,
		STATUS_AUTOTAG_FILE_READING=0x1011,
		STATUS_AUTOTAG_FILE_ANALYZING=0x1012,
		STATUS_AUTOTAG_FILE_DONE=0x101F,

		STATUS_AUTOTAG_QUERYING=0x1020,
		STATUS_AUTOTAG_DONE = 0x1FFF,
	};
	int OnStatus(nx_uri_t filename, int status) { return AutoTagCallback_OnStatus(filename, status); }
	int OnResults(ifc_gracenote_results *results) { return AutoTagCallback_OnResults(results); }
		enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AutoTagCallback_OnStatus(nx_uri_t filename, int status)=0;
	virtual int WASABICALL AutoTagCallback_OnResults(ifc_gracenote_results *results)=0;	
};