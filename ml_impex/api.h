#pragma once

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/service/waservicefactory.h>

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi