#include "tbdiag.h"
#pragma comment (lib, "tbdiag_md.lib") // found via project LIB path
#include "../Winamp/gen.h"
#include <stdio.h>
#include <shlwapi.h>

extern "C" __declspec(dllexport)
INT_PTR StartHandler(const wchar_t *iniPath)
{
	char path[MAX_PATH], manifestFilename[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	PathCombineA(manifestFilename, path, "talkback.ini");
	// initialize talkback as early as possible!
	int err = FCInitializeWithManifest(manifestFilename);

	return err;
}

int Init()
{
	return GEN_INIT_SUCCESS;
}

void Config()
{
}

void Quit()
{
	FCCleanup();
}

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
		"Talkback",
		Init,
		Config,
		Quit,
		0,
		0
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin()
{
	return &plugin;
}