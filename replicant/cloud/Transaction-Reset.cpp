#include "TransactionQueue.h"
#include "CloudSocket.h"

ns_error_t Transaction_Reset::GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json)
{
	cloud_socket->JSON_Start_Action(json, "test-repo-reset");
	yajl_gen_map_open(json);
	yajl_gen_map_close(json);
	cloud_socket->JSON_End_Action(json);
	return NErr_Success;
}

ns_error_t Transaction_Reset::OnTransactionAccepted(Cloud_DBConnection *db_connection)
{
	return NErr_Success;
}

ns_error_t Transaction_Reset::OnTransactionError(Cloud_DBConnection *db_connection)
{
	return NErr_Success;
}