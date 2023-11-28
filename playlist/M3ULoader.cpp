#include "M3ULoader.h"
#include <stdio.h>
#include "../nu/ns_wc.h"
#include <shlwapi.h>
#include <strsafe.h>

M3ULoader::M3ULoader()
		: utf8(false)
{}

M3ULoader::~M3ULoader(void)
{
	//Close();
}

class M3UInfo : public ifc_plentryinfo
{
public:
	M3UInfo(const char *_mediahash, const char *_metahash, const char *_cloud_id,
			const char *_cloud_status, const char *_cloud_devices) :
			mediahash(_strdup(_mediahash)), metahash(_strdup(_metahash)),
			cloud_id(_strdup(_cloud_id)), cloud_status(_strdup(_cloud_status)),
			cloud_devices(_strdup(_cloud_devices)){ }
	~M3UInfo() {
		if (mediahash) free((void*)mediahash);
		if (metahash) free((void*)metahash);
		if (cloud_id) free((void*)cloud_id);
		if (cloud_status) free((void*)cloud_status);
		if (cloud_devices) free((void*)cloud_devices);
	}

	const wchar_t *GetExtendedInfo(const wchar_t *parameter)
	{
		if (cloud_id && *cloud_id)
		{
			if (!_wcsnicmp(L"mediahash", parameter, 9))
			{
				static wchar_t data[128];
				StringCchPrintfW(data, 128, L"%hs", mediahash);
				return data;
			}
			else if (!_wcsnicmp(L"metahash", parameter, 8))
			{
				static wchar_t data2[128];
				StringCchPrintfW(data2, 128, L"%hs", metahash);
				return data2;
			}
			else if (!_wcsnicmp(L"cloud_id", parameter, 8))
			{
				static wchar_t data3[128];
				StringCchPrintfW(data3, 128, L"%hs", cloud_id);
				return data3;
			}
			else if (!_wcsnicmp(L"cloud_status", parameter, 12))
			{
				static wchar_t data4[16];
				StringCchPrintfW(data4, 128, L"%hs", cloud_status);
				return data4;
			}
			else if (!_wcsnicmp(L"cloud_devices", parameter, 13))
			{
				static wchar_t data5[128];
				StringCchPrintfW(data5, 128, L"%hs", cloud_devices);
				return data5;
			}
		}
		return 0;
	}

private:
	const char *mediahash, *metahash, *cloud_id, *cloud_status, *cloud_devices;
	RECVS_DISPATCH;
};

#define CBCLASS M3UInfo
START_DISPATCH;
CB(IFC_PLENTRYINFO_GETEXTENDEDINFO, GetExtendedInfo)
END_DISPATCH;
#undef CBCLASS


int M3ULoader::OnFileHelper(ifc_playlistloadercallback *playlist, const char *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo)
{
	if (length == -1000)
		length = -1;

	MultiByteToWideCharSZ(utf8 ? CP_UTF8 : CP_ACP, 0, trackName, -1, wideFilename, 1040);
	int ret;
	if (wcsstr(wideFilename, L"://") || PathIsRootW(wideFilename))
	{
		ret = playlist->OnFile(wideFilename, title, length, extraInfo);
	}
	else
	{
		wchar_t fullPath[MAX_PATH], canonicalizedPath[MAX_PATH];
		if (PathCombineW(fullPath, rootPath, wideFilename))
		{
			PathCanonicalizeW(canonicalizedPath, fullPath);
			ret = playlist->OnFile(canonicalizedPath, title, length, extraInfo);
		}
		else
		{
			ret = ifc_playlistloadercallback::LOAD_CONTINUE;
		}
	}
	return ret;
}

static bool StringEnds(const wchar_t *a, const wchar_t *b)
{
	size_t aLen = wcslen(a);
	size_t bLen	= wcslen(b);
	if (aLen < bLen) return false;  // too short
	if (!_wcsicmp(a + aLen- bLen, b))
		return true;
	return false;
}

