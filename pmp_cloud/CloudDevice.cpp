#include "api.h"
#include "main.h"
#include "CloudDevice.h"
#include "../Winamp/wa_ipc.h"
#include "nx/nxstring.h"
#include "nswasabi/ReferenceCounted.h"
#include "../nu/AutoURL.h"
#include <api/service/waServiceFactory.h>
#include "../ml_cloud/shared.h"
#include "../gen_ml/itemlist.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>

enum
{
	FIELD_ARTIST,
	FIELD_ALBUM,
	FIELD_TRACKNO,
	FIELD_ALBUMARTIST,
	FIELD_BPM,
	FIELD_CATEGORY,
	FIELD_COMMENT,
	FIELD_COMPOSER,
	FIELD_DIRECTOR,
	FIELD_DISC,
	FIELD_DISCS,
	FIELD_GENRE,
	FIELD_PRODUCER,
	FIELD_PUBLISHER,
	FIELD_TRACKS,
	FIELD_YEAR,
	FIELD_ALBUMGAIN,
	FIELD_TRACKGAIN,
	FIELD_RATING,
	FIELD_TITLE,
};

static const char *field_names[] = {
	"ARTIST",
	"ALBUM",
	"TRACKNO",
	"ALBUMARTIST",
	"BPM",
	"CATEGORY",
	"COMMENT",
	"COMPOSER",
	"DIRECTOR",
	"DISC",
	"DISCS",
	"GENRE",
	"PRODUCER",
	"PUBLISHER",
	"TRACKS",
	"YEAR",
	"ALBUMGAIN",
	"TRACKGAIN",
	"RATING",
	"TITLE",
};

CloudDevice::CloudDevice(nx_string_t device_token, int device_id, SpecialDevice special_device, DevicePlatform platform_type) :
						 special_device(special_device), platform_type(platform_type)
{
	current_edit_internal_id=0;
	device_name = 0;
	transfer_db_connection = 0;
	upload_waiter = 0;

	this->out_ids = 0;
	this->out_filenames = 0;

	this->context = 0;
	this->killswitch = 0;

	db_connection = 0;
	this->device_id = device_id;
	this->device_token = device_token;
	attributes.device_token = NXStringRetain(device_token);
}

CloudDevice::~CloudDevice()
{
	if (db_connection)
		db_connection->Release();

	if (transfer_db_connection)
		transfer_db_connection->Release();

	for (size_t i = 0; i < metadataMap.size(); i++) delete metadataMap[i];
	metadataMap.clear();

	value_cache.clear();
	int_cache.clear();
	ids.Reset();
	for (size_t i=0;i<cloud_files.size();i++)
	{
		NXStringRelease(cloud_files[i]);
	}
	cloud_files.clear();

	if (out_ids)
	{
		free(out_ids);
		out_ids = 0;
	}
	if (out_filenames)
	{
		free(out_filenames);
		out_filenames = 0;
	}

	if (upload_waiter)
		CloseHandle(upload_waiter);

	NXStringRelease(device_name);
}

#define RegisterAttribute(x) db_connection->Attribute_Add(#x, &attributes. ## x)
void CloudDevice::LazyLoad()
{
	if (!db_connection)
	{
		cloud_client->CreateDatabaseConnection(&db_connection);
		db_connection->BeginTransaction();
		RegisterAttribute(artist);
		RegisterAttribute(album);
		RegisterAttribute(trackno);
		RegisterAttribute(albumartist);
		RegisterAttribute(bpm);
		RegisterAttribute(category);
		RegisterAttribute(comment);
		RegisterAttribute(composer);
		RegisterAttribute(director);
		RegisterAttribute(disc);
		RegisterAttribute(discs);
		RegisterAttribute(genre);
		RegisterAttribute(producer);
		RegisterAttribute(publisher);
		RegisterAttribute(tracks);
		RegisterAttribute(year);
		RegisterAttribute(albumgain);
		RegisterAttribute(trackgain);
		RegisterAttribute(rating);
		RegisterAttribute(type);
		RegisterAttribute(lossless);

		db_connection->Devices_GetName(device_id, &device_name, 0);
		db_connection->Commit();
	}
}
#undef RegisterAttribute

__int64 CloudDevice::getDeviceCapacityAvailable()  // in bytes
{
	// in bytes
	if (device_id > 0)
	{
		int64_t total_size = 0, used_size = 0, current_size = 0;
		if (db_connection && db_connection->Devices_GetCapacity(device_id, &total_size, &used_size) == NErr_Success)
		{
			// as we cannot rely on the last sizes for the device once we've been
			// running for a while so attempt to get a more current size now used
			if (db_connection->IDMap_GetDeviceSizeSum(device_id, &current_size) != NErr_Success)
				current_size = used_size;

			// some devices will have capacity used but none available
			// so in those cases we indicate there is no available size
			return (total_size > 0 ? total_size - current_size : 0);
		}
	}
	return 0;
}

__int64 CloudDevice::getDeviceCapacityTotal()
{
	// in bytes
	int64_t total_size = 0, used_size = 0, current_size = 0;
	if (db_connection && db_connection->Devices_GetCapacity(device_id, &total_size, &used_size) == NErr_Success)
	{
		// some devices will have capacity used but none available
		// so if the total is zero then we use the used for things
		if (total_size > 0)
			return total_size;
		else if (used_size > 0)
		{
			// as we cannot rely on the last sizes for the device once we've been
			// running for a while so attempt to get a more current size now used
			if (db_connection->IDMap_GetDeviceSizeSum(device_id, &current_size) == NErr_Success)
				return current_size;
			else
				return used_size;
		}
	}
	return 0;
}

void CloudDevice::Eject()
{
	wchar_t title[64];
	WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_REMOVAL, title, 64);
	if (special_device == DEVICE_HSS || special_device == DEVICE_LOCAL_LIBRARY || special_device == DEVICE_ALL_SOURCES)
		MessageBox(plugin.hwndWinampParent, WASABI_API_LNGSTRINGW(IDS_DEVICE_REMOVE_NOT_SUPPORTED), title, MB_ICONASTERISK);
	else
	{
		if (MessageBox(plugin.hwndWinampParent, WASABI_API_LNGSTRINGW(IDS_DEVICE_REMOVE_PROMPT), title, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			RemoveStruct *remove = new RemoveStruct();
			remove->device_id = device_id;
			remove->device_token = NXStringRetain(device_token);
			cloud_client->DeviceRemove(remove);
		}
	}
	// if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this
	// TODO: implement
}

void CloudDevice::Close()
{
	if (special_device != DEVICE_ALL_SOURCES)
	{
		// save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this
		SendMessage(plugin.hwndPortablesParent, WM_PMP_IPC, (intptr_t)this, PMP_IPC_DEVICEDISCONNECTED);
		cloudDevices.erase(this);
		delete this;
	}
}

// return 0 for success, -1 for failed or cancelled
int CloudDevice::transferTrackToDevice(const itemRecordW * track, // the track to transfer
									   void * callbackContext, //pass this to the callback
									   void (*callbackFunc)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
									   songid_t * songid, // fill in the songid when you are finished
									   int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
									  )
{
	wchar_t msg[256];
	this->context = callbackContext;
	this->callback = callbackFunc;
	this->killswitch = killswitch;

	ReferenceCountedNXString nx_filename;
	ReferenceCountedNXURI filename;

	if (!transfer_db_connection)
	{
		if (cloud_client->CreateDatabaseConnection(&transfer_db_connection) != NErr_Success)
		{
			callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_OPEN_DB, msg, 256));
			return -1;
		}
	}

	NXStringCreateWithUTF16(&nx_filename, track->filename);
	NXURICreateWithNXString(&filename, nx_filename);

	int internal_id = 0, is_ignored = 0;
	transfer_db_connection->Media_FindByFilename(filename, local_device_id, &internal_id, &is_ignored);

	if (internal_id <= 0)
	{
		callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(is_ignored ? IDS_SONG_NOT_COMPATIBLE_SKIPPING : IDS_SONG_NOT_PART_OF_CLOUD_LIBRARY, msg, 256));
		return -1;
	}

	ReferenceCountedNXString media_hash;
	if (transfer_db_connection->IDMap_GetMediaHash(internal_id, &media_hash) != NErr_Success)
	{
		ReferenceCountedNXString meta_hash, media_hash, id_hash;
		if (ComputeMediaHash(track->filename, &media_hash) == NErr_Success)
		{
			MediaHashMetadata metadata(media_hash);
			transfer_db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_LOCAL);			
		}
		else
		{
			callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_SONG_NOT_COMPATIBLE_SKIPPING, msg, 256));
			return -1;
		}
	}

	int64_t cloud_id = 0;
	if (transfer_db_connection->IDMap_Get(internal_id, &cloud_id) != NErr_Success || !cloud_id)
	{
		// force an announcement
		cloud_client->MetadataAnnounce1(transfer_db_connection, internal_id);
	}

	if (!upload_waiter)
		upload_waiter = CreateEvent(0, FALSE, FALSE, 0);	

	cloud_client->Upload(filename, attributes.device_token, internal_id, this);
	WaitForSingleObject(upload_waiter, INFINITE);

	// TODO need to get this to refresh the cache status of the cloud icon

	*songid = 0;
	return 0;
}

