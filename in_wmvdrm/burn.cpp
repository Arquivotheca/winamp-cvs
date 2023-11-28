#include "main.h"
#include <windows.h>

#include "./api.h"
#include "../primo/obj_primo.h"
#include <api/service/waservicefactory.h>
#include "../burnlib/burnlib.h"

#ifdef _DEBUG
  #pragma comment( lib, "../burnlib/lib/burnlibd.lib")  
#else
  #pragma comment( lib, "../burnlib/lib/burnlib.lib")  
#endif 


extern "C" __declspec(dllexport)

#define BURNER_PRIMOFAILED  0x0FF0
#define BURNER_EMPTYPLAYLIST  0x0FF6

unsigned int burn_doBurn(const wchar_t *playlist, HWND ownerWnd, DWORD drive, DWORD speed, DWORD flags, const wchar_t *temppath)
{
	InitializeBurningLibrary(WASABI_API_SVC, plugin.hDllInstance, plugin.hMainWindow);
	BurnerPlaylist burnPL;
	burnPL.Load(playlist);
	unsigned int errorCode;
	if (!burnPL.GetCount()) errorCode = BURNER_EMPTYPLAYLIST;
	else
	{	
		obj_primo *primo=0;
waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
if (!primo)
		errorCode = BURNER_PRIMOFAILED;
		else
		{
			BurnPlaylistUI burnDlg;
			errorCode = burnDlg.Burn(primo, drive, speed, flags, &burnPL, temppath, ownerWnd);
			sf->releaseInterface(primo);
		}
	}
	return errorCode;
}