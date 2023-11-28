#include <windows.h>
#include <stdio.h>
#include "../winamp/wa_ipc.h"

#include "playlist.h"

HRESULT ResolveShortCut(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath);
extern winampGeneralPurposePlugin plugin;


//////////////////////////////
int (*warand)(void);




#define MALLOC_CHUNK 1024


#define CHECK_STUFF()
/* { \
  if (list_pos >= list_size && list_size>0) { MessageBox(NULL,"SHIT2!","a",0); } \
  if (list_size > malloced_size) MessageBox(NULL,"SHIT!","a",0); }
*/
typedef struct pl_entry {
  char filename[MAX_PATH];
  char filetitle[400];
  int length;
  char selected;
  char cached;
  char tmp;
} pl_entry;



static void PlayList_allocmem(int newsize);

static pl_entry *list;
static int list_size, malloced_size;
static pl_entry *tlist;
static int t_len;

void PlayList_loadstate(plstate *newstate)
{
  list=(pl_entry*)newstate->list;
  list_size=newstate->list_size;
  malloced_size=newstate->malloced_size;
  tlist=(pl_entry*)newstate->tlist;
  t_len=newstate->t_len;
}

void PlayList_savestate(plstate *newstate)
{
  newstate->list=(void*)list;
  newstate->list_size=list_size;
  newstate->malloced_size=malloced_size;
  newstate->tlist=(void*)tlist;
  newstate->t_len=t_len;

  list=tlist=0;
  list_size=malloced_size=t_len=0;

}


void PlayList_saveend(int start)
{
	if (tlist) GlobalFree(tlist);
	if (start < list_size)
	{
 		t_len=(list_size-start);
		tlist = (pl_entry *) GlobalAlloc(GMEM_FIXED,sizeof(pl_entry)*t_len);
		memcpy(tlist,list+start,sizeof(pl_entry)*t_len);
		list_size = start;
	}
}

void PlayList_restoreend(void)
{
	if (tlist)
	{
		PlayList_allocmem(list_size+t_len);
		memcpy(list+list_size,tlist,t_len*sizeof(pl_entry));
		list_size += t_len;
		GlobalFree(tlist);
		tlist=NULL;
		t_len=0;
	}
}


int PlayList_getselect(int x)
{
	if (x < 0 || x >= list_size) return 0;
  CHECK_STUFF();
	return list[x].selected;
}

void PlayList_setselect(int x, int sel)
{
	if (x < 0 || x >= list_size) return;
  CHECK_STUFF();
	list[x].selected=sel;
}

int PlayList_getcached(int x)
{
	if (x < 0 || x >= list_size) return 0;
  CHECK_STUFF();
	return list[x].cached;
}

void PlayList_setcached(int x, int cached)
{
	if (x < 0 || x >= list_size) return;
  CHECK_STUFF();
	list[x].cached=cached;
}


int PlayList_gettotallength(void)
{
	int l=0;
	int x;
  CHECK_STUFF();
	for (x = 0; x < list_size; x ++)
	{
		if (list[x].length >= 0)
			l+=list[x].length;
		else return -1;
	}
	return l;
}


static void PlayList_allocmem(int newsize)
{
	int os=malloced_size;
	if (newsize > malloced_size)
		malloced_size = newsize+MALLOC_CHUNK;
	else if (newsize < malloced_size-MALLOC_CHUNK)
		malloced_size = newsize;
	else return;

	if (!malloced_size) 
	{	
		if (list)
		{
			GlobalFree(list); 
			list=0;
		}
	}
	else if (list)
	{
		pl_entry *ol=list;
//		if (malloced_size < os) // removed 10.29.00jf
    list = (pl_entry *)GlobalReAlloc(list,sizeof(pl_entry)*(malloced_size),0);
  //  else list=NULL;
		if (!list) 
		{
			list = (pl_entry *) GlobalAlloc(GMEM_FIXED,sizeof(pl_entry)*(malloced_size));
			memcpy(list,ol,min(os,malloced_size)*sizeof(pl_entry));
			GlobalFree((void*)ol);
		}
	} 
	else 
	{
		list = (pl_entry *) GlobalAlloc(GPTR,sizeof(pl_entry)*(malloced_size));
	}
}


int conf_snipl;

