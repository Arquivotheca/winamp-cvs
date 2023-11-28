#include "api.h"
#include "main.h"
#include "sqlite/sqlite3.h"
#include "CloudDB.h"
#include "nu/vector.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>
#include "foundation/error.h"
#include "nx/nxsleep.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif
#define SQLPARAM(x) x, sizeof(x)

/* schema and statements for the [attributes] table */
static const char sql_attributes_schema[] = 
"CREATE TABLE IF NOT EXISTS [attributes] ("
"attribute_id INTEGER PRIMARY KEY,"
"attribute_name TEXT)";
static const char sql_attributes_find[] = "SELECT attribute_id FROM [attributes] WHERE attribute_name=?";
static const char sql_attributes_add[] = "INSERT INTO [attributes] (attribute_name) VALUES (?)";

/* schema and statements for the [valuemap] table */
/*static const char sql_valuemap_schema[] =
"CREATE TABLE IF NOT EXISTS [valuemap] ("
"value_id INTEGER PRIMARY KEY,"
"attribute_id INTEGER,"
"data TEXT,"
"CONSTRAINT attribute_data_constraint UNIQUE (attribute_id, data))";
static const char sql_value_add[] = "INSERT INTO [valuemap] (attribute_id, data) VALUES (?, ?)";
static const char sql_value_find[] = "SELECT value_id FROM [valuemap] WHERE attribute_id=? AND data=?";
*/

/* schema and statements for the [idmap] table 
meaning of ignore field
1 - excluded
2 - a local file that has been remotely DELETED.  we put it in this state so we can have the user (eventually) confirm/deny the physical deletion of the file
3 - an entry that has been locally REMOVED.  The server has not yet been told.
4 - an entry file that has been locally DELETED.  The server has not yet been told.
5 - an entry that has been removed from the database

meaning of the dirty field
THESE ARE FLAGS
1 - file has been edited locally and needs to be sent to the server
2 - full update (combined with flag above).  the lack of this flag will only send lastplay and playcount
4 - file has been edited remotely, the application may choose to physically modify the file and/or local non-replicant database
*/

static const char sql_idmap_schema[] =
"CREATE TABLE IF NOT EXISTS [idmap] ("
"media_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"cloud_id INTEGER UNIQUE,"
"mediahash TEXT DEFAULT NULL,"
"device_id INTEGER NOT NULL,"
"filepath TEXT,"
"metahash TEXT,"
"ignore INTEGER DEFAULT 0,"
"dirty INTEGER DEFAULT 0,"
"playcount INTEGER,"
"lastplayed INTEGER ,"
"lastupdated INTEGER,"
"filetime INTEGER,"
"filesize INTEGER,"
"bitrate INTEGER,"
"duration REAL,"
"albumhash TEXT,"
"mime_id INTEGER,"
"title TEXT,"
"idhash TEXT,"
"artist TEXT,"
"album TEXT,"
"trackno INTEGER,"
"albumartist TEXT,"
"bpm INTEGER,"
"category TEXT,"
"comment TEXT,"
"composer TEXT,"
"director TEXT,"
"disc INTEGER,"
"discs INTEGER,"
"genre TEXT,"
"producer TEXT,"
"publisher TEXT,"
"tracks INTEGER,"
"year TEXT,"
"albumgain TEXT,"
"trackgain TEXT,"
"rating INTEGER,"
"added INTEGER,"
"CONSTRAINT no_dupes UNIQUE (device_id, filepath))";

/* schema and statements for the [info] table */
static const char sql_info_schema[] =
"CREATE TABLE IF NOT EXISTS [info] ("
"device TEXT UNIQUE,"
"friendly_name TEXT,"
"schema INTEGER DEFAULT 1,"
"revision INTEGER,"
#if defined(_WIN32) && defined(DEBUG)
"logging INTEGER DEFAULT 1,"
#else
"logging INTEGER DEFAULT 0,"
#endif
"network_use INTEGER DEFAULT 1,"
"revision_id TEXT)";

static const char sql_devices_schema[] =
"CREATE TABLE IF NOT EXISTS [devices] ("
"device_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"device TEXT UNIQUE,"
"friendly_name TEXT,"
"device_size INTEGER,"
"device_used INTEGER,"
"platform TEXT,"
"transient INTEGER DEFAULT 1,"
"availability REAL,"
"device_type TEXT,"
"last_seen INTEGER,"
"device_on INTEGER,"
"local INTEGER DEFAULT 0,"
"lan INTEGER DEFAULT 0,"
"reachable INTEGER DEFAULT 0)";

