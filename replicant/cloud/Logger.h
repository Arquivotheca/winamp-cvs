#pragma once
#include "nx/nxfile.h"
#include "nx/nxuri.h"

class Logger
{
public:
	enum
	{
		LOG_REQUEST_JSON = (1 << 0),
		LOG_REQUEST_BODY = (1 << 1),
	
		LOG_UPLOAD = LOG_REQUEST_JSON | LOG_REQUEST_BODY,
		LOG_ALL = LOG_REQUEST_JSON | LOG_REQUEST_BODY,
	};

	Logger(const char *subdir, int logmask);
	~Logger();
	static void Init(nx_uri_t filepath, const char* dir);
	static nx_uri_t base_path;

	nx_uri_t log_path;
	nx_file_t RequestJSON();
	nx_file_t RequestBody();
	nx_file_t ResponseHeader();
	nx_file_t ResponseJSON();
	#ifdef DEBUG
	nx_file_t ParseTest();
	#endif
	void UpdateError(int error);

	nx_uri_t jspf_path;
	nx_file_t RequestJSPF();

private:
	nx_file_t request_json, request_jspf, request_body, response_header, response_json;
	#ifdef DEBUG
	nx_file_t parse_test;
	#endif
	int log_mask, error_found;
};