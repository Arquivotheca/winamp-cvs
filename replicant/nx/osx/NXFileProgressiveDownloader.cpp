#include "NXFileObject.h"
#include "nu/ProgressTracker.h"
#include "nx/nxthread.h"
#include "nx/nxsleep.h"
#include "jnetlib/jnetlib.h"
#include "nswasabi/AutoCharNX.h"
#include <time.h>
#include <new>

#define HTTP_BUFFER_SIZE 65536



class NXFileObject_ProgressiveDownloader: public NXFileObject
{
public:
	NXFileObject_ProgressiveDownloader();
	~NXFileObject_ProgressiveDownloader();
	ns_error_t Initialize(nx_uri_t uri, jnl_http_t http);

private:
	FILE *progressive_file_read, *progressive_file_write;

	ns_error_t Read(void *buffer, size_t bytes_requested, size_t *bytes_read);
	ns_error_t Write(const void *buffer, size_t bytes);
	ns_error_t Seek(uint64_t position);
	ns_error_t Tell(uint64_t *position);
	ns_error_t PeekByte(uint8_t *byte);
	ns_error_t Sync();
	ns_error_t Truncate();

private:
	static nx_thread_return_t NXTHREADCALL ProgressiveThread(nx_thread_parameter_t param);
	void Buffer();
	void OnFinish();
	bool WaitForPosition(uint64_t position, uint64_t size);
	void Internal_Write(const void *data, size_t data_len);
	void Internal_Reconnect(uint64_t position, uint64_t end);
	ns_error_t SetupConnection(uint64_t start_position, uint64_t end_position);
	int Wait(int milliseconds);
	int DoRead(void *buffer, size_t bufferlen);
	int Connect();
	bool Full();

	bool end_of_file;
	volatile bool stream_disconnected;
	volatile bool connected;
	nx_uri_t temp_uri;
	ProgressTracker progress_tracker;
	nx_thread_t download_thread;
	volatile int thread_abort, killswitch;
	jnl_http_t http;
};

NXFileObject_ProgressiveDownloader::NXFileObject_ProgressiveDownloader()
{
	progressive_file_read=0;
	progressive_file_write=0;

	end_of_file=false;
	stream_disconnected=false;
	connected=false;
	temp_uri=0;
	download_thread=0;
	thread_abort=0;
	http=0;
	killswitch=0;
}

NXFileObject_ProgressiveDownloader::~NXFileObject_ProgressiveDownloader()
{	
	thread_abort = 1;
	while (!stream_disconnected && !killswitch)
	{
		NXSleep(55);
	}

	if (progressive_file_read)
	fclose(progressive_file_read);
	if (progressive_file_write)
	fclose(progressive_file_write);
	if (temp_uri)
	{
		NXFile_unlink(temp_uri);
		NXURIRelease(temp_uri);
	}
}

ns_error_t NXFileObject_ProgressiveDownloader::Initialize(nx_uri_t uri, jnl_http_t http)
{
	ns_error_t ret = NXURICreateTemp(&temp_uri);
	if (ret != NErr_Success)
		return ret;

	this->uri = NXURIRetain(uri);
	this->http = jnl_http_retain(http);

	if (!http)
	{
		ret = SetupConnection(0, (uint64_t)-1);
		if (ret != NErr_Success)
		{
			NXURIRelease(temp_uri);
			return ret;
		}
	}

	progressive_file_write = NXFile_fopen(temp_uri, nx_file_FILE_readwrite_binary);
	progressive_file_read = NXFile_fopen(temp_uri, nx_file_FILE_read_binary);

	NXThreadCreate(&download_thread, ProgressiveThread, this);

	while (!connected && !stream_disconnected && !killswitch)
	{
		NXSleep(55);
	}

	return NErr_Success;
}

bool NXFileObject_ProgressiveDownloader::Full()
{
	if (progress_tracker.Valid(0, file_stats.file_size))
	{
		fclose(progressive_file_write);
		progressive_file_write=0;
		return true;
	}
	else
		return false;

}

void NXFileObject_ProgressiveDownloader::Buffer()
{
	NXSleep(10);
#if 0
	for (int i=0;i<101;i++)
	{
		NXSleep(55);
		if (killswitch)
			break;
	}
#endif
}

