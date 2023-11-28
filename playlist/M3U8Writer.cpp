#include "M3U8Writer.h"
#include "../nu/AutoChar.h"
#include <bfc/platform/types.h>

M3U8Writer::M3U8Writer() : fp(0)
{
}

int M3U8Writer::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp)
		return 0;
  
	uint8_t bom[3] = {0xEF, 0xBB, 0xBF};
	fwrite(bom, sizeof(bom), 1, fp);
	fprintf(fp,"#EXTM3U\n");

	return 1;
}

void M3U8Writer::Write(const wchar_t *filename)
{
	fprintf(fp,"%s\n", (char *)AutoChar(filename, CP_UTF8));
}

void M3U8Writer::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	fprintf(fp,"#EXTINF:%d,%s\n%s\n",length,(char *)AutoChar(title, CP_UTF8),(char *)AutoChar(filename, CP_UTF8));
}

void M3U8Writer::Close()
{
	fclose(fp);
}