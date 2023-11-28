/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:  main.cpp
 ** Project: Winamp
 ** Description: Winamp initialization code
 ** Author: Justin Frankel
 ** Created: April 1997
 **/
#include "main.h"
#include "../Agave/Language/lang.h"
#include <stdarg.h>
#include "vis.h"
#include "fft.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "menuv5.h"
#include "../gen_ml/ml.h"
#include "wa_dlg.h"
#include "strutil.h"
#include "./setup/setupfactory.h"
#include "./commandLink.h"
#include "AppRefCount.h"
#include <unknwn.h>
#include <shlwapi.h>
#include <shobjidl.h>

#ifndef WM_DWMSENDICONICTHUMBNAIL
#define WM_DWMSENDICONICTHUMBNAIL 0x0323
#endif

#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#endif

#ifndef THBN_CLICKED
#define THBN_CLICKED 0x1800
#endif

typedef HRESULT(WINAPI *CHANGEWINDOWMESSAGEFILTER)(UINT message, DWORD dwFlag);
static HMODULE user32Lib = 0;
static CHANGEWINDOWMESSAGEFILTER changeWMFilter;
static BOOL changeWMLoadTried = FALSE;

//#define BENSKI_TEST_WM_PRINTCLIENT
static UINT WM_TASKBARCREATED;
static UINT WM_TASKBARBUTTONCREATED;

UINT g_scrollMsg;
UINT songChangeBroadcastMessage = 0;
int g_noreg;
int disable_skin_borders = 0;
int no_notify_play = 0;
int last_no_notify_play = 0;
int main_delta_carryover = 0;
int g_restartonquit = 0;
char g_audiocdletter[4] = {0};
int g_audiocdletters = 0;
char *app_name, app_version[] = APP_VERSION, app_version_string[] = APP_VERSION_STRING; // application name and version strings
int g_fullstop;
char *app_date = __DATE__ ;
int g_stopaftercur;
int is_install;
HWND hTooltipWindow, hEQTooltipWindow, hVideoTooltipWindow, hPLTooltipWindow;
HWND hMainWindow = NULL;			// main window
HWND hEQWindow, hPLWindow, /*hMBWindow, */hVideoWindow, hExternalVisWindow = NULL;

HWND g_dialog_box_parent = NULL; // used by IPC_SETDIALOGBOXPARENT (FG, 5/19/03)
HINSTANCE language_pack_instance;
HINSTANCE hMainInstance;	// program instance
HANDLE hMainThread; // main thread handle
DWORD mainThreadId; // main thread ID
HMENU main_menu = 0, top_menu = 0, g_submenus_bookmarks1 = 0,
	  g_submenus_bookmarks2 = 0, g_submenus_skins1 = 0,
	  g_submenus_skins2 = 0, g_submenus_vis = 0,
	  g_submenus_options = 0, g_submenus_lang = 0,
	  g_submenus_play = 0;
int g_submenus_lang_id = 0;
int g_video_numaudiotracks = 1;
int g_video_curaudiotrack = 0;

int bStartPlaying = 0;
int paused = 0;
int playing = 0;
wchar_t caption[CAPTION_SIZE];				// current program caption
wchar_t FileName[FILENAME_SIZE];			// current file name
wchar_t FileTitle[FILETITLE_SIZE];			// current file title
wchar_t FileTitleNum[FILETITLE_SIZE];		// current file title + track position
int eggstat = 0;		// used for easter eggs
int g_srate, g_brate, g_nch, g_srate_exact;
int last_brate = -1, g_need_titleupd = 0, g_need_infoupd = 0;
int g_SkinTop, g_BookmarkTop, g_LangTop;
int g_mm_optionsbase_adj = 0; //used by IPC_ADJUST_OPTIONSMENUPOS
int g_mm_ffwindowsbase_adj = 0; //used by IPC_ADJUST_FFWINDOWSMENUPOS
int g_mm_ffoptionsbase_adj = 0; //used by IPC_ADJUST_FFOPTIONSMENUPOS
int g_has_video_plugin = 0; //filled in by in_init

char playlist_custom_font[128];
wchar_t playlist_custom_fontW[128];
int config_custom_plfont = 1;
int disable_skin_cursors = 0;
int vis_fullscreen = 0;
char *audits[AUDITSIZE];
int audit_ptr = 0;

struct ITaskbarList* pTaskbar = NULL;
struct ITaskbarList3* pTaskbar3 = NULL;

static LRESULT Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);

HWND find_otherwinamp(wchar_t *);

#undef HANDLE_WM_NCACTIVATE
#define HANDLE_WM_NCACTIVATE(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(BOOL)(fn)((hwnd), (BOOL)(wParam), (HWND)(lParam), 0L)

int stat_isit = 1; // used for faster version checkig
wchar_t szAppName[64]; //	window class name, generated on the fly.

EXTERN_C BOOL eggTyping = FALSE;
static char eggstr[9];								// nifty egg strings, one of which is generated
// to avoid detection :)

int g_exit_disabled = 0;
int g_safeMode = 0;
HANDLE g_hEventRunning;
int bNoHwndOther = 0;

EXTERN_C unsigned char appname_tmpbuf[8] =
{
	~'W', ~'i', ~'n', ~'a', ~'m', ~'p', 255, 0,
};

EXTERN_C unsigned char wa_secret_value[16]
=
{
	0x19, 0xEA, 0xD0, 0x00,
	0x7A, 0xC4, 0x92, 0xA2,
	0x33, 0x72, 0x14, 0xff,
	0xff, 0x40, 0x45, 0x11,
};

unsigned char stuff[8] =
{
		0xFF,0xFF,0xFF,0xFF,0x85,0xFF,0xFF,0xFF
};

static void CreateEQPresets()
{
	if (!PathFileExistsW(EQDIR1))
	{
		int x;
		struct
		{
			char *s;
			unsigned char tab[10];
		}
		eqsets[] =
		{
			{"Classical", {31, 31, 31, 31, 31, 31, 44, 44, 44, 48}},
			{"Club", {31, 31, 26, 22, 22, 22, 26, 31, 31, 31}},
			{"Dance", {16, 20, 28, 32, 32, 42, 44, 44, 32, 32}},
			{"Flat", {31, 31, 31, 31, 31, 31, 31, 31, 31, 31}},
			{"Laptop speakers/headphones", {24, 14, 23, 38, 36, 29, 24, 16, 11, 8}},
			{"Large hall", {15, 15, 22, 22, 31, 40, 40, 40, 31, 31}},
			{"Party", {20, 20, 31, 31, 31, 31, 31, 31, 20, 20}},
			{"Pop", {35, 24, 20, 19, 23, 34, 36, 36, 35, 35}},
			{"Reggae", {31, 31, 33, 42, 31, 21, 21, 31, 31, 31}},
			{"Rock", {19, 24, 41, 45, 38, 25, 17, 14, 14, 14}},
			{"Soft", {24, 29, 34, 36, 34, 25, 18, 16, 14, 12}},
			{"Ska", {36, 40, 39, 33, 25, 22, 17, 16, 14, 16}},
			{"Full Bass", {16, 16, 16, 22, 29, 39, 46, 49, 50, 50}},
			{"Soft Rock", {25, 25, 28, 33, 39, 41, 38, 33, 27, 17}},
			{"Full Treble", {48, 48, 48, 39, 27, 14, 6, 6, 6, 4}},
			{"Full Bass & Treble", {20, 22, 31, 44, 40, 29, 18, 14, 12, 12}},
			{"Live", {40, 31, 25, 23, 22, 22, 25, 27, 27, 28}},
			{"Techno", {19, 22, 31, 41, 40, 31, 19, 16, 16, 17}},
		};

		for (x = 0; x < sizeof(eqsets) / sizeof(eqsets[0]); x ++)
			writeEQfile_init(EQDIR1, eqsets[x].s, eqsets[x].tab);
	}
}


