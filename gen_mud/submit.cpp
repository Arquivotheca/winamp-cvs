#include "main.h"
#include "api.h"
#include "collect.h"
#include "OAuthKey.h"
#include "config.h"
#include "../nu/AutoUrl.h"
#include "../nu/AutoChar.h"
#include "../xml/obj_xml.h"
#include "post.h"
#include "XMLString.h"
#include <api/service/waservicefactory.h>
#include "../Winamp/buildType.h"
#include <strsafe.h>


#define HTTP_BUFFER_SIZE 8192

static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};


#ifdef INTERNAL
	/* QA MUD */
	static const char *mud_server="ntc073dp-l33.sis.aol.com";
	static const char *mud_port="8686";
	static const char *mud_path="/Musicusage/SetPlayData";
	static const char *mud_path_encoded="%2FMusicusage%2FSetPlayData";
	/* QH AVTrack */
	static const char *av_server="api-qh2.web.aol.com";
	static const char *av_port="2048";
	static const char *av_path="/aim/setAvTrack";
	static const char *av_path_encoded="%2Faim%2FsetAvTrack";
#else
	/* Production MUD */
	static const char *mud_server="dynapub.music.aol.com";
	static const char *mud_port="80";
	static const char *mud_path="/api/charts/SetPlayData";
	static const char *mud_path_encoded="%2Fapi%2Fcharts%2FSetPlayData";
	/* Production AVTrack */
	static const char *av_server="api.oscar.aol.com";
	static const char *av_port="80";
	static const char *av_path="/aim/setAvTrack";
	static const char *av_path_encoded="%2Faim%2FsetAvTrack";
#endif

static void AddParameter(char *&position, size_t &len, const char *param, const wchar_t *val)
{
	if (val)
	{
		AutoUrl encoded_val(val);
		StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, encoded_val);
	}
}

static void AddParameter(char *&position, size_t &len, const char *param, const char *val)
{
	if (val)
	{
		AutoUrl encoded_val(val);
		StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, encoded_val);
	}
}

static void AddParameter(char *&position, size_t &len, const char *param, int64_t val)
{
	char temp[64];
	StringCchPrintfA(temp, 64, "%I64d", val);
	StringCchPrintfExA(position, len, &position, &len, 0, "&%s=%s", param, temp);
}

static void BuildTime(__time64_t t, char *str, size_t strcch)
{
	strftime(str, strcch, "%Y-%m-%d %H:%M:%S", _gmtime64(&t));
}

class ResultsParser
{
public:
	void RegisterCallbacks(obj_xml *parser)
	{
		parser->xmlreader_registerCallback(L"response\fstatusCode", &statusCode);
		parser->xmlreader_registerCallback(L"response\fstatusText", &statusText);

	}
	void UnregisterCallbacks(obj_xml *parser)
	{
		parser->xmlreader_unregisterCallback(&statusCode);
		parser->xmlreader_unregisterCallback(&statusText);
	}

	XMLString statusCode, statusText;
};

static int Post(const char *url, const char *post_data)
{
	ResultsParser resultsParser;
	obj_xml *parser = 0;
		waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		parser->xmlreader_setCaseSensitive();
		resultsParser.RegisterCallbacks(parser);
		parser->xmlreader_open();
		int err = PostXML(url, post_data, parser);
		resultsParser.UnregisterCallbacks(parser);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		if (err != AUTH_SUCCESS)
			return err;
		Log(L"[%s] POST result: %s %s", MakeDateString(_time64(0)), resultsParser.statusCode.GetString(), resultsParser.statusText.GetString());
		switch(resultsParser.statusCode.GetUInt32())
		{
		case 200:
			return AUTH_SUCCESS;
		case 401:
			return AUTH_NOT_AUTHORIZED;
		}
	}
	else
		return AUTH_NOPARSER;
	return MUD_ERROR;
}



