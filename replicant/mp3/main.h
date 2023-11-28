#pragma once
#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif
	bool IsMyExtension(nx_uri_t filename);
	int EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int GetGaps(ifc_metadata *metadata, size_t *pregap, size_t *postgap);
#ifdef __cplusplus
}
#endif