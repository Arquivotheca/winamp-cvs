#include "CloudDB.h"
#include "main.h"
static const int schema_version=1;
#define SQLPARAM(x) x, sizeof(x)
static const char sql_info_get_devicename[] = "SELECT friendly_name FROM [info]";
static const char sql_info_set_devicename[] = "UPDATE [info] SET friendly_name=?";
static const char sql_info_increment_revision[] = "UPDATE [info] SET revision=revision+1";
static const char sql_info_get_revision[] = "SELECT revision FROM [info]";
static const char sql_info_set_revision[] = "UPDATE [info] SET revision=?";
static const char sql_info_get_logging[] = "SELECT logging FROM [info]";
static const char sql_info_set_logging[] = "UPDATE [info] SET logging=?";
static const char sql_info_get_revision_id[] = "SELECT revision_id FROM [info]";
static const char sql_info_set_revision_id[] = "UPDATE [info] SET revision_id=?";
static const char sql_info_populate[] = 
"INSERT OR IGNORE INTO [info] "
"(device, schema, revision) "
"VALUES "
"(?, ?, 0)";


int Cloud_DBConnection::Info_Populate(nx_string_t device_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_populate, SQLPARAM(sql_info_populate), device_id, schema_version);
	AutoResetStatement auto_reset(statement_info_populate);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Info_GetDeviceName(nx_string_t *device_name)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_getdevicename, SQLPARAM(sql_info_get_devicename));
	AutoResetStatement auto_reset(statement_info_getdevicename);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_info_getdevicename, device_name);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Info_SetDeviceName(nx_string_t device_name)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_setdevicename, SQLPARAM(sql_info_set_devicename), device_name);
	AutoResetStatement auto_reset(statement_info_setdevicename);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Info_IncrementRevision()
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_incrementrevision, SQLPARAM(sql_info_increment_revision));
	AutoResetStatement auto_reset(statement_info_incrementrevision);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Info_GetRevision(int64_t *revision)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_getrevision, SQLPARAM(sql_info_get_revision));
	AutoResetStatement auto_reset(statement_info_getrevision);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_info_getrevision, revision);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Info_SetRevision(int64_t revision)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_setrevision, SQLPARAM(sql_info_set_revision), revision);
	AutoResetStatement auto_reset(statement_info_setrevision);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Info_GetLogging(int *logging)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_info_getlogging, SQLPARAM(sql_info_get_logging));
	AutoResetStatement auto_reset(statement_info_getlogging);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_info_getlogging, logging);
		logMode = *logging;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Info_SetLogging(int logging)
{
	int sqlite_ret;
	logMode = logging;

	sqlite_ret=Step(statement_info_setlogging, SQLPARAM(sql_info_set_logging), logging);
	AutoResetStatement auto_reset(statement_info_setlogging);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Info_GetRevisionID(nx_string_t *revision_id)
{
		int sqlite_ret;

	sqlite_ret=Step(statement_info_getrevisionid, SQLPARAM(sql_info_get_revision_id));
	AutoResetStatement auto_reset(statement_info_getrevisionid);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_info_getrevisionid, revision_id);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

	int Cloud_DBConnection::Info_SetRevisionID(nx_string_t revision_id)
	{
			int sqlite_ret;

	sqlite_ret=Step(statement_info_setrevisionid, SQLPARAM(sql_info_set_revision_id), revision_id);
	AutoResetStatement auto_reset(statement_info_setrevisionid);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
	}