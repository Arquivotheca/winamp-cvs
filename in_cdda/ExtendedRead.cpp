#include "main.h"

#include "WindacPlay.h"
#include "VeritasPlay.h"
#include "api.h"
#include "workorder.h"
#include "CDDB.h"

//returns handle!=0 if successful, 0 if error
//size will return the final nb of bytes written to the output, -1 if unknown
//TODO> add output format stuff (srate, nch, bps)
extern "C" __declspec(dllexport) intptr_t winampGetExtendedRead_open(const char *fn, int *size, int *bps, int *nch, int *srate)
{
	s_last_error = NULL;
	C_CDPlay *wp;
	int ret = 1;

	char device;
	int track;
	if (!ParseName((char *)fn, device, track))
		return 0;

	if (playStatus[device].IsRipping() || (g_cdplay && g_cdplay->IsPlaying(device)))
	{
		char title[32];
		MessageBoxA(NULL, WASABI_API_LNGSTRING(IDS_CD_CURRENTLY_IN_USE),
		            WASABI_API_LNGSTRING_BUF(IDS_DRIVE_IN_USE,title,32), MB_OK);
		return 0;
	}

	// Get a ICddbDisc object for MusicID

	ICddbDiscPtr pDisc;
	OpenMusicIDWorkOrder();
	if (workorder)
	{
		MCIDEVICEID submitDev = 0;
		if (!CDOpen(&submitDev, device, L"MusicID")) submitDev = 0;
		if (submitDev)
		{
			DINFO info;
			int ret = GetDiscID(submitDev, &info);
			CDClose(&submitDev);
			if (ret == 0)
			{
				wchar_t szTOC[2048];
				if (Cddb_CalculateTOC(&info, szTOC, sizeof(szTOC)/sizeof(wchar_t)))
					Cddb_GetDiscFromCache(szTOC, &pDisc);
			}			
		}
	}

	if (config_rip_veritas)
	{
		VeritasPlay *vp = new VeritasPlay(true);
		wp = vp;
		ret = wp->open(device, track);
		if (ret)
			delete(wp);
		else if (workorder && pDisc != 0) // see if MusicID is interested
		{
			void *handle = 0;
			int x = workorder->GetSigHandle(&handle, pDisc, track);
			vp->submitHandle = handle;
		}
	}
	if (ret)
	{
		wp = new WindacPlay;
		if (wp->open(device, track))
		{
			delete(wp);
			return 0;
		}
	}
	if (size)
	{
		double s = (double)wp->getlength() / 1000;
		s *= (44100 * 4);
		*size = (int)s;
	}
	if (srate) *srate = 44100;
	if (nch) *nch = 2;
	if (bps) *bps = 16;
	playStatus[device].RippingStarted();
	return (intptr_t)wp;
}

//returns nb of bytes read. -1 if read error (like CD ejected). if (ret<len), EOF is assumed
extern "C" __declspec(dllexport) int winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, int *killswitch)
{
	s_last_error = NULL;
	C_CDPlay*wp = (C_CDPlay*)handle;
	return wp->read(dest, len, killswitch);
}

extern "C" __declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
{
	s_last_error = NULL;
	C_CDPlay *wp = (C_CDPlay*)handle;

	playStatus[wp->driveLetter].RippingStopped();
	delete wp;
	wp = 0;
}

// extended info writing (used by gen_ml to update the local CDDB database after a burn)

extern "C" __declspec(dllexport) char * winampGetExtendedRead_lasterror()
{
	char * retval = s_last_error;
	s_last_error = NULL;
	return retval;
}