static const char sql_mime_types_schema[] =
"CREATE TABLE IF NOT EXISTS [mime_types] ("
"mime_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"mime_type TEXT UNIQUE,"
"streamable INTEGER DEFAULT 0,"
"playable INTEGER DEFAULT 0)";

static const char sql_playlists_schema[] =
"CREATE TABLE IF NOT EXISTS [playlists] ("
"playlist_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"uuid TEXT UNIQUE,"
"name TEXT,"
"duration REAL,"
"entries INTEGER,"
"dirty INTEGER DEFAULT 0,"
"created INTEGER,"
"lastupdated INTEGER)";

/* right now, playlist_map is only for WAFA.  Hopefully we can unify everything to use this, but it'll need a lot more functionality first */
static const char sql_playlist_map_schema[] = 
"CREATE TABLE IF NOT EXISTS [playlist_map] ("
"playlist_id INTEGER,"
"play_order INTEGER,"
"metahash TEXT)";

static const char sql_devicemap_list[] = "SELECT media_id FROM [idmap] WHERE device_id=? AND ignore=0";

static const char sql_artwork_schema[] = 
"CREATE TABLE IF NOT EXISTS [artwork] ("
"media_id INTEGER,"
"attribute_id INTEGER,"
"arthash TEXT,"
"source_uri TEXT,"
"filetime INTEGER,"
"last_modified INTEGER,"
"CONSTRAINT no_dupes UNIQUE (media_id, attribute_id))";


static const char sql_reset_all[] = "UPDATE SQLITE_SEQUENCE SET seq=0 WHERE name='idmap'; UPDATE [info] SET revision=0; DELETE FROM [idmap]; "
									"DELETE FROM [mime_types]; DELETE FROM [playlists]; DELETE FROM [artwork]; DROP VIEW [metahashmap]; VACUUM;";
static const char sql_compact[] = "VACUUM;";

static const char sql_begin[] = "BEGIN TRANSACTION";
static const char sql_commit[] = "COMMIT";
static const char sql_rollback[] = "ROLLBACK";

static const char sql_pragma_wal[] = "PRAGMA journal_mode=WAL";
/* ------------------------------------------------ */

Cloud_DBConnection::Cloud_DBConnection()
{
	database_connection=0;
	transaction_iterator=0;
	wal_mode=false;
}

Cloud_DBConnection::~Cloud_DBConnection()
{
	if (database_connection)
		sqlite3_close(database_connection);
}

int Cloud_DBConnection::CreateConnection(Cloud_DBConnection **out_db, nx_uri_t database_path, nx_uri_t settings_path, nx_string_t device_token)
{
	Cloud_DBConnection *db = new (std::nothrow) ReferenceCounted<Cloud_DBConnection>;
	if (!db)
		return NErr_OutOfMemory;

	int ret = db->Initialize(database_path, device_token);
	if (ret != NErr_Success)
	{
		delete db;
		return ret;
	}

	*out_db = db;
	return NErr_Success;
}

void PopulateAttributes(ifc_clouddb *db_connection, Attributes &attributes, nx_string_t device_token);

