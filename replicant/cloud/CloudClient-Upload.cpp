#include "api.h"
#include "CloudThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include "CloudDB.h"
#include "main.h"
#include <time.h>

int CloudThread::UploadLAN(UploadStruct *upload)
{
	LANDeviceStruct *device = 0;
	for (LANDevices::iterator itr = lan_devices.begin();itr!=lan_devices.end();itr++)
	{
		device = *itr;
		if (!NXStringCompare(upload->destination_device, device->device_token, nx_compare_default))
		{
			break;
		}
		device = 0;
	}

	if (!device)
		return NErr_Error;

	ReferenceCountedNXString mime_type;
	db_connection->IDMap_GetMIME(upload->internal_id, &mime_type);

	bool do_logging = Config_GetLogging();

	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

		if (!callbacks.empty())
	{
		ReferenceCountedNXString message, action;
		NXStringCreateWithUTF8(&action, "upload"); // TODO: make this global singleton that's created at initialization
		NXStringCreateWithFormatting(&message, "Uploading to %s: %s", AutoCharPrintfUTF8(upload->destination_device), AutoCharPrintfUTF8(upload->filename));
		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnAction(this, action, message);
		}
	}

	char url[512];
	sprintf(url, "%s/transfer", AutoCharPrintfUTF8(device->location));
	int ret;
	if (do_logging)
	{
		Logger logger("upload", Logger::LOG_UPLOAD);
	ret =  cloud_socket.PostFileRaw(url, upload->filename, mime_type, upload->callback, &logger);
	logger.UpdateError(1);
	}
	else
	{
		ret =  cloud_socket.PostFileRaw(url, upload->filename, mime_type, upload->callback, 0);
	}
	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(ret);

		if (!callbacks.empty())
		{
			ReferenceCountedNXString message, action;
			NXStringCreateWithUTF8(&action, "upload"); // TODO: make this global singleton that's created at initialization
			NXStringCreateWithFormatting(&message, "Uploaded: %s", AutoCharPrintfUTF8(upload->filename));
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnAction(this, action, message);
			}
		}
	}
	return ret;
}

int CloudThread::UploadServer(UploadStruct *upload)
{
	if (REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) != NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unauthorized);
		return NErr_Success; // TODO?
	}

	int64_t cloud_id = 0;
	if (db_connection->IDMap_Get(upload->internal_id, &cloud_id) != NErr_Success || !cloud_id)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unknown);

		return NErr_Success; // TODO?
	}


	ReferenceCountedNXString mime_type;
	db_connection->IDMap_GetMIME(upload->internal_id, &mime_type);

	bool do_logging = Config_GetLogging();

	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);
	cloud_socket.JSON_Start(db_connection, hand, "media-upload");

	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("id"), cloud_id);
	yajl_gen_string(hand, JSON_FIELD("dest"));
	yajl_gen_array_open(hand);
	cloud_socket.JSON_GenNXString(hand, upload->destination_device);
	yajl_gen_array_close(hand);
	cloud_socket.JSON_AddInteger(hand, JSON_FIELD("timeout"), 86400);
	cloud_socket.JSON_End(hand);

	int ret;
	const unsigned char *buf;
	size_t len;
	yajl_gen_get_buf(hand, &buf, &len);
	ReferenceCountedNXString cloud_url;
	REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_False);

	yajl_handle json_hand=0;
	nsjson_tree_t tree;
	JSON_Tree_CreateParser(&tree);
	JSON_Tree_GetHandle(tree, &json_hand);

	if (!callbacks.empty())
	{
		ReferenceCountedNXString message;
		NXStringCreateWithFormatting(&message, "Uploading to %s: %s", AutoCharPrintfUTF8(upload->destination_device), AutoCharPrintfUTF8(upload->filename));
		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnUploadStart(this, upload->filename, message);
		}
	}

	if (do_logging)
	{
		Logger logger("upload", Logger::LOG_UPLOAD);
		ret = cloud_socket.PostFile(AutoCharUTF8(cloud_url), upload->filename, buf, len, mime_type, json_hand, upload->callback, &logger);
		if (ret != NErr_ConnectionFailed)
		{
			const JSON::Value *value = 0;
			if (JSON_Tree_Finish(tree, &value) == NErr_Success)
			{
				const JSON::Value *error = 0;
				if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success ||
					!Config_GetSlimLogging())
				{
					logger.UpdateError(1);
				}
#ifdef DEBUG
				if (value) value->Dump(logger.ParseTest(), 0);
#endif
			}
		}
	}
	else
	{
		ret = cloud_socket.PostFile(AutoCharUTF8(cloud_url), upload->filename, buf, len, mime_type, json_hand, upload->callback, 0); 
	}

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(ret);
	}
	else
	{
		const JSON::Value *value;
		JSON_Tree_Finish(tree, &value);

		if (upload->callback)
			upload->callback->OnFinished(ret);
	}

	ReferenceCountedNXString message;
	NXStringCreateWithFormatting(&message,
								 (ret == NErr_Success ? "Uploaded: %s" : (ret == NErr_Aborted ? "Upload Cancelled: %s" : "Upload Failed: %s")),
								 AutoCharPrintfUTF8(upload->filename));
	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		itr->OnUploadDone(this, upload->filename, message, ret);
	}

	// only process error responses if there was anything which came back
	if (ret != NErr_ConnectionFailed)
	{
		const JSON::Value *value = 0;
		JSON_Tree_Finish(tree, &value);
		Parse_Upload_Error(upload, value, "upload");
	}

	JSON_Tree_Destroy(tree);
	return NErr_Success; // TODO
}

