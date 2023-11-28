#include "CloudDB.h"
#include "nu/vector.h"
#define SQLPARAM(x) x, sizeof(x)

static const char sql_devices_add[] = "INSERT INTO [devices] (device, friendly_name) VALUES (?, ?)";
static const char sql_devices_find[] = "SELECT device_id FROM [devices] WHERE device=?";
static const char sql_devices_find_full[] = "SELECT device_id, platform, device_type, friendly_name FROM [devices] WHERE device=? ORDER BY device_id ASC";
static const char sql_devices_remove[] = "DELETE FROM [devices] WHERE device_id=?";
static const char sql_devices_update[] = "UPDATE [devices] SET friendly_name=? WHERE device_id=?";
static const char sql_devices_update_extra[] = "UPDATE [devices] SET platform=?, device_type=? WHERE device_id=?";
static const char sql_devices_getname[] = "SELECT friendly_name, device FROM [devices] WHERE device_id=? AND LENGTH(friendly_name)";
static const char sql_devices_list[] = "SELECT device FROM [devices] ORDER BY device_id ASC";
static const char sql_device_ids_list[] = "SELECT device_id FROM [devices] ORDER BY device_id ASC";
static const char sql_devices_get_capacity[] = "SELECT device_size, device_used FROM [devices] WHERE device_id=?";
static const char sql_devices_save_capacity[] = "UPDATE [devices] SET device_size=?, device_used=? WHERE device_id=?";
static const char sql_devices_get_lastseen[] = "SELECT last_seen, device_on FROM [devices] WHERE device_id=?";
static const char sql_devices_set_lastseen[] = "UPDATE [devices] SET last_seen=?, device_on=? WHERE device_id=?";
static const char sql_devices_set_availability[] = "UPDATE [devices] SET transient=?, availability=? WHERE device_id=?";
static const char sql_devices_set_local[] = "UPDATE [devices] SET local=1 WHERE device_id=?";
static const char sql_devices_set_lan[] = "UPDATE [devices] SET lan=? WHERE device_id=?";
static const char sql_devices_reset_lan[] = "UPDATE [devices] SET lan=0";

int Cloud_DBConnection::CloudDB_Devices_Add(nx_string_t device_token, nx_string_t friendly_name, DeviceInfoStruct *device_info, int *device_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_find, SQLPARAM(sql_devices_find), device_token);
	AutoResetStatement auto_reset(statement_devices_find);
	if (sqlite_ret == SQLITE_ROW)
	{
		// already in the table, so do an update
		Columns(statement_devices_find, device_id);

		if (friendly_name)
		{
			sqlite_ret=Step(statement_devices_update, SQLPARAM(sql_devices_update), friendly_name, *device_id);
			AutoResetStatement auto_reset(statement_devices_update);
			sqlite_ret=sqlite3_reset(statement_devices_update);
		}

		if (device_info)
		{
			sqlite_ret=Step(statement_devices_update_extra, SQLPARAM(sql_devices_update_extra), device_info->platform, device_info->type, *device_id);
			AutoResetStatement auto_reset(statement_devices_update_extra);
			sqlite_ret=sqlite3_reset(statement_devices_update_extra);
		}
		return NErr_NoAction;
	}
	else
	{
		// insert
		sqlite_ret=Step(statement_devices_add, SQLPARAM(sql_devices_add), device_token, friendly_name);
		AutoResetStatement auto_reset(statement_devices_add);
		*device_id = (int)sqlite3_last_insert_rowid(database_connection);
		return NErr_Success;
	}
}

