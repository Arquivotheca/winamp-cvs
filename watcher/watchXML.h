#ifndef NULLSOFT_ML_WATCHXML_HEADER
#define NULLSOFT_ML_WATCHXML_HEADER

#include <windows.h>

#include "api.h"
#include <api/service/waServiceFactory.h>
#include <../xml/api_xml.h>
#include <../xml/api_xmlreadercallback.h>

#define L(x)	L##x

#define TAG_WATCHER		L("WATCHER")
#define TAG_FILTER		L("FILTER")

#define ATTRIB_ID		L("ID")
#define ATTRIB_PATH		L("PATH")
#define ATTRIB_RECURSE	L("RECURSE")
#define ATTRIB_TRACK	L("TRACKMODE")
#define ATTRIB_FLTTYPE	L("TYPE")
#define ATTRIB_FLTVAL	L("VALUE")

#define ATTRIB_TRACK_NONE		L("OFF")
#define ATTRIB_TRACK_INPLACE	L("INPLACE")
#define ATTRIB_TRACK_CENTRAL	L("CENTRAL")

#define ATTRIB_FLTTYPE_INCLUDE	L("INCLUDE")
#define ATTRIB_FLTTYPE_EXCLUDE	L("EXCLUDE")
#define ATTRIB_FLTTYPE_MINSIZE	L("MINSIZE")

#include ".\watcher.h"

class MLWatchXML : protected api_xmlreadercallback
{
public:
	MLWatchXML(void);
	~MLWatchXML(void);

public:	
	int Parse(MLWatcher *destination, const void* xml, unsigned int length, const wchar_t* encoding);
	unsigned int GetStringLength(const MLWatcher* source);
	int GetString(MLWatcher* source, wchar_t *buffer, unsigned int cchLen);

 protected:
	void OnStartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params);
	void OnError(int linenum, int errcode, const wchar_t *errstr);
	static wchar_t* DuplicateString(void* heap, const wchar_t *source);
	RECVS_DISPATCH;

private:
	void	*heap;
	api_xml *parser;
	waServiceFactory *parserFactory;
	int errorCode;

	MLWatcher *watcher;
};

#endif // NULLSOFT_ML_WATCHXML_HEADER
