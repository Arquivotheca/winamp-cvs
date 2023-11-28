#include "CloudDB.h"
#include "nu/vector.h"
#include <stdlib.h>

/* [idmap] schema
[idmap] ("
media_id INTEGER PRIMARY KEY AUTOINCREMENT
cloud_id INTEGER
mediahash TEXT DEFAULT NULL
device_id INTEGER
filename TEXT
metahash TEXT
ignore INTEGER DEFAULT 0
*/

#define SQLPARAM(x) x, sizeof(x)
static const char sql_idmap_generate[] = "INSERT INTO [idmap] (cloud_id, device_id, filepath) VALUES (NULL, ?, ?)";
static const char sql_idmap_associate[] = "INSERT INTO [idmap] (cloud_id, device_id, filepath) VALUES (?, ?, ?)";
static const char sql_idmap_set[] = "UPDATE [idmap] SET cloud_id=? WHERE media_id=?";
static const char sql_idmap_next[] = "SELECT coalesce(max(cloud_id)+1, 1) FROM [idmap]";
static const char sql_idmap_find[] = "SELECT media_id FROM [idmap] WHERE cloud_id=?";
static const char sql_idmap_get[] = "SELECT cloud_id FROM [idmap] WHERE media_id=?";
static const char sql_idmap_remove[] = "UPDATE [idmap] SET ignore=3, filepath=media_id WHERE media_id=?";
static const char sql_idmap_delete[] = "UPDATE [idmap] SET ignore=4, filepath=media_id WHERE media_id=?";
static const char sql_idmap_removed[] = "UPDATE [idmap] SET ignore=5, filepath=media_id WHERE media_id=?";
static const char sql_idmap_get_metahash[] = "SELECT metahash FROM [idmap] WHERE media_id=?";
static const char sql_idmap_get_mediahash[] = "SELECT mediahash FROM [idmap] WHERE media_id=?";
static const char sql_idmap_set_mediahash[] = "UPDATE [idmap] SET mediahash=? WHERE media_id=?";
static const char sql_idmap_get_title[] = "SELECT title FROM [idmap] WHERE media_id=?";
static const char sql_idmap_set_title[] = "UPDATE [idmap] SET title=? WHERE media_id=?";
static const char sql_idmap_get_unannounced[] = "SELECT media_id FROM [idmap] WHERE cloud_id IS NULL AND mediahash IS NOT NULL AND ignore=0";
static const char sql_idmap_get_to_remove[] = "SELECT media_id FROM [idmap] WHERE cloud_id IS NOT NULL AND mediahash IS NOT NULL AND ignore=3";
static const char sql_idmap_get_to_delete[] = "SELECT media_id FROM [idmap] WHERE cloud_id IS NOT NULL AND mediahash IS NOT NULL AND ignore=4";
static const char sql_idmap_get_mediahash_null[] = "SELECT media_id FROM [idmap] WHERE mediahash IS NULL AND ignore=0 AND device_id=?";
static const char sql_idmap_get_devices_from_mediahash[] = "SELECT device_id, media_id FROM [idmap] WHERE mediahash=? AND ignore=0";
static const char sql_idmap_get_devices_tokens_from_mediahash[] = "SELECT [idmap].device_id, [devices].device FROM [idmap] inner join [devices] on [idmap].device_id = [devices].device_id WHERE mediahash=? AND ignore=0";
static const char sql_idmap_get_devices_from_metahash[] = "SELECT device_id, media_id FROM [idmap] WHERE metahash=? AND ignore=0";
static const char sql_idmap_get_ids_from_metahash[] = "SELECT media_id, device_id FROM [idmap] WHERE metahash=? AND ignore=0";
static const char sql_idmap_set_metahash[] = "UPDATE [idmap] SET metahash=? WHERE media_id=?";
static const char sql_idmap_set_albumhash[] = "UPDATE [idmap] SET albumhash=? WHERE media_id=?";
static const char sql_idmap_set_ignore[] = "UPDATE [idmap] SET ignore=1 WHERE media_id=?";
static const char sql_idmap_reset_ignored_one[] = "UPDATE [idmap] SET ignore=0 WHERE media_id=?";
static const char sql_idmap_reset_ignored[] = "UPDATE [idmap] SET ignore=0 WHERE ignore=1";
static const char sql_idmap_get_ignored[] = "SELECT media_id, filepath FROM [idmap] WHERE ignore=1";
static const char sql_idmap_set_dirty[] = "UPDATE [idmap] SET dirty=? WHERE media_id=?";
static const char sql_idmap_add_dirty[] = "UPDATE [idmap] SET dirty=dirty|? WHERE media_id=?";
static const char sql_idmap_get_dirty[] = "SELECT media_id, dirty FROM [idmap] WHERE dirty&?";
static const char sql_idmap_get_filepath[] = "SELECT filepath FROM [idmap] WHERE media_id=?";
static const char sql_idmap_set_properties[] = "UPDATE [idmap] SET playcount=?, lastplayed=?, lastupdated=?, filetime=?, filesize=?, bitrate=?, duration=? WHERE media_id=?";
static const char sql_idmap_get_properties[] = "SELECT playcount, lastplayed, lastupdated, filetime, filesize, bitrate, duration FROM [idmap] WHERE media_id=?";
static const char sql_idmap_set_played_properties[] = "UPDATE [idmap] SET playcount=?, lastplayed=? WHERE media_id=?";
static const char sql_idmap_get_played_properties[] = "SELECT playcount, lastplayed FROM [idmap] WHERE media_id=?";
static const char sql_idmap_get_sync_filepaths[] = "SELECT filepath FROM (SELECT device_id, mediahash, filepath FROM [idmap] WHERE ignore=0 and device_id=?) WHERE mediahash NOT IN (SELECT mediahash FROM [idmap] WHERE ignore=0 AND device_id=?)";
static const char sql_idmap_set_mime[] = "UPDATE [idmap] SET mime_id=? WHERE media_id=?";
static const char sql_idmap_get_devicesizesum[] = "SELECT SUM(filesize) FROM [idmap] WHERE device_id=? AND ignore=0";
static const char sql_idmap_get_device_cloud_files[] = "SELECT media_id, filepath FROM (SELECT media_id, device_id, mediahash, filepath FROM idmap WHERE ignore=0 AND device_id = ?) WHERE mediahash IN (SELECT mediahash FROM idmap WHERE ignore=0 AND (device_id IN (SELECT device_id FROM [devices] WHERE device='hss' OR device='dropbox' ORDER BY device_id ASC)))";
static const char sql_idmap_get_device_name_from_filepath[] = "SELECT friendly_name FROM [devices] WHERE device_id IN (SELECT DISTINCT device_id FROM [idmap] WHERE ignore=0 AND mediahash IN (SELECT mediahash FROM [idmap] WHERE filepath=? AND ignore=0))";
static const char sql_idmap_get_device_name_from_metahash[] = "SELECT friendly_name FROM [devices] WHERE device_id IN (SELECT DISTINCT device_id FROM [idmap] WHERE metahash=? AND ignore=0)";
static const char sql_mime_types_add_mime[] = "INSERT INTO [mime_types] (mime_type) VALUES (?)";
static const char sql_mime_types_get_mime_id[] = "SELECT mime_id FROM [mime_types] WHERE mime_type=?";
static const char sql_mime_types_set_playable[] = "UPDATE [mime_types] SET playable=?, streamable=? WHERE mime_type=?";
static const char sql_mime_types_add_playable[] = "INSERT INTO [mime_types] (mime_type, playable, streamable) VALUES (?, ?, ?)";
static const char sql_idmap_get_mime[] = "SELECT mime_type FROM [idmap] inner join [mime_types] on [idmap].mime_id = [mime_types].mime_id WHERE media_id=?";
static const char sql_idmap_get_idhash[] = "SELECT idhash FROM [idmap] WHERE media_id=?";
static const char sql_idmap_set_idhash[] = "UPDATE [idmap] SET idhash=? WHERE media_id=?";

