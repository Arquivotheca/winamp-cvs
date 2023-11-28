#include "device.h"

TemplateDevice device;


__int64 TemplateDevice::getDeviceCapacityAvailable()  // in bytes
{
// TODO: implement

	// TODO: implement
	return 0;
}

__int64 TemplateDevice::getDeviceCapacityTotal()
{
	 // in bytes
// TODO: implement
	return 0;
}

void TemplateDevice::Eject()
{
	 // if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this
	// TODO: implement
}

void TemplateDevice::Close()
{
	// save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this
// TODO: implement
} 

// return 0 for success, -1 for failed or cancelled
int TemplateDevice::transferTrackToDevice(const itemRecordW * track, // the track to transfer
																					void * callbackContext, //pass this to the callback
																					void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
																					songid_t * songid, // fill in the songid when you are finished
																					int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																					)
{
// TODO: implement
	return -1;
}
int TemplateDevice::trackAddedToTransferQueue(const itemRecordW *track)
{
	// return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
// TODO: implement
	return -1;
} 

void TemplateDevice::trackRemovedFromTransferQueue(const itemRecordW *track)
{
// TODO: implement

} 

// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
__int64 TemplateDevice::getTrackSizeOnDevice(const itemRecordW *track)
{
// TODO: implement
	return 0;
} 

void TemplateDevice::deleteTrack(songid_t songid)
{
	// physically remove from device. Be sure to remove it from all the playlists!
// TODO: implement
} 

void TemplateDevice::commitChanges()
{
	 // optional. Will be called at a good time to save changes
}

int TemplateDevice::getPlaylistCount()
{
// TODO: implement
// always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	return 1;
} 

// PlaylistName(0) should return the name of the device.
void TemplateDevice::getPlaylistName(int playlistnumber, wchar_t *buf, int len)
{
// TODO: implement

}
int TemplateDevice::getPlaylistLength(int playlistnumber)
{
// TODO: implement
	return 0;
}

songid_t TemplateDevice::getPlaylistTrack(int playlistnumber,int songnum)
{
	// returns a songid
	// TODO: implement
	return 0;
} 

void TemplateDevice::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	// with playlistnumber==0, set the name of the device.
// TODO: implement
} 

void TemplateDevice::playlistSwapItems(int playlistnumber, int posA, int posB)
{
	// swap the songs at position posA and posB
// TODO: implement
}

void TemplateDevice::sortPlaylist(int playlistnumber, int sortBy)
{
// TODO: implement
}

void TemplateDevice::addTrackToPlaylist(int playlistnumber, songid_t songid)
{
	// TODO: implement
	// adds songid to the end of the playlist
}

void TemplateDevice::removeTrackFromPlaylist(int playlistnumber, int songnum)
{
	// TODO: implement
	//where songnum is the position of the track in the playlist
} 

void TemplateDevice::deletePlaylist(int playlistnumber)
{
// TODO: implement
}

int TemplateDevice::newPlaylist(const wchar_t *name)
{
// TODO: implement
// create empty playlist, returns playlistnumber. -1 for failed.
	return -1;
} 

void TemplateDevice::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement
}

void TemplateDevice::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement

}
void TemplateDevice::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement

}
int TemplateDevice::getTrackTrackNum(songid_t songid)
{
// TODO: implement
return 0;
}
int TemplateDevice::getTrackDiscNum(songid_t songid)
{
// TODO: implement
return 0;
}
void TemplateDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
// TODO: implement
}

int TemplateDevice::getTrackYear(songid_t songid)
{
// TODO: implement
return 0;
}
__int64 TemplateDevice::getTrackSize(songid_t songid)
{
	 // in bytes
// TODO: implement
return 0;
}
int TemplateDevice::getTrackLength(songid_t songid)
{
	// in millisecs
	// TODO: implement
	return 0;
} 

int TemplateDevice::getTrackBitrate(songid_t songid)
{
	// in kbps
// TODO: implement
return 0;
} 

int TemplateDevice::getTrackPlayCount(songid_t songid)
{
// TODO: implement
	return 0;
}
int TemplateDevice::getTrackRating(songid_t songid)
{
	 //0-5
// TODO: implement
  return 0;
}

__time64_t TemplateDevice::getTrackLastPlayed(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;

} 

__time64_t TemplateDevice::getTrackLastUpdated(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;
} 

void TemplateDevice::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement
}

void TemplateDevice::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement
}

void TemplateDevice::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
// TODO: implement
}

int TemplateDevice::getTrackType(songid_t songid)
{
// TODO: implement
 return 0;
}
void TemplateDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) 
{
// TODO: implement
//optional
} 

// feel free to ignore any you don't support
void TemplateDevice::setTrackArtist(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackAlbum(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackTitle(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackTrackNum(songid_t songid, int value)
{
// TODO: implement

}
void TemplateDevice::setTrackDiscNum(songid_t songid, int value)
{
// TODO: implement

}
void TemplateDevice::setTrackGenre(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackYear(songid_t songid, int year)
{
// TODO: implement

}
void TemplateDevice::setTrackPlayCount(songid_t songid, int value)
{
// TODO: implement

}
void TemplateDevice::setTrackRating(songid_t songid, int value)
{
// TODO: implement

}
void TemplateDevice::setTrackLastPlayed(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void TemplateDevice::setTrackLastUpdated(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format
void TemplateDevice::setTrackAlbumArtist(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackPublisher(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackComposer(songid_t songid, const wchar_t *value)
{
// TODO: implement

}
void TemplateDevice::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
// TODO: implement

} //optional

bool TemplateDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	 // return false if unsupported
// TODO: implement
return false;
}

intptr_t TemplateDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	// TODO: implement
	return 0;
}

bool TemplateDevice::copyToHardDriveSupported()
{
// TODO: implement
return false;
}

__int64 TemplateDevice::songSizeOnHardDrive(songid_t song)
{
	// how big a song will be when copied back. Return -1 for not supported.
// TODO: implement
	return 0;

} 

int TemplateDevice::copyToHardDrive(songid_t song, // the song to copy
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
void TemplateDevice::setArt(songid_t songid, void *buf, int w, int h)
{
	//buf is in format ARGB32*
// TODO: implement

} 
bool TemplateDevice::hasArt(songid_t songid)
{
// TODO: implement
	return false;
}

pmpart_t TemplateDevice::getArt(songid_t songid)
{
// TODO: implement
	return 0;
}

void TemplateDevice::releaseArt(pmpart_t art)
{
// TODO: implement

}
int TemplateDevice::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h)
{
// TODO: implement
	return 0;
}

void TemplateDevice::getArtNaturalSize(pmpart_t art, int *w, int *h)
{
// TODO: implement

}
void TemplateDevice::setArtNaturalSize(pmpart_t art, int w, int h)
{
// TODO: implement

}
void TemplateDevice::getArtData(pmpart_t art, void* data)
{
	// data ARGB32* is at natural size
// TODO: implement
} 

bool TemplateDevice::artIsEqual(pmpart_t a, pmpart_t b)
{
// TODO: implement
return false;
}
