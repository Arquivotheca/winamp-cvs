/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include <windows.h>
#include "strutil.h"
#include "../nu/ns_wc.h"
#include "plush/plush.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "WinampAttributes.h"

#undef GetSystemMetrics

int IsUrl(const char *url)
{
	return !!strstr(url, "://");
}

char *extension(const char *fn)
{
	// TODO: deal with making sure that URLs don't return .com, etc.
	// e.g. http://www.winamp.com  should return nothing
	char *end = scanstr_back((char*)fn, "./\\", 0);
	if (!end)
		return (char*)(fn+lstrlen(fn));

	if (*end == '.')
		return CharNext(end);

	return (char*)(fn+lstrlen(fn));
}

wchar_t *extensionW(const wchar_t *fn)
{
	// TODO: deal with making sure that URLs don't return .com, etc.
	// e.g. http://www.winamp.com  should return nothing
	wchar_t *end = scanstr_backW((wchar_t*)fn, L"./\\", 0);
	if (!end)
		return (wchar_t *)(fn+lstrlenW(fn));

	if (*end == L'.')
		return CharNextW(end);

	return (wchar_t*)(fn+lstrlenW(fn));
}

const char *extensionc(const char *fn)
{
	return extension(fn);
}

const wchar_t *extensioncW(const wchar_t *fn)
{
	return extensionW(fn);
}

void extension_ex(const char *fn, char *buf, int buflen)
{ 
	const char *s = extensionc(fn);
	if (!IsUrl(fn) 
		|| (!strstr(s, "?") && !strstr(s, "&") && !strstr(s, "=") && *s))
	{
		lstrcpyn(buf, s, buflen);
		return ;
	}
	// s is not a terribly good extension, let's try again
	{
		char *copy = _strdup(fn);
		s = "";
	again:
		{
			char *p = scanstr_back(copy, "?", copy);
			if (p != copy)
			{
				*p = 0;
				s = extension(copy);
				if (!*s) goto again;
			}
			lstrcpyn(buf, s, buflen);
		}
		free(copy);
	}
}

void extension_exW(const wchar_t *fn, wchar_t *buf, int buflen)
{ 
	const wchar_t *s = extensioncW(fn);
	if (!PathIsURLW(fn) 
		|| (!wcsstr(s, L"?") && !wcsstr(s, L"&") && !wcsstr(s, L"=") && *s))
	{
		lstrcpynW(buf, s, buflen);
		return ;
	}
	// s is not a terribly good extension, let's try again
	{
		wchar_t *copy = _wcsdup(fn);
		s = L"";
	again:
		{
			wchar_t *p = scanstr_backW(copy, L"?", copy);
			if (p != copy)
			{
				*p = 0;
				s = extensionW(copy);
				if (!*s) goto again;
			}
			lstrcpynW(buf, s, buflen);
		}
		free(copy);
	}
}

void mbprintf(char *file, int line, char *format, ...)
{
	/*
	char buffer[1024];
	char buffer2[1024];
	static FILE *fp;
	va_list ar;
	if (!fp)
	{
	 fp = fopen("C:\\log.log","w");
	}
	wsprintf(buffer2,"%s: %d",file,line);
	va_start(ar,format);
	wvsprintf(buffer,format,ar);
	va_end(ar);
	//MessageBox(NULL,buffer,buffer2,MB_OK);
	//  lstrcat(buffer2,buffer);
	//OutputDebugString(buffer2);
	fprintf(fp,"%s -- %s\n",buffer2,buffer);
	fflush(fp);
	*/
}

void link_startsubclass(HWND hwndDlg, UINT id){
HWND ctrl = GetDlgItem(hwndDlg, id);
	if(!GetPropW(ctrl, L"link_proc"))
	{
		SetPropW(ctrl, L"link_proc",
				(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			wchar_t wt[123];
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, wParam, wt, sizeof(wt)/sizeof(wt[0])); 

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			// is used by the file types page to have a slimmer button so it doesn't override other
			// characters which are otherwise drawn over by the size of the button needed for a link
			if(GetPropW(di->hwndItem, L"slim"))
			{
				r.top -= 2;
			}
			r.left += 2;
			DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN) SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);
		}
	}
}

