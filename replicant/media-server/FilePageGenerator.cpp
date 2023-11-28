#include "FilePageGenerator.h"

#if defined(_WIN32) && !defined(strncasecmp)
#define strncasecmp _strnicmp
#endif

FilePageGenerator::FilePageGenerator()
{
	fp=0;
}

FilePageGenerator::~FilePageGenerator()
{
	NXFileRelease(fp);
}

int FilePageGenerator::Initialize(nx_uri_t filename, jnl_http_request_t serv)
{
	int ret = NXFileOpenFile(&fp, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;	

	uint64_t resume_end=0;
	uint64_t resume_pos=0;

	uint64_t file_total_length;
	NXFileLength(fp, &file_total_length);

	// check range request
	const char *range = jnl_http_request_get_header(serv, "Range");
	if (range)
	{
		if (!strncasecmp(range,"bytes=",6))
		{
			range+=6;
			const char *t=range;
			while ((*t < '0' || *t  > '9') && *t) t++;
			while (*t >= '0' && *t <= '9') 
			{
				resume_pos*=10;
				resume_pos+=*t-'0';
				t++;
			}
			if (*t != '-') 
			{
				resume_end=file_total_length;
			}
			else if (t[1])
			{
				while (*t >= '0' && *t <= '9') 
				{
					resume_end*=10;
					resume_end+=*t-'0';
					t++;
				}
			}
			else
			{
				resume_end=file_total_length;
			}
		}
		NXFileLockRegion(fp, resume_pos, resume_end);
		NXFileSeek(fp, 0);
	}
	
	char buf[512];

	// how many bytes we're actually going to send	
	uint64_t file_length;
	NXFileLength(fp, &file_length);
  sprintf(buf, "Content-Length: %llu", file_length);
	jnl_http_request_addheader(serv, buf);

	// the range request (if there is one)
	if (resume_pos || resume_end != file_total_length)
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 206 Partial Content");
		sprintf(buf, "Content-Range: bytes=%llu-%llu/%llu", resume_pos, resume_end, file_total_length);
		jnl_http_request_addheader(serv, buf);
	}
	else
	{
		jnl_http_request_set_reply_string(serv, "HTTP/1.1 200 OK");
	}

	return NErr_Success;
}

size_t FilePageGenerator::PageGenerator_GetData(void *buf, size_t size)
{
	size_t bytes_read=0;
	NXFileRead(fp, buf, size, &bytes_read);
	return bytes_read;
}