int PlayList_getitem_pl(int position, char *filetitle)
{
 // static int i;
  CHECK_STUFF();
  if (!list || position < 0 || position >= list_size) return 0;
  if (filetitle && conf_snipl) 
  {
	  if (list_size >= 10000) wsprintf(filetitle,"%5d. %s",position+1,list[position].filetitle);
	  else if (list_size >= 1000) wsprintf(filetitle,"%4d. %s",position+1,list[position].filetitle);
	  else if (list_size >= 100) wsprintf(filetitle,"%3d. %s",position+1,list[position].filetitle);
	  else if (list_size >= 10) wsprintf(filetitle,"%2d. %s",position+1,list[position].filetitle);
	  else wsprintf(filetitle,"%d. %s",position+1,list[position].filetitle);	
  }
  else if (filetitle) strcpy(filetitle,list[position].filetitle);
  return list[position].length;
}


int PlayList_getitem2(int position, char *filename, char *filetitle)
{
  if (!list || position < 0 || position >= list_size) return 0;
  CHECK_STUFF();
  if (filename) strcpy(filename,list[position].filename);
  if (filetitle) strcpy(filetitle,list[position].filetitle);
  return list[position].length;
}


int PlayList_getlength(void)
{
  return list_size;
}

//int plmodified=0;

// returns:
// 1, last item in list
// 0 not
// -1 no items in list
int PlayList_deleteitem(int item) { 
  if (!list) return -1;
  CHECK_STUFF();
  if (item < 0 || item >= list_size) return -1;
  list_size--; 
  if (list_size != item) 
	  memcpy(list+item,list+item+1,sizeof(list[0])*(list_size-item));
  PlayList_allocmem(list_size);
	//plmodified=1;
  return list?0:1;
}

void PlayList_delete(void) {
  CHECK_STUFF();
  if (list) GlobalFree(list);
  list=0;
  list_size=0;
  malloced_size=0;
	//plmodified=1;
}


void PlayList_append_withinfo(char *filename, char *title, int length)
{
  char *pp;
  char fn[MAX_PATH];
  CHECK_STUFF();
  lstrcpyn(fn,filename,sizeof(fn)-1);
  fn[MAX_PATH-1]=0;
  if (!strstr(filename,"://") && !strstr(filename,":\\\\"))
  {
	  if (!strstr(fn,"\\") && !GetFullPathName(filename,MAX_PATH-1,fn,&pp)) lstrcpyn(fn,filename,sizeof(fn));
  }
  
  PlayList_allocmem(list_size+1);
  strcpy(list[list_size].filename,fn);
  strncpy(list[list_size].filetitle,title,399);
  list[list_size].length = length;
  list[list_size].selected=0;
  list[list_size].cached=1;
  list_size++;
	//plmodified=1;
}


static void _appendcd(char *url)
{
  int n=0;
  char buf2[32];
  getFileInfo(url,"ntracks",buf2,sizeof(buf2));
  n=atoi(buf2);
  if (n>0 && n < 256)
  {
    int x;
    for (x = 0; x < n; x ++)
    {
      char s[64];
      wsprintf(s,"%s,%d",url,x+1);
      PlayList_append(s);
    }
  }
}

void PlayList_appendthing(char *url)
{
  if (!_stricmp(extension(url),"lnk"))
	{
		char temp2[MAX_PATH];
		if (ResolveShortCut(plugin.hwndParent,url,temp2)) strcpy(url,temp2);
    else return;
	}


  if (!_strnicmp(url,"cda://",6))
  {
    if (strlen(url)==7) _appendcd(url);
    else PlayList_append(url);
  }
  else if (strstr(url,"://"))
  {
    if (!_stricmp(extension(url),"m3u")) 
			  loadm3ufn(url,1);
		else if (!_stricmp(extension(url),"pls")) 
			loadplsfn(url,1);
		else PlayList_append(url);
  }
	else if (!lstrcmp(url+1,":\\"))
	{
		PlayList_adddir(url,1);
	}
  else
  {
    HANDLE h;
    WIN32_FIND_DATA d;

	  h =	FindFirstFile(url,&d);
	  if (h != INVALID_HANDLE_VALUE)
    {
		  FindClose(h);
		  if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			  PlayList_adddir(url,1);
		  else if (!_stricmp(extension(url),"m3u")) 
			  loadm3ufn(url,1);
		  else if (!_stricmp(extension(url),"pls")) 
			  loadplsfn(url,1);
		  else PlayList_append(url);
    }
    else PlayList_append(url);
  }
}



