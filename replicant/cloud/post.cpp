#include "main.h"
#include "api.h"
#include "Logger.h"
#include "CloudSocket.h"
#include "../libyajl/include/yajl/yajl_parse.h"
#include "foundation/error.h"
#include "nx/nxfile.h"
#include "nu/strsafe.h"
#include "nx/nxsleep.h"
#include "nswasabi/ReferenceCounted.h"

#define HTTP_BUFFER_SIZE 65536

static yajl_status FeedJSON(jnl_http_t http, yajl_handle parser, int &content_length, Logger *logger)
{
	char downloadedData[HTTP_BUFFER_SIZE];
	yajl_status status = yajl_status_ok; 
	int downloadSize = jnl_http_get_bytes(http, downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
		if (logger && logger->ResponseJSON())
			NXFileWrite(logger->ResponseJSON(), downloadedData, downloadSize);

		if (parser)
			status = yajl_parse(parser, (const unsigned char *)downloadedData, downloadSize);

		content_length -= downloadSize;
	}
	else
		NXSleep(10);

	return status;
}

static int RunJSONDownload(jnl_http_t http, yajl_handle parser, Logger *logger)
{
	// TODO: timeout
	int content_length = (int)jnl_http_content_length(http);

	while (content_length > 0)
	{
		if (jnl_http_run(http) == HTTPGET_RUN_ERROR)
			return NErr_ConnectionFailed;

		if (FeedJSON(http, parser, content_length, logger) != yajl_status_ok)
			return NErr_Malformed;
	}

	if (parser)
	{
		if (yajl_complete_parse(parser) != yajl_status_ok)
			return NErr_Malformed;
	}

	return NErr_Success;	
}

static int PostShit(jnl_http_t http, const void *data, size_t data_len, Logger *logger)
{
	jnl_connection_t connection = jnl_http_get_connection(http);
	if (connection)
	{
		const uint8_t *data8 = (const uint8_t *)data;
		if (jnl_http_run(http) == HTTPGET_RUN_ERROR)
			return HTTPGET_RUN_ERROR;

		while (data_len)
		{
			if (jnl_http_run(http) == HTTPGET_RUN_ERROR)
				return HTTPGET_RUN_ERROR;

			int connection_state = jnl_connection_get_state(connection);
			if (connection_state == JNL_CONNECTION_STATE_CLOSED || connection_state == JNL_CONNECTION_STATE_ERROR)
				return -1;

			size_t lengthToSend = jnl_connection_send_bytes_available(connection);
			if (lengthToSend > data_len)
				lengthToSend=data_len;

			if (lengthToSend)
			{
				if (logger && logger->RequestBody())
					NXFileWrite(logger->RequestBody(), data8, lengthToSend);

				jnl_connection_send(connection, data8, lengthToSend);
				data8 += lengthToSend;
				data_len-=lengthToSend;
			}
			else
			{
				NXSleep(10);
			}
		}
	}
	return 0;
}

static int PostFileHandle(jnl_http_t http, nx_file_t file, cb_cloud_upload *callback, Logger *logger)
{
	uint64_t progress=0;
	uint64_t file_length;
	NXFileLength(file, &file_length);

	bool log_binary = Config_GetLogBinary();
	if (!log_binary && logger && logger->RequestBody())
		NXFileWrite(logger->RequestBody(), "[FILE DATA]", 11);

	uint8_t data[65536];
	jnl_connection_t connection = jnl_http_get_connection(http);
	if (connection)
	{
		for (;;)
		{
			size_t bytes_read;

			ns_error_t ret = NXFileRead(file, data, 65536, &bytes_read);
			if (ret == NErr_EndOfFile)
				break;
			else if (ret != NErr_Success)
				return ret;

			const uint8_t *data8 = (const uint8_t *)data;
			while (bytes_read)
			{
				if (jnl_http_run(http) == -1)
					return NErr_ConnectionFailed;

				int connection_state = jnl_connection_get_state(connection);
				if (connection_state == JNL_CONNECTION_STATE_CLOSED || connection_state == JNL_CONNECTION_STATE_ERROR)
					return NErr_ConnectionFailed;

				size_t lengthToSend = jnl_connection_send_bytes_available(connection);
				if (lengthToSend > bytes_read)
					lengthToSend=bytes_read;

				if (lengthToSend)
				{
					if (log_binary && logger && logger->RequestBody())
						NXFileWrite(logger->RequestBody(), data8, lengthToSend);

					jnl_connection_send(connection, data8, lengthToSend);
					data8 += lengthToSend;
					bytes_read-=lengthToSend;
					progress += lengthToSend;
					if (callback)
					{
						if (callback->IsKilled() || callback->OnProgress(progress, file_length))
						{
							NXFileRelease(file);
							return NErr_Aborted;
						}
					}
				}
				else
				{
					NXSleep(10);
				}
			}
		}
	}
	if (progress != file_length)
		return -1;
	return 0;
}

