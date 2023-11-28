#include "main.h"
#include "api.h"
#include "collect.h"
#include "../nde/nde_c.h"
#include "../nu/AutoLock.h"
#include <shlwapi.h>
/*
we're going to queue results into a database (NDE format)
this lets us scrobble while we're "offline"
as well as gracefully handle retries when the server-side stuff is down
or authentication failed.
*/

/* DB Schema
Filename
Playback start timestamp
Artist
Album
Title
Track Number
Genre
MIME Type
Length of Play
*/

static Nullsoft::Utility::LockGuard dbcs;
static nde_table_t table = 0;
static nde_database_t db = 0;
using namespace Nullsoft::Utility;

enum
{
	DB_ID_FILENAME = 0,
	DB_ID_TIMESTAMP = 1,
	DB_ID_ARTIST = 2,
	DB_ID_ALBUM = 3,
	DB_ID_TITLE = 4,
	DB_ID_TRACK = 5,
	DB_ID_GENRE = 6,
	DB_ID_MIMETYPE = 7,
	DB_ID_PLAYLENGTH = 8,	
};

static bool OpenDatabase()
{
	AutoLock lock(dbcs);
	if (!db)
	{
		db = NDE_CreateDatabase();
	}
	return true;
}

void CloseDatabase()
{
	AutoLock lock(dbcs);
	NDE_Database_CloseTable(db, table);
	NDE_DestroyDatabase(db);
	db=0;
}

static void CreateFields(nde_table_t table)
{
	// create defaults
	NDE_Table_NewColumn(table, DB_ID_FILENAME, "filename", FIELD_FILENAME);
	NDE_Table_NewColumn(table, DB_ID_TIMESTAMP, "timestamp", FIELD_DATETIME);
	NDE_Table_NewColumn(table, DB_ID_ARTIST, "artist", FIELD_STRING);
	NDE_Table_NewColumn(table, DB_ID_ALBUM, "album", FIELD_STRING);
	NDE_Table_NewColumn(table, DB_ID_TITLE, "title", FIELD_STRING);
	NDE_Table_NewColumn(table, DB_ID_TRACK, "track", FIELD_INTEGER);
	NDE_Table_NewColumn(table, DB_ID_GENRE, "genre", FIELD_STRING);
	NDE_Table_NewColumn(table, DB_ID_MIMETYPE, "mimetype", FIELD_STRING);
	NDE_Table_NewColumn(table, DB_ID_PLAYLENGTH, "playlength", FIELD_LENGTH);
	NDE_Table_PostColumns(table);
	NDE_Table_AddIndexByID(table, DB_ID_TIMESTAMP, "timestamp");
}

static bool OpenTable()
{
	AutoLock lock(dbcs);
	if (!OpenDatabase())
		return false;

	if (!table)
	{
		const wchar_t *inidir = WASABI_API_APP->path_getUserSettingsPath();
		wchar_t tablePath[MAX_PATH], indexPath[MAX_PATH];
		PathCombineW(tablePath, inidir, L"plugins");
		PathAppendW(tablePath, L"mudqueue.dat");
		PathCombineW(indexPath, inidir, L"plugins");
		PathAppendW(indexPath, L"mudqueue.idx");
		table = NDE_Database_OpenTable(db, tablePath, indexPath, NDE_OPEN_ALWAYS, NDE_CACHE);
		if (table)
			CreateFields(table);
	}
	return !!table;
}

static void db_add(nde_scanner_t s, unsigned char id, wchar_t *data)
{
	if (data)
	{
		nde_field_t f = NDE_Scanner_NewFieldByID(s, id);
		NDE_StringField_SetNDEString(f, data);
	}
}

static void db_add_int(nde_scanner_t s, unsigned char id, int data)
{
	if (data)
	{
		nde_field_t f = NDE_Scanner_NewFieldByID(s, id);
		NDE_IntegerField_SetValue(f, data);
	}
}

static void db_add_time(nde_scanner_t s, unsigned char id, time_t data)
{
	if (data)
	{
		nde_field_t f = NDE_Scanner_NewFieldByID(s, id);
		NDE_IntegerField_SetValue(f, static_cast<int>(data)); // TODO: make DateTimeField specific function
	}
}

static void db_add_length(nde_scanner_t s, unsigned char id, int data)
{
	if (data)
	{
		nde_field_t f = NDE_Scanner_NewFieldByID(s, id);
		NDE_IntegerField_SetValue(f, data); // TODO: make LengthField specific function
	}
}


bool AddCollectedData(const CollectedData &data)
{
	AutoLock lock(dbcs);
	if (!OpenTable())
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner(table);
	if (s)
	{
		NDE_Scanner_New(s);
		db_add(s, DB_ID_FILENAME, data.filename);
		db_add_time(s, DB_ID_TIMESTAMP, data.timestamp);
		db_add(s, DB_ID_ARTIST, data.artist);
		db_add(s, DB_ID_ALBUM, data.album);
		db_add(s, DB_ID_TITLE, data.title);
		db_add_int(s, DB_ID_TRACK, data.track);
		db_add(s, DB_ID_GENRE, data.genre);
		db_add_length(s, DB_ID_PLAYLENGTH, data.playLength);
		NDE_Scanner_Post(s);
		NDE_Table_DestroyScanner(table, s);
		NDE_Table_Sync(table);
		return true;
	}
	return false;
}

// gets (and removes) the first entry in the database
bool GetCollectedData(CollectedData &data)
{
	AutoLock lock(dbcs);
	if (!OpenTable())
		return false;

	nde_scanner_t s = NDE_Table_CreateScanner(table);
	if (s)
	{
		NDE_Scanner_First(s);
		nde_field_t f = NDE_Scanner_GetFieldByID(s, DB_ID_FILENAME);
		if (f)
		{
			data.filename = NDE_StringField_GetString(f);
			ndestring_retain(data.filename);

			f = NDE_Scanner_GetFieldByID(s, DB_ID_TIMESTAMP);
			if (f)
				data.timestamp = NDE_IntegerField_GetValue(f); // TODO: DateTimeField specific function

			f = NDE_Scanner_GetFieldByID(s, DB_ID_ARTIST);
			if (f)
			{
				data.artist = NDE_StringField_GetString(f);
				ndestring_retain(data.artist);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_ALBUM);
			if (f)
			{
				data.album = NDE_StringField_GetString(f);
				ndestring_retain(data.album);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_TITLE);
			if (f)
			{
				data.title = NDE_StringField_GetString(f);
				ndestring_retain(data.title);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_TRACK);
			if (f)
			{
				data.track = NDE_IntegerField_GetValue(f);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_GENRE);
			if (f)
			{
				data.genre = NDE_StringField_GetString(f);
				ndestring_retain(data.genre);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_MIMETYPE);
			if (f)
			{
				data.mimetype = NDE_StringField_GetString(f);
				ndestring_retain(data.mimetype);
			}

			f = NDE_Scanner_GetFieldByID(s, DB_ID_PLAYLENGTH);
			if (f)
			{
				data.playLength= NDE_IntegerField_GetValue(f); // TODO: LengthField specific function
			}

			NDE_Scanner_Delete(s);
			NDE_Scanner_Post(s);
			NDE_Table_DestroyScanner(table, s);
			NDE_Table_Sync(table);
			return true;
		}

		NDE_Table_DestroyScanner(table, s);
	}
	return false;
}

void CompactDatabase()
{
	AutoLock lock(dbcs);
	if (OpenTable())
	{
		NDE_Table_Compact(table);
	}
}