void PlayList_append(char *filename) {
  char *pp;
  basicFileInfoStruct bi;
  char fn[MAX_PATH];
  CHECK_STUFF();
  lstrcpyn(fn,filename,sizeof(fn)-1);
  fn[MAX_PATH-1]=0;
  if (!strstr(filename,"://") && !strstr(filename,":\\\\"))
  {
	  if (!strstr(fn,"\\") && !GetFullPathName(filename,MAX_PATH-1,fn,&pp)) lstrcpyn(fn,filename,sizeof(fn));
	  if (strstr(fn,"~"))
	  {
			char *p;
			HANDLE h;
			WIN32_FIND_DATA d;
			h =	FindFirstFile(fn,&d);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				p=scanstr_back(fn,"\\",fn-1);
	  		strcpy(++p,d.cFileName);
			}
	  }
  }
  
  PlayList_allocmem(list_size+1);
  bi.filename=fn;
  bi.length=-1;
  bi.title=list[list_size].filetitle;
  bi.quickCheck=2;
  bi.titlelen=sizeof(list[list_size].filetitle);
  SendMessage(plugin.hwndParent,WM_WA_IPC,(WPARAM)&bi,IPC_GET_BASIC_FILE_INFO);
  strcpy(list[list_size].filename,fn);
  strncpy(list[list_size].filetitle,bi.title,399);
  list[list_size].length = bi.length;
  list[list_size].selected=0;
  list[list_size].cached=!bi.quickCheck;
  list_size++;
	//plmodified=1;
}



void PlayList_setitem(int x, char *filename, char *filetitle, int len) {
  CHECK_STUFF();
  if (x >= 0 && x < list_size) {
		strcpy(list[x].filename,filename);
		lstrcpyn(list[x].filetitle,filetitle,400);
    list[x].length=len;
    list[x].cached=1;
		//plmodified=1;
  } 
}

void PlayList_swap(int e1, int e2)
{
	pl_entry p;
	if (e1 < 0 || e2 < 0 || e1 >= list_size || e2 >= list_size || e1==e2) return;
	p = list[e1];
	list[e1] = list[e2];
	list[e2] = p;
	//plmodified=1;
}



void PlayList_addfromdlg(char *fns) {
  char buf[MAX_PATH];
  char *path = fns;
  fns += strlen(fns)+1;
	
  while (*fns) {
	  if (*path)
		wsprintf(buf,"%s%s%s",path,path[strlen(path)-1]=='\\'?"":"\\" ,fns);
	  else 
		wsprintf(buf,"%s",fns);
    PlayList_appendthing(buf);
	  fns += strlen(fns)+1;
  }

}


static int PlayList_sortfunc(const void *a, const void *b)
{
	char *a1, *a2, *p;
	a1 = ((pl_entry *)a)->filename;
	a2 = ((pl_entry *)b)->filename;
	p=scanstr_back(a1,"\\",a1-1)+1;
	if (p > a1) a1 = p;
	p=scanstr_back(a2,"\\",a2-1)+1;
	if (p > a2) a2 = p;

	return(_stricmp(a1,a2));
}

static int PlayList_sortfunc2(const void *a, const void *b)
{
	return(_stricmp(((pl_entry *)a)->filetitle,((pl_entry *)b)->filetitle));
}

static int PlayList_sortfunc3(const void *a, const void *b) // by dir, then by title
{
	int t;
	char *a1, *a2, *p, *ia1, *ia2;
	ia1 = a1 = ((pl_entry *)a)->filename;
	ia2 = a2 = ((pl_entry *)b)->filename;
	p=scanstr_back(a1,"\\",a1-1)+1;
	if (p > a1) a1 = p;
	p=scanstr_back(a2,"\\",a2-1)+1;
	if (p > a2) a2 = p;

	{
		int l1,l2;
		l1 = a1-ia1;
		l2 = a2-ia2;

		t=_strnicmp(ia1,ia2,min(l1,l2));
		if (t) return t;
		if (l1 != l2) return l1-l2;
		return (_stricmp(a1,a2));
		
	}
}


void PlayList_sort(int bytitle, int start_p)
{
  int x;
  if (start_p >= list_size || list_size < 2 || !list) return;
  for (x = 0; x < list_size; x++)
    list[x].tmp=0;
//  list[list_pos].tmp=1;
	if (bytitle == 0)
		qsort(list+start_p,list_size-start_p,sizeof(pl_entry),PlayList_sortfunc);
	else if (bytitle == 1)
		qsort(list+start_p,list_size-start_p,sizeof(pl_entry),PlayList_sortfunc2);
	else qsort(list+start_p,list_size-start_p,sizeof(pl_entry),PlayList_sortfunc3);
//  for (x = 0; x < list_size; x ++)
//  {
 //   if (list[x].tmp)
  //  {
  //    list_pos=x;
    //  break;
 //   }
//  }
	//plmodified=1;
}