int CloudDevice::trackAddedToTransferQueue(const itemRecordW *track)
{
	ReferenceCountedNXString nx_filename;
	ReferenceCountedNXURI filename;

	if (!transfer_db_connection)
	{
		if (cloud_client->CreateDatabaseConnection(&transfer_db_connection) != NErr_Success)
		{
			return 0;
		}
	}

	NXStringCreateWithUTF16(&nx_filename, track->filename);
	NXURICreateWithNXString(&filename, nx_filename);

	int internal_id = 0, is_ignored = 0;
	transfer_db_connection->Media_FindByFilename(filename, local_device_id, &internal_id, &is_ignored);

	// if an invalid internal_id or appears to have been deleted / ignored then attempt to add as new
	if (internal_id <= 0 || is_ignored != 0)
	{
		ItemRecordMetadata metadata(track);
		if (transfer_db_connection->Media_Add(filename, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL, &internal_id) != NErr_Success)
		{
			return 1;
		}
	}

	ReferenceCountedNXString media_hash;
	if (transfer_db_connection->IDMap_GetMediaHash(internal_id, &media_hash) != NErr_Success)
	{
		ReferenceCountedNXString meta_hash, media_hash, id_hash;
		if (ComputeMediaHash(track->filename, &media_hash) == NErr_Success)
		{
			MediaHashMetadata metadata(media_hash);
			transfer_db_connection->Media_Update(internal_id, &metadata, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_LOCAL);						
		}
		else
		{
			// slow processing - will try it again later...
			return 0;
		}
	}
	else
	{
		// check to make sure we've not already got an uploaded copy
		int *out_device_ids = 0;
		size_t num_device_ids = 0;
		transfer_db_connection->IDMap_Get_Devices_From_MediaHash(media_hash, &out_device_ids, &num_device_ids, 0);
		if (num_device_ids > 0)
		{
			for (size_t i = 0; i < num_device_ids; i++)
			{
				if (out_device_ids[i] == this->device_id)
				{
					free(out_device_ids);
					return 2;
				}
			}
		}

		if (out_device_ids) free(out_device_ids);
	}

	// give things a head-start by announcing the file if it has not already been
	int64_t cloud_id = 0;
	if (transfer_db_connection->IDMap_Get(internal_id, &cloud_id) != NErr_Success || !cloud_id)
	{
		// force an announcement
		cloud_client->MetadataAnnounce1(transfer_db_connection, internal_id);
	}

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
	// TODO: implement - used by auto-fill
	return 0;
} 

void CloudDevice::deleteTrack(songid_t songid)
{
	size_t position=ids.size();
	for (size_t i=0;i<position;i++)
	{
		if (ids[i] == songid)
		{
			ids.eraseAt(i);
			break;
		}
	}
	db_connection->BeginTransaction();
	db_connection->IDMap_Remove(songid);
	db_connection->Commit();
#ifdef _DEBUG
	if (cloud_client) cloud_client->Flush();
#endif

	// TODO: Be sure to remove it from all the playlists!	
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
	LazyLoad();
	if (playlistnumber == 0)
	{
		if (special_device == DEVICE_LOCAL_LIBRARY)
			WASABI_API_LNGSTRINGW_BUF(IDS_LOCAL_LIBRARY, buf, len);
		else if (device_name)
			StringCchCopy(buf, len, device_name->string);
		else if (special_device == DEVICE_HSS)
			StringCchCopy(buf, len, L"Winamp Storage");
		else if (special_device == DEVICE_DROPBOX)
			StringCchCopy(buf, len, L"DropBox");
		else
			StringCchCopy(buf, len, attributes.device_token->string);
	}
}

int CloudDevice::getPlaylistLength(int playlistnumber)
{
	LazyLoad();
	if (playlistnumber == 0)
	{
		return UpdateCaches();
	}

	return 0;
}

songid_t CloudDevice::getPlaylistTrack(int playlistnumber, int songnum)
{
	if (playlistnumber == 0)
	{
		if (special_device == DEVICE_ALL_SOURCES)
		{
			return songnum;
		}
		else
		{
			if (songnum >= (int)ids.size()) return 0;
			return (songid_t)ids[songnum];
		}
	}
	// returns a songid
	// TODO: implement
	return 0;
}

// with playlistnumber==0, set the name of the device.
void CloudDevice::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	// need to exclude setting the name for the 'local library' device
	// as we set a specific name for it to make it obvious in the list
	if (special_device != DEVICE_LOCAL_LIBRARY)
	{
		// also no need to do a rename update if it's the same as before
		if (wcscmp(buf, device_name->string))
		{
			LazyLoad();
			if (playlistnumber == 0)
			{
				ReferenceCountedNXString new_name;
				if (NXStringCreateWithUTF16(&new_name, buf) == NErr_Success)
				{
					RenameStruct *rename = new RenameStruct();
					rename->name = NXStringRetain(new_name);
					rename->old_name = NXStringRetain(device_name);
					rename->device = NXStringRetain(device_token);
					cloud_client->DeviceRename(rename);
				}
			}
		}
	}
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
	if (songid != 0) // songid of 0 is a special notation
	{

	}

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
	MessageBox(0, L"Playlist creation not currently implemented.", name, MB_OK | MB_ICONINFORMATION);
	return -1;
}

enum
{
	IDMAP_DURATION=80,
	IDMAP_PLAYCOUNT=81,
	IDMAP_LASTPLAYED=82,
	IDMAP_LASTUPDATED=83,
	IDMAP_FILESIZE=84,
	IDMAP_BITRATE=85,
	IDMAP_MEDIAHASH=86,
	IDMAP_MIMETYPE=88,
};

void CloudDevice::CacheLookupMediaHash(songid_t songid, nx_string_t *media_hash)
{
	int key = songid*100 + IDMAP_MEDIAHASH;
	nx_string_t value = 0;

	ValueCache::iterator found = value_cache.find(key);
	if (found == value_cache.end())
	{
		if (db_connection->IDMap_GetMediaHash(songid, &value) != NErr_Success)
		{
			value = 0;
		}
		value_cache[key] = value;
		*media_hash = NXStringRetain(value);
	}
	else
	{
		value = found->second;
		*media_hash = NXStringRetain(value);
	}
}

void CloudDevice::CacheLookupMIME(songid_t songid, wchar_t *buf, int len)
{
	int key = songid*100 + IDMAP_MIMETYPE;
	nx_string_t value = 0;

	ValueCache::iterator found = value_cache.find(key);
	if (found == value_cache.end())
	{
		if (db_connection->IDMap_GetMIME(songid, &value) != NErr_Success)
		{
			value = 0;
		}
		value_cache[key] = value;
	}
	else
	{
		value = found->second;
	}

	if (!value || NXStringGetBytes(0, value, buf, len*2, nx_charset_utf16le, nx_string_get_bytes_size_null_terminate) != NErr_Success) buf[0] = 0;
}

void CloudDevice::CacheLookup(songid_t songid, int attributeid, wchar_t *buf, int len)
{
	int key = songid*100 + attributeid;

	nx_string_t value=0;
	ValueCache::iterator found = value_cache.find(key);
	if (found == value_cache.end())
	{
		if (db_connection->IDMap_GetString(songid, field_names[attributeid], &value) != NErr_Success)
			value=0;
		value_cache[key] = value;
	}
	else
	{
		value = found->second;
	}

	if (!value || NXStringGetBytes(0, value, buf, len*2, nx_charset_utf16le, nx_string_get_bytes_size_null_terminate) != NErr_Success) buf[0] = 0;
}

int64_t CloudDevice::CacheLookup(songid_t songid, int attributeid)
{
	int key = songid * 100 + attributeid;
	int64_t value = -1;

	IntCache::iterator found = int_cache.find(key);

	if (found == int_cache.end())
	{
		if (db_connection->IDMap_GetInteger(songid, field_names[attributeid], &value) != NErr_Success)
			value = -1;
		int_cache[key] = value;
	}
	else
	{
		value = found->second;
	}

	return value;
}

int64_t CloudDevice::CacheLookupIDMap(songid_t songid, int attributeid)
{
	int key = songid * 100 + attributeid;
	int64_t value = -1;

	IntCache::iterator found = int_cache.find(key);

	if (found == int_cache.end())
	{
		// these are all stored in [idmap] instead of [media] as of build #42
		int64_t playcount = 0, lastplayed = 0, lastupdated = 0, filetime = 0, filesize = 0, bitrate = 0;
		double duration = 0;
		if (db_connection->IDMap_GetProperties(songid, &playcount, &lastplayed, &lastupdated, &filetime, &filesize, &bitrate, &duration) != NErr_Success)
			int_cache[key] = (value = -1);
		else
		{
			// TODO if we add other things to [idmap], we need to update this mapping
			switch (attributeid)
			{
				case IDMAP_DURATION: value = int_cache[key] = (int64_t)(duration*1000.0); break;
				case IDMAP_PLAYCOUNT: value = int_cache[key] = playcount; break;
				case IDMAP_LASTPLAYED: value = int_cache[key] = lastplayed; break;
				case IDMAP_LASTUPDATED: value = int_cache[key] = lastupdated; break;
				case IDMAP_FILESIZE: value = int_cache[key] = filesize; break;
				case IDMAP_BITRATE: value = int_cache[key] = (bitrate / 1000); break;
				default: int_cache[key] = (value = -1); break;
			}
		}
	}
	else
	{
		value = found->second;
	}

	return value;
}