void BuildAppName()
{
	// initialize strings, mostly avoiding attempts by hackers
	// (I hate those hacked versions that give no credit at all to us :)
	// TODO check me
	app_name = (char*)appname_tmpbuf;

	eggstr[0] = ~'N';
	eggstr[1] = ~'U';
	eggstr[2] = ~'L';
	eggstr[3] = ~'L';
	eggstr[4] = ~'S';
	eggstr[5] = ~'O';
	eggstr[6] = ~'F';
	eggstr[7] = ~'T';
	eggstr[8] = 0;


	{
		int x;
		tealike_crappy_code((unsigned long *)(app_name+=8), (unsigned long *)appname_tmpbuf);
		for (x = 0; x < 8; x ++) appname_tmpbuf[x] ^= 255;
		tealike_crappy_code((unsigned long *)(app_name+=8), (unsigned long *)appname_tmpbuf + 4);
		for (x = 0; x < 8; x ++) eggstr[x] ^= 255;

		app_name -= 16;
#if 0
		{
			char buf[512];
			for (x = 0; x < 16; x ++)
			{
				wsprintf(buf + x*5, "0x%02X,", wa_secret_value[x]);
			}
			MessageBox(NULL, buf, buf, MB_OK);
			OutputDebugString(buf);
		}
		//0xE7,0x4A,0x67,0x3C,0x44,0x05,0x0B,0x39,0xB9,0xD2,0xEB,0x63,0xCC,0xFD,0x07,0x69,
#endif

	}

	StringCchCopyW(szAppName, 64, L"Winamp v1.x");
	StringCchPrintfW(caption, CAPTION_SIZE, L"%S %S", app_name, app_version_string);
}


// creates (but does not show) main window
int CreateMainWindow()
{
	if (hMainWindow == NULL)
	{
		HICON hIcon;
		WNDCLASSW wcW = {0, };

		wcW.style = CS_DBLCLKS;
		wcW.lpfnWndProc = Main_WndProc;
		wcW.hInstance = hMainInstance;
		hIcon = wcW.hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(ICON_XP));
		wcW.hCursor = NULL;
		wcW.lpszClassName = szAppName;

		if (!RegisterClassW(&wcW))
			return 0;

		if (!CreateWindowExW(WS_EX_ACCEPTFILES, szAppName, L"Winamp", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CAPTION,
		                     config_wx, config_wy, 0, 0,        // WM_CREATE will size it
		                     NULL, NULL, hMainInstance, NULL)) return 0;

	}
	return 1;
}