int Cloud_DBConnection::Initialize(nx_uri_t database_path, nx_string_t device_token)
{
#ifdef _WIN32
	int sqlite_ret=sqlite3_open16((const void *)database_path->string, &database_connection);
#elif defined(__APPLE__)
	char database_path_string[PATH_MAX];
	int sqlite_ret;
	if (false == CFURLGetFileSystemRepresentation(database_path, true, (UInt8 *)database_path_string, 
		sizeof(database_path_string)/sizeof(database_path_string[0])))
		sqlite_ret = SQLITE_CANTOPEN;
	else
		sqlite_ret=sqlite3_open(database_path_string, &database_connection);
#else
	int sqlite_ret=sqlite3_open(database_path->string, &database_connection);
#endif
	if (sqlite_ret != SQLITE_OK)
		return NErr_Error;

	sqlite3_busy_timeout(database_connection, 10000);
#ifndef __ANDROID__
	{ // artificial scope for the prepared statement and auto-reset helper
		Cloud_Statement pragma_wal;
		sqlite_ret=Step(pragma_wal, SQLPARAM(sql_pragma_wal)); // PRAGMA journal_mode=WAL
		if (sqlite_ret == SQLITE_ROW)
		{
			const char *journal_mode = (const char *)sqlite3_column_text(pragma_wal, 0);
			if (journal_mode && !strcmp(journal_mode, "wal"))
				wal_mode=true;
		}
		if (pragma_wal)
			sqlite3_reset(pragma_wal);
	}
#endif

	PrepareStatement(statement_begin, SQLPARAM(sql_begin));
	PrepareStatement(statement_commit, SQLPARAM(sql_commit));
	PrepareStatement(statement_rollback, SQLPARAM(sql_rollback));										

	
	BeginTransaction();
	//sqlite_ret=sqlite3_exec(database_connection, sql_media_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_attributes_schema, 0, 0, 0);
	//sqlite_ret=sqlite3_exec(database_connection, sql_valuemap_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_idmap_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_info_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_devices_schema, 0, 0, 0);		 
	sqlite_ret=sqlite3_exec(database_connection, sql_mime_types_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_artwork_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_playlists_schema, 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, sql_playlist_map_schema, 0, 0, 0);
#ifdef __ANDROID__
	if (sqlite_ret != SQLITE_OK)
	{
		__android_log_print(ANDROID_LOG_ERROR, "libreplicant", "[CloudDB] Failed to create playlist_map: %s", sqlite3_errmsg(database_connection));
	}
#endif
	
	sqlite_ret=sqlite3_exec(database_connection, "CREATE INDEX IF NOT EXISTS index_idmap_metahash ON [idmap] (metahash)", 0, 0, 0);
	//sqlite_ret=sqlite3_exec(database_connection, "CREATE INDEX IF NOT EXISTS device_media ON [idmap] (device_id)", 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, "CREATE INDEX IF NOT EXISTS device_use ON [idmap] (media_id, mediahash, device_id, ignore)", 0, 0, 0);
	//sqlite_ret=sqlite3_exec(database_connection, "CREATE INDEX IF NOT EXISTS device_filepath ON [idmap] (device_id, filepath)", 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, "CREATE VIEW IF NOT EXISTS metahashmap AS "
		"SELECT "
		"[idmap].metahash as metahash,"
		"[idmap].albumhash as albumhash,"
		"MAX([idmap].artist) AS albumartist,"
		"MAX([idmap].artist) AS artist,"
		"MAX([idmap].title) AS title,"
		"MAX([idmap].album) AS album,"
		"MAX([idmap].trackno) AS trackno,"
		"MAX([idmap].genre) AS genre,"
		"MAX([idmap].year) AS year,"
		"MAX([idmap].duration) AS duration,"

		// doing as Windows specific for moment
		#ifdef _WIN32
		"MAX([idmap].filetime) AS filetime,"
		"MAX([idmap].filesize) AS filesize,"
		"MAX([idmap].bitrate) AS bitrate,"
		"MAX([idmap].disc) AS disc,"
		"MAX([idmap].rating) AS rating,"
		"MAX([idmap].publisher) AS publisher,"
		"MAX([idmap].composer) AS composer,"
		"GROUP_CONCAT(DISTINCT mime_type) as mime,"
		#endif

		"MAX([idmap].lastupdated) AS lastupdated,"
		"MAX([idmap].lastplayed) AS lastplayed,"
		"SUM([idmap].playcount) AS playcount,"
		"MAX([devices].local) AS local,"
		"MAX(1-[devices].local) AS remote,"
		"MIN([devices].transient) AS transient,"
		"MAX([mime_types].playable AND [devices].local)"
		" OR "
		"MAX([info].network_use AND [mime_types].streamable AND [devices].reachable)"
		" OR "
		"MAX([mime_types].streamable AND [devices].lan)"
		" AS playable,"
		"COUNT(idmap.media_id) AS instances "
		"FROM [idmap] "
		"JOIN [mime_types] ON [idmap].mime_id = [mime_types].mime_id "
		"JOIN [devices] ON [idmap].device_id = [devices].device_id "
		"JOIN [info] "
		"WHERE [idmap].ignore <= 1 "
		"GROUP BY [idmap].metahash", 0, 0, 0);
	/*
	sqlite_ret=sqlite3_exec(database_connection, "CREATE TABLE IF NOT EXISTS metahashmap AS SELECT * FROM metahashmap_view", 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, "CREATE TRIGGER IF NOT EXISTS trigger_metahashmap_update AFTER UPDATE ON idmap BEGIN DELETE FROM metahashmap WHERE metahash=old.metahash; INSERT INTO metahashmap SELECT * FROM metahashmap_view WHERE metahash=old.metahash; DELETE FROM metahashmap WHERE metahash=new.metahash; INSERT INTO metahashmap SELECT * FROM metahashmap_view WHERE metahash=new.metahash;	END;", 0, 0, 0);
	sqlite_ret=sqlite3_exec(database_connection, "CREATE TRIGGER IF NOT EXISTS trigger_metahashmap_delete AFTER DELETE ON idmap BEGIN DELETE FROM metahashmap WHERE metahash=new.metahash; INSERT INTO metahashmap SELECT * FROM metahashmap_view WHERE metahash=new.metahash; END;", 0, 0, 0);*/
	sqlite_ret=sqlite3_exec(database_connection, "CREATE TRIGGER IF NOT EXISTS trigger_devices_update_reachable AFTER UPDATE OF transient ON devices BEGIN UPDATE devices SET reachable=1-new.transient where rowid=new.rowid; END;", 0, 0, 0);
	CloudDB_Devices_Add(device_token, 0, 0, &attributes.device_id);
	PopulateAttributes(this, attributes, device_token);

	Commit();

	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_BeginTransaction()
{
	if (transaction_iterator++ == 0)
	{
		int sqlite_ret;
		AutoResetStatement auto_reset(statement_begin);
		sqlite_ret=sqlite3_step(statement_begin);
		if (sqlite_ret != SQLITE_DONE)
		{
#ifdef __ANDROID__
			const char *err = sqlite3_errmsg(database_connection);
		__android_log_print(ANDROID_LOG_WARN, "libreplicant", "[CloudDB] BEGIN TRANSACTION failed! sqlite_ret=%d, msg=%s", sqlite_ret, err); 
#endif
			return NErr_Error;
		}

		return NErr_Success;
	}
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_Commit()
{
	if (transaction_iterator == 0)
	{
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_WARN, "libreplicant", "[CloudDB] Commit w/o Begin"); 
#endif
		return NErr_Error;
	}

	if (--transaction_iterator == 0)
	{
		int sqlite_ret;
		do
		{
			AutoResetStatement auto_reset(statement_commit);
			sqlite_ret=sqlite3_step(statement_commit);
			if (sqlite_ret == SQLITE_ERROR)
			{
				const char *err = sqlite3_errmsg(database_connection);
#ifdef __ANDROID__
				__android_log_print(ANDROID_LOG_WARN, "libreplicant", "[CloudDB] Commit Error: %s", err); 
#endif
				return NErr_Error;
			}
			if (sqlite_ret == SQLITE_BUSY)
			{
				NXSleep(100);
			}
			else if (sqlite_ret != SQLITE_DONE)
			{
				const char *err = sqlite3_errmsg(database_connection);
#ifdef __ANDROID__
				__android_log_print(ANDROID_LOG_WARN, "libreplicant", "[CloudDB] Commit Error: %s", err); 
#endif
				NXSleep(100);
				err=err;
			}
			else
				assert(sqlite_ret == SQLITE_DONE);
		} while (sqlite_ret != SQLITE_DONE);
	}
	return NErr_Success;

}