void CloudDevice::CacheLookupAllSources(songid_t songid, int attribute, wchar_t *buf, int len)
{
	if (metadataMap.size() > 0)
	{
		nx_string_t value=0;
		metadataMap[songid]->GetField(attribute, 0, &value);
		if (!value || NXStringGetBytes(0, value, buf, len*2, nx_charset_utf16le, nx_string_get_bytes_size_null_terminate) != NErr_Success) buf[0] = 0;
		NXStringRelease(value);
	}
}

int64_t CloudDevice::CacheLookupAllSources(songid_t songid, int attribute)
{
	if (metadataMap.size() > 0)
	{
		__int64 value = 0;
		if (metadataMap[songid]->GetInteger(attribute, 0, &value) == NErr_Success)
		{
			return value;
		}
	}
	return -1;
}

void CloudDevice::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::ARTIST, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_ARTIST, buf, len);
	}
}

void CloudDevice::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::ALBUM, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_ALBUM, buf, len);
	}
}

void CloudDevice::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::TITLE, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_TITLE, buf, len);
	}
}

int CloudDevice::getTrackTrackNum(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return (int)CacheLookupAllSources(songid, MetadataKeys::TRACK);
	}
	else
	{
		return (int)CacheLookup(songid, FIELD_TRACKNO);
	}
}

int CloudDevice::getTrackDiscNum(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return (int)CacheLookupAllSources(songid, MetadataKeys::DISC);
	}
	else
	{
		return (int)CacheLookup(songid, FIELD_DISC);
	}
}

void CloudDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::GENRE, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_GENRE, buf, len);
	}
}

int CloudDevice::getTrackYear(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		if (metadataMap.size() > 0)
		{
			int year = -1;
			nx_string_t nx_year = 0;
			metadataMap[songid]->GetField(MetadataKeys::YEAR, 0, &nx_year);
			if (NXStringGetLength(nx_year) > 0)
			{
				year = _wtoi(nx_year->string);
			}
			NXStringRelease(nx_year);
			return year;
		}
		return -1;
	}
	else
	{
		return (int)CacheLookup(songid, FIELD_YEAR);
	}
}

__int64 CloudDevice::getTrackSize(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return CacheLookupAllSources(songid, MetadataKeys::FILE_SIZE);
	}
	else
	{
		return CacheLookupIDMap(songid, IDMAP_FILESIZE);
	}
}

int CloudDevice::getTrackLength(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		if (metadataMap.size() > 0)
		{
			double duration = 0;
			if (metadataMap[songid]->GetReal(MetadataKeys::LENGTH, 0, &duration) == NErr_Success)
			{
				return (int)(duration*1000.0);
			}
		}
		return -1;
	}
	else
	{
		return (int)CacheLookupIDMap(songid, IDMAP_DURATION);
	}
}

int CloudDevice::getTrackBitrate(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		if (metadataMap.size() > 0)
		{
			__int64 bitrate = 0;
			if (metadataMap[songid]->GetInteger(MetadataKeys::BITRATE, 0, &bitrate) == NErr_Success)
			{
				return (bitrate ? (int)bitrate / 1000 : 0);
			}
		}
		return -1;
	}
	else
	{
		return (int)CacheLookupIDMap(songid, IDMAP_BITRATE);
	}
}

int CloudDevice::getTrackPlayCount(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return (int)CacheLookupAllSources(songid, MetadataKeys::PLAY_COUNT);
	}
	else
	{
		return (int)CacheLookupIDMap(songid, IDMAP_PLAYCOUNT);
	}
}

int CloudDevice::getTrackRating(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return (int)CacheLookupAllSources(songid, MetadataKeys::RATING);
	}
	else
	{
		return (int)CacheLookup(songid, FIELD_RATING);
	}
}

__time64_t CloudDevice::getTrackLastPlayed(songid_t songid)
{
	// in unix time format
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return CacheLookupAllSources(songid, MetadataKeys::LAST_PLAY);
	}
	else
	{
		return CacheLookupIDMap(songid, IDMAP_LASTPLAYED);
	}
}

__time64_t CloudDevice::getTrackLastUpdated(songid_t songid)
{
	// in unix time format
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return CacheLookupAllSources(songid, MetadataKeys::LAST_UPDATE);
	}
	else
	{
		return CacheLookupIDMap(songid, IDMAP_LASTUPDATED);
	}
}

void CloudDevice::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::ALBUM_ARTIST, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_ALBUMARTIST, buf, len);
	}
}

void CloudDevice::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::PUBLISHER, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_PUBLISHER, buf, len);
	}
}

void CloudDevice::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::COMPOSER, buf, len);
	}
	else
	{
		CacheLookup(songid, FIELD_COMPOSER, buf, len);
	}
}

void CloudDevice::getTrackMimeType(songid_t songid, wchar_t * buf, int len)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		CacheLookupAllSources(songid, MetadataKeys::MIME_TYPE, buf, len);
	}
	else
	{
		CacheLookupMIME(songid, buf, len);
	}
}

int CloudDevice::getTrackType(songid_t songid)
{
	return 0; // TODO
	//return (int)CacheLookup(songid, attributes.type);
}

void CloudDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len)
{
	if (!_wcsicmp(L"cloud", field))
	{
		int key = songid * 100 + 99;
		int value_id = 0;

		buf[0] = 0;
		IntCache::iterator found = int_cache.find(key);
		if (found == int_cache.end())
		{
			if (special_device == DEVICE_HSS || special_device == DEVICE_DROPBOX)
			{
				// deals with known sources available to all devices
				value_id = int_cache[key] = 1;
			}
			else if (special_device == DEVICE_ALL_SOURCES)
			{
				if (metadataMap.size() > 0)
				{
					// deals with known sources available to all devices
					value_id = int_cache[key] = 1;

					__int64 cloud = 0;
					if (metadataMap[songid]->GetInteger(MetadataKeys::CLOUD, 0, &cloud) == NErr_Success)
					{
						if (cloud > 0)
							value_id = int_cache[key] = (int)cloud;
						else
							value_id = int_cache[key] = 0;
					}
					else
					{
						value_id = int_cache[key] = 4;
					}
				}
			}
			else
			{
				// will determines if the file is also available on a cloud location
				// and will then set the icon appropriately as needed for the device
				/*ReferenceCountedNXString media_hash;
				CacheLookupMediaHash(songid, media_hash);

				bool has_uploads = false;
				int *out_device_ids = 0;
				size_t num_device_ids = 0;
				db_connection->IDMap_Get_Devices_From_MediaHash(media_hash, &out_device_ids, &num_device_ids, 0);
				if (num_device_ids > 0)
				{
					for (size_t i = 1; i < cloudDevices.size(); i++)
					{
						for (size_t j = 0; j < num_device_ids; j++)
						{
							if(cloudDevices[i]->device_id == out_device_ids[j])
							{
								SpecialDevice special = cloudDevices[i]->special_device;
								if (special == DEVICE_HSS || special == DEVICE_DROPBOX)
								{
									has_uploads = true;
									break;
								}
							}
						}
					}
				}

				if (out_device_ids)
					free(out_device_ids);

				if (special_device == DEVICE_LOCAL_LIBRARY)
				{
					value_id = value_id_cache[key] = (has_uploads ? 0 : 4);
				}
				else
				{
					value_id = value_id_cache[key] = (has_uploads ? 1 : 3);
				}*/

				bool has_uploads = (cloud_files.find(songid) != cloud_files.end());
				if (special_device == DEVICE_LOCAL_LIBRARY)
				{
					value_id = int_cache[key] = (has_uploads ? 0 : 4);
				}
				else
				{
					value_id = int_cache[key] = (has_uploads ? 1 : 3);
				}
			}
		}
		else
		{
			value_id = (int)found->second;
		}

		if (buf) StringCchPrintfW(buf, len, L"%d", value_id);
	}
	else if (!_wcsicmp(L"cloud_status", field))
	{
		buf[0] = 0;
		int64_t last_seen = 0;
		int on = 0;
		if (db_connection && db_connection->Devices_GetLastSeen(device_id, &last_seen, &on) == NErr_Success)
		{
			// TODO need to come back and do this more appropriately to 'match' other clients
			wchar_t time_str[64] = {0};
			time_t now = time(0) - last_seen;
			if (now <= 5)
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_FEW_SEC_AGO, time_str, ARRAYSIZE(time_str));
			}
			else if (now < 60)
			{
				StringCchPrintfW(time_str, ARRAYSIZE(time_str), WASABI_API_LNGSTRINGW((now == 1 ? IDS_SEC_AGO : IDS_SECS_AGO)), now);
			}
			else
			{
				now /= 60;
				if (now < 60)
				{
					StringCchPrintfW(time_str, ARRAYSIZE(time_str), WASABI_API_LNGSTRINGW((now == 1 ? IDS_MIN_AGO : IDS_MINS_AGO)), now);
				}
				else
				{
					now /= 60;
					if (now < 60)
					{
						StringCchPrintfW(time_str, ARRAYSIZE(time_str), WASABI_API_LNGSTRINGW((now == 1 ? IDS_HOUR_AGO : IDS_HOURS_AGO)), now);
					}
					else
					{
						now /= 24;
						StringCchPrintfW(time_str, ARRAYSIZE(time_str), WASABI_API_LNGSTRINGW((now == 1 ? IDS_DAY_AGO : IDS_DAYS_AGO)), now);
					}
				}
			}
			wchar_t temp[64] = {0};
			WASABI_API_LNGSTRINGW_BUF((on ? IDS_ONLINE : IDS_OFFLINE), temp, ARRAYSIZE(temp));
			StringCchPrintfW(buf, len, WASABI_API_LNGSTRINGW(IDS_LAST_SEEN), temp, time_str);
		}
	}
	else if (!_wcsicmp(L"filepath", field))
	{
		buf[0] = 0;
		if (special_device != DEVICE_ALL_SOURCES)
		{
			ReferenceCountedNXURI filepath;
			if (db_connection && db_connection->IDMap_Get_Filepath(songid, &filepath) == NErr_Success)
			{
				lstrcpyn(buf, filepath->string, len);
			}
		}
	}
	else if (!_wcsicmp(L"metahash", field))
	{
		buf[0] = 0;
		if (special_device == DEVICE_ALL_SOURCES)
		{
			ReferenceCountedNXString metahash;
			if (metadataMap[songid]->GetField(MetadataKeys::METAHASH, 0, &metahash) == NErr_Success)
			{
				lstrcpyn(buf, metahash->string, len);
			}
		}
	}
	// TODO: implement
	//optional
}

