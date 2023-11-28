#include "std.h"
//#include <wasabicfg.h>
#include <bfc/parse/pathparse.h>
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN

#include <shlobj.h>
#include <multimon.h>
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x500)
DECLARE_HANDLE(HMONITOR);
#define HMONITOR_DECLARED
#endif

#include <objbase.h>
#include <shellapi.h>  // for ShellExecute
#include <math.h>
#include <mbctype.h>

#include <sys/stat.h>

#ifndef SPI_GETWHEELSCROLLLINES
#  define SPI_GETWHEELSCROLLLINES  104
#endif

#ifndef SPI_GETLISTBOXSMOOTHSCROLLING
#  define SPI_GETLISTBOXSMOOTHSCROLLING  0x1006
#endif

#endif  // WIN32

#include <bfc/bfc_assert.h>

#include <bfc/ptrlist.h>

#include <time.h>  // CUT if possible... maybe make bfc/platform/time.h
#ifdef __APPLE__
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <malloc.h>
#include <shlwapi.h>
#endif

#define USER_PTR_TO_ALLOCMGR_PTR(x) (((char *)x)-4)
#define ALLOCMGR_PTR_TO_USER_PTR(x) (((char *)x)+4)
#define ALLOCMGR_SIZE(x) *((size_t *)x)

#define ROUNDUP4(size) size = (size+3) & ~3

void *DO_MALLOC(size_t size EXTRA_INFO) 
{
  ASSERT(size > 0);
  if (size <= 0) return NULL;

	ROUNDUP4(size);

  void *p = malloc(size+4);
  if (p != NULL) MEMZERO(p, size+4);//BU added in response to talkback info

 if (p) ALLOCMGR_SIZE(p) = size;
 return ALLOCMGR_PTR_TO_USER_PTR(p);
}

wchar_t *DO_WMALLOC(size_t size EXTRA_INFO) 
{
  ASSERT(size > 0);
  if (size == 0) return NULL;

	size *= sizeof(wchar_t);

	ROUNDUP4(size);

  void *p = malloc(size+4);

 if (p) ALLOCMGR_SIZE(p) = size;
 return reinterpret_cast<wchar_t *>(ALLOCMGR_PTR_TO_USER_PTR(p));
}

void *DO_CALLOC(int records, int recordsize EXTRA_INFO) {
  ASSERT(records > 0 && recordsize > 0);
  return DO_MALLOC(records * recordsize EXTRA_PARAMS);
}

void DO_FREE(void *ptr EXTRA_INFO) {
  if (ptr == NULL) return;

  free(USER_PTR_TO_ALLOCMGR_PTR(ptr));
}

// Note: MEMCPY allows dest and src to overlap
void MEMCPY(void *dest, const void *src, size_t n) {
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  ASSERT(n >= 0);
  memmove(dest, src, n);
}

void *DO_MEMDUP(const void *src, size_t n EXTRA_INFO) {
  void *ret;
  ASSERT(n >= 0);
  if (src == NULL || n == 0) return NULL;
  ret = DO_MALLOC(n EXTRA_PARAMS);
  if (ret == NULL) return NULL;
  MEMCPY(ret, src, n);
  return ret;
}

#define FAKE_REALLOC

//PORTME: BU> umm... tried to merge w/ linux ver but doesn't look right
void *DO_REALLOC(void *ptr, size_t size EXTRA_INFO) {
  ASSERT(size >= 0);

  if (size == 0) {

    DO_FREE(ptr);
    return NULL;
  }

	ROUNDUP4(size);

  if(ptr == NULL) return DO_MALLOC(size);

  void *r = NULL;

  size_t oldsize = ALLOCMGR_SIZE(USER_PTR_TO_ALLOCMGR_PTR(ptr));


  r = realloc(USER_PTR_TO_ALLOCMGR_PTR(ptr), size+4);


 // r is an ALLOCMGR pointer

  if (r != NULL) {
    ALLOCMGR_SIZE(r) = size;
    r = ALLOCMGR_PTR_TO_USER_PTR(r);
  } else {
    // realloc failed !
    void *newblock = MALLOC(size);
    MEMCPY(newblock, ptr, MIN(oldsize, size));
    FREE(ptr);
    r = newblock;
  }

 // r is now a USER pointer


   if (size > oldsize)
    MEMZERO((char *)r+oldsize, size-oldsize);

  return r;
}

