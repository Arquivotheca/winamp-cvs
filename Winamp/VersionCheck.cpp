/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/

#include "Main.h"
#include "resource.h"
#include "api.h"
#include "language.h"
#include "..\jnetlib\api_httpget.h"
#include <api/service/waServiceFactory.h>
#include "Browser.h"
#include "../nu/AutoUrl.h"
#include "../nu/threadname.h"
#include "stats.h"
#include "../nu/threadpool/TimerHandle.h"

extern UpdateBrowser *updateBrowser;

class VersionCheckCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http)
		{
			http->AllowCompression();
			http->addheader("Accept: */*");
		}
	}

	void OnFinish(DownloadToken token)
	{
		api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http && http->getreplycode() == 200)
		{
			char *buf;
			size_t size;
			if (AGAVE_API_DOWNLOADMANAGER->GetBuffer(token, (void **)&buf, &size) == 0)
			{
				//			buf[size] = 0;
				char *p = buf;

				while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
				{
					p++;
					size--;
				}

				char newVer[6] = {0,};
				if (size >= 3 && p[1] == '.')
				{
					size_t i = 0;
					while (size && i != 6 && (i == 1 || IsCharDigit(p[i])))
					{
						newVer[i] = p[i];
						newVer[i + 1] = 0;
						size--;
						i++;
					}
					p += i;
					while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
					{
						size--;
						p++;
					}

					char curVer[6] = APP_VERSION;
					while (lstrlen(curVer) < 5)
						StringCchCat(curVer, 6, "0");
					while (lstrlen(newVer) < 5)
						StringCchCat(newVer, 6, "0");

					int verDif = strcmp(newVer, curVer);
					//#if defined(BETA) || defined(NIGHTLY)
					//				if (verDif == 0)
					//					verDif = 1; // if this is a BETA version, then we should upgrade if the versions are equal
					//#endif

					if (verDif == 0) // same version
					{
						char updateNumber[32] = "";
						char *u = updateNumber;
						while (size && u != (updateNumber + 31) && *p && *p != '\r' && *p != '\n')
						{
							size--;
							*u++ = *p++;
							*u = 0;
						}
						int update = atoi(updateNumber);
						if (update > config_newverchk3)
						{
							if (config_newverchk3) // only display update if we've already established a serial #
								verDif = 1;
							config_newverchk3 = update;
						}
					}
					if (verDif > 0) // same version or older
					{
						while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
						{
							size--;
							p++;
						}
						if (size)
						{
							char *disp = (char *)malloc(size + 1);
							memcpy(disp, p, size);
							disp[size]=0;
							if (!_strnicmp(p, "http://", 7))
							{
								PostMessage(hMainWindow, WM_WA_IPC, (WPARAM)disp, IPC_UPDATE_URL);
							}
							else
							{
								if (MessageBox(NULL, disp, getString(IDS_WINAMP_UPDATE_MSG,NULL,0), MB_YESNO) == IDYES)
								{
									myOpenURL(NULL, L"http://www.winamp.com/player");
								}
								free(disp);
							}
						}
					}
				}
			}
		}
		config_newverchk = getDay();
	}

	void OnError(DownloadToken token)
	{
		config_newverchk = getDay();
	}

	RECVS_DISPATCH;
};

#define CBCLASS VersionCheckCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
END_DISPATCH;
#undef CBCLASS

static VersionCheckCallback versionCheckCallback;

static void CheckVersion()
{
	if (AGAVE_API_DOWNLOADMANAGER)
	{
		char url[1024];
		char *urlend=0;
		size_t urlend_size=0;
		StringCchPrintfEx(url,1024, &urlend, &urlend_size, 0, "http://client.winamp.com/update/latest-version.php?v=%s",APP_VERSION);

		char uid[512]="";
		stats_getuidstr(uid);
		if (uid[0])
			StringCchPrintfEx(urlend, urlend_size, &urlend, &urlend_size, 0, "&ID=%s", uid);

		const wchar_t *langIdentifier = langManager?(langManager->GetLanguageIdentifier(LANG_IDENT_STR)):0;
		if (langIdentifier)
			StringCchPrintf(urlend, urlend_size, "&lang=%s", (char *)AutoUrl(langIdentifier));

		AGAVE_API_DOWNLOADMANAGER->DownloadEx(url, &versionCheckCallback, api_downloadManager::DOWNLOADEX_BUFFER);
	}
}

bool DoVerChk(int verchk)
{
	return verchk == 1 || (verchk > 1 && verchk + 1 < (int)getDay());
}

class PingCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http)
			http->addheader("Accept: */*");
	}
	RECVS_DISPATCH;
};

#define CBCLASS PingCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
END_DISPATCH;
#undef CBCLASS

static PingCallback pingCallback;

void Ping(const char *url)
{
	if (AGAVE_API_DOWNLOADMANAGER)
		AGAVE_API_DOWNLOADMANAGER->DownloadEx(url, &pingCallback, api_downloadManager::DOWNLOADEX_BUFFER);
}

void newversioncheck(void)
{
	if (isInetAvailable())
	{
		if (DoVerChk(config_newverchk))
		{
			// go ahead and call this on the main thread to ensure that the GUID gets created w/o a race condition
			char uid[512]="";
			stats_getuidstr(uid);
			CheckVersion();
		}
		if (DoVerChk(config_newverchk2))
		{
			char _url[MAX_URL];
			char *url=_url;
			size_t urlsize=MAX_URL;

			StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"http://client.winamp.com/update/client_session.php?v=%s",APP_VERSION);

			char uid[512]="";
			stats_getuidstr(uid);
			if (uid[0])
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"&ID=%s", uid);

			int values[Stats::NUM_STATS] = {0, };
			stats.GetStats(values);
			for (int x = 0; x < Stats::NUM_STATS; x ++)
			{
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"&st%d=%d", x + 1, values[x]);
			}

			wchar_t stat_str[256];
			stats.GetString("skin", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"&skin=%s",(char *)AutoUrl(stat_str));

			stats.GetString("colortheme", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"&ct=%s",(char *)AutoUrl(stat_str));

			stats.GetString("pmp", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0,"&pmp=%s",(char *)AutoUrl(stat_str));

			const wchar_t *langIdentifier = langManager?(langManager->GetLanguageIdentifier(LANG_IDENT_STR)):0;
			if (langIdentifier)
				StringCchPrintfEx(url, urlsize, &url, &urlsize, 0, "&lang=%s", (char *)AutoUrl(langIdentifier));

			Ping(_url);
			config_newverchk2 = getDay();
		}
	}
}