char *getGUIDstr(const GUID guid, char *target)
{
	StringCchPrintf(target, 40, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\0",
		(int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
		(int)guid.Data4[0], (int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
		(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7] );
	return target;
}

static int PassToOtherWinamp(wchar_t *lpszCmdParam, HWND hwnd_other, int bAdd, int bBookmark, int bHandle)
{
	int bWait = 0;
	if (lpszCmdParam && *lpszCmdParam)	// if we have command line params, pass
		// to other winamp window
	{
		int skinExit = 0;
		int bC = 0;
		HANDLE hSem;
		HINSTANCE existingWLZ = 0, templng = 0;
		DWORD_PTR vn = 0;

		// check if we're using a language pack with the already open winamp process
		// and if so then we're going to use the winamp.lng from it on the messagebox
		// only if we had a success and the other winamp returned the correct value
		// within the timeout period (can't be having it lock up so revert if needed)
		if(SendMessageTimeout(hwnd_other,WM_WA_IPC,1,IPC_GETLANGUAGEPACKINSTANCE, SMTO_NORMAL, 5000, &vn) && !vn){
			DWORD processid = 0;
			HANDLE hwaProcess = NULL, h = INVALID_HANDLE_VALUE;
			SIZE_T bread = 0;
			wchar_t lng_path_copy[MAX_PATH] = {0}, dirmask[MAX_PATH] = {0};
			WIN32_FIND_DATAW d = {0};
			char gs[40] = {0};

			GetWindowThreadProcessId(hwnd_other, &processid);
			hwaProcess = OpenProcess(PROCESS_VM_READ, FALSE, processid);
			ReadProcessMemory(hwaProcess, (wchar_t*)SendMessage(hwnd_other,WM_WA_IPC,3,IPC_GETLANGUAGEPACKINSTANCE), lng_path_copy, MAX_PATH, &bread);
			CloseHandle(hwaProcess);

			getGUIDstr(WinampLangGUID,gs);
			PathCombineW(dirmask, lng_path_copy, L"*.lng");
			h = FindFirstFileW(dirmask, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					PathCombineW(dirmask, lng_path_copy, d.cFileName);
					templng = LoadLibraryW(dirmask);
					if(templng){
						char s[39] = {0};
						if(LoadString(templng, LANG_DLL_GUID_STRING_ID, s, 39))
						{
							if(!_strnicmp(gs, s, 38)){
								existingWLZ = Lang_FakeWinampLangHInst(templng);
							}
						}
					}
				}
				while (FindNextFileW(h, &d));
				FindClose(h);
			}
		}

		lpszCmdParam = CheckSkin(lpszCmdParam, hwnd_other, &skinExit);
		if (skinExit)
		{
			// restore the language pack settings now that we've done the override and clean up as needed
			if(existingWLZ)
			{
				Lang_FakeWinampLangHInst(existingWLZ);
				FreeLibrary(templng);
			}
			return TRUE;
		}

		skinExit = 0;
		lpszCmdParam = CheckLang(lpszCmdParam, hwnd_other, &skinExit);
		// restore the language pack settings now that we've done the override and clean up as needed
		if(existingWLZ)
		{
			Lang_FakeWinampLangHInst(existingWLZ);
			FreeLibrary(templng);
		}

		if (skinExit)
			return TRUE;

		hSem = CreateSemaphore(0, 0, 65535, "WinampExplorerHack1");

		if (hSem && GetLastError() != ERROR_ALREADY_EXISTS)
		{
			bC = 1;
			if (!bAdd && !bBookmark && !bHandle)
			{
				SendMessage(hwnd_other, WM_WA_IPC, 0, IPC_DELETE_INT);
			}
		}
		if (hSem)
		{
			ReleaseSemaphore(hSem, 1, NULL);
			if (bBookmark)
			{
				static wchar_t tmp[MAX_PATH];
				StringCchPrintfW(tmp, MAX_PATH, L"/BOOKMARK %s", lpszCmdParam);
				lpszCmdParam = tmp;
			}
			else if (bHandle)
			{
				static wchar_t tmp[MAX_PATH];
				StringCchPrintfW(tmp, MAX_PATH, L"/HANDLE %s", lpszCmdParam);
				lpszCmdParam = tmp;
			}
			parseCmdLine(lpszCmdParam, hwnd_other);

			WaitForSingleObject(hSem, 5000);
			if (bC)
			{
				int n = 500;
				if (!bAdd && !bBookmark && !bHandle) SendMessage(hwnd_other, WM_WA_IPC, 0, IPC_STARTPLAY_INT);
				Sleep(200);
				while (1)
				{
					if (WaitForSingleObject(hSem, 100) == WAIT_TIMEOUT)
					{
						if (WaitForSingleObject(hSem, 900) == WAIT_TIMEOUT)
						{
							break;
						}
						else
						{
							ReleaseSemaphore(hSem, 1, NULL);
							n--;
						}
					}
					else
					{
						ReleaseSemaphore(hSem, 1, NULL);
						Sleep(100);
						n--;
					}
				}
				bWait = 1;
			}
			CloseHandle(hSem);
		}
	}
	else // otherwise, just raise other winamp window
	{
		ShowWindow(hwnd_other, SW_RESTORE);
		SetForegroundWindow(hwnd_other);
	}
	return TRUE;
}

#if 0
void removeAOD  // this is so fucking uberleet I just odnt know what to do
{
	static unsigned char aod1[] = {'n' ^ 0xff, 'o' ^ 0xff, 'a' ^ 0xff, 'O' ^ 0xff, 'd' ^ 0xff, 0};
	int x;
	for (x = 0; x < sizeof(aod1) - 1; x ++) aod1[x] ^= 0xff;
	if (GetPrivateProfileInt("Winamp", (char*)aod1, 0, INI_FILE) == 666)
	{
		static unsigned char buf[] =
		{
			'S' ^ 0xB3, 'O' ^ 0xB3, 'F' ^ 0xB3, 'T' ^ 0xB3, 'W' ^ 0xB3, 'A' ^ 0xB3, 'R' ^ 0xB3, 'E' ^ 0xB3, '\\' ^ 0xB3,
			'A' ^ 0xB3, 'M' ^ 0xB3, 'E' ^ 0xB3, 'R' ^ 0xB3, 'I' ^ 0xB3, 'C' ^ 0xB3, 'A' ^ 0xB3, ' ' ^ 0xB3,
			'O' ^ 0xB3, 'N' ^ 0xB3, 'L' ^ 0xB3, 'I' ^ 0xB3, 'N' ^ 0xB3, 'E' ^ 0xB3, '\\' ^ 0xB3, 'A' ^ 0xB3, 'O' ^ 0xB3, 'D' ^ 0xB3,
			0xb3
		};
		for (x = 0; x < sizeof(buf); x ++) buf[x] ^= 0xb3; // a simple mask
		// get rid of zee aod
		doRemoveAOD(HKEY_CURRENT_USER, (char*)buf);
	}
}
#endif

DWORD CALLBACK MainThread(LPVOID param);
extern wchar_t vidoutbuf_save[1024];
static LPWSTR lpszCmdParam = 0;
static int bAdd = 0;
static int bBookmark = 0;
static int bHandle = 0;

void ShowSafeModeMessage(int mode)
{
	if (g_safeMode)
	{
		wchar_t title[256], message[512];
		MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
		if (!mode)
		{
			msgbx.lpszText = getStringW((g_safeMode == 2 ? IDS_SAFE_MODE_ALL : IDS_SAFE_MODE_NORMAL), message, 512);
			msgbx.lpszCaption = getStringW(IDS_START_SAFE_MODE, title, 256);
		}
		else
		{
			msgbx.lpszText = getStringW(IDS_FAILED_SAFE_MODE_MSG, message, 512);
			msgbx.lpszCaption = getStringW(IDS_FAILED_SAFE_MODE, title, 256);
		}
		msgbx.lpszIcon = MAKEINTRESOURCEW(102);
		msgbx.hInstance = hMainInstance;
		msgbx.dwStyle = MB_USERICON;
		MessageBoxIndirectW(&msgbx);
	}
}

#ifdef BETA
time_t inline get_compile_time(char const *time) { 
	char s_month[5] = {0};
    int day = 0, year = 0;
    struct tm t = {0};
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(time, "%s %d %d", s_month, &day, &year);

    t.tm_mon = ((strstr(month_names, s_month) - month_names) / 3);
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR unused /*lpszCmdParam*/, int nCmdShow)
{
	#ifdef BETA
	// gives ~4 weeks from a build compile to when it'll show this (should be enough time)
	time_t now = time(0), compile = get_compile_time(app_date);
	struct tm *tn = localtime(&now);
	tn->tm_sec = tn->tm_min = tn->tm_hour = 0;
	now = mktime(tn);

	if ((now - compile) >= 2678400)
	{
		MSGBOXPARAMSW msgbx = {
			sizeof(MSGBOXPARAMSW),
			0,
			GetModuleHandle(NULL),
			L"This beta version of Winamp is now over 4 weeks old.\n\n"
			L"Please update to the latest Winamp version available.",
			L"Winamp Beta Expired",
			MB_USERICON,
			MAKEINTRESOURCEW(102),
			0, 0, 0
		};
		MessageBoxIndirectW(&msgbx);
		ShellExecuteW(NULL, L"open", L"http://www.winamp.com", NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}
	#endif

	DWORD threadId;
	DWORD res = 0;
	HANDLE mainThread;
	HWND hwnd_other = NULL;

	void *refCounter = InitAppRefCounterObject(GetCurrentThreadId());
	//SHSetInstanceExplorer((IUnknown *)refCounter);

	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	// this will allow us to run on Win2k / XP prior to SP1 where this function isn't present
	typedef BOOL (WINAPI *SETDLLDIRECTORYW)(LPCWSTR lpPathName);
	HINSTANCE hModule = LoadLibrary("kernel32.dll");
	SETDLLDIRECTORYW LoadedSetDllDirectoryW = (BOOL (WINAPI*)(LPCWSTR))GetProcAddress(hModule, "SetDllDirectoryW");
	if(LoadedSetDllDirectoryW != NULL){
		LoadedSetDllDirectoryW(L""); /* don't load from working directory !!!!! */
	}
	hMainInstance = hInstance;

	/* Skip the executable name in the commandline */
	lpszCmdParam = GetCommandLineW();
	lpszCmdParam = FindNextCommand(lpszCmdParam);

	BuildAppName();

	init_config();
	LoadPathsIni();
	lpszCmdParam = ParseParameters(lpszCmdParam, &bAdd, &bBookmark, &bHandle);

	CoInitializeEx(0,COINIT_MULTITHREADED);

	setup_config();

	if (0 == (128 & is_install))
	{
		hwnd_other = find_otherwinamp(lpszCmdParam);
		if (hwnd_other)
		{
			// unable to start safe mode so inform the user
			ShowSafeModeMessage(1);
			int x = PassToOtherWinamp(lpszCmdParam, hwnd_other, bAdd, bBookmark, bHandle);
			CoUninitialize();
			return x;
		}
	}

	// unable to start safe mode so inform the user
	ShowSafeModeMessage(0);

	mainThread = CreateThread(0, 0, MainThread, (LPVOID)nCmdShow, 0, &threadId);

	while (!AppRefCount_CanQuit())
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx(1, &mainThread, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		if (dwStatus == WAIT_OBJECT_0+1)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return msg.wParam;
				DispatchMessage(&msg);
			}
		}
		else if (dwStatus == WAIT_OBJECT_0)
		{
			GetExitCodeThread(mainThread, &res);
			CloseHandle(mainThread);
			AppRefCount_Release();
		}
	}
	return 0;
}