size_t MEMSIZE(void *ptr) {
  return ALLOCMGR_SIZE(USER_PTR_TO_ALLOCMGR_PTR(ptr));
}

static wchar_t TOUPPERANDSLASH(wchar_t a)
{
  if (a=='\\')
    a = '/';
  else
    a = TOUPPERW(a);
  return a;
}

int PATHEQL(const wchar_t *str1, const wchar_t *str2) {
  if (str1 == NULL) {
    if (str2 == NULL) return TRUE;
    return FALSE;
  }
  while (TOUPPERANDSLASH(*str1) == TOUPPERANDSLASH(*str2) && *str1 != 0 && *str2 != 0) str1++, str2++;
  return *str1 == *str2;
}


static int rand_inited;

static double divider=0.;

class dummyconstructor
{
public:
  dummyconstructor()
  {
#ifdef WIN32
    LARGE_INTEGER ll;
    QueryPerformanceFrequency(&ll);
    divider = (double)ll.QuadPart;    
#endif
  }
};

static dummyconstructor blep;

void Std::getMousePos(POINT *p) {
  ASSERT(p != NULL);
#ifdef WIN32
  GetCursorPos(p);
#elif defined(__APPLE__)
  Point pt;
  GetMouse(&pt);
  p->x = pt.v;
  p->y = pt.h;
#else
  Window w1, w2;
  int a, b;
  unsigned int c;
  int x, y;
  
  XQueryPointer( Linux::getDisplay(), Linux::RootWin(), &w1, &w2,
                 &x, &y, &a, &b, &c );
  
  p->x = x;
  p->y = y;
#endif
}

void Std::getMousePos(int *x, int *y) {
  POINT p;
  getMousePos(&p);
  if (x != NULL) *x = p.x;
  if (y != NULL) *y = p.y;
}

void Std::getMousePos(long *x, long *y) {
  getMousePos((int *)x, (int *)y);
}

void Std::setMousePos(POINT *p) {
  ASSERT(p != NULL);
#ifdef WIN32
  SetCursorPos(p->x, p->y);
#elif defined(__APPLE__)
  CGWarpMouseCursorPosition(HIPointFromPOINT(p));
#else
  POINT p2;
  getMousePos( &p2 );
  
  XWarpPointer( Linux::getDisplay(), None, None,
                0, 0, 1, 1, p->x - p2.x, p->y - p2.y );
#endif
}

void Std::setMousePos(int x, int y) {
  POINT p={x,y};
  setMousePos(&p);
}

void Std::getViewport(RECT *r, POINT *p, int full) 
{
#ifdef _WIN32
  getViewport(r, p, NULL, (HWND)0, full);
#elif defined(__APPLE__)
  CGDirectDisplayID display;
  CGDisplayCount count;
  if (CGGetDisplaysWithPoint(HIPointFromPOINT(p), 1, &display, &count) == kCGErrorSuccess)
  {
    HIRect rect = CGDisplayBounds(display);
    *r = RECTFromHIRect(&rect);
  // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
  }
#endif
}

void Std::getViewport(RECT *r, RECT *sr, int full) 
{
#ifdef _WIN32
  getViewport(r, NULL, sr, (HWND)0, full);
#elif defined(__APPLE__)
  CGDirectDisplayID display;
  CGDisplayCount count;
  if (CGGetDisplaysWithRect(HIRectFromRECT(sr), 1, &display, &count) == kCGErrorSuccess)
  {
    HIRect rect = CGDisplayBounds(display);
    *r = RECTFromHIRect(&rect);
  // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
  }
#endif
}