int Cloud_DBConnection::IDMap_Generate(int *internal_id, int device_id, nx_uri_t filename)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_generate, SQLPARAM(sql_idmap_generate), device_id, filename);
	AutoResetStatement auto_reset(statement_idmap_generate);
	if (sqlite_ret == SQLITE_DONE)
	{
		*internal_id = (int)sqlite3_last_insert_rowid(database_connection);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Associate(int *internal_id, int cloud_id, int device_id, nx_uri_t filename)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_find, SQLPARAM(sql_idmap_find), cloud_id);
	AutoResetStatement auto_reset(statement_idmap_find);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_find, internal_id);
		return NErr_Success;
	}

	sqlite_ret=Step(statement_idmap_associate, SQLPARAM(sql_idmap_associate), cloud_id, device_id, filename);
	AutoResetStatement auto_reset2(statement_idmap_associate);
	if (sqlite_ret == SQLITE_DONE)
	{
		*internal_id = (int)sqlite3_last_insert_rowid(database_connection);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_Find(int64_t cloud_id, int *internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_find, SQLPARAM(sql_idmap_find), cloud_id);
	AutoResetStatement auto_reset(statement_idmap_find);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_find, internal_id);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Set(int internal_id, int64_t cloud_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set, SQLPARAM(sql_idmap_set), cloud_id, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Next(int64_t *cloud_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_next, SQLPARAM(sql_idmap_next));
	AutoResetStatement auto_reset(statement_idmap_next);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_next, cloud_id);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_Get(int internal_id, int64_t *cloud_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get, SQLPARAM(sql_idmap_get), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (sqlite3_column_type(statement_idmap_get, 0) == SQLITE_NULL)
			return NErr_Empty;
		Columns(statement_idmap_get, cloud_id);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_Remove(int internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_remove, SQLPARAM(sql_idmap_remove), internal_id);
	AutoResetStatement auto_reset(statement_idmap_remove);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_Delete(int internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_delete, SQLPARAM(sql_idmap_delete), internal_id);
	AutoResetStatement auto_reset(statement_idmap_delete);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Removed(int internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_removed, SQLPARAM(sql_idmap_removed), internal_id);
	AutoResetStatement auto_reset(statement_idmap_removed);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetMetaHash(int internal_id, nx_string_t *metahash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_metahash, SQLPARAM(sql_idmap_get_metahash), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_metahash);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (sqlite3_column_type(statement_idmap_get_metahash, 0) == SQLITE_NULL)
			return NErr_Empty;
		Columns(statement_idmap_get_metahash, metahash);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetMediaHash(int internal_id, nx_string_t *mediahash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_mediahash, SQLPARAM(sql_idmap_get_mediahash), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_mediahash);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (sqlite3_column_type(statement_idmap_get_mediahash, 0) == SQLITE_NULL)
			return NErr_Empty;
		Columns(statement_idmap_get_mediahash, mediahash);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_SetMediaHash(int internal_id, nx_string_t mediahash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_mediahash, SQLPARAM(sql_idmap_set_mediahash), mediahash, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_mediahash);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_SetMetaHash(int internal_id, nx_string_t metahash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_metahash, SQLPARAM(sql_idmap_set_metahash), metahash, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_metahash);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_SetAlbumHash(int internal_id, nx_string_t albumhash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_albumhash, SQLPARAM(sql_idmap_set_albumhash), albumhash, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_albumhash);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_SetIgnore(int internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_ignore, SQLPARAM(sql_idmap_set_ignore), internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_ignore);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_ResetIgnored(int internal_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_reset_ignored_one, SQLPARAM(sql_idmap_reset_ignored_one), internal_id);
	AutoResetStatement auto_reset(statement_idmap_reset_ignored_one);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_ResetIgnored()
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_reset_ignored, SQLPARAM(sql_idmap_reset_ignored));
	AutoResetStatement auto_reset(statement_idmap_reset_ignored);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Get_Unannounced(int *internal_id, ns_error_t first)
{
	int sqlite_ret;

	if (first == NErr_True)
		sqlite_ret=Step(statement_idmap_get_unannounced, SQLPARAM(sql_idmap_get_unannounced));
	else
		sqlite_ret = sqlite3_step(statement_idmap_get_unannounced);

	if (sqlite_ret == SQLITE_ROW)
	{
		sqlite3_column_any(statement_idmap_get_unannounced, 0, internal_id);
		return NErr_Success;
	}
	else
	{
		sqlite3_reset(statement_idmap_get_unannounced);
		return NErr_EndOfEnumeration;
	}
}

