#include "B4SWriter.h"

/*
TODO: escape XML shit
*/

B4SWriter::B4SWriter() : fp(0)
{
}

int B4SWriter::Open(const wchar_t *filename)
{
	fp = _wfopen(filename, L"wt");
	if (!fp) 
		return 0;

	fprintf(fp, "<playlist>\n");

	return 1;
}

void B4SWriter::Write(const wchar_t *filename)
{
	fprintf(fp, "<entry playstring=\"%s\"/>\n", filename);
}

void B4SWriter::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	fprintf(fp, "<entry playstring=\"%s\">\n", filename);
	fprintf(fp, "<name>%s</name>\n", title);
	fprintf(fp, "<length>%d</length>\n", length);
	fprintf(fp, "</entry>\n", filename);
}

void B4SWriter::Close()
{
	fputs("</playlist>", fp);
	fclose(fp);

}