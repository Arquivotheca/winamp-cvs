#include "main.h"
#include "api.h"
#include "../jnetlib/api_httpget.h"
#include "../nu/AutoChar.h"
#include <api/service/waservicefactory.h>
#include <strsafe.h>

#define HTTP_BUFFER_SIZE 8192
#define POST_BUFFER_SIZE (128*1024)

static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};
#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
static void SetUserAgent(api_httpreceiver *http)
{

	char user_agent[USER_AGENT_SIZE];
	int bigVer = ((winampVersion & 0x0000FF00) >> 12);
	int smallVer = ((winampVersion & 0x000000FF));
	StringCchPrintfA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/%01x.%02x", bigVer, smallVer);
	http->addheader(user_agent);
}

static yajl_status FeedJSON(api_httpreceiver *http, yajl_handle parser, bool *noData)
{
	char downloadedData[HTTP_BUFFER_SIZE];
	yajl_status status = yajl_status_ok; 
	int downloadSize = http->get_bytes(downloadedData, HTTP_BUFFER_SIZE);
	if (downloadSize)
	{
if (parser)
		status = yajl_parse(parser, (const unsigned char *)downloadedData, downloadSize);
		*noData=false;
	}
	else	
		*noData = true;

	return status;
}

static int RunJSONDownload(api_httpreceiver *http, yajl_handle parser)
{

	int ret;
	bool noData;
	do
	{


		ret = http->run();
		if (FeedJSON(http, parser, &noData) != yajl_status_ok)
			return 1;
		if (noData)
			Sleep(10);
	}
	while (ret == HTTPRECEIVER_RUN_OK);

	// finish off the data
	do
	{
		if (FeedJSON(http, parser, &noData) != yajl_status_ok)
			return 1;
	} while (!noData);
if (parser)
	yajl_complete_parse(parser);

	if (ret != HTTPRECEIVER_RUN_ERROR)
		return 0;
	else
		return 1;
}

static int PostShit(api_httpreceiver *http, const void *data, size_t data_len)
{
	api_connection *connection = http->GetConnection();
	if (connection)
	{
		const uint8_t *data8 = (const uint8_t *)data;
		if (http->run() == -1)
			return -1;

		while (data_len)
		{
			if (http->run() == -1)
				return -1;

			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				return -1;

			size_t lengthToSend = min(data_len, connection->GetSendBytesAvailable());

			if (lengthToSend)
			{
				connection->send(data8, lengthToSend);
				data8 += lengthToSend;
				data_len-=lengthToSend;
			}
			else
			{
				Sleep(10);
			}
		}
	}
	return 0;
}

static int PostString(api_httpreceiver *http, const char *string)
{
	return PostShit(http, string, strlen(string));
}