void Std::getViewport(RECT *r, OSWINDOWHANDLE wnd, int full)
{
#ifdef _WIN32
	getViewport(r, NULL, NULL, wnd, full);
#elif defined(__APPLE__)
	GDHandle gd;
	Rect gdr;
	GetWindowGreatestAreaDevice(wnd, kWindowStructureRgn, &gd, &gdr);
	  
	if (full)
	{
		CGDirectDisplayID display = QDGetCGDirectDisplayID(gd);
		HIRect rect = CGDisplayBounds(display);
		*r = RECTFromHIRect(&rect);
	}
	else
	{
		// TODO: maybe use GetAvailableWindowPositioningBounds instead
		r->left = gdr.left;
		r->top = gdr.top;
		r->right = gdr.right;
		r->bottom = gdr.bottom;
	}
#endif
}
#ifdef _WIN32
static HINSTANCE user32_instance = 0;
BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFO lpmi) = 0;
#endif
void Std::getViewport(RECT *r, POINT *p, RECT *sr, OSWINDOWHANDLE wnd, int full) {
	ASSERT(r != NULL);
#ifdef WIN32
	if (p || sr || wnd) 
	{
		if (!user32_instance)
			user32_instance=LoadLibraryA("user32.dll");
		if (user32_instance)
		{
			HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags) = (HMONITOR (WINAPI *)(POINT,DWORD)) GetProcAddress(user32_instance,"MonitorFromPoint");
			HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags) = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(user32_instance, "MonitorFromRect");
			HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags)=(HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(user32_instance, "MonitorFromWindow");
			if (!Gmi)
				Gmi = (BOOL (WINAPI *)(HMONITOR,LPMONITORINFO)) GetProcAddress(user32_instance,"GetMonitorInfoA");
			if (Mfp && Mfr && Mfw && Gmi) 
			{
				HMONITOR hm;
				if (p)
					hm=Mfp(*p,MONITOR_DEFAULTTONEAREST);
				else if (sr)
					hm=Mfr(sr,MONITOR_DEFAULTTONEAREST);
				else if (wnd)
					hm=Mfw(wnd,MONITOR_DEFAULTTONEAREST);
				if (hm) {
					MONITORINFOEX mi;
					ZERO(mi);
					mi.cbSize=sizeof(mi);

					if (Gmi(hm,&mi)) 
					{
						if(!full) *r=mi.rcWork;
						else *r=mi.rcMonitor;
						return;
					}
				}
			}
		}
	}
	SystemParametersInfo(SPI_GETWORKAREA,0,r,0);
#endif
#ifdef LINUX
	r->left = r->top = 0;
	r->right = Std::getScreenWidth();
	r->bottom = Std::getScreenHeight();
#endif
}

#ifdef WIN32
class ViewportEnumerator {
  public:
    HMONITOR hm;
#ifdef WIN32
    int monitor_n;
#endif
    int counter;
};

static BOOL CALLBACK enumViewportsProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	ViewportEnumerator *ve = reinterpret_cast<ViewportEnumerator *>(dwData);
	ASSERT(ve != NULL);
	if (ve->counter == ve->monitor_n) {
		ve->hm = hMonitor;
		return FALSE;
	}
	ve->counter++;
	return TRUE;
}
#endif

void Std::srandom(unsigned int key) {
	if (key == 0) key = (unsigned int)Std::getTimeStamp();
	::srand(key);
}

int Std::random(int max) {
//  ASSERT(max <= RAND_MAX);
  if (!rand_inited++) srandom();
  int ret = ::rand();
  if (max != RAND_MAX+1) ret %= max;
  return ret;
}

unsigned int Std::random32(unsigned int max) {
  if (!rand_inited++) Std::srandom();
  unsigned int val=0;
  for (int i = 0; i < sizeof(unsigned int); i++) {
    val <<= 8;
    val |= ((::rand()>>2) & 0xff);
  }
  if (max != 0xffffffff) val %= max;
  return val;
}

#ifdef _WIN32 // PORT ME
void Std::usleep(int ms) {
  Sleep(ms);
//INLINE
}
#endif

time_t Std::getTimeStamp() {
  return time(NULL);
}

stdtimevalms Std::getTimeStampMS() {
#ifdef WIN32
  LARGE_INTEGER ll;
  if (!QueryPerformanceCounter(&ll)) return getTickCount() / 1000.f;
  stdtimevalms ret = (stdtimevalms)ll.QuadPart;
  return ret /= divider;
#else
  return getTickCount() / 1000.f;
#endif
}

uint32_t Std::getTickCount()
{
#ifdef _WIN32
  return GetTickCount();
#elif defined(__APPLE__)
  struct timeval newtime;
  
	gettimeofday(&newtime, 0);
  return newtime.tv_sec*1000 + newtime.tv_usec/1000;
#endif
//INLINE
}

#ifdef _WIN32 // PORT ME
void Std::ensureVisible(RECT *r) {
  RECT sr;
  POINT p={(r->right+r->left)/2,(r->bottom+r->top)/2};
  Std::getViewport(&sr,&p);
  int w = r->right-r->left;
  int h = r->bottom-r->top;
  if (r->bottom > sr.bottom) {
    r->bottom = sr.bottom;
    r->top = r->bottom-h;
  }
  if (r->right > sr.right) {
    r->right = sr.right;
    r->left = r->right-w;
  }
  if (r->left < sr.left) {
    r->left = sr.left;
    r->right = r->left+w;
  }
  if (r->top < sr.top) {
    r->top = sr.top;
    r->bottom = r->top+h;
  }
}
#endif

