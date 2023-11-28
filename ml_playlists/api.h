#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../Agave/Language/api_language.h"

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

#endif