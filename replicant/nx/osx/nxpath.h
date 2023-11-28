#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"

#ifdef __cplusplus
extern "C" {
#endif
// return NErr_Success / NErr_False
NX_API int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension);
    NX_API int NXPathIsURL(nx_uri_t path);
	NX_API ns_error_t NSPathSetTemporaryDirectory(nx_uri_t dir);
	NX_API ns_error_t NXPathGetTemporaryDirectory(nx_uri_t *dir);
#ifdef __cplusplus
}
#endif