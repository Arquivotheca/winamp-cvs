#include "api.h"
#include "webserver.h"
#include "../cloud/ifc_clouddb.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include "FilePageGenerator.h"
#include "BufferPageGenerator.h"
#include "nx/nxthread.h"
#include "nx/nxsleep.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

static ifc_clouddb *clouddb;
static nx_string_t device_id;
static int attribute_device, attribute_mime ;
static nx_thread_return_t NXTHREADCALL WebServerThread(nx_thread_parameter_t param);

MediaServer::MediaServer()
{
	destination=0;
	ip=0;
	server_thread=0;
}

MediaServer::~MediaServer()
{
	NXURIRelease(destination);
}

int MediaServer::MediaServer_SetDestinationDirectory(nx_uri_t directory)
{
	nx_uri_t old = destination;
	destination = NXURIRetain(directory);
	NXURIRelease(old);
	return NErr_Success;
}

int MediaServer::MediaServer_SetIPv4Address(int address)
{
	ip=address;
	return NErr_Success;
}

int MediaServer::MediaServer_Start()
{
	// TODO: benski some sort of locking would be smart here
	if (!server_thread)
	{
		if (!device_id)
			WASABI2_API_APP->GetDeviceID(&device_id);
		if (REPLICANT_API_CLOUD && device_id && !clouddb && REPLICANT_API_CLOUD->CreateDatabaseConnection(&clouddb, device_id) == NErr_Success)
		{
			clouddb->Attribute_Add("mime", &attribute_mime);
			clouddb->Attribute_Add("device", &attribute_device);	
		}

		NXThreadCreate(&server_thread, WebServerThread, 0);
	}
	return NErr_Success;
}

int MediaServer::MediaServer_Stop()
{
	// TODO: benski some sort of locking would be smart here
	nx_thread_return_t retval;
	if (server_thread)
		NXThreadJoin(server_thread, &retval);
	server_thread=0;
	return NErr_Success;
}

static const char *ExtensionForMIME(const char *mime)
{
	if (!strcmp(mime, "audio/mpeg"))
		return "mp3";
	if (!strcmp(mime, "audio/mp4"))
		return "m4a";
	if (!strcmp(mime, "audio/flac"))
		return "flac";
	
	return NULL;
}

static const char *ExtensionForMIME(nx_string_t mime)
{
	if (!NXStringKeywordCompareWithCString(mime, "audio/mpeg"))
		return "mp3";
	if (!NXStringKeywordCompareWithCString(mime, "audio/mp4"))
		return "m4a";
	if (!NXStringKeywordCompareWithCString(mime, "audio/flac"))
		return "flac";
	
	return NULL;
}

static int CopyTrack(nx_uri_t filename)
{
	ReferenceCountedPointer<ifc_metadata> metadata;
	
	if (REPLICANT_API_METADATA->CreateMetadata(&metadata, filename) == NErr_Success)
	{
		// TODO: benski> would be really nice to have an replicant ATF library :)
		ReferenceCountedNXString albumartist, album, track, title, mime;
		metadata->GetField(MetadataKeys::ALBUM_ARTIST, 0, &albumartist);
		metadata->GetField(MetadataKeys::ALBUM, 0, &album);
		metadata->GetField(MetadataKeys::TRACK, 0, &track);
		metadata->GetField(MetadataKeys::TITLE, 0, &title);
		metadata->GetField(MetadataKeys::MIME_TYPE, 0, &mime);

		const char *ext = ExtensionForMIME(mime);
		// TODO: make the destination folder(s) filesystem safe
		// TODO: create the various folders along the way 
		ReferenceCountedNXString filespec;
		//NXStringCreateWithFormatting(&filespec, "%s/%s/%s - %s.%s", AutoCharPrintfUTF8(albumartist), AutoCharPrintfUTF8(album), AutoCharPrintfUTF8(track), AutoCharPrintfUTF8(title), ext);
		NXStringCreateWithFormatting(&filespec, "%s - %s.%s", AutoCharPrintfUTF8(track), AutoCharPrintfUTF8(title), ext);
		ReferenceCountedNXURI destination_filename, filespec_uri;
		NXURICreateWithNXString(&filespec_uri, filespec);
		NXURICreateWithPath(&destination_filename, filespec_uri, REPLICANT_API_MEDIASERVER->destination);
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MediaServer] moving to %s", destination_filename->string);
#endif

		return NXFile_move(destination_filename, filename);
	}
	else
	{
		// TODO: use some sort of auto-generated random name
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[MediaServer] failed to open metadata reader");
#endif
		return NErr_Error;
	}
	
}



