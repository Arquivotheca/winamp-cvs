#include <windows.h>
#include <strsafe.h>

#include "../gen_ml/ml.h"

#include "../ml_pmp/pmp.h"
#include "../ml_pmp/transcoder.h"

#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"

#include "../Agave/Language/api_language.h"

#include "api.h"
#include "IpodDevice2.h"
#include "ipodinfo.h"
#include "resource.h"
#include "ipoddb.h"
#include "ipodplaylist.h"
#include "iPodArtworkDB.h"
#include "yail.h"

#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>

#include <shlwapi.h>

// some externs
extern Vector<IpodDevice2*> iPods;
extern PMPDevicePlugin plugin;
extern BOOL EjectVolume(TCHAR cDriveLetter);
extern unsigned long wintime_to_mactime (const __time64_t time);


static __int64 fileSize(const wchar_t * filename);
ArtDataObject * makeThumbMetadata(const ArtworkFormat * thumb, wchar_t drive, Image * image, ArtDB *artdb);

static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

__int64 IpodDevice2::getDeviceCapacityAvailable()  // in bytes
{
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return freeb.QuadPart;
}

__int64 IpodDevice2::getDeviceCapacityTotal()
{
	 // in bytes
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return total.QuadPart;
}

void IpodDevice2::Eject()
{
	UINT olderrmode=SetErrorMode(SEM_FAILCRITICALERRORS);
	// if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this
	for(int i=0; i<iPods.size(); i++)
	{
		if(((IpodDevice2*)iPods.at(i)) == this) 
		{ 
			iPods.eraseAt(i); 
			IpodDB::shutdown();
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
			delete this;
			break;
		}
	}
	SetErrorMode(olderrmode);
}

void IpodDevice2::Close()
{
	// save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	//writeiTunesDB();
	for(int i=0; i<iPods.size(); i++)
	{
		if(((IpodDevice2*)iPods.at(i)) == this) 
		{ 
			iPods.eraseAt(i); 
			IpodDB::shutdown();
			break; 
		}
	}
	delete this; 
} 

void GetFileInfo(const wchar_t * file, const wchar_t * metadata, wchar_t * buf, int len) {
  buf[0]=0;
  extendedFileInfoStructW m = {file,metadata,buf,len};
  SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
}

__int64 GetFileInfoInt64(wchar_t * file, wchar_t * metadata, BOOL *w=NULL) {
  wchar_t buf[100]=L"";
  GetFileInfo(file,metadata,buf,100);
  if(w && buf[0]==0) {(*w) = 0; return 0;}
  return _wtoi64(buf);
}

int GetFileInfoInt(wchar_t * file, wchar_t * metadata, BOOL *w=NULL) {
  wchar_t buf[100]=L"";
  GetFileInfo(file,metadata,buf,100);
  if(w && buf[0]==0) {(*w) = 0; return 0;}
  return _wtoi(buf);
}