int Submit(const CollectedData &data)
{
	int status = GetLoginStatus();
	if (status != LOGIN_LOGGEDIN && status != LOGIN_EXPIRING) // not logged in
		return SUBMIT_TRY_AGAIN;

	char post_data[2048]="";
	char *post_itr=post_data;
	size_t post_cch=sizeof(post_data)/sizeof(*post_data);

	OAuthKey key(session_key, strlen(session_key));

	key.FeedMessage("POST&", 5);
	key.FeedMessage("http%3A%2F%2F", 13);
	key.FeedMessage(mud_server, strlen(mud_server));
	if (strcmp(mud_port, "80"))
	{
		key.FeedMessage("%3A", 3);	
		key.FeedMessage(mud_port, strlen(mud_port));
	}
	key.FeedMessage(mud_path_encoded, strlen(mud_path_encoded));
	key.FeedMessage("&", 1);

	// parameters
	StringCbPrintfExA(post_itr, post_cch, &post_itr, &post_cch, 0, "a=%s", token_a);
	char *start = post_itr;
	key.FeedMessage("a%3D", 4);
	AutoUrl token_a_url1(token_a);
	AutoUrl token_a_url((char *)token_a_url1);
	key.FeedMessage(token_a_url, strlen((char *)token_a_url));
	
	
	//AddParameter(post_itr, post_cch, "a", token_a);
	AddParameter(post_itr, post_cch, "altitle", data.album);
	AddParameter(post_itr, post_cch, "aname", data.artist);
	AddParameter(post_itr, post_cch, "appname", L"gen_mud");
	//AddParameter(post_itr, post_cch, "asset", data.filename);
	AddParameter(post_itr, post_cch, "devId", AGAVE_API_AUTH->GetDevID());
	AddParameter(post_itr, post_cch, "device", L"winamp");
	AddParameter(post_itr, post_cch, "f", L"xml");
	//AddParameter(post_itr, post_cch, "mime", data.mimetype);
	//AddParameter(post_itr, post_cch, "ptime", data.playLength);
	char ptime[256];
	BuildTime(data.timestamp, ptime, 256);
	AddParameter(post_itr, post_cch, "ptime", ptime);
	if (data.track > 0)
		AddParameter(post_itr, post_cch, "tnum", data.track);
		__time64_t t = _time64(0);
	AddParameter(post_itr, post_cch, "ts", t);
	AddParameter(post_itr, post_cch, "ttitle", data.title);
	_tzset(); // this function call ensures that _timezone global var is valid
	AddParameter(post_itr, post_cch, "tzo", _timezone/60 - (_daylight?60:0));	

	AutoUrl encoded_post(start);
	key.FeedMessage((char *)encoded_post, strlen(encoded_post));

	key.EndMessage();
	char hash[512];
	key.GetBase64(hash, 512);

	StringCchPrintfA(post_itr, post_cch, "&sig_sha256=%s", AutoUrl(hash));

	char url[2048];
	StringCbPrintfA(url, sizeof(url), "http://%s:%s%s", mud_server, mud_port, mud_path);
	Log(L"[%s] Submitting %s to MUD, artist=%s, album=%s, title=%s", MakeDateString(_time64(0)), data.filename, data.artist, data.album, data.title);
	switch(Post(url, post_data))
	{
	case AUTH_SUCCESS:
		return SUBMIT_OK;
	case AUTH_NOT_AUTHORIZED:
	case AUTH_NOHTTP:
	case AUTH_404:
		return SUBMIT_TRY_AGAIN;

	}
	
	return SUBMIT_FAILED;
}


int SubmitAvTrack(CollectedData *data)
{
	int status = GetLoginStatus();
	if (status != LOGIN_LOGGEDIN && status != LOGIN_EXPIRING) // not logged in
		return SUBMIT_TRY_AGAIN;

	char post_data[2048]="";
	char *post_itr=post_data;
	size_t post_cch=sizeof(post_data)/sizeof(*post_data);

	OAuthKey key(session_key, strlen(session_key));

	key.FeedMessage("POST&", 5);
	key.FeedMessage("http%3A%2F%2F", 13);
	key.FeedMessage(av_server, strlen(av_server));

	if (strcmp(av_port, "80"))
	{
		key.FeedMessage("%3A", 3);	
		key.FeedMessage(av_port, strlen(av_port));
	}
	key.FeedMessage(av_path_encoded, strlen(av_path_encoded));
	key.FeedMessage("&", 1);


	char *start = post_itr+1;
	AddParameter(post_itr, post_cch, "a", token_a);
	if ( data->album )
		AddParameter(post_itr, post_cch, "album", data->album);
	if ( data->artist )
	{
		AddParameter(post_itr, post_cch, "artist", data->artist);
		wchar_t link[2048];
		StringCbPrintfW(link, sizeof(link), L"http://www.winamp.com/artist/%s", data->artist);
		AddParameter(post_itr, post_cch, "artistLink", link);
	}
	AddParameter(post_itr, post_cch, "f", L"xml");
	AddParameter(post_itr, post_cch, "k", AGAVE_API_AUTH->GetDevID());
	if ( data->timestamp ) 
		AddParameter(post_itr, post_cch, "startDate", data->timestamp);
	if ( data->title )
		AddParameter(post_itr, post_cch, "title", data->title);
	__time64_t t = _time64(0);
	AddParameter(post_itr, post_cch, "ts", t);


	AutoUrl encoded_post(start);
	key.FeedMessage((char *)encoded_post, strlen(encoded_post));

	key.EndMessage();
	char hash[512];
	key.GetBase64(hash, 512);

	StringCchPrintfA(post_itr, post_cch, "&sig_sha256=%s", AutoUrl(hash));

	char url[2048];
	StringCbPrintfA(url, sizeof(url), "http://%s:%s%s", av_server, av_port, av_path);
	if ( data->filename && data->artist && data->title )
		Log(L"[%s] Submitting %s to AV Track, artist=%s, album=%s, title=%s", MakeDateString(_time64(0)), data->filename, data->artist, data->album, data->title);
	else 
		Log(L"[%s] Resetting AV Track with empty values", MakeDateString(_time64(0)));
	switch(Post(url, start))
	{
	case AUTH_SUCCESS:
		return SUBMIT_OK;
	case AUTH_NOT_AUTHORIZED:
	case AUTH_NOHTTP:
	case AUTH_404:
		return SUBMIT_TRY_AGAIN;

	}
	
	return SUBMIT_FAILED;
}