int Cloud_DBConnection::IDMap_Get_To_Remove(int **out_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_to_remove, SQLPARAM(sql_idmap_get_to_remove));
	AutoResetStatement auto_reset(statement_idmap_get_to_remove);
	Vector<int, 32, 2> ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_to_remove, 0));
		sqlite_ret=sqlite3_step(statement_idmap_get_to_remove);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_MediaHash_Null(int **out_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_mediahash_null, SQLPARAM(sql_idmap_get_mediahash_null), attributes.device_id);
	AutoResetStatement auto_reset(statement_idmap_get_mediahash_null);
	Vector<int, 32, 2> ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_mediahash_null, 0));
		sqlite_ret=sqlite3_step(statement_idmap_get_mediahash_null);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_Devices_Token_From_MediaHash(nx_string_t mediahash, int **out_device_ids, nx_string_t **out_tokens, size_t *num_device_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_devices_token_from_mediahash, SQLPARAM(sql_idmap_get_devices_tokens_from_mediahash), mediahash);
	AutoResetStatement auto_reset(statement_idmap_get_devices_token_from_mediahash);
	Vector<int, 32, 2> ids, media_ids;
	Vector<nx_string_t, 32, 2> tokens;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_devices_token_from_mediahash, 0));

		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_devices_token_from_mediahash, 1, &value);
		tokens.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_devices_token_from_mediahash);
	}

	*out_device_ids = (int *)malloc(sizeof(int) * ids.size());
	*out_tokens = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	for (size_t i = 0; i < ids.size(); i++)
	{
		(*out_device_ids)[i] = ids[i];
		(*out_tokens)[i] = tokens[i];
	}

	*num_device_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_Devices_From_MediaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_devices_from_mediahash, SQLPARAM(sql_idmap_get_devices_from_mediahash), mediahash);
	AutoResetStatement auto_reset(statement_idmap_get_devices_from_mediahash);
	Vector<int, 32, 2> ids, media_ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_devices_from_mediahash, 0));
		if (out_internal_ids) media_ids.push_back(sqlite3_column_int(statement_idmap_get_devices_from_mediahash, 1));
		sqlite_ret=sqlite3_step(statement_idmap_get_devices_from_mediahash);
	}

	*out_device_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i = 0; i < ids.size(); i++)
	{
		(*out_device_ids)[i] = ids[i];
	}

	if (out_internal_ids)
	{
		*out_internal_ids = (int *)malloc(sizeof(int) * media_ids.size());
		for (size_t i = 0; i < media_ids.size(); i++)
		{
			(*out_internal_ids)[i] = media_ids[i];
		}
	}

	*num_device_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_Devices_From_MetaHash(nx_string_t metahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_devices_from_metahash, SQLPARAM(sql_idmap_get_devices_from_metahash), metahash);
	AutoResetStatement auto_reset(statement_idmap_get_devices_from_metahash);
	Vector<int, 32, 2> ids, media_ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_devices_from_metahash, 0));
		sqlite_ret=sqlite3_step(statement_idmap_get_devices_from_metahash);
	}

	*out_device_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i = 0; i < ids.size(); i++)
	{
		(*out_device_ids)[i] = ids[i];
	}

	if (out_internal_ids)
	{
		*out_internal_ids = (int *)malloc(sizeof(int) * media_ids.size());
		for (size_t i = 0; i < media_ids.size(); i++)
		{
			(*out_internal_ids)[i] = media_ids[i];
		}
	}

	*num_device_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_IDs_From_MetaHash(nx_string_t metahash, int64_t **out_ids, int **out_device_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_ids_from_metahash, SQLPARAM(sql_idmap_get_ids_from_metahash), metahash);
	AutoResetStatement auto_reset(statement_idmap_get_ids_from_metahash);
	Vector<int64_t, 32, 2> ids;
	Vector<int, 32, 2> devices;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_ids_from_metahash, 0));
		devices.push_back(sqlite3_column_int(statement_idmap_get_ids_from_metahash, 1));
		sqlite_ret=sqlite3_step(statement_idmap_get_ids_from_metahash);
	}

	*out_ids = (int64_t *)malloc(sizeof(int64_t) * ids.size());
	if (out_device_ids) *out_device_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
		if (out_device_ids) (*out_device_ids)[i] = devices[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_Ignored(int **out_ids, nx_string_t **out_filenames, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_ignored, SQLPARAM(sql_idmap_get_ignored));
	AutoResetStatement auto_reset(statement_idmap_get_ignored);
	Vector<int, 32, 2> ids;
	Vector<nx_string_t, 32, 2> filenames;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_ignored, 0));

		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_ignored, 1, &value);
		filenames.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_ignored);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	*out_filenames = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
		(*out_filenames)[i] = filenames[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::IDMap_SetDirty(int internal_id, int dirty)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_dirty, SQLPARAM(sql_idmap_set_dirty), dirty, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_dirty);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_AddDirty(int internal_id, int dirty)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_add_dirty, SQLPARAM(sql_idmap_add_dirty), dirty, internal_id);
	AutoResetStatement auto_reset(statement_idmap_add_dirty);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_Get_Dirty(int dirty_flag, int **out_ids, int **out_dirties, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_dirty, SQLPARAM(sql_idmap_get_dirty), dirty_flag);
	AutoResetStatement auto_reset(statement_idmap_get_dirty);
	Vector<int, 32, 2> ids;
	Vector<int, 32, 2> dirties;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_dirty, 0));
		dirties.push_back(sqlite3_column_int(statement_idmap_get_dirty, 1));
		sqlite_ret=sqlite3_step(statement_idmap_get_dirty);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	*out_dirties = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
		(*out_dirties)[i] = dirties[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_Get_Filepath(int internal_id, nx_uri_t *filepath)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_filepath, SQLPARAM(sql_idmap_get_filepath), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_filepath);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (sqlite3_column_type(statement_idmap_get_filepath, 0) == SQLITE_NULL)
			return NErr_Empty;
		Columns(statement_idmap_get_filepath, filepath);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_SetProperties(int media_id, int64_t playcount, int64_t lastplayed, int64_t lastupdated,
													int64_t filetime, int64_t filesize, int64_t bitrate, double duration)
{
	int sqlite_ret;

	sqlite_ret=PrepareStatement(statement_idmap_set_properties, SQLPARAM(sql_idmap_set_properties));
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 1, playcount);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	if (lastplayed)
		sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 2, lastplayed);
	else
		sqlite_ret = sqlite3_bind_null(statement_idmap_set_properties, 2);

	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 3, lastupdated);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 4, filetime);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 5, filesize);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 6, bitrate);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 7, duration);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_properties, 8, media_id);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	AutoResetStatement auto_reset(statement_idmap_set_properties);
	sqlite_ret = sqlite3_step(statement_idmap_set_properties);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetProperties(int media_id, int64_t *playcount, int64_t *lastplayed, int64_t *lastupdated,
													int64_t *filetime, int64_t *filesize, int64_t *bitrate, double *duration)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_properties, SQLPARAM(sql_idmap_get_properties), media_id);
	AutoResetStatement auto_reset(statement_idmap_get_properties);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (playcount) sqlite3_column_any(statement_idmap_get_properties, 0, playcount);
		if (lastplayed) sqlite3_column_any(statement_idmap_get_properties, 1, lastplayed);
		if (lastupdated) sqlite3_column_any(statement_idmap_get_properties, 2, lastupdated);
		if (filetime) sqlite3_column_any(statement_idmap_get_properties, 3, filetime);
		if (filesize) sqlite3_column_any(statement_idmap_get_properties, 4, filesize);
		if (bitrate) sqlite3_column_any(statement_idmap_get_properties, 5, bitrate);
		if (duration) sqlite3_column_any(statement_idmap_get_properties, 6, duration);
		sqlite3_reset(statement_idmap_get_properties);

		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_SetPlayedProperties(int media_id, int64_t playcount, int64_t lastplayed)
{
	int sqlite_ret;

	sqlite_ret=PrepareStatement(statement_idmap_set_played_properties, SQLPARAM(sql_idmap_set_played_properties));
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite_ret=sqlite3_bind_any(statement_idmap_set_played_properties, 1, playcount);
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	if (lastplayed)
		sqlite_ret=sqlite3_bind_any(statement_idmap_set_played_properties, 2, lastplayed);
	else
		sqlite_ret = sqlite3_bind_null(statement_idmap_set_played_properties, 2);

	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	AutoResetStatement auto_reset(statement_idmap_set_played_properties);
	sqlite_ret = sqlite3_step(statement_idmap_set_played_properties);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetPlayedProperties(int media_id, int64_t *playcount, int64_t *lastplayed)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_played_properties, SQLPARAM(sql_idmap_get_played_properties), media_id);
	AutoResetStatement auto_reset(statement_idmap_get_played_properties);
	if (sqlite_ret == SQLITE_ROW)
	{
		if (playcount) sqlite3_column_any(statement_idmap_get_played_properties, 0, playcount);
		if (lastplayed) sqlite3_column_any(statement_idmap_get_played_properties, 1, lastplayed);
		sqlite3_reset(statement_idmap_get_played_properties);

		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetSyncFilePaths(int source_device_id, int dest_device_id, nx_string_t **out_filepaths, size_t *num_filepaths)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_sync_filepaths, SQLPARAM(sql_idmap_get_sync_filepaths), source_device_id, dest_device_id);
	AutoResetStatement auto_reset(statement_idmap_get_sync_filepaths);

	Vector<nx_string_t, 32, 2> filepaths;
	while (sqlite_ret == SQLITE_ROW)
	{
		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_sync_filepaths, 0, &value);
		filepaths.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_sync_filepaths);
	}

	*out_filepaths = (nx_string_t *)malloc(sizeof(nx_string_t *) * filepaths.size());
	for (size_t i = 0; i < filepaths.size(); i++)
	{
		(*out_filepaths)[i] = filepaths[i];
	}

	*num_filepaths = filepaths.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_GetDeviceSizeSum(int device_id, int64_t *device_size)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_devicesizesum, SQLPARAM(sql_idmap_get_devicesizesum), device_id);
	AutoResetStatement auto_reset(statement_idmap_get_devicesizesum);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_get_devicesizesum, device_size);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetDeviceCloudFiles(int device_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_device_cloud_files, SQLPARAM(sql_idmap_get_device_cloud_files), device_id);

	Vector<int, 32, 2> ids;
	Vector<nx_string_t, 32, 2> filenames;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_idmap_get_device_cloud_files, 0));

		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_device_cloud_files, 1, &value);
		filenames.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_device_cloud_files);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	*out_filenames = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
		(*out_filenames)[i] = filenames[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_GetDeviceNameFromFilepath(nx_uri_t filepath, nx_string_t **out_devicenames, size_t *num_names)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_device_name_from_metahash, SQLPARAM(sql_idmap_get_device_name_from_filepath), filepath);

	Vector<nx_string_t, 32, 2> devicenames;
	while (sqlite_ret == SQLITE_ROW)
	{
		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_device_name_from_metahash, 0, &value);
		devicenames.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_device_name_from_metahash);
	}

	*out_devicenames = (nx_string_t *)malloc(sizeof(nx_string_t) * devicenames.size());
	for (size_t i=0;i<devicenames.size();i++)
	{
		(*out_devicenames)[i] = devicenames[i];
	}

	*num_names = devicenames.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_GetDeviceNameFromMetahash(nx_string_t metahash, nx_string_t **out_devicenames, size_t *num_names)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_device_name_from_filepath, SQLPARAM(sql_idmap_get_device_name_from_metahash), metahash);

	Vector<nx_string_t, 32, 2> devicenames;
	while (sqlite_ret == SQLITE_ROW)
	{
		nx_string_t value;
		sqlite3_column_any(statement_idmap_get_device_name_from_filepath, 0, &value);
		devicenames.push_back(value);

		sqlite_ret=sqlite3_step(statement_idmap_get_device_name_from_filepath);
	}

	*out_devicenames = (nx_string_t *)malloc(sizeof(nx_string_t) * devicenames.size());
	for (size_t i=0;i<devicenames.size();i++)
	{
		(*out_devicenames)[i] = devicenames[i];
	}

	*num_names = devicenames.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_GetMIME(int internal_id, nx_string_t *mime_type)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_mime, SQLPARAM(sql_idmap_get_mime), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_mime);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_get_mime, mime_type);
		if (*mime_type)
			return NErr_Success;
		else
			return NErr_Empty;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_SetMIME(int internal_id, nx_string_t mime_type)
{
	int sqlite_ret;
	int64_t mime_id;
	sqlite_ret=Step(statement_mime_types_add_mime, SQLPARAM(sql_mime_types_add_mime), mime_type);
	AutoResetStatement auto_reset(statement_mime_types_add_mime);
	if (sqlite_ret == SQLITE_DONE)
	{
		mime_id = sqlite3_last_insert_rowid(database_connection);
	}
	else if (sqlite_ret == SQLITE_CONSTRAINT)
	{
		sqlite_ret=Step(statement_mime_types_get_mime_id, SQLPARAM(sql_mime_types_get_mime_id), mime_type);
		AutoResetStatement auto_reset2(statement_mime_types_get_mime_id);
		if (sqlite_ret == SQLITE_ROW)
		{
			Columns(statement_mime_types_get_mime_id, &mime_id);
		}
		else
		{
			return NErr_Error;
		}
	}
	else if (sqlite_ret != SQLITE_DONE)
		return NErr_Error;

	sqlite_ret=Step(statement_idmap_set_mime, SQLPARAM(sql_idmap_set_mime), mime_id, internal_id);
	AutoResetStatement auto_reset3(statement_idmap_set_mime);
	if (sqlite_ret == SQLITE_DONE)
	{
		return NErr_Success;
	}
	else
	{
		return NErr_Error;
	}
}

