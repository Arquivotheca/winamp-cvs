#pragma once
#include "nx/nxuri.h"
#include "foundation/error.h"
#include "../libyajl/include/yajl/yajl_gen.h"
#include "nu/PtrDeque.h"
#include "Attributes.h"
#include "cb_cloudevents.h"

class CloudSocket;
class Cloud_DBConnection;

/*enum TransactionAction
{
	ACTION_NONE,
	ACTION_ANNOUNCE,
	ACTION_UPDATE,
	ACTION_DELETE,
	ACTION_FLUSH,
};*/

typedef nu::PtrDeque<cb_cloudevents> CallbackList;

class Transaction : public nu::PtrDequeNode
{
public:
	virtual ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json)=0;
	virtual ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection)=0;
	virtual ns_error_t OnTransactionError(Cloud_DBConnection *db_connection)=0;
};

class Transaction_Announce : public Transaction
{
public:
	Transaction_Announce();
	~Transaction_Announce();
	ns_error_t Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id);
private:
	ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json);
	ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection);
	ns_error_t OnTransactionError(Cloud_DBConnection *db_connection);
private:
	int internal_id;
	int64_t cloud_id;
	Attributes *attributes;
};

class Transaction_Delete : public Transaction
{
public:
	ns_error_t Initialize(Cloud_DBConnection *db_connection, int internal_id, CallbackList callbacks);
private:
	ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json);
	ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection);
	ns_error_t OnTransactionError(Cloud_DBConnection *db_connection);
private:
	int internal_id;
	int64_t cloud_id;
	CallbackList callbacks;
};

class Transaction_Update : public Transaction
{
public:
	ns_error_t Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id, CallbackList callbacks);
private:
	ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json);
	ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection);
	ns_error_t OnTransactionError(Cloud_DBConnection *db_connection);
private:
	int internal_id;
	int64_t cloud_id;
	CallbackList callbacks;
	Attributes *attributes;
};

class Transaction_Reset : public Transaction
{
private:
	ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json);
	ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection);
	ns_error_t OnTransactionError(Cloud_DBConnection *db_connection);
};

class Transaction_Played : public Transaction
{
public:
	ns_error_t Initialize(Cloud_DBConnection *db_connection, Attributes &attributes, int internal_id, CallbackList callbacks);
private:
	ns_error_t GenerateJSON(Cloud_DBConnection *db_connection, CloudSocket *cloud_socket, int *id_iterator, yajl_gen json);
	ns_error_t OnTransactionAccepted(Cloud_DBConnection *db_connection);
	ns_error_t OnTransactionError(Cloud_DBConnection *db_connection);
private:
	int internal_id;
	int64_t cloud_id;
	CallbackList callbacks;
	Attributes *attributes;
};