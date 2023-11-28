#include "main.h"
#include "CloudDevice.h"
#include "../Winamp/wa_ipc.h"
#include "AutoCloudURL.h"
#include <strsafe.h>

int revision=8;

static const char *json_upload_albumart_template = 
"{\"version\": 1,"
" \"command\": {"
    "\"type\": \"media-attach-art\","
    "\"username\": \"demo-may1\","
    "\"revision\": %u,"
    "\"auth-token\": \"LOL\","
    "\"device-id\": \"ole-reference-protocol\","
    "\"idhash\": \"%s\","
    "\"content-hash\": \"%s\""
"}}";

static const char *json_edit_metadata = 
"{\"version\": 1,"
" \"command\": {"
    "\"type\": \"push\","
    "\"username\": \"demo-may1\","
    "\"revision\": 11,"
    "\"auth-token\": \"LOL\","
    "\"device-id\": \"ole-reference-protocol\","
     "\"actions\": ["
       "{\"update\": {"
            "\"idhash\":"
                "["
                "\"%s\","
                "\"%s\""
                "],"
            "\"%s\": [\"%s\", \"%s\"]"
        "}}"
"}}";

CloudDevice::CloudDevice(itemRecordListW &record_list)
{
		songs.Items = record_list.Items;
	songs.Size  = record_list.Size;
	songs.Alloc = record_list.Alloc;
	}

__int64 CloudDevice::getDeviceCapacityAvailable()  // in bytes
{
// TODO: implement

	// TODO: implement
	return 0;
}

__int64 CloudDevice::getDeviceCapacityTotal()
{
	 // in bytes
// TODO: implement
	return 0;
}

void CloudDevice::Eject()
{
	 // if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this
	// TODO: implement
}

void CloudDevice::Close()
{
	// save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this
// TODO: implement
} 

static const char *json_add =
"{\"version\": 1,"
 "\"command\": {"
    "\"type\": \"push\","
    "\"username\": \"demo-may1\","
    "\"revision\": %u,"
    "\"auth-token\": \"LOL\","
    "\"device-id\": \"files-to-sql.sh\","
    "\"actions\": ["
        "{\"add\": {"
            "\"filename\": \"%s\","
						"\"idhash\": \"%s\","
						"\"metahash\": \"%s\","
            "\"mediahash\": \"%s\","
            "\"artist\": \"%s\","
            "\"title\": \"%s\","
            "\"album\": \"%s\","
            "\"filesize\": %u,"
            "\"mimetype\": \"audio/mpeg\","
            "\"type\": 0,"
            "\"playcount\": %u,"
            "\"lastupd\": %u,"
            "\"lastplay\": %u,"
            "\"filetime\": %u"
        "}}"
    "]"
"}}";

static const char *json_upload=
"{\"version\": 1,"
 "\"command\": {"
    "\"type\": \"upload\","
    "\"username\": \"demo-may1\","
    "\"revision\": %u,"
    "\"auth-token\": \"LOL\","
    "\"device-id\": \"ole-reference-protocol\","
    "\"idhash\": \"%s\""
"}}";