static BOOL LoadWMFilter()
{
	if (!changeWMLoadTried) 
	{
		user32Lib = LoadLibrary("user32.dll");
		if (user32Lib) 
			changeWMFilter = (CHANGEWINDOWMESSAGEFILTER)GetProcAddress(user32Lib, "ChangeWindowMessageFilter");
		changeWMLoadTried = TRUE;
	}
	
	return user32Lib && changeWMFilter;
}

VOID CALLBACK PrefsShowProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	if (IsWindow(prefs_hwnd)) SetForegroundWindow(prefs_hwnd);
}

DWORD CALLBACK MainThread(LPVOID param)
{
	WPARAM exitParam;
	//LONG threadRefCount=0;
	//IUnknown *threadRef=0;

	int auditx = 0;

	int nCmdShow = (int)param;
	
//	SHCreateThreadRef(&threadRefCount, &threadRef);
//	SHSetThreadRef(threadRef);

	language_pack_instance = hMainInstance;

	playlistStr[0] = 0;
	playlistStr[18] = 0; // keep the last byte null terminated (and don't overwrite) so we can be smoewhat thread-safe (may have junk data, but it won't read outside the array)

	vidoutbuf_save[0] = 0;
	vidoutbuf_save[1023] = 0; // keep the last byte null terminated (and don't overwrite) so we can be smoewhat thread-safe (may have junk data, but it won't read outside the array)

	mainThreadId = GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS);

	InitializeCriticalSection(&embedcs);

	for (auditx = 0; auditx < AUDITSIZE; auditx++) audits[auditx] = NULL;
	CoInitialize(0);
	Wasabi_Load();
	plstring_init();
	/*Browser_Create();*/

	if ((128 & is_install) || (!g_noreg && GetPrivateProfileIntW(L"WinampReg", L"NeedReg", 1, INI_FILE)))
	{
		is_install = 128; //  nothing else
		Setup_RegisterService();
	}

//#define ERROR_FEEDBACK for test purposes only

#ifdef ERROR_FEEDBACK
	{
		HMODULE hm;
		wchar_t crasherDll[MAX_PATH*2];
		PathCombineW(crasherDll, PLUGINDIR, L"gen_crasher.dll");
		hm = LoadLibraryW(crasherDll);
		if (!hm)
		{
			PathCombineW(crasherDll, PLUGINDIR, L"gen_talkback.dll");
			hm = LoadLibraryW(crasherDll);
		}
		if (hm)
		{
			int (__cdecl* StartHandler)(wchar_t *iniPath) = NULL;
			*(FARPROC*)&StartHandler = GetProcAddress(hm, "StartHandler");
			if (StartHandler)
			{
				wchar_t iniPath[260];
				if(SUCCEEDED(StringCchPrintfW(iniPath, MAX_PATH, L"%s\\Plugins", CONFIGDIR)))
				{
					StartHandler(iniPath);
				}
			}
		}
	}
