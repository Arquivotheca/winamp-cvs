#pragma once
#include "sqlite/sqlite3.h"
#include "../libyajl/include/yajl/yajl_gen.h"
#include "../libyajl/include/yajl/yajl_parse.h"
#include "foundation/types.h"
#include "sha1.h"
#include "nswasabi/AutoCharNX.h"
#include "AutoNormalize.h"
#include "nx/nxstring.h"
#include "foundation/error.h"
#include "jnetlib/jnetlib.h"
#ifdef _WIN32
#include "../../nu/AutoChar.h" // TODO: benski> I don't like this one bit but it'll work for now
#endif
#include "cb_cloudevents.h"
#include "Logger.h"
#include "nx/nxdata.h"

class Cloud_DBConnection;
#define JSON_FIELD(x) (const unsigned char *)(x), (sizeof((x))-1)
class CloudSocket
{
public:
	CloudSocket();
	~CloudSocket();
	ns_error_t Initialize(nx_string_t device_id);
	
	static int GenerateRevisionID(nx_string_t *revision_id);
	int SetupHTTP(jnl_http_t *out_socket, Logger *logger);
	void SetUserAgent(jnl_http_t http);

	void JSON_GenNXString(yajl_gen hand, nx_string_t value);
	void JSON_AddNXString(yajl_gen hand, const unsigned char *field, size_t field_length, nx_string_t value, size_t limit=0);
	void JSON_AddNXURI(yajl_gen hand, const unsigned char *field, size_t field_length, nx_uri_t value, size_t limit=0);
	void JSON_AddString(yajl_gen hand, const unsigned char *field, size_t field_length, const wchar_t *value);
	void JSON_AddString(yajl_gen hand, const unsigned char *field, size_t field_length, const char *value);
	void JSON_AddInteger(yajl_gen hand, const unsigned char *field, size_t field_length, int value);
	void JSON_AddInteger64(yajl_gen hand, const unsigned char *field, size_t field_length, int64_t value);
	void JSON_AddTime(yajl_gen hand, const unsigned char *field, size_t field_length, time_t value);
	void JSON_AddNull(yajl_gen hand, const unsigned char *field, size_t field_length);
	void JSON_Start(Cloud_DBConnection *db_connection, yajl_gen hand, const char *type);
	void JSON_End(yajl_gen hand);

	void JSON_Start_Actions(yajl_gen hand);
	void JSON_End_Actions(yajl_gen hand);

	void JSON_Start_Action(yajl_gen hand, const char *action);
	void JSON_End_Action(yajl_gen hand);

	int PostArt(const char *url, nx_data_t data, const unsigned char *json_data, size_t json_len, yajl_handle parser, Logger *logger);
	int PostJSON(const char *url, const unsigned char *json_data, size_t json_len, yajl_handle parser, Logger *logger);
	int PostFile(const char *url, nx_uri_t filename, const unsigned char *json_data, size_t json_length, 
				 nx_string_t mime_type, yajl_handle parser, cb_cloud_upload *callback, Logger *logger);
	int PostFileRaw(const char *url, nx_uri_t filename,nx_string_t mime_type, cb_cloud_upload *callback, Logger *logger);
	int DownloadFile(const char *url, nx_uri_t filename, const unsigned char *json_data, size_t json_length, cb_cloud_upload *callback, Logger *logger);
	int DownloadFile(const char *url, yajl_handle parser, const unsigned char *json_data, size_t json_length, cb_cloud_upload *callback, Logger *logger);
protected:		
#ifdef _WIN32
	AutoNormalize normalizer;
	AutoCharGrow converter;
#endif
	AutoCharUTF8 nx_converter;

	nx_string_t device_id;
	jnl_http_t cloud_socket;

};