int M3ULoader::Load(const wchar_t *filename, ifc_playlistloadercallback *playlist)
{
	FILE *fp;

	// TODO: download temp file if it's a URL

	fp = _wfopen(filename, L"rt");
	if (!fp) return IFC_PLAYLISTLOADER_FAILED;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size == -1)
	{
		fclose(fp);
		fp = 0;
		return IFC_PLAYLISTLOADER_FAILED;
	}

	if (StringEnds(filename, L".m3u8"))
		utf8 = true;

	int ext = 0;
	char *p;
	char linebuf[2048];

	char ext_title[MAX_PATH] = {0}, ext_mediahash[128] = {0},
		 ext_metahash[128] = {0}, ext_cloud_id[128] = {0},
		 ext_cloud_status[16] = {0}, ext_cloud_devices[128] = {0};
	int ext_len = -1;

	wchar_t rootPath[MAX_PATH];
	const wchar_t *callbackPath = playlist->GetBasePath();
	if (callbackPath)
		StringCchCopyW(rootPath, MAX_PATH, callbackPath);
	else
	{
		StringCchCopyW(rootPath, MAX_PATH, filename);
		PathRemoveFileSpecW(rootPath);
	}

	unsigned char BOM[3] = {0, 0, 0};
	if (fread(BOM, 3, 1, fp) == 1 && BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
		utf8 = true;
	else
		fseek(fp, 0, SEEK_SET);

	while (1)
	{
		if (feof(fp)) break;
		linebuf[0] = 0;
		fgets(linebuf, sizeof(linebuf) - 1, fp);
		linebuf[sizeof(linebuf) - 1] = 0;
		if (!linebuf[0]) continue;
		if (!strncmp(linebuf, "#EXTM3U", 7))
		{
			ext = 1;
			continue;
		}
		if (!strncmp(linebuf, "#UTF8", 5))
		{
			utf8 = 1;
			continue;
		}
		p = linebuf;
		while (*p == ' ' || *p == '\t')
			p = CharNext(p);
		if (*p != '#' && *p != '\n' && *p != '\r' && *p)
		{
			char buf[4096];
			char *p2 = CharPrev(linebuf, linebuf + strlen(linebuf)); //GetLastCharacter(linebuf);
			if (*p2 == '\n') *p2 = 0;
			if (!strncmp(p, "ASF ", 4) && strlen(p) > 4)
				p += 4;

			if (strncmp(p, "\\\\", 2) && strncmp(p + 1, ":\\", 2) && strncmp(p + 1, ":/", 2) && !strstr(p, "://"))
			{
				if (p[0] == '\\')
				{
					buf[0] = (char)rootPath[0];
					buf[1] = (char)rootPath[1];
					StringCchCopy(buf + 2, sizeof(buf) - 2, p);
					buf[sizeof(buf) - 1] = 0;
					p = buf;
				}
			}
			int ret;

			// generate extra info from the cloud specific values (if present)
			M3UInfo info(ext_mediahash, ext_metahash, ext_cloud_id, ext_cloud_status, ext_cloud_devices);

			if (ext_title[0])
			{
				MultiByteToWideCharSZ(utf8 ? CP_UTF8 : CP_ACP, 0, ext_title, -1, wideTitle, 400);
				ret = OnFileHelper(playlist, p, wideTitle, ext_len*1000, rootPath, &info); // TODO: pass extra info junk
			}
			else
			{
				ret = OnFileHelper(playlist, p, 0, -1, rootPath, &info); // TODO: pass extra info junk
				
			}

			if (ret != ifc_playlistloadercallback::LOAD_CONTINUE)
			{
				break;
			}

			ext_len = -1;
			ext_title[0] = 0;
		}
		else
		{
			if (ext && !strncmp(p, "#EXTINF:", 8))
			{
				p += 8;
				ext_len = atoi(p);
				while ((*p >= '0' && *p <= '9') || *p == '-') p = CharNext(p);
				if (*p)
				{
					char *p2 = CharPrev(p, p + strlen(p)); // GetLastCharacter(p);
					if (*p2 == '\n') *p2 = 0;
					StringCchCopyA(ext_title, MAX_PATH, CharNext(p));
				}
				else
				{
					ext_len = -1;
					ext_title[0] = 0;
				}
			}
			// cloud specific playlist line for holding information about the entry
			else if (ext && !strncmp(p, "#EXT-X-NS-CLOUD:", 16))
			{
				p += 16;
				char *pt = strtok(p, ",");
				while (pt != NULL)
				{
					int end = strcspn(pt, "=");

					if (!strncmp(pt, "mediahash", end))
					{
						lstrcpyn(ext_mediahash, pt+end+1, 128);
					}
					else if (!strncmp(pt, "metahash", end))
					{
						lstrcpyn(ext_metahash, pt+end+1, 128);
					}
					else if (!strncmp(pt, "cloud_id", end))
					{
						lstrcpyn(ext_cloud_id, pt+end+1, 128);
					}
					else if (!strncmp(pt, "cloud_status", end))
					{
						lstrcpyn(ext_cloud_status, pt+end+1, 16);
					}
					else if (!strncmp(pt, "cloud_devices", end))
					{
						char *p2 = pt+end+1;
						while (*p2 != '\n') p2 = CharNext(p2);
						if (*p2 == '\n') *p2 = 0;
						lstrcpyn(ext_cloud_devices, pt+end+1, 128);
					}
					pt = strtok(NULL, ",");
				}
			}
			else
			{
				ext_len = -1;
				ext_title[0] = 0;
				ext_mediahash[0] = 0;
				ext_metahash[0] = 0;
				ext_cloud_id[0] = 0;
				ext_cloud_status[0] = 0;
				ext_cloud_devices[0] = 0;
			}
		}
	}

	fclose(fp);

	return IFC_PLAYLISTLOADER_SUCCESS;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS M3ULoader
START_DISPATCH;
CB(IFC_PLAYLISTLOADER_LOAD, Load)
#if 0
VCB(IFC_PLAYLISTLOADER_CLOSE, Close)
CB(IFC_PLAYLISTLOADER_GETITEM, GetItem)
CB(IFC_PLAYLISTLOADER_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLISTLOADER_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
CB(IFC_PLAYLISTLOADER_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
CB(IFC_PLAYLISTLOADER_NEXTITEM, NextItem)
#endif
END_DISPATCH;