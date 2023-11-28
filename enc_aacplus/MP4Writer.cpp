#include "MP4Writer.h"
#include <strsafe.h>
MP4Writer::MP4Writer()
{
	wchar_t tmppath[MAX_PATH-14];
	GetTempPathW(MAX_PATH-14,tmppath);
	GetTempFileNameW(tmppath, L"mp4", 0, tempfile);
	mp4File = MP4Create(tempfile);
	//MP4AddODTrack(mp4File);
	if(!mp4File)
	{
		return;
	}
}

MP4Writer::~MP4Writer()
{
}

void MP4Writer::CloseTo(const wchar_t *filename)
{
	MP4Close(mp4File);
	//MP4Optimize(tempfile, NULL, 0);
	mp4File=0;
	MP4MakeIsmaCompliant(tempfile, 0, true);
	DeleteFileW(filename);
	if (MoveFileW(tempfile,filename) == 0) // if the function fails
	{
		CopyFileW(tempfile,filename, FALSE);
		DeleteFileW(tempfile);
	}
}

void MP4Writer::WriteASC(void *buf, size_t size)
{
	MP4SetTrackESConfiguration(mp4File, mp4Track, (unsigned __int8 *)buf, size);
}

void MP4Writer::WriteGaps(unsigned int pregap, unsigned int postgap, unsigned int totalSamples)
{
	char data[128];
	StringCchPrintfA(data, 128, "%08X %08X %08X %016X %08X %08X %08X %08X %08X %08X %08X %08X", 0, pregap, postgap, totalSamples, 0, 0,0, 0,0, 0,0, 0);
	MP4SetMetadataFreeForm(mp4File, "iTunSMPB", (u_int8_t *)data, lstrlenA(data));

}

void MP4Writer::Write(void *buf, size_t size, MP4Duration duration)
{
	MP4WriteSample(mp4File, mp4Track, (unsigned __int8 *)buf, size, duration);
}

static unsigned __int8 CalculateProfileLevel(int sampleRate, int aacRate, int nch, bool he_aac, bool backwardsCompatible)
{
	if (he_aac) 
	{
		if (backwardsCompatible)
		{
			if (nch>2)
				return 0x10;  /* HQ Audio Profile L3 */
			else
				return 0x0f;  /* HQ Audio Profile L2 */
		}
		else
		{
			if (nch>2)
			{
				if (sampleRate > 48000 || aacRate > 24000)
					return 0x2f;  /* HE AAC Profile L5 */
				else
					return 0x2e;  /* HE AAC Profile L4 */
			}
			else
			{
				if (sampleRate > 48000)
					return  0x2f;  /* HE AAC Profile L5 */
				else if (aacRate > 24000)
					return 0x2d;  /* HE AAC Profile L3 */
				else
					return 0x2c;  /* HE AAC Profile L2 */
			}
		}
	}
	else // LC-AAC
	{
		if (nch>2)
		{
			if (sampleRate > 48000)
				return 0x1f;  /* Natural Audio Profile L2 */
			else
				return 0x10;  /* HQ Audio Profile L3 */
		}
		else
		{
			if (sampleRate > 48000)
				return 0x1f;  /* Natural Audio Profile L2 */
			else
				return 0x0f;  /* HQ Audio Profile L2 */
		}
	}
}

void MP4Writer::AddAudioTrack(aacPlusEncOutputFormat *format, bool backwardsCompatible)
{
	unsigned int aacRate = format->sampleRate, nch=2, sampleRate = format->sampleRate;
	if (format->sbrMode == SBR_NORMAL)
		aacRate/=2;

	switch(format->channelMode)
	{
	case MONO:
	case PARAMETRIC_STEREO:
		nch=1; break;
	case STEREO:
	case STEREO_INDEPENDENT:
	case DUAL_CHANNEL:
		nch=2; break;
	case MODE_4_CHANNEL_2CPE:
		nch=4; break;
	case MODE_4_CHANNEL_MPEG:
		nch=4; break;
	case MODE_5_CHANNEL:
		nch=5; break;
	case MODE_5_1_CHANNEL:
		nch=6; break;
	case MODE_6_1_CHANNEL:
		nch=7; break;
	case MODE_7_1_CHANNEL:
		nch=8; break;
	}
	MP4SetTimeScale(mp4File, sampleRate);
	int frameRate;
	if (format->sbrMode != SBR_OFF)
		frameRate=2048;
	else
		frameRate=1024;

	mp4Track = MP4AddAudioTrack(mp4File, sampleRate, frameRate, MP4_MPEG4_AUDIO_TYPE); 
	MP4SetAudioProfileLevel(mp4File,
		CalculateProfileLevel(sampleRate, aacRate, nch, (format->sbrMode != SBR_OFF), backwardsCompatible));
}
