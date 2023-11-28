#include <windows.h>
#include "../gen_ml/ml.h"
#include "../ml_pmp/pmp.h"
#include "UsbDevice.h"
#include <strsafe.h>

wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song);
BOOL RecursiveCreateDirectory(wchar_t* buf1);

static void removebadchars(wchar_t *s) {
  while (*s) {
    if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
      *s=L'_';
    s++;
  }
}

/*
static wchar_t * lastbackslash(wchar_t *path) {
  wchar_t *p = path;
  while (*p) p++;
  while (*p != '\\' && p > path) p = CharPrev(path, p);
  return p;
}

static wchar_t *getfilename(wchar_t *path) {
  wchar_t *p = lastbackslash(path);
  if (*p == '\\') p = CharNext(p);
  return _wcsdup(p);
}

// includes trailing slash
static wchar_t *getdir(wchar_t *path) {
  wchar_t *p = lastbackslash(path) + 1;
  wchar_t c = *p;
  *p = 0;
  wchar_t *ret = _wcsdup(path);
  *p = c;
  return ret;
}

*/

// Skip_Root: removes drive/host/share name in a path
wchar_t * Skip_Root(wchar_t *path) {
  wchar_t *p = CharNext(path);
  wchar_t *p2 = CharNext(p);
  if (*path && *p == L':' && *p2 == L'\\') return CharNext(p2);
  else if (*path == L'\\' && *p == L'\\') {
    // skip host and share name
    int x = 2;
    while (x--) {
      while (*p2 != L'\\') {
        if (!*p2) return NULL;
        p2 = CharNext(p2);
      }
      p2 = CharNext(p2);
    }
    return p2;
  }
  return NULL;
}

// RecursiveCreateDirectory: creates all non-existent folders in a path
BOOL RecursiveCreateDirectory(wchar_t* buf1) {
  wchar_t *p=buf1;
  wchar_t ch='c';
  int errors = 0;
  if (*p) {
    p = Skip_Root(buf1);
    if (!p) return true ;

    while (ch) {
      //WIN32_FIND_DATA *fd;
      while (*p != '\\' && *p) p=CharNext(p);
      ch=*p;
      *p=0;
      int pp = wcslen(buf1)-1;

      while(buf1[pp] == '.' || 
            buf1[pp] == ' ' || 
            (buf1[pp] == '\\' && (buf1[pp-1] == '.' || buf1[pp-1] == ' ' || buf1[pp-1] == '/'))
            || buf1[pp] == '/' && buf1)
      {
        if(buf1[pp] == '\\')
        {
          buf1[pp-1] = '_';
          pp -= 2;
        }else{
          buf1[pp] = '_';
          pp--;
        }
      }

      HANDLE h;
      WIN32_FIND_DATA fd;
      // Avoid a "There is no disk in the drive" error box on empty removable drives
      SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      h = FindFirstFile(buf1,&fd);
      SetErrorMode(0);
      if (h == INVALID_HANDLE_VALUE)
      { 
        if (!CreateDirectory(buf1,NULL)) errors++;
      } else {
        FindClose(h);
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) errors++;
      }
      *p++ = ch;
    }
  }

  return errors != 0;
}