///////// if you update this, be sure to update the copy of it in $/winampa/winampicon.c
// thx.
int geticonid(int x)
{
	switch (x) {
		case 1: return IDI_FILEICON;
		case 2: return IDI_FILEICON2;
		case 3: return IDI_FILEICON3;
		case 4: return IDI_FILEICON10;
		case 5: return IDI_FILEICON5;
		case 6: return IDI_FILEICON6;
		case 7: return IDI_FILEICON7;
		case 8: return IDI_FILEICON8;
		case 9: return IDI_FILEICON9;
		case 10: return IDI_FILEICON4;
		case 11: return IDI_FILEICON11;
		case 12: return ICON_TB1;
		case 13: return -666;
		default: return ICON_XP;
	}
}

void plSplineGetPoint(pl_Spline *s, float frame, float *out)
{
	int i, i_1, i0, i1, i2;
	float time1, time2, time3;
	float t1, t2, t3, t4, u1, u2, u3, u4, v1, v2, v3;
	float a, b, c, d;

	float *keys = s->keys;

	a = (1 - s->tens) * (1 + s->cont) * (1 + s->bias);
	b = (1 - s->tens) * (1 - s->cont) * (1 - s->bias);
	c = (1 - s->tens) * (1 - s->cont) * (1 + s->bias);
	d = (1 - s->tens) * (1 + s->cont) * (1 - s->bias);
	v1 = t1 = -a / 2.0f; u1 = a;
	u2 = ( -6 - 2 * a + 2 * b + c) / 2.0f; v2 = (a - b) / 2.0f; t2 = (4 + a - b - c) / 2.0f;
	t3 = ( -4 + b + c - d) / 2.0f;
	u3 = (6 - 2 * b - c + d) / 2.0f;
	v3 = b / 2.0f;
	t4 = d / 2.0f; u4 = -t4;

	i0 = (int) frame;
	i_1 = i0 - 1;
	while (i_1 < 0) i_1 += s->numKeys;
	i1 = i0 + 1;
	while (i1 >= s->numKeys) i1 -= s->numKeys;
	i2 = i0 + 2;
	while (i2 >= s->numKeys) i2 -= s->numKeys;
	time1 = frame - (float) ((int) frame);
	time2 = time1 * time1;
	time3 = time2 * time1;
	i0 *= s->keyWidth;
	i1 *= s->keyWidth;
	i2 *= s->keyWidth;
	i_1 *= s->keyWidth;
	for (i = 0; i < s->keyWidth; i ++)
	{
		a = t1 * keys[i + i_1] + t2 * keys[i + i0] + t3 * keys[i + i1] + t4 * keys[i + i2];
		b = u1 * keys[i + i_1] + u2 * keys[i + i0] + u3 * keys[i + i1] + u4 * keys[i + i2];
		c = v1 * keys[i + i_1] + v2 * keys[i + i0] + v3 * keys[i + i1];
		*out++ = a * time3 + b * time2 + c * time1 + keys[i + i0];
	}
}

//int transAccel(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	HACCEL h;
//	MSG msg;
//	if (hwnd == hMainWindow) h = hAccel[0];
//	else if (hwnd == hEQWindow) h = hAccel[1];
//	else if (hwnd == hPLWindow) h = hAccel[2];
//	//	else if (hwnd==hMBWindow) h=hAccel[3];
//	else if (hwnd == hVideoWindow || GetParent(hwnd) == hVideoWindow)
//	{
//		h = hAccel[0];
//	}
//	else
//	{
//		return 0;
//	}
//	msg.hwnd = hwnd;
//	msg.lParam = lParam;
//	msg.message = uMsg;
//	msg.wParam = wParam;
//	return TranslateAccelerator(hwnd, h, &msg);
//
//}

