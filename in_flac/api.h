#ifndef NULLSOFT_IN_FLAC_API_H
#define NULLSOFT_IN_FLAC_API_H

#include <api/service/api_service.h>
#include "../Agave/Config/api_config.h"
extern api_config *AGAVE_API_CONFIG;
extern api_service *WASABI_API_SVC;

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr

#endif