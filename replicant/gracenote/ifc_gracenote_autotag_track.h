#pragma once
#include "foundation/dispatch.h"
#include "ifc_gracenote_results.h"
#include "ifc_gracenote_callback.h"
#include "ifc_gracenote_autotag_callback.h"

class ifc_gracenote_autotag_track : public Wasabi2::Dispatchable
{
protected:
	ifc_gracenote_autotag_track() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_gracenote_autotag_track() {}
public:
	/* Run is synchronous!  All callbacks will be made inside Run()
	and when it returns, all lookup operations have completed */
	int Run(nx_uri_t filename) { return AutoTag_Track_Run(filename); }
	/* you can call Save() either inside a callback or after Run() has completed */
	int Save(ifc_gracenote_results *results, int flags) { return AutoTag_Track_Save(results, flags); }

	int Run_Simple(nx_string_t artist, nx_string_t title) { return AutoTag_Track_Run_Simple(artist, title); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AutoTag_Track_Run(nx_uri_t filename)=0;
	virtual int WASABICALL AutoTag_Track_Save(ifc_gracenote_results *results, int flags)=0;
	virtual int WASABICALL AutoTag_Track_Run_Simple(nx_string_t artist, nx_string_t title)=0;
};