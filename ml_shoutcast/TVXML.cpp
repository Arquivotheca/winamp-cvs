#include "main.h"
#include "api.h"
#include "../xml/api_xml.h"
#include "../jnetlib/api_httpget.h"
#include <api/service/waservicefactory.h>
#include "../xml/api_xmlreadercallback.h"
#include <strsafe.h>

static const wchar_t InsertStation[] = L"INSERT INTO tv (id, name, mime, bitrate, nowplaying, genre, listeners, rating) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
static const wchar_t DeleteAll[] = L"delete from tv";

class TVUpdater : public Updater
{
public:
	TVUpdater() : command(0)
	{}

	void Open(api_db *database)
	{
		api_db_command *deletecommand = database->CreateCommand(DeleteAll, -1, DB_UTF16);
		if (deletecommand)
		{
			while (1)
			{
				int res = deletecommand->ExecuteNonQuery();
				if (res == SQLITE_DONE)
					break;
			}
			database->ReleaseCommand(deletecommand);
		}

		command = database->CreateCommand(InsertStation, -1, DB_UTF16);
	}
	void Close(api_db *database)
	{
		database->ReleaseCommand(command);
		command = 0;
	}
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params)
	{
		if (!lstrcmpiW(xmlpath, L"stationlist\fstation"))
		{
			command->BindString(1, params->getItemValue(L"id"), -1, TRUE, DB_UTF16);
			command->BindString(2, params->getItemValue(L"name"), -1, TRUE, DB_UTF16);
			command->BindString(3, params->getItemValue(L"mt"), -1, TRUE, DB_UTF16);
			command->BindString(4, params->getItemValue(L"br"), -1, TRUE, DB_UTF16);
			command->BindString(5, params->getItemValue(L"ct"), -1, TRUE, DB_UTF16);
			command->BindString(6, params->getItemValue(L"genre"), -1, TRUE, DB_UTF16);
			command->BindString(7, params->getItemValue(L"lc"), -1, TRUE, DB_UTF16);
			command->BindString(8, params->getItemValue(L"rt"), -1, TRUE, DB_UTF16);

			while (1)
			{
				int res = command->ExecuteNonQuery();
				if (res == SQLITE_BUSY)
					continue;
				break;
			}
		}
		else if (!lstrcmpiW(xmlpath, L"stationlist\ftunein"))
		{
			//params->getItemValue(L"base")
			// TODO: save this somehwere
		}
	}
	const char *GetUrl()
	{
		return "http://www.shoutcast.com/sbin/newtvlister.phtml";
	}
	HWND GetUpdateWindow()
	{
		return tvHWND;
	}

protected:
	api_db_command *command;
	RECVS_DISPATCH;
};

TVUpdater tvUpdater;


DWORD CALLBACK DownloadXML(void *param);
void DownloadTV()
{
	DWORD threadId;
	CreateThread(0, 0, DownloadXML, (void *)&tvUpdater, 0, &threadId);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS TVUpdater
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