// return 0 for success, -1 for failed or cancelled
int IpodDevice2::transferTrackToDevice(const itemRecordW * track, // the track to transfer
																					void * callbackContext, //pass this to the callback
																					void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
																					songid_t * songid, // fill in the songid when you are finished
																					int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																					)
{
	bool transcodefile = false;
	wchar_t outfile[2048];
	wchar_t infile[2048];
	outfile[0] = L'\0';
	infile[0] = L'\0';

	StringCchCopy(infile,2048,((IpodSong*)track)->filename);

	//iPod_mhit * mhit = db->mhsdsongs->mhlt->AddTrack();
	// create a new song object
	IpodSong* song = new IpodSong();

	// arrive at a directory number
	directoryNumber = (directoryNumber + 1) % 20;

	// create the output filename and directory
	wchar_t * ext = PathFindExtensionW(infile);

	if(transcoder && transcoder->ShouldTranscode(infile)) 
	{
		int r = transcoder->CanTranscode(infile, ext, track->length);
		if(r != 0 && r != -1) transcodefile = true;
	}

	bool video = false;
	int isVideo = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, ext, -1, L".m4v", -1)-2;
	if (isVideo == 0)
	{
		video = true;
	}

	int isMP4 = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, ext, -1, L".mp4", -1)-2;

	if(isMP4 == 0) 
	{
		wchar_t buf[100]=L"0";
		extendedFileInfoStructW m = {infile,L"type",buf,100};
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
	
		int isMP4Video = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, buf, -1, L"1", -1)-2;
		if(isMP4Video == 0) 
		{
			video=true; 
			PathRenameExtensionW(infile, L".m4v");
		}
		else
		{
			PathRenameExtensionW(infile, L".m4a");
		}
	}
	// get the next available pid to use in the filename
	song->pid = IpodDB::getNextSongPid();

	StringCchPrintf(outfile, 2048, L"%c:\\iPod_Control\\Music\\F%02d\\", drive, directoryNumber);
	CreateDirectory(outfile,NULL);
	wsprintf(outfile,L"%c:\\iPod_Control\\Music\\F%02d\\w%05d%s", drive, directoryNumber, song->pid, ext);
	
	OutputDebugString(outfile);

	// and the location in the ipod naming scheme
	wchar_t location[2048];
	StringCbCopy(location, sizeof(location), outfile+2);
	int i=0;
	while(location[i] != 0) 
	{ 
		if(location[i]==L'\\') 
		{
			location[i]=L':';
		}
		i++; 
	}

	{
		wchar_t buf[100]=L"";
		int which = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0);
		extendedFileInfoStructW m = {infile,which?L"replaygain_album_gain":L"replaygain_track_gain",buf,100};
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
		if(buf[0]) 
		{
			double gain = _wtof(&buf[buf[0]==L'+'?1:0]);
			//AJ TODO:
			//mhit->soundcheck = (unsigned long)(1000.0 * pow(10.0,-0.1*gain));
		}
	}

	// fill in the new song (item) with our metadata

	//mhit->AddString(MHOD_TITLE)->SetString(track->title);
	lstrcpyn(song->title, track->title, FIELD_LENGTH);
	//mhit->AddString(MHOD_LOCATION)->SetString(location);
	lstrcpyn(song->filename, location, FIELD_LENGTH);
	//mhit->AddString(MHOD_ALBUM)->SetString(track->album);
	lstrcpyn(song->album, track->album, FIELD_LENGTH);
	//mhit->AddString(MHOD_ARTIST)->SetString(track->artist);
	lstrcpyn(song->artist, track->artist, FIELD_LENGTH);
	//mhit->AddString(MHOD_GENRE)->SetString(track->genre);
	lstrcpyn(song->genre, track->genre, FIELD_LENGTH);
	//mhit->AddString(MHOD_COMMENT)->SetString(track->comment);
	lstrcpyn(song->comment, track->comment, FIELD_LENGTH);
	//mhit->AddString(MHOD_ALBUMARTIST)->SetString(track->albumartist);
	lstrcpyn(song->albumartist, track->albumartist, FIELD_LENGTH);
	//mhit->AddString(MHOD_COMPOSER)->SetString(track->composer);
	lstrcpyn(song->composer, track->composer, FIELD_LENGTH);
	//mhit->length = (track->length>0)?track->length*1000:0;
	song->length = (track->length>0)?track->length*1000:0;
	//mhit->year = (track->year>0)?track->year:0;
	song->year = (track->year>0)?track->year:0;
	//mhit->tracknum = (track->track>0)?track->track:0;
	song->track = (track->track>0)?track->track:0;
	song->rating = (unsigned char)(track->rating);
	//mhit->playcount = mhit->playcount2 = track->playcount;
	song->playcount = track->playcount;
	//mhit->lastplayedtime = wintime_to_mactime(track->lastplay);
	song->last_played_time = wintime_to_mactime(track->lastplay);
	//mhit->lastmodifiedtime = wintime_to_mactime(track->lastupd);
	song->last_modified_time = wintime_to_mactime(track->lastupd);
	//mhit->samplerate = 44100; // TODO: benski> we could query this from the input plugin, but we'd have to be careful with HE-AAC
	song->sample_rate = 44100; 
	song->sample_rate2 = 44100;
	song->media_type = video?0x02:0x01;
	song->movie_flag = video?1:0;
	song->sample_count = GetFileInfoInt64(infile,L"numsamples");
	song->pregap = (unsigned long)GetFileInfoInt64(infile,L"pregap");
	song->postgap = (unsigned long)GetFileInfoInt64(infile,L"postgap");
	song->gapless_data = (unsigned long)GetFileInfoInt64(infile,L"endoffset");
	song->track_gapless = 1;

	wchar_t *pubdate = getRecordExtendedItem(track,L"podcastpubdate");
	if(pubdate && *pubdate)
	{
		song->released_time=wintime_to_mactime(_wtoi(pubdate));
	}


	// copy the file over
	int r;
	if(transcodefile)
	{
		r = transcoder->TranscodeFile(infile,outfile,killswitch,callback,callbackContext);
	}
	else 
	{
		// using the standard win32 CopyFile routine
		r = CopyFile(infile,outfile,1);
		if(r) 
		{
			song->size = (unsigned long)fileSize(outfile);
			if (track->bitrate > 0)
			{
				song->bitrate = track->bitrate;
			}
			else if (track->length >= 0)
			{
				song->bitrate = (8 * (song->size / song->length))/1000;
			}
			else 
			{
				song->bitrate = 128;
			}
			
			// Add the track to the master playlist
			ipodPlaylists.at(0)->songs.push_back(song);
			*songid = (songid_t)song;

			// now, write the record into the sqlite db
			IpodDB::addItem(song);

			// art
			wchar_t inifile[] = {drive,L":\\iPod_Control\\iTunes\\ml_pmp.ini"};
			if(artdb && thumbs.size() && GetPrivateProfileInt(L"ml_pmp",L"albumart",1,inifile))
			{
				int w,h;
				ARGB32 *bits;
				if (AGAVE_API_ALBUMART->GetAlbumArt_NoAMG(infile, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS)
				{
					setArt((songid_t)song,bits,w,h);
					WASABI_API_MEMMGR->sysFree(bits);
				}
			} // end art
		}
	} 
	return r;
}

