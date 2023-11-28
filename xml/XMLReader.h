#ifndef NULLSOFT_XML_XMLREADER_H
#define NULLSOFT_XML_XMLREADER_H

#include "obj_xml.h"
#include "../nu/PtrList.h"
#include "../expat/expat.h"
#include "../nu/Vector.h"

struct CallbackStruct
{
	CallbackStruct(ifc_xmlreadercallback *_callback, const wchar_t *_match, bool doUpper);
	CallbackStruct();
	~CallbackStruct(); 
  ifc_xmlreadercallback *callback;
	wchar_t *match;	
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
	operator wchar_t *();
	void Grow(size_t newLen, CopyData copyData=RETAIN_DATA);
	void CopyFrom(const wchar_t *intake);
	wchar_t *str;
	size_t len;
	
};

class XMLReader : public obj_xml
{
public:
	XMLReader();
	~XMLReader();
	void RegisterCallback(const wchar_t *matchstr, ifc_xmlreadercallback *callback);
	void UnregisterCallback(ifc_xmlreadercallback *callback);
	int Open();
	int OpenNamespace();
  void OldFeed(void *data, size_t dataSize);
	int Feed(void *data, size_t dataSize);
	void Close();
	void PushContext();
	void PopContext();
	void Reset();
	void SetEncoding(const wchar_t *encoding);
	int SetCaseSensitive();

protected:
	RECVS_DISPATCH;

public:
	void XMLCALL StartTag(const XML_Char *name, const XML_Char **atts);
	void XMLCALL EndTag(const XML_Char *name);
	void XMLCALL TextHandler(const XML_Char *s, int len);

private:
	const wchar_t *BuildPath();
	const wchar_t *AddPath(const wchar_t *node);
	const wchar_t *RemovePath(const wchar_t *node);
	QDString pathString;//, pathUpper;
	QDString endPathString;//, endPathUpper;
	QDString currentNode;

private:
	nu::PtrList<CallbackStruct> callbacks;
	Vector<XML_Parser> context;
	XML_Parser parser;
	size_t pathSize; // in characters
bool case_sensitive;
	QDString textCache;
	
};
#endif