#include "CloudDB.h"
#include "nu/vector.h"

#define SQLPARAM(x) x, sizeof(x)

static const char sql_artwork_get_uncalculated[] = "SELECT media_id, filepath FROM [idmap] WHERE ignore=0 AND device_id=? AND media_id NOT IN (SELECT media_id FROM artwork WHERE artwork.attribute_id=?)";
static const char sql_artwork_associate[] = 
"INSERT OR REPLACE INTO [artwork] "
"(media_id, attribute_id, arthash, source_uri, filetime, last_modified) "
"VALUES (?, ?, ?, ?, ?, ?)";
static const char sql_artwork_get[] = "SELECT arthash FROM artwork WHERE media_id=?";

int Cloud_DBConnection::CloudDB_Artwork_GetWork(int device_id, int attribute_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_artwork_get_uncalculated, SQLPARAM(sql_artwork_get_uncalculated), device_id, attribute_id);

	Vector<int, 32, 2> ids;
	Vector<nx_string_t, 32, 2> filenames;
	while (sqlite_ret == SQLITE_ROW)
	{
		ids.push_back(sqlite3_column_int(statement_artwork_get_uncalculated, 0));

		if (out_filenames)
		{
			nx_string_t value;
			sqlite3_column_any(statement_artwork_get_uncalculated, 1, &value);
			filenames.push_back(value);
		}

		sqlite_ret=sqlite3_step(statement_artwork_get_uncalculated);
	}

	*out_ids = (int *)malloc(sizeof(int) * ids.size());
	if (out_filenames)
		*out_filenames = (nx_string_t *)malloc(sizeof(nx_string_t) * ids.size());
	for (size_t i=0;i<ids.size();i++)
	{
		(*out_ids)[i] = ids[i];
		if (out_filenames)
			(*out_filenames)[i] = filenames[i];
	}

	*num_ids = ids.size();
	return NErr_Success;
}

int Cloud_DBConnection::Artwork_Associate(int internal_id, int attribute_id, nx_string_t arthash, nx_uri_t source_uri, int64_t filetime, int64_t last_modified)
{
	int sqlite_ret;
	sqlite_ret=Step(statement_artwork_associate, SQLPARAM(sql_artwork_associate), internal_id, attribute_id, arthash, source_uri, filetime, last_modified);
	AutoResetStatement auto_reset(statement_artwork_associate);
	if (sqlite_ret == SQLITE_DONE)
	{
		return NErr_Success;
	}
	return NErr_Error;	
}

int Cloud_DBConnection::Artwork_Get(int internal_id, nx_string_t *arthash)
{
	int sqlite_ret;
	sqlite_ret=Step(statement_artwork_get, SQLPARAM(sql_artwork_get), internal_id);
	AutoResetStatement auto_reset(statement_artwork_get);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_artwork_get, arthash);
		return NErr_Success;
	}
	return NErr_Empty;	
}