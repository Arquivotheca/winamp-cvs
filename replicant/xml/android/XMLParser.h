#pragma once

#include "xml/obj_xml.h"
#include "nx/nxstring.h"
#include <expat.h>
#include "nu/PtrDeque.h"
#include "nu/vector.h"


struct CallbackStruct : public nu::PtrDequeNode
{
	CallbackStruct(ifc_xmlcallback *_callback, nx_string_t _match, int match_style);
	CallbackStruct();
	~CallbackStruct(); 
	ifc_xmlcallback *callback;
	nx_string_t match;
	int match_style;
};

class QDString // quick and dirty string
{
public:
	QDString();
	~QDString();
	enum CopyData
	{
		DESTROY_DATA = 0,
		RETAIN_DATA = 1,
	};
	operator char *();
	void Grow(size_t newLen, CopyData copyData=RETAIN_DATA);
	void CopyFrom(const char *intake);
	char *str;
	size_t len;
};

class XMLParser : public obj_xml
{
public:
	static nx_string_t GetServiceName() { return 0; } // TODO!!
	XMLParser();
	~XMLParser();

	/* callbacks that expat will call */
	void XMLCALL StartTag(const XML_Char *name, const XML_Char **atts);
	void XMLCALL EndTag(const XML_Char *name);
	void XMLCALL TextHandler(const XML_Char *s, int len);

private:
	/* Internal helper functions */
	const char *AddPath(const char *node);
	const char *BuildPath();
	const char *RemovePath(const char *node);

	/* obj_xml implementation */
	void WASABICALL XML_RegisterCallback(nx_string_t match_string, int match_style, ifc_xmlcallback *callback);
	void WASABICALL XML_UnregisterCallback(ifc_xmlcallback *callback);
	int WASABICALL XML_Feed(const void *data, size_t dataSize);
	int WASABICALL XML_Flush();
	void WASABICALL XML_Close();
	void WASABICALL XML_PushContext();
	void WASABICALL XML_PopContext();
	int WASABICALL XML_Open();
	int WASABICALL XML_OpenNamespace();
	void *WASABICALL XML_GetBuffer(size_t bytes);
	int WASABICALL XML_ParseBuffer(size_t bytes);
	void WASABICALL XML_Reset();

	XML_Parser parser;
	size_t path_size; // in characters
	typedef nu::PtrDeque<CallbackStruct> CallbackList;
	CallbackList callbacks;

	/* here we are taking advantage of the fact that XML_SetUserData/XML_GetUserData is the first member of the struct */
	typedef nu::PtrDeque<nu::PtrDequeNode> ContextList;
	ContextList context;

	QDString path_string;
	QDString end_path_string;
	QDString current_node;
#if 0
	void SetEncoding(const char *encoding);
#endif	
};
