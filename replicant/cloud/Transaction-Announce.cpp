#include "api.h"
#include "TransactionQueue.h"
#include "CloudDB.h"
#include "CloudSocket.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include <stdio.h>

#if defined(_MSC_VER) 
#define snprintf _snprintf
#endif

extern CloudAPI cloud_api;

Transaction_Announce::Transaction_Announce()
{
	attributes=0;

	internal_id=-1;
	cloud_id=-1;
}

Transaction_Announce::~Transaction_Announce()
{
}

ns_error_t Transaction_Announce::Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id)
{
	this->attributes = &attributes;
	this->internal_id = internal_id;
	this->cloud_id = 0;

	return NErr_Success;
}

inline static ns_error_t DBtoJSON_String(yajl_gen hand, Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int internal_id, const unsigned char *field, size_t field_cch, const char *column, size_t limit)
{
	ReferenceCountedNXString value;

	if (db_connection->IDMap_GetString(internal_id, column, &value) == NErr_Success)
	{
		if (value->len > 0)
		{
			cloud_socket->JSON_AddNXString(hand, field, field_cch, value, limit);
			return NErr_Success;
		}
		else
			return NErr_Empty;
	}
	return NErr_Error;
}

inline static void DBtoJSON_Integer(yajl_gen hand, Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int internal_id, const unsigned char *field, size_t field_cch, const char *column)
{
	int64_t value;

	if (db_connection->IDMap_GetInteger(internal_id, column, &value) == NErr_Success)
		if (value)
			cloud_socket->JSON_AddInteger64(hand, field, field_cch, value);
}

ns_error_t Transaction_Announce::GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen hand)
{
	// make sure it's not already in the database
	if (db_connection->IDMap_Get(internal_id, &cloud_id) == NErr_Success && cloud_id != 0)
	{
		return NErr_Empty;
	}
	ReferenceCountedNXString mediahash, metahash, idhash, title;
	ReferenceCountedNXURI filepath;

	// ensure we have mediahash and idhash to send in order to prevent spamming the server
	if (db_connection->IDMap_GetMediaHash(internal_id, &mediahash) != NErr_Success)
		return NErr_Empty;

	if (db_connection->IDMap_GetIDHash(internal_id, &idhash) != NErr_Success)
			return NErr_Empty;

	cloud_socket->JSON_Start_Action(hand, "announce");
	yajl_gen_map_open(hand);


	cloud_socket->JSON_AddNXString(hand, JSON_FIELD("mediahash"), mediahash, 40);
	cloud_socket->JSON_AddNXString(hand, JSON_FIELD("idhash"), idhash, 40);

	DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("artist"), "artist", 1024);
	DBtoJSON_String(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("album"), "album", 1024);
	if (db_connection->IDMap_GetTitle(internal_id, &title) == NErr_Success)
			cloud_socket->JSON_AddNXString(hand, JSON_FIELD("title"), title, 1024);
	
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
	if (playcount) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("playcount"), playcount);
	if (lastplayed) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("lastplay"), lastplayed);
	if (lastupdated) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("lastupd"), lastupdated);
	if (filetime) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("filetime"), filetime);
	if (filesize) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("filesize"), filesize);
	if (bitrate) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("bitrate"), bitrate);
	DBtoJSON_Integer(hand, db_connection, cloud_socket, internal_id, JSON_FIELD("added"), "added");

	ReferenceCountedNXString mime_type;
	if (db_connection->IDMap_GetMIME(internal_id, &mime_type) == NErr_Success)
		cloud_socket->JSON_AddNXString(hand, JSON_FIELD("mimetype"), mime_type, 127);

	ReferenceCountedNXString arthash;
	if (db_connection->Artwork_Get(internal_id, &arthash) == NErr_Success)
	{
		yajl_gen_string(hand, (const unsigned char *)"art", 3);	
		yajl_gen_map_open(hand);
		cloud_socket->JSON_AddNXString(hand, JSON_FIELD("album"), arthash, 40);
		yajl_gen_map_close(hand);
	}

	db_connection->IDMap_Next(&cloud_id);
	cloud_id += *id_iterator;
	cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("id"), cloud_id);
	*id_iterator = (*id_iterator) + 1;

	yajl_gen_map_close(hand);
	cloud_socket->JSON_End_Action(hand);

	return NErr_Success;
}


ns_error_t Transaction_Announce::OnTransactionAccepted(Cloud_DBConnection *db_connection)
{
	db_connection->IDMap_Set(internal_id, cloud_id);
	// TODO: we should check lastupdate at the start and then again here.  if lastupdate changed, don't set dirty=0
	db_connection->IDMap_SetDirty(internal_id, 0);
	return NErr_Success;
}

ns_error_t Transaction_Announce::OnTransactionError(Cloud_DBConnection *db_connection)
{
	// on failure, set the ignore flag to skip it
	db_connection->IDMap_SetIgnore(internal_id);
	return NErr_Success;
}
