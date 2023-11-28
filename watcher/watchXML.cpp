#include ".\watchXML.h"
#include <strsafe.h>

#define WATCHER_OPEN_STR		L("<") TAG_WATCHER L(" ") ATTRIB_ID L("=\"%s\" ") ATTRIB_PATH L("=\"%s\" ")  ATTRIB_RECURSE L("=\"%d\" ") ATTRIB_TRACK L("=\"%s\">")
#define WATCHER_CLOSE_STR		L("</") TAG_WATCHER L(">")
#define FILTER_EXTENSION_STR	L("<") TAG_FILTER L(" ") ATTRIB_FLTTYPE L("=\"%s\" ") ATTRIB_FLTVAL L("=\"%s\"/>")
#define FILTER_SIZE_STR			L("<") TAG_FILTER L(" ") ATTRIB_FLTTYPE L("=\"") ATTRIB_FLTTYPE_MINSIZE L("\" ") ATTRIB_FLTVAL L("=\"%d\"/>")


MLWatchXML::MLWatchXML(void) : errorCode(0), parser(NULL), parserFactory(NULL)
{	
	heap = GetProcessHeap();
}

MLWatchXML::~MLWatchXML(void)
{
	if (parser)
	{
		parser->xmlreader_unregisterCallback(this);
	}

	if (parserFactory && parser)
		parserFactory->releaseInterface(parser);

	parserFactory = NULL;
	parser = NULL;
}

int MLWatchXML::Parse(MLWatcher *destination, const void* xml, unsigned int length, const wchar_t* encoding)
{
	errorCode = 0;
	if (!xml) errorCode = 1003;
	if (!destination) errorCode = 1004;

	if (!parserFactory && !errorCode)
	{
		parserFactory = WASABI_API_SVC->service_getServiceByGuid(api_xmlGUID);
		if (!parserFactory) errorCode = 1006;
	}
	if (!parser && !errorCode)
	{
		parser = (api_xml *)parserFactory->getInterface();
		if (!parser) { errorCode = 1002; return 0;}
		parser->xmlreader_registerCallback(TAG_WATCHER, this);
		parser->xmlreader_registerCallback(TAG_WATCHER L"\f" TAG_FILTER, this);
		parser->xmlreader_registerCallback(L"*\f" TAG_WATCHER, this);
		parser->xmlreader_registerCallback(L"*\f" TAG_WATCHER L"\f" TAG_FILTER, this);
	}
	
	if (!errorCode)
	{
		watcher = destination;
		// set default values
		watcher->trackMode = WATCHER_TRACKMODE_CENTRAL;
		watcher->recurse = 1;
		// reset filter
		watcher->fltExt.Clear();
		watcher->fltExtType = WATCHER_FILTERTYPE_NONE;
		watcher->fltSize = 0;

		parser->xmlreader_open();
		parser->xmlreader_setEncoding(encoding);
		parser->xmlreader_feed((void*)xml, length);
		parser->xmlreader_close();
	}
	return (errorCode == 0);
}

void MLWatchXML::OnStartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params)
{
		const wchar_t *value;
		if (0 == lstrcmpW(xmltag, TAG_WATCHER))
		{
			value = params->getItemValue(ATTRIB_ID);
			if (!value)
			{
				errorCode = 1020;
				parser->xmlreader_interrupt();
				return;
			}
			watcher->id = DuplicateString(heap, value);

			
			value = params->getItemValue(ATTRIB_PATH);
			if (!value)
			{
				errorCode = 1021;
				parser->xmlreader_interrupt();
				return;
			}
			watcher->path = DuplicateString(heap, value);
			value = params->getItemValue(ATTRIB_RECURSE);
			watcher->recurse = (value && 0 == lstrcmpW(value, L"1"));
			value = params->getItemValue(ATTRIB_TRACK);
			if (value)
			{
				if (0 == lstrcmpW(value, ATTRIB_TRACK_INPLACE)) watcher->trackMode =  WATCHER_TRACKMODE_INPLACE;
				else if (0 == lstrcmpW(value, ATTRIB_TRACK_CENTRAL)) watcher->trackMode =  WATCHER_TRACKMODE_CENTRAL;
			}		
		}
		else if (0 == lstrcmpW(xmltag, TAG_FILTER))
		{
			value = params->getItemValue(ATTRIB_FLTTYPE);
			if (value)
			{
				if (0== lstrcmpW(value, ATTRIB_FLTTYPE_INCLUDE))
				{
					watcher->SetExtensionFilter(params->getItemValue(ATTRIB_FLTVAL), WATCHER_FILTERTYPE_INCLUDE);
				}
				else if (0== lstrcmpW(value, ATTRIB_FLTTYPE_EXCLUDE))
				{
					watcher->SetExtensionFilter(params->getItemValue(ATTRIB_FLTVAL), WATCHER_FILTERTYPE_EXCLUDE);
				}
				else if (0== lstrcmpW(value, ATTRIB_FLTTYPE_MINSIZE))
				{
					value = params->getItemValue(ATTRIB_FLTVAL);
					__int64 sz = (value) ? _wtoi64(value) : 0; 
					watcher->SetMinSizeFilter((size_t)sz);
				}
			}
		
			
		}
}