void CloudThread::Internal_Upload(UploadStruct *upload)
{
#if 0
	if (REPLICANT_API_ARTWORK)
	{
		artwork_t artwork;
		if (REPLICANT_API_ARTWORK->GetArtwork(upload->filename, MetadataKeys::ALBUM, &artwork) == NErr_Success)
		{
			yajl_gen hand = yajl_gen_alloc(0);

			yajl_gen_config(hand, yajl_gen_beautify);
			cloud_socket.JSON_Start(db_connection, hand, "metadata-art-attach");

			cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("id"), upload->internal_id);
			cloud_socket.JSON_AddString(hand, JSON_FIELD("content-hash"), "testtesttestestestestestestestestestestt");

			cloud_socket.JSON_End(hand);

			int ret;
			const unsigned char *buf;
			size_t len;
			yajl_gen_get_buf(hand, &buf, &len);


			ReferenceCountedNXString cloud_url;
			REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_True);
			Logger logger("art-attach", Logger::LOG_UPLOAD);
			cloud_socket.PostArt(AutoCharUTF8(cloud_url), artwork.data, buf, len, 0, &logger);
			yajl_gen_free(hand);
			logger.UpdateError(1);
		}
	}
#endif
	if (UploadLAN(upload) != NErr_Success)
		UploadServer(upload);
	delete upload;
}

