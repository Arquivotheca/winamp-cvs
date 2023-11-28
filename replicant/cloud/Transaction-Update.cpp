#include "TransactionQueue.h"
#include "CloudDB.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudSocket.h"

ns_error_t Transaction_Update::Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id, CallbackList callbacks)
{
	this->internal_id = internal_id;
	this->attributes = &attributes;
	this->callbacks = callbacks;
	this->cloud_id = 0;
	return db_connection->IDMap_Get(internal_id, &cloud_id);
}

inline static ns_error_t DBtoJSON_String(yajl_gen hand, Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int internal_id, const unsigned char *field, size_t field_cch, const char *column, size_t limit)
{
	ReferenceCountedNXString value;

	if (db_connection->IDMap_GetString(internal_id, column, &value) == NErr_Success)
	{
		cloud_socket->JSON_AddNXString(hand, field, field_cch, value, limit);
		return NErr_Success;
	}
	else
	{
		cloud_socket->JSON_AddNull(hand, field, field_cch);
		return NErr_Empty;
	}
}

inline static void DBtoJSON_Integer(yajl_gen hand, Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int internal_id, const unsigned char *field, size_t field_cch, const char *column)
{
	int64_t value;

	if (db_connection->IDMap_GetInteger(internal_id, column, &value) == NErr_Success && value)
		cloud_socket->JSON_AddInteger64(hand, field, field_cch, value);
	else
		cloud_socket->JSON_AddNull(hand, field, field_cch);
}

ns_error_t Transaction_Update::GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen hand)
{
	if (cloud_id > 0)
	{
		ReferenceCountedNXString mediahash, metahash, idhash, title;
		ReferenceCountedNXURI filepath;
				// ensure we have mediahash and idhash to send in order to prevent spamming the server

		if (db_connection->IDMap_GetMediaHash(internal_id, &mediahash) != NErr_Success)
			return NErr_Empty;

		if (db_connection->IDMap_GetIDHash(internal_id, &idhash) != NErr_Success)
			return NErr_Empty;

		cloud_socket->JSON_Start_Action(hand, "update");
		yajl_gen_map_open(hand);
		cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("id"), cloud_id);


		cloud_socket->JSON_AddNXString(hand, JSON_FIELD("mediahash"), mediahash, 40);
		cloud_socket->JSON_AddNXString(hand, JSON_FIELD("idhash"), idhash, 40);

		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("artist"), "artist", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("album"), "album", 1024);
		if (db_connection->IDMap_GetTitle(internal_id, &title) == NErr_Success)
			cloud_socket->JSON_AddNXString(hand, JSON_FIELD("title"), title, 1024);
		else
			cloud_socket->JSON_AddNull(hand, JSON_FIELD("title"));
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("trackno"), "trackno");
		if (db_connection->IDMap_GetMetaHash(internal_id, &metahash) == NErr_Success)
			cloud_socket->JSON_AddNXString(hand, JSON_FIELD("metahash"), metahash, 40);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("albumartist"), "albumartist", 1024);
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("bpm"), "bpm");
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("category"), "category", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("comment"), "comment", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("composer"), "composer", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("director"), "director", 1024);
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("disc"), "disc");
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("discs"), "discs");
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("genre"), "genre", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("producer"), "producer", 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("publisher"), "publisher", 1024);
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("tracks"), "tracks");
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("year"), "year", 19);
		if (db_connection->IDMap_Get_Filepath(internal_id, &filepath) == NErr_Success)
			cloud_socket->JSON_AddNXURI(hand, JSON_FIELD("filepath"), filepath, 1024);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("albumgain"), "albumgain", 32);
		DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("trackgain"), "trackgain", 32);
		DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("rating"), "rating");

		int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
		double duration = 0;
		db_connection->IDMap_GetProperties(internal_id, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration);
		if (duration) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("length"), (int64_t)(duration*1000.0));
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("length"));
		if (playcount) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("playcount"), playcount);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("playcount"));
		if (lastplayed) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("lastplay"), lastplayed);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("lastplay"));
		if (lastupdated) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("lastupd"), lastupdated);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("lastupd"));
		if (filetime) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("filetime"), filetime);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("filetime"));
		if (filesize) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("filesize"), filesize);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("filesize"));
		if (bitrate) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("bitrate"), bitrate);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("bitrate"));

		ReferenceCountedNXString mime_type;
		if (db_connection->IDMap_GetMIME(internal_id, &mime_type) == NErr_Success)
			cloud_socket->JSON_AddNXString(hand, JSON_FIELD("mimetype"), mime_type, 127);
																				 
		/* TODO: benski> we should only put the artwork in the update if the artwork.lastupdated >= idmap.lastupdated */
		ReferenceCountedNXString arthash;
		if (db_connection->Artwork_Get(internal_id, &arthash) == NErr_Success)
		{
			yajl_gen_string(hand, (const unsigned char *)"art", 3);	
			yajl_gen_array_open(hand);
			yajl_gen_map_open(hand);
			cloud_socket->JSON_AddString(hand, JSON_FIELD("type"), "album");
			cloud_socket->JSON_AddNXString(hand, JSON_FIELD("hash"), arthash, 40);
			// TODO cloud_socket->JSON_AddNXString(hand, JSON_FIELD("source"), source, 1024);
			// TODO cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("filetime"), filetime);
			// TODO cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("sourcetime"), sourcetime);
			yajl_gen_map_close(hand);
			yajl_gen_array_close(hand);
		}

		yajl_gen_map_close(hand);
		cloud_socket->JSON_End_Action(hand);

		if (!callbacks.empty())
		{
			ReferenceCountedNXString message, action;
			NXStringCreateWithUTF8(&action, "metadata-push-update"); // TODO: make this global singleton that's created at initialization
			NXStringCreateWithFormatting(&message, "Updating: Cloud-ID = %d", cloud_id);
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnAction(0, action, message);
			}
		}

		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t Transaction_Update::OnTransactionAccepted(Cloud_DBConnection *db_connection)
{
	// TODO: we should check lastupdate at the start and then again here.  if lastupdate changed, don't set dirty=0
	db_connection->IDMap_SetDirty(internal_id, 0);
	return NErr_Success;
}

ns_error_t Transaction_Update::OnTransactionError(Cloud_DBConnection *db_connection)
{
	// on failure, set the dirty flag to zero to skip it
	// TODO: we should check lastupdate at the start and then again here.  if lastupdate changed, don't set dirty=0
	db_connection->IDMap_SetDirty(internal_id, 0);
	// TODO: benski> maybe this instead? not sure.	db_connection->IDMap_SetIgnore(internal_id);
	return NErr_Success;
}
