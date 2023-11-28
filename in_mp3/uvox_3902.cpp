#include "uvox_3902.h"
#include "api.h"
#include <api/service/waservicefactory.h>
#include <strsafe.h>

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (api_t && WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
}

Ultravox3902::Ultravox3902() : parser(0)
{
	title[0]=album[0]=artist[0]=0;
	ServiceBuild(parser, obj_xmlGUID);
	if (parser)
	{
		parser->xmlreader_setCaseSensitive();
		parser->xmlreader_registerCallback(L"metadata\f*", this);
		parser->xmlreader_open();
	}
}

Ultravox3902::~Ultravox3902()
{
	if (parser)
	{
		parser->xmlreader_unregisterCallback(this);
		parser->xmlreader_close();
	}
	ServiceRelease(parser, obj_xmlGUID);
}

int Ultravox3902::Parse(const char *xml_data)
{
	if (parser)
	{
		int ret = parser->xmlreader_feed((void *)xml_data, strlen(xml_data));
		if (ret != API_XML_SUCCESS)
			return ret;
		return parser->xmlreader_feed(0, 0);
	}
	return API_XML_FAILURE;
}

void Ultravox3902::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	if (!wcscmp(xmlpath, L"metadata\fTIT2"))
	{
		StringCbCatW(title, sizeof(title), str);
	}
	else if (!wcscmp(xmlpath, L"metadata\fTALB"))
	{
		StringCbCatW(album, sizeof(album), str);
	}
	else if (!wcscmp(xmlpath, L"metadata\fTPE1"))
	{
		StringCbCatW(artist, sizeof(artist), str);
	}
}

int Ultravox3902::GetExtendedData(const char *tag, wchar_t *data, int dataLen)
{
	if (!_stricmp(tag, "title"))
		StringCchCopy(data, dataLen, title);
	else if (!_stricmp(tag, "album"))
		StringCchCopy(data, dataLen, album);
	else if (!_stricmp(tag, "artist"))
		StringCchCopy(data, dataLen, artist);
	else
		return 0;
	return 1;
}

#define CBCLASS Ultravox3902
START_DISPATCH;
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
#undef CBCLASS