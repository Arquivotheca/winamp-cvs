#include "ACMEncoder.h"

#define rev32(X) ((((DWORD)(X)&0xFF)<<24)|(((DWORD)(X)&0xFF00)<<8)|(((DWORD)(X)&0xFF0000)>>8)|(((DWORD)(X)&0xFF000000)>>24))

static DWORD FileTell(HANDLE hFile)
{
	return SetFilePointer(hFile, 0, 0, FILE_CURRENT);
}
static void FileAlign(HANDLE hFile)
{
	if (FileTell(hFile)&1) SetFilePointer(hFile, 1, 0, FILE_CURRENT);
}

#define BUFSIZE 0x20000

ACMEncoder::ACMEncoder(int srate, int nch, int bps, ACMConfig *config)
{
	m_did_header = 0;
	m_srate = srate;
	m_nch = nch;
	m_bps = bps;
	m_error = 0;
	hStream = 0;
	hStreamResample = 0;
	m_acm_resample_buf = NULL;
	m_acm_resample_outbuf = NULL;
	m_bytes_done = 0;
	m_hlen = 0;
	m_nsam = 0;

	m_acm_buf = (unsigned char *)malloc(BUFSIZE);
	m_acm_outbuf = (unsigned char *)malloc(BUFSIZE);
	m_bytes_inbuf = 0;
	m_bytes_outbuf = 0;
	m_convert_wfx = config->convert_wfx;
	do_header = config->header;

	m_wfx_src.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx_src.nChannels = nch;
	m_wfx_src.nSamplesPerSec = srate;
	m_wfx_src.nAvgBytesPerSec = srate * nch * (bps >> 3);
	m_wfx_src.nBlockAlign = nch * (bps >> 3);
	m_wfx_src.wBitsPerSample = bps;
	m_wfx_src.cbSize = 0;
	MMRESULT rs = acmStreamOpen(&hStream, 0, &m_wfx_src, &m_convert_wfx.wfx, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME);
	if (rs)
	{
		// need resampling
		WAVEFORMATEX wfx1;
		ZeroMemory(&wfx1, sizeof(wfx1));
		wfx1.wFormatTag = WAVE_FORMAT_PCM;
		if (acmFormatSuggest(0, &m_convert_wfx.wfx, &wfx1, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG)) m_error = 1;
		else if (acmStreamOpen(&hStream, 0, &wfx1, &m_convert_wfx.wfx, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME)) m_error = 1;
		else if (acmStreamOpen(&hStreamResample, 0, &m_wfx_src, &wfx1, 0, 0, 0, ACM_STREAMOPENF_NONREALTIME)) m_error = 1;
		else
		{
			ZeroMemory(&ahdResample, sizeof(ahdResample));
			ahdResample.cbStruct = sizeof(ahdResample);
			ahdResample.pbSrc = m_acm_resample_buf = (unsigned char *)malloc(BUFSIZE);
			ahdResample.cbSrcLength = BUFSIZE;
			ahdResample.pbDst = m_acm_resample_outbuf = (unsigned char *)malloc(BUFSIZE);
			ahdResample.cbDstLength = BUFSIZE;
			if (acmStreamPrepareHeader(hStreamResample, &ahdResample, 0)) m_error = 1;
			m_bytes_inbuf_resample = 0;
			m_bytes_outbuf_resample = 0;
		}
	}

	if (!hStream)
	{
		m_error = 1;
		return ;
	}

	ZeroMemory(&ahd, sizeof(ahd));
	ahd.cbStruct = sizeof(ahd);
	ahd.pbSrc = m_acm_buf;
	ahd.cbSrcLength = BUFSIZE;
	ahd.pbDst = m_acm_outbuf;
	ahd.cbDstLength = BUFSIZE;
	if (acmStreamPrepareHeader(hStream, &ahd, 0)) m_error = 1;
}

ACMEncoder::~ACMEncoder()
{
	free(m_acm_buf);
	free(m_acm_outbuf);
	free(m_acm_resample_buf);
	free(m_acm_resample_outbuf);
	if (hStream)
	{
		if (ahd.fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) acmStreamUnprepareHeader(hStream, &ahd, 0);
		acmStreamClose(hStream, 0);
	}
	if (hStreamResample)
	{
		if (ahdResample.fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) acmStreamUnprepareHeader(hStreamResample, &ahdResample, 0);
		acmStreamClose(hStreamResample, 0);
	}
}

