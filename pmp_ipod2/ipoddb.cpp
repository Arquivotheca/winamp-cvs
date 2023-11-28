#include "ipoddb.h"
#include <strsafe.h>
#include <shlwapi.h>

#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "sqlite-3_6_23/sqlite3.h"

#include "ipodplaylist.h"

static int pidGeneratorCallback(void* hpid, int numCols, char** data, char** colNames);

// convert windows timestamp to Macintosh timestamp
unsigned long wintime_to_mactime (const __time64_t time)
{
#ifdef IPODDB_PROFILER
	profiler(iPodDB__wintime_to_mactime);
#endif

    return (unsigned long)(time + 2082844800);
}


// default ctor, initialize members
IpodSong::IpodSong() 
{
	rating = '0';

	filename[0]=artist[0]=album[0]=title[0]=genre[0]=albumartist[0]=publisher[0]=composer[0]=comment[0]=0;
	pid=year=track=length=discnum=bitrate=playcount=artwork_status=artwork_cache_id=last_played_time=last_modified_time=media_type=movie_flag=sample_rate=sample_rate2=sample_count=pregap=postgap=gapless_data=track_gapless=released_time=(int)(size=0);
}

// initialize static member
sqlite3* IpodDB::db = NULL;

// opent the db for business
bool IpodDB::init(wchar_t drive)
{
	int rc;

	// open the library db
	wchar_t libraryDBName[] = {drive,L":\\iPod_Control\\iTunes\\iTunes Library.itlp\\Library.itdb"};
	wchar_t locationsSQL[FIELD_LENGTH*2];
	locationsSQL[0] = L'\0';

	StringCchPrintf(locationsSQL, FIELD_LENGTH*2, L"	 ATTACH																	\
														 DATABASE																\
															 \"%c:\\iPod_Control\\iTunes\\iTunes Library.itlp\\Locations.itdb\"	\
														 AS																		\
															 locations", drive);
	char utf8LibraryDBName[FIELD_LENGTH];
	utf8LibraryDBName[0] = 0;

	char utf8AttachLocationsSQL[FIELD_LENGTH*2];
	utf8AttachLocationsSQL[0] = 0;

	StringCchCopyA(utf8LibraryDBName, FIELD_LENGTH, AutoChar((const wchar_t*)libraryDBName, CP_UTF8));
	StringCchCopyA(utf8AttachLocationsSQL, FIELD_LENGTH*2, AutoChar((const wchar_t*)locationsSQL, CP_UTF8));

	if (!db)
	{
		rc = sqlite3_open( utf8LibraryDBName, &db );
		if( rc )
		{
			sqlite3_close( db );
            return false;
		}

		// attach the locations db
		rc = sqlite3_exec(db, utf8AttachLocationsSQL, NULL, NULL, NULL);

		if( rc!=SQLITE_OK )
		{
			sqlite3_close( db );
			return false;
		}
	}

	return true;
}

// close the db
void IpodDB::shutdown()
{
	if (db)
	{
		sqlite3_close( db );
		db = NULL;
	}
}

// the container callback, this grabs all the playlists
int containerCallback(void* vplaylists, int numCols, char** data, char** colNames)
{
	char buff[FIELD_LENGTH] = {0};
	
	// mine out our songs vector, need to push to it
	Playlists* playlists = (Playlists*) vplaylists;

	// create a new song
	IpodPlaylist* playlist = new IpodPlaylist();

	// The master playlist is the hidden playlist
	int isHidden;
	isHidden = 0; 

	// iterate thru the columns and populate the song
	for(int i=0; i<numCols; i++)
	{
		int isPlaylistItemCount = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"playlist_item_count", -1)-2;
		if (isPlaylistItemCount == 0)
		{
			playlist->numItems = atoi(data[i]);
		}

		int isPid = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"container_pid", -1)-2;
		if (isPid == 0)
		{
			playlist->pid = _atoi64(data[i]);
		}

		int isName = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"name", -1)-2;
		if (isName == 0)
		{
			// convert to wide char and populate the title
			SHLocalStrDupW(AutoWideDup(data[i], CP_UTF8), &playlist->name);
		}

		int isDateCreated = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"date_created", -1)-2;
		if (isDateCreated == 0)
		{
			playlist->dateCreated = atoi(data[i]);
		}

		int isDateModified = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"date_modified", -1)-2;
		if (isDateModified == 0)
		{
			playlist->dateModified = atoi(data[i]);
		}

		int isPlaylistHidden = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"is_hidden", -1)-2;
		if (isPlaylistHidden == 0)
		{
			isHidden = atoi(data[i]);
		}
	}

	// if this is the master playlist, then just update the name and pid
	// dont push this to the list of playlists
	if (isHidden)
	{
		playlists->at(0)->numItems = playlist->numItems;
		playlists->at(0)->pid = playlist->pid;
		SHLocalStrDupW(playlist->name, &playlists->at(0)->name);
		playlists->at(0)->dateCreated = playlist->dateCreated;
		playlists->at(0)->dateModified = playlist->dateModified;

		delete playlist;
	}
	else
	{
		// add the playlist to the songs vector
		playlists->push_back(playlist);
	}
	return 0;
}