#endif  // ERROR_FEEDBACK

	draw_firstinit();

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");
	g_scrollMsg = RegisterWindowMessage("MSWHEEL_ROLLMSG");
	
	WM_TASKBARBUTTONCREATED = RegisterWindowMessageW(L"TaskbarButtonCreated");
	
	if (LoadWMFilter()) 
	{
		changeWMFilter(WM_TASKBARBUTTONCREATED, 1/*MSGFLT_ADD*/);
		changeWMFilter(WM_DWMSENDICONICTHUMBNAIL, 1/*MSGFLT_ADD*/);
		changeWMFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, 1/*MSGFLT_ADD*/);
		changeWMFilter(WM_COMMAND, 1/*MSGFLT_ADD*/);  //for thumbnail toolbar buttons
	}

	InitCommonControls();
	CommandLink_RegisterClass(hMainInstance);

	Skin_CleanupAfterCrash();
	Lang_CleanupAfterCrash();

	{
		int langExit = 0;
		lpszCmdParam = CheckLang(lpszCmdParam, 0, &langExit);
		if (langExit)
		{
			Lang_EndLangSupport();
			Lang_CleanupZip();
			return TRUE;
		}
	}

	if (!g_safeMode)
	{
		language_pack_instance = Lang_InitLangSupport(hMainInstance, WinampLangGUID);
		Lang_FollowUserDecimalLocale();
	}

	if (bBookmark)
	{		
		w5s_init();
		in_init();
		Bookmark_AddCommandline(lpszCmdParam);
		in_deinit();
		w5s_deinit();
		Wasabi_Unload();
		RemoveRegistrar();
		ExitProcess(0);
	}

	{
		int skinExit = 0;
		lpszCmdParam = CheckSkin(lpszCmdParam, 0, &skinExit);
		if (skinExit)
		{
			return TRUE;
		}
	}

	{
		// remove general purpose plug-in (if set)
		wchar_t buf[1024];
		buf[0] = 0;
		_r_sW("remove_genplug", buf, 1024);
		if (buf[0])
		{
			IFileTypeRegistrar *registrar=0;
			if (GetRegistrar(&registrar) == 0 && registrar)
			{
				registrar->DeleteItem(buf);
				registrar->Release();
			}

			_w_s("remove_genplug", 0);
		}
	}

	fft_init();
	SpectralAnalyzer_Create();
	JSAPI1_Initialize();
	stats_init();

	verify_reginfo();

	w5s_init();
	in_init();
	out_init();
	vis_init();

	if (*lpszCmdParam && !bAdd && !bHandle) config_read(1);
	else config_read(0);

	CreateEQPresets();

	if (is_install)
		DoInstall(is_install);

	reg_associated_filetypes(0);

	if (config_splash) splashDlg(SPLASH_DELAY);	// display splash screen if desired

	PlayList_getcurrent(FileName, FileTitle, FileTitleNum); // update filename and filetitle if a list was loaded

	songChangeBroadcastMessage = RegisterWindowMessageW(L"WinampSongChange");

	if (!InitApplication(hMainInstance))
	{
		LPMessageBox(NULL, IDS_ERRORINIT, IDS_ERROR, MB_OK);
		return (FALSE);
	}

	if (!InitInstance(hMainInstance, nCmdShow))
	{
		LPMessageBox(NULL, IDS_ERRORINIT, IDS_ERROR, MB_OK);
		return (FALSE);
	}

	if (!bHandle)
	{
		if (*lpszCmdParam) // if command line parameters, parse them
		{
			parseCmdLine(lpszCmdParam, 0);
			plEditRefresh();
			{
				if (config_shuffle) PlayList_randpos(-BIGINT);
				if (!bAdd)
					bStartPlaying = 1;
			}
		}
		else // otherwise, we're using our loaded playlist
		{
			if (config_shuffle) PlayList_randpos(-BIGINT);
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		}
	}

	//SetCurrentDirectoryW(config_cwd);

	plEditSelect(PlayList_getPosition());

	Ole_initDragDrop();
	
	if (!(GetAsyncKeyState(VK_RCONTROL)&0x8000) || !(GetAsyncKeyState(VK_LCONTROL)&0x8000))
	{
		load_genplugins(); // load general purpose plugins

		if (!Skin_Check_Modern_Support())
		{
			wchar_t msg[512];
			StringCchPrintfW(msg, 512, getStringW(IDS_NO_MODERN_SKIN_SUPPORT, NULL, 0), config_skin);
			MessageBoxW(NULL, msg, getStringW(IDS_SKIN_LOAD_ERROR, NULL, 0), MB_ICONWARNING | MB_OK | MB_TOPMOST);
		}
	}

	//disable video menu if no video plugins are present
	if (!g_has_video_plugin)
	{
		RemoveMenu(main_menu, WINAMP_OPTIONS_VIDEO, MF_BYCOMMAND);
		g_mm_optionsbase_adj -= 1;
	}

	set_aot(0); // in case our gen plugins did anything fun
	set_priority();

	{
		int v = _r_i("show_prefs", 0);
		if (v != 0)
		{
			if (v > 0) prefs_last_page = v;
			_w_i("show_prefs", 0);
			PostMessage(hMainWindow,WM_COMMAND,WINAMP_OPTIONS_PREFS,0);
			SetTimer(hMainWindow, 969, 1, PrefsShowProc);
		}
	}

	if (bStartPlaying)
	{
		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		SendMessage(hMainWindow, WM_COMMAND, WINAMP_BUTTON2, 0);
		//SendMessage(hMainWindow,WM_WA_IPC,0,IPC_STARTPLAY);
		draw_paint(NULL);
	}
	WADlg_init(hMainWindow);
#ifdef BENSKI_TEST_WM_PRINTCLIENT
	SetTimer(hMainWindow, 9999, 10000, 0);
#endif
	if (bHandle && *lpszCmdParam)
	{
		PostMessage(hMainWindow, WM_WA_IPC, (WPARAM)lpszCmdParam, IPC_HANDLE_URI);
		//HandleFilename(lpszCmdParam);
	}
	exitParam = WinampMessageLoop();
	JSAPI1_Uninitialize();
	unload_genplugins();
	w5s_deinit();
	stats_save();
	SpectralAnalyzer_Destroy();
	/*Browser_Destroy();*/

	Ole_uninitDragDrop();
	if (g_restartonquit)
	{
		char buf[MAX_PATH] = "\"";
		STARTUPINFO si = {sizeof(si), };
		PROCESS_INFORMATION pi;
		GetModuleFileName(NULL, buf + 1, sizeof(buf) - 1);
		StringCchCat(buf, MAX_PATH, "\"");
		if (g_restartonquit == 2) StringCchCat(buf, MAX_PATH, " /SAFE=1");
		CreateProcess(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}

	RemoveRegistrar();
	Wasabi_Unload();
	CoUninitialize();
	//ExitProcess(exitParam);
	return exitParam;
} // WinMain

