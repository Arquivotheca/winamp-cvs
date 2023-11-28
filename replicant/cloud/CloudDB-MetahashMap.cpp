#include "CloudDB.h"
#include "nu/vector.h"
#include <stdlib.h>
#include "ItemMetadata.h"

#define SQLPARAM(x) x, sizeof(x)
static const char sql_metahashmap_get_metadata[] = "SELECT * FROM [metahashmap]";

int Cloud_DBConnection::CloudDB_MetahashMap_GetMetadata(MetadataMap *metadata, size_t *num_metahash)
{
	int sqlite_ret, i = 0;

	sqlite_ret=Step(statement_metahashmap_get_metadata, SQLPARAM(sql_metahashmap_get_metadata));

	while (sqlite_ret == SQLITE_ROW)
	{
		if (metadata)
		{
			double valued = 0;
			int64_t valuei = 0;

			ItemMetadata *meta = new ItemMetadata();

			nx_string_t value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 0, &value);
			meta->SetField(MetadataKeys::METAHASH, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 2, &value);
			meta->SetField(MetadataKeys::ALBUM_ARTIST, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 3, &value);
			meta->SetField(MetadataKeys::ARTIST, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 4, &value);
			meta->SetField(MetadataKeys::TITLE, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 5, &value);
			meta->SetField(MetadataKeys::ALBUM, 0, value);
			NXStringRelease(value);

			sqlite3_column_any(statement_metahashmap_get_metadata, 6, &valuei);
			meta->SetInteger(MetadataKeys::TRACK, 0, valuei);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 7, &value);
			meta->SetField(MetadataKeys::GENRE, 0, value);
			NXStringRelease(value);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 8, &valuei);
			meta->SetInteger(MetadataKeys::YEAR, 0, valuei);

			sqlite3_column_any(statement_metahashmap_get_metadata, 9, &valued);
			meta->SetReal(MetadataKeys::LENGTH, 0, valued);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 10, &valuei);
			meta->SetInteger(MetadataKeys::FILE_TIME, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 11, &valuei);
			meta->SetInteger(MetadataKeys::FILE_SIZE, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 12, &valuei);
			meta->SetInteger(MetadataKeys::BITRATE, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 13, &valuei);
			meta->SetInteger(MetadataKeys::DISC, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 14, &valuei);
			meta->SetInteger(MetadataKeys::RATING, 0, valuei);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 15, &value);
			meta->SetField(MetadataKeys::PUBLISHER, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 16, &value);
			meta->SetField(MetadataKeys::COMPOSER, 0, value);
			NXStringRelease(value);

			value = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 17, &value);
			meta->SetField(MetadataKeys::MIME_TYPE, 0, value);
			NXStringRelease(value);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 18, &valuei);
			meta->SetInteger(MetadataKeys::LAST_UPDATE, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 19, &valuei);
			meta->SetInteger(MetadataKeys::LAST_PLAY, 0, valuei);

			valuei = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 20, &valuei);
			meta->SetInteger(MetadataKeys::PLAY_COUNT, 0, valuei);


			// now we determine the cloud icon status for caching as needed...
			int64_t local = 0, remote = 0, transient = 0;
			sqlite3_column_any(statement_metahashmap_get_metadata, 21, &local);
			sqlite3_column_any(statement_metahashmap_get_metadata, 22, &remote);
			sqlite3_column_any(statement_metahashmap_get_metadata, 23, &transient);

			valuei = 4;
			// has a local instance available (whether to show or not show icon)
			if (local)
			{
				// has a remote instance + on always on remote storage (hss)
				if (remote && !transient)
				{
					valuei = 0;
				}
				// has a remote instance but not on always on (other desktop)
				else if (remote && transient)
				{
				}
			}
			// has no local instance available (whether to show partial or full icon)
			else
			{
				// has a remote instance + on always on remote storage (hss)
				if (remote && !transient)
				{
					valuei = 1;
				}
				// has a remote instance but not on always on (other desktop)
				else if (remote && transient)
				{
					valuei = 3;
				}
			}
			meta->SetInteger(MetadataKeys::CLOUD, 0, valuei);
			metadata->push_back(meta);
		}

		sqlite_ret=sqlite3_step(statement_metahashmap_get_metadata);
		i++;
	}

	*num_metahash = i;
	return NErr_Success;
}