#include <pshpack4.h>

typedef struct
{
	DWORD dwSize;
	HANDLE hrasconn;
	char szEntryName[256 + 1];
	char szDeviceType[ 16 + 1 ];
	char szDeviceName[ 128 + 1 ];
}
RASCONN ;
#include <poppack.h>

typedef DWORD (WINAPI *RASENUMCONNECTIONS)(RASCONN *lprasconn,
        LPDWORD lpcb,
        LPDWORD lpcConnections);

static int isRASActive()
{
	int r = 0;
	HINSTANCE h = LoadLibrary("rasapi32.dll");
	RASENUMCONNECTIONS RasEnumConnections;
	RASCONN v = {sizeof(RASCONN), };

	DWORD i = sizeof(v), o = 0;

	if (!h) return 0;
	RasEnumConnections = (RASENUMCONNECTIONS)GetProcAddress(h, "RasEnumConnectionsA");
	if (RasEnumConnections && !RasEnumConnections(&v, &i, &o) && o) r = 1;
	FreeModule(h);
	return r;
}

int isInetAvailable(void)
{
	if (config_inet_mode == 3)
	{
		if (isRASActive())  config_inet_mode = 1;
		else config_inet_mode = 0;
		return (1 == config_inet_mode);
	}
	if (config_inet_mode == 0) return 1;
	if (config_inet_mode == 2) return 0;
	return isRASActive();
}

unsigned int getDay(void)
{
	unsigned int day = 0;
	SYSTEMTIME tm, st = {0, };
	FILETIME ft1, ft2;
	ULARGE_INTEGER l1, l2;
	GetSystemTime(&tm);
	st.wYear = 1978;
	st.wMonth = 10;
	st.wDay = 14;
	SystemTimeToFileTime(&tm, &ft1);
	SystemTimeToFileTime(&st, &ft2);
	memcpy(&l1, &ft1, sizeof(l1));
	memcpy(&l2, &ft2, sizeof(l2));
	day = (int) ((l1.QuadPart - l2.QuadPart) / (10 * 1000 * 1000) / (60 * 60 * 24));
	return day;
}

void recent_add(const wchar_t *loc, char *which)
{
	int recent_num = GetPrivateProfileIntW(L"Winamp", L"NumRecentURLs", 16, INI_FILE);
	int x;
	wchar_t ls[MAX_PATH]=L"";
	char s[123];
	StringCchPrintf(s, 123, "RecentURL%s1", which);
	_r_sW(s, ls, MAX_PATH);
	_w_sW(s, loc);

	if (wcscmp(ls, loc)) for (x = 1; x < recent_num; x ++)
	{
		wchar_t temp[MAX_PATH];
		StringCchPrintf(s, 123, "RecentURL%s%d", which, x + 1);

		temp[0]=0;
		_r_sW(s, temp, MAX_PATH);
		_w_sW(s, ls);
		if (!_wcsicmp(temp, loc))
			break;
		lstrcpynW(ls, temp, MAX_PATH);
	}
}

