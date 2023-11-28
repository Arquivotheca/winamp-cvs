#include "RenameDownloader.h"
#include "api.h"
#include "../nu/AutoUrl.h"
#include "main.h"
#include "../jnetlib/api_httpget.h"
#include <strsafe.h>

class PingCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *jnet = WASABI_API_DLMGR->GetReceiver(token);
		if (jnet)
		{
			jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
			jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
		}
	}

	RECVS_DISPATCH;
};

#define CBCLASS PingCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
END_DISPATCH;
#undef CBCLASS

static PingCallback ping_callback;
void RenameDevice(const char *root_url, const wchar_t *new_name)
{
	if (WASABI_API_DLMGR)
	{
		char url[1024];
		StringCbPrintfA(url, sizeof(url), "%s/set?nick=%s", root_url, AutoUrl(new_name));
		WASABI_API_DLMGR->DownloadEx(url, &ping_callback, api_downloadManager::DOWNLOADEX_BUFFER);
	}
}