int Std::getScreenWidth()
{
#ifdef WIN32
  RECT r;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.right-r.left;
#elif defined(__APPLE__)
  CGDirectDisplayID  mainID = CGMainDisplayID();
  return CGDisplayPixelsWide(mainID);
#elif defined(LINUX)
  return DisplayWidth(Linux::getDisplay(), Linux::getScreenNum());
#endif
}

int Std::getScreenHeight() 
{
#ifdef WIN32
  RECT r;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.bottom-r.top;
#elif defined(__APPLE__)
  CGDirectDisplayID  mainID = CGMainDisplayID();
  return CGDisplayPixelsHigh(mainID);  
#else
  return DisplayHeight( Linux::getDisplay(), Linux::getScreenNum() );
#endif
}
#ifndef __APPLE__ // PORT ME
int Std::messageBox(const wchar_t *txt, const wchar_t *title, int flags) {
#ifdef WIN32
  return MessageBoxW(NULL, txt, title, flags);
#else
 OutputDebugString(txt);
#endif
}
#endif

int Std::getDoubleClickDelay() {
#ifdef WIN32
  return GetDoubleClickTime();
#elif defined(__APPLE__)
  return GetDblTime();
#endif
}

#undef GetSystemMetrics //FG> DUH!
int Std::getDoubleClickX() {
#ifdef WIN32
  return GetSystemMetrics(SM_CYDOUBLECLK);
#else
  return 5;
#endif
}

int Std::getDoubleClickY() {
#ifdef WIN32
  return GetSystemMetrics(SM_CXDOUBLECLK);
#else
  return 5;
#endif
}

int Std::osparam_getScrollLines() {
  int ret = 3;
#ifdef WIN32
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ret, 0);
#endif
  return ret;
}

int Std::osparam_getSmoothScroll() {
  int ret = 1;
#ifdef WIN32
  SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &ret, 0);
#endif
  return ret;
}



const wchar_t Std::dirChar() {
#ifdef WIN32
  return '\\';
#else
  return '/';
#endif
}

const char *Std::dirCharStr() {
#ifdef WIN32
  return "\\";
#else
  return "/";
#endif
};

const wchar_t *Std::dirCharStrW() 
{
#ifdef WIN32
  return L"\\";
#else
  return L"/";
#endif
};

int Std::isDirChar(int thechar, int allow_multiple_platforms) {
  if (thechar == DIRCHAR) return 1;
  if (allow_multiple_platforms) {
#ifdef WIN32
    if (thechar == '/') return 1;
#else
    if (thechar == '\\') return 1;
#endif
  }
  return 0;
}

const wchar_t *Std::matchAllFiles() {
#ifdef WIN32
  return L"*.*";
#else
  return L"*";
#endif
}

const wchar_t *Std::dotDir() {
  return L".";
}

const wchar_t *Std::dotDotDir() 
{
  return L"..";
}
#ifdef _WIN32 // PORT ME
bool Std::isRootPath(const wchar_t *path) 
{
	return PathIsRootW(path)==TRUE;
}
#endif

int Std::switchChar() {
#ifdef WIN32
  return '/';
#else
  return '-';
#endif
}


