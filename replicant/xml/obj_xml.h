#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "foundation/error.h"
#include "nx/nxstring.h"
#include "service/types.h"

class ifc_xmlcallback;

// {3DB2A390-BE91-41f3-BEC6-B736EC7792CA}
static const GUID xml_service_guid = 
{ 0x3db2a390, 0xbe91, 0x41f3, { 0xbe, 0xc6, 0xb7, 0x36, 0xec, 0x77, 0x92, 0xca } };


class obj_xml : public Wasabi2::Dispatchable
{
protected:
	obj_xml() : Dispatchable(DISPATCHABLE_VERSION) {}
	~obj_xml() {}
public:
	static GUID GetServiceType() { return SVC_TYPE_OBJECT; }
	static GUID GetServiceGUID() { return xml_service_guid; }
	
	enum
	{
		MATCH_EXACT = 0,
		MATCH_BEGINS = 1,
		MATCH_SINGLE = 2,
		MATCH_REGEX = 3,
	};

	void RegisterCallback(nx_string_t match_string, int match_style, ifc_xmlcallback *callback) { XML_RegisterCallback(match_string, match_style, callback); }
	void UnregisterCallback(ifc_xmlcallback *callback) { XML_UnregisterCallback(callback); }
	int Open() { return XML_Open(); }
	int Feed(const void *data, size_t data_size) { return XML_Feed(data, data_size); }
	void Flush() { XML_Flush(); }
	void Close() { XML_Close(); }
	void *GetBuffer(size_t bytes) { return XML_GetBuffer(bytes); }
	int ParseBuffer(size_t bytes) { return XML_ParseBuffer(bytes); }
	
	void Reset() { return XML_Reset(); }
//	void xmlreader_reset(); // call to allow an existing obj_xml object to parse a new file.  keeps your existing callbacks
//	void xmlreader_setEncoding(const wchar_t *encoding); // call to manually set encoding (maybe from HTTP headers)

	enum 
	{
	DISPATCHABLE_VERSION=0,
	};
private:

	virtual void WASABICALL XML_RegisterCallback(nx_string_t match_string, int match_style, ifc_xmlcallback *callback)=0;
	virtual void WASABICALL XML_UnregisterCallback(ifc_xmlcallback *callback)=0;
	virtual int WASABICALL XML_Open()=0;
	virtual int WASABICALL XML_OpenNamespace()=0;
	virtual int WASABICALL XML_Feed(const void *data, size_t data_size)=0;
	virtual int WASABICALL XML_Flush()=0;
	virtual void WASABICALL XML_Close()=0;
	virtual void WASABICALL XML_PushContext()=0;
	virtual void WASABICALL XML_PopContext()=0;
	virtual void *WASABICALL XML_GetBuffer(size_t bytes)=0;
	virtual int WASABICALL XML_ParseBuffer(size_t bytes)=0;
	virtual void WASABICALL XML_Reset()=0;
};
