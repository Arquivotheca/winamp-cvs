#include "PLSLoader.h"

#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include <shlwapi.h>
#include <strsafe.h>

#define FILENAME_SIZE 1040
#define FILETITLE_SIZE 400

PLSLoader::PLSLoader()
{
}

PLSLoader::~PLSLoader()
{}

class PLSInfo : public ifc_plentryinfo
{
public:
	PLSInfo(const wchar_t *_filename, int _entryNum) : filename(_filename), entryNum(_entryNum)	{	}
	const wchar_t *GetExtendedInfo(const wchar_t *parameter)
	{
		static wchar_t data[1024];
		wchar_t fieldbuf[100];
		StringCchPrintfW(fieldbuf, 100, L"%s%d", parameter, entryNum);
		GetPrivateProfileStringW(L"playlist", fieldbuf, L"", data, 1024, filename);
		if (data[0])
			return data;
		else
			return 0;
	}
	private:
	const wchar_t *filename;
	int entryNum;
	RECVS_DISPATCH;
};

#define CBCLASS PLSInfo
START_DISPATCH;
CB(IFC_PLENTRYINFO_GETEXTENDEDINFO, GetExtendedInfo)
END_DISPATCH;
#undef CBCLASS

int PLSLoader::OnFileHelper(ifc_playlistloadercallback *playlist, const wchar_t *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo)
{
	if (length == -1000)
		length = -1;

	int ret;
	if (wcsstr(trackName, L"://") || PathIsRootW(trackName))
	{
		ret = playlist->OnFile(trackName, title, length, extraInfo);
	}
	else
	{
		wchar_t fullPath[MAX_PATH], canonicalizedPath[MAX_PATH];

		if (PathCombineW(fullPath, rootPath, trackName))
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

int PLSLoader::Load(const wchar_t *filename, ifc_playlistloadercallback *playlist)
{
	int x, numfiles;
	int ext = 0;
	char fieldbuf[100];
	char fnbuf[FILENAME_SIZE];
	char tmp[MAX_PATH]="";

	wchar_t rootPath[MAX_PATH];
	const wchar_t *callbackPath = playlist->GetBasePath();
	if (callbackPath)
		lstrcpynW(rootPath, callbackPath, MAX_PATH);
	else
	{
		lstrcpynW(rootPath, filename, MAX_PATH);
		PathRemoveFileSpecW(rootPath);
	}
	tmp[0]=(char)rootPath[0];
	tmp[1]=(char)rootPath[1];
	tmp[2]=(char)rootPath[2];

	AutoChar fn(filename);

	numfiles = GetPrivateProfileInt("playlist", "NumberOfEntries", 0, fn);
	ext = GetPrivateProfileInt("playlist", "Version", 1, fn);
	if (numfiles == 0)
		return IFC_PLAYLISTLOADER_FAILED;

	for (x = 1; x <= numfiles; x ++)
	{
		int flen = -1;
		char ftitle[FILETITLE_SIZE] = "";
		StringCchPrintf(fieldbuf, 100, "File%d", x);
		GetPrivateProfileString("playlist", fieldbuf, "", fnbuf, FILENAME_SIZE, fn);
		if (ext)
		{
			StringCchPrintf(fieldbuf, 100, "Title%d", x);
			GetPrivateProfileString("playlist", fieldbuf, "", ftitle, FILETITLE_SIZE, fn);
			StringCchPrintf(fieldbuf, 100, "Length%d", x);
			flen = GetPrivateProfileInt("playlist", fieldbuf, -1, fn);
		}

		if (*fnbuf)
		{
			char *p;
			char buf[512];

			p = fnbuf;

			if (strncmp(p, "\\\\", 2) && strncmp(p + 1, ":\\", 2) && !strstr(p, ":/"))
			{
				if (p[0] == '\\')
				{
					buf[0] = tmp[0];
					buf[1] = tmp[1];
					lstrcpyn(buf + 2, p, 510);
					buf[511] = 0;
					p = buf;
				}
			}

			PLSInfo info(filename, x);

			int ret;
			if (ftitle[0])
			{
				ret = OnFileHelper(playlist, AutoWide(p), AutoWide(ftitle), flen*1000, rootPath, &info);
			}
			else
			{
				ret = OnFileHelper(playlist, AutoWide(p), 0, -1, rootPath, &info); 
			}
			if (ret != ifc_playlistloadercallback::LOAD_CONTINUE)
			{
				break;
			}
		}
	}

	return IFC_PLAYLISTLOADER_SUCCESS;
}

#define CBCLASS PLSLoader
START_DISPATCH;
CB(IFC_PLAYLISTLOADER_LOAD, Load)
END_DISPATCH;
#undef CBCLASS