int Cloud_DBConnection::CloudDB_MIME_SetPlayable(nx_string_t mime_type, int playable, int streamable)
{
	int sqlite_ret;
	sqlite_ret=Step(statement_mime_types_add_playable, SQLPARAM(sql_mime_types_add_playable), mime_type, playable, streamable);
	AutoResetStatement auto_reset(statement_mime_types_add_playable);
	if (sqlite_ret == SQLITE_DONE)
	{
		return NErr_Success;
	}
	else if (sqlite_ret == SQLITE_CONSTRAINT)
	{
		sqlite_ret=Step(statement_mime_types_set_playable, SQLPARAM(sql_mime_types_set_playable), playable, streamable, mime_type);
		AutoResetStatement auto_reset2(statement_mime_types_set_playable);
		if (sqlite_ret == SQLITE_DONE)
		{
			return NErr_Success;
		}
		else
		{
			return NErr_Error;
		}
	}
	
	return NErr_Error;
}

int Cloud_DBConnection::IDMap_SetTitle(int internal_id, nx_string_t title)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_title, SQLPARAM(sql_idmap_set_title), title, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_title);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetTitle(int internal_id, nx_string_t *title)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_title, SQLPARAM(sql_idmap_get_title), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_title);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_get_title, title);
		if (*title)
			return NErr_Success;
		else
			return NErr_Empty;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_GetIDHash(int internal_id, nx_string_t *idhash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_get_idhash, SQLPARAM(sql_idmap_get_idhash), internal_id);
	AutoResetStatement auto_reset(statement_idmap_get_idhash);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_idmap_get_idhash, idhash);
		if (*idhash)
			return NErr_Success;
		else
			return NErr_Empty;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::IDMap_SetIDHash(int internal_id, nx_string_t idhash)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_idmap_set_idhash, SQLPARAM(sql_idmap_set_idhash), idhash, internal_id);
	AutoResetStatement auto_reset(statement_idmap_set_idhash);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

// TODO: we could make this take an enum and have an array of Cloud_Statements
int Cloud_DBConnection::CloudDB_IDMap_GetString(int internal_id, const char *column, nx_string_t *value)
{
	int sqlite_ret;
	Cloud_Statement getter;
	char query[256];
	int len = sprintf(query, "SELECT %s FROM [idmap] WHERE media_id=?", column);
	sqlite_ret=Step(getter, query, len+1, internal_id);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(getter, value);
		if (*value)
			return NErr_Success;
		else
			return NErr_Empty;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_IDMap_GetInteger(int internal_id, const char *column, int64_t *value)
{
	int sqlite_ret;
	Cloud_Statement getter;
	char query[256];
	int len = sprintf(query, "SELECT %s FROM [idmap] WHERE media_id=?", column);
	sqlite_ret=Step(getter, query, len+1, internal_id);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(getter, value);
		if (*value)
			return NErr_Success;
		else
			return NErr_Empty;
	}
	else
		return NErr_Error;
}