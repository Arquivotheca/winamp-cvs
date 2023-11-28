#pragma once
#include "metadata/ifc_metadata.h"
#include "jnetlib/jnetlib.h"
#include "nsid3v2/nsid3v2.h"

class HTTPMetadata : public ifc_metadata
{
public:
	HTTPMetadata();
	~HTTPMetadata();

	int Initialize(jnl_http_t http, nsid3v2_tag_t tag);
	
	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, int index, double *value);
	
private:
	void ParseHTTPMetadata(jnl_http_t http);

	nsid3v2_tag_t id3v2_tag;
	
	nx_string_t server, mime_type, uri;
};
