#include "filelock/api_filelock.h"
#include "nx/nxsemaphore.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/ThreadLoop.h"
#include "nx/nxthread.h"
#include "nu/PtrDeque.h"
#include "nu/ThreadLoop.h"
#include "nswasabi/ServiceName.h"

class FileLock : public api_filelock
{
public:
	FileLock();
	WASABI_SERVICE_NAME("File Locking Service");
private:
	int WASABICALL FileLock_WaitForRead(nx_uri_t filename);
	int WASABICALL FileLock_WaitForReadInterruptable(nx_uri_t filename, cb_filelock *callback);
	int WASABICALL FileLock_WaitForWrite(nx_uri_t filename);
	int WASABICALL FileLock_WaitForWriteInterruptable(nx_uri_t filename, cb_filelock *callback);
	int WASABICALL FileLock_UnlockFile(nx_uri_t filename);
	
	enum
	{
		REASON_READ=0,
		REASON_WRITE=1,
	};
	class _FileLockData : public nu::PtrDequeNode, public Wasabi2::Dispatchable
	{
	public:
		_FileLockData();
		~_FileLockData();
		nx_uri_t GetFilename() const;
		int Set(nx_uri_t filename, cb_filelock *callback, int reason);
		void Wait();
		bool IsWriter() const;
		void Trigger();
		void Interrupt();
	private:
		nx_uri_t filename;
		cb_filelock *callback;
		nx_semaphore_t sem;
		bool semaphore_triggered;
		int reason;
	};
	typedef ReferenceCounted<_FileLockData> FileLockData;
	typedef nu::PtrDeque<FileLockData> FileLockList;
	FileLockList file_lock_list;
	
	int Find(FileLockData **data, nx_uri_t filename);
	
	ThreadLoop thread_loop;
	nx_thread_return_t NXTHREADCALL FileLockThreadProcedure();
	static nx_thread_return_t NXTHREADCALL _FileLockThreadProcedure(nx_thread_parameter_t parameter);

	static void APC_RegisterWait(void *_file_lock, void *_file_lock_data, double unused);
	static void APC_RegisterInterrupt(void *_file_lock, void *_file_lock_data, double unused);
	static void APC_UnlockFile(void *_file_lock, void *_file_lock_data, double unused);
};