class MediaServerConnection : public ifc_connection_callback
{
public:
	ifc_pagegenerator *WASABICALL ConnectionCallback_OnConnection(jnl_http_request_t serv);

};

static ifc_pagegenerator *OnConnection_Stream(jnl_http_request_t serv)
{
	const char *id = jnl_http_request_get_parameter(serv, "id");
	if (!id)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 400 Bad Request");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	int64_t cloud_id = strtoull(id, 0, 10);
	int internal_id;
	if (clouddb->IDMap_Find(cloud_id, &internal_id) != NErr_Success)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 404 Not Found");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	ReferenceCountedNXString value, mime;
	ReferenceCountedNXURI filename;

	if (
		/* TODO: clouddb->IDMap_GetString(internal_id, attribute_device, &value) != NErr_Success
		|| */clouddb->IDMap_GetMIME(internal_id, &mime) != NErr_Success
		|| clouddb->IDMap_Get_Filepath(internal_id, &filename) != NErr_Success)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 500 Internal Server Error");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	/* TODO
	if (NXStringKeywordCompare(value, device_id))
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 404 Not Found");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}
	*/

	FilePageGenerator *generator = new (std::nothrow)ReferenceCounted<FilePageGenerator>;
	if (!generator)
	{
		delete generator;
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 500 Internal Server Error");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	int ret = generator->Initialize(filename, serv);
	if (ret == NErr_FileNotFound)
	{
		delete generator;
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 404 Not Found");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}
	else if (ret != NErr_Success)
	{
		delete generator;
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 500 Internal Server Error");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	char buf[512];
	sprintf(buf, "Content-Type:%s", AutoCharPrintfUTF8(mime));
	jnl_http_request_addheader(serv, buf);
	jnl_http_request_send_reply(serv);
	return generator;		

}

static ifc_pagegenerator *OnConnection_Root(jnl_http_request_t serv)
{
	char buf[8192];

	sprintf(buf, "<?xml version=\"1.0\"?>"
		"<info>"
		"<device id=\"%s\" />"
		"</info>", AutoCharPrintfUTF8(device_id));


	BufferPageGenerator *generator = new (std::nothrow)ReferenceCounted<BufferPageGenerator>;
	if (!generator)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 500 Internal Server Error");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	int ret = generator->Initialize(buf, strlen(buf));
	if (ret != NErr_Success)
	{
		delete generator;
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 500 Internal Server Error");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}


	jnl_http_request_addheader(serv, "Content-Type:application/xml");
	char buf2[512];
	sprintf(buf2, "Content-Length:%u", strlen(buf));
	jnl_http_request_addheader(serv, buf2);
	jnl_http_request_set_reply_string(serv, "HTTP/1.1 200 OK");
	jnl_http_request_send_reply(serv);
	return generator;
}

static ifc_pagegenerator *OnConnection_Transfer(jnl_http_request_t serv)
{
	if (strcmp(jnl_http_request_get_method(serv), "POST"))
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 405 Method Not Allowed");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	uint64_t content_length;
	const char *content_length_header = jnl_http_request_get_header(serv, "Content-Length");
	if (!content_length_header)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 411 Length Required");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	const char *ext = ExtensionForMIME(jnl_http_request_get_header(serv, "Content-Type"));
	if (!ext)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 415 Unsupported Media Type");
		jnl_http_request_send_reply(serv);
		return 0; // no data
	}

	content_length = strtoull(content_length_header, 0, 10);

	jnl_connection_t connection = jnl_http_request_get_connection(serv);
	ReferenceCountedNXURI temp_filename;
	NXURICreateTempWithExtension(&temp_filename, ExtensionForMIME(jnl_http_request_get_header(serv, "Content-Type")));
	nx_file_t temp_file;
	NXFileOpenFile(&temp_file, temp_filename, nx_file_FILE_write_binary);

	while (content_length)
	{
		// TODO: timeout
		int ret = jnl_http_request_run(serv);
		if (ret == -1)
		{
			NXFileRelease(temp_file);
			NXFile_unlink(temp_filename);
			jnl_connection_release(connection);
			return 0;
		}
		char buffer[65536];
		size_t bytes_read = jnl_connection_receive(connection, buffer, 65536);
		if (bytes_read)
		{
			if (bytes_read > content_length)
			{
				NXFileRelease(temp_file);
				NXFile_unlink(temp_filename);
				jnl_connection_release(connection);
				jnl_http_request_set_reply_string(serv, "HTTP/1.1 413 Request Entity Too Large");
				jnl_http_request_send_reply(serv);
				return 0; // no data
			}
			NXFileWrite(temp_file, buffer, bytes_read);
			content_length -= bytes_read;
		}
		else
		{
			NXSleep(10);
		}
	}
	NXFileRelease(temp_file);
	jnl_connection_release(connection);
	
	if (CopyTrack(temp_filename) != NErr_Success)
	{		
		#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[MediaServer] CopyTrack failed: %s", strerror(errno));
#endif
		NXFile_unlink(temp_filename);
	}

	jnl_http_request_set_reply_string(serv, "HTTP/1.1 201 Created");
	jnl_http_request_send_reply(serv);
	return 0; // no data
}

ifc_pagegenerator *MediaServerConnection::ConnectionCallback_OnConnection(jnl_http_request_t serv)
{
	const char *user_agent = WASABI2_API_APP->GetUserAgent();
	char buf[512];
	sprintf(buf, "Server:%s", user_agent);
	jnl_http_request_addheader(serv, buf);

	const char *request_file = jnl_http_request_get_uri(serv);
	if (!strcmp(request_file, "/stream"))
	{
		return OnConnection_Stream(serv);
	}
	else if (!strcmp(request_file, "/"))
	{
		return OnConnection_Root(serv);
	}
	else if (!strcmp(request_file, "/transfer"))
	{
		return OnConnection_Transfer(serv);
	}

	jnl_http_request_set_reply_string(serv, "HTTP/1.1 404 Not Found");
	jnl_http_request_send_reply(serv);
	return 0; // no data
}

static MediaServerConnection mc;
static nx_thread_return_t NXTHREADCALL WebServerThread(nx_thread_parameter_t param)
{
	WebServer w;
	w.SetConnectionCallback(&mc);
	w.addListenPort(50000);
	unsigned short port=0;
	for (unsigned short i=0;!port && i<5; i++)
	{
		port = w.getListenPort(i);
	}

	if (REPLICANT_API_SSDP)									 
	{
		char host[128];
		in_addr a; 
		a.s_addr=REPLICANT_API_MEDIASERVER->ip;
		char name[128];
	  jnl_dns_ntop(AF_INET, &a, name, 128);
	  sprintf(host, /*maxhostlen,*/ "http://%s:%u", name, port);
		ReferenceCountedNXURI location;
		NXURICreateWithUTF8(&location, host);
		ReferenceCountedNXString st, usn;
		NXStringCreateWithUTF8(&st, "urn:nullsoft-com:Sync:1");
		NXStringCreateWithFormatting(&usn, "urn:nullsoft-com:Sync:1::%s", AutoCharPrintfUTF8(device_id));
		REPLICANT_API_SSDP->RegisterService(location, st, usn);
		REPLICANT_API_SSDP->Search(st);
	}

	for (;;)
	{
		NXSleep(10);
		w.run();
	}
}

