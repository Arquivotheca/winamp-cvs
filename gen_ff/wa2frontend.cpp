#include <precomp.h>
/*------------------------------------------------------------------------------------------------
 Winamp 2.9/5 frontend class
------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "wa2frontend.h"
#include "../winamp/frontend.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "../gen_ml/ml.h"
#include "../gen_ml/main.h"
#include "../gen_hotkeys/wa_hotkeys.h"
#include "../nu/AutoChar.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"

#define WINAMP_EDIT_ID3                 40188


Winamp2FrontEnd wa2;

void Winamp2FrontEnd::init(HWND hwndParent)
{
	// find Winamp's HWND so we can talk to it
	hwnd_winamp = hwndParent; //FindWindow("Winamp v1.x",NULL);

	// check that hwnd_winamp isnt null and that we know about this specific ipc version
	getVersion();

	// more init
	enabled = 1;
	visible = 1;
	foundvis = 0;

	got_length_cache = 0;
	cached_length = -1;
	cached_length_time = 0;
	got_pos_cache = 0;
	cached_pos = -1;
	cached_pos_time = 0;

	*(void **)&export_sa_get=(void*)SendMessage(hwnd_winamp,WM_WA_IPC,2,IPC_GETSADATAFUNC);
	*(void **)&export_sa_setreq=(void *)SendMessage(hwnd_winamp,WM_WA_IPC,1,IPC_GETSADATAFUNC);
	*(void **)&export_sa_get_deprecated=(void*)SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETSADATAFUNC);
	*(void **)&export_vu_get=(void*)SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETVUDATAFUNC);
	if (reinterpret_cast<intptr_t>(export_vu_get) == -1)
		export_vu_get=0;

	hwnd_playlist = getWnd(IPC_GETWND_PE);
	video_ideal_height = -1;
	video_ideal_width = -1;
}

//-----------------------------------------------------------------------------------------------
Winamp2FrontEnd::Winamp2FrontEnd()
{
	m_version = (char *)malloc(64);
	*m_version = 0;
	enabled=visible=0;
	direct_mouse_wheel_message = WM_NULL;
}

//-----------------------------------------------------------------------------------------------
Winamp2FrontEnd::~Winamp2FrontEnd()
{
	setWindowsVisible(1);
	free(m_version);
}

//-----------------------------------------------------------------------------------------------
const char *Winamp2FrontEnd::getVersion()
{
	if (hwnd_winamp == NULL)
	{
		char err[16];
		MessageBox(NULL, WASABI_API_LNGSTRING(IDS_COULD_NOT_FIND_WINAMP),
		           WASABI_API_LNGSTRING_BUF(IDS_ERROR,err,16), 0);
		m_version = "Winamp not found";
		return m_version;
	}

	if (hwnd_winamp == NULL)
		return NULL;

	if (*m_version == 0)
	{
		// get version number
		int version = SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETVERSION);
		// check that we support this version of the ipc
		//assert(((version & 0xFF) >> 8) > 0x20);
		// format the version number into a string
		wsprintf(m_version, "%d.%d%d", (version & 0xF000) >> 12, version & (0xF0 >> 4), version & 0xF);
	}
	return m_version;
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::userButton(int button, int modifier)
{
	if (hwnd_winamp == NULL) return;
	int mod = 0;
	switch (modifier)
	{
	case WA2_USERBUTTONMOD_NONE : break;
	case WA2_USERBUTTONMOD_SHIFT: mod = 100; break;
	case WA2_USERBUTTONMOD_CTRL : mod = 110; break;
	}
	switch (button)
	{
	case WA2_USERBUTTON_PREV: SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON1 + mod,0); break;
	case WA2_USERBUTTON_PLAY: SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON2 + mod,0); break;
	case WA2_USERBUTTON_PAUSE: SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON3 + mod,0); break;
	case WA2_USERBUTTON_STOP: SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON4 + mod,0); break;
	case WA2_USERBUTTON_NEXT: SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON5 + mod,0); break;
	}
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setOnTop(int ontop)
{
	if (!!ontop == !!isOnTop()) return;
	toggleOnTop();
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::toggleOnTop()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_AOT, 0);
}


//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::enqueueFile(const wchar_t *file)
{
	if (hwnd_winamp == NULL) return;
	COPYDATASTRUCT cds;
	cds.dwData = IPC_PLAYFILEW;
	cds.lpData = (void *)file;
	cds.cbData = sizeof(wchar_t) * (wcslen(file)+1);
	SendMessage(hwnd_winamp,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isPlaying()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_ISPLAYING) == 1;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isPaused()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_ISPLAYING) == 3;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isStopped()
{
	if (hwnd_winamp == NULL) return 1;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_ISPLAYING) == 0;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getPosition()
{
	if (hwnd_winamp == NULL) return 0;
	if (got_pos_cache && GetTickCount() < cached_pos_time + 20) return cached_pos;
	got_pos_cache=1;
	cached_pos_time=GetTickCount();
	return cached_pos=SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETOUTPUTTIME);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getLength()
{
	if (got_length_cache && GetTickCount() < cached_length_time + 3000) return cached_length;
	if (hwnd_winamp == NULL) return 0;
	int l = SendMessage(hwnd_winamp,WM_WA_IPC,1,IPC_GETOUTPUTTIME);
	if (l == -1) cached_length=-1;
	else cached_length=l*1000;
	got_length_cache = 1;
	cached_length_time = GetTickCount();
	return cached_length;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::seekTo(int ms)
{
	got_pos_cache = 0;
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,ms,IPC_JUMPTOTIME);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setVolume(int v)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,v,IPC_SETVOLUME);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getVolume()
{
	if (hwnd_winamp == NULL) return 255;
	return SendMessage(hwnd_winamp,WM_WA_IPC,-666,IPC_SETVOLUME);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setPanning(int p)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,p,IPC_SETPANNING);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getPanning()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,-666,IPC_SETPANNING);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getInfo(int what)
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,what,IPC_GETINFO);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getEqData(int what)
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,what,IPC_GETEQDATA);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setEqData(int what, int val)
{
	if (hwnd_winamp == NULL) return;
	//SendMessage(hwnd_winamp,WM_WA_IPC,what,IPC_GETEQDATA); // f this :)
	SendMessage(hwnd_winamp,WM_WA_IPC,0xDB000000 | ((what&0xFF) << 16) | (val&0xFFFF),IPC_SETEQDATA);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getShuffle()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GET_SHUFFLE);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getRepeat()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GET_REPEAT);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setShuffle(int shuffle)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,shuffle,IPC_SET_SHUFFLE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setRepeat(int repeat)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,repeat,IPC_SET_REPEAT);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setManualPlaylistAdvance(int manual)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,manual,IPC_SET_MANUALPLADVANCE);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getManualPlaylistAdvance()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GET_MANUALPLADVANCE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::enableWindows(int enabled)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_WA_IPC,enabled?0:0xdeadbeef,IPC_ENABLEDISABLE_ALL_WINDOWS);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::areWindowsEnabled()
{
	if (hwnd_winamp == NULL) return 1;
	return 1; // todo !!
}

//-----------------------------------------------------------------------------------------------
int _IsWindowVisible(HWND w)
{
	if (!IsWindowVisible(w)) return 0;
	RECT r;
	GetWindowRect(w, &r);
	return !(r.left >= 3000 && r.top >= 3000);
}

int Winamp2FrontEnd::isMainWindowVisible()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_ISMAINWNDVISIBLE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setWindowsVisible(int v)
{
	if (visible == v) return;
	if (hwnd_winamp == NULL) return;
	if (v)
	{
		if (saved_main && ! isMainWindowVisible())
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_MAIN_WINDOW, 0);
		if (saved_eq && !isWindowVisible(IPC_GETWND_EQ))
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_EQ, 0);
#ifdef MINIBROWSER_SUPPORT
		if (saved_mb && !isWindowVisible(IPC_GETWND_MB))
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_MINIBROWSER, 0);
#endif
		if (saved_pe && !isWindowVisible(IPC_GETWND_PE))
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_PLEDIT, 0);
		if (saved_video && !isWindowVisible(IPC_GETWND_VIDEO))
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_VIDEO, 0);
	}
	else
	{
		if (isMainWindowVisible())
		{
			saved_main = 1;
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_MAIN_WINDOW, 0);
		}
		else saved_main = 0;
		if (isWindowVisible(IPC_GETWND_EQ))
		{
			saved_eq = 1;
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_EQ, 0);
		}
		else saved_eq = 0;

#ifdef MINIBROWSER_SUPPORT
		if (isWindowVisible(IPC_GETWND_MB))
		{
			saved_mb = 1;
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_MINIBROWSER, 0);
		}
		else saved_mb = 0;
#endif
		if (isWindowVisible(IPC_GETWND_PE))
		{
			saved_pe = 1;
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_PLEDIT, 0);
		}
		else saved_pe = 0;
		if (isWindowVisible(IPC_GETWND_VIDEO))
		{
			saved_video = 1;
			SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_OPTIONS_VIDEO, 0);
		}
		else saved_video = 0;
	}
	visible = v;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::areWindowsVisible()
{
	if (hwnd_winamp == NULL) return 1;
	return visible;
}

//-----------------------------------------------------------------------------------------------
HWND Winamp2FrontEnd::getWnd(int wnd)
{
	if (hwnd_winamp == NULL) return NULL;
	return (HWND)SendMessage(hwnd_winamp,WM_WA_IPC,wnd,IPC_GETWND);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getPlaylistLength()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETLISTLENGTH);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getCurPlaylistEntry()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETLISTPOS);
}

//-----------------------------------------------------------------------------------------------
const wchar_t *Winamp2FrontEnd::getTitle(int plentry)
{
	if (hwnd_winamp == NULL) return NULL;
	return (const wchar_t *)SendMessage(hwnd_winamp,WM_WA_IPC,plentry,IPC_GETPLAYLISTTITLEW);
}

//-----------------------------------------------------------------------------------------------
const char *Winamp2FrontEnd::getFile(int plentry)
{
	if (hwnd_winamp == NULL) return NULL;
	return (const char *)SendMessage(hwnd_winamp,WM_WA_IPC,plentry,IPC_GETPLAYLISTFILE);
}

const wchar_t *Winamp2FrontEnd::getFileW(int plentry)
{
	if (hwnd_winamp == NULL) return NULL;
	return (const wchar_t *)SendMessage(hwnd_winamp,WM_WA_IPC,plentry,IPC_GETPLAYLISTFILEW);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::playAudioCD(int cd)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_COMMAND,ID_MAIN_PLAY_AUDIOCD1+cd,0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::openFileDialog(HWND w)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)w, IPC_OPENFILEBOX);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::openUrlDialog(HWND w)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON2_CTRL, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::openDirectoryDialog(HWND w)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)w, IPC_OPENDIRBOX);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::ejectPopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::previousPopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 1, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::nextPopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 2, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::pausePopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 3, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::playPopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 4, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::stopPopupMenu()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, 5, IPC_SPAWNBUTTONPOPUP);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setDialogBoxParent(HWND w)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)w, IPC_SETDIALOGBOXPARENT);
}

void Winamp2FrontEnd::updateDialogBoxParent(HWND w)
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)w, IPC_UPDATEDIALOGBOXPARENT);
}


//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isOnTop()
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_IS_AOT);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::triggerPopupMenu(int x, int y)
{
	if (hwnd_winamp == NULL) return;
	HMENU hMenu = (HMENU)SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)0,IPC_GET_HMENU);
	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, x, y, 0, hwnd_winamp, NULL);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::triggerEQPresetMenu(int x, int y)
{
	waSpawnMenuParms p = {hwnd_winamp, x, y};
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNEQPRESETMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerFileMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNFILEMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPlayMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPLAYMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerOptionsMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNOPTIONSMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerWindowsMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNWINDOWSMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerHelpMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNHELPMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPEFileMenu(int x, int y, int width, int height)
{
	int IPC_GETMLWINDOW=SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_GETMLWINDOW > 65536) SendMessage(hwnd_winamp, WM_WA_IPC, -1, IPC_GETMLWINDOW);
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	p.wnd = getWnd(IPC_GETWND_PE);
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPEFILEMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPEPlaylistMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	p.wnd = getWnd(IPC_GETWND_PE);
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPEPLAYLISTMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPESortMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	p.wnd = getWnd(IPC_GETWND_PE);
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPESORTMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPEHelpMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPEHELPMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerPEListOfListsMenu(int x, int y)
{
	int IPC_GETMLWINDOW=SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_GETMLWINDOW > 65536) SendMessage(hwnd_winamp, WM_WA_IPC, -1, IPC_GETMLWINDOW);
	waSpawnMenuParms p = {hwnd_winamp, x, y};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNPELISTOFPLAYLISTS);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerMLFileMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNMLFILEMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerMLViewMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNMLVIEWMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::triggerMLHelpMenu(int x, int y, int width, int height)
{
	waSpawnMenuParms2 p = {hwnd_winamp, x, y, width, height};
	return SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&p, IPC_SPAWNMLHELPMENU);
}

//-----------------------------------------------------------------------------------------------
HMENU Winamp2FrontEnd::getPopupMenu()
{
	if (hwnd_winamp == NULL) return NULL;
	return (HMENU)SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)0,IPC_GET_HMENU);
}

//-----------------------------------------------------------------------------------------------
HMENU Winamp2FrontEnd::getTopMenu()
{
	if (hwnd_winamp == NULL) return NULL;
	return (HMENU)SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)-1,IPC_GET_HMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::adjustOptionsPopupMenu(int direction)
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)direction,IPC_ADJUST_OPTIONSMENUPOS);
}

//-----------------------------------------------------------------------------------------------
HMENU Winamp2FrontEnd::getMenuBarMenu(int which)
{
	if (hwnd_winamp == NULL) return NULL;
	return (HMENU)SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)which+1,IPC_GET_HMENU);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::adjustFFWindowsMenu(int direction)
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)direction,IPC_ADJUST_FFWINDOWSMENUPOS);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::adjustFFOptionsMenu(int direction)
{
	if (hwnd_winamp == NULL) return 0;
	return SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)direction,IPC_ADJUST_FFOPTIONSMENUPOS);
}

//-----------------------------------------------------------------------------------------------
HWND Winamp2FrontEnd::getMainWindow()
{
	return hwnd_winamp;
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::quit()
{
	if (hwnd_winamp == NULL) return;
	SendMessage(hwnd_winamp,WM_CLOSE,0,0);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isWindowVisible(int which)
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, which, IPC_ISWNDVISIBLE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setWindowVisible(intptr_t which, int visible)
{
	int state = isWindowVisible(which);
	if (!state == !visible) return;
	int id = 0;
	switch (which)
	{
	case IPC_GETWND_EQ:
		DebugString("Showing EQ!!\n");
		id = WINAMP_OPTIONS_EQ;
		break;
	case IPC_GETWND_PE:
		id = WINAMP_OPTIONS_PLEDIT;
		break;
#ifdef MINIBROWSER_SUPPORT
	case IPC_GETWND_MB:
		id = WINAMP_OPTIONS_MINIBROWSER;
		break;
#endif
	case IPC_GETWND_VIDEO:
		id = WINAMP_OPTIONS_VIDEO;
		break;
	}
	SendMessage(hwnd_winamp, WM_COMMAND, id, 0);
}


//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::sendPlCmd(int which, int x, int y, int menu_align_flag)
{
	windowCommand wc = {which, x, y, menu_align_flag};
	SendMessage(hwnd_winamp, WM_WA_IPC, (intptr_t)&wc, IPC_PLCMD);
}

#ifdef MINIBROWSER_SUPPORT
//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::sendMbCmd(int which, int x, int y, int menu_align_flag)
{
	windowCommand wc = {which, x, y, menu_align_flag};
	SendMessage(hwnd_winamp, WM_WA_IPC, (intptr_t)&wc, IPC_MBCMD);
}
#endif

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::sendVidCmd(int which, int x, int y, int menu_align_flag)
{
	windowCommand wc = {which, x, y, menu_align_flag};
	SendMessage(hwnd_winamp, WM_WA_IPC, (intptr_t)&wc, IPC_VIDCMD);
}


//-----------------------------------------------------------------------------------------------
#define WINAMP_VISPLUGIN                40192
void Winamp2FrontEnd::toggleVis()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_VISPLUGIN, 0);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isVisRunning()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_ISVISRUNNING);
}

//-----------------------------------------------------------------------------------------------
HWND Winamp2FrontEnd::getVisWnd()
{
	return (HWND)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETVISWND);
}

//-----------------------------------------------------------------------------------------------
IDropTarget *Winamp2FrontEnd::getDropTarget()
{
	return (IDropTarget *)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETDROPTARGET);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getSamplerate()
{
	int realRate = SendMessage(hwnd_winamp,WM_WA_IPC,5,IPC_GETINFO);
	if (realRate == 1)
		return 1000*SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETINFO);
	else
		return realRate;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getBitrate()
{
	return SendMessage(hwnd_winamp,WM_WA_IPC,1,IPC_GETINFO);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getChannels()
{
	return SendMessage(hwnd_winamp,WM_WA_IPC,2,IPC_GETINFO);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isValidEmbedWndState(embedWindowState *ws)
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, (intptr_t)ws, IPC_EMBED_ISVALID);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::PE_getNumItems()
{
	return SendMessage(hwnd_playlist, WM_USER, IPC_PE_GETINDEXTOTAL, 0);
}

//-----------------------------------------------------------------------------------------------
fileinfo2 *Winamp2FrontEnd::PE_getFileTitle(int index)
{
	static fileinfo2 fi;
	fi.fileindex = index;
	*(fi.filetitle) = 0;
	*(fi.filelength) = 0;
	SendMessage(hwnd_playlist, WM_USER, IPC_PE_GETINDEXTITLE, (LPARAM)&fi);
	return &fi;
}

//-----------------------------------------------------------------------------------------------
fileinfo2W *Winamp2FrontEnd::PE_getFileTitleW(int index)
{
	static fileinfo2W fi;
	fi.fileindex = index;
	*(fi.filetitle) = 0;
	*(fi.filelength) = 0;
	SendMessage(hwnd_playlist, WM_USER, IPC_PE_GETINDEXTITLEW, (LPARAM)&fi);
	return &fi;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::PE_getCurrentIndex()
{
	//return SendMessage(hwnd_playlist, WM_USER, IPC_PE_GETCURINDEX, 0);
	return SendMessage(hwnd_winamp, WM_USER, 0, IPC_GETLISTPOS);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::PE_setCurrentIndex(int i)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, i, IPC_SETPLAYLISTPOS);
}

//-----------------------------------------------------------------------------------------------
HWND Winamp2FrontEnd::getMediaLibrary()
{
	int IPC_GETMLWINDOW=SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	return IPC_GETMLWINDOW > 65536 ? (HWND)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETMLWINDOW) : NULL;
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::ensureMediaLibraryLoaded()
{
	int IPC_GETMLWINDOW=SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_GETMLWINDOW > 65536) SendMessage(hwnd_winamp, WM_WA_IPC, -1, IPC_GETMLWINDOW);
}


//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isPlayingVideo()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO) > 0;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isPlayingVideoFullscreen()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_IS_FULLSCREEN);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isDoubleSize()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_ISDOUBLESIZE);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getTimeDisplayMode()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETTIMEDISPLAYMODE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::switchSkin(const wchar_t *skinname)
{
	static StringW wideSkinName;
	wideSkinName=skinname;
	PostMessage(hwnd_winamp, WM_WA_IPC, (intptr_t)wideSkinName.getValue(), IPC_SETSKINW);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visNext()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_NEXT, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visFullscreen()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_FS, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visPrev()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_PREV, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visRandom(int set)
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_RANDOM | (set << 16), 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::pollVisRandom()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_RANDOM | 0xFFFF0000, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visConfig()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_CFG, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::visMenu()
{
	PostMessage(hwnd_winamp, WM_COMMAND, ID_VIS_MENU, 0);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::getIdealVideoSize(int *w, int *h)
{
	if (w) *w = video_ideal_width;
	if (h) *h = video_ideal_height;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getStopOnVideoClose()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETSTOPONVIDEOCLOSE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setStopOnVideoClose(int stop)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, stop, IPC_SETSTOPONVIDEOCLOSE);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::GetVideoResize()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETVIDEORESIZE);
}


void Winamp2FrontEnd::SetVideoResize(int stop)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, stop, IPC_SETVIDEORESIZE);
}


//-----------------------------------------------------------------------------------------------
BOOL CALLBACK findVisWndProc(HWND hwnd, LPARAM lParam)
{
	Winamp2FrontEnd *fe = (Winamp2FrontEnd*)lParam;
	if (hwnd == fe->getVisWnd())
	{
		fe->setFoundVis(); return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isVis(HWND hwnd)
{
	if (hwnd == wa2.getVisWnd()) return 1;
	foundvis = 0;
	EnumChildWindows(hwnd, findVisWndProc, (LPARAM)this);
	return foundvis;
}

//-----------------------------------------------------------------------------------------------
HWND Winamp2FrontEnd::getPreferencesWindow()
{
	return (HWND)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETPREFSWND);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setPlEditWidthHeight(int width, int height)
{
	POINT pt={width, height};
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&pt, IPC_SET_PE_WIDTHHEIGHT);
}

//-----------------------------------------------------------------------------------------------
HINSTANCE Winamp2FrontEnd::getLanguagePackInstance()
{
	return (HINSTANCE)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETLANGUAGEPACKINSTANCE);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::openTrackInfo()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_EDIT_ID3, 0);
}

//-----------------------------------------------------------------------------------------------
const char *Winamp2FrontEnd::getOutputPlugin()
{
	return (const char *)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETOUTPUTPLUGIN);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::setDrawBorders(int d)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, d, IPC_SETDRAWBORDERS);
}

//-----------------------------------------------------------------------------------------------
void Winamp2FrontEnd::disableSkinnedCursors(int disable)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, disable, IPC_DISABLESKINCURSORS);
}

//-----------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getMetaData(const wchar_t *filename, const wchar_t *name, wchar_t *data, int data_len)
{

	if (!_wcsnicmp(filename, L"file://", 7))
		filename+=7;
	extendedFileInfoStructW efis=
	{
		filename,
		name,
		data,
		data_len,
	};
	return SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);
}
//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::invalidateCache()
{
	got_length_cache = 0;
	got_pos_cache = 0;
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::registerGlobalHotkey(const char *name, int msg, int wparam, int lparam, int flags, const char *id)
{
	int m_genhotkeys_add_ipc=SendMessage(hwnd_winamp,WM_WA_IPC,(WPARAM)&"GenHotkeysAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);
	genHotkeysAddStruct hs={0,};
	hs.name = (char *)name;
	hs.uMsg = msg;
	hs.wParam = wparam;
	hs.lParam = lparam;
	hs.flags = flags;
	hs.id = (char *)id;
	if (m_genhotkeys_add_ipc > 65536) SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&hs, m_genhotkeys_add_ipc);
}

//------------------------------------------------------------------------------------------------
const char *Winamp2FrontEnd::getVideoInfoString()
{
	return (const char *)SendMessage(hwnd_winamp, WM_WA_IPC, 4, IPC_GETINFO);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::playFile(const wchar_t *file)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_DELETE);
	COPYDATASTRUCT cds;
	cds.dwData = IPC_PLAYFILEW;
	cds.lpData = (void *)file;
	cds.cbData = sizeof(wchar_t) * (wcslen(file)+1);  // +1 to get the NULL, missing forever
	SendMessage(hwnd_winamp, WM_COPYDATA, (WPARAM)hwnd_winamp, (LPARAM)&cds);
	SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_STARTPLAY);
}

void Winamp2FrontEnd::clearPlaylist()
{
	SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_DELETE);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::rewind5s()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_REW5S, 0);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::forward5s()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_FFWD5S, 0);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::endOfPlaylist()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON5_CTRL, 0);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::startOfPlaylist()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON1_CTRL, 0);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::stopWithFade()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON4_SHIFT, 0);
}

//------------------------------------------------------------------------------------------------
void Winamp2FrontEnd::stopAfterCurrent()
{
	SendMessage(hwnd_winamp, WM_COMMAND, WINAMP_BUTTON4_CTRL, 0);
}

//------------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isWindowShade(int whichwnd)
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, whichwnd, IPC_IS_WNDSHADE);
}

//------------------------------------------------------------------------------------------------
int Winamp2FrontEnd::getCurTrackRating()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETRATING);
}

void Winamp2FrontEnd::setCurTrackRating(int rating)
{
	SendMessage(hwnd_winamp, WM_WA_IPC, rating, IPC_SETRATING);
}

//------------------------------------------------------------------------------------------------
int Winamp2FrontEnd::isExitEnabled()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_IS_EXIT_ENABLED);
}

//------------------------------------------------------------------------------------------------
int Winamp2FrontEnd::pushExitDisabled()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_PUSH_DISABLE_EXIT);
}

//------------------------------------------------------------------------------------------------
int Winamp2FrontEnd::popExitDisabled()
{
	return SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_POP_DISABLE_EXIT);
}

void Winamp2FrontEnd::GetFileInfo(const wchar_t *filename, wchar_t *title, int titleCch, int *length)
{
	basicFileInfoStructW infoStruct;
	infoStruct.filename = filename;
	infoStruct.quickCheck = 0;
	infoStruct.title = title;
	infoStruct.titlelen = titleCch;
	SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&infoStruct, IPC_GET_BASIC_FILE_INFOW);
	*length = infoStruct.length;
}

const wchar_t *Winamp2FrontEnd::GetCurrentTitle()
{
	return (const wchar_t *)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GET_PLAYING_TITLE);
}

const wchar_t *Winamp2FrontEnd::GetCurrentFile()
{
	return (const wchar_t *)SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
}

void *Winamp2FrontEnd::CanPlay(const wchar_t *fn)
{
	return (void *)SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)fn, IPC_CANPLAY);
}

bool Winamp2FrontEnd::IsPlaylist(const wchar_t *fn)
{
	return AGAVE_API_PLAYLISTMANAGER->CanLoad(fn);
}



HWND GetMainContainerHWND();
bool Winamp2FrontEnd::GetAlbumArt(const wchar_t *filename)
{
	// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
	#if 0
	if (!_wcsnicmp(filename, L"file://", 7))
		filename+=7;

	artFetchData fetch = {sizeof(artFetchData), GetMainContainerHWND(), };

	

	wchar_t artist[512]=L"", album[512]=L"", gracenoteFileId[512]=L"", year[5]=L"";
		getMetaData(filename, L"albumartist", artist, 512);
		if (!WCSICMP(artist, L""))		//Add artist if albumartist not available
			getMetaData(filename, L"artist", artist, 512);
		getMetaData(filename, L"album", album, 512);
		getMetaData(filename, L"GracenoteFileID", gracenoteFileId, 512);
		getMetaData(filename, L"year", year, 5);

		fetch.artist=artist;
		fetch.album=album;
		fetch.gracenoteFileId=gracenoteFileId;
		fetch.year=_wtoi(year);

	int r = SendMessage(hwnd_winamp, WM_WA_IPC, (WPARAM)&fetch, IPC_FETCH_ALBUMART);
	if(r == 0 && fetch.imgData && fetch.imgDataLen) // success, save art in correct location
	{
		AGAVE_API_ALBUMART->SetAlbumArt(filename,L"cover",0,0,fetch.imgData,fetch.imgDataLen,fetch.type);
		WASABI_API_MEMMGR->sysFree(fetch.imgData);
		return true;
	}
	#endif
	return false;
}

static void code(long* v, long* k)
{
	unsigned long y = v[0], z = v[1], sum = 0,         /* set up */
		delta = 0x9e3779b9, n = 32 ;  /* key schedule constant*/

	while (n-- > 0)
	{                                       /* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^(z + sum) ^((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^(y + sum) ^((y >> 5) + k[3]); /* end cycle */
	}
	v[0] = y; v[1] = z;

}

static int getRegVer()
{
	int *x = (int*)malloc(32);
	long s[3];
	long ss[2] = {GetTickCount(), (int)x + (int)s}; // fucko: better challenge, too
	long tealike_key[4] = { 31337, 0xf00d, 0xdead, 0xbeef}; //fucko: hide this a bit
	free(x);
	s[0] = ss[0];
	s[1] = ss[1];
	s[2] = 0;

	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)s, IPC_GETREGISTEREDVERSION);

	code(ss, tealike_key);

	if (memcmp(s, ss, 8)) return 0;

	return s[2];
}

bool Winamp2FrontEnd::IsWinampPro()
{
	return !!getRegVer();
}

void Winamp2FrontEnd::openUrl(const wchar_t *url)
{
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)url, IPC_OPEN_URL);
}