int IpodDevice2::trackAddedToTransferQueue(const itemRecordW *track)
{
	// return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
// TODO: implement
	return -1;
} 

void IpodDevice2::trackRemovedFromTransferQueue(const itemRecordW *track)
{
// TODO: implement

} 

// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
__int64 IpodDevice2::getTrackSizeOnDevice(const itemRecordW *track)
{
// TODO: implement
	return 0;
} 

void IpodDevice2::deleteTrack(songid_t songid)
{
	// physically remove from device. Be sure to remove it from all the playlists!
// TODO: implement
} 

void IpodDevice2::commitChanges()
{
	 // optional. Will be called at a good time to save changes
}

int IpodDevice2::getPlaylistCount()
{
	return ipodPlaylists.size();
} 

// PlaylistName(0) should return the name of the device.
void IpodDevice2::getPlaylistName(int playlistnumber, wchar_t *buf, int len)
{
	lstrcpyn(buf, ipodPlaylists.at(playlistnumber)->name, len);
}

int IpodDevice2::getPlaylistLength(int playlistnumber)
{
	return ipodPlaylists.at(playlistnumber)->numItems;
}

songid_t IpodDevice2::getPlaylistTrack(int playlistnumber,int songnum)
{
	if (ipodPlaylists.at(playlistnumber)->songs.size() < (songnum+1))
	{
		// The playlist has not been populated yet, do that now
		bool itemsPopulated = IpodDB::populateSongs(ipodPlaylists.at(playlistnumber), drive);

		if (!itemsPopulated)
		{
			return NULL;
		}
	}

	// returns a songid
	return (songid_t) ipodPlaylists.at(playlistnumber)->songs.at(songnum);
} 

void IpodDevice2::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	// with playlistnumber==0, set the name of the device.
// TODO: implement
} 

void IpodDevice2::playlistSwapItems(int playlistnumber, int posA, int posB)
{
	// swap the songs at position posA and posB
// TODO: implement
}

void IpodDevice2::sortPlaylist(int playlistnumber, int sortBy)
{
// TODO: implement
}

void IpodDevice2::addTrackToPlaylist(int playlistnumber, songid_t songid)
{
	// TODO: implement
	// adds songid to the end of the playlist
}

void IpodDevice2::removeTrackFromPlaylist(int playlistnumber, int songnum)
{
	// TODO: implement
	//where songnum is the position of the track in the playlist
} 

void IpodDevice2::deletePlaylist(int playlistnumber)
{
// TODO: implement
}

int IpodDevice2::newPlaylist(const wchar_t *name)
{
// TODO: implement
// create empty playlist, returns playlistnumber. -1 for failed.
	return -1;
} 

void IpodDevice2::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->artist);
}

void IpodDevice2::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->album);
}

void IpodDevice2::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->title);
}

int IpodDevice2::getTrackTrackNum(songid_t songid)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;	
	return song->track;
}

int IpodDevice2::getTrackDiscNum(songid_t songid)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;	
	return song->discnum;
}
void IpodDevice2::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->genre);
}

