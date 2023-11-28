#include "M3ULoader.h"
#include "M3UFileInfo.h"
#include "constants.h"
#include "nx/nx.h"
#include "foundation/error.h"
#include "metadata/ifc_metadata.h"
#include <stdlib.h>
#include "nu/strsafe.h"

M3ULoader::M3ULoader()
{
	filename=0;
}

M3ULoader::~M3ULoader()
{
	NXStringRelease(filename);
}

/* reads one line from a file.  appended newline (\r and \n) characters are trimmed */
static int Internal_ReadLine(FILE *fp, char *linebuf, size_t linebuf_len)
{
	long start_position = ftell(fp);
	fgets(linebuf, (int)linebuf_len, fp);
	long delta = ftell(fp) - start_position;

	if (delta == 0)
		return NErr_Error;

	/* trim trailing newline characters */
	char *end_trim = linebuf + delta - 1; /* safe because we checked for delta == 0 */
	while (end_trim != linebuf && (*end_trim == '\n' || *end_trim == '\r'))
		*end_trim-- = 0;
	
	return NErr_Success;
}

int M3ULoader::PlaylistLoader_Load(nx_uri_t filename, cb_playlistloader *playlist)
{
	M3UFileInfo file_info;
	int ret;
	FILE *fp;
	bool ext=false;
	char *p;
	char linebuf[2048];

	/* try to open the file */	
	fp = NXFile_fopen(filename, nx_file_FILE_read_binary);
	if (!fp)
		return NErr_FileNotFound;

	/* not sure what the purpose of this is, but this was in the winamp 2 code */
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size == -1)
	{
		fclose(fp);
		fp = 0;
		return NErr_Error;
	}

	/* if it was a .m3u8 file, mark it as utf8, always */
	if (NXPathMatchExtension(filename, extension_m3u8))
		file_info.SetUTF8(true);
	
	/* look for a UTF-8 BOM */
	unsigned char BOM[3] = {0, 0, 0};
	if (fread(BOM, 1, 3, fp) == 3 && BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
		file_info.SetUTF8(true);
	else
		fseek(fp, 0, SEEK_SET);
	
	while (1)
	{
		if (feof(fp)) 
			break;

		/* read one line */
		if (Internal_ReadLine(fp, linebuf, sizeof(linebuf)) != NErr_Success)
			continue;
		
		p = linebuf;

		/* eat leading whitespace and tabs */
		while (*p == ' ' || *p == '\t')
			p++;

		if (!strncmp(p, "#EXTM3U", 7))
		{
			ext = true;
			continue;
		}
		if (!strncmp(p, "#UTF8", 5))
		{
			file_info.SetUTF8(true);
			continue;
		}

		if (*p != '#' && *p != '\n' && *p != '\r' && *p)
		{
			if (!strncmp(p, "ASF ", 4) && strlen(p) > 4)
				p += 4;

			file_info.SetFilename(p);
			ret = playlist->OnFile(&file_info);
			if (ret != NErr_Success)
				break;
			file_info.Reset();
		}
		else if (ext && !strncmp(p, "#EXTINF:", 8))
		{
			p += 8;
			file_info.SetLength(strtol(p, &p, 10));

			if (*p)
			{
				file_info.SetTitle(p+1);
			}
		}
	}
	fclose(fp);

	return NErr_Success;
}