static void guid_to_string(char *destination, GUID &guid) 
{
	//{ 0x1b3ca60c, 0xda98, 0x4826, { 0xb4, 0xa9, 0xd7, 0x97, 0x48, 0xa5, 0xfd, 0x73 } };
	//sscanf( source, " { 0x%08x, 0x%04x, 0x%04x, { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x } } ; ",
	
	// {2E9CE2F8-E26D-4629-A3FF-5DF619136B2C}
	sprintf(destination, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
	
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

}

// return 0 for success, -1 for failed or cancelled
int CloudDevice::transferTrackToDevice(const itemRecordW * track, // the track to transfer
																					void * callbackContext, //pass this to the callback
																					void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
																					songid_t * songid, // fill in the songid when you are finished
																					int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																					)
{
	GUID idhash_guid, metahash_guid, mediahash_guid, contenthash_guid;
	char idhash[64], metahash[64], mediahash[64], contenthash[64];
	CoCreateGuid(&idhash_guid);
	CoCreateGuid(&metahash_guid);
	CoCreateGuid(&mediahash_guid);
	CoCreateGuid(&contenthash_guid);
	guid_to_string(idhash, idhash_guid);
	guid_to_string(metahash, metahash_guid);
	guid_to_string(mediahash, mediahash_guid);
	guid_to_string(contenthash, contenthash_guid);

	char json_data[16384];
	StringCbPrintfA(json_data, sizeof(json_data), json_add, revision, "/content/test/a.mp3", idhash, metahash, mediahash, 
		(const char *)AutoChar(track->artist, CP_UTF8),
		(const char *)AutoChar(track->title, CP_UTF8),
		(const char *)AutoChar(track->album, CP_UTF8),
		track->filesize,
		track->playcount,
		track->lastupd,
		track->lastplay,
		track->filetime);

	PostJSON("http://o2d2.office.aol.com:8090/command", json_data, 0);
	revision++;

	StringCbPrintfA(json_data, sizeof(json_data), json_upload, revision, idhash);
	PostFile("http://o2d2.office.aol.com:8090/command", track->filename, json_data, 0);

	StringCbPrintfA(json_data, sizeof(json_data), json_upload_albumart_template, revision, idhash, contenthash);
	

	PostAlbumArt("http://o2d2.office.aol.com:8090/command", track->filename, json_data, 0);
	revision++;
	// TODO: create itemRecordW with all da crazy shit
	return 0;
}
int CloudDevice::trackAddedToTransferQueue(const itemRecordW *track)
{
	// return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
	return 0;
} 

void CloudDevice::trackRemovedFromTransferQueue(const itemRecordW *track)
{
// TODO: implement

} 

// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
__int64 CloudDevice::getTrackSizeOnDevice(const itemRecordW *track)
{
// TODO: implement
	return 0;
} 

void CloudDevice::deleteTrack(songid_t songid)
{
	// physically remove from device. Be sure to remove it from all the playlists!
// TODO: implement
} 

void CloudDevice::commitChanges()
{
	 // optional. Will be called at a good time to save changes
}

int CloudDevice::getPlaylistCount()
{
// TODO: implement
// always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	return 1;
} 

// PlaylistName(0) should return the name of the device.
void CloudDevice::getPlaylistName(int playlistnumber, wchar_t *buf, int len)
{
	if (playlistnumber == 0)
	{
		StringCchCopy(buf, len, L"Winamp Cloud");
	}
}
int CloudDevice::getPlaylistLength(int playlistnumber)
{
	if (playlistnumber == 0)
	{
		return songs.Size;
	}

	return 0;
}

songid_t CloudDevice::getPlaylistTrack(int playlistnumber,int songnum)
{
	if (playlistnumber == 0)
	{
		if (songnum >= songs.Size)
			return 0;

		return (songid_t)&songs.Items[songnum];
	}
	// returns a songid
	// TODO: implement
	return 0;
} 

void CloudDevice::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	// with playlistnumber==0, set the name of the device.
// TODO: implement
} 

void CloudDevice::playlistSwapItems(int playlistnumber, int posA, int posB)
{
	// swap the songs at position posA and posB
// TODO: implement
}

void CloudDevice::sortPlaylist(int playlistnumber, int sortBy)
{
// TODO: implement
}

void CloudDevice::addTrackToPlaylist(int playlistnumber, songid_t songid)
{
	// TODO: implement
	// adds songid to the end of the playlist
}

void CloudDevice::removeTrackFromPlaylist(int playlistnumber, int songnum)
{
	// TODO: implement
	//where songnum is the position of the track in the playlist
} 

void CloudDevice::deletePlaylist(int playlistnumber)
{
// TODO: implement
}

int CloudDevice::newPlaylist(const wchar_t *name)
{
// TODO: implement
// create empty playlist, returns playlistnumber. -1 for failed.
	return -1;
} 