int IpodDevice2::getTrackYear(songid_t songid)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;	
	return song->year;
}
__int64 IpodDevice2::getTrackSize(songid_t songid)
{
	 // in bytes
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;	
	return song->size;
}
int IpodDevice2::getTrackLength(songid_t songid)
{
	// in millisecs
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;
	
	return song->length;
} 

int IpodDevice2::getTrackBitrate(songid_t songid)
{
	// in kbps
	IpodSong* song = (IpodSong*)songid;
	if (!song) return 0;
	
	return song->bitrate;	
} 

int IpodDevice2::getTrackPlayCount(songid_t songid)
{
// TODO: implement
	return 0;
}
int IpodDevice2::getTrackRating(songid_t songid)
{
	 //0-5
// TODO: implement
  return 0;
}

__time64_t IpodDevice2::getTrackLastPlayed(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;

} 

__time64_t IpodDevice2::getTrackLastUpdated(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;
} 

void IpodDevice2::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->albumartist);
}

void IpodDevice2::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->publisher);
}

void IpodDevice2::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
	IpodSong* song = (IpodSong*)songid;
	if (!song) return;	
	StringCchCopy(buf, len, song->composer);
}

int IpodDevice2::getTrackType(songid_t songid)
{
// TODO: implement
 return 0;
}
void IpodDevice2::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) 
{
// TODO: implement
//optional
} 

