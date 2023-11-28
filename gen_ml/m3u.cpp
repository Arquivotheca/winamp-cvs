#include <windows.h>
#include <stdio.h>
#include "playlist.h"
#include "../winamp/wa_ipc.h"
#include "PLSWriter.h"
#include "M3UWriter.h"

int loadplsfn(char *fn, int whattodo) 
{
  char fnbuf[MAX_PATH];
  char tmp[MAX_PATH];
  int i=0;
  int ext=0;
  int oldpos = -10;
  char fieldbuf[100], *p;
  int x, numfiles;
  int list_size=PlayList_getlength();

  if (strstr(fn,"://"))
	{
		int rval=1;
    extern HWND g_hwnd;
    char TEMP_FILE[MAX_PATH*2];
    char tmppath[1024];
    GetTempPath(1024,tmppath);
    GetTempFileName(tmppath,"pls",0,TEMP_FILE);
		int (*httpRetrieveFile)(HWND hwnd, char *url, char *file, char *dlgtitle);
		*(void**)&httpRetrieveFile = (void*)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETHTTPGETTER);
		if (httpRetrieveFile && !httpRetrieveFile(g_hwnd,fn,TEMP_FILE,"Fetching Playlist"))
		{
			rval = loadplsfn(TEMP_FILE,whattodo);
		}
		DeleteFile(TEMP_FILE);
		return rval;
	}
  
  GetFullPathName(fn,MAX_PATH-1,tmp,&p);
  *(p-1)=0;

  numfiles = GetPrivateProfileInt("playlist","NumberOfEntries",-1,fn);
  ext=GetPrivateProfileInt("playlist","Version",1,fn)>1?1:0;
  if (numfiles <= 0) return 1;
  if (list_size) 
	{
		if (whattodo < 1)
		{
			PlayList_delete();
			i=1;
		}
	}

  for (x = 1; x <= numfiles; x ++) {
    int flen=-1;
    char ftitle[MAX_PATH]="";
		wsprintf(fieldbuf,"File%d",x);
		GetPrivateProfileString("playlist",fieldbuf,"",fnbuf,MAX_PATH,fn);
    if (ext)
    {
		  wsprintf(fieldbuf,"Title%d",x);
		  GetPrivateProfileString("playlist",fieldbuf,"",ftitle,MAX_PATH,fn);
		  wsprintf(fieldbuf,"Length%d",x);
      flen=GetPrivateProfileInt("playlist",fieldbuf,-1,fn);
    }
    if (*fnbuf) {
      char *p;
      char buf[512];
      p=fnbuf;

      if (strncmp(p,"\\\\",2) && strncmp(p+1,":\\",2) && !strstr(p,":/"))
      {
        if (p[0] == '\\') 
        {
          buf[0]=tmp[0];
          buf[1]=tmp[1];
          strncpy(buf+2,p,510);
          buf[511]=0;
        }
        else if (tmp[strlen(tmp)-1]!='\\') wsprintf(buf,"%s\\%s",tmp,p);
        else wsprintf(buf,"%s%s",tmp,p);
        p=buf;
      }

      if (ftitle[0])
        PlayList_append_withinfo(p,ftitle,flen);
  		else PlayList_append(p);
		}

	}
  return 0;
}

