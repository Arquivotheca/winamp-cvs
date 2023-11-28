/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author:
** Created:
**/

#include "main.h"
#include "stats.h"
#include "WinampAttributes.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "api.h"
#include <malloc.h>
#include <rpc.h>

/* benski> ideas for new stats
bitmask of interesting config options (e.g. 24bit, replay gain)
number of smart views
number of tracks burned
color theme

other things:
add generic key/value system to api_stats for strings (e.g. colortheme)
*/

Stats stats;

Stats::Stats()
{
	memset(values, 0, sizeof(values));
	values[LIBRARY_SIZE]=-1; // for historical reasons
}

void Stats::Init()
{
	char str[Stats::NUM_STATS*9+1]; // each stat is written as 8 digit hex and a comma (9 characters)
	char *p=str;
	int x;
	GetPrivateProfileStringA("WinampReg","Stats","",str,sizeof(str),INI_FILEA);
	for (x = 0; x < NUM_STATS; x ++)
	{
		values[x]=strtol(p,&p,16);
		if (*p) p++;
		else break;
	}
}

void Stats::SetStat(int stat, int value)
{
	if (stat >= 0 && stat < NUM_STATS)
		values[stat] = value;
}

void Stats::IncrementStat(int stat)
{
	if (stat >= 0 && stat < NUM_STATS)
		values[stat]++;
}

void Stats::Write()
{
	char str[Stats::NUM_STATS*9+1]; // each stat is written as 8 digit hex and a comma (9 characters)
	char *str_ptr = str;
	size_t str_size = sizeof(str)/sizeof(*str);
	str[0]=0;

	for (int x = 0; x < NUM_STATS; x ++)
	{
		StringCchPrintfEx(str_ptr, str_size, &str_ptr, &str_size, 0, "%08X,",values[x]);
	}
	WritePrivateProfileString("WinampReg","Stats",str,INI_FILEA);
}

void Stats::GetStats(int stats[NUM_STATS]) const
{
	memcpy(stats, values, sizeof(*stats)*NUM_STATS);
}

void Stats::SetString(const char *key, const wchar_t *value)
{
	WritePrivateProfileString("WinampReg",key,AutoChar(value, CP_UTF8),INI_FILEA);
}

void Stats::GetString(const char *key, wchar_t *value, size_t value_cch) const
	{
		*value = 0;
		char *utf8 = (char *)alloca(value_cch);
		if (utf8)
		{
			GetPrivateProfileString("WinampReg",key,"",utf8,value_cch,INI_FILEA);
			MultiByteToWideCharSZ(CP_UTF8, 0, utf8, -1, value, value_cch);
		}
	}

// return a bitmask of interesting configuration choices
/*static int stats_get_cfg()
{
	int s = 0;
	s |= !!config_replaygain;
	s |= (config_audio_bits == 24) << 1;
	/* TODO: 
	agent on or off
	EQ on
	global hotkeys enabled
	info panel on or off
	remember search on or off
	*/
/*}*/

void stats_write(void)
{
	/* benski>
	  write skin and color theme (if available) on close
		since we'll have a reliable way to get color themes (gen_ff hasn't loaded yet when versioncheck runs)
		and it's a more accurate picture of the skin the user was using
		*/
	const wchar_t *colorTheme = 0;
	if (WASABI_API_COLORTHEMES)
		colorTheme = WASABI_API_COLORTHEMES->getGammaSet();
	stats.SetString("colortheme", colorTheme);
	stats.SetString("skin", config_skin);

	stats.IncrementStat(Stats::LAUNCHES);
	stats.SetStat(Stats::REGVER, g_regver);
	stats.SetStat(Stats::PLEDIT_LENGTH, PlayList_getlength());
	stats.Write();
}

void stats_save()
{
	stats.Write();
}

void stats_getuidstr(char str[512])
{
	GUID uid;
	GetPrivateProfileString("WinampReg","ID","",str,128,INI_FILEA);

	if (strlen(str) > sizeof(GUID)*2) // reset bad ID's which were being generated for some time (fixed in 5.5)
		str[0]=0;

	if (!str[0])
	{
		int x;
		unsigned char *p;

		size_t strsize = 512;
		char *strbuf = str;

		CoCreateGuid(&uid);
		p=(unsigned char *)&uid;
		str[0]=0;
		for (x = 0; x < sizeof(uid); x ++)
		{
			StringCchPrintfEx(strbuf, strsize, &strbuf, &strsize, 0, "%02X", p[x]);
		}
		WritePrivateProfileString("WinampReg","ID",str,INI_FILEA);
	}
}

void Stats_OnPlay(const wchar_t *playstring)
{
	if (!_wcsnicmp(playstring, L"http://", 7) 
		||	    !_wcsnicmp(playstring, L"sc://", 5) 
		||	    !_wcsnicmp(playstring, L"mms://", 6) 
		||	    !_wcsnicmp(playstring, L"icy://", 6))
		stats.IncrementStat(Stats::STREAMS_PLAYED);
	else if (!_wcsnicmp(playstring, L"cda://", 6) ||
		!_wcsicmp(extensionW(playstring), L"cda"))
		stats.IncrementStat(Stats::CDS_PLAYED);
	else 
		stats.IncrementStat(Stats::FILES_PLAYED);
}

void stats_init()
{
	stats.Init();
}

#define CBCLASS Stats
START_DISPATCH;
VCB(SETSTAT, SetStat);
VCB(INCREMENTSTAT, IncrementStat);
VCB(SETSTRING, SetString);
END_DISPATCH;
#undef CBCLASS