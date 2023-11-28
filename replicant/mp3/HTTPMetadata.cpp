#include "HTTPMetadata.h"
#include "metadata/MetadataKeys.h"
/*
 provided fields
 */

HTTPMetadata::HTTPMetadata()
{
	server=0;
	mime_type=0;
	uri=0;
}


HTTPMetadata::~HTTPMetadata()
{
	if (server) 
		NXStringRelease(server);

	if (uri) 
		NXStringRelease(uri);

	if (mime_type) 
		NXStringRelease(mime_type);
}

void HTTPMetadata::ParseHTTPMetadata(jnl_http_t http)
{
	const char *header_value;
	header_value = jnl_http_getheader(http, "Server");
	if (header_value)
		NXStringCreateWithCString(&server, header_value, nx_charset_latin1);
	
	const char *http_url = jnl_http_get_url(http);
	if (http_url)
		NXStringCreateWithUTF8(&uri, http_url);
	
	const char *http_mime = jnl_http_getheader(http, "Content-Type");
	if (http_mime)
        NXStringCreateWithUTF8(&mime_type, http_mime);
}

int HTTPMetadata::Metadata_GetField(int field, int index, nx_string_t *value)
{
	if (field == MetadataKeys::URI)
	{
		if (index == 0)
		{
			if (uri)
				*value = NXStringRetain(uri);
			else
				*value = 0;
			return NErr_Success;
		}
		else
			return NErr_EndOfEnumeration;
	}
	else if (field == MetadataKeys::SERVER)
	{
		if (index == 0)
		{
			if (server)
				*value = NXStringRetain(server);
			else
				*value = 0;
			return NErr_Success;
		}
		else
			return NErr_EndOfEnumeration;
		
	}
	else if (field == MetadataKeys::MIME_TYPE)
	{
		if (index == 0)
		{
			if (mime_type)
				*value = NXStringRetain(mime_type);
			else
				*value = 0;
			return NErr_Success;
		}
		else
			return NErr_EndOfEnumeration;
		
	}
	/* TODO: check id3v2 tag */
	return NErr_Unknown;
}

int HTTPMetadata::Metadata_GetInteger(int field, int index, int64_t *value)
{
	return NErr_Unknown;
}

int HTTPMetadata::Metadata_GetReal(int field, int index, double *value)
{
	return NErr_Unknown;
}
