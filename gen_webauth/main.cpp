#include "api.h"
#include "main.h"
#include "service.h"
#include "../Winamp/wa_ipc.h"

int winampVersion = 0;
OmService *webauthService = NULL;

int Init()
{
	winampVersion = (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
	api_service *service= (api_service *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);

	if (!service || service == (api_service *)1) {
		return 1;
	}
	WasabiInit(service);
	HRESULT result = OMBROWSERMNGR->Initialize(NULL, plugin.hwndParent);

	if (FAILED(result)) {
		MessageBox(plugin.hwndParent, L"OM Browser init failed!", L"", MB_OK);
		return 1;
	}

	result = OmService::CreateInstance(&webauthService);
	if (FAILED(result)) {
		MessageBox(plugin.hwndParent, L"OmService CreateInstance failed!", L"", MB_OK);
		return 1;
	}

	HWND authWindow;
	result = OMBROWSERMNGR->CreatePopup(webauthService, 20, 20, 640, 480, NULL, NULL, 0, &authWindow);
	if (SUCCEEDED(result)) {
		ShowWindow(authWindow, SW_SHOWNORMAL);
	} else {
		MessageBox(plugin.hwndParent, L"webauthService CreatePopup failed!", L"", MB_OK);
		return 1;
	}
	return 0;
}

void Config()
{
  MessageBox(plugin.hwndParent, L"Config event triggered for gen_webauth.", L"", MB_OK);
}

void Quit()
{
  if (webauthService) webauthService->Release();
  webauthService = NULL;

  OMBROWSERMNGR->Finish();
}

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
		0,
		Init,
		Config,
		Quit,
		0,
		0,
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin()
{
	return &plugin;
}
