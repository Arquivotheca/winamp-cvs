#include "TransactionQueue.h"
#include "CloudDB.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudSocket.h"

ns_error_t Transaction_Played::Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id, CallbackList callbacks)
{
	this->internal_id = internal_id;
	this->attributes = &attributes;
	this->callbacks = callbacks;
	this->cloud_id = 0;
	return db_connection->IDMap_Get(internal_id, &cloud_id);
}

ns_error_t Transaction_Played::GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen hand)
{
	if (cloud_id > 0)
	{
		ReferenceCountedNXString mediahash, metahash, idhash;
		ReferenceCountedNXURI filepath;
				// ensure we have mediahash and idhash to send in order to prevent spamming the server

		if (db_connection->IDMap_GetMediaHash(internal_id, &mediahash) != NErr_Success)
			return NErr_Empty;

		if (db_connection->IDMap_GetIDHash(internal_id, &idhash) != NErr_Success)
			return NErr_Empty;

		cloud_socket->JSON_Start_Action(hand, "update");
		yajl_gen_map_open(hand);
		cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("id"), cloud_id);


		int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
		double duration = 0;
		db_connection->IDMap_GetProperties(internal_id, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration);
		if (playcount) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("playcount"), playcount);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("playcount"));
		if (lastplayed) cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("lastplay"), lastplayed);
		else cloud_socket->JSON_AddNull(hand, JSON_FIELD("lastplay"));

		yajl_gen_map_close(hand);
		cloud_socket->JSON_End_Action(hand);

		if (!callbacks.empty())
		{
			ReferenceCountedNXString message, action;
			NXStringCreateWithUTF8(&action, "metadata-push-update"); // TODO: make this global singleton that's created at initialization
			NXStringCreateWithFormatting(&message, "Updating Play Count: Cloud-ID = %d", cloud_id);
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnAction(0, action, message);
			}
		}

		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t Transaction_Played::OnTransactionAccepted(Cloud_DBConnection *db_connection)
{
	// TODO: we should check lastupdate at the start and then again here.  if lastupdate changed, don't set dirty=0
	db_connection->IDMap_SetDirty(internal_id, 0);
	return NErr_Success;
}

ns_error_t Transaction_Played::OnTransactionError(Cloud_DBConnection *db_connection)
{
	// on failure, set the dirty flag to zero to skip it
	// TODO: we should check lastupdate at the start and then again here.  if lastupdate changed, don't set dirty=0
	db_connection->IDMap_SetDirty(internal_id, 0);
	// TODO: benski> maybe this instead? not sure.	db_connection->IDMap_SetIgnore(internal_id);
	return NErr_Success;
}
