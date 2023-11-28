#include "main.h"
#include "api.h"
#include "../xml/obj_xml.h"
#include "../jnetlib/api_httpget.h"
#include <api/service/waservicefactory.h>
#include "../xml/ifc_xmlreadercallback.h"
#include <strsafe.h>

bool Updater::Run()
{
	if (running)
		return false;
	running=true;
	return true;
}

void Updater::Stop()
{
	running=false;
}

void DeleteAllRecords(nde_scanner_t s)
{
	NDE_Scanner_First(s);
	
	while (NDE_Scanner_GetRecordsCount(s))
	{
		NDE_Scanner_Delete(s);
	}
}

class RadioUpdater : public Updater
{
public:
	RadioUpdater() : table(0), scanner(0)
	{
	}

	void Open(const wchar_t *filename, const wchar_t *index)
	{
		table = CreateRadioTable(filename, index);

		scanner = NDE_Table_CreateScanner(table);
		DeleteAllRecords(scanner);
	}

	void Close()
	{
		NDE_Table_DestroyScanner(table, scanner);
		NDE_Table_Sync(table);
		NDE_Database_CloseTable(db, table);
	}
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
	{
		if (!lstrcmpiW(xmlpath, L"stationlist\fstation"))
		{	
			nde_field_t f = 0;
			NDE_Scanner_New(scanner);
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_ID);
			NDE_IntegerField_SetValue(f, _wtoi(params->getItemValue(L"id")));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_NAME);
			NDE_StringField_SetString(f, params->getItemValue(L"name"));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_MIMETYPE);
			NDE_StringField_SetString(f, params->getItemValue(L"mt"));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_BITRATE);
			NDE_IntegerField_SetValue(f, _wtoi(params->getItemValue(L"br")));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_NOWPLAYING);
			NDE_StringField_SetString(f, params->getItemValue(L"ct"));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_GENRE);
			NDE_StringField_SetString(f, params->getItemValue(L"genre"));
			f = NDE_Scanner_NewFieldByID(scanner, RADIOTABLE_LISTENERS);
			NDE_IntegerField_SetValue(f, _wtoi(params->getItemValue(L"lc")));
			NDE_Scanner_Post(scanner);
		}
		else if (!lstrcmpiW(xmlpath, L"stationlist\ftunein"))
		{
			//params->getItemValue(L"base")
			// TODO: save this somehwere
		}
	}
	const char *GetUrl()
	{
		return "http://www.shoutcast.com/sbin/old-newxml.phtml?genre=";
	}
	HWND GetUpdateWindow()
	{
		return radioHWND;
	}
	
protected:
	nde_table_t table;
	nde_scanner_t scanner;
	RECVS_DISPATCH;
};


RadioUpdater radioUpdater;

extern int winampVersion;

#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
static void SetUserAgent(api_httpreceiver *http)
{
	char user_agent[USER_AGENT_SIZE];
	int bigVer = ((winampVersion & 0x0000FF00) >> 12);
	int smallVer = ((winampVersion & 0x000000FF));
	StringCchPrintfA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%01x.%02x", bigVer, smallVer);
	http->addheader(user_agent);
}

#define HTTP_BUFFER_SIZE 32768

static int FeedXMLHTTP(api_httpreceiver *http, obj_xml *parser, bool *noData)
{
	char downloadedData[HTTP_BUFFER_SIZE];
	int xmlResult = API_XML_SUCCESS; 
	int downloadSize = http->get_bytes(downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
		xmlResult = parser->xmlreader_feed((void *)downloadedData, downloadSize);
		*noData=false;
	}
	else	
		*noData = true;

	return xmlResult;
}

static void RunXMLDownload(api_httpreceiver *http, obj_xml *parser)
{
	bool noData;
	int ret;
	do
	{
		Sleep(50);
		ret = http->run();
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
	if (ret != HTTPRECEIVER_RUN_ERROR)
		return ;
	else
		return ;
}

void RunHTTP(const char *url, api_httpreceiver *http, obj_xml *parser)
{
	if (!http)
		return;
	http->AllowCompression();
	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, mediaLibrary.GetProxy());
	SetUserAgent(http);
	http->connect(url);
	int ret;
	bool first = true;

	do
	{
		Sleep(50);
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

				return ;
			}
			break;
		default:
			return ;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);
	const char *er = http->geterrorstr();

}

DWORD CALLBACK DownloadXML(void *param)
{
	Updater *xml_to_db = (Updater *)param;

	if (!xml_to_db->Run())
		return 0;


	wchar_t dbFilename[MAX_PATH], dbIndex[MAX_PATH];
	StringCchPrintfW(dbFilename, MAX_PATH, L"%S\\Plugins\\ml\\shoutcast.dat", mediaLibrary.GetIniDirectory());
	StringCchPrintfW(dbIndex, MAX_PATH, L"%S\\Plugins\\ml\\shoutcast.idx", mediaLibrary.GetIniDirectory());
	
	xml_to_db->Open(dbFilename, dbIndex);
	
	obj_xml *parser = 0;
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (!parser)
		return 0; // no sense in continuing if there's no parser available
	
	parser->xmlreader_registerCallback(L"stationlist\f*", xml_to_db);
	parser->xmlreader_open();

	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf) http = (api_httpreceiver *)sf->getInterface();

	RunHTTP(xml_to_db->GetUrl(), http, parser);
	sf->releaseInterface(http);

	xml_to_db->Close();

	PostMessage(xml_to_db->GetUpdateWindow(), WM_USER + 10, 0, 0);
	xml_to_db->Stop();

	return 0;
}

void Download()
{
	DWORD threadId;
	CreateThread(0, 0, DownloadXML, (void *)&radioUpdater, 0, &threadId);
}

#define CBCLASS RadioUpdater
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
#undef CBCLASS