int CloudThread::UploadPlaylist(nx_string_t uuid, int64_t playlist_id, int set)
{
	bool do_logging = Config_GetLogging();

	yajl_gen jspf_hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(jspf_hand, yajl_gen_beautify);

	nx_string_t name;
	int64_t id = 0, entries = 0, lastupdated = 0, created = 0;
	double duration = 0;
	db_connection->Playlists_Get(uuid, &id, &name, &duration, &entries, &lastupdated, &created);

	/* start of jspf generation */
	cloud_socket.JSON_Start_Action(jspf_hand, "playlist");
	yajl_gen_map_open(jspf_hand);

	cloud_socket.JSON_AddNXString(jspf_hand, JSON_FIELD("name"), name);
	ReferenceCountedNXString identifier;
	NXStringCreateWithFormatting(&identifier, "urn:nullsoft-com:playlist-uuid:%s", AutoCharPrintfUTF8(uuid));
	cloud_socket.JSON_AddNXString(jspf_hand, JSON_FIELD("identifier"), identifier);

	time_t t = 0;
	time(&t);
	struct tm *tm = gmtime(&t);
	char time_buf[64] = {0};
	sprintf(time_buf, "%i-%02i-%02iT%02i:%02i:%02iZ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	cloud_socket.JSON_AddString(jspf_hand, JSON_FIELD("date"), time_buf);

	cloud_socket.JSON_Start_Action(jspf_hand, "track");
	yajl_gen_array_open(jspf_hand);

	for (int i = 0; i < entries; i++)
	{
		PlaylistEntry *item = new (std::nothrow) PlaylistEntry;
		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnPlaylistUpload(this, db_connection, uuid, i, item);
		}

		yajl_gen_map_open(jspf_hand);

		if (item->location)
		{
			yajl_gen_string(jspf_hand, JSON_FIELD("location"));
			yajl_gen_array_open(jspf_hand);
			cloud_socket.JSON_GenNXString(jspf_hand, item->location);
			yajl_gen_array_close(jspf_hand);
		}

		yajl_gen_string(jspf_hand, JSON_FIELD("identifier"));
		yajl_gen_array_open(jspf_hand);

		if (item->media_id > 0)
		{
			ReferenceCountedNXString mediaid_urn;
			NXStringCreateWithFormatting(&mediaid_urn, "urn:nullsoft-com:media-id:%d", item->media_id);
			cloud_socket.JSON_GenNXString(jspf_hand, mediaid_urn);
		}
		
		if (item->metahash)
		{
			ReferenceCountedNXString metahash_urn;
			NXStringCreateWithFormatting(&metahash_urn, "urn:nullsoft-com:metahash:%s", AutoCharPrintfUTF8(item->metahash));
			cloud_socket.JSON_GenNXString(jspf_hand, metahash_urn);
		}

		if (item->mediahash)
		{
			ReferenceCountedNXString mediahash_urn;
			NXStringCreateWithFormatting(&mediahash_urn, "urn:nullsoft-com:mediahash:%s", AutoCharPrintfUTF8(item->mediahash));
			cloud_socket.JSON_GenNXString(jspf_hand, mediahash_urn);
		}
		
		yajl_gen_array_close(jspf_hand);

		if (item->title)
			cloud_socket.JSON_AddNXString(jspf_hand, JSON_FIELD("title"), item->title);

		if (item->duration >= 0)
			cloud_socket.JSON_AddInteger(jspf_hand, JSON_FIELD("duration"), item->duration);
		else
			cloud_socket.JSON_AddNull(jspf_hand, JSON_FIELD("duration"));

		yajl_gen_map_close(jspf_hand);
	}

	yajl_gen_array_close(jspf_hand);
	cloud_socket.JSON_End_Action(jspf_hand);

	yajl_gen_map_close(jspf_hand);
	cloud_socket.JSON_End_Action(jspf_hand);
	cloud_socket.JSON_End(jspf_hand);
	/* end of jspf generation */

	const unsigned char *jspf_data;
	size_t jspf_len;
	yajl_gen_get_buf(jspf_hand, &jspf_data, &jspf_len);
	Logger jspf_logger((set ? "metadata-playlist-set" : "metadata-playlist-new"), Logger::LOG_ALL);
	NXFileWrite(jspf_logger.RequestJSPF(), jspf_data, jspf_len);
	NXFileRelease(jspf_logger.RequestJSPF());
	yajl_gen_free(jspf_hand);

	yajl_gen hand = yajl_gen_alloc(0);
	if (do_logging)
		yajl_gen_config(hand, yajl_gen_beautify);

	cloud_socket.JSON_Start(db_connection, hand, (set ? "metadata-playlist-set" : "metadata-playlist-new"));
	cloud_socket.JSON_Start_Action(hand, "playlist");
	yajl_gen_map_open(hand);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("id"), playlist_id);
	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("uuid"), uuid);

	ReferenceCountedNXString playlisthash, playlist;
	NXStringCreateWithBytes(&playlist, jspf_data, jspf_len, nx_charset_utf8);
	db_connection->ComputePlaylistHash(playlist, &playlisthash);
	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("content-hash"), playlisthash);

	cloud_socket.JSON_AddNXString(hand, JSON_FIELD("name"), name);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("entries"), entries);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("duration"), (int64_t)duration);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("duration-exact"), 0);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("created"), lastupdated);
	cloud_socket.JSON_AddInteger64(hand, JSON_FIELD("lastupd"), lastupdated);
	//cloud_socket.JSON_AddNXURI(hand, JSON_FIELD("imported-filepath"), upload->filename);

	yajl_gen_map_close(hand);
	cloud_socket.JSON_End_Action(hand);
	cloud_socket.JSON_End(hand);

	int ret;
	const unsigned char *buf;
	size_t len;
	yajl_gen_get_buf(hand, &buf, &len);
	ReferenceCountedNXString cloud_url;
	REPLICANT_API_CLOUD->GetAPIURL(&cloud_url, /*http=*/NErr_False);

	yajl_handle json_hand=0;
	nsjson_tree_t tree;
	JSON_Tree_CreateParser(&tree);
	JSON_Tree_GetHandle(tree, &json_hand);

	if (!callbacks.empty())
	{
		ReferenceCountedNXString message, action;
		NXStringCreateWithUTF8(&action, (set ? "metadata-playlist-set" : "metadata-playlist-new")); // TODO: make this global singleton that's created at initialization
		NXStringCreateWithFormatting(&message, (set ? "Uploading Updated Playlist: %s [%d items]" : "Uploading New Playlist: %s [%d items]"), AutoCharPrintfUTF8(name), entries);
		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnAction(this, action, message);
		}
	}

	ReferenceCountedNXString mime_type;
	NXStringCreateWithUTF8(&mime_type, "application/xspf+json"); // TODO: make this global singleton that's created at initialization
	if (do_logging)
	{
		Logger logger((set ? "metadata-playlist-set" : "metadata-playlist-new"), Logger::LOG_UPLOAD);
		ret = cloud_socket.PostFile(AutoCharUTF8(cloud_url), jspf_logger.jspf_path, buf, len, mime_type, json_hand, 0, &logger);
		if (ret != NErr_ConnectionFailed)
		{
			const JSON::Value *value = 0;
			if (JSON_Tree_Finish(tree, &value) == NErr_Success)
			{
				const JSON::Value *error = 0;
				if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success ||
					!Config_GetSlimLogging())
				{
					logger.UpdateError(1);
					jspf_logger.UpdateError(1);
				}
#ifdef DEBUG
				if (value) value->Dump(logger.ParseTest(), 0);
#endif
			}
		}
	}
	else
	{
		ret = cloud_socket.PostFile(AutoCharUTF8(cloud_url), jspf_logger.jspf_path, buf, len, mime_type, json_hand, 0, 0); 
	}

	yajl_gen_free(hand);
	if (ret == NErr_Success)
	{
		const JSON::Value *value, *revision;
		JSON_Tree_Finish(tree, &value);
		if (value->FindNextKey(0, "rev", &revision, 0) == NErr_Success)
		{
			db_connection->BeginTransaction();
			int64_t revision_number;
			if (revision->GetInteger(&revision_number) == NErr_Success)
			{
				db_connection->Info_SetRevision(revision_number);
			}
			db_connection->Playlists_SetDirty(uuid, ifc_clouddb::PLAYLIST_DONE);
			db_connection->Commit();

			/*if (upload->callback)
				upload->callback->OnFinished(ret);*/

			if (!callbacks.empty())
			{
				ReferenceCountedNXString message, action;
				NXStringCreateWithUTF8(&action, (set ? "metadata-playlist-set" : "metadata-playlist-new")); // TODO: make this global singleton that's created at initialization
				NXStringCreateWithFormatting(&message, (set ? "Uploaded Updated Playlist: %s" : "Uploaded New Playlist: %s"), AutoCharPrintfUTF8(name));
				for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
				{
					itr->OnAction(this, action, message);
				}
			}
		}
	}
	else
	{
		// if we have a not known then could be we're out of sync
		// so we drop the playlist and attempt to ask the user...
		const JSON::Value *value = 0, *code = 0, *error = 0;
		JSON_Tree_Finish(tree, &value);
		if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success)
		{
			if (error->FindNextKey(0, "code", &code, 0) == NErr_Success)
			{
				int64_t code_int = 0;
				code->GetInteger(&code_int);

				// if already exists then prevent trying to re-add
				// and as applicable update the local client modes
				if (code_int == 88)
				{
					db_connection->Playlists_SetDirty(uuid, ifc_clouddb::PLAYLIST_DONE);
					/*for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
					{
						// TODO
						itr->OnPlaylistAddUpdate(this, uuid);
					}*/
				}
				// if already removed then need to deal with and
				// as applicable update the local client storage
				else if (code_int == 80)
				{
					db_connection->Playlists_Removed(uuid);

					for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
					{
						itr->OnPlaylistRemove(this, db_connection, uuid);
					}
				}
			}
		}

		/*if (upload->callback)
			upload->callback->OnFinished(ret);*/
	}

	// only process error responses if there was anything which came back
	if (ret != NErr_ConnectionFailed)
	{
		const JSON::Value *value = 0;
		JSON_Tree_Finish(tree, &value);
		Parse_Upload_Error(0/*upload*/, value, (set ? "metadata-playlist-set" : "metadata-playlist-new"));
	}

	JSON_Tree_Destroy(tree);
	return NErr_Success; // TODO
}

