#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"
#include "nx/nxuri.h"
#include "nx/nxstring.h"
#ifdef __cplusplus
extern "C" {
#endif

// return NErr_Success / NErr_False
NX_API int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension);

// return NErr_Success / NErr_False
NX_API int NXPathProtocol(nx_uri_t filename, const char *protocol);

// return NErr_Success / NErr_False
NX_API int NXPathIsURL(nx_uri_t filename);
#ifdef __cplusplus
}
#endif