void CloudDevice::setMetadata(int internal_id, int field_id, const wchar_t *value)
{
	ReferenceCountedNXString nxvalue;
	if (value && *value)
		NXStringCreateWithUTF16(&nxvalue, value);
	nx_string_t &in_place = value_cache[internal_id*100+field_id];
	NXStringRelease(in_place);
	in_place = NXStringRetain(nxvalue);
}

void CloudDevice::setMetadata(int internal_id, int field_id, int64_t value)
{
	if (value > 0)
	{
		int_cache[internal_id*100+field_id] = value;
	}
	else
	{
		IntCache::iterator found= int_cache.find(internal_id*100+field_id);
		if (found != int_cache.end())
			int_cache.erase(found);
	}
}

// feel free to ignore any you don't support

// TODO: benski> ideally, we should batch all the setTrack*(), etc calls into a data structure and do it an atomic transaction via Media_Update, but this will work for now
void CloudDevice::setTrackArtist(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_ARTIST, value);
}

void CloudDevice::setTrackAlbum(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_ALBUM, value);
}

void CloudDevice::setTrackTitle(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_TITLE, value);
}

void CloudDevice::setTrackTrackNum(songid_t songid, int value)
{
	setMetadata(songid, FIELD_TRACKNO, value);
}

void CloudDevice::setTrackDiscNum(songid_t songid, int value)
{
		setMetadata(songid, FIELD_DISC, value);
}

void CloudDevice::setTrackGenre(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_GENRE, value);
}

void CloudDevice::setTrackYear(songid_t songid, int year)
{
	setMetadata(songid, FIELD_YEAR, year);
}

void CloudDevice::setTrackPlayCount(songid_t songid, int value)
{
// TODO	setMetadata(songid, FIELD_PLAYCOUNT, value);
}

void CloudDevice::setTrackRating(songid_t songid, int value)
{
	setMetadata(songid, FIELD_RATING, value);
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
	setMetadata(songid, FIELD_ALBUMARTIST, value);
}

void CloudDevice::setTrackPublisher(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_PUBLISHER, value);
}

void CloudDevice::setTrackComposer(songid_t songid, const wchar_t *value)
{
	setMetadata(songid, FIELD_COMPOSER, value);
}

void CloudDevice::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
	// TODO: implement
} //optional

typedef struct { songid_t song; CloudDevice * dev; const wchar_t * filename; } tagItem;

wchar_t * tagFunc(const wchar_t * tag, void * p) { //return 0 if not found, -1 for empty tag
	tagItem * s = (tagItem *)p;
	int len = 2048;
	wchar_t * buf = (wchar_t *)malloc(sizeof(wchar_t)*len);
	buf[0]=0;
	// TODO check we're supporting everything we know about!!
	// TODO flesh out to support more values where appropriate
	if (!_wcsicmp(tag, L"artist"))	s->dev->getTrackArtist(s->song, buf, len);
	else if (!_wcsicmp(tag, L"album"))	s->dev->getTrackAlbum(s->song,buf,len);
	else if (!_wcsicmp(tag, L"title"))	s->dev->getTrackTitle(s->song,buf,len);
	else if (!_wcsicmp(tag, L"genre"))	s->dev->getTrackGenre(s->song,buf,len);
	else if (!_wcsicmp(tag, L"year"))	wsprintf(buf,L"%d",s->dev->getTrackYear(s->song));
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))	wsprintf(buf,L"%d",s->dev->getTrackTrackNum(s->song));
	else if (!_wcsicmp(tag, L"discnumber"))	wsprintf(buf,L"%d",s->dev->getTrackDiscNum(s->song));
	else if (!_wcsicmp(tag, L"bitrate"))	wsprintf(buf,L"%d",s->dev->getTrackBitrate(s->song));
	else if (!_wcsicmp(tag, L"filename"))	lstrcpyn(buf,s->filename,len);
	else if (!_wcsicmp(tag, L"albumartist"))	s->dev->getTrackAlbumArtist(s->song,buf,len);
	else if (!_wcsicmp(tag, L"composer"))	s->dev->getTrackComposer(s->song,buf,len);
	else if (!_wcsicmp(tag, L"publisher"))	s->dev->getTrackPublisher(s->song,buf,len);
	return buf;
}

void tagFreeFunc(wchar_t * tag, void * p) { if(tag) free(tag); }

bool getTitle(CloudDevice * dev, songid_t song, const wchar_t * filename, wchar_t * buf, int len) {
	buf[0]=0; buf[len-1]=0;
	tagItem item = {song,dev,filename};
	waFormatTitleExtended fmt={filename,0,NULL,&item,buf,len,tagFunc,tagFreeFunc};
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
	return (buf[0] != 0);
}

static const char *GetExtensionForMIME(nx_string_t mime_type)
{
	if (!mime_type)
		return 0;

	if (!NXStringKeywordCompareWithCString(mime_type, "audio/mp4"))
		return "m4a";
	else if (!NXStringKeywordCompareWithCString(mime_type, "audio/mpeg"))
		return "mp3";
	else if (!NXStringKeywordCompareWithCString(mime_type, "audio/x-ms-wma"))
		return "wma";
	else if (!NXStringKeywordCompareWithCString(mime_type, "application/ogg") || !NXStringKeywordCompareWithCString(mime_type, "audio/ogg"))
		return "ogg";
	else if (!NXStringKeywordCompareWithCString(mime_type, "audio/flac"))
		return "flac";

	return 0;	
}