void NXFileObject_ProgressiveDownloader::OnFinish()
{
	stream_disconnected=true;
}

bool NXFileObject_ProgressiveDownloader::WaitForPosition(uint64_t position, uint64_t size)
{
	do
	{
		bool valid = progress_tracker.Valid(position, position+size);
		if (valid)
			return true;
		else
		{
			if (position < this->position)
			{
				Internal_Reconnect(position, position+size);
			}
			else
			{
				Buffer();
			}
		}

	} while (!killswitch);
	return false;
}

ns_error_t NXFileObject_ProgressiveDownloader::Read(void *buffer, size_t bytes_requested, size_t *bytes_read)
{
	if (end_of_file)
		return NErr_EndOfFile;

	if (WaitForPosition(this->position, (uint64_t)bytes_requested) == false)
	{
		*bytes_read = 0;
		return NErr_Error;
	}

	size_t r = fread(buffer, 1, bytes_requested, progressive_file_read);
	this->position += r;
	*bytes_read = r;
	return NErr_Success;		
}

void NXFileObject_ProgressiveDownloader::Internal_Reconnect(uint64_t position, uint64_t end)
{
	if (download_thread)
	{
		thread_abort=1;
		nx_thread_return_t thread_ret;
		NXThreadJoin(download_thread, &thread_ret);
		thread_abort=0;
		download_thread=0;
	}

	uint64_t new_start, new_end;
	progress_tracker.Seek(position, end, &new_start, &new_end);

	stream_disconnected=false;
	connected=false;

	SetupConnection(new_start, new_end);
	fseeko(progressive_file_write, new_start, SEEK_SET);

	NXThreadCreate(&download_thread, ProgressiveThread, this);

	while (!connected && !stream_disconnected && !killswitch)
	{
		NXSleep(55); 
	}

	Buffer();
}