int Cloud_DBConnection::CloudDB_Reset_All()
{
	int sqlite_ret;

	sqlite_ret=sqlite3_exec(database_connection, sql_reset_all, 0, 0, 0);
	if (sqlite_ret == SQLITE_DONE)
	{
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Compact()
{
	int sqlite_ret;

	sqlite_ret=sqlite3_exec(database_connection, sql_compact, 0, 0, 0);
	if (sqlite_ret == SQLITE_DONE)
	{
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int Cloud_DBConnection::CloudDB_Rollback()
{
	int sqlite_ret;

	if (transaction_iterator == 0)
	{
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_WARN, "libreplicant", "[CloudDB] Rollback w/o Begin"); 
	#endif
		return NErr_Error;
	}

	if (--transaction_iterator == 0)
	{
		AutoResetStatement auto_reset(statement_rollback);
		sqlite_ret=sqlite3_step(statement_rollback);
		sqlite_ret=sqlite_ret;
	}
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_Attribute_Add(const char *attribute_name, int *attribute_id)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_attributes_find, SQLPARAM(sql_attributes_find), attribute_name);
	AutoResetStatement auto_reset(statement_attributes_find);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_attributes_find, attribute_id);
		return NErr_Success;
	}
	else
	{
		sqlite_ret=Step(statement_attributes_add, SQLPARAM(sql_attributes_add), attribute_name);
		AutoResetStatement auto_reset(statement_attributes_add);
		*attribute_id = (int)sqlite3_last_insert_rowid(database_connection);
		return NErr_Success;
	}
}

int Cloud_DBConnection::CloudDB_Media_GetIDs(int device_id, int **out_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_devicemap_list, SQLPARAM(sql_devicemap_list), device_id);

	AutoResetStatement auto_reset(statement_devicemap_list);
	Vector<int, 32, 2> ids;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_devicemap_list, 0));
		sqlite_ret=sqlite3_step(statement_devicemap_list);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::WriterBlocks()
{
	if (wal_mode)
		return NErr_False;
	else
		return NErr_True;
}