bool CloudDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	if(!enqueue) //clear playlist
	{ 
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
	}

	ReferenceCountedNXString url, username;
	// gets us the http version instead of https
	REPLICANT_API_CLOUD->GetAPIURL(&url, /*http=*/NErr_True);
	REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0);

	bool errors = false;
	if (special_device == DEVICE_ALL_SOURCES)
	{
		for(int i = 0; i < listLength; i++)
		{
			songid_t curSong = songidList[i];

			ReferenceCountedNXString metahash;
			metadataMap[curSong]->GetField(MetadataKeys::METAHASH, 0, &metahash);

			size_t num_ids = 0;
			int64_t *out_id = 0;
			int *out_device_ids = 0;
			db_connection->IDMap_Get_IDs_From_MetaHash(metahash, &out_id, &out_device_ids, &num_ids);
			//NXStringRelease(metahash);

			if (num_ids > 0)
			{
				bool local_added = false;
				for (size_t j = 0; j < num_ids; j++)
				{
					if (out_id[j] > 0 && out_device_ids[j] == local_device_id)
					{
						enqueueFileWithMetaStructW s = {0};
						wchar_t fn[1024] = {0};
						ReferenceCountedNXURI filepath;
						if (db_connection->IDMap_Get_Filepath(out_id[j], &filepath) == NErr_Success)
						{
							s.filename = filepath->string;
						}

						if (!s.filename || !PathFileExistsW(s.filename))
						{
							ReferenceCountedNXString mime_type, media_hash;
							db_connection->IDMap_GetMediaHash(out_id[j], &media_hash);

							const char *ext=0;
							if (db_connection->IDMap_GetMIME(out_id[j], &mime_type) == NErr_Success)
								ext = GetExtensionForMIME(mime_type);

							if (ext)
								StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s.%S", url->string, AutoUrl(username->string), out_id[j], media_hash->string, ext);
							else
								StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s", url->string, AutoUrl(username->string), out_id[j], media_hash->string);

							s.filename = fn;
						}

						// attempt where possible to fill enqueueFileWithMetaStructW with enough to make the
						// playlist entry more like a 'local' playlist entry so it is like from our library.
						double duration = 0;
						if (db_connection->IDMap_GetProperties(out_id[j], 0, 0, 0, 0, 0, 0, &duration) == NErr_Success)
						{
							s.length = (int)duration;
						}

						// if we can get a formatted title then use it otherwise
						// set the title as the filename so the time can show up
						wchar_t buf[2048] = {0};
						if (getTitle(this, curSong, s.filename, buf, 2048))
						{
							s.title = buf;
						}
						else
						{
							s.title = s.filename;
						}

						local_added = true;
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
						break;
					}
				}

				// if no match, then insert a demostream url from what we can determine
				if (!local_added)
				{
					enqueueFileWithMetaStructW s = {0};
					wchar_t fn[1024] = {0};

					ReferenceCountedNXString mime_type, media_hash;
					db_connection->IDMap_GetMediaHash(out_id[0], &media_hash);

					const char *ext=0;
					if (db_connection->IDMap_GetMIME(out_id[0], &mime_type) == NErr_Success)
						ext = GetExtensionForMIME(mime_type);

					if (ext)
						StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s.%S", url->string, AutoUrl(username->string), out_id[0], media_hash->string, ext);
					else
						StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s", url->string, AutoUrl(username->string), out_id[0], media_hash->string);

					s.filename = fn;
					if (s.filename)
					{
						// attempt where possible to fill enqueueFileWithMetaStructW with enough to make the
						// playlist entry more like a 'local' playlist entry so it is like from our library.
						double duration = 0;
						if (db_connection->IDMap_GetProperties(out_id[0], 0, 0, 0, 0, 0, 0, &duration) == NErr_Success)
						{
							s.length = (int)duration;
						}

						// if we can get a formatted title then use it otherwise
						// set the title as the filename so the time can show up
						wchar_t buf[2048] = {0};
						if (getTitle(this, curSong, s.filename, buf, 2048))
						{
							s.title = buf;
						}
						else
						{
							s.title = s.filename;
						}

						//local_added = true;
						SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
					}
					else
					{
						errors = true;
					}
				}
			}

			if (out_id) free(out_id);
			if (out_device_ids) free(out_ids);
		}
	}
	else
	{
		for(int i = 0; i < listLength; i++)
		{
			int curSong = songidList[i];
			if (curSong)
			{
				int64_t id = 0;
				if (db_connection->IDMap_Get(curSong, &id) == NErr_Success && id > 0)
				{
					wchar_t fn[1024] = {0};
					ReferenceCountedNXURI filepath;

					enqueueFileWithMetaStructW s = {0};
					if (special_device != DEVICE_LOCAL_LIBRARY)
					{
						// see if we have a local track with a matching media hash and use that
						// otherwise we'll have to go the streaming attempt for playing the file

						/* TODO: benski> if the device is another client (not HSS), we can try to find the mediahash 
						on HSS (or really any devices.reachable==1 device) and stream from there */
						ReferenceCountedNXString media_hash;
						CacheLookupMediaHash(curSong, &media_hash);
						if (db_connection->Media_FindFilepathByMediahash(local_device_id, media_hash, &filepath) == NErr_Success)
						{
							s.filename = filepath->string;
						}

						if (!s.filename || !PathFileExistsW(s.filename))
						{
							ReferenceCountedNXString mime_type;

							const char *ext=0;
							if (db_connection->IDMap_GetMIME(curSong, &mime_type) == NErr_Success)
								ext = GetExtensionForMIME(mime_type);

							if (ext)
								StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s.%S", url->string, AutoUrl(username->string), id, media_hash->string, ext);
							else
								StringCbPrintf(fn, sizeof(fn), L"%sdemostream/%S/%llu/%s", url->string, AutoUrl(username->string), id, media_hash->string);

							s.filename = fn;
						}
					}
					else
					{
						db_connection->IDMap_Get_Filepath(curSong, &filepath);
						s.filename = filepath->string;
					}

					// attempt where possible to fill enqueueFileWithMetaStructW with enough to make the
					// playlist entry more like a 'local' playlist entry so it is like from our library.
					double duration = 0;
					if (db_connection->IDMap_GetProperties(curSong, 0, 0, 0, 0, 0, 0, &duration) == NErr_Success)
					{
						s.length = (int)duration;
					}

					// if we can get a formatted title then use it otherwise
					// set the title as the filename so the time can show up
					wchar_t buf[2048] = {0};
					if (getTitle(this, curSong, s.filename, buf, 2048))
					{
						s.title = buf;
					}
					else
					{
						s.title = s.filename;
					}

					SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
				}
				else
				{
					errors = true;
				}
			}
		}
	}

	if(!errors && !enqueue) 
	{ 
		//play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	else if (errors)
	{
		MessageBox(plugin.hwndLibraryParent, L"Some of the requested files could not be added to the playlist", L"Error adding files", MB_ICONWARNING);
	}
	return true;
}

BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax)
{
	unsigned long filenameLength;

	if (NULL == resourceName)
		return FALSE;

	if (FAILED(StringCchCopyExW(buffer, bufferMax, L"res://", &buffer, &bufferMax, 0)))
		return FALSE;

	filenameLength = GetModuleFileNameW(plugin.hDllInstance, buffer, bufferMax);
	if (0 == filenameLength || bufferMax == filenameLength)
		return FALSE;

	buffer += filenameLength;
	bufferMax -= filenameLength;

	if (NULL != resourceType)
	{
		if (FALSE != IS_INTRESOURCE(resourceType))
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceType)))
				return FALSE;
		}
		else
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceType)))
				return FALSE;
		}
	}

	if (FALSE != IS_INTRESOURCE(resourceName))
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceName)))
			return FALSE;
	}
	else
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceName)))
			return FALSE;
	}

	return TRUE;
}

int CloudDevice::UpdateCaches()
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		for (size_t i = 0; i < metadataMap.size(); i++) delete metadataMap[i];
		metadataMap.clear();

		size_t num_metahash = 0;
		if (db_connection) db_connection->MetahashMap_GetMetadata(&metadataMap, &num_metahash);
		return num_metahash;
	}
	else
	{
		value_cache.clear();
		int_cache.clear();
		ids.Reset();
		for (size_t i=0;i<cloud_files.size();i++)
		{
			NXStringRelease(cloud_files[i]);
		}
		cloud_files.clear();

		int *media_ids = 0;
		size_t num_ids = 0;
		if (db_connection) db_connection->Media_GetIDs(device_id, &media_ids, &num_ids);
		this->ids.set(media_ids, num_ids);

		size_t num_files = 0;
		if (out_ids)
		{
			free(out_ids);
			out_ids = 0;
		}
		if (out_filenames)
		{
			free(out_filenames);
			out_filenames = 0;
		}
		if (db_connection) db_connection->IDMap_GetDeviceCloudFiles(device_id, &out_filenames, &out_ids, &num_files);
		for (size_t i = 0; i < num_files; i++)
		{
			cloud_files[out_ids[i]] = out_filenames[i];
		}
		return num_ids;
	}
}

