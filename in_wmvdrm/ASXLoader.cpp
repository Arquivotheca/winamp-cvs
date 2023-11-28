#include "main.h"
#include "api.h"
#include "ASXLoader.h"
#include <stdio.h>
#include "../nu/AutoWide.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "../xml/obj_xml.h"
#include "api.h"
#include <api/service/waservicefactory.h>
#include "../jnetlib/api_httpget.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>
#include "XMLString.h"

#define FILENAME_SIZE 1040

void SetUserAgent(api_httpreceiver *http)
{
	char agent[256];
	StringCchPrintfA(agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString());
	http->addheader(agent);
}

const wchar_t *GetLastCharactercW(const wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

const wchar_t *scanstr_backcW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval)
{
	const wchar_t *s = GetLastCharactercW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	while (1)
	{
		const wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

class ASXInfo : public ifc_plentryinfo
{
public:
	ASXInfo()  : isRadio(false), repeat(0)
	{
	}
	const wchar_t *GetExtendedInfo(const wchar_t *parameter)
	{
		if (!_wcsicmp(parameter, L"context"))
		{
			if (isRadio)
				return L"radio";
		}
		else if (!_wcsicmp(parameter, L"repeat"))
		{
			if (repeat)
			{
				StringCchPrintfW(returnTemp, 20, L"%d", repeat);
				return returnTemp;
			}
		}
		return 0;
	}
	bool isRadio;
	int repeat;
protected:
	RECVS_DISPATCH;
	wchar_t returnTemp[20];
};

#define CBCLASS ASXInfo
START_DISPATCH;
CB(IFC_PLENTRYINFO_GETEXTENDEDINFO, GetExtendedInfo)
END_DISPATCH;
#undef CBCLASS

class ASXXML : public ifc_xmlreadercallback
{
public:
	ASXXML(api_playlistloadercallback *_playlist, const wchar_t *_root, obj_xml *_parser) : playlist(_playlist), rootPath(_root), parser(_parser)
	{
	}

	void ASXXML::OnFileHelper(ifc_playlistloadercallback *playlist, const wchar_t *filename, const wchar_t *title, int length, ifc_plentryinfo *extraInfo)
{
	if (wcsstr(filename, L"://") || PathIsRootW(filename))
	{
		playlist->OnFile(filename, title, length, extraInfo);
	}
	else
	{
		wchar_t fullPath[MAX_PATH], canonicalizedPath[MAX_PATH];
		PathCombineW(fullPath, rootPath, filename);
		PathCanonicalizeW(canonicalizedPath, fullPath);
		playlist->OnFile(canonicalizedPath, title, length, extraInfo);
	}
}

	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
	{
		if (!_wcsicmp(xmltag, L"ENTRYREF"))
		{
			const wchar_t *url = params->getItemValue(L"HREF");

			const wchar_t *titleHack = params->getItemValue(L"CLIENTBIND");
			int lengthHack = -1;
			wchar_t titleBuf[256] = L"";

			if (titleHack)
			{
				// get the length out of the parantheses
				StringCchCopyW(titleBuf, 256, titleHack);
				wchar_t *end = titleBuf + lstrlenW(titleBuf);
				while (*end != '(' && end != titleBuf)
					end = CharPrevW(titleBuf, end);

				*end = 0;
				end++;
				lengthHack = _wtoi(end);
			}

			wchar_t filename[FILENAME_SIZE];
			if (wcschr(url, L'?'))
				StringCchPrintfW(filename, FILENAME_SIZE, L"%s&=.asx", url);
			else
				StringCchPrintfW(filename, FILENAME_SIZE, L"%s?.asx", url);

			OnFileHelper(playlist, filename, titleBuf, lengthHack*1000, &info);

		}
		else if (!_wcsicmp(xmlpath, L"ASX\fENTRY\fREF") || !_wcsicmp(xmlpath, L"ASX\fREPEAT\fENTRY\fREF"))
		{
			const wchar_t *track = params->getItemValue(L"HREF");
			const wchar_t *trackTitle = 0;
			wchar_t fullTitle[128];
			wchar_t fullFilename[FILENAME_SIZE];
			// if there is no extension given, we need to add ?.wma or &=.wma to the end of the URL
			// this could be 2 lines of code if that wasn't the case :(
			if (track)
			{
				if (title.GetString()[0] && artist.GetString()[0])
				{
					StringCchPrintfW(fullTitle, 128, L"%s - %s", artist.GetString(), title.GetString());
					trackTitle = fullTitle;
				}
				if (!_wcsnicmp(track, L"http://", 7))
				{
					const wchar_t *end = scanstr_backcW(track, L"/.", 0);
					if (!end || *end == L'/')
					{
						if (wcschr(track, L'?'))
							StringCchPrintfW(fullFilename, FILENAME_SIZE, L"%s&=.wma", track);
						else
							StringCchPrintfW(fullFilename, FILENAME_SIZE, L"%s?.wma", track);
						track = fullFilename;
					}
				}
				
				OnFileHelper(playlist, track, trackTitle, -1, &info);
			}
		}
		else if (!_wcsicmp(xmltag, L"REPEAT"))
		{
			const wchar_t *param;
			if (param = params->getItemValue(L"count"))
			{
				info.repeat = _wtoi(param);
			}
			else
				info.repeat = -1;
		}
		else if (!_wcsicmp(xmltag, L"PARAM"))
		{
			const wchar_t *param;
			if (param = params->getItemValue(L"name"))
			{
				if (!_wcsicmp(param, L"context"))
				{
					const wchar_t * value = params->getItemValue(L"value");
					if (!_wcsicmp(value, L"station"))
						info.isRadio = true;
				} 
				else if (!_wcsicmp(param, L"encoding"))
				{
					const wchar_t *value=params->getItemValue(L"value");
					if (value)
						parser->xmlreader_setEncoding(value); // I hope we can set it on the fly like this!
				}
			}
		}
	}

	void EndTag(const wchar_t *xmlpath, const wchar_t *xmltag)
	{
		if (!_wcsicmp(xmltag, L"REPEAT"))
		{
			info.repeat = 0;
		}
	}
	api_playlistloadercallback *playlist;
	XMLString title, artist;
	ASXInfo info;
	const wchar_t *rootPath;
	obj_xml *parser;
protected:
	RECVS_DISPATCH;

};

#define CBCLASS ASXXML
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONENDELEMENT, EndTag)
END_DISPATCH;
#undef CBCLASS


/*
TODO:
 
don't add tracks until all parts of the "ENTRY" tag are processed.  There are some ASX playlists where the title comes AFTER the ref's
 
maybe have separate XML callbacks for metadata (title, author, shit like that) so that logic can be separated from the main ASX logic
*/

// ASX isn't really XML.  That means we need to URL-encode the text so our XML parser doesn't choke
// if microsoft followed standards, the world would be a better place.
int ASXLoader::GayASX_to_XML_converter(obj_xml *parser, char *buffer, int len)
{
	// benski> I have no idea if ASX is always ASCII, or if it's UTF-8 or what.
	// but really I can't be bothered with Microsoft's lameness right now, so we'll assume it's local code page for the time being
	char *start = buffer;
	int sofar = 0;
	for (int i = 0;i < len;i++)
	{
		if (buffer[i] == '&')
		{
			if (sofar)
			{
				if (parser->xmlreader_feed(start, sofar) != API_XML_SUCCESS)
					return API_XML_FAILURE;
			}

			if (parser->xmlreader_feed("&amp;", 5) != API_XML_SUCCESS) // no null terminator
				return API_XML_FAILURE;
			start = &buffer[i + 1];
			sofar = 0;
		}
		else 
		{
			/**
			  * ok, this might look really weird
			  * but ASX doesn't have case sensitivity
				* so lots of playlists have things like
				* <title>This is the title</Title>
				* and so we have to accomodate
				* for this nonsense
				*/

			if (inTag && !inQuotes)
				buffer[i] = toupper(buffer[i]);

			if (buffer[i] == '>')
			{
				inTag=false;
			}
			else if (buffer[i] == '<')
			{
				inTag=true;
			}

			// dro> only do uppercase handling on parts of the tag not inbetween quotes
			// (some servers just don't like having the urls case messed with, the swines)
			if (buffer[i] == '"')
			{
				if(!inQuotes)
					inQuotes=true;
				else
					inQuotes=false;
			}

			sofar++;
		}
	}
	if (sofar && parser->xmlreader_feed(start, sofar) != API_XML_SUCCESS)
		return API_XML_FAILURE;
	OutputDebugStringA(buffer);
	return API_XML_SUCCESS;
}


#define HTTP_BUFFER_SIZE 16384
int ASXLoader::FeedXMLHTTP(api_httpreceiver *http, obj_xml *parser, bool *noData)
{
	char downloadedData[HTTP_BUFFER_SIZE];
	int xmlResult = API_XML_SUCCESS; 
	int downloadSize = http->get_bytes(downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
		xmlResult = GayASX_to_XML_converter(parser, downloadedData, downloadSize);
		*noData=false;
	}
	else	
		*noData = true;

	return xmlResult;
}

void ASXLoader::RunXMLDownload(api_httpreceiver *http, obj_xml *parser)
{
int ret;
	bool noData;
	do
	{
		Sleep(50);
		ret = http->run();
		if (FeedXMLHTTP(http, parser, &noData) != API_XML_SUCCESS)
			return ;
	}
	while (ret == HTTPRECEIVER_RUN_OK);

	// finish off the data
	do
	{
		if (FeedXMLHTTP(http, parser, &noData) != API_XML_SUCCESS)
			return ;
  } while (!noData);

	parser->xmlreader_feed(0, 0);
}

int ASXLoader::LoadFile(obj_xml *parser, const wchar_t *filename)
{
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return IFC_PLAYLISTLOADER_FAILED;

	char data[1024];

	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			if (GayASX_to_XML_converter(parser, data, bytesRead) != API_XML_SUCCESS)
			{
				CloseHandle(file);
				return IFC_PLAYLISTLOADER_FAILED;
			}
		}
		else
			break;
	}

	CloseHandle(file);
	if (parser->xmlreader_feed(0, 0) != API_XML_SUCCESS)
		return IFC_PLAYLISTLOADER_FAILED;

	return IFC_PLAYLISTLOADER_SUCCESS;
}


