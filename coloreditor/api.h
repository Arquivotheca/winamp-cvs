#ifndef NULLSOFT_CONSOLE_API_H
#define NULLSOFT_CONSOLE_API_H

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include <api/wndmgr/api_wndmgr.h>
#define WASABI_API_WNDMGR wndManagerApi

#include <api/config/api_config.h>
#define WASABI_API_CONFIG configApi

#include <api/service/api_service.h>
#define WASABI_API_SVC serviceApi

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/skin/api_skin.h>
#define WASABI_API_SKIN skinApi

#include <api/script/api_maki.h>
#define WASABI_API_MAKI makiApi

#include <api/wnd/api_wnd.h>
#define WASABI_API_WND wndApi

#include <api/timer/api_timer.h>
#define WASABI_API_TIMER timerApi

#include <api/font/api_font.h>
#define WASABI_API_FONT fontApi

#include <api/imgldr/api_imgldr.h>
#define WASABI_API_IMGLDR imgLoaderApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi
#endif