static int PostString(jnl_http_t http, const char *string, Logger *logger)
{
	return PostShit(http, string, strlen(string), logger);
}

static int PostJSON(jnl_http_t http, const unsigned char *json_data, size_t json_len, Logger *logger)
{
	if (logger && logger->RequestJSON())
		NXFileWrite(logger->RequestJSON(), json_data, json_len);

	return PostShit(http, json_data, json_len, logger);
}

static void LogHeaders(jnl_http_t http, Logger *logger)
{
	if (logger && logger->ResponseHeader())
	{
		const char *reply = jnl_http_getreply(http);
		NXFileWrite(logger->ResponseHeader(), reply, strlen(reply));
		NXFileWrite(logger->ResponseHeader(), "\r\n", 2);

		const char *all_headers = jnl_http_get_all_headers(http);
		while (*all_headers)
		{
			const char *header = all_headers;
			all_headers += strlen(all_headers) + 1;
			NXFileWrite(logger->ResponseHeader(), header, strlen(header));
			NXFileWrite(logger->ResponseHeader(), "\r\n", 2);
		}
	}
}

static int ErrorCodeForReply(int reply_code)
{
	switch(reply_code)
	{
		case 200:
		case 201:
			return NErr_Success;
		case 400:
			return NErr_BadRequest;
		case 401:
			return NErr_Unauthorized;
		case 403:
			return NErr_Forbidden;
		case 404:
			return NErr_NotFound;
		case 405:
			return NErr_BadMethod;
		case 406:
			return NErr_NotAcceptable;
		case 407:
			return NErr_ProxyAuthenticationRequired;
		case 408:
			return NErr_RequestTimeout;
		case 409:
			return NErr_Conflict;
		case 410:
			return NErr_Gone;
		case 413:
			return NErr_TooLarge;
		case 500:
			return NErr_InternalServerError;
		case 503:
			return NErr_ServiceUnavailable;
		default:
			return NErr_Error;
	}
}

