typedef WReader* (*T_RF_create)();
typedef int (_stdcall *T_gzip_writefile)(const char* path,const void* buf,DWORD size);
extern T_gzip_writefile gzip_writefile;
void InitReadFile();