int Std::enumViewports(int monitor_n, RECT *r, int full) {
  ASSERT(r != NULL);
#ifdef WIN32
  		if (!user32_instance)
			user32_instance=LoadLibraryA("user32.dll");
    if (user32_instance)
		{
			if (!Gmi)
				Gmi = (BOOL (WINAPI *)(HMONITOR,LPMONITORINFO)) GetProcAddress(user32_instance,"GetMonitorInfoA");
    BOOL (WINAPI *Edm)(HDC hdc, LPCRECT clip, MONITORENUMPROC proc, LPARAM dwdata) = (BOOL (WINAPI *)(HDC,LPCRECT,MONITORENUMPROC,LPARAM)) GetProcAddress(user32_instance,"EnumDisplayMonitors");
    if (Gmi && Edm) 
		{
      ViewportEnumerator ve;
      ve.monitor_n = monitor_n;
      ve.hm = NULL;
      ve.counter = 0;
      Edm(NULL, NULL, enumViewportsProc, reinterpret_cast<LPARAM>(&ve));
      HMONITOR hm = ve.hm;
      if (hm) {
        MONITORINFOEX mi;
        ZERO(mi);
        mi.cbSize=sizeof(mi);
        if (Gmi(hm,&mi)) 
				{
          if(!full) *r=mi.rcWork;
          else *r=mi.rcMonitor;
          return 1;
        }
      }
    }

  }
  SystemParametersInfo(SPI_GETWORKAREA,0,r,0);
  return 0;
#elif defined(__APPLE__)
  CGDirectDisplayID monitors[256]; // TODO: allocate dynamically
  CGDisplayCount count;
  if (monitor_n >= 256)
    return 0;
  
  CGGetActiveDisplayList(256, monitors, &count);
  if (count <= monitor_n)
    return 0;
  
  HIRect rect = CGDisplayBounds(monitors[monitor_n]);
  *r = RECTFromHIRect(&rect);
  // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
#elif defined(LINUX)
  if ( monitor_n > 0 )
    return 0;
  r->left = r->top = 0;
  r->right = Std::getScreenWidth();
  r->bottom = Std::getScreenHeight();
  return 1;
#endif
}



const char *Std::scanstr_back(const char *str, const char *toscan, const char *defval) {
  int strl = STRLEN(str);
  const char *s=str+strl-1;
  if (strl < 1) return defval;
  if (STRLEN(toscan) < 1) return defval;
  while (1) {
    const char *t=toscan;
    while (*t) if (*t++ == *s) return s;
#ifdef _WIN32
    t=CharPrevA(str,s);
#else
    t = s-1;
#endif
    if (t==s) return defval;
    s=t;
  }
}

const wchar_t *Std::extension(const wchar_t *fn) 
{
#ifdef _WIN32 // PORT ME
		wchar_t *x = PathFindExtensionW(fn);

	if (*x)
		return CharNextW(x);
	else
		return x;
#else
#warning port me
  return 0;
#endif
}

const wchar_t *Std::filename(const wchar_t *fn) 
{
#ifdef _WIN32
	return PathFindFileNameW(fn);
#else
#warning port me
  return 0;
#endif
}

bool Std::isMatchPattern(const wchar_t *p) 
{
  while (*p) {
    switch (*p++) {
      case '?':
      case '*':
      case '[':
      case '\\':
        return TRUE;
    }
  }
  return FALSE;
}

bool Std::isValidMatchPattern(const wchar_t *p, int *error_type) 
{
  /* init error_type */
  *error_type = PATTERN_VALID;
  /* loop through pattern to EOS */
  while (*p) {
    /* determine pattern type */
    switch (*p) {
      /* check literal escape, it cannot be at end of pattern */
      case '\\':
        if (!*++p) {
          *error_type = PATTERN_ESC;
          return FALSE;
        }
        p++;
        break;
      /* the [..] construct must be well formed */
      case '[':
        p++;
        /* if the next character is ']' then bad pattern */
        if (*p == ']') {
          *error_type = PATTERN_EMPTY;
          return FALSE;
        }
        /* if end of pattern here then bad pattern */
        if (!*p) {
          *error_type = PATTERN_CLOSE;
          return FALSE;
        }
        /* loop to end of [..] construct */
        while (*p != ']') {
          /* check for literal escape */
          if (*p == '\\') {
            p++;
            /* if end of pattern here then bad pattern */
            if (!*p++) {
              *error_type = PATTERN_ESC;
              return FALSE;
            }
          } else p++;
        /* if end of pattern here then bad pattern */
        if (!*p) {
          *error_type = PATTERN_CLOSE;
          return FALSE;
        }
        /* if this a range */
        if (*p == '-') {
          /* we must have an end of range */
          if (!*++p || *p == ']') {
            *error_type = PATTERN_RANGE;
            return FALSE;
          } else {
            /* check for literal escape */
            if (*p == '\\') p++;
            /* if end of pattern here then bad pattern */
            if (!*p++) {
              *error_type = PATTERN_ESC;
              return FALSE;
            }
          }
        }
      }
      break;
      /* all other characters are valid pattern elements */
      case '*':
      case '?':
      default:
        p++; /* "normal" character */
        break;
    }
  }
  return TRUE;
}