void MoveOffscreen(HWND hwnd)
{
	RECT r;
	GetWindowRect(hwnd, &r);
	SetWindowPos(hwnd, 0, r.left, OFFSCREEN_Y_POS, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

int g_showcode, deferring_show = 0;
#include "../gen_ml/ml_ipc.h"
extern librarySendToMenuStruct mainSendTo = {0};

// Main Winamp window procedure
// we use message crackers when available, write our own for the ones that aren't
LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == g_scrollMsg)
	{
		wParam <<= 16; uMsg = WM_MOUSEWHEEL;
	}
	if (uMsg == WM_TASKBARCREATED)
	{
		if (systray_intray)
		{
			systray_restore();
			systray_minimize(caption);
		}
		return 0;
	}
	if (uMsg == WM_TASKBARBUTTONCREATED)
	{
		OnTaskbarButtonCreated();
		return 0;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}
	
	switch (uMsg)
	{
	case WM_INITMENUPOPUP:
	{
		HMENU hMenu = (HMENU)wParam;
		if (wParam && hMenu == mainSendTo.build_hMenu && mainSendTo.mode == 1)
		{
			int IPC_LIBRARY_SENDTOMENU = SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
				mainSendTo.mode = 2;
		}
		if (config_usecursors && !disable_skin_cursors)
		{
			if (Skin_Cursors[2]) SetCursor(Skin_Cursors[2]);
			else SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else SetCursor(LoadCursor(NULL, IDC_ARROW));

		if (hMenu == main_menu)
		{
			MENUITEMINFOW i = {sizeof(i), };
			i.fMask = MIIM_TYPE;
			i.fType = MFT_STRING;
			i.dwTypeData = (g_regver > 0 ? getStringW(IDS_WINAMP_PRO_MENUITEM,NULL,0) : getStringW(IDS_WINAMP_MENUITEM,NULL,0));
			i.cch = wcslen(i.dwTypeData);
			SetMenuItemInfoW(main_menu, 0, TRUE, &i);
			EnableMenuItem(main_menu, WINAMP_FILE_QUIT, MF_BYCOMMAND | (g_exit_disabled ? MF_GRAYED : MF_ENABLED));
		}
		else if (hMenu == g_submenus_play && g_audiocdletters)
		{
			for (int i = 0; i < g_audiocdletters; i++)
			{
				MENUITEMINFOW mii = {sizeof(mii),};
				wchar_t str[64] = {0}, tmp[64] = {0};
				int old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
				DWORD system_flags = 0, max_file_len = 0;
				wchar_t drives[4] = {L" :\\"}, vol_buf[40] = {0}, empty[64] = {0};
				drives[0] = g_audiocdletter[i];

				GetVolumeInformationW(drives,vol_buf,sizeof(vol_buf),0,&max_file_len,&system_flags,0,0);
				SetErrorMode(old_error_mode);

				getStringW(IDS_EMPTY,empty,64);
				StringCchPrintfW(str, 256, getStringW(IDS_AUDIO_CD,tmp,64),g_audiocdletter[i],(vol_buf[0]?vol_buf:empty));

				mii.fMask = MIIM_STRING;
				mii.dwTypeData = str;
				mii.cch = wcslen(str);
				mii.wID = ID_MAIN_PLAY_AUDIOCD + i;
				SetMenuItemInfoW(hMenu, ID_MAIN_PLAY_AUDIOCD + i, FALSE, &mii);
			}
		}
		else if (hMenu == g_submenus_bookmarks1 || hMenu == g_submenus_bookmarks2)
		{
			MENUITEMINFOW i = {sizeof(i), };
			FILE *fp = 0;
			int a = 34768;
			int offs = 3;
			int count = GetMenuItemCount(hMenu) + 1;
            if (hMenu != g_submenus_bookmarks1) offs = 0;

			i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
			i.fType = MFT_STRING;
			i.wID = 34768;

			// this will remove the "(no bookmarks)" item from Main menu->Play->Bookmsrk
			// if it still exists -> removed as of 5.55 since we handle the menu better.
			if(!offs) RemoveMenu(hMenu, ID_MAIN_PLAY_BOOKMARK_NONE, MF_BYCOMMAND);

			// remove all of the items we might have added - do by command for certainty
			while (count){
				if(!RemoveMenu(hMenu, a++, MF_BYCOMMAND)) break;
				count--;
			}

			fp = _wfopen(BOOKMARKFILE8, L"rt");
			if (fp)
			{
				while (1)
				{
					char ft[4096], fn[MAX_PATH];
					fgets(fn, MAX_PATH, fp);
					if (feof(fp)) break;
					fgets(ft, 4096, fp);
					if (feof(fp)) break;
					if (ft[0] && fn[0])
					{
						if (fn[lstrlen(fn) - 1] == '\n') fn[lstrlen(fn) - 1] = 0;
						if (ft[lstrlen(ft) - 1] == '\n') ft[lstrlen(ft) - 1] = 0;
						if (ft[0] && fn[0])
						{
							i.dwTypeData = AutoWideDup(ft, CP_UTF8);
							i.cch = lstrlenW(i.dwTypeData);
							RemoveMenu(hMenu, i.wID, MF_BYCOMMAND);
							InsertMenuItemW(hMenu, i.wID + offs - 34768, TRUE, &i);
							i.wID++;
						}
					}
				}
				fclose(fp);
			}

			g_BookmarkTop = i.wID;

			// put in a place holder item if there were no read bookmarks
			if (g_BookmarkTop == 34768)
			{
				i.dwTypeData = getStringW(IDS_NO_BOOKMARKS,NULL,0);
				i.cch = lstrlenW(i.dwTypeData);
				InsertMenuItemW(hMenu, i.wID + offs - 34768, TRUE, &i);
				EnableMenuItem(hMenu, i.wID, MF_BYCOMMAND | MF_GRAYED);
			}
		}
		else if (hMenu == g_submenus_skins1 || hMenu == g_submenus_skins2)
		{
			MENUITEMINFOW i = {sizeof(i), };
			HANDLE h;
			WIN32_FIND_DATAW d;
			wchar_t dirmask[MAX_PATH];
			int a = 0, mod_got = 0, bento_got = 0, checked = 0;

			i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE;
			i.fType = MFT_STRING;
			i.wID = 32768;
			PathCombineW(dirmask, SKINDIR, L"*");

			if (!config_skin[0])
				CheckMenuItem(hMenu, 32767, MF_CHECKED);
			else
				CheckMenuItem(hMenu, 32767, MF_UNCHECKED);

			h = FindFirstFileW(dirmask, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) continue;
					if ((d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
					    !_wcsicmp(extensionW(d.cFileName), L"zip") ||
					    !_wcsicmp(extensionW(d.cFileName), L"wal") ||
					    !_wcsicmp(extensionW(d.cFileName), L"wsz"))
					{
						if (!_wcsicmp(config_skin, d.cFileName))
						{
							i.fState = MFS_CHECKED | MFS_ENABLED;
							checked = 1;
						}
						else
						{
							i.fState = MFS_UNCHECKED | MFS_ENABLED;
						}
						if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							i.dwItemData = 0;
						else
						{
							if (!_wcsicmp(extensionW(d.cFileName), L"zip"))
								i.dwItemData = 1;
							else if (!_wcsicmp(extensionW(d.cFileName), L"wal"))
								i.dwItemData = 4;
							else
								i.dwItemData = 2;
							extensionW(d.cFileName)[ -1] = 0;
						}
						i.dwTypeData = d.cFileName;
						i.cch = wcslen(d.cFileName);
						if (!a) if (!RemoveMenu(hMenu, i.wID + 4 - 32768, MF_BYPOSITION)) a = 1;
						if (!i.dwItemData && !_wcsicmp(d.cFileName, MODERN_SKIN_NAME))
						{
							mod_got = 1;
							InsertMenuItemW(hMenu, 4, TRUE, &i);
						}
						else if (!_wcsicmp(d.cFileName, BENTO_SKIN_NAME))
						{
							// place below classic + modern (if it exists)
							bento_got = 1;
							InsertMenuItemW(hMenu, 4 + mod_got, TRUE, &i);
						}
						else if (!_wcsicmp(d.cFileName, BIG_BENTO_SKIN_NAME))
						{
							// place below classic + modern + normal bento (if it exists)
							InsertMenuItemW(hMenu, 4 + mod_got + bento_got, TRUE, &i);
						}
						else
							InsertMenuItemW(hMenu, i.wID + 4 - 32768, TRUE, &i);
						i.wID++;
					}
				}
				while (i.wID < 34700 && FindNextFileW(h, &d));
				FindClose(h);
				g_SkinTop = i.wID;
				while (!a) if (!RemoveMenu(hMenu, i.wID++ + 4 - 32768, MF_BYPOSITION)) a = 1;
				if (!checked) CheckMenuItem(hMenu, 32767, MF_CHECKED);
			}
		}
		else if ((hMenu == g_submenus_lang) && config_wlz_menu)
		{
			MENUITEMINFOW i = {sizeof(i), };
			HANDLE h;
			WIN32_FIND_DATAW d;
			wchar_t dirmask[MAX_PATH];
			int a = 0;

			i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE;
			i.fType = MFT_STRING;
			i.wID = 34700;
			PathCombineW(dirmask, LANGDIR, L"*");

			i.dwTypeData = L"English (US)";
			i.cch = wcslen(i.dwTypeData);
			InsertMenuItemW(hMenu, i.wID - 34700, TRUE, &i);
			i.wID++;

			CheckMenuItem(hMenu, 34700, (!config_langpack[0]?MF_CHECKED:MF_UNCHECKED));

			h = FindFirstFileW(dirmask, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) continue;
					if (!_wcsicmp(extensionW(d.cFileName), L"wlz")) {
						if (!_wcsicmp(config_langpack, d.cFileName))
						{
							i.fState = MFS_CHECKED | MFS_ENABLED;
						}
						else
						{
							i.fState = MFS_UNCHECKED | MFS_ENABLED;
						}
						extensionW(d.cFileName)[ -1] = 0;
						i.dwTypeData = d.cFileName;
						i.cch = wcslen(d.cFileName);
						if (!a) if (!RemoveMenu(hMenu, i.wID - 34700, MF_BYPOSITION)) a = 1;
						InsertMenuItemW(hMenu, i.wID - 34700, TRUE, &i);
						i.wID++;
					}
				}
				while (i.wID < 34800 && FindNextFileW(h, &d));
				FindClose(h);
				g_LangTop = i.wID;
				while (!a) if (!RemoveMenu(hMenu, i.wID++ - 34700, MF_BYPOSITION)) a = 1;
			}
		}
		break;
	}
	case WM_DISPLAYCHANGE: Main_OnDisplayChange(hwnd); break;
	case WM_WA_SYSTRAY: return (Main_OnWASystray(hwnd, (int)LOWORD(lParam)) ? 0 : -1L);
	case WM_WA_MPEG_EOF: return (Main_OnWAMPEGEOF(hwnd) ? 0 : -1L); // sent by decode thread
	case WM_WA_IPC: return (Main_OnIPC(hwnd, lParam, (int)(DWORD)wParam));
		HANDLE_MSG(hwnd, WM_COMMAND, Main_OnCommand);
		HANDLE_MSG(hwnd, WM_SYSCOMMAND, Main_OnSysCommand);
		HANDLE_MSG(hwnd, WM_CREATE, Main_OnCreate);
		HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette);
		HANDLE_MSG(hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged);
		HANDLE_MSG(hwnd, WM_SIZE, Main_OnSize);
		HANDLE_MSG(hwnd, WM_DROPFILES, Main_OnDropFiles);
		HANDLE_MSG(hwnd, WM_TIMER, Main_OnTimer);
		HANDLE_MSG(hwnd, WM_PAINT, draw_paint);
	case WM_PRINTCLIENT:
		draw_printclient((HDC)wParam, lParam);
		return 0;
		HANDLE_MSG(hwnd, WM_RBUTTONUP, Main_OnRButtonUp);
		HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, Main_OnLButtonDblClk);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, Main_OnLButtonUp);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, Main_OnLButtonDown);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, Main_OnMouseMove);
	case WM_CAPTURECHANGED: return Main_OnCaptureChanged((HWND)lParam);
		HANDLE_MSG(hwnd, WM_DESTROY, Main_OnDestroy);
		HANDLE_MSG(hwnd, WM_CLOSE, Main_OnClose);
	case WM_NCACTIVATE:
	{
		LRESULT result = (LRESULT)(DWORD)(BOOL)(Main_OnNCActivate)((hwnd), (BOOL)(wParam), (HWND)(lParam), 0L);
		if (IsIconic(hwnd))
			break;
		else
			return result;
	}
	case WM_NCHITTEST:
		if (IsIconic(hwnd))
			break;
		return (LRESULT)(DWORD)(UINT)(Main_OnNCHitTest)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
	case WM_NCCALCSIZE:
		if (IsIconic(hwnd))
			break;
		return (LRESULT)(DWORD)(UINT)Main_OnNCCalcSize(hwnd, (BOOL)wParam, (NCCALCSIZE_PARAMS *)lParam);

		HANDLE_MSG(hwnd, WM_ENDSESSION, Main_OnEndSession);
	case WM_QUERYENDSESSION:
		return !!SendMessage(hwnd, WM_WA_IPC, 0, IPC_HOOK_OKTOQUIT);
	case WM_KEYDOWN:
	{
		static int pos;
		TCHAR buf[2] = {(TCHAR)wParam, 0};
		CharUpperBuff(buf, 1);

		if (buf[0] == eggstr[pos])
		{	
			eggTyping = TRUE;
			if (!eggstr[++pos])
			{
				eggTyping = FALSE;
				eggstat = !eggstat;
				pos = 0;
				draw_tbar(1, config_windowshade, eggstat);
			}
		}
		else pos = 0;
		break;

	}
	case WM_NCPAINT: return 0;
	case WM_COPYDATA: return Main_OnCopyData((HWND)wParam, (COPYDATASTRUCT *)lParam);
	case WM_GETTEXT: return Main_OnGetText((wchar_t *)lParam, (int)wParam);
	case WM_NOTIFY:
		{
			LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
			if(tt->hdr.hwndFrom = hTooltipWindow)
			{
				switch (tt->hdr.code)
				{
					case TTN_SHOW:
						SetWindowPos(tt->hdr.hwndFrom,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
						break;
					case TTN_NEEDTEXTW:
					{
						LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
						wchar_t booga[81];
						GetWindowTextW(hwnd, booga, 79);
						booga[79] = 0;
						StringCchCopyW(tt->szText, 80, booga);
						tt->lpszText = tt->szText;
						return 0;
					}
				}
			}
		}
		break;
	case WM_SETFOCUS:
		if (!config_mw_open)
		{
			if (config_pe_open) SetForegroundWindow(hPLWindow);
			else if (config_eq_open) SetForegroundWindow(hEQWindow);
			// else if (config_mb_open) SetForegroundWindow(hMBWindow);
			else if (config_video_open) SetForegroundWindow(hVideoWindow);
			else
			{
				EnterCriticalSection(&embedcs);
				{
					embedWindowState *p = embedwndlist;
					while (p)
					{
						if (IsWindowVisible(p->me))
						{
							SetForegroundWindow(p->me); break;
						}
						p = p->link;
					}
				}
				LeaveCriticalSection(&embedcs);
			}
		}
		else
			{}
		break;
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam), dLines;
		// if the delta changes then ignore prior carryover
		// hopefully this will go with the expected action.
		if(zDelta < 0 && main_delta_carryover > 0 ||
		   zDelta > 0 && main_delta_carryover < 0)
		{
			main_delta_carryover = 0;
		}
		// otherwise add on the carryover from the prior message
		else zDelta += main_delta_carryover;

		if (0 == (MK_MBUTTON & LOWORD(wParam)))
			zDelta *= 2;

		dLines = zDelta / WHEEL_DELTA;
		main_delta_carryover = zDelta - dLines * WHEEL_DELTA;

		if (0 != dLines)
		{
			zDelta = (dLines > 0) ? dLines : -dLines;

			if (0 != (MK_MBUTTON & LOWORD(wParam)))
			{
				if (dLines >= 0) dLines = WINAMP_FFWD5S;
				else dLines = WINAMP_REW5S;
			}
			else
			{
				if (dLines >= 0) dLines = WINAMP_VOLUMEUP;
				else dLines = WINAMP_VOLUMEDOWN;
			}

			while (zDelta--)
			{
				SendMessage(hwnd, WM_COMMAND, dLines, 0);
			}
		}
	}
	break;
	case WM_DWMSENDICONICTHUMBNAIL:
		{
			int x = HIWORD(lParam);
			int y = LOWORD(lParam);
			OnIconicThumbnail(x, y);
		}
		break;
