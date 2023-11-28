#include "api.h"
#include "Logger.h"
#include "nswasabi/ReferenceCounted.h"
#include <time.h>
#ifdef _WIN32
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#endif

nx_uri_t Logger::base_path=0;
static nx_uri_t request_json_uri, request_jspf_uri, request_body_uri, response_header_uri, response_json_uri;
#ifdef DEBUG
static nx_uri_t parse_test_uri;
#endif

Logger::Logger(const char *subdir, int logmask) : log_mask(logmask), error_found(0)
{
	ReferenceCountedNXURI subdir_uri;
	NXURICreateWithUTF8(&subdir_uri, subdir);

#ifdef _WIN32
	SYSTEMTIME system_time;
	wchar_t local_path[MAX_PATH], time_string[MAX_PATH] = {0};
	GetLocalTime(&system_time);
	int adjust = GetDateFormat(LOCALE_INVARIANT, DATE_SHORTDATE, &system_time, NULL, time_string, MAX_PATH);
	time_string[adjust-1] = ' ';
	adjust += GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, &time_string[adjust], MAX_PATH - adjust);
	// strip down to what is safe
	for(int i = 0; i < adjust; i++) if (time_string[i] && !(time_string[i] >= '0' && time_string[i] <= '9' || time_string[i] == ' ')) time_string[i] = '-';

	if (subdir && subdir[0])
	{
		PathCombine(local_path, base_path->string, subdir_uri->string);
		CreateDirectory(local_path, 0);
		PathCombine(local_path, local_path, time_string);
	}
	else
	{
		PathCombine(local_path, base_path->string, time_string);
	}

	SHCreateDirectoryEx(NULL, local_path, NULL);	// will recursively create folders unlike CreateDirectory(..)

	ReferenceCountedNXString local_path_nx;
	NXStringCreateWithUTF16(&local_path_nx, local_path);
	NXURICreateWithNXString(&log_path, local_path_nx);
#else
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = gmtime(&rawtime);

	char datestamp[128];
	strftime(datestamp, 128, "%Y-%m-%d %H-%M-%S", timeinfo);
	ReferenceCountedNXURI nx_datestamp, local_path;	
	NXURICreateWithUTF8(&nx_datestamp, datestamp);

	if (subdir && subdir[0])
	{
		ReferenceCountedNXURI temp, nx_subdir;
		NXURICreateWithUTF8(&nx_subdir, subdir);
		NXURICreateWithPath(&temp, nx_subdir, base_path);
		mkdir(temp->string, 0777);
		NXURICreateWithPath(&log_path, nx_datestamp, temp);
	}
	else
	{
		NXURICreateWithPath(&log_path, nx_datestamp, base_path);
	}

	mkdir(log_path->string, 0777);
#endif

	request_json = 0;
	request_body = 0;
	request_jspf = 0;
	response_header = 0;
	response_json = 0;
	#ifdef DEBUG
	parse_test = 0;
	#endif
}

void Logger::UpdateError(int error)
{
	error_found = error;
}

nx_file_t Logger::RequestJSON()
{
	if (log_mask & LOG_REQUEST_JSON)
	{
		if (!request_json)
		{
			ReferenceCountedNXURI filename;
			NXURICreateWithPath(&filename, request_json_uri, log_path);
			NXFileOpenFile(&request_json, filename, nx_file_FILE_write_binary);
		}
	}
	return request_json;

}

nx_file_t Logger::RequestBody()
{
	if (log_mask & LOG_REQUEST_BODY)
	{
		if (!request_body)
		{
			ReferenceCountedNXURI filename;
			NXURICreateWithPath(&filename, request_body_uri, log_path);
			NXFileOpenFile(&request_body, filename, nx_file_FILE_write_binary);
		}
	}
	return request_body;
}

nx_file_t Logger::RequestJSPF()
{
	if (log_mask & LOG_REQUEST_BODY)
	{
		if (!request_jspf)
		{
			NXURICreateWithPath(&jspf_path, request_jspf_uri, log_path);
			NXFileOpenFile(&request_jspf, jspf_path, nx_file_FILE_write_binary);
		}
	}
	return request_jspf;
}

nx_file_t Logger::ResponseHeader()
{
	if (!response_header)
	{
		ReferenceCountedNXURI filename;
		NXURICreateWithPath(&filename, response_header_uri, log_path);
		NXFileOpenFile(&response_header, filename, nx_file_FILE_write_binary);
	}
	return response_header;
}

nx_file_t Logger::ResponseJSON()
{
	if (!response_json)
	{
		ReferenceCountedNXURI filename;
		NXURICreateWithPath(&filename, response_json_uri, log_path);
		NXFileOpenFile(&response_json, filename, nx_file_FILE_write_binary);
	}
	return response_json;
}

#ifdef DEBUG
nx_file_t Logger::ParseTest()
{
	if (!parse_test)
	{
		ReferenceCountedNXURI filename;
		NXURICreateWithPath(&filename, parse_test_uri, log_path);
		NXFileOpenFile(&parse_test, filename, nx_file_FILE_write_binary);
	}
	return parse_test;
}
#endif

Logger::~Logger()
{
	NXFileRelease(request_json);
	NXFileRelease(request_body);
	NXFileRelease(response_header);
	NXFileRelease(response_json);
	#ifdef DEBUG
	NXFileRelease(parse_test);
	#endif

	if (!error_found)
	{
		#ifdef _WIN32
		wchar_t base_path[MAX_PATH+1] = {0};
		wcsncpy(base_path, Logger::log_path->string, MAX_PATH);

		SHFILEOPSTRUCT shop = {0};
		shop.wFunc=FO_DELETE;
		shop.pFrom=base_path;
					 
		shop.fFlags=FOF_NO_UI|FOF_NORECURSION;
		SHFileOperation(&shop);
		#else
		// TODO: portme
		#endif
	}

	NXURIRelease(log_path);
}

void Logger::Init(nx_uri_t settings, const char* dir = "cloud-log")
{
	ReferenceCountedNXURI cloud_log;
	NXURICreateWithUTF8(&cloud_log, dir);
	NXURICreateWithPath(&base_path, cloud_log, settings);

#ifdef _WIN32 // TODO: portme
	SHCreateDirectoryEx(NULL, base_path->string, NULL);	// will recursively create folders unlike CreateDirectory(..)
#else
	mkdir(base_path->string, 0777);
#endif

	NXURICreateWithUTF8(&request_json_uri, "request-json.txt");
	NXURICreateWithUTF8(&request_body_uri, "request-body.bin");
	NXURICreateWithUTF8(&request_jspf_uri, "request-jspf.bin");
	NXURICreateWithUTF8(&response_header_uri, "response-header.txt");
	NXURICreateWithUTF8(&response_json_uri, "response-json.txt");
	#ifdef DEBUG
	NXURICreateWithUTF8(&parse_test_uri, "json-parse-test.txt");
	#endif
}