int PostJSON(const char *url, const char *json_data, yajl_handle parser)
{
	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
		return 1;

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	size_t datalen = strlen(json_data);
	uint8_t *dataPtr=(uint8_t *)json_data;
	size_t clen = datalen;
	size_t transferred=0;
	size_t total_clen = datalen;

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	http->set_sendbufsize(POST_BUFFER_SIZE);
	SetUserAgent(http);

	char data[POST_BUFFER_SIZE];

	StringCbPrintfA(data, sizeof(data), "Content-Length: %u", datalen);
	http->addheader(data);
	http->addheader("Content-Type: application/json");

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	http->connect(url, 0, "POST");

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
	api_connection *connection = http->GetConnection();
	if (connection)
	{
		if (http->run() == -1)
			goto connection_failed;

		while (clen)
		{
			if (http->run() == -1)
				goto connection_failed;

			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;

			size_t lengthToSend = min(clen, connection->GetSendBytesAvailable());

			if (lengthToSend)
			{
				connection->send(dataPtr, lengthToSend);
				dataPtr += lengthToSend;
				clen-=lengthToSend;
				transferred+=lengthToSend;
			}
			else
			{
				Sleep(10);
			}
		}
		int x;
		while (x = connection->GetSendBytesInQueue())
		{
			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;
			Sleep(10);
			if (http->run() == -1)
				goto connection_failed;

		}
	}

	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{

				int ret = RunJSONDownload(http, parser);
				sf->releaseInterface(http);
				return ret;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	const char *err = http->geterrorstr();
	sf->releaseInterface(http);
	return 1;
}

static const char *multipart_start_line = "--IL1K3TURTLESAL0TRE4LLY\r\n";
static const char *multipart_name_art = "Content-Disposition: form-data; filename=\"art\"\r\n";
static const char *multipart_encoding_binary="Content-Transfer-Encoding: binary\r\n\r\n";
static const char *multipart_split_line = "\r\n--IL1K3TURTLESAL0TRE4LLY\r\n";
static const char *multipart_name_command = "Content-Disposition: form-data; name=\"json\"\r\n";
static const char *multipart_content_json="Content-Type: application/json\r\n\r\n";
static const char *multipart_terminator="\r\n--IL1K3TURTLESAL0TRE4LLY--";

int PostFile(const char *url, const wchar_t *filename, const char *json_data, yajl_handle parser)
{

	void *fileData=0;
	size_t fileLength=0;

	FILE *b = _wfopen(filename, L"rb");
	fseek(b, 0, SEEK_END);
	fileLength = (size_t)_ftelli64(b);
	fseek(b, 0, SEEK_SET);
	fileData = malloc(fileLength);
	fread(fileData, fileLength, 1, b);
	fclose(b);

	wchar_t mime_type[256];
	AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"mime", mime_type, 256);


	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
	{
		free(fileData);
			return 1;
	}

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	http->set_sendbufsize(POST_BUFFER_SIZE);
	SetUserAgent(http);

	char header_temp[1024];

	char image_content_type[128];
	StringCbPrintfA(image_content_type, sizeof(image_content_type), "Content-Type: %S\r\n",  mime_type);

	size_t content_length = strlen(multipart_start_line)
		+ strlen(multipart_name_art)
		+ strlen(image_content_type)
		+ strlen(multipart_encoding_binary)
		+ fileLength
		+ strlen(multipart_split_line)
		+ strlen(multipart_name_command)
		+ strlen(multipart_content_json)
		+ strlen(json_data)
		+ strlen(multipart_terminator);

	http->addheader("Content-Type: multipart/form-data; boundary=IL1K3TURTLESAL0TRE4LLY");
	StringCbPrintfA(header_temp, sizeof(header_temp), "Content-Length: %u", content_length);
	http->addheader(header_temp);

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	http->connect(url, 0, "POST");

	api_connection *connection = http->GetConnection();
	if (connection)
	{
		PostString(http, multipart_start_line);
		PostString(http, multipart_name_art);
		PostString(http, image_content_type);
		PostString(http, multipart_encoding_binary);
		PostShit(http, fileData, fileLength);
		
		PostString(http, multipart_split_line);
		PostString(http, multipart_name_command);
		PostString(http, multipart_content_json);
		
		PostString(http, json_data); 
		PostString(http, multipart_terminator);

		while (connection->GetSendBytesInQueue())
		{
			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;
			Sleep(10);
			if (http->run() == -1)
				goto connection_failed;

		}
	}

	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{

				int ret = RunJSONDownload(http, parser);
				sf->releaseInterface(http);
					free(fileData);
				return ret;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
				free(fileData);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	sf->releaseInterface(http);
	free(fileData);
	return 1;
}

int PostAlbumArt(const char *url, const wchar_t *filename, const char *json_data, yajl_handle parser)
{

	void *artData=0;
	size_t artLength=0;
	wchar_t *mimeType=0;
#if 1
	if (AGAVE_API_ALBUMART->GetAlbumArtData(filename, L"cover", &artData, &artLength, &mimeType) != ALBUMART_SUCCESS)
		return 1;
#else
	mimeType = (wchar_t *)WASABI_API_MEMMGR->sysMalloc(5*sizeof(wchar_t));
	wcscpy(mimeType, L"jpeg");
	FILE *b = _wfopen(filename, L"rb");
	fseek(b, 0, SEEK_END);
	artLength = (size_t)_ftelli64(b);
	fseek(b, 0, SEEK_SET);
	artData = WASABI_API_MEMMGR->sysMalloc(artLength);
	fread(artData, artLength, 1, b);
	fclose(b);
#endif

	api_httpreceiver *http = 0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf)
		http = (api_httpreceiver *)sf->getInterface();

	if (!http)
	{
		WASABI_API_MEMMGR->sysFree(artData);
		WASABI_API_MEMMGR->sysFree(mimeType);
		return 1;
	}

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

	http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
	http->set_sendbufsize(POST_BUFFER_SIZE);
	SetUserAgent(http);

	char header_temp[1024];

	char image_content_type[128];
	StringCbPrintfA(image_content_type, sizeof(image_content_type), "Content-Type: image/%S\r\n",  mimeType);

	size_t content_length = strlen(multipart_start_line)
		+ strlen(multipart_name_art)
		+ strlen(image_content_type)
		+ strlen(multipart_encoding_binary)
		+ artLength
		+ strlen(multipart_split_line)
		+ strlen(multipart_name_command)
		+ strlen(multipart_content_json)
		+ strlen(json_data)
		+ strlen(multipart_terminator);

	http->addheader("Content-Type: multipart/form-data; boundary=IL1K3TURTLESAL0TRE4LLY");
	StringCbPrintfA(header_temp, sizeof(header_temp), "Content-Length: %u", content_length);
	http->addheader(header_temp);

	/* connect */
	//http->AddHeaderValue("Expect", "100-continue");
	http->connect(url, 0, "POST");

	api_connection *connection = http->GetConnection();
	if (connection)
	{
		PostString(http, multipart_start_line);
		PostString(http, multipart_name_art);
		PostString(http, image_content_type);
		PostString(http, multipart_encoding_binary);
		PostShit(http, artData, artLength);
		
		PostString(http, multipart_split_line);
		PostString(http, multipart_name_command);
		PostString(http, multipart_content_json);
		
		PostString(http, json_data); 
		PostString(http, multipart_terminator);

		while (connection->GetSendBytesInQueue())
		{
			int connection_state = connection->get_state();
			if (connection_state == CONNECTION_STATE_CLOSED || connection_state == CONNECTION_STATE_ERROR)
				goto connection_failed;
			Sleep(10);
			if (http->run() == -1)
				goto connection_failed;

		}
	}

	/* retrieve reply */
	int ret;
	do
	{
		Sleep(55);
		ret = http->run();
		if (ret == -1) // connection failed
			break;

		// ---- check our reply code ----
		int status = http->get_status();
		switch (status)
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			break;

		case HTTPRECEIVER_STATUS_READING_CONTENT:
			{

				int ret = RunJSONDownload(http, parser);
				sf->releaseInterface(http);
				WASABI_API_MEMMGR->sysFree(artData);
				WASABI_API_MEMMGR->sysFree(mimeType);
				return ret;
			}
			break;
		case HTTPRECEIVER_STATUS_ERROR:
		default:
			sf->releaseInterface(http);
			WASABI_API_MEMMGR->sysFree(artData);
			WASABI_API_MEMMGR->sysFree(mimeType);
			return 1;
		}
	}
	while (ret == HTTPRECEIVER_RUN_OK);


connection_failed:
	sf->releaseInterface(http);
	WASABI_API_MEMMGR->sysFree(artData);
	WASABI_API_MEMMGR->sysFree(mimeType);
	return 1;
}