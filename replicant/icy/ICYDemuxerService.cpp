#include "api.h"
#include "ICYDemuxerService.h"
#include "ICYDemuxer.h"
#include "jnetlib/jnetlib.h"
#include "icy/svc_icy_playback.h"
#include "service/ifc_servicefactory.h"
#include "nu/strsafe.h"
#include "nswasabi/ReferenceCounted.h"

static char *accept_list=0;

static int NX_ONCE_API init_accept(nx_once_t once, void *param, void **unused)
{
	GUID http_demuxer_guid = svc_icy_playback::GetServiceType();
	ifc_serviceFactory *sf;
	size_t size=0;

	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
	{
		svc_icy_playback *l = (svc_icy_playback*)sf->GetInterface();
		if (l)
		{
			const char *this_accept;
			size_t i=0;
			while (this_accept=l->EnumerateAcceptedTypes(i++))
			{
				size += strlen(this_accept)+1;
			}
		}
	}

	if (size)
	{
		accept_list = (char *)malloc(size);
		if (accept_list)
		{
			size_t accept_length = size;
			char *p_accept = accept_list;

			n=0;
			while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
			{
				svc_icy_playback *l = (svc_icy_playback*)sf->GetInterface();
				if (l)
				{
					const char *this_accept;
					size_t i=0;
					while (this_accept=l->EnumerateAcceptedTypes(i++))
					{
						if (accept_list == p_accept) // first one added
							StringCchCopyExA(p_accept, accept_length, this_accept, &p_accept, &accept_length, 0);
						else
							StringCchPrintfExA(p_accept, accept_length, &p_accept, &accept_length, 0, " %s", this_accept);
					}
				}
			}
		}
	}
	return 1;
}


ICYDemuxerService::ICYDemuxerService()
{
	NXOnceInit(&once);
	
}

ICYDemuxerService::~ICYDemuxerService()
{
	free(accept_list);
	accept_list=0;
}

const char *ICYDemuxerService::HTTPDemuxerService_EnumerateAcceptedTypes(size_t i)
{
	NXOnce(&once, init_accept, 0);

	if (i == 0)
		return accept_list;

	return 0;
}

const char *ICYDemuxerService::HTTPDemuxerService_GetUserAgent()
{
	return 0;
}

void ICYDemuxerService::HTTPDemuxerService_CustomizeHTTP(jnl_http_t http)
{
	jnl_http_addheader(http, "Icy-MetaData:1");
}

static NError FindPlayback(jnl_http_t http, ifc_icy_playback **playback)
{
	GUID http_demuxer_guid = svc_icy_playback::GetServiceType();
	ifc_serviceFactory *sf;

	//bool again;
	int pass=0;
	//do
	//{
		size_t n = 0;
		//again=false;
		while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
		{
			svc_icy_playback *l = (svc_icy_playback*)sf->GetInterface();
			if (l)
			{
				NError err = l->CreatePlayback(http, playback, 0);
				if (err == NErr_Success)
					return NErr_Success;

				//if (err == NErr_TryAgain)
					//again=true;
			}
		}
	//} while (again);
	return NErr_NoMatchingImplementation;
}

NError ICYDemuxerService::HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass)
{
	const char *icy_metadata = jnl_http_getheader(http, "icy-metaint");
	if (icy_metadata && icy_metadata[0])
	{
		ifc_icy_playback *playback=0;
		if (FindPlayback(http, &playback) == NErr_Success)
		{
			ICYDemuxer *icy_demuxer = new (std::nothrow) ReferenceCounted<ICYDemuxer>;
			if (!icy_demuxer)
			{
				playback->Release();
				return NErr_OutOfMemory;
			}
			icy_demuxer->Initialize(http, playback);
			playback->Release();
			*demuxer = icy_demuxer;
			return NErr_Success;
		}
	}
	return NErr_False;
}