#ifdef _WIN32 // PORT ME
int Std::matche(register const wchar_t *p, register const wchar_t *t) 
{
  register wchar_t range_start, range_end;  /* start and end in range */

  BOOLEAN invert;             /* is this [..] or [!..] */
  BOOLEAN member_match;       /* have I matched the [..] construct? */
  BOOLEAN loop;               /* should I terminate? */

  for ( ; *p; p++, t++) 
	{
    /* if this is the end of the text then this is the end of the match */
    if (!*t) {
      return ( *p == '*' && *++p == '\0' ) ? MATCH_VALID : MATCH_ABORT;
    }
    /* determine and react to pattern type */
    switch (*p) {
      case '?': /* single any character match */
        break;
      case '*': /* multiple any character match */
        return matche_after_star (p, t);

      /* [..] construct, single member/exclusion character match */
      case '[': {
        /* move to beginning of range */
        p++;
        /* check if this is a member match or exclusion match */
        invert = FALSE;
        if (*p == '!' || *p == '^') {
          invert = TRUE;
          p++;
        }
        /* if closing bracket here or at range start then we have a malformed pattern */
        if (*p == ']') return MATCH_PATTERN;

        member_match = FALSE;
        loop = TRUE;
        while (loop) {
          /* if end of construct then loop is done */
          if (*p == ']') {
            loop = FALSE;
            continue;
          }
          /* matching a '!', '^', '-', '\' or a ']' */
          if (*p == '\\') range_start = range_end = *++p;
          else  range_start = range_end = *p;
          /* if end of pattern then bad pattern (Missing ']') */
          if (!*p) return MATCH_PATTERN;
          /* check for range bar */
          if (*++p == '-') {
            /* get the range end */
            range_end = *++p;
            /* if end of pattern or construct then bad pattern */
            if (range_end == '\0' || range_end == ']') return MATCH_PATTERN;
            /* special character range end */
            if (range_end == '\\') {
              range_end = *++p;
              /* if end of text then we have a bad pattern */
              if (!range_end) return MATCH_PATTERN;
            }
            /* move just beyond this range */
            p++;
          }
          /* if the text character is in range then match found.
             make sure the range letters have the proper
             relationship to one another before comparison */
          if (range_start < range_end) {
            if (*t >= range_start && *t <= range_end) {
              member_match = TRUE;
              loop = FALSE;
            }
          } else {
            if (*t >= range_end && *t <= range_start) {
              member_match = TRUE;
              loop = FALSE;
            }
          }
        }
        /* if there was a match in an exclusion set then no match */
        /* if there was no match in a member set then no match */
        if ((invert && member_match) || !(invert || member_match)) return MATCH_RANGE;
        /* if this is not an exclusion then skip the rest of the [...] construct that already matched. */
        if (member_match) {
          while (*p != ']') {
            /* bad pattern (Missing ']') */
            if (!*p) return MATCH_PATTERN;
            /* skip exact match */
            if (*p == '\\') {
              p++;
              /* if end of text then we have a bad pattern */
              if (!*p) return MATCH_PATTERN;
            }
            /* move to next pattern char */
            p++;
          }
        }
        break;
      }
      case '\\':  /* next character is quoted and must match exactly */
        /* move pattern pointer to quoted char and fall through */
        p++;
        /* if end of text then we have a bad pattern */
        if (!*p) return MATCH_PATTERN;
        /* must match this character exactly */
      default:
        if (TOUPPERW(*p) != TOUPPERW(*t)) return MATCH_LITERAL;
    }
  }
  /* if end of text not reached then the pattern fails */
  if (*t) return MATCH_END;
  else return MATCH_VALID;
}

int Std::matche_after_star(register const wchar_t *p, register const wchar_t *t) 
{
  register int match = 0;
  register wchar_t nextp;
  /* pass over existing ? and * in pattern */
  while ( *p == '?' || *p == '*' ) 
	{
    /* take one char for each ? and + */
    if (*p == '?') {
      /* if end of text then no match */
      if (!*t++) return MATCH_ABORT;
    }
    /* move to next char in pattern */
    p++;
  }
  /* if end of pattern we have matched regardless of text left */
  if (!*p) return MATCH_VALID;
  /* get the next character to match which must be a literal or '[' */
  nextp = *p;
  if (nextp == '\\') {
    nextp = p[1];
    /* if end of text then we have a bad pattern */
    if (!nextp) return MATCH_PATTERN;
  }
  /* Continue until we run out of text or definite result seen */
  do {
    /* a precondition for matching is that the next character
       in the pattern match the next character in the text or that
       the next pattern char is the beginning of a range.  Increment
       text pointer as we go here */
    if (TOUPPERW(nextp) == TOUPPERW(*t) || nextp == '[') match = matche(p, t);
    /* if the end of text is reached then no match */
    if (!*t++) match = MATCH_ABORT;
  } while ( match != MATCH_VALID && match != MATCH_ABORT && match != MATCH_PATTERN);
  /* return result */
  return match;
}