// feel free to ignore any you don't support
void IpodDevice2::setTrackArtist(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackAlbum(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackTitle(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackTrackNum(songid_t songid, int value)
{
// TODO: implement

}
void IpodDevice2::setTrackDiscNum(songid_t songid, int value)
{
// TODO: implement

}
void IpodDevice2::setTrackGenre(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackYear(songid_t songid, int year)
{
// TODO: implement

}
void IpodDevice2::setTrackPlayCount(songid_t songid, int value)
{
// TODO: implement

}
void IpodDevice2::setTrackRating(songid_t songid, int value)
{
// TODO: implement

}
void IpodDevice2::setTrackLastPlayed(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void IpodDevice2::setTrackLastUpdated(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void IpodDevice2::setTrackAlbumArtist(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackPublisher(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackComposer(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void IpodDevice2::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
// TODO: implement

} //optional

bool IpodDevice2::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	if(!enqueue) //clear playlist
	{ 
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
	}

	for(int i=0; i<listLength; i++) 
	{
		IpodSong *curSong = (IpodSong*)songidList[i];

		if (curSong)
		{
			enqueueFileWithMetaStructW s={0};
			s.filename = _wcsdup(curSong->filename);
			s.title = _wcsdup(curSong->title);
			s.length = curSong->length/1000;

			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		}
		else
		{
			char titleStr[32];
			MessageBoxA(plugin.hwndWinampParent,WASABI_API_LNGSTRING(IDS_CANNOT_OPEN_FILE),
						WASABI_API_LNGSTRING_BUF(IDS_ERROR,titleStr,32),0);
		}
	}

	if(!enqueue) 
	{ 
		//play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	return true;
}

intptr_t IpodDevice2::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	// TODO: implement
	return 0;
}

bool IpodDevice2::copyToHardDriveSupported()
{
// TODO: implement
return false;
}

__int64 IpodDevice2::songSizeOnHardDrive(songid_t song)
{
	// how big a song will be when copied back. Return -1 for not supported.
// TODO: implement
	return 0;

} 

int IpodDevice2::copyToHardDrive(songid_t song, // the song to copy
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

static void fileputinhole(const wchar_t* file, unsigned int pos, int len, void* newdata) {
	__int64 fs = fileSize(file);
	int open_flags = OPEN_EXISTING;
	if(fs <= 0) open_flags = CREATE_NEW;
	HANDLE hw = CreateFile(file,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,open_flags,0,NULL);
	if(hw == INVALID_HANDLE_VALUE) return;
	SetFilePointer(hw,pos,NULL,FILE_BEGIN);
	DWORD written=0;
	WriteFile(hw,newdata,len,&written,NULL);
	CloseHandle(hw);
}

static bool replaceart(songid_t songid, wchar_t driveW, ArtDB *artdb, nu::PtrList<const ArtworkFormat> * thumbs, nu::PtrList<Image> * images) {
	//return false;
	__int64 dbid = ((IpodSong*)songid)->artwork_cache_id;
	int done=0;
	ArtImageList::ArtImageMapIterator art = artdb->imageListDS->imageList->images.find(dbid);
	if(art != artdb->imageListDS->imageList->images.end() && art->second) { // replace old art
		for(size_t i=0; i!=art->second->dataobjs.size(); i++) if(art->second->dataobjs[i]->image) {
			ArtImageName * in = art->second->dataobjs[i]->image;
			for(size_t j=0; j!=thumbs->size(); j++)
			{
				if(in->corrid == thumbs->at(j)->correlation_id) {
					wchar_t file[MAX_PATH];
					wsprintfW(file,L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",driveW,in->corrid);
					int size = images->at(j)->get16BitSize(thumbs->at(j)->row_align, thumbs->at(j)->image_align);
					if(size == in->imagesize) {
						unsigned short *data = (unsigned short *)malloc(size);
						images->at(j)->exportToRGB565((RGB565*)data, thumbs->at(j)->format, thumbs->at(j)->row_align, thumbs->at(j)->image_align);
						fileputinhole(file,in->ithmboffset,in->imagesize,data);
						free(data);
						done++;
					}
				}
			}
		}
	}
	return (done == thumbs->size());
}


// art functions
void IpodDevice2::setArt(songid_t songid, void *bits, int w, int h) { //buf is in format ARGB32*
	
	if(!artdb || !thumbs.size()) return; // art not supported
	IpodSong * song = (IpodSong *)songid;

	if(bits == NULL || w == 0 || h == 0) { // remove art
		ArtImageList::ArtImageMapIterator arti = artdb->imageListDS->imageList->images.find(song->artwork_cache_id);
		if(arti == artdb->imageListDS->imageList->images.end() || !arti->second) return;
		ArtImage * art = arti->second;		
		for(nu::PtrList<ArtDataObject>::iterator j = art->dataobjs.begin(); j!=art->dataobjs.end(); j++) {
			ArtImageName *n = (*j)->image;
			if(n) {
				ArtFile * f = artdb->fileListDS->fileList->getFile(n->corrid);
				if(f) {
					bool found=false;
					for(size_t i=0; i!=f->images.size(); i++) {
						if(!found && f->images[i]->start == n->ithmboffset && --f->images[i]->refcount==0) {
							delete f->images[i];
							f->images.eraseindex(i--);
							found=true;
						}
					}
				}
			}
		}
		//AJ
		//mhit->mhii_link = 0;
		//mhit->artworkcount = 0;
		//mhit->hasArtwork = 0;
		artdb->imageListDS->imageList->images.erase(arti);
		delete art;
	} else {
		//setArt(songid,NULL,0,0); // clear old art first
		
		HQSkinBitmap albumart((ARGB32*)bits, w, h); // wrap image into a bitmap object (no copying done)

		nu::PtrList<Image> images;
		for(size_t i=0; i!=thumbs.size(); i++) {
			BltCanvas canvas(thumbs[i]->width,thumbs[i]->height);
			albumart.stretch(&canvas, 0, 0, thumbs[i]->width,thumbs[i]->height);
			images.push_back(new Image((ARGB32 *)canvas.getBits(), thumbs[i]->width,thumbs[i]->height));
		}
		
		if(!replaceart(songid,drive,artdb,&thumbs,&images)) {
			setArt(songid,NULL,0,0);
			ArtImage * artimg = new ArtImage();
			artimg->songid = song->artwork_cache_id;
			artimg->id = artdb->nextid++;
			artimg->srcImageSize = w*h*4;//0; //fileSize(infile);
			//artimg->srcImageSize = mhit->unk45 = rand();
			//AJ mhit->artworksize = 0;
			for(size_t i=0; i!=thumbs.size(); i++)
			{
				artimg->dataobjs.push_back(makeThumbMetadata(thumbs[i],drive,images[i],artdb));
				//mhit->artworksize += thumbs[i]->width * thumbs[i]->height * sizeof(short);
			}
			artdb->imageListDS->imageList->images.insert(ArtImageList::ArtImageMapPair(artimg->songid,artimg));
			//AJ mhit->artworkcount = 1;
			//AJ mhit->hasArtwork = 1;
			//AJ mhit->mhii_link = artimg->id;
		}
		images.deleteAll();
	}
}

bool IpodDevice2::hasArt(songid_t songid)
{
	if(!artdb) return false;
	IpodSong* song = (IpodSong*)songid;
//	__int64 dbid = ((iPod_mhit*)songid)->dbid;
	//__int64 dbid = song->pid;
	// match artwork based on artwork_cache_id
	__int64 dbid = song->artwork_cache_id;
	if (0 == dbid) return false;

	ArtImageList::ArtImageMapIterator art = artdb->imageListDS->imageList->images.find(dbid);
	if(art == artdb->imageListDS->imageList->images.end() || !art->second) return false;
	int l = art->second->dataobjs.size();
	
	ArtImageName * in=0;

	for(int i=0; i<l; i++) 
	{
		if(art->second->dataobjs[i]->image) {
			in = art->second->dataobjs[i]->image;
			//if(in->corrid == largethumb->correlation_id) break; // that's the best we can do :)
		}
	}
	if(in) return true;
	return false;
}

class ipodart_t {
public:
	ipodart_t(ArtImageName *in, wchar_t driveW,int w, int h, const ArtworkFormat* format): w(w),h(h),image(0),error(0),resized(0),format(format) {
		wsprintf(fn,L"%c:\\iPod_Control\\Artwork",driveW);
		wchar_t *p = fn+wcslen(fn);
		in->filename->GetString(p,MAX_PATH - (p - fn));
		while(*p) {if(*p == L':') *p=L'\\'; p++;}
		offset = in->ithmboffset;
	}
	~ipodart_t() { if(image) delete image; }
	Image * GetImage() {
		if(image || error) return image;
		int size = Image::get16BitSize(w,h,format->row_align, format->image_align);
		RGB565 * r = (RGB565*)calloc(size,1);
		if(!r) { return 0; error=1; }
		FILE *f = _wfopen(fn,L"rb");
		if(!f) { free(r); error=1; return 0; }
		fseek(f,offset,0);
		if(fread(r,size,1,f) != 1) { free(r); fclose(f); error=1; return 0; }
		fclose(f);
		image = new Image(r,w,h,format->format,format->row_align, format->image_align);
		free(r);
		return image;
	}
	Image * RegetImage() {
		if(image) delete image; image=0;
		return GetImage();
	}
	int GetError() {return error;}
	int getHeight(){if(image) return image->getHeight(); else return h;}
	int getWidth() {if(image) return image->getWidth(); else return w;}
	int resized;
	void Resize(int neww, int newh)
	{
		HQSkinBitmap temp(image->getData(), image->getWidth(), image->getHeight()); // wrap into a SkinBitmap (no copying involved)
		BltCanvas newImage(neww,newh);
		temp.stretch(&newImage, 0, 0, neww, newh);
		delete image;
		image = new Image((ARGB32 *)newImage.getBits(), neww, newh);
		resized=1;
	}
private:
	wchar_t fn[MAX_PATH];
	int offset;
	int w,h;
	Image * image;
	int error;
	const ArtworkFormat* format;
};

pmpart_t IpodDevice2::getArt(songid_t songid)
{
	if(!artdb) return 0;
	//__int64 dbid = ((iPod_mhit*)songid)->dbid;
	IpodSong* song = (IpodSong*)songid;
	// find based on the artwork_cache_id
	__int64 dbid = song->artwork_cache_id;

	if (dbid == 0) return 0;

	ArtImageList::ArtImageMapIterator art = artdb->imageListDS->imageList->images.find(dbid);
	if(art == artdb->imageListDS->imageList->images.end() || !art->second) return 0;
	int l = art->second->dataobjs.size();
	
	ArtImageName * in=0;
	int w=0,h=0;
	const ArtworkFormat * format=0;

	for(int i=0; i<l; i++) {
		if(art->second->dataobjs[i]->image && (w < art->second->dataobjs[i]->image->imgw || h < art->second->dataobjs[i]->image->imgh)) {
			in = art->second->dataobjs[i]->image;
			w = in->imgw;
			h = in->imgh;
			for(size_t i=0; i < thumbs.size(); i++)
			{
				const ArtworkFormat *f = thumbs.at(i);
				if(f->width == w && f->height == h)
					format = f;
			}
		}
	}
	if(!in || !format) return 0;

	return (pmpart_t)new ipodart_t(in,drive,w,h,format);
}

void IpodDevice2::releaseArt(pmpart_t art)
{
	if(!art) return;
	ipodart_t *image = (ipodart_t *)art;
	delete image;
}

int IpodDevice2::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h)
{
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return 0;
	HQSkinBitmap temp(image->getData(), image->getWidth(), image->getHeight()); // wrap into a SkinBitmap (no copying involved)
	DCCanvas canvas(dc);
	temp.stretch(&canvas,x,y,w,h);
	return 1;
}

void IpodDevice2::getArtNaturalSize(pmpart_t art, int *w, int *h)
{
	ipodart_t *image = (ipodart_t*)art;
	if(!image) return;
	*h = image->getHeight();
	*w = image->getWidth();
}
void IpodDevice2::setArtNaturalSize(pmpart_t art, int w, int h)
{
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return;
	if(w == image->getWidth() && h == image->getHeight()) return;
	if(((ipodart_t*)art)->resized) {
		image = ((ipodart_t*)art)->RegetImage();
		if(!image) return;
	}
	((ipodart_t*)art)->Resize(w, h);
}
void IpodDevice2::getArtData(pmpart_t art, void* data)
{
	// data ARGB32* is at natural size
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return;
	image->exportToARGB32((ARGB32*)data);
} 

bool IpodDevice2::artIsEqual(pmpart_t at, pmpart_t bt)
{
	if(at == bt) return true;
	if(!at || !bt) return false;
	if(((ipodart_t*)at)->getWidth() != ((ipodart_t*)bt)->getWidth()) return false;
	if(((ipodart_t*)at)->getHeight() != ((ipodart_t*)bt)->getHeight()) return false;
	Image *a = ((ipodart_t*)at)->RegetImage();
	Image *b = ((ipodart_t*)bt)->RegetImage();
	if(!a && !b) return true;
	if(!a || !b) return false;
	if(a->getWidth() != b->getWidth()) return false;
	if(b->getHeight() != b->getHeight()) return false;
	return memcmp(a->getData(),b->getData(),a->getWidth()*a->getHeight()*sizeof(ARGB32)) == 0;
}

// Added for IpodDevice2

IpodDevice2::IpodDevice2(wchar_t drive) : transcoder(NULL) 
{
	this->drive = drive;

	transferQueueLength=0;
	srand(GetTickCount());
	directoryNumber = rand() % 20;

	iPods.push_back(this);

	pmpDeviceLoading load;
	load.dev = this;
	load.UpdateCaption = NULL;

	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)&load,PMP_IPC_DEVICELOADING);

	if(load.UpdateCaption) 
	{
		load.UpdateCaption(WASABI_API_LNGSTRINGW(IDS_IPOD_LOADING),load.context);
	}

	const iPodInfo * info = GetiPodInfo(drive);
	// check if it is sqlite enabled

	bool in=false;
	for(int i=0; i<iPods.size(); i++) 
	{
		if((void*)iPods.at(i) == (void*)this) 
		{
			in=true;
		}
	}

	if(!in) return; //we were deleted during GetiPodInfo...

	if (info)
	{
		// we do have a sqlite ipod - yay
		// start digging thru the db
		// The tables in order of relevancy are
		// 1. item 
		// 2. container (playlists)
		// 3. artist
		// 4. album
		// 5. composer
		// 6. genre_map
		// 7. track_artist
		// 8. track_size_calc
		// 9. avformat_info
		//10. video_info
		
		// there are several others that are mostly used to link the above tables

		// now that we have reasonable confidence that we are able
		// to pull data from the ipod database we create a master playlist
		// and add it to the list of playlists
		IpodPlaylist* masterPlaylist = new IpodPlaylist(true, 0 );

		// init our sqlite db
		bool dbInitialized = IpodDB::init(drive);

		if (!dbInitialized)
		{
			return;
		}

		ipodPlaylists.push_back(masterPlaylist);

		// try and read in playlists if any
		IpodDB::populatePlaylists(&ipodPlaylists);

		bool itemsPopulated = IpodDB::populateSongs(masterPlaylist, drive);

		if (!itemsPopulated)
		{
			IpodDB::shutdown();
			return;
		}

		// artwork info
		const ArtworkFormat* art = GetArtworkFormats(info);
		if(art)
		{
			for(int i=0; art[i].type != THUMB_INVALID; i++) 
			{
				if(art[i].type >= THUMB_COVER_SMALL && art[i].type <= THUMB_COVER_LARGE)
					thumbs.push_back(&art[i]);
			}
		}

		if(parseiTunesDB(thumbs.size()!=0) < 0) 
		{
			for(int j=0; j<iPods.size(); j++) if(iPods.at(j) == this) { iPods.eraseAt(j); break; }
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);	
			delete this;
			return;
		}
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);
	}
	else
	{
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
		return;
	}

	transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
	if(transcoder) 
	{
		transcoder->AddAcceptableFormat(mmioFOURCC('M','4','A',' '));
		transcoder->AddAcceptableFormat(L"mp3");
		//transcoder->AddAcceptableFormat(L"wav");
		transcoder->AddAcceptableFormat(L"m4v");
		transcoder->AddAcceptableFormat(L"m4b");
		transcoder->AddAcceptableFormat(L"aa\0\0");
		transcoder->AddAcceptableFormat(L"mp4");
	}
}

static unsigned char * readFile(char * path, int &len) {
  FILE * f = fopen(path,"rb");
  if(!f) return 0;
  fseek(f,0,2); //seek to end
  int l = ftell(f); //length of file
  unsigned char * data = (unsigned char *)malloc(l);
  if(!data) return 0;
  fseek(f,0,0);
  if(fread(data,1,l,f) != l) { fclose(f); free(data); return 0; }
  fclose(f);
	len = l;
  return data;
}

static unsigned char * readFile(char * path) {
	int l=0;
	return readFile(path,l);
}

int IpodDevice2::parseiTunesDB(bool parseArt) {
	if(parseArt) 
	{
		char dbPath[] = "x:\\iPod_Control\\Artwork\\ArtworkDB";
		dbPath[0]=drive;
		int l=0;
		unsigned char * data = readFile(dbPath,l);
		bool createNew=false;

		if(data) 
		{
			artdb = new ArtDB();
			int r = artdb->parse(data,l,drive);
			if(r<0) 
			{
				delete artdb;
				artdb=NULL;
			}
			free(data);
		} else createNew=true;

		if(createNew) 
		{
			char dir[] = {drive,":\\iPod_Control\\Artwork"};
			CreateDirectoryA(dir,NULL);
			artdb = new ArtDB();
			artdb->makeEmptyDB(drive);
		}
	}

	return 0;
}


IpodDevice2::~IpodDevice2()
{
	char lockPath[] = {drive, ":\\iPod_Control\\iTunes\\iTunesLock"};
	_unlink(lockPath);
	IpodDB::shutdown();
	if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
}


static __int64 fileSize(const wchar_t * filename)
{
	WIN32_FIND_DATA f={0};
	HANDLE h = FindFirstFileW(filename,&f);
	if(h == INVALID_HANDLE_VALUE) return -1;
	FindClose(h);
	ULARGE_INTEGER i;
	i.HighPart = f.nFileSizeHigh;
	i.LowPart = f.nFileSizeLow;
	return i.QuadPart;
}

ArtDataObject * makeThumbMetadata(const ArtworkFormat * thumb, wchar_t drive, Image * image, ArtDB *artdb) {
wchar_t file[MAX_PATH];
	wsprintfW(file,L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",drive,thumb->correlation_id);

	bool found=false;
	ArtFile *f=NULL;
	for(size_t i=0; i < artdb->fileListDS->fileList->files.size(); i++) {
		 f = artdb->fileListDS->fileList->files[i];
		 if(f->corrid == thumb->correlation_id) { found=true; break; }
	}
	if(!found) {
		f = new ArtFile;
		f->corrid = thumb->correlation_id;
		f->imagesize = image->get16BitSize(thumb->row_align, thumb->image_align);
		artdb->fileListDS->fileList->files.push_back(f);
		f->file = _wcsdup(file);
	}

	ArtDataObject * ms = new ArtDataObject;
	ms->type=2;
	ms->image = new ArtImageName;
	ms->image->corrid = thumb->correlation_id;
	ms->image->imgw = thumb->width;
	ms->image->imgh = thumb->height;
	ms->image->imagesize = image->get16BitSize(thumb->row_align, thumb->image_align);
	
	//__int64 fs = fileSize(file);
	//ms->image->ithmboffset = fs>0?fs:0;
	ms->image->ithmboffset = f->getNextHole(ms->image->imagesize);

	wchar_t buf[100];
	StringCchPrintf(buf,100,L":F%04d_1.ithmb",thumb->correlation_id);
	ms->image->filename = new ArtDataObject;
	ms->image->filename->type=3;
	ms->image->filename->SetString(buf);
	unsigned short *data = (unsigned short *)calloc(ms->image->imagesize,1);
	image->exportToRGB565((RGB565*)data, thumb->format, thumb->row_align, thumb->image_align);
	fileputinhole(file,ms->image->ithmboffset,ms->image->imagesize,data);
	//writeDataToThumb(file,data,thumb->width * thumb->height);
	free(data);

	f->images.push_back(new ArtFileImage(ms->image->ithmboffset,ms->image->imagesize,1));
	f->sortImages();

	return ms;
}
