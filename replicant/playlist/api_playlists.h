#pragma once

#include "foundation/dispatch.h"
#include "service/types.h"
#include "metadata/ifc_metadata.h"

// manages Winamp's master list of playlists

/* Important note to users of this API:
 This API does not actually parse or in any way read the contents of the playlist files themselves.
 It only manages the master "list" of playlists, used in e.g. ml_playlists.

 --- important ---
 This also means that some information retrieved through this API can be inaccurate,
 such as playlist item length or total time.  These values are provided as a cache,
 to speed up display of UI.  They are not to be relied on for determining how many items
 are actually in the playlist.  Don't use this value to allocate memory for data structures,
 unless it's just an initial guess and you allow for realloc'ing if the count is higher.
 -----------------

 It is recommended (but not required) that you call SetInteger to update calculated values,
 such as playlist item length, whenever you parse the playlist and have accurate information.

 If you need playlist parsing, use api_playlistmanager.

 This API is thread-safe, as long as you properly call Lock/Unlock
 Methods which don't require external locking are marked with [*]
 Note that these methods still lock internally

 This uses the ifc_metadata interface to get and retrieve properties.  
 */

// {2DC3C390-D9B8-4a49-B230-EF240ADDDCDB}
static const GUID api_playlistsGUID = 
{ 0x2dc3c390, 0xd9b8, 0x4a49, { 0xb2, 0x30, 0xef, 0x24, 0xa, 0xdd, 0xdc, 0xdb } };

class ifc_playlists_enumerator : public Wasabi2::Dispatchable
{
	protected:
	ifc_playlists_enumerator() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_playlists_enumerator() {}
public:
	int OnInfo(ifc_metadata *playlists_information) { return PlaylistsEnumerator_OnInfo(playlists_information); }
	int OnPlaylist(ifc_metadata *playlist_information) { return PlaylistsEnumerator_OnPlaylist(playlist_information); }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	/* this is information about the playlists collection as a whole.  pretty much all that's valid is MetadataKeys.LENGTH (sum of time) and MetadatKeys.TRACKS (number of playlists) */
	virtual int WASABICALL PlaylistsEnumerator_OnInfo(ifc_metadata *playlists_information)=0;
	virtual int WASABICALL PlaylistsEnumerator_OnPlaylist(ifc_metadata *playlist_information)=0;
};

class api_playlists : public Wasabi2::Dispatchable
{
protected:
	api_playlists() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_playlists() {}

public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return api_playlistsGUID; }
	
	// call these to lock the list of playlists so no one changes in the middle of an operation.  be careful with this!
	// you can use AutoLockT<api_playlists> to help you out
	// indices are only valid between these two calls.  call GetGUID() if you need session-persistent identifiers
	void Lock();
	void Unlock();

	size_t GetIterator();

	// get information about playlists
	int Enumerate(ifc_playlists_enumerator *enumerator) { return Playlists_Enumerate(enumerator); }

	// get information about playlists
	size_t GetCount(); // returns number of playlists
	int GetFilename(nx_uri_t *filename, size_t index); 
	int GetName(nx_string_t *name, size_t index); 
	GUID GetGUID(size_t index); // retrieves a unique ID which identifies this playlist
	int GetPosition(GUID playlist_guid, size_t *index); // retrieves the index where a particular playlist ID lives.

	// manipulating playlists
	// at this time, it is not recommended that you use this API.  It is reserved for ml_playlists.
	int MoveBefore(size_t index1, size_t index2); // moves playlist at position index1 to before index2.  setting index2 to anything larger than GetCount() moves to end
	size_t AddPlaylist(nx_uri_t filename, nx_string_t playlistName, GUID playlist_guid = INVALID_GUID); // [*] adds a new playlist, returns new index.  Generates a GUID if you don't pass third parameter.
	// benski> AddPlaylist locks internally, but you need to lock externally if you want to trust the return value																																																					
	size_t AddPlaylist_NoCallback(nx_uri_t filename, nx_string_t playlistName, GUID playlist_guid = INVALID_GUID); 
	// same as AddPlaylist, but doesn't do a syscallback, use when you want to make a few SetInfo calls, 
	// when you are done, call WASABI_API_SYSCB->syscb_issueCallback(Playlists::CALLBACK, Playlists::PLAYLIST_ADDED, newIndex, 0) yourself

	int SetGUID(size_t index, GUID playlist_guid); // sets (overrides) a playlist ID.  Don't use unless you have some very specific need
	int RenamePlaylist(size_t index, nx_string_t name);
	int MovePlaylist(size_t index, nx_uri_t filename); // sets a new filename.  NOTE: IT'S UP TO YOU TO PHYSICALLY MOVE/RENAME/CREATE THE NEW FILENAME.
	
	int RemovePlaylist(size_t index); // removes a particular playlist
	int ClearPlaylists(); // [*] clears the entire list of playlists.  Use at your own risk :)

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	virtual void WASABICALL Playlists_Lock()=0;
	virtual void WASABICALL Playlists_Unlock()=0;
	virtual size_t WASABICALL Playlists_GetIterator()=0;
	virtual int WASABICALL Playlists_Enumerate(ifc_playlists_enumerator *enumerator)=0;
	virtual size_t WASABICALL Playlists_GetCount()=0;
	virtual int WASABICALL Playlists_GetFilename(nx_uri_t *filename, size_t index)=0;
	virtual int WASABICALL Playlists_GetName(nx_string_t *name, size_t index)=0;
	virtual GUID WASABICALL Playlists_GetGUID(size_t index)=0;
	virtual int WASABICALL Playlists_GetPosition(GUID playlist_guid, size_t *index)=0;
	virtual int WASABICALL Playlists_MoveBefore(size_t index1, size_t index2)=0;
	virtual size_t WASABICALL Playlists_AddPlaylist(nx_uri_t filename, nx_string_t playlistName, GUID playlist_guid = INVALID_GUID)=0;
	virtual size_t WASABICALL Playlists_AddPlaylist_NoCallback(nx_uri_t filename, nx_string_t playlistName, GUID playlist_guid = INVALID_GUID)=0;
	virtual int WASABICALL Playlists_SetGUID(size_t index, GUID playlist_guid)=0;
	virtual int WASABICALL Playlists_RenamePlaylist(size_t index, nx_string_t name)=0;
	virtual int WASABICALL Playlists_MovePlaylist(size_t index, nx_uri_t filename)=0;
	virtual int WASABICALL Playlists_RemovePlaylist(size_t index)=0;
	virtual int WASABICALL Playlists_ClearPlaylists()=0;
};