intptr_t CloudDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	switch (param1)
	{
		case DEVICE_NOT_READY_TO_VIEW:
			return (!network_fail ? !firstpull : 0);

		case DEVICE_GET_CLOUD_SOURCES_MENU:
		{
			if (!firstpull)
			{
				HMENU menu = CreatePopupMenu();
				InsertMenu(menu, 0, MF_BYPOSITION | MF_GRAYED, CLOUD_SOURCE_MENUS + 1,
						   WASABI_API_LNGSTRINGW(IDS_UNABLE_DETERMINE_CLOUD_SOURCES));
				return (intptr_t)menu;
			}

			int* count = (int *)param2;
			C_ItemList * items = (C_ItemList *)param4;
			if (!items->GetSize()) return 0;

			HMENU menu = CreatePopupMenu();
			int multiple = (items->GetSize() > 1);
			*count = cloudDevices.size();

			// TODO need to get this working correctly for 'all sources'
			for (size_t i = 1, j = 0; i < cloudDevices.size(); i++)
			{
				// TODO when further device <-> device support happens then this can be changed to allow the menu items, etc
				bool supports_uploads = (cloudDevices[i]->special_device == DEVICE_HSS || cloudDevices[i]->special_device == DEVICE_DROPBOX);
				InsertMenu(menu, (cloudDevices[i]->special_device == DEVICE_LOCAL_LIBRARY ? 0 : ++j),
						   MF_BYPOSITION | (multiple || !supports_uploads ? MF_GRAYED : 0), CLOUD_SOURCE_MENUS + (cloudDevices[i]->device_id),
						   (cloudDevices[i]->special_device == DEVICE_LOCAL_LIBRARY ? WASABI_API_LNGSTRINGW(IDS_LOCAL_LIBRARY) :
																					  cloudDevices[i]->device_name->string));
			}

			// loop over the items and check every device it is known on
			/*for (int it = 0; it < items->GetSize(); it++)
			{*/
				// see if we have a local track with a matching media hash and use that
				// otherwise we'll have to go the streaming attempt for playing the file
				ReferenceCountedNXString hash;
				if (special_device == DEVICE_ALL_SOURCES)
				{
					metadataMap[(songid_t)items->Get(0)]->GetField(MetadataKeys::METAHASH, 0, &hash);
				}
				else
				{
					CacheLookupMediaHash((songid_t)items->Get(0), &hash);
				}

				int *out_device_ids = 0;
				size_t num_device_ids = 0;
				if (special_device == DEVICE_ALL_SOURCES)
				{
					db_connection->IDMap_Get_Devices_From_MetaHash(hash, &out_device_ids, &num_device_ids, 0);
				}
				else
				{
					db_connection->IDMap_Get_Devices_From_MediaHash(hash, &out_device_ids, &num_device_ids, 0);
				}
				//NXStringRelease(hash);
				if (num_device_ids > 0)
				{
					for (size_t i = 0; i < num_device_ids; i++)
					{
						CheckMenuItem(menu, CLOUD_SOURCE_MENUS + (out_device_ids[i]), MF_CHECKED);
						// if we have availability, then we need to allow for removes even if adds are not supported
						EnableMenuItem(menu, CLOUD_SOURCE_MENUS + (out_device_ids[i]), MF_ENABLED);
					}
				}

				if (out_device_ids) free(out_device_ids);
			//}

			return (intptr_t)menu;
		}

		case DEVICE_DO_CLOUD_SOURCES_MENU:
		{
			if (!firstpull) return 0;

			int ret = 0;
			C_ItemList * items = (C_ItemList *)param4;
			if (items->GetSize() == 1)
			{
				// see if we have a local track with a matching media hash and use that
				// otherwise we'll have to go the streaming attempt for playing the file
				ReferenceCountedNXString hash;
				if (special_device == DEVICE_ALL_SOURCES)
				{
					metadataMap[(songid_t)items->Get(0)]->GetField(MetadataKeys::METAHASH, 0, &hash);
				}
				else
				{
					CacheLookupMediaHash((songid_t)items->Get(0), &hash);
				}

				int found = 0, device = (param2 - CLOUD_SOURCE_MENUS);
				int *out_device_ids = 0, *out_media_ids = 0;
				size_t num_device_ids = 0;
				if (special_device == DEVICE_ALL_SOURCES)
				{
					db_connection->IDMap_Get_Devices_From_MetaHash(hash, &out_device_ids, &num_device_ids, &out_media_ids);
				}
				else
				{
					db_connection->IDMap_Get_Devices_From_MediaHash(hash, &out_device_ids, &num_device_ids, &out_media_ids);
				}

				if (num_device_ids > 0)
				{
					for (size_t i = 0; i < num_device_ids; i++)
					{
						if (device == out_device_ids[i])
						{
							ReferenceCountedNXString name;
							if (db_connection->Devices_GetName(out_device_ids[i], &name, 0) == NErr_Success)
							{
								found = out_device_ids[i];
								wchar_t buf[256];
								StringCchPrintfW(buf, ARRAYSIZE(buf), WASABI_API_LNGSTRINGW((device == local_device_id) ? IDS_REMOVE_FROM_LOCAL_CLOUD_DEVICE : IDS_REMOVE_FROM_CLOUD_DEVICE), name->string);
								if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_REMOVE_FROM_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
								{
									if (device == local_device_id)
									{
										deleteTrack(out_media_ids[i]);
									}
									else
									{
										db_connection->BeginTransaction();
										db_connection->IDMap_Remove(out_media_ids[i]);
										db_connection->Commit();
#ifdef _DEBUG
										if (cloud_client) cloud_client->Flush();
#endif
									}

									// if we're removing and it's in the current view, say it's ok to remove
									// but only if this is going to be the last viable option to choose from
									if (num_device_ids == 1 || (out_media_ids[i] == (songid_t)items->Get(0)))
									{
										ret = 1;
									}
								}
								break;
							}
						}
					}
				}

				if (!found)
				{
					// TODO use token as needed
					nx_string_t name, token;
					if (db_connection->Devices_GetName(device, &name, &token) == NErr_Success)
					{
						wchar_t buf[1024] = {0};
						bool supports_uploads = (!NXStringKeywordCompareWithCString(token, HSS_CLIENT) ||
												 !NXStringKeywordCompareWithCString(token, DROPBOX_CLIENT));

						// local add - currently no way to really hit this so not coded for it, etc
						/*if (device == local_device_id)
						{
							StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to add this song to the 'Local Library' device?");
							if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
							{
								ReferenceCountedNXString filepath;
								if (db_connection->Media_FindFilepathByMediahash(device, media_hash, &filepath) == NErr_Success)
								{
									// TODO
									//this->OnFileAdded(filename);
								}
							}
						}
						// uploads (hss / dropbox) from the local device
						else*/
						if (supports_uploads)
						{
							//StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to upload this song to '%s' so it will be available on your other Cloud sources?", name->string);
							//if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
							{
								HWND ml_pmp_window = FindWindow(L"ml_pmp_window", NULL);
								if (IsWindow(ml_pmp_window))
								{
									ReferenceCountedNXURI filepath;
									int ret = NErr_Success;
									if (special_device == DEVICE_ALL_SOURCES)
										ret = db_connection->Media_FindFilepathByMetahash(local_device_id, hash, &filepath);
									else
										ret = db_connection->Media_FindFilepathByMediahash(local_device_id, hash, &filepath);

									if (ret == NErr_Success)
									{
										cloudDeviceTransfer *transfer = new (std::nothrow) cloudDeviceTransfer;
										ZeroMemory(transfer->filenames, MAX_PATH + 1);
										lstrcpyn(transfer->filenames, filepath->string, MAX_PATH);
										transfer->device_token = NXStringRetain(token);
										PostMessage(ml_pmp_window, WM_PMP_IPC, (WPARAM)transfer, PMP_IPC_DEVICECLOUDTRANSFER);
									}
									else
									{
										// TODO possibly try to find a file from the local library / main playlist ???
										StringCchPrintfW(buf, ARRAYSIZE(buf), WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_FIND_FILE), name->string);
										MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_OK | MB_ICONWARNING);
									}
								}
							}
						}
						// TODO device <-> device
						else
						{
							/*StringCchPrintfW(buf, ARRAYSIZE(buf), L"Are you sure you want to add this song to the '%s' device so it can be accessed directly on it?", name->string);
							if (MessageBox(0, buf, WASABI_API_LNGSTRINGW(IDS_ADD_TO_CLOUD_DEVICE), MB_YESNO | MB_ICONQUESTION) == IDYES)
							{
							}*/
						}
					}
				}

				if (out_device_ids) free(out_device_ids);
				if (out_media_ids) free(out_media_ids);
			}
			else
				MessageBox(0, L"multiple selection support not implemented", 0, 0);

			return ret;
		}

		case DEVICE_SYNC_UNSUPPORTED:
			// prevent the sync actions on anything other than the local view
			// TODO change this when device<->device support is sorted out...
			return (!firstpull || special_device == CloudDevice::DEVICE_CLIENT);

		case DEVICE_IS_CLOUD_TX_DEVICE:
			return !NXStringKeywordCompare(device_token, (nx_string_t)param2);

		case DEVICE_GET_CLOUD_DEVICE_ID:
			return device_id;

		case DEVICE_SUPPORTS_PODCASTS:
			// prevent podcast sync functionality since we're doing it via other means
			return 1;

		case DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA:
			// TODO when we support then re-enable
			//		currently we don't have much handling to properly handle this in the 'all sources' view
			return (special_device == DEVICE_ALL_SOURCES);

		case DEVICE_CAN_RENAME_DEVICE:
			// make the local device be a specical case we only do via the preferences
			return (special_device != DEVICE_LOCAL_LIBRARY) && (special_device != DEVICE_HSS) && (special_device != DEVICE_DROPBOX);

			// prevent showing the remove action on the views / menus for specific devices
		case DEVICE_DOES_NOT_SUPPORT_REMOVE:
			// TODO when we've sorted out D-31694 then we can re-enable this
			return 1;//(special_device == DEVICE_ALL_SOURCES || special_device == DEVICE_LOCAL_LIBRARY || special_device == DEVICE_HSS);

		case DEVICE_PLAYLISTS_UNSUPPORTED:
			if (!firstpull) return 1;
			return (special_device != DEVICE_LOCAL_LIBRARY && special_device != DEVICE_HSS);

		case DEVICE_REFRESH:
			return (UpdateCaches() > 0);

		case DEVICE_SUPPORTED_METADATA:
		{
			// notes:
			// - not shown on webview - SUPPORTS_DISCNUM, SUPPORTS_LASTUPDATED, SUPPORTS_ALBUMARTIST, SUPPORTS_COMPOSER, SUPPORTS_PUBLISHER
			// - supported by pmp but not used -  SUPPORTS_ALBUMART
			intptr_t supported = SUPPORTS_ARTIST | SUPPORTS_ALBUM | SUPPORTS_TITLE | SUPPORTS_TRACKNUM | SUPPORTS_DISCNUM |
								 SUPPORTS_GENRE | SUPPORTS_YEAR | SUPPORTS_SIZE | SUPPORTS_LENGTH | SUPPORTS_BITRATE |
								 SUPPORTS_PLAYCOUNT | SUPPORTS_LASTPLAYED | SUPPORTS_LASTUPDATED | SUPPORTS_ALBUMARTIST |
								 SUPPORTS_COMPOSER | SUPPORTS_PUBLISHER | SUPPORTS_RATING |
								 // additions from the 5.7 support
								 SUPPORTS_MIMETYPE;
			return supported;
		}

		case DEVICE_SENDTO_UNSUPPORTED:
			// only allow once we've done a first pull, otherwise block to prevent issues
			// TODO: allow clients to say whether or not they support cloud sync
			// NOTE: changed back to limiting to just HSS and Dropbox until above is done
			//return (!firstpull && special_device != CloudDevice::DEVICE_LOCAL_LIBRARY || (firstpull && special_device != CloudDevice::DEVICE_HSS && special_device != CloudDevice::DEVICE_DROPBOX));
			return (!firstpull || special_device == CloudDevice::DEVICE_LOCAL_LIBRARY || special_device == CloudDevice::DEVICE_ALL_SOURCES);
			//return (!firstpull && special_device != CloudDevice::DEVICE_LOCAL_LIBRARY);// || (firstpull && special_device != CloudDevice::DEVICE_HSS && special_device != CloudDevice::DEVICE_DROPBOX));

		case DEVICE_GET_UNIQUE_ID:
			if (special_device == DEVICE_HSS)
			{
				lstrcpynA((char*)param2, HSS_CLIENT, param3);
			}
			else if (special_device == DEVICE_DROPBOX)
			{
				lstrcpynA((char*)param2, DROPBOX_CLIENT, param3);
			}
			else if (special_device == DEVICE_ALL_SOURCES)
			{
				lstrcpynA((char*)param2, ALL_SOURCES_CLIENT, param3);
			}
			else
			{
				if (special_device == DEVICE_LOCAL_LIBRARY)
				{
					lstrcpynA((char*)param2,"local_desktop",param3);
				}
				else
				{
					StringCchPrintfA((char*)param2,param3,"device_%d",device_id);
				}
			}
			return 1;

		case DEVICE_GET_INI_FILE:
		{
			// we use this to store the pmp view settings in the cloud folder
			// instead of cluttering up the root of the Plugins\ml folder
			wchar_t name[256] = {0};
			if (special_device == DEVICE_HSS)
			{
				lstrcpyn(name, local_device_token->string, MAX_PATH);
			}
			else if (special_device == DEVICE_DROPBOX)
			{
				lstrcpyn(name, L"dropbox", MAX_PATH);
			}
			else
			{
				if (special_device == DEVICE_LOCAL_LIBRARY)
				{
					lstrcpyn(name, L"local", MAX_PATH);
				}
				else
				{
					StringCchPrintf(name, MAX_PATH, L"device_%d", device_id);
				}
			}

			// build this slow so we make sure each directory exists
			PathCombine((wchar_t*)param2, WASABI_API_APP->path_getUserSettingsPath(), L"Cloud");
			CreateDirectory((wchar_t*)param2, NULL);

			wchar_t pl_dir[MAX_PATH];
			lstrcpynW(pl_dir, (wchar_t*)param2, MAX_PATH);
			PathAppend(pl_dir, L"playlists");
			CreateDirectory(pl_dir, NULL);

			PathAppend((wchar_t*)param2, L"views");
			CreateDirectory((wchar_t*)param2, NULL);
			wchar_t ini_filespec[MAX_PATH];
			StringCchPrintf(ini_filespec, MAX_PATH, L"cloud_device_%s.ini", name);
			PathAppend((wchar_t*)param2, ini_filespec);
			return 1;
		}

		case DEVICE_GET_CONNECTION_TYPE:
		{
			const char **type = (const char **)param2;
			*type = "cloud";
			return 1;
		}

		case DEVICE_GET_DISPLAY_TYPE:
		{
			const char **display_type = (const char **)param2;

			if (special_device == DEVICE_HSS)
				*display_type = "Winamp Storage";
			else if (special_device == DEVICE_DROPBOX)
				*display_type = "Dropbox Storage";
			else
			{
				if (special_device == DEVICE_LOCAL_LIBRARY)
					*display_type = "Local Library Client";
				else
					*display_type = "Remote Cloud Client";
			}
			return 1;
		}

		case DEVICE_GET_MODEL:
		{
			if (special_device == DEVICE_LOCAL_LIBRARY)
				WASABI_API_LNGSTRINGW_BUF(IDS_LOCAL_LIBRARY, (wchar_t*)param2, param3);
			else if (device_name)
				StringCchCopy((wchar_t*)param2, param3, device_name->string);		
			else if (special_device == DEVICE_HSS)
				StringCchCopy((wchar_t*)param2, param3, L"Winamp Storage");
			else if (special_device == DEVICE_DROPBOX)
				StringCchCopy((wchar_t*)param2, param3, L"DropBox");
			else
				StringCchCopy((wchar_t*)param2, param3, attributes.device_token->string);
			return 1;
		}

		case DEVICE_GET_ICON:
			if (param2 <= 16 && param3 <= 16)
			{
				wchar_t *buffer = (wchar_t *)param4;
				if (NULL != buffer && 
					FALSE == FormatResProtocol(MAKEINTRESOURCE((special_device == DEVICE_HSS ? IDB_DEVICE_CLOUD :
																(special_device == DEVICE_DROPBOX ? IDB_DEVICE_DROPBOX :
																 (platform_type == PLATFORM_ANDROID ? IDB_DEVICE_ANDROID :
																  (platform_type == PLATFORM_WINDOWS_LAPTOP ? IDB_DEVICE_LAPTOP_CLIENT :
																   IDB_DEVICE_PC_CLIENT))))), RT_BITMAP, buffer, 260))
				{
					buffer[0] = L'\0';
				}
			}
			return 1;

		case DEVICE_VETO_ENCODER:
		case DEVICE_VETO_TRANSCODING:
			// TODO when we support transcoding then disable this as needed
			return 1;

		case DEVICE_GET_PREFS_PARENT:
			{
				if (cloud_hinst && cloud_hinst != (HINSTANCE)1)
				{
					winampMediaLibraryPlugin *(*gp)();
					gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(cloud_hinst, "winampGetMediaLibraryPlugin");
					if (!gp)
					{
						return 0;
					}
					winampMediaLibraryPlugin *mlplugin = gp();
					if (!mlplugin || (mlplugin->version != MLHDR_VER && mlplugin->version != MLHDR_VER_OLD))
					{
						return 0;
					}
					else
					{
						return mlplugin->MessageProc(0x402, 0, 0, 0);
					}
				}
			}
			return 0;

		case DEVICE_GET_NODE_ICON_ID:
		{
			int icon_id = 103;
			if (special_device == DEVICE_HSS)
			{
				icon_id = 101;
			}
			else if (special_device == DEVICE_DROPBOX)
			{
				icon_id = 102;
			}
			else if (special_device == DEVICE_CLIENT)
			{
				if (platform_type == PLATFORM_ANDROID)
				{
					icon_id = 104;
				}
				else if (platform_type == PLATFORM_WINDOWS_LAPTOP)
				{
					icon_id = 106;
				}
			}
			return icon_id;
		}
		break;

		case DEVICE_DONE_SETTING:
			{
				songid_t id = (songid_t)param2;
				// TODO: we have a just-in-time caching system.  we should actually cache a whole record (songid) on the first metadata query so that we can properly update all fields
				current_edit_internal_id = id; // set the context for our ifc_metadata implementation
				if (special_device == DEVICE_LOCAL_LIBRARY) // if it's the local library, we'll "pretend" it was a remote update also so that the changes will eventually get flushed back to disk
					db_connection->Media_Update(id, this, ifc_clouddb::DIRTY_REMOTE|ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);
				else
					db_connection->Media_Update(id, this, ifc_clouddb::DIRTY_LOCAL|ifc_clouddb::DIRTY_FULL);
				// TODO: we should requery all fields, or at least ones that are calculated like mediahash, metahash, etc.
			}
			break;
	}

	return 0;
}

