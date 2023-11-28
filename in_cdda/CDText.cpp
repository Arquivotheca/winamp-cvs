#include "main.h"
#include "cddb.h"
#include "api.h"
#include "../primo/obj_primo.h"
#include "../nu/ns_wc.h"
#include "../nde/ndestring.h"

const char *ReadLine(const char *input, char *output, size_t size, int codepage)
{
	size--; // leave room for null terminator
	while (*input && *input != '\r' && size)
	{
		char *next = CharNextExA(codepage, input, 0);
		while (input != next)
		{
			if (size)
			{
				*output = *input;
				output++;
				*output = 0; // safe because we left room for the null terminator
				size--;
			}
			input++;
		}
	}


	if (*input == '\r')
		input++;
	if (*input == '\n')
		input++;

	return input;
}


static bool CDText_Process(DINFO *ps, const char *title, const char *performers, const char *composers, int codepage)
{
	char thisTitle[1024];

	const char *titles = title;
	// first, get disc title
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);

	if (thisTitle[0])
	{
		ndestring_release(ps->title);
		int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
		ps->title = ndestring_malloc(count*sizeof(wchar_t));
		MultiByteToWideChar(codepage, 0, thisTitle, -1, ps->title, count);
	}

	// now get track titles
	int trackNum = 0;
	while (*titles)
	{
		if (trackNum == ps->ntracks)
			break;
		TRACKINFO &trackInfo = ps->tracks[trackNum];
		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);

		if (thisTitle[0])
		{
			ndestring_release(trackInfo.title);
			int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
			trackInfo.title = ndestring_malloc(count*sizeof(wchar_t));
			MultiByteToWideChar(codepage, 0, thisTitle, -1, trackInfo.title, count);
		}

		trackNum++;
	}

	titles = performers;
	// now get disc artist
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);

	if (thisTitle[0])
	{
		ndestring_release(ps->artist);
		int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
		ps->artist = ndestring_malloc(count*sizeof(wchar_t));
		MultiByteToWideChar(codepage, 0, thisTitle, -1, ps->artist, count);
	}

	// now get track artists
	trackNum = 0;
	while (*titles)
	{
		if (trackNum == ps->ntracks)
			break;
		TRACKINFO &trackInfo = ps->tracks[trackNum];

		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);

		if (thisTitle[0])
		{
			ndestring_release(trackInfo.artist);
			int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
			trackInfo.artist = ndestring_malloc(count*sizeof(wchar_t));
			MultiByteToWideChar(codepage, 0, thisTitle, -1, trackInfo.artist, count);
		}

		trackNum++;
	}

	titles = composers;
	// now get disc composer
	thisTitle[0] = 0;
	titles = ReadLine(titles, thisTitle, 1024, codepage);

	if (thisTitle[0])
	{
		ndestring_release(ps->composer);
		int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
		ps->composer = ndestring_malloc(count*sizeof(wchar_t));
		MultiByteToWideChar(codepage, 0, thisTitle, -1, ps->composer, count);
	}

	// now get track composers
	trackNum = 0;
	while (*titles)
	{
		if (trackNum == ps->ntracks)
			break;
		TRACKINFO &trackInfo = ps->tracks[trackNum];

		thisTitle[0] = 0;
		titles = ReadLine(titles, thisTitle, 1024, codepage);

		if (thisTitle[0])
		{
			ndestring_release(trackInfo.composer);
			int count = MultiByteToWideChar(codepage, 0, thisTitle, -1, 0, 0);
			trackInfo.composer = ndestring_malloc(count*sizeof(wchar_t));
			MultiByteToWideChar(codepage, 0, thisTitle, -1, trackInfo.composer, count);
		}

		trackNum++;
	}

	ps->populated = true;
	return true;

}

bool DoCDText(DINFO *ps, char device)
{
	if (!device)
		return false;

	if (config_use_veritas) 
	{
		obj_primo *primo=0;
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());

		if (!primo)
			return false;

		DWORD unit = device;
		DWORD tracks;
		if (primo->DiscInfoEx(&unit, 0, NULL, NULL, NULL, &tracks, NULL, NULL) != PRIMOSDK_OK) // CDTextInfoEJ suggest that this needs to be called first
		{
			sf->releaseInterface(primo);
			return false;
		}
		if (ps->ntracks == 0) // go ahead and set if it's not set yet
			ps->ntracks = tracks;
		char titleE[8192] = "", performerE[8192] = "", composerE[8192] = "", titleJ[2000] = "", performerJ[2000] = "", composerJ[2000] = "";
		if (primo->CDTextInfoEJ(&unit, (PBYTE)titleE, (PBYTE)performerE, (PBYTE)composerE, (PBYTE)titleJ, (PBYTE)performerJ, (PBYTE)composerJ) == PRIMOSDK_OK)
		{
			sf->releaseInterface(primo);
			// read titles
			if (titleE[0])
				return CDText_Process(ps, titleE, performerE, composerE, 28591);
			else
				return CDText_Process(ps, titleJ, performerJ, composerJ, 932);
		}
	}

	return false;
}
