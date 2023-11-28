#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./api_dropbox.h"

#include "../nu/vector.h"

static HWND g_hwndListener = NULL;


#define NWC_ACCELLISTENER	TEXT("DropBoxAccelListener")
#define IDC_DISPLAY_DROPBOX	40005

#define ACCEL_COMMAND_MIN	40000
#define ACCEL_COMMAND_MAX	40100

typedef struct __CMDTARGET
{
	INT cmd;
	UUID classUid;
} CMDTARGET;

typedef Vector<CMDTARGET> TargetList;

typedef struct __LISTENER
{
	HACCEL		accelTable;
	TargetList	*tragetList;
} LISTENER;

#define GetListener(__hwnd) ((LISTENER*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))


#define SLM_FIRST		(WM_USER + 1)
#define SLM_REGISTER		(SLM_FIRST + 0)
#define SLM_UNREGISTER	(SLM_FIRST + 1)

static void CALLBACK UninitializePluginShortcut(void)
{
	if (NULL != g_hwndListener)
	{
		DestroyWindow(g_hwndListener);
		g_hwndListener = NULL;
	}
}

static void PluginShortcut_CreateInstance(const UUID &classUid)
{
	api_dropbox *dropBoxApi = QueryWasabiInterface(api_dropbox, dropBoxApiGuid);
	if (NULL != dropBoxApi)
	{
		HWND hParent = plugin.hwndParent; // do not use dialog parent for now
        HWND hDrop = dropBoxApi->CreateDropBox(hParent, &classUid);
		ReleaseWasabiInterface(dropBoxApiGuid, dropBoxApi);
		HWND hRoot = GetAncestor(hDrop, GA_ROOT); // fix in case some skin reparented as;
	
		if (!IsWindowVisible(hDrop))
			ShowWindow(hDrop, SW_SHOWNA);

		if (IsWindowVisible(hRoot) && IsWindowEnabled(hRoot))
			SetActiveWindow(hRoot);
	}
	
}

static LRESULT ShortcutListener_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	LISTENER *pl = (LISTENER*)malloc(sizeof(LISTENER));
	if (NULL == pl)
		return -1;
	
	ZeroMemory(pl, sizeof(LISTENER));
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)pl) && ERROR_SUCCESS != GetLastError())
	{
		free(pl);
		return -1;
	}
	return 0;
}

static void ShortcutListener_OnDestroy(HWND hwnd)
{
	LISTENER *pl = GetListener(hwnd);
	SetWindowLongPtr(hwnd, 0, 0);

	if (NULL != WASABI_API_APP)
		WASABI_API_APP->app_removeAccelerators(hwnd);

	if (NULL == pl) return;
		
	if (NULL != pl->accelTable)
		DestroyAcceleratorTable(pl->accelTable);

	if (NULL != pl->tragetList)
		delete(pl->tragetList);
}

static INT ShortcutListener_AddCmdTarget(HWND hwnd, const UUID &classUid)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl)	return 0;
	
	INT commandId = ACCEL_COMMAND_MIN;

	if (NULL != pl->tragetList)
	{
		for(;;)
		{
			size_t index = pl->tragetList->size();
			while(index-- && pl->tragetList->at(index).cmd != commandId);
			if (index < pl->tragetList->size())
				commandId++;
			else
				break;
		}
	}

	CMDTARGET link;
	link.classUid = classUid;
	link.cmd = commandId;

	if (NULL == pl->tragetList)
	{
		pl->tragetList = new TargetList();
		if (NULL == pl->tragetList)
			return 0;
	}
	pl->tragetList->push_back(link);
	return commandId;

}
static BOOL ShortcutListener_RemoveCmdTarget(HWND hwnd, const UUID &classUid, INT *commmandIdOut)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl)	return FALSE;

	if (NULL != pl->tragetList)
	{
		size_t index = pl->tragetList->size();
		while(index--)
		{
			if (IsEqualGUID(pl->tragetList->at(index).classUid, classUid))
			{
				if (NULL != commmandIdOut)
					*commmandIdOut = pl->tragetList->at(index).cmd;
				pl->tragetList->eraseAt(index);
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL ShortcutListener_GetCmdTarget(HWND hwnd, INT commandId, UUID *classUidOut)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl || NULL == pl->tragetList)
		return FALSE;

	size_t index = pl->tragetList->size();
	while(index--)
	{
		if (pl->tragetList->at(index).cmd == commandId)
		{
			if (NULL != classUidOut)
				CopyMemory(classUidOut, &pl->tragetList->at(index).classUid, sizeof(UUID));
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ShortcutListner_Find(HWND hwnd, const ACCEL *accel, UUID *classUidOut)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl || NULL == pl->accelTable || NULL == accel)
		return FALSE;

	INT searchSize = CopyAcceleratorTable(pl->accelTable, NULL, 0);
	ACCEL *searchTable = (ACCEL*)malloc(sizeof(ACCEL) * searchSize);
	if (NULL == searchTable)
		return FALSE;

	BOOL foundOk = FALSE;
	if(0 != CopyAcceleratorTable(pl->accelTable, searchTable, searchSize))
	{
		for (INT i = 0; i < searchSize; i++)
		{
			if (searchTable[i].key == accel->key &&
				searchTable[i].fVirt == accel->fVirt)
			{
				if (NULL != classUidOut)
					ShortcutListener_GetCmdTarget(hwnd, searchTable[i].cmd, classUidOut);
				foundOk = TRUE;
				break;
			}
		}
	}

	free(searchTable);
	return foundOk;
}
static BOOL ShortcutListener_RegisterAccelTable(HWND hwnd, ACCEL *accelList, INT accelCount)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl || NULL == WASABI_API_APP) return FALSE;

	if (NULL != pl->accelTable)
	{
		WASABI_API_APP->app_removeAccelerators(hwnd);
		DestroyAcceleratorTable(pl->accelTable);
		pl->accelTable = NULL;
	}
		
	if (NULL == accelList || 0 ==accelCount)
		return TRUE;

	pl->accelTable = CreateAcceleratorTable(accelList, accelCount);
	if (NULL != pl->accelTable)
	{
		WASABI_API_APP->app_addAccelerators(hwnd, &pl->accelTable, 1, TRANSLATE_MODE_GLOBAL);
		return TRUE;
	}

	return FALSE;
}
static LRESULT ShortcutListener_OnRegister(HWND hwnd, const UUID *classUid, const ACCEL *accel)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl || NULL == WASABI_API_APP) return FALSE;

	UUID registeredUid;
	if (ShortcutListner_Find(hwnd, accel, &registeredUid))
		return IsEqualGUID(*classUid, registeredUid);

	INT accelCount = (NULL != pl->accelTable) ?	CopyAcceleratorTable(pl->accelTable, NULL, 0) : 0;
	accelCount += 1;

	ACCEL *accelList = (ACCEL*)malloc(sizeof(ACCEL) * accelCount);
	if (NULL == accelList)
		return FALSE;

	INT accelIndex = (accelCount > 1) ? CopyAcceleratorTable(pl->accelTable, accelList, accelCount - 1) : 0;
	accelList[accelIndex].fVirt = accel->fVirt;
	accelList[accelIndex].key = accel->key;
	accelList[accelIndex].cmd = ShortcutListener_AddCmdTarget(hwnd, *classUid);

	BOOL success = FALSE;
	if (0 != accelList[accelIndex].cmd)
		success = ShortcutListener_RegisterAccelTable(hwnd, accelList, accelCount);
	
	free(accelList);
	return success;
}