void CloudDevice::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	if (item->artist)
		StringCchCopy(buf, len, item->artist);
	else
		buf[0]=0;
}

void CloudDevice::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
		if (item->album)
		StringCchCopy(buf, len, item->album);
	else
		buf[0]=0;

}
void CloudDevice::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;

		if (item->title)
		StringCchCopy(buf, len, item->title);
	else
		buf[0]=0;

}
int CloudDevice::getTrackTrackNum(songid_t songid)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->track;	
}

int CloudDevice::getTrackDiscNum(songid_t songid)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->disc;
}

void CloudDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
			if (item->genre)
		StringCchCopy(buf, len, item->genre);
	else
		buf[0]=0;
}

int CloudDevice::getTrackYear(songid_t songid)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->year;
}

__int64 CloudDevice::getTrackSize(songid_t songid)
{
	 // in bytes
	const itemRecordW *item = (const itemRecordW *)songid;
	return (__int64)item->filesize;

}
int CloudDevice::getTrackLength(songid_t songid)
{
	// in millisecs
		const itemRecordW *item = (const itemRecordW *)songid;
		return item ->length;
} 

int CloudDevice::getTrackBitrate(songid_t songid)
{
	// in kbps
	const itemRecordW *item = (const itemRecordW *)songid;
	return item ->bitrate;

} 

int CloudDevice::getTrackPlayCount(songid_t songid)
{
	const itemRecordW *item = (const itemRecordW *)songid;
return item->playcount;
}
int CloudDevice::getTrackRating(songid_t songid)
{
	 //0-5
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->rating;
}

__time64_t CloudDevice::getTrackLastPlayed(songid_t songid)
{
	// in unix time format
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->lastplay;

} 

__time64_t CloudDevice::getTrackLastUpdated(songid_t songid)
{
	// in unix time format
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->lastupd;
} 

void CloudDevice::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
				if (item->albumartist)
		StringCchCopy(buf, len, item->albumartist);
	else
		buf[0]=0;
}

void CloudDevice::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
					if (item->publisher)
		StringCchCopy(buf, len, item->publisher);
	else
		buf[0]=0;
}

void CloudDevice::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
	const itemRecordW *item = (const itemRecordW *)songid;
					if (item->composer)
		StringCchCopy(buf, len, item->composer);
	else
		buf[0]=0;
}

int CloudDevice::getTrackType(songid_t songid)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	return item->type; 
}

void CloudDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) 
{
// TODO: implement
//optional
} 

static void setMetadata(itemRecordW *item, const char *field_name, wchar_t *&update_field, const wchar_t *value)
{
	const wchar_t *cloud_id = getRecordExtendedItem(item, L"cloud_id");
	char json_data[4096];
	StringCbPrintfA(json_data, sizeof(json_data), json_edit_metadata, (char *)AutoChar(cloud_id, CP_UTF8), (char *)AutoChar(cloud_id, CP_UTF8),	field_name, (char *)AutoChar(update_field, CP_UTF8), value?(char *)AutoChar(value, CP_UTF8):"");
	PostJSON("http://o2d2.office.aol.com:8090/command", json_data, 0);
	free(update_field);
	if (value)
		update_field = wcsdup(value);
	else
		update_field=0;
}

// feel free to ignore any you don't support
void CloudDevice::setTrackArtist(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "artist", item->artist, value);
}

void CloudDevice::setTrackAlbum(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "album", item->album, value);
}

