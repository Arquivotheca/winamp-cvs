#ifndef NULLSOFT_IN_CDDA_CDDB_H
#define NULLSOFT_IN_CDDA_CDDB_H

#define S_MULTIPLE		MATCH_MULTIPLE

struct TRACKINFO
{
	TRACKINFO()
	{
		artist=0;
		title=0;
		tagID=0;
		composer=0;
		conductor=0;
		extData=0;
		remixing=0;
		isrc=0;
	}
	TRACKINFO(const TRACKINFO &copy);
	TRACKINFO &operator =(const TRACKINFO &copy);
	~TRACKINFO();
	
	
	
	void Reset();

	wchar_t *artist;
	wchar_t *title;
	wchar_t *tagID;
	wchar_t *composer;
	wchar_t *conductor;
	wchar_t *extData;
	wchar_t *remixing;
	wchar_t *isrc;
};

 struct DINFO
{
	DINFO()
	{
		label=0;
		notes=0;
		genre=0;
		artist=0;
		year=0;
		tuid=0;
		title=0;
		composer=0;
		conductor=0;
		remixing=0;
		compilation=false;
		discnum=0;
		numdiscs=0;
		ntracks=0;
		CDDBID=0;
		nDiscLength=0;
		populated=false;
	}
	DINFO(const DINFO &copy);
	DINFO &operator =(const DINFO &copy);
	~DINFO();
	void Reset();

	wchar_t *title;
	wchar_t *artist;
	wchar_t *tuid;
	wchar_t *year;
	wchar_t *genre;
	wchar_t *label;
	wchar_t *notes;
	bool compilation;
	int discnum;
	int numdiscs;
	int ntracks;
	wchar_t *conductor;
	wchar_t *composer;
	wchar_t *remixing;
	TRACKINFO tracks[130];

	unsigned int CDDBID;
	unsigned int pnFrames[130];
	unsigned int nDiscLength;

	bool populated;

};

extern char config_use_cddb;


#define CDDB_E_BADTOC		0x82FD0001

#define CDDB_NONE				0x0000	// 
#define CDDB_NOCACHE			0x1000	// 
#define CDDB_NOINET				0x2000	// 

#define CDDB_RESOLVE_MULTIPLE	0x0001  // selects first



#define CDDB_UI_MODAL			0x0010	// 
#define CDDB_UI_RESULT_MODAL		0x0020	// result window will stay modal 
#define CDDB_UI_USE_PARENT		0x0040	// instead of creating child window from hwndParent - just reuse hwndParent

#define CDDB_UI_NOMATCH			0x0100	// displays submit new dialog
#define CDDB_UI_MULTIPLE			0x0200	// displays multiple choices dialog

#define AUTOCLOSE_NOW		0x00000000
#define AUTOCLOSE_NEVER		0xFFFFFFFF

int GetDiscID(MCIDEVICEID wDeviceID, DINFO* psDI);
int GetCDDBInfo(DINFO *ps, char device);

#include "CDDBInterface.h"

typedef HRESULT (CALLBACK *CDDB_CB)(HRESULT /*result*/, ICddbDisc* /*pDisc*/, DWORD* /*pdwAutoCloseDelay*/, ULONG_PTR /*user*/);

HRESULT  DoCDDBDlg(DINFO *ps, HWND hwnd, int editopt);
void GetDiscInfo(ICddbDiscPtr pDisc, DINFO *ps);
bool GetRole(ICddbTrack *track, BSTR roleId, BSTR *str);
bool GetRole(ICddbDisc *track, BSTR roleId, BSTR *str);
extern ICDDBControl *pCDDBControl;

void InitializeCddbCache(void);
void UninitializeCddbCache(void);
HRESULT CddbCache_SetDisc(DINFO *pDiscInfo, HRESULT lookupResult);
void Cddb_Initialize(void);
void Cddb_Uninitialize(void);

HRESULT Cddb_InitializeThread(void);
HRESULT Cddb_UninitializeThread(void);

LPCWSTR Cddb_CalculateTOC(DINFO *pDisc, LPWSTR pszTOC, size_t cchTOC);
HRESULT Cddb_DoLookup(LPCWSTR pszTOC, HWND hwndParent, CDDB_CB callback, UINT flags, ULONG_PTR user);

void DefaultValues(DINFO *ps);
//void AddToDatabase(DINFO *disc);
bool DoCDText(DINFO *ps, char device);
//void RemoveFromDatabase(DINFO *disc);
void ShutDownCDDB();
HRESULT Cddb_GetDiscFromCache(BSTR bstrTOC, ICddbDisc **ppDisc);
bool CDEdit(CHAR cDevice, DINFO *ps, HWND hwnd);

HRESULT Cddb_GetIUIOptions(void **ppUIOptions);
HRESULT Cddb_GetIControl(void **ppControl);
HRESULT Cddb_GetICacheManger(void **ppCache);

HRESULT Cddb_DisplayDiscInfo(ICddbDisc *pDisc, CDDBUIFlags *pUIFlags, HWND hwndParent);

void Cddb_GetResultText(HRESULT hr, LPWSTR pszResult, INT cchResult, LPWSTR  pszReason, INT cchReason);


HRESULT Cddb_DisplayResultDlg(HWND hwndParent, HRESULT result, DWORD dwAutoCloseDelay, UINT flags); // flags can be CDDB_MODAL | CDDB_DISPLAY_IN_PARENT

// info calls work only in callback
HWND Cddb_GetInfoWindow(void); 
void Cdbb_DisplayInfo(LPCWSTR pszTitle, LPCWSTR pszCaption, LPCWSTR pszStatus, INT percentCompleted);

void StoreDisc(unsigned int cddb_id, ICddbDiscPtr pDisc);
bool StoreCDText(unsigned int cddb_id, char device);
void StoreCDNoInfo(unsigned int cddb_id);
bool QueryDINFO(unsigned int cddb_id, DINFO *info);
bool StoreDINFO(unsigned cddb_id, DINFO *info);
#endif