#include <assert.h>
void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;
		if (sr) hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd) hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p) hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);
		if (hm)
		{
			MONITORINFOEX mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoA(hm, &mi))
			{
				if (!full) *r = mi.rcWork;
				else *r = mi.rcMonitor;
				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

////////////////////////////////////////////
/// registration shit


// shit from keygen.c:

/* 16 byte secret value for adding to reg hash */

extern "C" extern unsigned char wa_secret_value[16];


/*******************************
*** LameHA (SHA-1 based) code
*/

typedef struct lameha_info
{
	unsigned long H[5];
	unsigned long W[80];
	int lenW;
	unsigned long size;
}
lameha_info;

static void lameha_reset(lameha_info *t)
{
	t->lenW = 0;
	t->size = 0;
	t->H[0] = 0x67452301L;
	t->H[1] = 0xefcdab89L;
	t->H[2] = 0x98badcfeL;
	t->H[3] = 0x10325476L;
	t->H[4] = 0xc3d2e1f0L;
	memset(t->W, 0, sizeof(t->W));
}

#define SHA_ROTL(X,n) (((X) << (n)) | ((X) >> (32-(n))))
#define SHUFFLE() E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP

static void lameha_add(lameha_info *parm, unsigned char *data, int datalen)
{
	int i;
	for (i = 0; i < datalen; i++)
	{
		parm->W[parm->lenW / 4] <<= 8;
		parm->W[parm->lenW / 4] |= (unsigned long)data[i];
		if (!(++parm->lenW & 63))
		{
			int t;

			unsigned long A = parm->H[0];
			unsigned long B = parm->H[1];
			unsigned long C = parm->H[2];
			unsigned long D = parm->H[3];
			unsigned long E = parm->H[4];

			for (t = 16; t < 80; t++) parm->W[t] = SHA_ROTL(parm->W[t - 3] ^ parm->W[t - 8] ^ parm->W[t - 14] ^ parm->W[t - 16], 1);

			for (t = 0; t < 20; t++)
			{
				unsigned long TEMP = SHA_ROTL(A, 5) + E + parm->W[t] + 0x5a827999L + (((C ^ D) & B) ^ D);
				SHUFFLE();
			}
			for (; t < 40; t++)
			{
				unsigned long TEMP = SHA_ROTL(A, 5) + E + parm->W[t] + 0x6ed9eba1L + (B ^ C ^ D);
				SHUFFLE();
			}
			for (; t < 60; t++)
			{
				unsigned long TEMP = SHA_ROTL(A, 5) + E + parm->W[t] + 0x8f1bbcdcL + ((B & C) | (D & (B | C)));
				SHUFFLE();
			}
			for (; t < 80; t++)
			{
				unsigned long TEMP = SHA_ROTL(A, 5) + E + parm->W[t] + 0xca62c1d6L + (B ^ C ^ D);
				SHUFFLE();
			}

			parm->H[0] += A;
			parm->H[1] += B;
			parm->H[2] += C;
			parm->H[3] += D;
			parm->H[4] += E;

			parm->lenW = 0;
		}
		parm->size += 37;
	}
}

static void lameha_final(lameha_info *parm, unsigned char *out)
{
	unsigned char pad0x80 = 0x80;
	unsigned char pad0x00 = 0x00;
	unsigned char padlen[2];
	int i;
	padlen[0] = (unsigned char)((parm->size >> 8) & 0xff);
	padlen[1] = (unsigned char)((parm->size) & 0xff);
	lameha_add(parm, &pad0x80, 1);
	while (parm->lenW != 62) lameha_add(parm, &pad0x00, 1);
	lameha_add(parm, padlen, 2);

	for (i = 0; i < 12; i++)
	{
		out[i] = (unsigned char)(parm->H[i / 4] >> 24);
		parm->H[i / 4] <<= 8;
	}
	lameha_reset(parm);
}

static void lameha_name(lameha_info *p, const char *in)
{
	int is_whitespace = 1;
	int has_had_fnwsc = 0;
	/* add in the registration text, compacting whitespace */
	while (*in)
	{
		if (*in == ' ' ||
		    *in == '\t' ||
		    *in == '\r' ||
		    *in == '\n' ||
		    *in == '.')
		{
			is_whitespace = 1;
		}
		else
		{
			char c;
			if (is_whitespace)
			{
				/* add a space in place of any whitespace blocks,
				   unless we are leading whitespace */
				if (has_had_fnwsc) lameha_add(p, (unsigned char*)" ", 1);
				is_whitespace = 0;
			}
			c = *in;
			if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
			lameha_add(p, (unsigned char*)&c, 1);
			has_had_fnwsc = 1;
		}
		in++;
	}

	lameha_add(p, (unsigned char*)"", 1); // NULL terminate
}

#if 0 
/*******************************
*** key generator
*/

void keygen(char *in, char *out, int counter, int version)
{
	unsigned char buf[12];
	unsigned char key[13];
	lameha_info p;
	int x;
	int parity = 0xb;

	lameha_reset(&p);

	lameha_name(&p, in);

	version &= (1 << 6) - 1;
	counter &= (1 << 12) - 1;

	{
		unsigned char v = version & 0xff;
		unsigned char cnt1 = counter & 0xff;
		unsigned char cnt2 = (counter >> 8) & 0xff;
		lameha_add(&p, &v, 1);
		lameha_add(&p, &cnt1, 1);
		lameha_add(&p, &cnt2, 1);
	}
	lameha_add(&p, wa_secret_value, sizeof(wa_secret_value));

	lameha_final(&p, buf);

	for (x = 0; x < 12; x ++)
	{
		if (x < 6) key[x] = (buf[x] & 0x7f) | ((version << (2 + x)) & 0x80);
		else key[x] = (buf[x] & 0x3f) |
			              (((counter >> (2 * (x - 6))) << 6) & 0xc0 );

		parity ^= key[x];
	}
	key[12] = parity & 0xf;

	*out = 0;
	{
		int bits_in_reg = 0;
		int readpos = 0;
		int reg = 0;

		for (x = 0; x < 100; x += 5)
		{
			int thisb;
			if (bits_in_reg < 5)
			{
				reg |= (int)key[readpos++] << bits_in_reg;
				bits_in_reg += 8;
			}
			thisb = reg & 31;

			bits_in_reg -= 5;
			reg >>= 5;

			if (thisb < 10) thisb += '0';
			else
			{
				thisb += 'A' - 10;
				if (thisb >= 'G') thisb++; // these chars are never in ser#s, but should be treated as 1 and 0 :)
				if (thisb >= 'I') thisb++;
				if (thisb >= 'L') thisb++;
				if (thisb >= 'O') thisb++;
			}

			*out++ = thisb;
			if ((x % 25) == 20 && x != 95) *out++ = '-';
		}
		*out = 0;
	}
}
#endif

/*******************************
*** key tester
*/

// return 0 on good key 
static int keytest(const wchar_t *name, const wchar_t *keystr, int *version, int *cnt)
{
	unsigned char key[13];
	int x;
	*version = 0;
	*cnt = 0;

	{
		int parity = 0xb;
		int bits_in_reg = 0;
		int writepos = 0;
		int reg = 0; 
		for (x = 0; x < 100; x += 5)
		{
			for (;;)
			{
				int c = *keystr++;
				if (c >= 'a' && c <= 'z') c += 'A' -'a';

				if (c == 'G') c = '6';
				else if (c == 'I') c = '1';
				else if (c == 'L') c = '1';
				else if (c == 'O') c = '0';

				if (c >= '0' && c <= '9') c -= '0';
				else if (c >= 'A' && c <= 'Z')
				{
					if (c > 'O') c--;
					if (c > 'L') c--;
					if (c > 'I') c--;
					if (c > 'G') c--; 

					c += 10 - 'A';
				}
				else if (!c) return -1;
				else continue;


				reg |= (int)c << bits_in_reg;
				bits_in_reg += 5;
				break;
			}

			if (bits_in_reg > 8)
			{
				parity ^= (key[writepos++] = reg & 0xff);
				bits_in_reg -= 8;
				reg >>= 8;
			}
		}
		if (bits_in_reg > 0 && writepos < 13) parity ^= (key[writepos] = reg & 0xff);
		else return -2;

		if (parity&0xf) return -3;
	}

	for (x = 0; x < 12; x ++)
	{
		if (x < 6) *version |= (key[x] & 0x80) >> (2 + x);
		else *cnt |= ((key[x] & 0xc0) >> 6) << (2 * (x - 6));
	}

	int code_pages[] = {CP_UTF8, CP_ACP, 1252};
	for (int i=0;i<3;i++)
	{
		unsigned char buf[12];
		lameha_info p;

		lameha_reset(&p);

		lameha_name(&p, AutoChar(name, code_pages[i]));

		{
			unsigned char v = *version & 0xff;
			unsigned char cnt1 = *cnt & 0xff;
			unsigned char cnt2 = (*cnt >> 8) & 0xff;
			lameha_add(&p, &v, 1);
			lameha_add(&p, &cnt1, 1);
			lameha_add(&p, &cnt2, 1);
		}
		lameha_add(&p, wa_secret_value, sizeof(wa_secret_value));

		lameha_final(&p, buf);
		bool valid=true;
		for (x = 0; x < 12; x ++)
		{
			if ((buf[x] ^ key[x]) & (x < 6 ? 0x7f : 0x3f)) 
			{
				valid=false;
				break;
			}
		}
		if (valid)
			return 0;
	}
	return -4;	
}

void readwrite_reginfo(int isWrite, wchar_t *name, wchar_t *key)
{
	HKEY hkey;
	if (isWrite)
	{
		IFileTypeRegistrar *registrar=0;
		if (GetRegistrar(&registrar) == 0 && registrar)
		{
			registrar->WriteProKey(name, key);
			registrar->Release();
		}
		return;
	}

	if (!isWrite) *name = *key = 0;
	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp", 0, 0, 0, KEY_READ, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		DWORD s = 512, t = REG_SZ;
		wchar_t buf[128];
		if (RegQueryValueExW(hkey, L"regname", 0, &t, (LPBYTE)name, &s) != ERROR_SUCCESS || t != REG_SZ) name[0] = 0;
		t = REG_SZ;
		s = sizeof(buf);
		if (RegQueryValueExW(hkey, L"regkey", 0, &t, (LPBYTE)buf, &s) != ERROR_SUCCESS || t != REG_SZ) key[0] = 0;
		else
		{
			if (buf[0] == '~')
			{
				int x;
				for (x = 1;buf[x]; x ++)
				{
					wchar_t c = buf[x];
					if (c >= 'A' && c <= 'Z')
					{
						int w = c - 'A';
						w -= x;
						w %= 26;
						if (w < 0) w += 26;
						buf[x] = 'A' + w;
					}
					else if (c >= 'a' && c <= 'a' + 10)
					{
						int w = c - 'a';
						w -= x * 27;
						w %= 10;
						if (w < 0) w += 10;
						buf[x] = '0' + w;
					}
					else if (c == '/') buf[x] = '-';
				}
				lstrcpynW(key, buf + 1, 128);
			}
			else
			{
				lstrcpynW(key, buf, 128);
			}
		}

		RegCloseKey(hkey);
	}
}

