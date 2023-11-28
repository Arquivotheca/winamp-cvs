#pragma once
#include "foundation/dispatch.h"
#include "ifc_gracenote_results.h"
#include "ifc_gracenote_callback.h"
#include "ifc_gracenote_autotag_callback.h"

enum
{
	AUTOTAG_QUERY_FLAG_DEFAULT = 0x00000000,
	AUTOTAG_QUERY_FLAG_RETURN_ALL = 0x00000010,
	AUTOTAG_QUERY_FLAG_RETURN_SINGLE = 0x00000001,
	AUTOTAG_QUERY_FLAG_NO_THREADS = 0x00000002,
};

class ifc_gracenote_autotag_album : public Wasabi2::Dispatchable
{
protected:
	ifc_gracenote_autotag_album() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_gracenote_autotag_album() {}
public:
	int Add(nx_uri_t filename) { return AutoTag_Album_Add(filename); }
	/* Run is synchronous!  All callbacks will be made inside Run()
	and when it returns, all lookup operations have completed */
	int Run(int query_flag) { return AutoTag_Album_Run(query_flag); }
	/* you can call Save() either inside a callback or after Run() has completed */
	int SaveAll(ifc_gracenote_results *results, int flags) { return AutoTag_Album_SaveAll(results, flags); }
	int SaveTrack(ifc_gracenote_results *results, int flags) { return AutoTag_Album_SaveTrack(results, flags); }
	int AddSimple(nx_string_t artist, nx_string_t title) { return AutoTag_Album_AddSimple(artist, title); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL AutoTag_Album_Add(nx_uri_t filename)=0;
	virtual int WASABICALL AutoTag_Album_Run(int query_flag)=0;
	virtual int WASABICALL AutoTag_Album_SaveAll(ifc_gracenote_results *results, int flags)=0;
	virtual int WASABICALL AutoTag_Album_SaveTrack(ifc_gracenote_results *results, int flags)=0;
	virtual int WASABICALL AutoTag_Album_AddSimple(nx_string_t artist, nx_string_t title)=0;
};