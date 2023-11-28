#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

// this is a big hack

char *guessTitles(const char *filename, 
                 int *tracknum,
                 char **artist, 
                 char **album,
                 char **title) // should free the result of this function after using artist/album/title
{
  *tracknum=0;
  *artist=0;
  *album=0;
  *title=0;

  if (!filename[0]) return 0;

  char *_artist=NULL;
  char *_album=NULL;

  
  const char *f=filename;
  while (*f) f++;
  while (f>filename && *f != '/' && *f != '\\') f--;

  if (f == filename) return 0;

  int dirlen = f-filename;

  char *fullfn=(char*)_strdup(filename);
  fullfn[dirlen]=0;

  char *n=fullfn+dirlen;
  while (n >= fullfn && *n != '/' && *n != '\\') n--;
  n++;

  // try to guess artist and album from the dirname

  if (!strstr(n,"-")) // assume dir name is album name, artist name unknown
  {
    *album=n;
    _album=n;
  }
  else 
  {
    *artist=_artist=n;
    _album=strstr(n,"-")+1;
    char *t=_album-2;
    while (t >= n && *t == ' ') t--;
    t[1]=0;

    while (*_album == ' ') _album++;
    *album=_album;
  }


  // get filename shizit
  strcpy(fullfn+dirlen+1,filename+dirlen+1);

  n=fullfn+dirlen+1;
  while (*n) n++;
  while (n > fullfn+dirlen+1 && *n != '.') n--;
  if (*n == '.') *n=0;
  n=fullfn+dirlen+1;

  while (*n == ' ') n++;


  // detect XX. filename
  if (strstr(n,".") && atoi(n) && atoi(n) < 130)
  {
    char *tmp=n;
    while (*tmp >= '0' && *tmp <= '9') tmp++;
    while (*tmp == ' ') tmp++;
    if (*tmp == '.') 
    { 
      *tracknum=atoi(n); 
      n=tmp+1; 
      while (*n == '.') n++;
      while (*n == ' ') n++;
    }
  }

  // detect XX- filename
  if (!*tracknum && strstr(n,"-") && atoi(n) && atoi(n) < 130)
  {
    char *tmp=n;
    while (*tmp >= '0' && *tmp <= '9') tmp++;
    while (*tmp == ' ') tmp++;
    if (*tmp == '-') 
    { 
      *tracknum=atoi(n); 
      n=tmp+1; 
      while (*n == '-') n++;
      while (*n == ' ') n++;
    }
  }
 
  while (strstr(n,"-"))
  {
    char *a=n;
    n=strstr(n,"-");
    {
      char *t=n-1;
      while (t >= a && *t == ' ') t--;
      t[1]=0;
    }
    *n=0;
    n++;
    while (*n == '-') n++;
    while (*n == ' ') n++;

    // a is the next token.

    if (!*tracknum && !_strnicmp(a,"Track ",6) && atoi(a+6)) *tracknum=atoi(a+6);
    else if (*artist== _artist)
    {
      *artist=a;
    }
    if (*artist != _artist && *tracknum) break;
  }
  *title=n;
 
  
  return fullfn;  
}