int loadm3ufn(char *filename, int whattodo) 
{ // -1 prompt, 0 replace, 1 add
	FILE *fp;
	char linebuf[2048];
	char tmp[MAX_PATH];
  char ext_title[MAX_PATH]="";
  int ext_len=-1;
	int size;
	char *p;
	int i=0;
  int ext=0;
	if (strstr(filename,"://"))
	{
		int rval=1;
    extern HWND g_hwnd;
    char TEMP_FILE[MAX_PATH*2];
    char tmppath[1024];
    GetTempPath(1024,tmppath);
    GetTempFileName(tmppath,"m3u",0,TEMP_FILE);
		int (*httpRetrieveFile)(HWND hwnd, char *url, char *file, char *dlgtitle);
		*(void**)&httpRetrieveFile = (void*)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETHTTPGETTER);
		if (httpRetrieveFile && !httpRetrieveFile(g_hwnd,filename,TEMP_FILE,"Fetching Playlist"))
		{
			rval = loadm3ufn(TEMP_FILE,whattodo);
		}
		DeleteFile(TEMP_FILE);
		return rval;
	}
	GetFullPathName(filename,MAX_PATH-1,tmp,&p);
	*(p-1)=0;
	fp = fopen(filename,"rt");
	if (!fp) return 1;
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	if (size == -1)
	{
		fclose(fp);
		return 1;
	}

	if (PlayList_getlength()) 
	{
		if (whattodo < 1)
		{
			PlayList_delete();
			i=1;
  	}
	}
	
	while (1)
	{
		if (feof(fp)) break;
    linebuf[0]=0;
		fgets(linebuf,sizeof(linebuf)-1,fp);
    linebuf[sizeof(linebuf)-1]=0;
    if (!linebuf[0]) continue;
    if (!strncmp(linebuf,"#EXTM3U",7))
    {
      ext=1;
      continue;
    }
		p = linebuf;
		while (*p == ' ' || *p == '\t') p = CharNext(p);
		if (*p != '#' && *p != '\n' && *p != '\r' && *p) 
		{
      char buf[512];
			char *p2 = linebuf+strlen(linebuf)-1;
			if (*p2 == '\n') *p2 = 0;
			if (!strncmp(p,"ASF ", 4) && strlen(p) > 4) p+=4;
      {
        if (strncmp(p,"\\\\",2) && strncmp(p+1,":\\",2) && strncmp(p+1,":/",2)&& !strstr(p,"://"))
        {
          if (p[0] == '\\') 
          {
            buf[0]=tmp[0];
            buf[1]=tmp[1];
            strncpy(buf+2,p,510);
            buf[511]=0;
          }
          else if (tmp[strlen(tmp)-1]!='\\') wsprintf(buf,"%s\\%s",tmp,p);
          else wsprintf(buf,"%s%s",tmp,p);
          p=buf;
        }
        if (ext_title[0])
          PlayList_append_withinfo(p,ext_title,ext_len);
				else if (ext) 
        {
          if (!_stricmp(extension(p),"m3u"))
            loadm3ufn(p,1);
          else
            PlayList_append(p);
        }
        else PlayList_appendthing(p);
      }
      ext_len=-1;
      ext_title[0]=0;
		}
    else
    {
      if (ext&&!strncmp(p,"#EXTINF:",8))
      {
        p+=8;
        ext_len=atoi(p);
        while ((*p >= '0' && *p <= '9') || *p == '-') p=CharNext(p);
        if (*p) 
        {
			    char *p2 = p+strlen(p)-1;
			    if (*p2 == '\n') *p2 = 0;
          strncpy(ext_title,CharNext(p),MAX_PATH-1);
          ext_title[MAX_PATH-1]=0;
        }
        else
        {
          ext_len=-1;
          ext_title[0]=0;
        }
      }
      else
      {
        ext_len=-1;
        ext_title[0]=0;
      }
    }
	}
	fclose(fp);
  return 0;
}

void saveplsfn(char *fn, int rel) 
{
	PLSWriter pls;
	pls.Open(fn);

  char fnbuf[MAX_PATH], ftbuf[MAX_PATH*2];
  for (int x=0;x<PlayList_getlength();x++) 
	{
		int len=PlayList_getitem2(x,fnbuf,ftbuf);
		if (rel) 
			PlayList_makerelative(fn,fnbuf);
		pls.SetFilename(fnbuf);
    if (PlayList_getcached(x))
    {
			pls.SetTitle(ftbuf);
			pls.SetLength(len);
    }
		pls.Next();
	}
	pls.Close();
}

int savem3ufn(char *filename, int rel, int ext)
{
	M3UWriter m3u;
	if (!m3u.Open(filename, ext))
		return -1;
  
  char fnbuf[MAX_PATH], ftbuf[MAX_PATH*2];
  for (int x = 0; x < PlayList_getlength(); x++)
  {
    int len=PlayList_getitem2(x,fnbuf,ftbuf);
    if (rel) 
			PlayList_makerelative(filename,fnbuf);
		if (PlayList_getcached(x)) // is the title and length info any good?
			m3u.SetExtended(fnbuf, ftbuf, len);
		else
			m3u.SetFilename(fnbuf);
  }
	m3u.Close();
  return 0;
}