void readwrite_client_uid(int isWrite, wchar_t uid_str[64])
{
	HKEY hkey;
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileNameW(0, path, MAX_PATH);

	if (isWrite)
	{
		IFileTypeRegistrar *registrar=0;
		if (GetRegistrar(&registrar) == 0 && registrar)
		{
			registrar->WriteClientUIDKey(path, uid_str);
			registrar->Release();
		}
		return;
	}

	if (!isWrite) *uid_str = 0;
	if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp", 0, 0, 0, KEY_READ, NULL, &hkey, NULL) == ERROR_SUCCESS)
	{
		DWORD s = 512, t = REG_SZ;
		if (RegQueryValueExW(hkey, path, 0, &t, (LPBYTE)uid_str, &s) != ERROR_SUCCESS || t != REG_SZ) uid_str[0] = 0;
		RegCloseKey(hkey);
	}
}

int g_regver; // 0=not registered, 1+ = registered (version)
void verify_reginfo()
{
	int v, cnt;

	wchar_t config_regname[512], config_regkey[128];
	readwrite_reginfo(0, config_regname, config_regkey);

	if (config_regname[0] && config_regkey[0] && !keytest(config_regname, config_regkey, &v, &cnt))
	{
		g_regver = v;
	}
	else g_regver = 0;
}