// populate from the item table
bool IpodDB::populateSongs(IpodPlaylist* playlist, wchar_t drive)
{
	char *zErrMsg = 0;
	int rc;
	
	// get the playlist id
	int64_t pid = playlist->pid;

	const char* sql = "SELECT																	\
					item.pid							as item__pid,							\
					item.artist							as item__artist,						\
					item.title							as item__title,							\
					item.album							as item__album,							\
					genre_map.genre						as genre_map__genre,					\
					item.album_artist					as item__album_artist,					\
					item.composer						as item__composer,						\
					item.total_time_ms					as item__total_time_ms,					\
					item.year							as item__year,							\
					item.track_number					as item__track_number,					\
					item.disc_number					as item__disc_number,					\
					item.artwork_status					as item__artwork_status,				\
					item.artwork_cache_id				as item__artwork_cache_id,				\
					avformat_info.bit_rate				as avformat_info__bit_rate,				\
					locations.base_location.path		as locations__base_location_path,		\
					locations.location.location			as locations__location,					\
					locations.location.file_size		as locations__file_size					\
				FROM																			\
					item,																		\
					avformat_info,																\
					genre_map,																	\
					item_to_container,															\
					locations.location,															\
					locations.base_location														\
				WHERE																			\
					item.pid = avformat_info.item_pid											\
				AND																				\
					item.pid = locations.location.item_pid										\
				AND																				\
					item.genre_id = genre_map.id												\
				AND																				\
					locations.location.base_location_id = locations.base_location.id			\
				AND																				\
					item.pid = item_to_container.item_pid										\
				AND																				\
					item_to_container.container_pid = ?";

	sqlite3_stmt *stmt = NULL;
	const char* tail = NULL;

	rc = sqlite3_prepare_v2(
							db,				/* Database handle */
							sql,			/* SQL statement, UTF-8 encoded */
							-1,				/* Maximum length of zSql in bytes. */
							&stmt,			/* OUT: Statement handle */
							&tail			/* OUT: Pointer to unused portion of zSql */
	);

	// bind the pid
	rc = sqlite3_bind_int64(stmt, 1, pid);

	// loop thru the resultset and populate the playlist
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// create a new song
		IpodSong* song = new IpodSong();
		int numCols = sqlite3_column_count(stmt);

		// iterate thru the columns and populate the song
		for(int i=0; i<numCols; i++)
		{
			const char* colName = sqlite3_column_name(stmt, i);
			const char* data = (const char *) sqlite3_column_text(stmt, i);
			int isPid = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__pid", -1)-2;
			if (isPid == 0)
			{
				song->pid = _atoi64(data);
			}

			int isArtist = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__artist", -1)-2;
			if (isArtist == 0)
			{
				// convert to wide char and populate the artist
				lstrcpyn(song->artist, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isTitle = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__title", -1)-2;
			if (isTitle == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->title, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isAlbum = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__album", -1)-2;
			if (isAlbum == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->album, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isGenre = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"genre_map__genre", -1)-2;
			if (isGenre == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->genre, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isAlbumArtist = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__album_artist", -1)-2;
			if (isAlbumArtist == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->albumartist, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isComposer = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__composer", -1)-2;
			if (isComposer == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->composer, AutoWideDup(data, CP_UTF8), FIELD_LENGTH);
			}

			int isLength = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__total_time_ms", -1)-2;
			if (isLength == 0)
			{
				song->length = atoi(data);
			}

			int isYear = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__year", -1)-2;
			if (isYear == 0)
			{
				song->year = atoi(data);
			}

			int isTrackNumber = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__track_number", -1)-2;
			if (isTrackNumber == 0)
			{
				song->track = atoi(data);
			}

			int isDiscNumber = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__disc_number", -1)-2;
			if (isDiscNumber == 0)
			{
				song->discnum = atoi(data);
			}

			int isArtworkStatus = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__artwork_status", -1)-2;
			if (isArtworkStatus == 0)
			{
				song->artwork_status = atoi(data);
			}

			int isArtworkCacheId = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"item__artwork_cache_id", -1)-2;
			if (isArtworkCacheId == 0)
			{
				song->artwork_cache_id = atoi(data);
			}

			int isBitrate = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"avformat_info__bit_rate", -1)-2;
			if (isBitrate == 0)
			{
				song->bitrate = atoi(data);
			}

			int isBaseLocation = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"locations__base_location_path", -1)-2;
			if (isBaseLocation == 0)
			{
				// convert to wide char and populate the title
				lstrcpyn(song->filename, &drive, FIELD_LENGTH);
				StringCchCat(song->filename, MAX_PATH, L":/");
				StringCchCat(song->filename, MAX_PATH, AutoWideDup(data, CP_UTF8));
			}

			int isFilename = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"locations__location", -1)-2;
			if (isFilename == 0)
			{
				// convert to wide char and populate the title
				StringCchCat(song->filename, MAX_PATH, L"/");
				StringCchCat(song->filename, MAX_PATH, AutoWideDup(data, CP_UTF8));
			}

			int isFilesize = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colName, CP_UTF8), -1, L"locations__file_size", -1)-2;
			if (isFilesize == 0)
			{
				song->size = atoi(data);
			}
		}

		// add the song to the songs vector
		playlist->songs.push_back(song);
	}

	if( rc == SQLITE_OK || rc==SQLITE_DONE )
	{
		rc = sqlite3_clear_bindings(stmt);
		rc = sqlite3_reset(stmt);
	}
	else
	{
		char buff[1000] = {0};
		sprintf(buff, "SQL error: %s\n", zErrMsg);
		return false;
	}

	return true;
}