ns_error_t NXFileObject_ProgressiveDownloader::SetupConnection(uint64_t start_position, uint64_t end_position)
{
	if (!http)
		http = jnl_http_create(/*(jnl_dns_t)-1,*/ HTTP_BUFFER_SIZE, 0/*, 0 TODO: proxy */);

	if (!http)
		return NErr_FailedCreate;

	jnl_http_reset_headers(http);

	if (start_position && start_position != (uint64_t)-1)
	{
		if (end_position == (uint64_t)-1)
		{
			char temp[128];
			sprintf(temp, "Range: bytes=%llu-", start_position);
			jnl_http_addheader(http, temp);
		}
		else
		{
			char temp[128];
			sprintf(temp, "Range: bytes=%llu-%llu", start_position, end_position);
			jnl_http_addheader(http, temp);
		}
	}

	// TODO: SetUserAgent(http);
	jnl_http_addheader(http, "Connection: Close"); // TODO: change if we ever want a persistent connection and downloading in chunks
	jnl_http_connect(http, AutoCharUTF8(uri), 1, "GET");

	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::Seek(uint64_t new_position)
{
	if (new_position >= (region.end - region.start))
	{
		this->position = region.end - region.start;
		end_of_file=true;
	}
	else
	{
		if (!progress_tracker.Valid(new_position, new_position))
		{
			Internal_Reconnect(new_position, (uint64_t)-1);
		}
		fseeko(progressive_file_read, new_position, SEEK_SET);
		this->position = new_position;
		end_of_file=false;
	}
	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::Tell(uint64_t *position)
{
	if (end_of_file)
		*position = region.end - region.start;
	else
		*position = this->position - region.start;
	return NErr_Success;
}


/* API used by download thread */
void NXFileObject_ProgressiveDownloader::Internal_Write(const void *data, size_t data_len)
{
	size_t bytes_written = fwrite(data, 1, data_len, progressive_file_write);
	fflush(progressive_file_write);
	progress_tracker.Write(bytes_written);
}

int NXFileObject_ProgressiveDownloader::Wait(int milliseconds)
{
	// TODO:
	if (thread_abort)
		return 1;
	if (killswitch)
		return -1;

	NXSleep(milliseconds);
	return 0;		
}

int NXFileObject_ProgressiveDownloader::DoRead(void *buffer, size_t bufferlen)
{
	int ret = jnl_http_run(http);
	size_t bytes_received;
	do
	{
		ret = jnl_http_run(http);
		bytes_received = jnl_http_get_bytes(http, buffer, bufferlen);
		if (bytes_received)
			Internal_Write(buffer, bytes_received);
	} while (bytes_received);
	return ret;
}

int NXFileObject_ProgressiveDownloader::Connect()
{
	// TODO: configurable timeout
		/* wait for connection */
	time_t start_time = time(0);

	int http_status = jnl_http_get_status(http);
	while (http_status == HTTPGET_STATUS_CONNECTING || http_status == HTTPGET_STATUS_READING_HEADERS)
	{
		if (Wait(55) != 0)
			return NErr_Interrupted;

		int ret = jnl_http_run(http);
		if (ret == HTTPGET_RUN_ERROR)
			return NErr_ConnectionFailed;
		if (start_time + 15 < time(0))
			return NErr_TimedOut;

		http_status = jnl_http_get_status(http);
	} 
	
	if (http_status == HTTPGET_STATUS_ERROR)
	{
		switch(jnl_http_getreplycode(http))
		{
		case 400:
			return NErr_BadRequest;
		case 401:
			// TODO: deal with this specially
			return NErr_Unauthorized;
		case 403:
			// TODO: deal with this specially?
			return NErr_Forbidden;
		case 404:
			return NErr_NotFound;
		case 405:
			return NErr_BadMethod;
		case 406:
			return NErr_NotAcceptable;
		case 407:
			// TODO: deal with this specially
			return NErr_ProxyAuthenticationRequired;
		case 408:
			return NErr_RequestTimeout;
		case 409:
			return NErr_Conflict;
		case 410:
			return NErr_Gone;
		case 500:
			return NErr_InternalServerError;
		case 503:
			return NErr_ServiceUnavailable;

		default:
					return NErr_ConnectionFailed;
			}
	}
	else
	{
		if (!file_stats.file_size)
		{
			file_stats.file_size = jnl_http_content_length(http);// TODO interlock
			region.end=file_stats.file_size;
		}
			connected=true;

		return NErr_Success;
	}

	
}

ns_error_t NXFileObject_ProgressiveDownloader::PeekByte(uint8_t *byte)
{
	if (position == region.end)
		return NErr_EndOfFile;

	// make sure we have enough room
	if (WaitForPosition(this->position, (uint64_t)1) == false)
		return NErr_Error;

	int read_byte = fgetc(progressive_file_read);
	if (read_byte != EOF)
		ungetc(read_byte, progressive_file_read);
	else
		return NErr_EndOfFile;

	*byte = (uint8_t)read_byte;
	return NErr_Success;
}

ns_error_t NXFileObject_ProgressiveDownloader::Sync()
{
	return NErr_NotImplemented;
}

ns_error_t NXFileObject_ProgressiveDownloader::Truncate()
{
	return NErr_NotImplemented;
}

ns_error_t NXFileObject_ProgressiveDownloader::Write(const void *buffer, size_t bytes)
{
	return NErr_NotImplemented;
}

nx_thread_return_t NXFileObject_ProgressiveDownloader::ProgressiveThread(nx_thread_parameter_t param)
{
	NXFileObject_ProgressiveDownloader *reader = (NXFileObject_ProgressiveDownloader *)param;
	char buffer[HTTP_BUFFER_SIZE];

	if (reader->Connect() == NErr_Success)
	{
		int ret = 0;
		while (ret == 0)
		{
			if (reader->DoRead(buffer, sizeof(buffer)) == HTTPGET_RUN_CONNECTION_CLOSED)
			{
				if (reader->Full())
				{
					break;
				}
			}
			
			ret=reader->Wait(10);			
		}
	}
	reader->OnFinish();

	return 0;
}

ns_error_t NXFileOpenProgressiveDownloader(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags, jnl_http_t http)
{
	NXFileObject_ProgressiveDownloader *file_object = new (std::nothrow) NXFileObject_ProgressiveDownloader;
	if (!file_object)
		return NErr_OutOfMemory;

	ns_error_t ret = file_object->Initialize(filename, http);
	if (ret != NErr_Success)
	{
		delete file_object;
		return ret;
	}

	*out_file = (nx_file_t)file_object;
	return NErr_Success;
}
