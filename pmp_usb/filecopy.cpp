#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "../Agave/Language/api_language.h"
#include "../gen_ml/ml.h"
#include "../ml_pmp/pmp.h"
#include "resource.h"

extern PMPDevicePlugin plugin;

typedef struct CopyData {
  void * callbackContext;
  void (*callback)(void * callbackContext, wchar_t * status);
  int * killswitch;
} CopyData;

static int Win98Copy(wchar_t * infile, wchar_t * outfile, CopyData * inst) {
  FILE * in = _wfopen(infile,L"rb");
  if(!in) {
	inst->callback(inst->callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_FILE));
    return -1;
  }
  
  fseek(in,0,2);
  int len = ftell(in);
  fseek(in,0,0);

  FILE * out = _wfopen(outfile,L"wb");
  if(!out) {
    inst->callback(inst->callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_CREATE_FILE));
    fclose(in);
    return -1;
  }
  wchar_t status[100];
  char buf[65536];
  unsigned int l;
  int pos=0;
  int i=0;
  do {
    if(i == 0) { // do callback
      i=5;
	  wsprintf(status,WASABI_API_LNGSTRINGW(IDS_TRANSFERING_PERCENT), pos / (len / 100));
      inst->callback(inst->callbackContext,status);
    }
    i--;
    l=fread(buf,1,sizeof(buf),in);
    pos += l;
  } while(l>0 && fwrite(buf,1,l,out) == l && !*inst->killswitch);

  fclose(out);
  fclose(in);  

  if(*inst->killswitch) { //cancelled!
    inst->callback(inst->callbackContext,WASABI_API_LNGSTRINGW(IDS_CANCELLED));
    _wunlink(outfile);
    return -1;
  }
  inst->callback(inst->callbackContext,WASABI_API_LNGSTRINGW((pos==len?IDS_DONE:IDS_TRANSFER_FAILED)));
  return pos==len?0:-1;
}
/*
typedef DWORD (WINAPI *LPPROGRESS_ROUTINE)(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData OPTIONAL
    );

typedef BOOL (WINAPI*COPYFILEEXTYPE)(LPCTSTR lpExistingFileName,
                              LPCTSTR lpNewFileName,
                              LPPROGRESS_ROUTINE lpProgressRoutine,
                              LPVOID lpData,
                              LPBOOL pbCancel,
                              DWORD dwCopyFlags);


#define PROGRESS_CONTINUE   0
#define PROGRESS_CANCEL     1
#define PROGRESS_STOP       2
#define PROGRESS_QUIET      3

DWORD CALLBACK CopyToIpodProgressRoutine(
  LARGE_INTEGER TotalFileSize,
  LARGE_INTEGER TotalBytesTransferred,
  LARGE_INTEGER StreamSize,
  LARGE_INTEGER StreamBytesTransferred,
  DWORD dwStreamNumber,
  DWORD dwCallbackReason,
  HANDLE hSourceFile,
  HANDLE hDestinationFile,
  LPVOID lpData)
{
  return PROGRESS_CONTINUE; 
}
*/
int CopyFile(wchar_t * infile, wchar_t * outfile, void * callbackContext, void (*callback)(void * callbackContext, wchar_t * status), int * killswitch) {
  CopyData c;
  c.callback = callback;
  c.callbackContext = callbackContext;
  c.killswitch = killswitch;
  return Win98Copy(infile,outfile,&c);
  
  /*
  HMODULE hmod = LoadLibrary("kernel32.dll");
  COPYFILEEXTYPE CopyFileEx;
  CopyFileEx=(COPYFILEEXTYPE)GetProcAddress(hmod, "CopyFileExW");
  bool ret;
  if(!CopyFileEx)  
    ret = Win98Copy(infile,outfile);
  else {
    ret = CopyFileEx(p->ipod->g_pcfile,p->ipod_fullfile,CopyToIpodProgressRoutine,lpParam,NULL,NULL);
  FreeLibraryAndExitThread(hmod,0x0);
  return ret;
  */
}