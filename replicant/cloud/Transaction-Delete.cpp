#include "TransactionQueue.h"
#include "CloudDB.h"
#include "CloudSocket.h"
#include "nswasabi/ReferenceCounted.h"

ns_error_t Transaction_Delete::Initialize(Cloud_DBConnection *db_connection, int internal_id, CallbackList callbacks)
{
	this->internal_id = internal_id;
	this->callbacks = callbacks;
	this->cloud_id = 0;
	return db_connection->IDMap_Get(internal_id, &cloud_id);
}

ns_error_t Transaction_Delete::GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen hand)
{
	if (cloud_id > 0)
	{
		cloud_socket->JSON_Start_Action(hand, "delete");
		yajl_gen_map_open(hand);
		cloud_socket->JSON_AddInteger64(hand, JSON_FIELD("id"), cloud_id);
		yajl_gen_map_close(hand);
		cloud_socket->JSON_End_Action(hand);


		if (!callbacks.empty())
		{
			ReferenceCountedNXString message, action;
			NXStringCreateWithUTF8(&action, "metadata-push-delete"); // TODO: make this global singleton that's created at initialization
			NXStringCreateWithFormatting(&message, "Removing: Cloud-ID = %d", cloud_id);
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnAction(0, action, message);
			}
		}

		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t Transaction_Delete::OnTransactionAccepted(Cloud_DBConnection *db_connection)
{
	db_connection->IDMap_Removed(internal_id);
	return NErr_Success;
}

ns_error_t Transaction_Delete::OnTransactionError(Cloud_DBConnection *db_connection)
{
	return NErr_Success;
}