// FixReplacementVars: replaces <Artist>, <Title>, <Album>, and #, ##, ##, with appropriate data
// DOES NOT add a file extention!!
wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song)
{
#define ADD_STR(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (*outp) outp++; }
#define ADD_STR_U(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (*outp) { *outp=towupper(*outp); outp++; } }
#define ADD_STR_L(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (*outp) { *outp=towlower(*outp); outp++; } }

  wchar_t tmpsrc[4096];
  lstrcpyn(tmpsrc,str,sizeof(tmpsrc)/sizeof(wchar_t)); //lstrcpyn is nice enough to make sure it's null terminated.

  wchar_t *inp=tmpsrc;
  wchar_t *outp=str;

  while (*inp && outp-str < str_size-2)
  {
    if (*inp == L'<')
    {
      if (!wcsncmp(inp,L"<TITLE>",7))
      {
        ADD_STR_U(dev->getTrackTitle);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<title>",7))
      {
        ADD_STR_L(dev->getTrackTitle);
        inp+=7;
      }
      else if (!_wcsnicmp(inp,L"<Title>",7))
      {
        ADD_STR(dev->getTrackTitle);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<ALBUM>",7))
      {
        ADD_STR_U(dev->getTrackAlbum);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<album>",7))
      {
        ADD_STR_L(dev->getTrackAlbum);
        inp+=7;
      }
      else if (!_wcsnicmp(inp,L"<Album>",7))
      {
        ADD_STR(dev->getTrackAlbum);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<GENRE>",7))
      {
        ADD_STR_U(dev->getTrackGenre);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<genre>",7))
      {
        ADD_STR_L(dev->getTrackGenre);
        inp+=7;
      }
      else if (!_wcsnicmp(inp,L"<Genre>",7))
      {
        ADD_STR(dev->getTrackGenre);
        inp+=7;
      }
      else if (!wcsncmp(inp,L"<ARTIST>",8))
      {
        ADD_STR_U(dev->getTrackArtist);
        inp+=8;
      }
      else if (!wcsncmp(inp,L"<artist>",8))
      {
        ADD_STR_L(dev->getTrackArtist);
        inp+=8;
      }
      else if (!_wcsnicmp(inp,L"<Artist>",8))
      {
        ADD_STR(dev->getTrackArtist);
        inp+=8;
      }
			else if (!wcsncmp(inp,L"<ALBUMARTIST>",13))
      {
				wchar_t temp[128];
				temp[0]=0;
				
				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(outp-str));
				while (*outp) { *outp=towupper(*outp); outp++; }

        inp+=13;
      }
      else if (!wcsncmp(inp,L"<albumartist>",13))
      {
				wchar_t temp[128];
				temp[0]=0;
				
				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(outp-str));
				while (*outp) { *outp=towlower(*outp); outp++; }
        inp+=13;
      }
      else if (!_wcsnicmp(inp,L"<Albumartist>",13))
      {
				wchar_t temp[128];
				temp[0]=0;
				
				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(outp-str));
				while (*outp) outp++;
        inp+=13;
      }

      else if (!_wcsnicmp(inp,L"<year>",6))
      {
        wchar_t year[64];
        int y = dev->getTrackYear(song);
        if(y==0) year[0]=0;
        else wsprintf(year, L"%d", y);
        
        lstrcpyn(outp,year,str_size-1-(outp-str)); while (*outp) outp++;

        inp+=6;
      }
			else if (!_wcsnicmp(inp,L"<filename>",10))
			{
				wchar_t tfn[MAX_PATH], *ext, *fn;
				StringCchCopy(tfn,MAX_PATH,((UsbSong*)song)->filename);
				ext = wcsrchr(tfn, L'.');
				*ext = 0; //kill extension since its added later
				fn = wcsrchr(tfn, L'\\');
				fn++;
				lstrcpyn(outp,fn,str_size-1-(outp-str));
				while (*outp) outp++;
				inp+=10;
			}
      else *outp++=*inp++;
    }
    else if (*inp == L'#')
    {
			int	nd=0;
      wchar_t tmp[64];
      while (*inp == L'#') nd++,inp++;
      int track = dev->getTrackTrackNum(song);
      if (!track)
      {
        tmp[0]=0;
        while (*inp == L' ') inp++;
				while (*inp == L'\\') inp++; 
        if (*inp == L'-' || *inp == L'.' || *inp == L'_') // separator
        {
          inp++;
          while (*inp == L' ') inp++;
        }
      }
      else
      {
        if (nd > 1)
        {
          wchar_t tmp2[32];
          if (nd > 5) nd=5;
          wsprintf(tmp2,L"%%%02dd",nd);
          wsprintf(tmp,tmp2,track);
        }
        else wsprintf(tmp, L"%d",track);
      }
      lstrcpyn(outp,tmp,str_size-1-(outp-str)); while (*outp) outp++;
    }
    else *outp++=*inp++;
  }

  *outp=0;


  inp=str;
  outp=str;
  wchar_t lastc=0;
  while (*inp) 
  {
    wchar_t ch=*inp++;
    if (ch == L'\t') ch=L' ';

    if (ch == L' ' && (lastc == L' ' || lastc == L'\\' || lastc == L'/')) continue; // ignore space after slash, or another space
    
    if ((ch == L'\\' || ch == L'/') && lastc == L' ') outp--;  // if we have a space then slash, back up to write the slash where the space was
    *outp++=ch;
    lastc=ch;
  }
  *outp=0;
#undef ADD_STR
#undef ADD_STR_L
#undef ADD_STR_U

  return str;
}