#include "api.h"
#include "main.h"
#include "ICYMetadata.h"
#include "foundation/atomics.h"
#include "metadata/MetadataKeys.h"

/*
 provided fields
 bitrate     (icy-br header, data from decoder)
 server      (server header, icy-notice2)
 title       (synthesized from icy-name header or streamtitle)
 url         (icy-url header, streamurl)
 artist      (synthesized from icy-name header or streamtitle)
 streamtitle (icy-name or streamtitle)
 channels    (from decoder)
 samplerate  (from decoder)
 streamtype  ("SHOUTcast")
 mimetype    (content-type header)
 type        ("audio")
 
 */

ICYMetadata::ICYMetadata()
{
	server=0;
	mime_type=0;
	uri=0;
	stream_title=0;
	stream_name=0;
	stream_url=0;
}

ICYMetadata::~ICYMetadata()
{
	NXStringRelease(server);
	NXStringRelease(mime_type);
	NXStringRelease(uri);
	NXStringRelease(stream_title);
	NXStringRelease(stream_url);
	NXStringRelease(stream_name);	
}

int ICYMetadata::Initialize(jnl_http_t http)
{
	ParseHTTPMetadata(http);	
	return NErr_Success;
}
/* 
 Since ICYMetadata will get rebuilt somewhat frequently 
 (as the shoutcast title gets updated)
 we'll re-use some of the old strings 
 */
int ICYMetadata::Initialize(ICYMetadata *previous_metadata, const char *metadata_string)
{
	server = NXStringRetain(previous_metadata->server);
	uri = NXStringRetain(previous_metadata->uri);
	mime_type = NXStringRetain(previous_metadata->mime_type);
	stream_title = NXStringRetain(previous_metadata->stream_title);
	stream_name = NXStringRetain(previous_metadata->stream_name);
	stream_url = NXStringRetain(previous_metadata->stream_url);
	
	ParseStreamMetadata(metadata_string);
	return NErr_Success;
}

void ICYMetadata::ParseStreamMetadata(const char *metadata_string)
{
	// this is slow but i'm just trying to get it to work
	const char *key_end, *value_start, *value_end;
	while ((key_end = strstr(metadata_string, "='")))
	{
		value_start = key_end+2;
		value_end = strstr(value_start, "';");
		if (value_end)
		{
#ifdef _WIN32
			if (!strnicmp(metadata_string, "StreamTitle", key_end-metadata_string))
#else
			if (!strncasecmp(metadata_string, "StreamTitle", key_end-metadata_string))
#endif
			{
				if (stream_title)
					NXStringRelease(stream_title);
				stream_title=0;
				NXStringCreateWithBytes(&stream_title, value_start, value_end-value_start, nx_charset_latin1);
			}
#ifdef _WIN32
			else if (!strnicmp(metadata_string, "StreamUrl", key_end-metadata_string))
#else
			else if (!strncasecmp(metadata_string, "StreamUrl", key_end-metadata_string))
#endif
			{
				if (stream_url)
					NXStringRelease(stream_url);
				stream_url=0;
				NXStringCreateWithBytes(&stream_url, value_start, value_end-value_start, nx_charset_latin1);
			}
		}
		else break;
		metadata_string = value_end+1;
	}
}

void ICYMetadata::ParseHTTPMetadata(jnl_http_t http)
{
	const char *header_value;
	header_value = jnl_http_getheader(http, "Server");
	if (!header_value)
	{
		header_value = jnl_http_getheader(http, "icy-notice2");
		if (header_value)
		{
			const char *end = strstr(header_value, "<BR>");
			if (end)
				NXStringCreateWithBytes(&server, header_value, end-header_value, nx_charset_latin1);
			else
				NXStringCreateWithCString(&server, header_value, nx_charset_latin1);
		}
	}
	else
		NXStringCreateWithCString(&server, header_value, nx_charset_latin1);
	
	header_value = jnl_http_getheader(http, "icy-url");
	if (header_value)
		NXStringCreateWithCString(&uri, header_value, nx_charset_latin1);
	
	header_value = jnl_http_getheader(http, "icy-name");
	if (header_value)
	{
		NXStringCreateWithCString(&stream_title, header_value, nx_charset_latin1);
		stream_name=NXStringRetain(stream_title);
	}
}

static int ReturnMetadataValue(int index, nx_string_t input_value, nx_string_t *output_value)
{
	if (index == 0)
	{
		if (input_value)
		{
			*output_value = NXStringRetain(input_value);
			return NErr_Success;
		}
		else
			return NErr_Empty;
	}
	else 
		return NErr_EndOfEnumeration;
}

int ICYMetadata::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (field == MetadataKey_streamtitle)
	{
		return ReturnMetadataValue(index, stream_title, value);
	}
	else if (field == MetadataKeys::URI)
	{
		return ReturnMetadataValue(index, uri, value);
	}
	else if (field == MetadataKeys::SERVER)
	{
		return ReturnMetadataValue(index, server, value);
	}
	else if (field == MetadataKey_streamname)
	{
		return ReturnMetadataValue(index, stream_name, value);
	}
	else if (field == MetadataKey_streamurl)
	{
		return ReturnMetadataValue(index, stream_url, value);
	}
	
	return NErr_Unknown;
}

int ICYMetadata::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	return NErr_Unknown;
}

int ICYMetadata::Metadata_GetReal(int field, unsigned int index, double *value)
{
	return NErr_Unknown;
}
