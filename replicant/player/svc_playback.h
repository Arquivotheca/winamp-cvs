#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "foundation/error.h"

/* service interface to be implemented separately for different filetypes
(mp3, flac, etc)
this doesn't do much besides create ifc_playback objects

if you return NErr_TryAgain, you will be called again with pass=1 after all other services get a chance at the file
*/

class ifc_player;
class ifc_playback;

// {B4EE3D01-5505-4654-84C1-F0AFF81B4B3F}
static const GUID svc_playback_guid = 
{ 0xb4ee3d01, 0x5505, 0x4654, { 0x84, 0xc1, 0xf0, 0xaf, 0xf8, 0x1b, 0x4b, 0x3f } };

class NOVTABLE svc_playback : public Wasabi2::Dispatchable
{
protected:
	svc_playback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_playback() {}

public:
	static GUID GetServiceType() { return svc_playback_guid; }
	NError CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object)
	{
		if (dispatchable_version == 0)
		{
			if (pass == 0)
				return PlaybackService_CreatePlayback(filename, player, out_playback_object);
			else
				return NErr_False;
		}
		else
			return PlaybackService_CreatePlayback(pass, filename, player, out_playback_object);
	}

		/* TODO:
		// play an already open HTTP stream.  return an error to get the CreatePlayback function called instead
		CreatePlaybackHTTP(JNL_HTTPGet *stream, ifc_player *player, ifc_playback **out_playback_object); 
		// if we ever decide to implement a common filereader class
		CreatePlaybackStream(Agave_Reader *reader, ifc_player *player, ifc_playback **out_playback_object); 
		*/
	enum
	{
		DISP_CREATEPLAYBACK = 0,
		DISPATCHABLE_VERSION=1,
	};
protected:
	/* this no longer have to be implemented, here for compatibility only */
	virtual int WASABICALL PlaybackService_CreatePlayback(nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object) { return PlaybackService_CreatePlayback(0, filename, player, out_playback_object); }

	virtual int WASABICALL PlaybackService_CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object)=0;
};
