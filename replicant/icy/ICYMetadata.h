#pragma once
#include "metadata/ifc_metadata.h"
#include "jnetlib/jnetlib.h"

class ICYMetadata : public ifc_metadata
{
public:
	ICYMetadata();
	~ICYMetadata();

	int Initialize(jnl_http_t http);
	int Initialize(ICYMetadata *previous_metadata, const char *metadata_string);

	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	int WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }

private:
	void ParseStreamMetadata(const char *metadata_string);
	void ParseHTTPMetadata(jnl_http_t http);
	
	nx_string_t server, mime_type, uri, stream_title, stream_name, stream_url;
};
