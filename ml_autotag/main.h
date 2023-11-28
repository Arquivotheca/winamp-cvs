#ifndef NULLSOFT_ML_AUTOTAG_MAIN_H
#define NULLSOFT_ML_AUTOTAG_MAIN_H

#include <windows.h>
#include <shlwapi.h>
#include "../gen_ml/ml.h"
#include "../nu/AutoWide.h"
#include "../nu/PtrList.h"
#include "../nu/listview.h"

#include "resource.h"

#include <api/service/waServiceFactory.h>
#include <api/service/api_service.h>
#define WASABI_API_SVC serviceManager
extern api_service * WASABI_API_SVC;

#include "../gracenote/api_gracenote.h"
#define AGAVE_API_GRACENOTE gracenoteApi
extern api_gracenote * AGAVE_API_GRACENOTE;

#include "../Winamp/api_decodefile.h"
#define AGAVE_API_DECODE decodeApi
extern api_decodefile *AGAVE_API_DECODE;

#include "../ml_local/api_mldb.h"
#define AGAVE_API_MLDB mldbApi
extern api_mldb *AGAVE_API_MLDB;

#include "../Agave/Language/api_language.h"

extern winampMediaLibraryPlugin plugin;

LRESULT SetFileInfo(const wchar_t *filename, const wchar_t *metadata, const wchar_t *data);
void WriteFileInfo(const wchar_t *file);
int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len);

#endif