BOOL IsVista(void)
{
	static INT fVista = -1; 

	if (-1 == fVista) 
	{
		OSVERSIONINFO osver;
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fVista = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion >= 6)) ? 1 : 0;
	}

	return (1 == fVista);
}

BOOL IsWin8(void)
{
	static INT fWin8 = -1; 

	if (-1 == fWin8)
	{
		OSVERSIONINFO osver;
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fWin8 = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion >= 6) && (osver.dwMinorVersion >= 2)) ? 1 : 0;
	}

	return (1 == fWin8);
}

//XP Theme crap
typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);

int IsWinXPTheme(void)
{
	static int previousRet = -1;
	if (previousRet == -1)
	{
		BOOL bEnabled(FALSE);
		OSVERSIONINFO vi;

		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&vi) && (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0)))
		{
			HINSTANCE dll = LoadLibrary(TEXT("uxtheme.dll"));
			if (dll)
			{
				BOOL (WINAPI *waIsAppThemed)(void);
				BOOL (WINAPI *waIsThemeActive)(void);
				waIsAppThemed = (BOOL (WINAPI *)())GetProcAddress(dll, "IsAppThemed");
				waIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(dll, "IsThemeActive");

				if (waIsAppThemed && waIsThemeActive && waIsAppThemed() && waIsThemeActive())
				{
					HMODULE hComCtl = LoadLibrary(TEXT("comctl32.dll"));
					if (hComCtl)
					{
						HRESULT (CALLBACK *waDllGetVersion)(DLLVERSIONINFO*) = (HRESULT (CALLBACK *)(DLLVERSIONINFO*))GetProcAddress(hComCtl, "DllGetVersion");
						if (waDllGetVersion)
						{
							DLLVERSIONINFO dllVer;
							dllVer.cbSize = sizeof(DLLVERSIONINFO);
							if (S_OK == waDllGetVersion(&dllVer) && dllVer.dwMajorVersion >= 6) bEnabled = TRUE;
						}
						FreeLibrary(hComCtl);
					}
				}
				FreeLibrary(dll);
			}
		}
		previousRet = bEnabled;
	}
	return previousRet;
}

