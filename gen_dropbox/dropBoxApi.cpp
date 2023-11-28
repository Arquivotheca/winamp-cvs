#include "./main.h"
#include "./plugin.h"
#include "./dropBoxApi.h"
#include "./dropWindow.h"

typedef struct __EMBEDENUMPARAM
{
	HWND hDrop;
	UUID classUid;
} EMBEDENUMPARAM;

static int EmbedWndEnumerator(embedWindowState *ws, embedEnumStruct *param)
{
	EMBEDENUMPARAM *eep = (EMBEDENUMPARAM*)(INT_PTR)param->user_data;
	if (NULL == eep || NULL == ws->me) return 1;
	UUID classUid;
	if (DropboxWindow_GetClassUid(ws->me, &classUid) && IsEqualGUID(eep->classUid, classUid))
	{
		eep->hDrop = ws->me;
		return 1;
	}
	return 0;
}

static HWND FindWindowByClassUid(HWND hParent, const UUID &classUid)
{
	HWND hDrop= NULL;
	UUID windowClassUid;
	DWORD winampPid, windowPid;
	GetWindowThreadProcessId(plugin.hwndParent, &winampPid);

	for (;;)
	{
		hDrop = FindWindowEx(hParent, hDrop, NWC_DROPBOX, NULL);
		if (NULL == hDrop) break;
		GetWindowThreadProcessId(hDrop, &windowPid);
		if (winampPid == windowPid && 
			DropboxWindow_GetClassUid(hDrop, &windowClassUid) && 
			IsEqualGUID(classUid, windowClassUid))
		{
			return hDrop;
		}
	}

	EMBEDENUMPARAM param;
	embedEnumStruct ees;

	CopyMemory(&param.classUid, &classUid, sizeof(UUID));
	param.hDrop = NULL;

	ees.enumProc = EmbedWndEnumerator;
	ees.user_data = (INT)(INT_PTR)&param;
	SENDWAIPC(plugin.hwndParent, IPC_EMBED_ENUM, &ees);
	hDrop = param.hDrop;

	return hDrop;
}

HWND DropBoxApi::CreateDropBox(HWND hParent, const GUID *classUid)
{
	

	if (!RegisterDropBox(plugin.hDllInstance))
		return NULL;
	
	if (NULL == classUid || IsEqualGUID(*classUid, GUID_NULL))
		classUid = &defaultClassUid;

	const DROPBOXCLASSINFO *classInfo = Plugin_FindRegisteredClass(*classUid);
	if (NULL == classInfo)
		return NULL;

	if (0 != (DBCS_ONEINSTANCE & classInfo->style))
	{
		HWND hInstance = FindWindowByClassUid(NULL, *classUid);
		if (NULL != hInstance)
			return hInstance;
	}

	return DropBox_CreateWindow(hParent, classInfo);
}

#define CBCLASS DropBoxApi
START_DISPATCH;
CB(API_DROPBOX_CREATEDROPBOX, CreateDropBox)
END_DISPATCH;
#undef CBCLASS