#ifndef NULLSOFT_ML_RG_API_H
#define NULLSOFT_ML_RG_API_H

#include "../Wasabi/api/service/api_service.h"
#define WASABI_API_SVC serviceApi

#include "../Winamp/api_decodefile.h"
extern api_decodefile *decodeFile;

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#endif