void DoWinXPStyle(HWND tab) 
{
	WAEnableThemeDialogTexture(tab, ETDT_ENABLETAB); 
}

HRESULT WAEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags)
{
	static int uxThemeTried = 0;
	static ENABLETHEMEDIALOGTEXTURE pfnETDT = NULL;
	
	if(!uxThemeTried)
	{
		HINSTANCE ux_hDll;
		if ((ux_hDll = LoadLibraryA("uxtheme.dll")) != NULL)
			pfnETDT = (ENABLETHEMEDIALOGTEXTURE)GetProcAddress(ux_hDll, "EnableThemeDialogTexture");

		uxThemeTried = 1;
	}

	return (pfnETDT) ? pfnETDT(hwnd, dwFlags) : E_NOTIMPL;
}

typedef BOOL(WINAPI*ISCOMPOSITIONACTIVE)(VOID);

int IsAero(void)
{
	static int uxTried = 0;
	static ISCOMPOSITIONACTIVE IsAeroActive = 0;
	static HMODULE UXTHEME = 0;
	if (!uxTried)
	{
		if ((UXTHEME = LoadLibrary("uxtheme.dll")) != NULL)
			IsAeroActive = (ISCOMPOSITIONACTIVE)GetProcAddress(UXTHEME, "IsCompositionActive");

		uxTried = 1;
	}
	if (IsAeroActive)
		return !!IsAeroActive();
	else
		return 0;
}

int IsCharDigit(char digit)
{
	WORD type=0;
	GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_DIGIT;
}