int ASXLoader::LoadURL(obj_xml *parser, const wchar_t *url)
{
	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf) http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return IFC_PLAYLISTLOADER_FAILED;
	http->AllowCompression();
	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, winamp.GetProxy()); 
	SetUserAgent(http);
	http->connect(AutoChar(url));
	int ret;
	bool first = true;

	do
	{
		Sleep(10);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int replycode = http->getreplycode();
		switch (replycode)
		{
		case 0:
		case 100:
			break;
		case 200:
			{
				RunXMLDownload(http, parser);
				sf->releaseInterface(http);
				return IFC_PLAYLISTLOADER_SUCCESS;
			}
			break;
		default:
			sf->releaseInterface(http);
			return IFC_PLAYLISTLOADER_FAILED;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);
	//const char *er = http->geterrorstr();
	sf->releaseInterface(http);
	return IFC_PLAYLISTLOADER_FAILED;
}

static int loadasxv2fn(const wchar_t *filename, api_playlistloadercallback *playlist)
{
	int i=1;
	wchar_t ref[1040];
	wchar_t key[100];
	while (1)
	{
		StringCchPrintfW(key, 100, L"Ref%d", i++);
		GetPrivateProfileStringW(L"Reference", key, L"?", ref, 1040, filename);
		if (!lstrcmpiW(ref, L"?"))
		break;
		else
		{
				if (!_wcsnicmp(ref, L"http://", 7))
				{
					const wchar_t *end = scanstr_backcW(ref, L"/.", 0);
					if (!end || *end == L'/')
					{
						if (wcschr(ref, L'?'))
							StringCchCatW(ref, FILENAME_SIZE, L"&=.wma");
						else
								StringCchCatW(ref, FILENAME_SIZE, L"?.wma");
					}
				}

				playlist->OnFile(ref, 0, 0, 0);
		}

	}
	return IFC_PLAYLISTLOADER_SUCCESS;
}

