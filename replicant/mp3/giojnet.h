#pragma once
#include "nsmp3/giobase.h"
#include "jnetlib/jnetlib.h"
#include "mp3/MPEGHeader.h"
#include "mp3/LAMEInfo.h"
#include "mp3/CVbriHeader.h"
#include "GioReplicant.h"
#include "mp3/OFL.h"
#include "nsid3v2/nsid3v2.h"
#include "http/ifc_http.h"
#include "metadata/ifc_metadata.h"
#include "nswasabi/ID3v2Metadata.h"
#include "player/ifc_player.h"

/* TODO: factor common stuff between this and GioFile */

class GioJNetLib: public GioReplicant
{
public:
	GioJNetLib();
	~GioJNetLib();
	int Open(jnl_http_t http, ifc_http *http_parent);
	
	bool HasContentLength() const;
	/* CGioBase implementation */
	SSC Read(void *pBuffer, int cbToRead, int *pcbRead);
	bool IsEof() const;
	void Run();
	void Reset() { state = State_Data; }
	/* Internal helper functions */
	int Internal_Buffer(size_t bytes, ifc_player *player=0);
	int Internal_PeekFirstFrame(uint8_t *frame_buffer, size_t buffer_len, size_t *buffer_written);
	int Internal_Skip(size_t bytes); /* skips the requested number of bytes in the stream */
	int Internal_Read(void *buffer, size_t bytes); /* attempts to read the whole buffer, even if there's not enough data in the buffer*/

		/* ifc_metadata override */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	

private:

	enum GioState
	{
		State_Unknown,
		State_ID3v2,
		State_Header,
		State_Data,
		State_Closed,
		State_Fail,
	};
	jnl_http_t http;
	ifc_http *http_parent;
	GioState state;
		
	nx_string_t http_uri;
	ID3v2Metadata id3v2_metadata;
};