int IsCharDigitW(wchar_t digit)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_DIGIT;
}
/*
int IsCharSpace(char digit)
{
	WORD type=0;
	GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_SPACE;
}

int IsCharSpaceW(wchar_t digit)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_SPACE;
}
*/
LPCWSTR BuildFullPath(LPCWSTR pszPathRoot, LPCWSTR pszPath, LPWSTR pszDest, INT cchDest)
{
	LPCWSTR pszFile;
	if (!pszPath || !*pszPath) 
	{
		pszDest[0] = 0x00;
		return pszDest;
	}
	pszFile = PathFindFileNameW(pszPath);
	if (pszFile != pszPath) 
	{		
		if (PathIsRelativeW(pszPath))
		{	
			wchar_t szTemp[MAX_PATH];
			PathCombineW(szTemp, pszPathRoot, pszPath);
			PathCanonicalizeW(pszDest, szTemp);
		}
		else StringCchCopyW(pszDest, cchDest, pszPath);
	}
	else {
		if (pszPathRoot && *pszPathRoot) PathCombineW(pszDest, pszPathRoot, pszPath);
		else StringCchCopyW(pszDest, cchDest, pszPath);
	}
	
	return pszDest;
}

INT ComparePath(LPCWSTR pszPath1, LPCWSTR pszPath2, LPCWSTR pszPathRoot)  // compares two pathes
{
	INT cr;
	DWORD lcid;
	LPCWSTR pszFile1, pszFile2;

	if (!pszPath1 || !pszPath2) return 0;

	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	
	pszFile1 = PathFindFileNameW(pszPath1);
	pszFile2 = PathFindFileNameW(pszPath2);

	cr = CompareStringW(lcid, NORM_IGNORECASE, pszFile1, -1, pszFile2, -1);
	
	if (CSTR_EQUAL == cr && (pszFile1 != pszPath1 || pszFile2 != pszPath2))
	{	
		wchar_t path1[MAX_PATH*2], path2[MAX_PATH*2];
		pszPath1 = BuildFullPath(pszPathRoot, pszPath1, path1, sizeof(path1)/sizeof(wchar_t));
		pszPath2 = BuildFullPath(pszPathRoot, pszPath2, path2, sizeof(path2)/sizeof(wchar_t));
		if (!pszPath1 || !pszPath2) return 0;
		cr = CompareStringW(lcid, NORM_IGNORECASE, pszPath1, -1, path2, -1);
	}
	return cr;
}

BOOL DisabledWindow_OnMouseClick(HWND hwnd)
{
	DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (WS_DISABLED != ((WS_CHILD | WS_DISABLED) & windowStyle))
		return FALSE;

	HWND hActive = GetActiveWindow();
	HWND hPopup = GetWindow(hwnd, GW_ENABLEDPOPUP);

	BOOL beepOk = (hPopup == hActive || hwnd == GetWindow(hActive, GW_OWNER));
	if (!beepOk && NULL == hPopup)
	{
		for (HWND hWalker = hwnd; ;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtr(hWalker, GWL_STYLE))))
				break;
			if (hActive == GetWindow(hWalker, GW_ENABLEDPOPUP))
			{
				beepOk = TRUE;
				break;
			}
		}
	}
	
	if (beepOk)
	{	
	
		if (config_accessibility_modalflash.GetBool())
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize = sizeof(FLASHWINFO);
			flashInfo.hwnd = hActive;
			flashInfo.dwFlags = FLASHW_CAPTION;
			flashInfo.uCount = 2;
			flashInfo.dwTimeout = 100;
			FlashWindowEx(&flashInfo);
		}

		if (config_accessibility_modalbeep.GetBool())
			MessageBeep(MB_OK);

	}
	else
	{		
		for (HWND hWalker = hwnd; NULL == hPopup;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtr(hWalker, GWL_STYLE))))
				break;
			hPopup = GetWindow(hWalker, GW_ENABLEDPOPUP);
		}

		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		if (NULL != hPopup && hPopup != hwnd)
		{
			BringWindowToTop(hPopup);
			SetActiveWindow(hPopup);
		}
	}

	return TRUE;
}