bool Std::match(const wchar_t *p, const wchar_t *t)
{
  int error_type;
  error_type = matche(p,t);
  return (error_type == MATCH_VALID ) ? TRUE : FALSE;
}
#endif
#ifndef _NOSTUDIO
int Std::getCurDir(wchar_t *str, int maxlen) {
  ASSERT(str != NULL);
#ifdef WIN32
  int retval = 0;

  retval = GetCurrentDirectoryW(maxlen, str);

  return retval;

#else
#warning port me
  return 0;
  //return getcwd( str, maxlen ) != NULL;
#endif
}

int Std::setCurDir(const wchar_t *str) {
  ASSERT(str != NULL);
#ifdef WIN32
  int retval = 0;

  retval = SetCurrentDirectoryW(str);

  return retval;

#else
#warning port me
  return 0;
//  return chdir( str );
#endif
}


int Std::getNumCPUs() {
#ifdef WIN32
  SYSTEM_INFO si;
  ZERO(si);
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors;
#else
#ifdef PLATFORM_WARNINGS
#warning Std::getNumCPUs not implemented on LINUX
#endif
  return 1;
#endif
}

THREADID Std::getCurrentThreadId() {
#ifdef WIN32
  return GetCurrentThreadId();
#else
  return pthread_self();
#endif
}

#ifdef WIN32
void Std::setThreadPriority(int delta, HANDLE thread_handle) 
{
  if (thread_handle == NULL) thread_handle = GetCurrentThread();
  int v = THREAD_PRIORITY_NORMAL;
  switch (delta) {
    case -32767: v = THREAD_PRIORITY_IDLE; break;
    case -2: v = THREAD_PRIORITY_LOWEST; break;
    case -1: v = THREAD_PRIORITY_BELOW_NORMAL; break;
    case 1: v = THREAD_PRIORITY_ABOVE_NORMAL; break;
    case 2: v = THREAD_PRIORITY_HIGHEST; break;
    case 32767: v = THREAD_PRIORITY_TIME_CRITICAL; break;
  }
  SetThreadPriority(thread_handle, v);
}
#else
#ifdef PLATFORM_WARNINGS
#warning Std::setThreadPriority not implemented on LINUX
#endif
#endif


String Std::getLastErrorString(int _err) {
  String ret;
  if (_err == -1) {
#ifdef WIN32
    _err = GetLastError();
#else
#ifdef PLATFORM_WARNINGS
#warning Std::getLastErrorString not implemented on LINUX
#endif
#endif
  }
#ifdef WIN32
  char *sysbuf;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, _err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&sysbuf, 0, NULL);
  ret = sysbuf;
  LocalFree(sysbuf);
#endif
  return ret;
}

// gets the system font path
void Std::getFontPath(int bufsize, wchar_t *dst) 
{
  ASSERT(dst != NULL);
#ifdef WIN32
  
	wchar_t path[MAX_PATH]=L"";
  SHGetSpecialFolderPathW(NULL, path, CSIDL_FONTS, FALSE);
  WCSCPYN(dst, path, bufsize);


#else
#ifdef PLATFORM_WARNINGS
#warning Std::getFontPath not implemented on LINUX
#endif
#endif

}

const wchar_t wasabi_default_fontnameW[] = WASABI_DEFAULT_FONTNAMEW;
int default_font_scale = 100;
wchar_t default_font[256] =
#ifdef WIN32
WASABI_DEFAULT_FONTNAMEW L".ttf";
#elif defined(__APPLE__)
WASABI_DEFAULT_FONTNAMEW;
#elif defined(LINUX)
  // even tho this isn't the way we'll port this, the style is fun.
"-*-arial-medium-r-*--10-*-*-*-*-*-*-*";
#endif

// gets the filename of a font file guaranteed to be in the system font path.
void Std::getDefaultFont(int bufsize, wchar_t *dst)
{
  ASSERT(dst != NULL);
  WCSCPYN(dst, default_font, bufsize);
}

void Std::setDefaultFont(const wchar_t *newdefaultfont) 
{
  WCSCPYN(default_font, newdefaultfont, 256);
}

int Std::getDefaultFontScale() {
  return default_font_scale;
}