void MLWatchXML::OnError(int linenum, int errcode, const wchar_t *errstr)
{
	errorCode = errcode;
}

unsigned int MLWatchXML::GetStringLength(const MLWatcher* source)
{
	if (!source){ errorCode = 1005; return 0; }
	errorCode = 0;
	unsigned int len = 0;

	api_watchfilter *filter = ((MLWatcher*)source)->GetExtensionFilter();
	unsigned int fltLen = filter->GetStringLength();
	len += lstrlenW(source->id);
	len += lstrlenW(source->path);
	len += lstrlenW(WATCHER_OPEN_STR) - 4*2;
	len += lstrlenW(WATCHER_CLOSE_STR);
	switch(source->trackMode)
	{
		case WATCHER_TRACKMODE_OFF:
			len += lstrlenW(ATTRIB_TRACK_NONE);
			break;
		case WATCHER_TRACKMODE_INPLACE:
			len += lstrlenW(ATTRIB_TRACK_INPLACE);
			break;
		case WATCHER_TRACKMODE_CENTRAL:
			len += lstrlenW(ATTRIB_TRACK_CENTRAL);
			break;
	}

	if (fltLen)
	{
		len += fltLen;
		len += lstrlenW( (source->fltExtType == WATCHER_FILTERTYPE_INCLUDE) ? ATTRIB_FLTTYPE_INCLUDE : ATTRIB_FLTTYPE_EXCLUDE);
		len += lstrlenW(FILTER_EXTENSION_STR) - 2*2;
	}
	
	if (source->fltSize)
	{
        len += lstrlenW(FILTER_SIZE_STR) - 1*2;
		size_t test = source->fltSize;
		while((test = test / 10) > 0) len++;
		len++;
 	}
	
	return len;

}
int MLWatchXML::GetString(MLWatcher* source, wchar_t *buffer, unsigned int cchLen)
{
	if (!source){ errorCode = 1005; return 0; }
	errorCode = 0;

	wchar_t *flt = NULL;
	unsigned int len = 0;
	wchar_t *tBuf = NULL;
    HRESULT retCode;

	if (source->fltExtType != WATCHER_FILTERTYPE_NONE)
	{
		api_watchfilter *filter = ((MLWatcher*)source)->GetExtensionFilter();
		unsigned int len = filter->GetStringLength();
		flt = (wchar_t*)HeapAlloc(heap, NULL, (len + 1) * sizeof(wchar_t));
		if (!source->fltExt.GetString(flt, len))
		{
			HeapFree(heap, NULL, flt);
			flt = NULL;
		}
	}
	
	
	retCode = StringCchPrintfW(buffer, cchLen, WATCHER_OPEN_STR, source->id, source->path, source->recurse, 
										(source->trackMode) ? (source->trackMode == 1) ? ATTRIB_TRACK_INPLACE : ATTRIB_TRACK_CENTRAL : ATTRIB_TRACK_NONE);
	if (S_OK != retCode) errorCode = 1010;
	else
	{
		len = lstrlenW(buffer);
		tBuf = buffer + len;
	}
	if (!errorCode && flt)
	{
		retCode = StringCchPrintfW(tBuf, cchLen - len, FILTER_EXTENSION_STR, 
										(source->fltExtType == WATCHER_FILTERTYPE_INCLUDE) ? ATTRIB_FLTTYPE_INCLUDE : ATTRIB_FLTTYPE_EXCLUDE,
										flt);
		if (S_OK != retCode) errorCode = 1010;
		else
		{
			len += lstrlenW(tBuf);
			tBuf = buffer + len;
		}
	}
	if (!errorCode && source->fltSize)
	{
		retCode = StringCchPrintfW(tBuf, cchLen - len, FILTER_SIZE_STR, source->fltSize);
		if (S_OK != retCode) errorCode = 1010;
		else
		{
			len += lstrlenW(tBuf);
			tBuf = buffer + len;
		}
	}
	if (!errorCode)
	{
		retCode = StringCchPrintfW(tBuf, cchLen - len, WATCHER_CLOSE_STR, TAG_WATCHER);
		if (S_OK != retCode) errorCode = 1010;
	}
	
	if (flt) HeapFree(heap, NULL, flt);
	return (errorCode == 0);
}

inline wchar_t* MLWatchXML::DuplicateString(void *heap, const wchar_t *source)
{
	unsigned int len = (lstrlenW(source)+1)*sizeof(wchar_t);
	wchar_t *dst = (wchar_t*)HeapAlloc(heap, NULL, len);
	CopyMemory(dst, source, len);
	return dst;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MLWatchXML
START_DISPATCH;
VCB(ONSTARTELEMENT, OnStartTag)
VCB(ONERROR, OnError)
END_DISPATCH;