int Cloud_DBConnection::CloudDB_Devices_Remove(int device_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_remove, SQLPARAM(sql_devices_remove), device_id);
	AutoResetStatement auto_reset(statement_devices_remove);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Devices_Find(nx_string_t device_token, int *device_id, DeviceInfoStruct *device_info)
{
	int sqlite_ret;

	if (device_info)
	{
		sqlite_ret=Step(statement_devices_find_full, SQLPARAM(sql_devices_find_full), device_token);
		AutoResetStatement auto_reset(statement_devices_find_full);
		if (sqlite_ret == SQLITE_ROW)
		{
			Columns(statement_devices_find_full, device_id, &device_info->platform, &device_info->type, &device_info->name);
			return NErr_Success;
		}
		else
			return NErr_Empty;
	}
	else
	{
		sqlite_ret=Step(statement_devices_find, SQLPARAM(sql_devices_find), device_token);
		AutoResetStatement auto_reset(statement_devices_find);
		if (sqlite_ret == SQLITE_ROW)
		{
			Columns(statement_devices_find, device_id);
			return NErr_Success;
		}
		else
			return NErr_Empty;
	}
}

int Cloud_DBConnection::CloudDB_Devices_GetIDs(nx_string_t **devices, size_t *num_devices)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_list, SQLPARAM(sql_devices_list));
	AutoResetStatement auto_reset(statement_devices_list);
	Vector<nx_string_t, 32, 2> ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		nx_string_t value;
		sqlite3_column_any(statement_devices_list, 0, &value);
		ids.push_back(value);
		sqlite_ret=sqlite3_step(statement_devices_list);
	}
	*devices = (nx_string_t *)malloc(sizeof(nx_string_t *) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*devices)[i] = ids[i];
	}

	*num_devices = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_Devices_GetDeviceIDs(int **device_ids, size_t *num_devices)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_device_ids_list, SQLPARAM(sql_device_ids_list));
	AutoResetStatement auto_reset(statement_device_ids_list);
	Vector<int, 32, 2> ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_device_ids_list, 0));
		sqlite_ret=sqlite3_step(statement_device_ids_list);
	}
	*device_ids = (int *)malloc(sizeof(int *) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*device_ids)[i] = ids[i];
	}

	*num_devices = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_Devices_GetName(int device_id, nx_string_t *name, nx_string_t *device_token)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_getname, SQLPARAM(sql_devices_getname), device_id);
	AutoResetStatement auto_reset(statement_devices_getname);

	if (sqlite_ret == SQLITE_ROW)
	{
		if (!device_token) Columns(statement_devices_getname, name);
		else Columns(statement_devices_getname, name, device_token);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Devices_GetCapacity(int device_id, int64_t *total_size, int64_t *used_size)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_get_capacity, SQLPARAM(sql_devices_get_capacity), device_id);
	AutoResetStatement auto_reset(statement_devices_get_capacity);

	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_devices_get_capacity, total_size, used_size);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Devices_StoreCapacity(int device_id, int64_t total_size, int64_t used_size)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_save_capacity, SQLPARAM(sql_devices_save_capacity), total_size, used_size, device_id);
	AutoResetStatement auto_reset(statement_devices_save_capacity);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Devices_GetLastSeen(int device_id, int64_t *last_seen, int *on)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_get_lastseen, SQLPARAM(sql_devices_get_lastseen), device_id);
	AutoResetStatement auto_reset(statement_devices_get_lastseen);

	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_devices_get_lastseen, last_seen, on);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Devices_SetLastSeen(int device_id, int64_t last_seen, int on)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_set_lastseen, SQLPARAM(sql_devices_set_lastseen), last_seen, on, device_id);
	AutoResetStatement auto_reset(statement_devices_set_lastseen);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Devices_SetAvailability(int device_id, int64_t transient, double availability)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_set_availability, SQLPARAM(sql_devices_set_availability), transient, availability, device_id);
	AutoResetStatement auto_reset(statement_devices_set_availability);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Devices_SetLocal(int device_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_set_local, SQLPARAM(sql_devices_set_local), device_id);
	AutoResetStatement auto_reset(statement_devices_set_local);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Devices_SetLAN(int device_id, int lan)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_set_lan, SQLPARAM(sql_devices_set_lan), lan, device_id);
	AutoResetStatement auto_reset(statement_devices_set_lan);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::Devices_ResetLAN()
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devices_reset_lan, SQLPARAM(sql_devices_reset_lan));
	AutoResetStatement auto_reset(statement_devices_reset_lan);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}