#pragma once
#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "nx/nxuri.h"
#include "ifc_playlistloader.h"

// {94D27FCA-0AE2-41FA-9500-5422D3C10332}
static const GUID playlist_handler_service_type_guid = 
{ 0x94d27fca, 0xae2, 0x41fa, { 0x95, 0x0, 0x54, 0x22, 0xd3, 0xc1, 0x3, 0x32 } };

class svc_playlisthandler : public Wasabi2::Dispatchable
{
protected:
	svc_playlisthandler() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_playlisthandler() {}

public:
	static GUID GetServiceType() { return playlist_handler_service_type_guid; }

	 // returns 0 when it's done
	nx_string_t EnumerateExtensions(size_t n) { return PlaylistHandler_EnumerateExtensions(n); }

	nx_string_t EnumerateMIMETypes(size_t n); // returns 0 when it's done, returns char * to match HTTP specs
	nx_string_t GetName();  // returns a name suitable for display to user of this playlist form (e.g. PLS Playlist)

	// returns NErr_True and NErr_False, so be careful ...
	int SupportedFilename(nx_uri_t filename) { return PlaylistHandler_SupportedFilename(filename); }

	int SupportedMIMEType(nx_string_t filename); // returns NErr_True and NErr_False, so be careful ...
	/* the actual filename gets passed to ifc_playlistloader::Load().  This filename *can and should& be the real filename, but gets used only to determine which loader to use 
	   this can be useful, e.g., if you have binary resource data and just need to get a specific playlist loader.  The file extension should always be correct, though! */
	int CreateLoader(nx_uri_t filename, ifc_playlistloader **loader) { return PlaylistHandler_CreateLoader(filename, loader); }
	
	int HasWriter(); // returns NErr_True if writing is supported
	//ifc_playlistwriter CreateWriter(const wchar_t *writer);
	//void ReleaseWriter(ifc_playlistwriter *writer);

	size_t SniffSizeRequired(); // return number of bytes required for detection on an unknown file
	int IsOurs(const int8_t *data, size_t sizeBytes); // returns NErr_True and NErr_False, so be careful ...

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual nx_string_t WASABICALL PlaylistHandler_EnumerateExtensions(size_t n)=0;
	virtual int WASABICALL PlaylistHandler_SupportedFilename(nx_uri_t filename)=0;
	virtual int WASABICALL PlaylistHandler_CreateLoader(nx_uri_t filename, ifc_playlistloader **loader)=0;
};

