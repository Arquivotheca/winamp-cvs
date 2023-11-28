#include "FileLock.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

FileLock::_FileLockData::_FileLockData() : Wasabi2::Dispatchable(0)
{
	filename=0;
	callback=0;
	sem=0;
	reason=0;
	semaphore_triggered=false;
}

FileLock::_FileLockData::~_FileLockData()
{
	NXURIRelease(filename);
	if (callback)
		callback->Release();

	NXSemaphoreClose(sem);
}

nx_uri_t FileLock::_FileLockData::GetFilename() const
{
	return filename;
}

int FileLock::_FileLockData::Set(nx_uri_t _filename, cb_filelock *_callback, int _reason)
{
	int ret = NXSemaphoreCreate(&sem);
	if (ret != NErr_Success)
	{
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] Failed to create semaphore for %s", _filename->string);
#endif
		return ret;
	}

	filename = NXURIRetain(_filename);
	if (_callback)
	{
		callback = _callback;
		callback->Retain();
	}

	reason = _reason;

	return NErr_Success;
}

void FileLock::_FileLockData::Wait()
{
	NXSemaphoreWait(sem);
}

bool FileLock::_FileLockData::IsWriter() const
{
	return reason == REASON_WRITE;
}

void FileLock::_FileLockData::Trigger()
{
	if (!semaphore_triggered)
		NXSemaphoreRelease(sem);
	semaphore_triggered=true;
}

void FileLock::_FileLockData::Interrupt()
{
	if (callback)
		callback->Interrupt();
}

/* --------------- */
static int 
	FileLock_IsFileNameEqual(nx_uri_t filename1, nx_uri_t filename2)
{
	if (filename1 == filename2)
		return NErr_True;

	if (NULL == filename1 || NULL == filename2)
		return NErr_BadParameter;

	int err;
	nx_string_t string1, string2;

	err = NXURIGetNXString(&string1, filename1);
	if (NErr_Success != err)
		return err;

	err = NXURIGetNXString(&string2, filename2);
	if (NErr_Success != err)
	{
		NXStringRelease(string1);
		return err;
	}

	if (nx_compare_equal_to == NXStringCompare(string1, string2, 
		nx_compare_case_insensitive))
	{
		err = NErr_True;
	}
	else
	{
		err = NErr_False;
	}

	NXStringRelease(string1);
	NXStringRelease(string2);

	return err;
}

FileLock::FileLock()
{
	nx_thread_t t;
	NXThreadCreate(&t, _FileLockThreadProcedure, this);
}

int FileLock::Find(FileLockData **out_data, nx_uri_t filename)
{
	/* now see if there's anyone else we need to wait on */
	for (FileLockList::iterator itr=file_lock_list.begin();itr!=file_lock_list.end();itr++)
	{
		FileLockData *data = *itr;
		if (NErr_True == FileLock_IsFileNameEqual(data->GetFilename(), filename))
		{
			*out_data = data;
			return NErr_True;
		}
	}

	return NErr_False;
}

int FileLock::FileLock_WaitForRead(nx_uri_t filename)
{
	/* add ourselves to the end of the list first */
	FileLockData *added = new (std::nothrow)FileLockData;
	if (!added)
		return NErr_OutOfMemory;

	int ret = added->Set(filename, 0, REASON_READ);
	if (ret != NErr_Success)
	{
		delete added;
		return ret;
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		added->Retain();
		apc->func = APC_RegisterWait;
		apc->param1 = this;
		apc->param2 = added;
		thread_loop.Schedule(apc);
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] START WaitForRead for %s", filename->string);
#endif
		added->Wait();
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] DONE  WaitForRead for %s", filename->string);
#endif
		added->Release();
		return NErr_Success;
	}
	else 
	{
		delete added;
		return NErr_OutOfMemory;
	}
}

int FileLock::FileLock_WaitForReadInterruptable(nx_uri_t filename, cb_filelock *callback)
{
	/* add ourselves to the end of the list first */
	FileLockData *added = new (std::nothrow)FileLockData;
	if (!added)
		return NErr_OutOfMemory;

	int ret = added->Set(filename, callback, REASON_READ);
	if (ret != NErr_Success)
	{
		delete added;
		return ret;
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		added->Retain();
		apc->func = APC_RegisterWait;
		apc->param1 = this;
		apc->param2 = added;
		thread_loop.Schedule(apc);
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] START Wait for %s", filename->string);
#endif
		added->Wait();
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] DONE  Wait for %s", filename->string);
#endif
		added->Release();
		return NErr_Success;
	}
	else 
	{
		delete added;
		return NErr_OutOfMemory;
	}
}

int FileLock::FileLock_WaitForWrite(nx_uri_t filename)
{
	/* add ourselves to the end of the list first */
	FileLockData *added = new (std::nothrow)FileLockData;
	if (!added)
		return NErr_OutOfMemory;

	int ret = added->Set(filename, 0, REASON_WRITE);
	if (ret != NErr_Success)
	{
		delete added;
		return ret;
	}

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		added->Retain();
		apc->func = APC_RegisterInterrupt;
		apc->param1 = this;
		apc->param2 = added;
		thread_loop.Schedule(apc);
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] START Inputter-Wait for %s", filename->string);
#endif
		added->Wait();
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] DONE  Inputter-Wait for %s", filename->string);
#endif
		added->Release();
		return NErr_Success;
	}
	else 
	{
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] Failed to create threadloop_node_t for %s", filename->string);
#endif
		delete added;
		return NErr_OutOfMemory;
	}
}