int ASXLoader::Load(const wchar_t *filename, api_playlistloadercallback *playlist)
{
	obj_xml *parser = 0;
	waServiceFactory *parserFactory = 0;

	HANDLE quickTest = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (quickTest != INVALID_HANDLE_VALUE)
	{
		char reference[11];
		DWORD bytesRead=0;
		ReadFile(quickTest, reference, 11, &bytesRead, 0);
		CloseHandle(quickTest);
		if (bytesRead == 11 && !_strnicmp(reference, "[Reference]", 11))
			return loadasxv2fn(filename, playlist);		
	}

	parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		wchar_t rootPath[MAX_PATH];
		const wchar_t *callbackPath = playlist->GetBasePath();
		if (callbackPath)
			lstrcpynW(rootPath, callbackPath, MAX_PATH);
		else
		{
			lstrcpynW(rootPath, filename, MAX_PATH);
			PathRemoveFileSpecW(rootPath);
		}

		ASXXML asxXml(playlist, rootPath, parser);
		parser->xmlreader_registerCallback(L"ASX\f*", &asxXml);
		parser->xmlreader_registerCallback(L"ASX\fENTRY\fTITLE", &asxXml.title);
		parser->xmlreader_registerCallback(L"ASX\fENTRY\fAUTHOR", &asxXml.artist);
		parser->xmlreader_open();
		parser->xmlreader_setEncoding(L"windows-1252");
		
		int ret;
		if (wcsstr(filename, L"://"))
			ret = LoadURL(parser, filename);
		else
			ret = LoadFile(parser, filename);

		parser->xmlreader_unregisterCallback(&asxXml);
		parser->xmlreader_unregisterCallback(&asxXml.title);
		parser->xmlreader_unregisterCallback(&asxXml.artist);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		return ret;
	}


	return IFC_PLAYLISTLOADER_FAILED;
}

#define CBCLASS ASXLoader
START_DISPATCH;
CB(IFC_PLAYLISTLOADER_LOAD, Load)
END_DISPATCH;
#undef CBCLASS