// populate from the container table
bool IpodDB::populatePlaylists(Playlists* playlists)
{
	char *zErrMsg = 0;
	int rc;
	char* sql = "SELECT													\
					count(item_pid) AS playlist_item_count,				\
					container_pid,										\
					name,												\
					date_created,										\
					date_modified,										\
					is_hidden											\
				FROM													\
					container,											\
					item_to_container									\
				WHERE													\
					container.pid = item_to_container.container_pid		\
				GROUP BY												\
					container.pid";
	
	rc = sqlite3_exec(db, sql, containerCallback, (void *)playlists, &zErrMsg);
	
	if( rc!=SQLITE_OK )
	{
		char buff[1000] = {0};
		sprintf(buff, "SQL error: %s\n", zErrMsg);
		return false;
	}

	return true;
}

int64_t IpodDB::getNextSongPid()
{
	// come up with a better way of doing this,
	// but for now return a 64 bit signed random number
	char *zErrMsg = 0;
	int rc;
	char* sql = "SELECT													\
					random()					AS item_pid				\
					";
	int64_t pid=0;

	rc = sqlite3_exec(db, sql, pidGeneratorCallback, (void *)&pid, &zErrMsg);
	
	if( rc!=SQLITE_OK )
	{
		char buff[1000] = {0};
		sprintf(buff, "SQL error: %s\n", zErrMsg);
		return 0;
	}

	return pid;
}

// the pid Generator Callback, generates a new pid for entry into the item table
static int pidGeneratorCallback(void* hpid, int numCols, char** data, char** colNames)
{
	char buff[FIELD_LENGTH] = {0};
	
	// get the handle to the pid
	int64_t * pid = (int64_t*) hpid;

	// iterate thru the columns and populate the song
	for(int i=0; i<numCols; i++)
	{
		int isPid = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, AutoWideDup(colNames[i], CP_UTF8), -1, L"item_pid", -1)-2;
		if (isPid == 0)
		{
			*pid = _atoi64(data[i]);
			return 1;
		}
	}
	return 0;
}

bool IpodDB::addItem(IpodSong* song)
{
	// come up with a better way of doing this,
	// but for now return a 64 bit signed random number
	char *zErrMsg = 0;
	int rc;
	char* sql = "	INSERT												\
						INTO item										\
						(												\
							pid,										\
							revision_level,								\
							media_kind,									\
							is_song,									\
							is_audio_book,								\
							is_music_video,								\
							is_movie,									\
							is_tv_show,									\
							is_ringtone,								\
							is_voice_memo,								\
							is_rental,									\
							is_itunes_u,								\
							is_podcast,									\
							date_modified,								\
							date_backed_up,								\
							year,										\
							content_rating,								\
							content_rating_level,						\
							is_compilation,								\
							is_user_disabled,							\
							remember_bookmark,							\
							exclude_from_shuffle,						\
							part_of_gapless_album,						\
							artwork_status,								\
							artwork_cache_id,							\
							start_time_ms,								\
							stop_time_ms,								\
							total_time_ms,								\
							total_burn_time_ms,							\
							track_number,								\
							track_count,								\
							disc_number,								\
							disc_count,									\
							bpm,										\
							relative_volume,							\
							eq_preset,									\
							radio_stream_status,						\
							genius_id,									\
							genre_id,									\
							category_id,								\
							album_pid,									\
							artist_pid,									\
							composer_pid,								\
							title,										\
							artist,										\
							album,										\
							album_artist,								\
							composer,									\
							sort_title,									\
							sort_artist,								\
							sort_album,									\
							sort_album_artist,							\
							sort_composer,								\
							title_order,								\
							artist_order,								\
							album_order,								\
							genre_order,								\
							composer_order,								\
							album_artist_order,							\
							album_by_artist_order,						\
							series_name_order,							\
							comment,									\
							grouping,									\
							description,								\
							description_long,							\
							copyright,									\
							track_artist_pid,							\
							physical_order,								\
							has_lyrics,									\
							date_released								\
						)												\
						VALUES											\
						(												\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?,											\
							?											\
						)";

	int64_t pid=0;

	rc = sqlite3_exec(db, sql, pidGeneratorCallback, (void *)&pid, &zErrMsg);
	
	if( rc!=SQLITE_OK )
	{
		char buff[1000] = {0};
		sprintf(buff, "SQL error: %s\n", zErrMsg);
		return 0;
	}

	return pid;
}