int ACMEncoder::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	char *pin = (char *)in;
	char *pout = (char *)out;
	int retval = 0;

	if (!m_did_header && do_header)
	{
		int s = 44;
		s = 4 + 4 + 12 - 4;

		int t;
		if (m_convert_wfx.wfx.wFormatTag == WAVE_FORMAT_PCM) t = 0x10;
		else t = sizeof(WAVEFORMATEX) + m_convert_wfx.wfx.cbSize;
		s += 4 + t;
		if (s&1) s++;

		if (m_convert_wfx.wfx.wFormatTag != WAVE_FORMAT_PCM)
			s += 12;

		s += 8;

		if (out_avail < s) return 0;
		//xx bytes of randomness
		m_hlen = s;
		m_did_header = 1;
		out_avail -= s;
		pout += s;
		retval = s;
	}

	if (!m_bytes_outbuf)
	{
		if (hStreamResample)
		{
			if (!m_bytes_outbuf_resample)
			{
				DWORD flags = ACM_STREAMCONVERTF_BLOCKALIGN;

				int l = min(in_avail, BUFSIZE - m_bytes_inbuf_resample);
				if (l < 0) l = 0;
				if (l > 0) memcpy(m_acm_resample_buf + m_bytes_inbuf_resample, in, l);
				m_bytes_inbuf_resample += l;
				*in_used = l;
				m_nsam += l;

				ahdResample.cbSrcLength = m_bytes_inbuf_resample;
				acmStreamConvert(hStreamResample, &ahdResample, flags);
				m_bytes_inbuf_resample -= ahdResample.cbSrcLengthUsed;
				memcpy(m_acm_resample_buf, m_acm_resample_buf + ahdResample.cbSrcLengthUsed, m_bytes_inbuf_resample); //memmove
				m_bytes_outbuf_resample = ahdResample.cbDstLengthUsed;
			}
			in = (void*)m_acm_resample_outbuf;
			in_avail = m_bytes_outbuf_resample;
			m_bytes_outbuf_resample = 0;
			in_used = NULL;
		}

		DWORD flags = ACM_STREAMCONVERTF_BLOCKALIGN;

		int l = min(in_avail, BUFSIZE - m_bytes_inbuf);
		if (l < 0) l = 0;
		if (l > 0) memcpy(m_acm_buf + m_bytes_inbuf, in, l);
		m_bytes_inbuf += l;
		if (in_used)
		{
			*in_used = l;
			m_nsam += l;
		}

		if (m_bytes_inbuf)
		{
			ahd.cbSrcLength = m_bytes_inbuf;
			acmStreamConvert(hStream, &ahd, flags);
			m_bytes_inbuf -= ahd.cbSrcLengthUsed;
			memcpy(m_acm_buf, m_acm_buf + ahd.cbSrcLengthUsed, m_bytes_inbuf); //memmove
			m_bytes_outbuf = ahd.cbDstLengthUsed;
			m_bytes_done += l;
		}
	}
	if (m_bytes_outbuf)
	{
		int l = min(out_avail, m_bytes_outbuf);
		memcpy(pout, m_acm_outbuf, l);
		m_bytes_outbuf -= l;
		memcpy(m_acm_outbuf, m_acm_outbuf + l, m_bytes_outbuf);
		retval += l;
	}

	return retval;
}

void ACMEncoder::FinishAudio(const wchar_t *filename)
{
	if (!do_header) return ;

	HANDLE fh = CreateFileW(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fh == INVALID_HANDLE_VALUE)
		return;

	int len, i;
	const unsigned char ispred1[4] =
	  {
	    0x52 , 0x49 , 0x46 , 0x46
	  };
	const unsigned char ispred2[12] =
	  {
	    0x57, 0x41 , 0x56 , 0x45 , 0x66 , 0x6d , 0x74 , 0x20 , 0x10 , 0x0 , 0x0 , 0x0
	  };
	const unsigned char iza[4] =
	  {
	    0x64 , 0x61 , 0x74 , 0x61
	  };

	int bps = m_bps;
	int srate = m_srate;
	int nch = m_nch;
	len = m_bytes_done;
	DWORD a;

	FileAlign(fh);

	SetFilePointer(fh, 0, 0, FILE_BEGIN);

	WriteFile(fh, ispred1, sizeof(ispred1), &a, NULL);
	i = len + (m_hlen) - 8;
	if (i&1) i++;
	WriteFile(fh, &i, 4, &a, NULL);
	WriteFile(fh, ispred2, sizeof(ispred2) - (hStream ? 4 : 0), &a, NULL);

	int t;
	if (m_convert_wfx.wfx.wFormatTag == WAVE_FORMAT_PCM) t = 0x10;
	else t = sizeof(WAVEFORMATEX) + m_convert_wfx.wfx.cbSize;
	WriteFile(fh, &t, 4, &a, 0);
	WriteFile(fh, &m_convert_wfx.wfx, t, &a, 0);

	FileAlign(fh);

	DWORD fact_ofs = 0;
	if (m_convert_wfx.wfx.wFormatTag != WAVE_FORMAT_PCM)
	{
		t = rev32('fact');
		WriteFile(fh, &t, 4, &a, 0);
		t = 4;
		WriteFile(fh, &t, 4, &a, 0);
		fact_ofs = FileTell(fh);
		SetFilePointer(fh, 4, 0, FILE_CURRENT);
	}

	t = rev32('data');
	WriteFile(fh, &t, 4, &a, 0);
	DWORD data_ofs = FileTell(fh);

	{
		DWORD t, bw;
		SetFilePointer(fh, 4, 0, FILE_BEGIN);
		t = GetFileSize(fh, 0) - 8;
		WriteFile(fh, &t, 4, &bw, 0);
		DWORD data_size = GetFileSize(fh, 0) - (data_ofs + 4);
		SetFilePointer(fh, data_ofs, 0, FILE_BEGIN);
		WriteFile(fh, &data_size, 4, &bw, 0);
		if (fact_ofs)
		{
			SetFilePointer(fh, fact_ofs, 0, FILE_BEGIN);
			t = m_nsam / ((m_bps >> 3) * m_nch);
			WriteFile(fh, &t, 4, &bw, 0);
		}
	}

	CloseHandle(fh);
}

int ACMEncoder::GetLastError() { return m_error; }