static LRESULT ShortcutListener_OnUnregister(HWND hwnd, const UUID *classUid)
{
	LISTENER *pl = GetListener(hwnd);
	if (NULL == pl || NULL == WASABI_API_APP || NULL == classUid) return FALSE;

	INT commandId;
	if (!ShortcutListener_RemoveCmdTarget(hwnd, *classUid, &commandId))
		return FALSE;

	BOOL success = FALSE;
	INT accelCount = CopyAcceleratorTable(pl->accelTable, NULL, 0);
	ACCEL *accelList = (ACCEL*)malloc(sizeof(ACCEL) * accelCount);
			
	if(NULL != accelList &&
		0 != CopyAcceleratorTable(pl->accelTable, accelList, accelCount))
	{
		for (INT i = 0; i < accelCount; i++)
		{
			if (accelList[i].cmd == commandId)
			{
				if (i != (accelCount - 1))
					MoveMemory(&accelList[i], &accelList[i + 1], sizeof(ACCEL) * (accelCount - (i + 1)));
				accelCount -= 1;
				success = ShortcutListener_RegisterAccelTable(hwnd, accelList, accelCount);;
			}
		}
	}

	if (NULL != accelList)
		free(accelList);
	
	return success;
}

static void ShortcutListener_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	if (controlId >= ACCEL_COMMAND_MIN && controlId <= ACCEL_COMMAND_MIN)
	{
		UUID targetUid;
		if (ShortcutListener_GetCmdTarget(hwnd, controlId, &targetUid))
			PluginShortcut_CreateInstance(targetUid);
	}
}

static LRESULT CALLBACK ShortcutListener_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:		return ShortcutListener_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:		ShortcutListener_OnDestroy(hwnd); break;
		case WM_COMMAND:		ShortcutListener_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case SLM_REGISTER:		return ShortcutListener_OnRegister(hwnd, (const UUID*)lParam, (const ACCEL*)wParam);
		case SLM_UNREGISTER:		return ShortcutListener_OnUnregister(hwnd, (const UUID*)lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static BOOL PluginShortcut_RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	if (GetClassInfo(hInstance, NWC_ACCELLISTENER, &wc)) return TRUE;
	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_ACCELLISTENER;
	wc.lpfnWndProc	= ShortcutListener_WindowProc;
	wc.cbWndExtra	= sizeof(LISTENER*);
	return ( 0 != RegisterClass(&wc));
}

BOOL PluginShortcut_Register(const UUID &classUid, const ACCEL *pAccel)
{
	if (NULL == WASABI_API_APP || NULL == pAccel)
		return FALSE;

	if (NULL == g_hwndListener)
	{		
		PluginShortcut_RegisterWindowClass(plugin.hDllInstance);
		g_hwndListener = CreateWindow(NWC_ACCELLISTENER, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, plugin.hDllInstance, NULL);
		if (NULL == g_hwndListener)
			return FALSE;
		Plugin_RegisterUnloadCallback(UninitializePluginShortcut);
	}
	return (BOOL)SendMessage(g_hwndListener, SLM_REGISTER, (WPARAM)pAccel, (LPARAM)&classUid);
}

BOOL PluginShortcut_Unregister(const UUID &classUid)
{	
	if (NULL == g_hwndListener)
		return FALSE;
	return (BOOL)SendMessage(g_hwndListener, SLM_UNREGISTER, (WPARAM)0, (LPARAM)&classUid);
}