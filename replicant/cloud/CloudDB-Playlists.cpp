#include "CloudDB.h"
#include "main.h"
#include "nu/vector.h"
#define SQLPARAM(x) x, sizeof(x)

static const char sql_playlists_add[] = "INSERT INTO [playlists] (uuid, name, duration, entries, created, lastupdated, dirty) VALUES (?, ?, ?, ?, ?, ?, ?)";
static const char sql_playlists_update[] = "UPDATE [playlists] SET name=?, duration=?, entries=?, lastupdated=?, dirty=? WHERE uuid=?";
static const char sql_playlists_remove[] = "UPDATE [playlists] SET dirty=8 WHERE uuid=?";
static const char sql_playlists_removed[] = "DELETE FROM [playlists] WHERE uuid=?";
static const char sql_playlists_setdirty[] = "UPDATE [playlists] SET dirty=? WHERE uuid=?";
static const char sql_playlists_get[] = "SELECT playlist_id, name, duration, entries, lastupdated, created FROM [playlists] WHERE uuid=? AND dirty < 8";
static const char sql_playlists_get_lastupdate[] = "SELECT lastupdated FROM [playlists] WHERE uuid=? AND dirty < 8";
static const char sql_playlists_find[] = "SELECT playlist_id, dirty FROM [playlists] WHERE uuid=?";
static const char sql_playlists_list[] = "SELECT playlist_id, uuid FROM [playlists] WHERE dirty < 8 ORDER BY playlist_id ASC";
static const char sql_playlists_get_dirty[] = "SELECT playlist_id, uuid, dirty FROM [playlists] WHERE dirty != 0 ORDER BY playlist_id ASC";
static const char sql_playlists_getid[] = "SELECT playlist_id FROM [playlists] WHERE uuid=?";
static const char sql_playlists_setid[] = "UPDATE [playlists] SET playlist_id=? WHERE uuid=?";

static const char sql_playlistmap_get_metahash[] = "SELECT metahash FROM [playlistmap] WHERE playlist_id=? ORDER BY play_order LIMIT 1 OFFSET ?";

/*
dirty = 0 - nothing to do, is up-to-date
dirty = 1 - new local playlist to upload
dirty = 2 - local playlist needs updating
dirty = 4 - remotely added / updated - needs to be pulled down
dirty = 8 - requested to be removed
*/