#if 0
	case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
		{
			//MessageBoxA(NULL, "winamp/live", "winamp/live", MB_OK);
			OnThumbnailPreview();
		}
		break;
#endif
	case WM_MOVE:
#if 0
		if ((int)LOWORD(lParam) < 32768 && (int)HIWORD(lParam) < 32768)
		{
			if (/*(int)LOWORD(lParam) != 3000 && */(int)HIWORD(lParam) != OFFSCREEN_Y_POS)
			{
				if ((int) LOWORD(lParam) != config_wx ||
				    (int) HIWORD(lParam) != config_wy)
					if (config_keeponscreen&1)
					{
						config_wx = (int) LOWORD(lParam);
						config_wy = (int) HIWORD(lParam);
						set_aot(1);
					}
			}
		}
#endif
		break;
	case WM_SETCURSOR:
	
		switch(HIWORD(lParam))
		{
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_XBUTTONDOWN:
				DisabledWindow_OnMouseClick(hwnd);
				break;
		}
		if (config_usecursors && !disable_skin_cursors)
		{
			if ((HWND)wParam == hMainWindow && HIWORD(lParam) == WM_MOUSEMOVE) ui_handlecursor();
			return TRUE;
		}
		else SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	}
	return (DefWindowProcW(hwnd, uMsg, wParam, lParam));
}

