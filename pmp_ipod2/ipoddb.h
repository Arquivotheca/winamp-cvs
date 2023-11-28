#pragma once

#include <windows.h>

#include "../nu/Vector.h"

struct sqlite3;
class IpodPlaylist;
class IpodSong;

typedef Vector<IpodPlaylist*> Playlists;

#define FIELD_LENGTH 1024

class IpodSong 
{
public:
	IpodSong();
	wchar_t filename[MAX_PATH];
	wchar_t artist[FIELD_LENGTH];
	wchar_t album[FIELD_LENGTH];
	wchar_t title[FIELD_LENGTH];
	wchar_t genre[FIELD_LENGTH];
	wchar_t albumartist[FIELD_LENGTH];
	wchar_t publisher[FIELD_LENGTH];
	wchar_t composer[FIELD_LENGTH];
	wchar_t comment[FIELD_LENGTH];
	unsigned char rating;
	int year,track,length,discnum,bitrate,playcount,artwork_status,artwork_cache_id;
	int64_t pid;
	__time64_t last_played_time;
	__time64_t last_modified_time;
	uint32_t released_time;
	uint32_t media_type;
	uint32_t movie_flag;
	uint16_t sample_rate;
	uint16_t sample_rate2;
	uint64_t sample_count;
	uint32_t pregap;
	uint32_t postgap;
	uint32_t gapless_data;
	uint16_t track_gapless;
	__int64 size;
};

class IpodDB
{
public:
	static bool init(wchar_t drive);
	static bool populateSongs(IpodPlaylist *playlist, wchar_t drive);
	static bool populatePlaylists(Playlists *playlists);
	static void shutdown();
	static int64_t getNextSongPid();
	static bool addItem(IpodSong* song);
private:
	IpodDB(){}
	static sqlite3* db;
};