int Cloud_DBConnection::CloudDB_Playlists_AddUpdate(nx_string_t uuid, nx_string_t name, double duration, int64_t entries,
													int64_t lastupdated, int64_t created, int dirty, int* mode)
{
	int sqlite_ret;

	*mode = 0;
	sqlite_ret=Step(statement_playlists_add, SQLPARAM(sql_playlists_add),
					uuid, name, duration, entries, created, lastupdated, dirty);
	AutoResetStatement auto_reset(statement_playlists_add);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else if (sqlite_ret == SQLITE_CONSTRAINT)
	{
		*mode = 1;
		// if re-adding a previously removed playlist then we can update to get it back in
		return CloudDB_Playlists_Update(uuid, name, duration, entries, lastupdated, dirty);
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_Update(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int dirty)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_update, SQLPARAM(sql_playlists_update), name, duration, entries, lastupdated, dirty, uuid);
	AutoResetStatement auto_reset(statement_playlists_update);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_Remove(nx_string_t uuid)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_remove, SQLPARAM(sql_playlists_remove), uuid);
	AutoResetStatement auto_reset(statement_playlists_remove);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_Removed(nx_string_t uuid)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_removed, SQLPARAM(sql_playlists_removed), uuid);
	AutoResetStatement auto_reset(statement_playlists_removed);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_SetDirty(nx_string_t uuid, int dirty)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_setdirty, SQLPARAM(sql_playlists_setdirty), dirty, uuid);
	AutoResetStatement auto_reset(statement_playlists_setdirty);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_Get(nx_string_t uuid, int64_t *playlist_id, nx_string_t *name,
											  double *duration, int64_t *entries, int64_t *lastupdated, int64_t *created)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_get, SQLPARAM(sql_playlists_get), uuid);
	AutoResetStatement auto_reset(statement_playlists_get);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_playlists_get, playlist_id, name, duration, entries, lastupdated);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_GetLastUpdate(nx_string_t uuid, int64_t *lastupdated)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_get_lastupdate, SQLPARAM(sql_playlists_get_lastupdate), uuid);
	AutoResetStatement auto_reset(statement_playlists_get_lastupdate);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_playlists_get_lastupdate, lastupdated);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_Find(nx_string_t uuid, int64_t *playlist_id, int64_t *dirty)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_find, SQLPARAM(sql_playlists_find), uuid);
	AutoResetStatement auto_reset(statement_playlists_find);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_playlists_find, playlist_id, dirty);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlist_GetIDs(nx_string_t **uuids, int64_t **playlist_ids, size_t *num_playlists)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_list, SQLPARAM(sql_playlists_list));
	AutoResetStatement auto_reset(statement_playlists_list);
	Vector<int64_t, 32, 2> ids;
	Vector<nx_string_t, 32, 2> pl_uuids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_playlists_list, 0));

		nx_string_t value;
		sqlite3_column_any(statement_playlists_list, 1, &value);
		pl_uuids.push_back(value);

		sqlite_ret=sqlite3_step(statement_playlists_list);
	}

	*playlist_ids = (int64_t *)malloc(sizeof(int64_t) * ids.size());
	*uuids = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*playlist_ids)[i] = ids[i];
		(*uuids)[i] = pl_uuids[i];
	}

	*num_playlists = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::Playlists_Get_Dirty(nx_string_t **uuids, int64_t **playlist_ids, int **dirties, size_t *num_playlists)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_get_dirty, SQLPARAM(sql_playlists_get_dirty));
	AutoResetStatement auto_reset(statement_playlists_get_dirty);
	Vector<int64_t, 32, 2> ids;
	Vector<int, 32, 2> dirty;
	Vector<nx_string_t, 32, 2> pl_uuids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_playlists_get_dirty, 0));

		nx_string_t value;
		sqlite3_column_any(statement_playlists_get_dirty, 1, &value);
		pl_uuids.push_back(value);

		dirty.push_back(sqlite3_column_int(statement_playlists_get_dirty, 2));

		sqlite_ret=sqlite3_step(statement_playlists_get_dirty);
	}

	*playlist_ids = (int64_t *)malloc(sizeof(int64_t) * ids.size());
	*uuids = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	*dirties = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*playlist_ids)[i] = ids[i];
		(*uuids)[i] = pl_uuids[i];
		(*dirties)[i] = dirty[i];
	}

	*num_playlists = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_Playlists_GetID(nx_string_t uuid, int64_t *id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_getid, SQLPARAM(sql_playlists_getid), uuid);
	AutoResetStatement auto_reset(statement_playlists_getid);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_playlists_getid, id);
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Playlists_SetID(nx_string_t uuid, int64_t id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_playlists_setid, SQLPARAM(sql_playlists_setid), id, uuid);
	AutoResetStatement auto_reset(statement_playlists_setid);
	if (sqlite_ret == SQLITE_DONE)
		return NErr_Success;
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_PlaylistMap_GetMetahash(nx_string_t uuid, int item, nx_string_t *metahash)
{
	int sqlite_ret;
	int64_t dummy_dirty;
	int64_t playlist_id;

	int ret = CloudDB_Playlists_Find(uuid, &playlist_id, &dummy_dirty);
	if (ret != NErr_Success)
		return ret;

	sqlite_ret=Step(statement_playlistmap_get_metahash, SQLPARAM(sql_playlistmap_get_metahash), playlist_id, item);
	AutoResetStatement auto_reset(statement_playlistmap_get_metahash);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_playlistmap_get_metahash, metahash);
		return NErr_Success;
	}
	else
		return NErr_Error;
}