bool CloudDevice::copyToHardDriveSupported()
{
	// TODO: benski> revisit
	/*if (special_device == DEVICE_HSS)
		return true;
	else*/
		return false;	
}

__int64 CloudDevice::songSizeOnHardDrive(songid_t songid)
{
	if (special_device == DEVICE_ALL_SOURCES)
	{
		return CacheLookupAllSources(songid, MetadataKeys::FILE_SIZE);
	}
	else
	{
		return CacheLookupIDMap(songid, IDMAP_FILESIZE);
	}
}

int CloudDevice::copyToHardDrive(songid_t song, // the song to copy
								 wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
								 void * callbackContext, //pass this to the callback
								 void (*callbackFunc)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
								 int * killswitch) // if this gets set to anything other than zero, the transfer has been cancelled by the user
{
	if (!transfer_db_connection)
	{
		if (cloud_client->CreateDatabaseConnection(&transfer_db_connection) != NErr_Success)
		{
			wchar_t msg[256];
			callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_OPEN_DB, msg, 256));
			return -1;
		}
	}

	ReferenceCountedNXString mime_type;

	const char *ext=0;
	if (transfer_db_connection->IDMap_GetMIME(song, &mime_type) == NErr_Success)
		ext = GetExtensionForMIME(mime_type);

	if (!ext)
	{
		// TODO: not sure what to tell the user here
		return -1;
	}

	wchar_t destination[MAX_PATH];
	StringCbPrintf(destination, sizeof(destination), L"%s.%S", path, ext);
	wcscpy(path, destination); // i think ml_pmp wants this to be modified

	this->context = callbackContext;
	this->callback = callbackFunc;
	this->killswitch = killswitch;

	if (!upload_waiter)
		upload_waiter = CreateEvent(0, FALSE, FALSE, 0);

	ReferenceCountedNXURI filename;
	NXURICreateWithUTF8(&filename, AutoChar(destination, CP_UTF8));
	cloud_client->Download(filename, song, this);
	WaitForSingleObject(upload_waiter, INFINITE);

	// TODO: implement
	// // -1 for failed/not supported. 0 for success.
	return 0;
} 

