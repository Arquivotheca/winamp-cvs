#include "main.h"
#include "api.h"
#include "../nu/AudioOutput.h"
#include "../Winamp/wa_ipc.h"
#include "skeleton.h"
#include <ogg/ogg.h>
#include "../nu/PtrMap.h"
#include "svc_oggdecoder.h"
#include <api/service/waservicefactory.h>

class OggWait
{
public:
	int WaitOrAbort(int time_in_ms)
	{
		HANDLE events[] = {killswitch/*, seek_event*/};
		int ret = WaitForMultipleObjects(1/*2*/, events, FALSE, time_in_ms);
		if (ret == WAIT_TIMEOUT)
			return 0;
		else if (ret == WAIT_OBJECT_0)
			return 1;
		else if (ret == WAIT_OBJECT_0+1)
			return 2;

		return -1;
	}
};

struct OggStream
{
	OggStream(int serial_number)
	{
		decoder = 0;
		ogg_stream_init(&stream, serial_number);
	}
	ogg_stream_state stream;
	ifc_oggdecoder *decoder;
};

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{
	0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
};

static ifc_oggdecoder *FindDecoder(const ogg_packet *packet)
{
	size_t n=0;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::OGGDECODER, n++))
	{
		svc_oggdecoder *dec = static_cast<svc_oggdecoder *>(sf->getInterface());
		if (dec)
		{
			ifc_oggdecoder *decoder=0;
			if (dec->CreateDecoder(packet))
			{
				sf->releaseInterface(dec);
				return decoder;
			}
			sf->releaseInterface(dec);
		}
	}
	return 0;
}

DWORD CALLBACK OggPlayThread(LPVOID param)
{
	PtrMap<int, OggStream> streams;
	OggSkeleton *skeleton=0;

	wchar_t *filename = (wchar_t *)param;
	ogg_sync_state sync_state;

	if (ogg_sync_init(&sync_state) != 0)
	{
		if (WaitForSingleObject(killswitch, 200) == WAIT_TIMEOUT)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		return 1;
	}

	HANDLE ogg_file = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (ogg_file == INVALID_HANDLE_VALUE)
	{
		ogg_sync_clear(&sync_state);
		if (WaitForSingleObject(killswitch, 200) == WAIT_TIMEOUT)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		return 1;
	}

	
	for (;;)
	{
		ogg_page current_page;
		if (ogg_sync_pageout(&sync_state, &current_page) != 1)
		{
			char *ogg_buffer = ogg_sync_buffer(&sync_state, 4096);
			DWORD bytes_read;
			ReadFile(ogg_file, ogg_buffer, 4096, &bytes_read, 0);
			ogg_sync_wrote(&sync_state, bytes_read);
		}
		else
		{
			// ok, we got a page. now figure out what the fuck we're going to do with it
			OggStream *current_stream = 0;
			int serial_number = ogg_page_serialno(&current_page);
			int bos = ogg_page_bos(&current_page);
			if (bos)
			{ // first page of an elementary stream
				
				streams[serial_number] = current_stream = new OggStream(serial_number);
			}
			else
			{
				current_stream = streams[serial_number];
			}

			if (current_stream)
			{
				// give the page to the corresponding stream object
				ogg_stream_pagein(&current_stream->stream, &current_page);

				// read packets out of the stream
				for (;;)
				{
					ogg_packet current_packet;
					int ret = ogg_stream_packetout(&current_stream->stream, &current_packet);
					if (ret == -1)
					{
						// TODO: turn on sync-fail light
					}
					else if (ret == 0)
					{
						// need more data
						break;
					}
					else
					{
						// TODO: turn off sync-fail light

						// if it's the first packet, we need to create a decoder
						if (current_packet.b_o_s)
						{
							// special-case the skeleton stream
							if (!skeleton && OggSkeleton::IsSkeletonPacket(&current_packet))
							{
								skeleton = new OggSkeleton(&current_packet);
								// TODO: make OggSkeleton implement ifc_oggdecoder
							}
							else
							{
								current_stream->decoder = FindDecoder(&current_packet);
							}
						}
					}
				}

			}
		}
		
	



	}


	return 0;
}

