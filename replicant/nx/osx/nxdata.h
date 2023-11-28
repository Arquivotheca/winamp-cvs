#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"
#include <CoreFoundation/CoreFoundation.h>
#include "nx/nxfile.h"
#include "nx/nxstring.h"

/* OS X implementation */
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct nx_data_struct_t *nx_data_t;

	NX_API nx_data_t NXDataRetain(nx_data_t data);
	NX_API void NXDataRelease(nx_data_t data);
	NX_API int NXDataCreate(nx_data_t *data, const void *bytes, size_t length);
	NX_API int NXDataCreateWithSize(nx_data_t *data, void **bytes, size_t length);
	/* creates an empty data object. useful if you need to store MIME, source URI, etc. without having actual data */
	NX_API int NXDataCreateEmpty(nx_data_t *data);
	NX_API int NXDataCreateWithCFData(nx_data_t *out_data, CFDataRef data_ref, nx_string_t mime_type);
	NX_API int NXDataCreateFromURI(nx_data_t *data, nx_uri_t filename);

	/* you need to call CFRelease() on what yuo get back (if the function succeeded) */
	NX_API int NXDataGetCFData(nx_data_t data, CFDataRef *data_ref);
	
	NX_API size_t NXDataSize(nx_data_t data);
	NX_API const void * NXDataPointer(nx_data_t data);
	NX_API int NXDataGet(nx_data_t data, const void **bytes, size_t *length);
	NX_API int NXDataCopyBytes(nx_data_t data, void *buffer, size_t offset, size_t length);

	/* You can _only_ call these on your own nx_data_t object _before_ you give it to anyone else! */
	NX_API int NXDataSetMIME(nx_data_t data, nx_string_t mime_type);
	NX_API int NXDataSetDescription(nx_data_t data, nx_string_t description);
	NX_API int NXDataSetSourceURI(nx_data_t data, nx_uri_t source_uri);
	NX_API int NXDataSetSourceStat(nx_data_t data, nx_file_stat_t source_stats);	

	/* you need to call NXStringRelease on what you get back (if the function succeeded) */
	NX_API int NXDataGetMIME(nx_data_t data, nx_string_t *mime_type);
	NX_API int NXDataGetDescription(nx_data_t, nx_string_t *description);
	NX_API int NXDataGetSourceURI(nx_data_t, nx_uri_t *source_uri);
	NX_API int NXDataGetSourceStat(nx_data_t, nx_file_stat_t *source_stats);

#ifdef __cplusplus
}
#endif
