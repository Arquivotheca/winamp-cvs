#include <stdio.h>
#include "api.h"
#include "CloudAPI.h"
#include "nu/utf.h"
#include "main.h"
#include "Logger.h"
#include "CloudSocket.h"
#include "foundation/error.h"
#include "../nu/AutoChar.h"
#include "nswasabi/AutoCharNX.h"
#include "nswasabi/ReferenceCounted.h"
#ifdef _WIN32
#include "../Winamp/buildType.h"
#endif
#include "CloudDB.h"

#include "nu/strsafe.h"
#include "version.h"

static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};
extern CloudAPI cloud_api;
void CloudSocket::SetUserAgent(jnl_http_t http)
{
	char user_agent[128];
	sprintf(user_agent, "User-Agent: %s", WASABI2_API_APP->GetUserAgent());
	jnl_http_addheader(http, user_agent);
}

int CloudSocket::SetupHTTP(jnl_http_t *out_socket, Logger *logger)
{
	if (cloud_socket)
	{
		int ret = jnl_http_run(cloud_socket);
		if (ret != HTTPGET_RUN_OK)
		{
			jnl_http_release(cloud_socket);
			cloud_socket=0;
		}
		else
			jnl_http_reset_headers(cloud_socket);
	}

	if (!cloud_socket)
	{


#if 0 // TODO: proxy settings for Replicant.  might be cool to have it parter of some larger networking API
		int use_proxy = 1;
		bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
		if (proxy80)
		{
			ReferenceCountedNXString url;
			REPLICANT_API_CLOUD->GetAPIURL(&url, /*http=*/NErr_False);

			if (strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
				use_proxy = 0;
		}

		const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

		cloud_socket = jnl_http_create((jnl_dns_t)-1, 8192, 65536, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
#else
		cloud_socket = jnl_http_create(65536, 65536);
#endif

		if (!cloud_socket)
			return NErr_FailedCreate;

		jnl_http_set_persistent(cloud_socket);
	}

	SetUserAgent(cloud_socket);
	jnl_http_addheader(cloud_socket, "Connection: Keep-Alive");
	/* TODO: benski> re-enable when we work out compression w/ persistent connection:
	jnl_http_allow_compression(cloud_socket); */
	jnl_http_allow_accept_all_reply_codes(cloud_socket);

	*out_socket = cloud_socket;

	return NErr_Success;
}

CloudSocket::~CloudSocket()
{
	jnl_http_release(cloud_socket);
}

CloudSocket::CloudSocket() 
{
	device_id = 0;
	cloud_socket=0;
}

ns_error_t CloudSocket::Initialize(nx_string_t device_id)
{
	this->device_id = NXStringRetain(device_id);
	return NErr_Success;
}

void CloudSocket::JSON_AddNXString(yajl_gen hand, const unsigned char *field, size_t field_length, nx_string_t value, size_t limit)
{
	if (value)
	{
		nx_converter.Set(value);
		const char *utf8 = nx_converter;
		if (utf8)
		{
			yajl_gen_string(hand, field, field_length);
			if (limit == 0)
				limit = nx_converter.size();
			else
				limit = utf8_strnlen(utf8, nx_converter.size(), limit);
			yajl_gen_string(hand, (unsigned char *)utf8, limit);
		}
	}
}

void CloudSocket::JSON_AddNXURI(yajl_gen hand, const unsigned char *field, size_t field_length, nx_uri_t value, size_t limit)
{
	if (value)
	{
		nx_converter.Set(value);
		const char *utf8 = nx_converter;
		if (utf8)
		{
			yajl_gen_string(hand, field, field_length);
			if (limit == 0)
				limit = nx_converter.size();
			else
				limit = utf8_strnlen(utf8, nx_converter.size(), limit);
			yajl_gen_string(hand, (unsigned char *)utf8, limit);
		}
	}
}

#ifdef _WIN32
void CloudSocket::JSON_AddString(yajl_gen hand, const unsigned char *field, size_t field_length, const wchar_t *value)
{
	if (!value)
		value=L"";

	const wchar_t *normalized_string = normalizer.Normalize(NormalizationC, value);
	if (normalized_string)
	{
		size_t cch;
		const char *utf8 = converter.Convert(normalized_string, CP_UTF8, 0, &cch);
		if (utf8)
		{
			yajl_gen_string(hand, field, field_length);
			yajl_gen_string(hand, (const unsigned char *)utf8, cch);
		}
	}
}
#endif

void CloudSocket::JSON_AddString(yajl_gen hand, const unsigned char *field, size_t field_length, const char *value)
{
	yajl_gen_string(hand, field, field_length);
	yajl_gen_string(hand, (const unsigned char *)value, strlen(value));
}

void CloudSocket::JSON_AddInteger(yajl_gen hand, const unsigned char *field, size_t field_length, int value)
{
	char utf8[64];
	size_t cch = sprintf(utf8, "%d", value);

	yajl_gen_string(hand, field, field_length);
	yajl_gen_number(hand, utf8, cch);
}

void CloudSocket::JSON_AddInteger64(yajl_gen hand, const unsigned char *field, size_t field_length, int64_t value)
{
	char utf8[64];
	size_t cch = sprintf(utf8, "%lld", value);

	yajl_gen_string(hand, field, field_length);
	yajl_gen_number(hand, utf8, cch);
}

void CloudSocket::JSON_AddTime(yajl_gen hand, const unsigned char *field, size_t field_length, time_t value)
{
	char utf8[64];
	size_t cch = sprintf(utf8, "%llu", (uint64_t)value);

	yajl_gen_string(hand, field, field_length);
	yajl_gen_number(hand, utf8, cch);
}

void CloudSocket::JSON_AddNull(yajl_gen hand, const unsigned char *field, size_t field_length)
{
	yajl_gen_string(hand, field, field_length);
	yajl_gen_null(hand);
}

void CloudSocket::JSON_GenNXString(yajl_gen hand, nx_string_t value)
{
	if (value)
	{
		nx_converter.Set(value);
		const char *utf8 = nx_converter;
		if (utf8)
		{
			yajl_gen_string(hand, (unsigned char *)utf8, nx_converter.size());
		}
	}
}

void CloudSocket::JSON_Start(Cloud_DBConnection *db_connection, yajl_gen hand, const char *type)
{
	yajl_gen_map_open(hand);
	yajl_gen_string(hand, JSON_FIELD("v"));
	yajl_gen_integer(hand, 1);

	yajl_gen_string(hand, JSON_FIELD("cmd"));
	yajl_gen_map_open(hand);

	yajl_gen_string(hand, JSON_FIELD("rev"));
	int64_t revision=0;
	db_connection->Info_GetRevision(&revision);
	yajl_gen_integer(hand, revision);

	ReferenceCountedNXString revision_id;
	if (db_connection->Info_GetRevisionID(&revision_id) == NErr_Success)
	{
		if (revision_id)
			JSON_AddNXString(hand, JSON_FIELD("rid"), revision_id);
		else
			JSON_AddNull(hand, JSON_FIELD("rid"));
	}

	yajl_gen_string(hand, JSON_FIELD("ua"));

	yajl_gen_map_open(hand);

	ReferenceCountedNXString product_shortname;
	if (WASABI2_API_APP->GetProductShortName(&product_shortname) == NErr_Success)
		JSON_AddNXString(hand, JSON_FIELD("app"), product_shortname);

	ReferenceCountedNXString winamp_version;
	if (WASABI2_API_APP->GetVersionString(&winamp_version) == NErr_Success)
		JSON_AddNXString(hand, JSON_FIELD("v"), winamp_version);
	JSON_AddInteger(hand, JSON_FIELD("p"), cloud_build_number);	
	JSON_AddInteger(hand, JSON_FIELD("b"), WASABI2_API_APP->GetBuildNumber());	

	yajl_gen_map_close(hand);

	yajl_gen_string(hand, JSON_FIELD("type"));
	yajl_gen_string(hand, (const unsigned char *)type, strlen(type));

	ReferenceCountedNXString username, authtoken;
	if (REPLICANT_API_CLOUD->GetCredentials(&username, &authtoken, 0) == NErr_Success)
	{
		JSON_AddNXString(hand, JSON_FIELD("user"), username);
		JSON_AddNXString(hand, JSON_FIELD("auth"), authtoken);
	}

	JSON_AddNXString(hand, JSON_FIELD("dev"), device_id);
}

void CloudSocket::JSON_End(yajl_gen hand)
{
	yajl_gen_map_close(hand);
	yajl_gen_map_close(hand);
}

void CloudSocket::JSON_Start_Actions(yajl_gen hand)
{
	yajl_gen_string(hand, JSON_FIELD("acts"));
	yajl_gen_array_open(hand);
}

void CloudSocket::JSON_End_Actions(yajl_gen hand)
{
	yajl_gen_array_close(hand);
}

void CloudSocket::JSON_Start_Action(yajl_gen hand, const char *action)
{
	yajl_gen_map_open(hand);
	yajl_gen_string(hand, (unsigned const char *)action, strlen(action));	
}

void CloudSocket::JSON_End_Action(yajl_gen hand)
{
	yajl_gen_map_close(hand);
}

int CloudSocket::GenerateRevisionID(nx_string_t *revision_id)
{
#ifdef _WIN32
	GUID guid;
	CoCreateGuid(&guid);
	return NXStringCreateWithFormatting(revision_id, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
    (int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
    (int)guid.Data4[0], (int)guid.Data4[1],
    (int)guid.Data4[2], (int)guid.Data4[3],
    (int)guid.Data4[4], (int)guid.Data4[5],
    (int)guid.Data4[6], (int)guid.Data4[7]);
#else
	uint8_t ur[16];
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, ur, 16);
	close(fd);

	return NXStringCreateWithFormatting(revision_id, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		ur[0], ur[1], ur[2], ur[3], 
		ur[4], ur[5], ur[6], ur[7], 
		ur[8], ur[9], ur[10], ur[11], 
		ur[12], ur[13], ur[14], ur[15]);		
#endif
}