void Std::setDefaultFontScale(int scale) {
  default_font_scale = scale;
}

#ifndef __APPLE__ // PORT ME
int Std::createDirectory(const wchar_t *dirname) 
{
#ifdef WIN32
  if(!CreateDirectoryW(dirname,NULL))
#else
  if(mkdir(dirname, 0755))
#endif
  {
    // create all the path
    PathParserW pp(dirname);
    int l = pp.getNumStrings();
    for(int i=2;i<=l;i++) 
		{
      StringW dir;
      for(int j=0;j<i;j++)
				dir.AppendFolder(pp.enumString(j));
#ifdef WIN32
      CreateDirectoryW(dir,NULL);
#else
      mkdir(dir, 0755);
#endif
    }
  }
  return 1;
}
#endif

int Std::getFileInfos(const char *filename, fileInfoStruct *infos) {
  int retval = 0;
#ifdef WIN32
  HANDLE h;
  WIN32_FIND_DATAA fd;
  if((h=FindFirstFileA(filename, &fd))==INVALID_HANDLE_VALUE) return 0;

  infos->fileSizeHigh=fd.nFileSizeHigh;
  infos->fileSizeLow=fd.nFileSizeLow;
  struct _stati64 statbuf;
  if (_stati64(filename, &statbuf) == -1) return 0;
  infos->lastWriteTime = statbuf.st_mtime;
  infos->readonly = fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
  FindClose(h);

  return 1;
#else
  struct stat st;

  int ret = stat( filename, &st );
  if ( ret != 0 )
    return 0;

  infos->fileSizeHigh = 0;
  infos->fileSizeLow = st.st_size;
  infos->lastWriteTime = st.st_mtime;
  return 1;
#endif
}

void Std::shellExec(const wchar_t *cmd, const wchar_t *params)
{
#ifdef WIN32
  ShellExecuteW(NULL, NULL, cmd, params, L".", SW_SHOWNORMAL);
#else
	system(StringPrintf("%S %S", cmd, params));
#endif
}


#endif // nostudio

void MEMFILL32(void *lptr, unsigned long val, unsigned int n) {
  if (n == 0) return;
#if defined(WIN32) && !defined(_WIN64)
__asm {
  mov eax, val
  mov edi, lptr
  mov ecx, n
  rep stosd
};
#elif defined(GCC)
//http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html ;)
asm ("cld\n\t"
     "rep\n\t"
     "stosl"
     : /* no output registers */
     : "c" (n), "a" (val), "D" (lptr)
     : "%ecx", "%edi" );
#else
  uint32_t *ptr32 = (uint32_t *)lptr;
  for (unsigned int i=0;i!=n;i++)
  {
	 ptr32[i] = val;
  }
#endif
}

void MEMFILL(unsigned short *ptr, unsigned short val, unsigned int n) {
  if (n == 0) return;
  unsigned long v = (unsigned long)val | ((unsigned long)val << 16);
  int r = n & 1;
  MEMFILL32(ptr, v, n/2);
  if (r) ptr[n-1] = val;
}

void MEMCPY32(void *dest, const void *src, size_t words)
{
	// TODO: write fast asm version of this
	memcpy(dest, src, words*4);
}

void MEMCPY_(void *dest, const void *src, size_t n)
{
	memcpy(dest, src, n);
}

void *MALLOC_(size_t size)
{
  ASSERT(size > 0);

	ROUNDUP4(size);
  void *p = malloc(size+4);
 if (p) ALLOCMGR_SIZE(p) = size;
 return ALLOCMGR_PTR_TO_USER_PTR(p);
}

void *REALLOC_(void *ptr, size_t size)
{
  ASSERT(size >= 0);

  if (size == 0) {
    DO_FREE(ptr);

    return NULL;
  }

	ROUNDUP4(size);
  if(ptr == NULL) return MALLOC_(size);

  void *r = NULL;

  size_t oldsize = ALLOCMGR_SIZE(USER_PTR_TO_ALLOCMGR_PTR(ptr));



  r = realloc(USER_PTR_TO_ALLOCMGR_PTR(ptr), size+4);

 // r is an ALLOCMGR pointer

  if (r != NULL) {
    ALLOCMGR_SIZE(r) = size;
    r = ALLOCMGR_PTR_TO_USER_PTR(r);
  } else {
    // realloc failed !
    r = MALLOC(size);
  }

 // r is now a USER pointer
  return r;
}

