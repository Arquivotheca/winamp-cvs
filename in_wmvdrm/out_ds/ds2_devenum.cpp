#include "ds2.h"
#include "strsafe.h"
using namespace Nullsoft::Utility;
#ifdef DS2_HAVE_DEVICES

static const GUID NULL_GUID;

static bool _getcaps(IDirectSound * pDS,LPDSCAPS pCaps,DWORD * speakercfg)
{
	bool rv=1;
	if (pCaps)
	{
		memset(pCaps,0,sizeof(*pCaps));
		pCaps->dwSize=sizeof(*pCaps);
		if (FAILED(pDS->GetCaps(pCaps))) rv=0;
	}
	if (speakercfg)
	{
		if (FAILED(pDS->GetSpeakerConfig(speakercfg))) rv=0;
	}
	return rv;
}

bool DS2::TryGetDevCaps(const GUID *g,LPDSCAPS pCaps,DWORD * speakercfg)
{
	bool rv=0;
	AutoLock lock (ds2Guard LOCKNAME("TryGetDevCaps"));
	if (!g) g=&NULL_GUID;
	if (pDS && GetCurDev()==*g)
	{
		rv=_getcaps(pDS,pCaps,speakercfg);
	}
	
	return rv;
}
#endif