// art functions
void CloudDevice::setArt(songid_t songid, void *buf, int w, int h)
{
	// TODO:
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

int CloudDevice::CloudUploadCallback_OnProgress(uint64_t bytes, uint64_t total)
{
	int percent = MulDiv(100, (int)bytes, (int)total);
	wchar_t msg[128] = {0}, buf[64] = {0};
	StringCbPrintfW(msg, sizeof(msg), WASABI_API_LNGSTRINGW_BUF(IDS_UPLOADING, buf, 64), percent);
	if (context) callback(context, msg);
	if (this->killswitch && *this->killswitch)
	{
		if (upload_waiter)
		{
			SetEvent(upload_waiter);
			CloseHandle(upload_waiter);
		}
	}
	return (this->killswitch && *this->killswitch);
}

void CloudDevice::CloudUploadCallback_OnFinished(int ret)
{
	wchar_t msg[128] = {0};

	if (ret == NErr_Success)
	{
		if (context) callback(context, WASABI_API_LNGSTRINGW_BUF(IDS_UPLOADED, msg, 128));
	}
	else
	{
		wchar_t temp[128] = {0};
		switch (ret)
		{
			case NErr_Unknown:
				swprintf(temp, 128, WASABI_API_LNGSTRINGW_BUF(IDS_FAILED_MISSING_CLOUD_ID, msg, 128), ret);
				break;
			case NErr_ConnectionFailed:
				swprintf(temp, 128, WASABI_API_LNGSTRINGW_BUF(IDS_FAILED_SERVER_CONNECTION, msg, 128));
				break;
			case NErr_Unauthorized:
				swprintf(temp, 128, WASABI_API_LNGSTRINGW_BUF(IDS_FAILED_LOGIN_DETAILS, msg, 128), ret);
				break;
			case NErr_Aborted:
				WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_CANCELLED, temp, 128);
				break;
			default:
				swprintf(temp, 128, WASABI_API_LNGSTRINGW_BUF(IDS_FAILED_X, msg, 128), ret);
				break;
		}
		if (context) callback(context, temp);
	}
	SetEvent(upload_waiter);
}

void CloudDevice::CloudUploadCallback_OnError(nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field)
{
	wchar_t temp[512] = {0};
	if (field && field->len)
	{
		StringCchPrintf(temp, ARRAYSIZE(temp), WASABI_API_LNGSTRINGW(IDS_UPLOAD_FAILED_FULL), code->string, message->string, field->string);
	}
	else
	{
		if (code && code->len)
		{
			StringCchPrintf(temp, ARRAYSIZE(temp), WASABI_API_LNGSTRINGW(IDS_UPLOAD_FAILED_REDUCED), code->string, message->string);
		}
		else
		{
			StringCchPrintf(temp, ARRAYSIZE(temp), WASABI_API_LNGSTRINGW(IDS_UPLOAD_FAILED_SLIM), message->string);
		}
	}
	if (context) callback(context, temp);
}

int CloudDevice::CloudUploadCallback_IsKilled()
{
	return (this->killswitch ? *this->killswitch : 0);
}

/* ifc_metadata implementation 
this is context-sensitive.  we set this->current_edit_internal_id right before passing this to Media_Update(). 
This is fine because Media_Update doesn't need to retain the metadata reference past the lifetime of the function */

/* TODO

	FIELD_ALBUMGAIN,
	FIELD_TRACKGAIN,

	IDMAP_DURATION=80,
	IDMAP_PLAYCOUNT=81,
	IDMAP_LASTPLAYED=82,
	IDMAP_LASTUPDATED=83,
	IDMAP_FILESIZE=84,
	IDMAP_BITRATE=85,
	IDMAP_MEDIAHASH=86,
	IDMAP_MIMETYPE=88,
*/

ns_error_t CloudDevice::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	nx_string_t our_value=0;
	switch(field)
	{
		case MetadataKeys::ARTIST: our_value = value_cache[current_edit_internal_id*100 + FIELD_ARTIST]; break;
		case MetadataKeys::ALBUM: our_value = value_cache[current_edit_internal_id*100 + FIELD_ALBUM]; break;
		case MetadataKeys::ALBUM_ARTIST: our_value = value_cache[current_edit_internal_id*100 + FIELD_ALBUMARTIST]; break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::CATEGORY: our_value = value_cache[current_edit_internal_id*100 + FIELD_CATEGORY]; break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::COMMENT: our_value = value_cache[current_edit_internal_id*100 + FIELD_COMMENT]; break;
		case MetadataKeys::COMPOSER: our_value = value_cache[current_edit_internal_id*100 + FIELD_COMPOSER]; break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::DIRECTOR: our_value = value_cache[current_edit_internal_id*100 + FIELD_DIRECTOR]; break;
		case MetadataKeys::GENRE: our_value = value_cache[current_edit_internal_id*100 + FIELD_GENRE]; break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::PRODUCER: our_value = value_cache[current_edit_internal_id*100 + FIELD_PRODUCER]; break;
		case MetadataKeys::PUBLISHER: our_value = value_cache[current_edit_internal_id*100 + FIELD_PUBLISHER]; break;
		case MetadataKeys::TITLE: our_value = value_cache[current_edit_internal_id*100 + FIELD_TITLE]; break;
		default: return NErr_Unknown;
	}

	if (!our_value)
		return NErr_Empty;
	*value = NXStringRetain(our_value);
	return NErr_Success;
}

ns_error_t CloudDevice::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	IntCache::iterator found;
	switch(field)
	{
		case MetadataKeys::TRACK: found=int_cache.find(current_edit_internal_id*100 + FIELD_TRACKNO); break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::BPM: found=int_cache.find(current_edit_internal_id*100 + FIELD_BPM); break;
		case MetadataKeys::DISC: found=int_cache.find(current_edit_internal_id*100 + FIELD_DISC); break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::DISCS: found=int_cache.find(current_edit_internal_id*100 + FIELD_DISCS); break;
		// TODO: benski> cut because this will never be in our cache: case MetadataKeys::TRACKS: found=int_cache.find(current_edit_internal_id*100 + FIELD_TRACKS); break;
		case MetadataKeys::YEAR: found=int_cache.find(current_edit_internal_id*100 + FIELD_YEAR); break;
		// TODO: benski> I'm cutting this because we aren't guaranteed to have it in cache:  case MetadataKeys::RATING: found=int_cache.find(current_edit_internal_id*100 + FIELD_RATING); break;
		default: return NErr_Unknown;
	}

	if (found == int_cache.end())
		return NErr_Empty;
	if (found->second <= 0)
		return NErr_Empty;

	*value = found->second;
	return NErr_Success;
}

ns_error_t CloudDevice::Metadata_GetReal(int field, unsigned int index, double *value)
{
	return NErr_NotImplemented;
}