static LRESULT Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{

	//  char buf[512];
	//  wsprintf(buf,"got WM_SYSCOMMAND %08x\n",cmd);
	//  OutputDebugString(buf);
	// video
	if (((cmd & 0xfff0) == SC_SCREENSAVE || (cmd & 0xfff0) == SC_MONITORPOWER) && config_video_noss && video_isVideoPlaying())
	{
		return -1;
	}

	if (!Main_OnCommand(hwnd, cmd, (HWND) x, (UINT) y))
		FORWARD_WM_SYSCOMMAND(hwnd, cmd, x, y, DefWindowProcW);
	return 1;
}

static LRESULT WINAPI browseCheckBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		if (!(config_rofiob&2))	CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);
	}
	if (uMsg == WM_COMMAND)
	{
		if (LOWORD(wParam) == IDC_CHECK1)
		{
			config_rofiob &= ~2;
			if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK1)) config_rofiob |= 2;
			config_write(0);
		}
	}
	return 0;
}

int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
	{
		HWND h;
		SetWindowTextW(hwnd, getStringW((!lpData?IDS_OPENDIR:IDS_ADD_FOLDER), NULL, 0));
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)config_cwd);

		h = FindWindowExW(hwnd, NULL, NULL, L"__foo");
		if (h) ShowWindow(h, SW_HIDE);
		h = LPCreateDialogW(IDD_BROWSE_RECDLG, hwnd, browseCheckBoxProc);
		SetWindowPos(h, 0, 4, 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		ShowWindow(h, SW_SHOWNA);
	}
	}
	return 0;
}

LRESULT sendMlIpc(int msg, WPARAM param)
{
	static LRESULT IPC_GETMLWINDOW;
	//static
	HWND mlwnd;

	if (!IPC_GETMLWINDOW) IPC_GETMLWINDOW = SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"LibraryGetWnd", IPC_REGISTER_WINAMP_IPCMESSAGE);
	//if (!mlwnd || (int)mlwnd == 1)
	mlwnd = (HWND)SendMessage(hMainWindow, WM_WA_IPC, 0, IPC_GETMLWINDOW);

	if (param == 0 && msg == 0) return (LRESULT)mlwnd;

	if (mlwnd != NULL)
		return SendMessage(mlwnd, WM_ML_IPC, param, msg);

	return 0;
}

void tealike_crappy_code(unsigned long v[2], unsigned long k[4])
{
	unsigned long y = v[0], z = v[1], sum = 0,              /* set up */
	                                        delta = 0x9e3779b9UL, n = 32 ;  /* key schedule constant*/

	while (n-- > 0)
	{
		/* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^(z + sum) ^((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^(y + sum) ^((y >> 5) + k[3]);   /* end cycle */
	}
	v[0] = y; v[1] = z;

}




// command line parsing, for IPC or normal modes
// goes to a lot of trouble to look for "'s.


HWND find_otherwinamp(wchar_t *cmdline)
{
	int y = 0;
	wchar_t buf[MAX_PATH];

	StringCchPrintfW(buf, MAX_PATH, L"%s_%x_CLASS", szAppName, APP_VERSION_NUM);
again:
	g_hEventRunning = CreateEventW(0, 1, 0, buf);
	if (g_hEventRunning && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		int x;
		CloseHandle(g_hEventRunning);
		g_hEventRunning = 0;
		// check for window for 4s, then give up
		if (!bNoHwndOther && (!config_minst || *cmdline)) for (x = 0; x < 40; x ++)
			{
				HWND lhwnd = NULL;
				int failed = 0;
				while ((lhwnd = FindWindowExW(NULL, lhwnd, szAppName, NULL)))
				{
					DWORD_PTR vn = 0; //APP_VERSION_NUM
					if (lhwnd == hMainWindow)
						continue;
					if (!SendMessageTimeout(lhwnd, WM_WA_IPC, 0, IPC_GETVERSION, SMTO_NORMAL, 5000, &vn))
					{
						failed = 1;
					}
					else if (vn == APP_VERSION_NUM) return lhwnd;
				}
				if (failed) return NULL; // no valid winamp windows, but one that fucked up

				Sleep(100);
			}
		if (y++ < 20) goto again;
	}
	return NULL;
}


// returns 0 if showing/hiding sould be aborted
int Ipc_WindowToggle(INT_PTR which, INT_PTR how)
{
	return SendMessage(hMainWindow, WM_WA_IPC, which, how ? IPC_CB_ONSHOWWND : IPC_CB_ONHIDEWND);
}
//}

#ifdef BENSKI_TEST_WM_PRINTCLIENT
static void PrintWindow(HWND hWnd)
{
	HDC hDCMem = CreateCompatibleDC(NULL);
	HBITMAP hBmp = NULL;
	RECT rect;

	GetWindowRect(hWnd, & rect);
	{
		HDC hDC = GetDC(hWnd);
		hBmp = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
		ReleaseDC(hWnd, hDC);
	}
	{
		HGDIOBJ hOld = SelectObject(hDCMem, hBmp);
		SendMessage(hWnd, WM_PRINT, (WPARAM) hDCMem, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);

		SelectObject(hDCMem, hOld);
	}
	DeleteObject(hDCMem);

	OpenClipboard(hWnd);

	EmptyClipboard(); 
	SetClipboardData(CF_BITMAP, hBmp);
	CloseClipboard();
}
#endif