int CloudThread::Parse_Upload_Error(UploadStruct *upload, const JSON::Value *value, const char *subdir)
{
	const JSON::Value *error = 0, *code = 0, *str = 0, *field = 0;
	if (value && value->FindNextKey(0, "error", &error, 0) == NErr_Success)
	{
		ReferenceCountedNXString code_string, str_string, field_string, action;
		if (error->FindNextKey(0, "code", &code, 0) == NErr_Success)
		{
			code->GetString(&code_string);
		}
		if (error->FindNextKey(0, "str", &str, 0) == NErr_Success)
		{
			str->GetString(&str_string);
		}
		if (error->FindNextKey(0, "field", &field, 0) == NErr_Success)
		{
			field->GetString(&field_string);
		}
		NXStringCreateWithUTF8(&action, subdir);

		// we fire off an error callback to the upload callback as well as global ones
		if (upload && upload->callback)
			upload->callback->OnError(action, code_string, str_string, field_string);

		for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
		{
			itr->OnError(this, action, code_string, str_string, field_string);
		}

		return 1;
	}
	return 0;
}

ns_error_t CloudThread::CloudClient_Upload(nx_uri_t filename, nx_string_t destination_device, int internal_id, cb_cloud_upload *callback)
{
	UploadStruct *upload = new (std::nothrow) UploadStruct;
	if (!upload)
		return NErr_OutOfMemory;

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
	{
		delete upload;
		return NErr_OutOfMemory;
	}

	upload->filename = NXURIRetain(filename);
	upload->destination_device = NXStringRetain(destination_device);
	upload->internal_id = internal_id;
	upload->callback = callback;
	if (callback)
		callback->Retain();

	apc->func = APC_Upload;
	apc->param1 = this;
	apc->param2 = upload;
	thread_loop.Schedule(apc);
	return NErr_Success;
}