static int RunPost(jnl_http_t http, yajl_handle parser, Logger *logger)
{
	jnl_connection_t connection = jnl_http_get_connection(http);
	if (connection)
	{
		while (jnl_connection_send_bytes_in_queue(connection))
		{
			int connection_state = jnl_connection_get_state(connection);
			if (connection_state == JNL_CONNECTION_STATE_CLOSED || connection_state == JNL_CONNECTION_STATE_ERROR)
				goto connection_failed;
			NXSleep(10);
			if (jnl_http_run(http) == -1)
				goto connection_failed;
		}
	}

	/* retrieve reply */
	int httpret;
	do
	{
		// TODO: timeout
		NXSleep(55);
		httpret = jnl_http_run(http);
		if (httpret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = jnl_http_get_status(http);
		switch (status)
		{
			case JNL_HTTP_STATUS_CONNECTING:
			case JNL_HTTP_STATUS_READING_HEADERS:
				break;

			case JNL_HTTP_STATUS_READING_CONTENT:
				{
					LogHeaders(http, logger);
					int ret=NErr_Success;
					if (jnl_http_content_length(http) > 0)
						 RunJSONDownload(http, parser, logger);
					int reply_code = jnl_http_getreplycode(http);
					if (ret == NErr_Success)
						return ErrorCodeForReply(reply_code);
					else
						return ret;				
				}
				break;
			case JNL_HTTP_STATUS_ERROR:
			default:
				// TODO const char *error_str = http->geterrorstr();
				return NErr_ConnectionFailed;
		}
	}
	while (httpret == JNL_HTTP_RUN_OK);

connection_failed:
	// TODO const char *err = http->geterrorstr();
	return NErr_ConnectionFailed;
}

int CloudSocket::PostJSON(const char *url, const unsigned char *json_data, size_t json_len, yajl_handle parser, Logger *logger)
{
	if (!url || url && !* url)
		return NErr_BadParameter;

	jnl_http_t cloud_socket=0;
	int ret = SetupHTTP(&cloud_socket, logger);
	if (ret != NErr_Success)
		return ret;

	char header_content_length[128];
	StringCbPrintfA(header_content_length, sizeof(header_content_length), "Content-Length: %u", json_len);
	jnl_http_addheader(cloud_socket, header_content_length);
	jnl_http_addheader(cloud_socket, "Content-Type: application/json");

	/* connect */
	//cloud_socket->AddHeaderValue("Expect", "100-continue");
	jnl_http_connect(cloud_socket, url, 1, "POST");

#if 0
	// spin and wait for a 100 response
	for (;;)
	{
		Sleep(55);
		int ret = http->run();
		if (ret != HTTPRECEIVER_RUN_OK) // connection failed or closed
			goto connection_failed;

		int reply_code = http->getreplycode();
		if (reply_code == 100)
			break;
		else if (reply_code)
			goto connection_failed;
	}
#endif
	/* POST the data */
	::PostJSON(cloud_socket, json_data, json_len, logger);

	ret = RunPost(cloud_socket, parser, logger);
// TODO const char *err = cloud_socket->geterrorstr();
	return ret;
}

static const char *multipart_start_line = "--IL1K3TURTLESAL0TRE4LLY\r\n";
static const char *multipart_name_art = "Content-Disposition: form-data; name=\"art.jpeg\"; filename=\"art.jpeg\"\r\n";
static const char *multipart_encoding_binary="Content-Transfer-Encoding: binary\r\n\r\n";
static const char *multipart_name_filename = "Content-Disposition: form-data; name=\"file.mp3\"; filename=\"file.mp3\"\r\n";
static const char *multipart_split_line = "\r\n--IL1K3TURTLESAL0TRE4LLY\r\n";
static const char *multipart_name_command = "Content-Disposition: form-data; name=\"json\"\r\n";
static const char *multipart_content_json="Content-Type: application/json\r\n\r\n";
static const char *multipart_terminator="\r\n--IL1K3TURTLESAL0TRE4LLY--";

int CloudSocket::PostArt(const char *url, nx_data_t data, const unsigned char *json_data, size_t json_len, yajl_handle parser, Logger *logger)
{
	jnl_http_t cloud_socket=0;
	int ret;

	const void *art_data;
	size_t art_length;
	ret = NXDataGet(data, &art_data, &art_length);
	if (ret != NErr_Success)
		return ret;

	ReferenceCountedNXString mime_type;
	char image_content_type[128];
	if (NXDataGetMIME(data, &mime_type) == NErr_Success)
		StringCbPrintfA(image_content_type, sizeof(image_content_type), "Content-Type:%s\r\n\r\n",  AutoCharPrintfUTF8(mime_type));
	else
		StringCbCopyA(image_content_type, sizeof(image_content_type), "Content-Type:application/octet-stream\r\n\r\n");

	uint64_t content_length = strlen(multipart_start_line)
		+ strlen(multipart_name_command)
		+ strlen(multipart_content_json)
		+ json_len		
		+ strlen(multipart_split_line)					
		+ strlen(multipart_name_art)
		+ strlen(image_content_type)
		+ art_length
		+ strlen(multipart_terminator);

	ret = SetupHTTP(&cloud_socket, logger);
	if (ret != NErr_Success)
	{
		return ret;
	}

	char header_temp[1024];
	jnl_http_addheader(cloud_socket, "Content-Type: multipart/form-data; boundary=IL1K3TURTLESAL0TRE4LLY");

	StringCbPrintfA(header_temp, sizeof(header_temp), "Content-Length: %llu", content_length);
	jnl_http_addheader(cloud_socket, header_temp);

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	jnl_http_connect(cloud_socket, url, 1, "POST");

	PostString(cloud_socket, multipart_start_line, logger);

	PostString(cloud_socket, multipart_name_command, logger);
	PostString(cloud_socket, multipart_content_json, logger);
	::PostJSON(cloud_socket, json_data, json_len, logger); 

	PostString(cloud_socket, multipart_split_line, logger);

	PostString(cloud_socket, multipart_name_art, logger);
	PostString(cloud_socket, image_content_type, logger);
	PostShit(cloud_socket, art_data, art_length, logger);
	
	PostString(cloud_socket, multipart_terminator, logger);

	ret = RunPost(cloud_socket, parser, logger);


	return ret;
}


int CloudSocket::PostFileRaw(const char *url, nx_uri_t filename,nx_string_t mime_type, cb_cloud_upload *callback, Logger *logger)
{
	jnl_http_t cloud_socket=0;
	int ret;
	nx_file_t file;

	ret = NXFileOpenFile(&file, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	uint64_t file_length;
	NXFileLength(file, &file_length);

	cloud_socket = jnl_http_create(8192, 65536);
	
	if (cloud_socket == 0)
	{
		NXFileRelease(file);
		return ret;
	}
	SetUserAgent(cloud_socket);
	jnl_http_allow_accept_all_reply_codes(cloud_socket);

	char header_temp[1024];

	jnl_http_addheadervalue(cloud_socket, "Content-Type", mime_type?AutoCharPrintfUTF8(mime_type):"application/octet-stream");

	StringCbPrintfA(header_temp, sizeof(header_temp), "Content-Length: %llu", file_length);
	jnl_http_addheader(cloud_socket, header_temp);

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	jnl_http_connect(cloud_socket, url, 1, "POST");

	if (callback)
	{
		if (callback->IsKilled() || callback->OnProgress(0, file_length))
		{
			NXFileRelease(file);
			return NErr_Aborted;
		}
	}

	ret = PostFileHandle(cloud_socket, file, callback, logger); 
	if (!ret)
	{
		ret = RunPost(cloud_socket, 0, logger);
	}
	NXFileRelease(file);
	jnl_http_release(cloud_socket);
	return ret;
}

int CloudSocket::PostFile(const char *url, nx_uri_t filename, const unsigned char *json_data, size_t json_length, nx_string_t mime_type, yajl_handle parser, cb_cloud_upload *callback, Logger *logger)
{
	jnl_http_t cloud_socket=0;
	int ret;
	nx_file_t file;

	ret = NXFileOpenFile(&file, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	uint64_t file_length;
	NXFileLength(file, &file_length);

	char file_content_type[128];
	StringCbPrintfA(file_content_type, sizeof(file_content_type), "Content-Type: %s\r\n\r\n", mime_type?AutoCharPrintfUTF8(mime_type):"application/octet-stream");

	uint64_t content_length = strlen(multipart_start_line)
		+ strlen(multipart_name_command)
		+ strlen(multipart_content_json)
		+ json_length
		+ strlen(multipart_split_line)
		+ strlen(multipart_name_filename)
		+ strlen(file_content_type)
		//+ strlen(multipart_encoding_binary)
		+ file_length		
		+ strlen(multipart_terminator);

	ret = SetupHTTP(&cloud_socket, logger);
	if (ret != NErr_Success)
	{
		NXFileRelease(file);
		return ret;
	}
	char header_temp[1024];

	jnl_http_addheader(cloud_socket, "Content-Type: multipart/form-data; boundary=IL1K3TURTLESAL0TRE4LLY");

	StringCbPrintfA(header_temp, sizeof(header_temp), "Content-Length: %llu", content_length);
	jnl_http_addheader(cloud_socket, header_temp);

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	jnl_http_connect(cloud_socket, url, 1, "POST");

	PostString(cloud_socket, multipart_start_line, logger);

	PostString(cloud_socket, multipart_name_command, logger);
	PostString(cloud_socket, multipart_content_json, logger);
	::PostJSON(cloud_socket, json_data, json_length, logger); 

	PostString(cloud_socket, multipart_split_line, logger);

	PostString(cloud_socket, multipart_name_filename, logger);
	PostString(cloud_socket, file_content_type, logger);

	if (callback)
	{
		if (callback->IsKilled() || callback->OnProgress(0, file_length))
		{
			NXFileRelease(file);
			return NErr_Aborted;
		}
	}

	ret = PostFileHandle(cloud_socket, file, callback, logger); 
	if (!ret)
	{
		PostString(cloud_socket, multipart_terminator, logger);
		ret = RunPost(cloud_socket, parser, logger);
	}
	NXFileRelease(file);
	return ret;
}

static yajl_status FeedFile(jnl_http_t http, nx_file_t destination, uint64_t &content_length, Logger *logger)
{
	char downloadedData[HTTP_BUFFER_SIZE];
	yajl_status status = yajl_status_ok; 
	int downloadSize = jnl_http_get_bytes(http, downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
		NXFileWrite(destination, downloadedData, downloadSize);

		content_length -= downloadSize;
	}
	else
		NXSleep(10);

	return status;
}

static int RunFileDownload(jnl_http_t http, nx_file_t destination, Logger *logger)
{
	// TODO: timeout
	uint64_t content_length = jnl_http_content_length(http);

	while (content_length > 0)
	{
		if (jnl_http_run(http) != HTTPGET_RUN_OK)
			return NErr_ConnectionFailed;

		if (FeedFile(http, destination, content_length, logger) != yajl_status_ok)
			return NErr_Malformed;
	}

	return NErr_Success;	
}

static int RunPost(jnl_http_t http, nx_file_t destination, Logger *logger)
{
	jnl_connection_t connection = jnl_http_get_connection(http);
	if (connection)
	{
		while (jnl_connection_send_bytes_in_queue(connection))
		{
			int connection_state = jnl_connection_get_state(connection);
			if (connection_state == JNL_CONNECTION_STATE_CLOSED || connection_state == JNL_CONNECTION_STATE_ERROR)
				goto connection_failed;
			NXSleep(10);
			if (jnl_http_run(http) == -1)
				goto connection_failed;
		}
	}

	/* retrieve reply */
	int httpret;
	do
	{
		// TODO: timeout
		NXSleep(55);
		httpret = jnl_http_run(http);
		if (httpret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = jnl_http_get_status(http);
		switch (status)
		{
			case JNL_HTTP_STATUS_CONNECTING:
			case JNL_HTTP_STATUS_READING_HEADERS:
				break;

			case JNL_HTTP_STATUS_READING_CONTENT:
				{
					LogHeaders(http, logger);
					int ret=NErr_Success;
					if (jnl_http_content_length(http) > 0)
						 RunFileDownload(http, destination, logger);
					int reply_code = jnl_http_getreplycode(http);
					if (ret == NErr_Success)
						return ErrorCodeForReply(reply_code);
					else
						return ret;				
				}
				break;
			case JNL_HTTP_STATUS_ERROR:
			default:
				// TODO const char *error_str = http->geterrorstr();
				return NErr_ConnectionFailed;
		}
	}
	while (httpret == JNL_HTTP_RUN_OK);

connection_failed:
	// TODO const char *err = http->geterrorstr();
	return NErr_ConnectionFailed;
}

int CloudSocket::DownloadFile(const char *url, nx_uri_t filename, const unsigned char *json_data, size_t json_length, cb_cloud_upload *callback, Logger *logger)
{
	jnl_http_t cloud_socket=0;
	int ret;
	nx_file_t destination;

	ret = NXFileOpenFile(&destination, filename, nx_file_FILE_write_binary);
	if (ret != NErr_Success)
		return ret;

	if (!json_data)
	{
		// if a GET request then spawn a new connection as the persistent
		// connection is typically https:// but the current scheme is for
		// GET to use http:// only so we need a http:// connection to run
		cloud_socket = jnl_http_create(8192, 65536);
		if (cloud_socket == 0)
		{
			NXFileRelease(destination);
			return ret;
		}
		SetUserAgent(cloud_socket);
		jnl_http_allow_accept_all_reply_codes(cloud_socket);
	}
	else
	{
		ret = SetupHTTP(&cloud_socket, logger);
		if (ret != NErr_Success)
		{
			NXFileRelease(destination);
			return ret;
		}
	}

	if (json_data)
	{
		char header_content_length[128];
		StringCbPrintfA(header_content_length, sizeof(header_content_length), "Content-Length: %u", json_length);
		jnl_http_addheader(cloud_socket, header_content_length);
		jnl_http_addheader(cloud_socket, "Content-Type: application/json");

		/* connect */
		//cloud_socket->AddHeaderValue("Expect", "100-continue");
		jnl_http_connect(cloud_socket, url, 1, "POST");

#if 0
	// spin and wait for a 100 response
	for (;;)
	{
		Sleep(55);
		int ret = http->run();
		if (ret != HTTPRECEIVER_RUN_OK) // connection failed or closed
			goto connection_failed;

		int reply_code = http->getreplycode();
		if (reply_code == 100)
			break;
		else if (reply_code)
			goto connection_failed;
	}
#endif
	/* POST the data */
	::PostJSON(cloud_socket, json_data, json_length, logger);
	}
	else
	{
		jnl_http_connect(cloud_socket, url, 1, "GET");
	}
	ret = RunPost(cloud_socket, destination, logger);
	NXFileRelease(destination);

	if (!json_data)
	{
		// clean up the GET specific connection
		jnl_http_release(cloud_socket);
	}
	// TODO const char *err = cloud_socket->geterrorstr();
	return ret;
}

int CloudSocket::DownloadFile(const char *url, yajl_handle parser, const unsigned char *json_data, size_t json_length, cb_cloud_upload *callback, Logger *logger)
{
	jnl_http_t cloud_socket=0;
	int ret;
	//nx_file_t destination;

	/*ret = NXFileOpenFile(&destination, filename, nx_file_FILE_write_binary);
	if (ret != NErr_Success)
		return ret;*/

	if (!json_data)
	{
		// if a GET request then spawn a new connection as the persistent
		// connection is typically https:// but the current scheme is for
		// GET to use http:// only so we need a http:// connection to run
		cloud_socket = jnl_http_create(8192, 65536);
		if (cloud_socket == 0)
		{
			//NXFileRelease(destination);
			return NErr_FailedCreate;
		}
		SetUserAgent(cloud_socket);
		jnl_http_allow_accept_all_reply_codes(cloud_socket);
	}
	else
	{
		ret = SetupHTTP(&cloud_socket, logger);
		if (ret != NErr_Success)
		{
			//NXFileRelease(destination);
			return ret;
		}
	}

	if (json_data)
	{
		char header_content_length[128];
		StringCbPrintfA(header_content_length, sizeof(header_content_length), "Content-Length: %u", json_length);
		jnl_http_addheader(cloud_socket, header_content_length);
		jnl_http_addheader(cloud_socket, "Content-Type: application/json");

		/* connect */
		//cloud_socket->AddHeaderValue("Expect", "100-continue");
		jnl_http_connect(cloud_socket, url, 1, "POST");

#if 0
	// spin and wait for a 100 response
	for (;;)
	{
		Sleep(55);
		int ret = http->run();
		if (ret != HTTPRECEIVER_RUN_OK) // connection failed or closed
			goto connection_failed;

		int reply_code = http->getreplycode();
		if (reply_code == 100)
			break;
		else if (reply_code)
			goto connection_failed;
	}
#endif
	/* POST the data */
	::PostJSON(cloud_socket, json_data, json_length, logger);
	}
	else
	{
		jnl_http_connect(cloud_socket, url, 1, "GET");
	}
	ret = RunPost(cloud_socket, parser, logger);
	//NXFileRelease(destination);

	if (!json_data)
	{
		// clean up the GET specific connection
		jnl_http_release(cloud_socket);
	}
	// TODO const char *err = cloud_socket->geterrorstr();
	return ret;
}