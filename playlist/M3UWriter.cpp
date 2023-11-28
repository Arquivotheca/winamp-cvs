#include "M3UWriter.h"
#include "../nu/AutoChar.h"

M3UWriter::M3UWriter() : fp(0)
{
}

int M3UWriter::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp)
		return 0;

	fprintf(fp,"#EXTM3U\n");

	return 1;
}

void M3UWriter::Write(const wchar_t *filename)
{
	fprintf(fp,"%s\n", (char *)AutoChar(filename));
}

void M3UWriter::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	fprintf(fp,"#EXTINF:%d,%s\n%s\n",length,(char *)AutoChar(title),(char *)AutoChar(filename));
}

void M3UWriter::Close()
{
	fclose(fp);
}