void PlayList_reverse(void)
{
  int s, b;
  s = 0;
  b = list_size-1;
  while (s < b)
  {
	  pl_entry p;
//	  if (list_pos == s)
//		  list_pos = b;
//	  else if (list_pos == b)
//		  list_pos = s;
	  p = list[s];
	  list[s] = list[b];
	  list[b] = p;
	  s++;
	  b--;
  }
	//plmodified=1;
}

void PlayList_randomize(void)
{
  int x, b;
  pl_entry p;
  for (x = 0; x < list_size-1; x ++)
  {
		b = (warand()%(list_size-x-1)) + x + 1;
		if (warand()%(list_size-x))
		{
      p=list[b];
      list[b]=list[x];
      list[x]=p;
//      if (b == list_pos) list_pos=x;
 //     else if (x == list_pos) list_pos=b;
		}
  }
	//plmodified=1;
}

static void _PlayList_adddir(char *path, char *p, int rec) // YOU MUST PASS THIS THING A VALID DIRECTORY
{
	WIN32_FIND_DATA d;
	char filespec[MAX_PATH+15];
	HANDLE i;

  if (!path[0]) return;
 
	if (path[strlen(path)-1] == '\\') path[strlen(path)-1] = 0;
	

	wsprintf(filespec,"%s\\*.*",path);
	i =  FindFirstFile(filespec,&d);
	if (i != INVALID_HANDLE_VALUE)
	{
		do {
			if (rec && d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && d.cFileName[0]!='.')
			{
				wsprintf(filespec,"%s\\%s",path,d.cFileName);
				_PlayList_adddir(filespec,p,1);
			}

      if (!(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        char *ext=extension(d.cFileName);
        if (ext[0])
        {
          if (!_stricmp(ext,"lnk"))
          {
            char temp2[MAX_PATH];
            char thisf[MAX_PATH];
            wsprintf(thisf,"%s\\%s",path,d.cFileName);
    		    if (ResolveShortCut(plugin.hwndParent,thisf,temp2))
            {
              HANDLE h2;
              WIN32_FIND_DATA d2;
              if (strstr(temp2,"://")) PlayList_append(temp2);
              else
              {
                h2=FindFirstFile(temp2,&d2);
                if (h2 != INVALID_HANDLE_VALUE)
                {
                  if (!(d2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                  {
                    char *t=p;
                    char *s=extension(d2.cFileName);
                    while (*t)
                    {
                      if (!_stricmp(s,t))
                      {
                        PlayList_append(temp2);
                        break;
                      }
                      t+=strlen(t)+1;
                    }
                  }
                  else if (rec==1)
                  {
                    _PlayList_adddir(temp2,p,2);
                  }
                  FindClose(h2);
                }
              }
            }
          }
          else // !shortcut
          {
            char *a=p;
			      wsprintf(filespec,"%s\\%s",path,d.cFileName);
            while (*a) 
            {
              if (!_stricmp(a,ext)) 
              {
	      		      PlayList_append(filespec);
                  break;
              }
              a+=strlen(a)+1;
            }
          }
        }
      }
		} while (FindNextFile(i,&d));
		FindClose(i);
	}  
}

void PlayList_adddir(char *path, int recurse) 
{
  char *w=(char*)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GET_EXTLIST);
		_PlayList_adddir(path,w,recurse); 
  GlobalFree((HGLOBAL)w);
}

void PlayList_makerelative(char *listfile,char *filename)
{
	char path[MAX_PATH];
	char *p;
  strcpy(path,listfile);
	p=scanstr_back(path,"\\",path-1);
	if (p >= path) *(p+1) = 0;
	else return;
	if (strlen(path) < strlen(filename) && !_strnicmp(filename,path,strlen(path)))
	{
		strcpy(path,filename+strlen(path));
		strcpy(filename,path);
	}
  else if (path[1]==':' && path[2]=='\\')
  {
    path[2]=0;
	  if (strlen(path) < strlen(filename) && !_strnicmp(filename,path,strlen(path)))
	  {
		  strcpy(path,filename+strlen(path));
		  strcpy(filename,path);
	  }
  }
}