void CloudDevice::setTrackTitle(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "title", item->title, value);

}
void CloudDevice::setTrackTrackNum(songid_t songid, int value)
{
// TODO: implement

}
void CloudDevice::setTrackDiscNum(songid_t songid, int value)
{
// TODO: implement

}
void CloudDevice::setTrackGenre(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "genre", item->genre, value);


}
void CloudDevice::setTrackYear(songid_t songid, int year)
{
// TODO: implement

}
void CloudDevice::setTrackPlayCount(songid_t songid, int value)
{
// TODO: implement

}
void CloudDevice::setTrackRating(songid_t songid, int value)
{
// TODO: implement

}
void CloudDevice::setTrackLastPlayed(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void CloudDevice::setTrackLastUpdated(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void CloudDevice::setTrackAlbumArtist(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "albumartist", item->albumartist, value);
}
void CloudDevice::setTrackPublisher(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "publisher", item->publisher, value);

}
void CloudDevice::setTrackComposer(songid_t songid, const wchar_t *value)
{
	itemRecordW *item = (itemRecordW *)songid;
	setMetadata(item, "composer", item->composer, value);

}
void CloudDevice::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
// TODO: implement

} //optional

bool CloudDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	wchar_t url[1024];
	if (!enqueue) 
	{ //clear playlist
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
		/*int l=SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_PE_GETINDEXTOTAL);
		while(l>=0) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,--l,IPC_PE_DELETEINDEX);*/
	}

  for (int i=0; i<listLength; i++) 
	{
		
		itemRecordW *item = (itemRecordW *)songidList[i];
		const wchar_t *cloud_id = getRecordExtendedItem(item, L"cloud_id");
		if (cloud_id)
		{
			StringCchPrintf(url,1024,  L"http://o2d2.office.aol.com:8090/uglystream/demo-may1/%S", AutoCloudURL(item->filename+1));
			enqueueFileWithMetaStructW s={url, 0, -1};
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		}
  }

  if (!enqueue) 
	{ //play item startPlaybackAt
    SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
    SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
    SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
  }
  return true;
}

intptr_t CloudDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	switch(param1) 
	{
	case DEVICE_SUPPORTED_METADATA:
		{
			intptr_t supported = SUPPORTS_ARTIST | SUPPORTS_ALBUM | SUPPORTS_TITLE | SUPPORTS_TRACKNUM | SUPPORTS_DISCNUM | SUPPORTS_GENRE | 
				SUPPORTS_YEAR | SUPPORTS_SIZE | SUPPORTS_LENGTH | SUPPORTS_BITRATE | SUPPORTS_LASTUPDATED | SUPPORTS_ALBUMARTIST | 
				SUPPORTS_COMPOSER | SUPPORTS_PUBLISHER;
			return supported;
		}
	}

	// TODO: implement
	return 0;
}

bool CloudDevice::copyToHardDriveSupported()
{
// TODO: implement
return false;
}

__int64 CloudDevice::songSizeOnHardDrive(songid_t song)
{
	// how big a song will be when copied back. Return -1 for not supported.
// TODO: implement
	return 0;

} 

int CloudDevice::copyToHardDrive(songid_t song, // the song to copy
																		wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
																		void * callbackContext, //pass this to the callback
																		void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
																		int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																		)
{
// TODO: implement
// // -1 for failed/not supported. 0 for success.
	return -1;
} 


// art functions
void CloudDevice::setArt(songid_t songid, void *buf, int w, int h)
{
	const itemRecordW *item = (const itemRecordW *)songid;
	//buf is in format ARGB32*

	const wchar_t *cloud_id = getRecordExtendedItem(item, L"cloud_id");
	if (cloud_id)
	{

	}
} 

pmpart_t CloudDevice::getArt(songid_t songid)
{
// TODO: implement
	return 0;
}

void CloudDevice::releaseArt(pmpart_t art)
{
// TODO: implement

}
int CloudDevice::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h)
{
// TODO: implement
	return 0;
}

void CloudDevice::getArtNaturalSize(pmpart_t art, int *w, int *h)
{
// TODO: implement

}
void CloudDevice::setArtNaturalSize(pmpart_t art, int w, int h)
{
// TODO: implement

}
void CloudDevice::getArtData(pmpart_t art, void* data)
{
	// data ARGB32* is at natural size
// TODO: implement
} 

bool CloudDevice::artIsEqual(pmpart_t a, pmpart_t b)
{
// TODO: implement
return false;
}