int FileLock::FileLock_WaitForWriteInterruptable(nx_uri_t filename, cb_filelock *callback)
{
	/* add ourselves to the end of the list first */
	FileLockData *added = new (std::nothrow)FileLockData;
	if (!added)
		return NErr_OutOfMemory;

	int ret = added->Set(filename, callback, REASON_WRITE);
	if (ret != NErr_Success)
	{
		delete added;
		return ret;
	}


	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		added->Retain();
		apc->func = APC_RegisterInterrupt;
		apc->param1 = this;
		apc->param2 = added;
		thread_loop.Schedule(apc);
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] START Inputter-Wait for %s", filename->string);
#endif
		added->Wait();
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] DONE  Inputter-Wait for %s", filename->string);
#endif
		added->Release();
		return NErr_Success;
	}
	else 
	{
#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] Failed to create threadloop_node_t for %s", filename->string);
#endif
		delete added;
		return NErr_OutOfMemory;
	}
}

int FileLock::FileLock_UnlockFile(nx_uri_t filename)
{
	threadloop_node_t *apc = thread_loop.GetAPC();
	if (apc)
	{
		apc->func = APC_UnlockFile;
		apc->param1 = this;
		apc->param2 = (void *)NXURIRetain(filename);
		thread_loop.Schedule(apc);
		return NErr_Success;
	}
	else 
	{
		return NErr_OutOfMemory;
	}
}

nx_thread_return_t NXTHREADCALL FileLock::FileLockThreadProcedure()
{
	thread_loop.Run();
	return 0;
}

nx_thread_return_t NXTHREADCALL FileLock::_FileLockThreadProcedure(nx_thread_parameter_t parameter)
{
	FileLock *file_lock = (FileLock *)parameter;
	return file_lock->FileLockThreadProcedure();
}

void FileLock::APC_RegisterWait(void *_file_lock, void *_file_lock_data, double unused)
{
	FileLock *file_lock = (FileLock *)_file_lock;
	FileLockData *file_lock_data = (FileLockData *)_file_lock_data;
	file_lock->file_lock_list.push_back(file_lock_data);

	/* Trigger all readers from the front of the list */
	for (FileLockList::iterator itr=file_lock->file_lock_list.begin();itr!=file_lock->file_lock_list.end();itr++)
	{
		FileLockData *data = *itr;
		if (NErr_True == FileLock_IsFileNameEqual(data->GetFilename(), file_lock_data->GetFilename()))		
		{
			if (data->IsWriter())
				break;

			data->Trigger();
		}
	}
}

void FileLock::APC_RegisterInterrupt(void *_file_lock, void *_file_lock_data, double unused)
{
	FileLock *file_lock = (FileLock *)_file_lock;
	FileLockData *file_lock_data = (FileLockData *)_file_lock_data;
	file_lock->file_lock_list.push_back(file_lock_data);

	bool first=true;

	/* Interrupt all readers in the list.  Trigger ourself if we're first on the list */
	for (FileLockList::iterator itr=file_lock->file_lock_list.begin();itr!=file_lock->file_lock_list.end();itr++)
	{
		FileLockData *data = *itr;
		if (NErr_True == FileLock_IsFileNameEqual(data->GetFilename(), file_lock_data->GetFilename()))		
		{
			if (data->IsWriter())
			{
				if (first && data == file_lock_data) /* we're first on the list */
				{
					data->Trigger();
				}
			}
			else
			{
				data->Interrupt();
			}
			first=false;
		}
	}
}

void FileLock::APC_UnlockFile(void *_file_lock, void *_filename, double unused)
{
	FileLock *file_lock = (FileLock *)_file_lock;
	nx_uri_t filename = (nx_uri_t)_filename;

#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileLock] Unlocking %s", filename->string);
#endif

	/* remove one from the list */
	FileLockData *locked=0;
	file_lock->Find(&locked, filename);

	if (locked)
	{
		locked->Trigger();
		file_lock->file_lock_list.erase(locked);
		locked->Release();		

		/* and signal the next */
		bool first=true;
		/* If the first item is a writer, we just trigger it.  Otherwise trigger all readers from the front of the list */
		for (FileLockList::iterator itr=file_lock->file_lock_list.begin();itr!=file_lock->file_lock_list.end();itr++)
		{
			FileLockData *data = *itr;
			if (NErr_True == FileLock_IsFileNameEqual(data->GetFilename(), filename))	
			{
				if (data->IsWriter())
				{
					if (first) /* we're first on the list */
					{
						data->Trigger();
					}
					break;
				}
				first=false;

				data->Trigger();
			}
		}
	}

	NXURIRelease(filename);	
}
