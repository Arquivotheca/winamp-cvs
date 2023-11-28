#pragma once
#include "nu/PtrList.h"
#include "nx/nxstring.h"
#include "CloudDevice.h"
#include "../gen_ml/ml.h"
#include "../ml_pmp/pmp.h"
#include "../replicant/cloud/ifc_cloudclient.h"

extern PMPDevicePlugin plugin;
extern HINSTANCE cloud_hinst;
extern int winampVersion, firstpull, network_fail, IPC_GET_CLOUD_HINST;
extern ifc_cloudclient *cloud_client;
extern nx_string_t local_device_token;
extern int local_device_id;
extern nu::PtrList<CloudDevice> cloudDevices;

BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax);