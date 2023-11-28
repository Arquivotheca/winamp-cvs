// the modern file info box

#include "Main.h"
#include "resource.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include <tataki/export.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include <api/service/waservicefactory.h>
#include <api/service/svcs/svc_imgload.h>
#include "language.h"
#ifndef _WIN64
#include "../gracenote/api_gracenote.h"
#endif
#include "../Agave/Language/api_language.h"
#include "decodefile.h"
#include "../nu/AutoLock.h"
#include <api/service/svcs/svc_imgwrite.h>

extern Nullsoft::Utility::LockGuard getMetadataGuard;
#define TEXTBUFFER_MAX	65536
size_t g_fileinfo_openpage=0;

int ModernInfoBox(In_Module * in, const wchar_t * filename, HWND parent);

static INT_PTR CALLBACK FileInfo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FileInfo_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FileInfo_Artwork(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HWND (__cdecl *AddUnifiedFileInfoPane)(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen);

class info_params
{
public:
	const wchar_t * filename;
	In_Module * in;
	nu::HandleList<HWND> tabs;
	info_params(const wchar_t* filename, In_Module * in) : filename(filename), in(in), tabs() {}
};

int ModernInfoBox(In_Module * in, const wchar_t * filename, HWND parent)
{
	Tataki::Init(serviceManager);
	info_params params(_wcsdup(filename), in);
	int ret = (int)LPDialogBoxParamW(IDD_FILEINFO, parent, FileInfo, (LPARAM)&params);
	Tataki::Quit();
	return ret;
}

static VOID WINAPI OnSelChanged(HWND hwndDlg, HWND hwndTab, info_params * p)
{
	ShowWindow(p->tabs[g_fileinfo_openpage], SW_HIDE);
	EnableWindow(p->tabs[g_fileinfo_openpage], 0);
	g_fileinfo_openpage=TabCtrl_GetCurSel(hwndTab);
	ShowWindow(p->tabs[g_fileinfo_openpage], SW_SHOWNA);
	EnableWindow(p->tabs[g_fileinfo_openpage], 1);
	if (GetFocus() != hwndTab)
	{
		SetFocus(p->tabs[g_fileinfo_openpage]);
	}
}

static HWND CreateTab(int n, const wchar_t *file, HWND parent, wchar_t * name, size_t namelen, AddUnifiedFileInfoPane aufip)
{
	switch (n)
	{
	case 0:
		getStringW(IDS_BASICINFO,name,namelen);
		return LPCreateDialogW(IDD_FILEINFO_METADATA,parent,(WNDPROC)FileInfo_Metadata);
	case 1:
		getStringW(IDS_ARTWORK,name,namelen);
		return LPCreateDialogW(IDD_FILEINFO_ARTWORK,parent,(WNDPROC)FileInfo_Artwork);
	default:
		getStringW(IDS_ADVANCED,name,namelen);
		if (aufip) return aufip(n-2,file,parent,name,namelen);
		return NULL;
	}
}

static INT_PTR CALLBACK FileInfo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetPropW(hwndDlg,L"INBUILT_NOWRITEINFO", (HANDLE)0);

		info_params * p = (info_params *)lParam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

		SetDlgItemTextW(hwndDlg,IDC_FN,p->filename);

		AddUnifiedFileInfoPane aufip = (AddUnifiedFileInfoPane)GetProcAddress(p->in->hDllInstance, "winampAddUnifiedFileInfoPane");

		HWND hwndTab = GetDlgItem(hwndDlg,IDC_TAB1);
		wchar_t name[100];
		TCITEMW tie;
		tie.mask = TCIF_TEXT;
		tie.pszText = name;
		HWND tab=NULL;
		int n=0;

		while (NULL != (tab = CreateTab(n, p->filename, hwndDlg, name, 100, aufip)))
		{
			p->tabs.push_back(tab);
			ShowWindow(tab,SW_HIDE);

			if (!SendMessage(hMainWindow,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
				SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)tab,IPC_USE_UXTHEME_FUNC);

			SendMessageW(hwndTab, TCM_INSERTITEMW, n, (LPARAM)&tie);
			n++;
		}

		RECT r;
		GetWindowRect(hwndTab,&r);
		TabCtrl_AdjustRect(hwndTab,FALSE,&r);
		MapWindowPoints(NULL,hwndDlg,(LPPOINT)&r,2);
		r.left += 3;
		r.top += 2;
		r.right -= 4;

		for (size_t i=0; i < p->tabs.size(); i++)
			SetWindowPos(p->tabs[i],HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);

		if (g_fileinfo_openpage >= p->tabs.size()) g_fileinfo_openpage = 0;

		TabCtrl_SetCurSel(hwndTab,g_fileinfo_openpage);

		ShowWindow(p->tabs[g_fileinfo_openpage], SW_SHOWNA);

		// show alt+3 window and restore last position as applicable
		POINT pt = {alt3_rect.left, alt3_rect.top};
		if (!windowOffScreen(hwndDlg, pt))
			SetWindowPos(hwndDlg, HWND_TOP, alt3_rect.left, alt3_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);

		return 1;
	}
	break;
	case WM_NOTIFY:
	{
		LPNMHDR lpn = (LPNMHDR) lParam;
		if (lpn && lpn->code==TCN_SELCHANGE)
		{
			info_params * p = (info_params *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			OnSelChanged(hwndDlg,GetDlgItem(hwndDlg,IDC_TAB1),p);
		}
	}
	break;
	case WM_USER:
	{
		info_params * p = (info_params *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		for (size_t i=0; i < p->tabs.size(); i++)
			SendMessage(p->tabs[i],uMsg,wParam,lParam);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			Nullsoft::Utility::AutoLock metadata_lock(getMetadataGuard);
			info_params * p = (info_params *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			for (size_t i=0; i < p->tabs.size(); i++)
			{
				SendMessage(p->tabs[i],uMsg,wParam,lParam);
			}
			GetWindowRect(hwndDlg, &alt3_rect);
			EndDialog(hwndDlg,0);
			free((void*)p->filename);
		}
		break;
		case IDCANCEL:
		{
			Nullsoft::Utility::AutoLock metadata_lock(getMetadataGuard);
			info_params * p = (info_params *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			for (size_t i=0; i < p->tabs.size(); i++)
			{
				SendMessage(p->tabs[i],uMsg,wParam,lParam);
			}
			GetWindowRect(hwndDlg, &alt3_rect);
			EndDialog(hwndDlg,1);
			free((void*)p->filename);
		}
		break;
		}
		break;
	case WM_CLOSE:
		return FileInfo(hwndDlg,WM_COMMAND,MAKEWPARAM(IDCANCEL,0),0);
	}
	return 0;
}

const wchar_t *genres[] =
{
	L"Blues", L"Classic Rock", L"Country", L"Dance", L"Disco", L"Funk", L"Grunge", L"Hip-Hop",
	L"Jazz", L"Metal", L"New Age", L"Oldies", L"Other", L"Pop", L"R&B", L"Rap", L"Reggae", L"Rock",
	L"Techno", L"Industrial", L"Alternative", L"Ska", L"Death Metal", L"Pranks", L"Soundtrack",
	L"Euro-Techno", L"Ambient", L"Trip-Hop", L"Vocal", L"Jazz+Funk", L"Fusion", L"Trance",
	L"Classical", L"Instrumental", L"Acid", L"House", L"Game", L"Sound Clip", L"Gospel", L"Noise",
	L"Alt. Rock", L"Bass", L"Soul", L"Punk", L"Space", L"Meditative", L"Instrumental Pop",
	L"Instrumental Rock", L"Ethnic", L"Gothic", L"Darkwave", L"Techno-Industrial",
	L"Electronic", L"Pop-Folk", L"Eurodance", L"Dream", L"Southern Rock", L"Comedy", L"Cult",
	L"Gangsta Rap", L"Top 40", L"Christian Rap", L"Pop/Funk", L"Jungle", L"Native American",
	L"Cabaret", L"New Wave", L"Psychedelic", L"Rave", L"Showtunes", L"Trailer", L"Lo-Fi", L"Tribal",
	L"Acid Punk", L"Acid Jazz", L"Polka", L"Retro", L"Musical", L"Rock & Roll", L"Hard Rock",
	L"Folk", L"Folk-Rock", L"National Folk", L"Swing", L"Fast-Fusion", L"Bebop", L"Latin", L"Revival",
	L"Celtic", L"Bluegrass", L"Avantgarde", L"Gothic Rock", L"Progressive Rock", L"Psychedelic Rock",
	L"Symphonic Rock", L"Slow Rock", L"Big Band", L"Chorus", L"Easy Listening", L"Acoustic", L"Humour",
	L"Speech", L"Chanson", L"Opera", L"Chamber Music", L"Sonata", L"Symphony", L"Booty Bass", L"Primus",
	L"Porn Groove", L"Satire", L"Slow Jam", L"Club", L"Tango", L"Samba", L"Folklore",
	L"Ballad", L"Power Ballad", L"Rhythmic Soul", L"Freestyle", L"Duet", L"Punk Rock", L"Drum Solo",
	L"A Cappella", L"Euro-House", L"Dance Hall", L"Goa", L"Drum & Bass", L"Club-House",
	L"Hardcore", L"Terror", L"Indie", L"BritPop", L"Afro-Punk", L"Polsk Punk", L"Beat",
	L"Christian Gangsta Rap", L"Heavy Metal", L"Black Metal", L"Crossover", L"Contemporary Christian",
	L"Christian Rock", L"Merengue", L"Salsa", L"Thrash Metal", L"Anime", L"JPop", L"Synthpop",
	L"Abstract", L"Art Rock", L"Baroque", L"Bhangra", L"Big Beat", L"Breakbeat", L"Chillout", L"Downtempo", L"Dub", L"EBM", L"Eclectic", L"Electro",
	L"Electroclash", L"Emo", L"Experimental", L"Garage", L"Global", L"IDM", L"Illbient", L"Industro-Goth", L"Jam Band", L"Krautrock", L"Leftfield", L"Lounge",
	L"Math Rock", L"New Romantic", L"Nu-Breakz", L"Post-Punk", L"Post-Rock", L"Psytrance", L"Shoegaze", L"Space Rock", L"Trop Rock", L"World Music", L"Neoclassical",
	L"Audiobook", L"Audio Theatre", L"Neue Deutsche Welle", L"Podcast", L"Indie Rock", L"G-Funk", L"Dubstep", L"Garage Rock", L"Psybient",
	L"Glam Rock", L"Dream Pop", L"Merseybeat", L"K-Pop", L"Chiptune", L"Grime", L"Grindcore", L"Indietronic", L"Indietronica", L"Jazz Rock", L"Jazz Fusion", L"Post-Punk Revival", L"Electronica", L"Psychill", L"Ethnotronic", L"Americana", L"Ambient Dub", L"Digital Dub", L"Chillwave", L"Stoner Rock",
};

const size_t numberOfGenres = sizeof(genres) / sizeof(genres[0]);

const wchar_t * strfields[] =
{
	L"artist",
	L"album",
	L"albumartist",
	L"title",
	L"year",
	L"genre",
	L"comment",
	L"composer",
	L"publisher",
	L"disc",
	L"track",
	L"bpm",
	L"GracenoteFileID",
	L"GracenoteExtData"
};

const int strfieldctrls[] =
{
	IDC_ARTIST,
	IDC_ALBUM,
	IDC_ALBUM_ARTIST,
	IDC_TITLE,
	IDC_YEAR,
	IDC_GENRE,
	IDC_COMMENT,
	IDC_COMPOSER,
	IDC_PUBLISHER,
	IDC_DISC,
	IDC_TRACK,
	IDC_BPM,
	IDC_GN_FILEID,
	IDC_GN_EXTDATA
};

const int staticstrfieldctrls[] =
{
	IDC_STATIC_ARTIST,
	IDC_STATIC_ALBUM,
	IDC_STATIC_ALBUM_ARTIST,
	IDC_STATIC_TITLE,
	IDC_STATIC_YEAR,
	IDC_STATIC_GENRE,
	IDC_STATIC_COMMENT,
	IDC_STATIC_COMPOSER,
	IDC_STATIC_PUBLISHER,
	IDC_STATIC_DISC,
	IDC_STATIC_TRACK,
	IDC_STATIC_BPM,
	0,
	0
};
#ifndef _WIN64
static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	punk->Release();
	return pcp;
}

class autoTagListen : public _ICDDBMusicIDManagerEvents
{
public:
	autoTagListen(api_gracenote* gn,	ICDDBMusicIDManager3 *musicid, wchar_t *_gracenoteFileId) : gn(gn),musicid(musicid),done(0), abort(0), gracenoteFileId(_gracenoteFileId)
	{
	}

	HRESULT OnTrackIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long* _Abort)
	{
		if (abort) *_Abort=1;
		/*switch(Status)
		{
		case STATUS_MUSICID_Error:
		case STATUS_MUSICID_ProcessingFile:
		case STATUS_MUSICID_LookingUpWaveForm:
		case STATUS_MUSICID_LookingUpText:
		case STATUS_MUSICID_CheckForAbort:
		case STATUS_ALBUMID_Querying:
		case STATUS_ALBUMID_Queried:
		case STATUS_ALBUMID_Processing:
		case STATUS_ALBUMID_Processed:
		case STATUS_MUSICID_AnalyzingWaveForm:
		case STATUS_ALBUMID_QueryingWF:
		}*/
		// do shit
		return S_OK;
	}
	HRESULT OnTrackIDComplete(LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut)
	{
		done=1;
		if (match_code <= 1)
		{
			char title[16];
			if (!abort) MessageBox(h, getString(IDS_NO_MATCH_FOUND, NULL, 0),
				                       getString(IDS_FAILED, title, 16), 0);
			EndDialog(hwndDlg,0);
			return S_OK;
		}
		long num;
		pListOut->get_Count(&num);
		if (!num) return S_OK;
		ICddbFileInfoPtr infotag;

		pListOut->GetFileInfo(1,&infotag);

		if (infotag)
		{
			ICddbFileTagPtr tag;
			ICddbDisc2Ptr disc;

			infotag->get_Tag(&tag);
			infotag->get_Disc(&disc);

			ICddbFileTag2_5Ptr tag2_5;
			tag->QueryInterface(&tag2_5);

			ICddbDisc2_5Ptr disc2_5;
			disc->QueryInterface(&disc2_5);

			wchar_t buf[2048];
			BSTR bstr=buf;
#define PUTINFO(id, ctrl) tag->get_ ## id ## (&bstr); SetDlgItemTextW(h, ctrl, bstr);
#define PUTINFO2(id, ctrl) tag2_5->get_ ## id ## (&bstr); SetDlgItemTextW(h, ctrl, bstr);
			PUTINFO(LeadArtist, IDC_ARTIST);
			PUTINFO(Album, IDC_ALBUM);
			PUTINFO(Title, IDC_TITLE);
			PUTINFO(Album, IDC_ALBUM);

			if (disc2_5 == NULL
			    || (FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(3, &bstr))
			        && FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(2, &bstr))
			        && FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(1, &bstr))
			        && FAILED(disc2_5->get_V2GenreStringPrimary(&bstr)))
			   )
			{
				PUTINFO(Genre, IDC_GENRE);
			}
			else
			{
				SetDlgItemTextW(h,IDC_GENRE,bstr);
			}
			// sending this will ensure that the genre is applied other than in the metadata page
			SendMessage(GetParent(h),WM_USER,(WPARAM)strfields[5],(LPARAM)bstr);

			PUTINFO(Year, IDC_YEAR);
			PUTINFO(Label, IDC_PUBLISHER);
			PUTINFO(BeatsPerMinute, IDC_BPM);
			PUTINFO(TrackPosition, IDC_TRACK);
			PUTINFO(PartOfSet, IDC_DISC);
			PUTINFO2(Composer, IDC_COMPOSER);
			PUTINFO2(DiscArtist, IDC_ALBUM_ARTIST);
			PUTINFO(FileId, IDC_GN_FILEID);
			PUTINFO2(ExtDataSerialized, IDC_GN_EXTDATA);

#undef PUTINFO
#undef PUTINFO2
			// CRASH POINT
			// This is the starting cause of crashing in the cddb stuff
			// Without this or with a manual AddRef() before and it'll work ok
			//infotag->Release();
		}
		EndDialog(hwndDlg,0);
		return S_OK;
	}
	HRESULT FillTag(ICddbFileInfo *info, BSTR filename)
	{
#define PUTINFO(id, ctrl) GetDlgItemTextW(h, ctrl, buf, 2048); if(buf[0]) infotag->put_ ## id ## (buf);
#define PUTINFO2(id, ctrl) GetDlgItemTextW(h, ctrl, buf, 2048); if(buf[0]) tag2_5->put_ ## id ## (buf);

		ICddbID3TagPtr infotag;
		infotag.CreateInstance(CLSID_CddbID3Tag);
		ICddbFileTag2_5Ptr tag2_5;
		infotag->QueryInterface(&tag2_5);
		wchar_t buf[2048];

		PUTINFO(LeadArtist, IDC_ARTIST);
		PUTINFO(Album, IDC_ALBUM);
		PUTINFO(Title, IDC_TITLE);
		PUTINFO(Album, IDC_ALBUM);
		PUTINFO(Genre, IDC_GENRE);
		PUTINFO(Year, IDC_YEAR);
		PUTINFO(Label, IDC_PUBLISHER);
		PUTINFO(BeatsPerMinute, IDC_BPM);
		PUTINFO(TrackPosition, IDC_TRACK);
		PUTINFO(PartOfSet, IDC_DISC);
		PUTINFO2(Composer, IDC_COMPOSER);
		PUTINFO2(DiscArtist, IDC_ALBUM_ARTIST);

		in_get_extended_fileinfoW(filename,L"length",buf,2048);
		tag2_5->put_LengthMS(buf);

		if (gracenoteFileId && *gracenoteFileId)
			infotag->put_FileId(gracenoteFileId);

#undef PUTINFO
#undef PUTINFO2
		info->put_Tag(infotag);
		return S_OK;
	}

	STDMETHODIMP STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObject)
	{
		if (!ppvObject)
			return E_POINTER;

		else if (IsEqualIID(riid, __uuidof(_ICDDBMusicIDManagerEvents)))
			*ppvObject = (_ICDDBMusicIDManagerEvents *)this;
		else if (IsEqualIID(riid, IID_IDispatch))
			*ppvObject = (IDispatch *)this;
		else if (IsEqualIID(riid, IID_IUnknown))
			*ppvObject = this;
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
	{
		switch (dispid)
		{
		case 1: // OnTrackIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long* Abort
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			BSTR filename = pdispparams->rgvarg[1].bstrVal;
			CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[2].lVal;
			return OnTrackIDStatusUpdate(status,filename,abort);
		}
		case 3: // OnTrackIDComplete, params: LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut
		{
			IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
			IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
			long match_code = pdispparams->rgvarg[2].lVal;

			ICddbFileInfoPtr pInfoIn;
			ICddbFileInfoListPtr matchList;
			disp1->QueryInterface(&matchList);
			disp2->QueryInterface(&pInfoIn);

			return OnTrackIDComplete(match_code,pInfoIn,matchList);
		}
		case 10: // OnGetFingerprintInfo
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;
			ICddbFileInfo *info;
			disp->QueryInterface(&info);
			extern DecodeFile *decodeFile;
			return gn->CreateFingerprint(musicid, decodeFile, info, filename, abort);
		}
		break;
		case 11: // OnGetTagInfo
		{
			//long *Abort = pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;

			ICddbFileInfo *info;
			disp->QueryInterface(&info);
			return FillTag(info, filename);
		}
		break;
		}
		return DISP_E_MEMBERNOTFOUND;
	}
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
	{
		*rgdispid = DISPID_UNKNOWN; return DISP_E_UNKNOWNNAME;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR * pctinfo)
	{
		return E_NOTIMPL;
	}

	HWND hwndDlg,h;
	api_gracenote* gn;
	ICDDBMusicIDManager3 *musicid;
	int done;
	long abort;
	wchar_t *gracenoteFileId;
};

static INT_PTR CALLBACK FileInfo_Autotagging(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// have the close button disabled otherwise it might cause confusion
		EnableMenuItem(GetSystemMenu(hwndDlg, 0), SC_CLOSE, MF_BYCOMMAND|MF_GRAYED);
		SetWindowLong(hwndDlg,GWLP_USERDATA,lParam);
		autoTagListen * p = (autoTagListen *)lParam;
		p->hwndDlg = hwndDlg;
		if (p->done) EndDialog(hwndDlg,0);
	}
	break;
	}
	return 0;
}
#endif
LPCWSTR RepairMutlilineString(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 1)
		return NULL;

	LPWSTR temp = (WCHAR*)malloc(sizeof(WCHAR) * cchBufferMax);
	if (NULL == temp) return NULL;

	LPWSTR p1, p2;
	p1 = pszBuffer;
	p2 = temp;

	INT cchLen;
	for (cchLen = 0; L'\0' != *p1 && ((p2 - temp) < cchBufferMax); cchLen++)
	{
		if(*p1 == L'\n')
		{
			*p2++ = L'\r';
			cchLen++;
		}
		*p2++ = *p1++;
	}
	CopyMemory(pszBuffer, temp, sizeof(WCHAR) * cchLen);
	pszBuffer[cchLen] = L'\0';
	free(temp);
	return pszBuffer;
}

static INT_PTR CALLBACK FileInfo_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int my_change=0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		my_change=1;

		int y;
		SendMessage(GetDlgItem(hwndDlg, IDC_GENRE), CB_RESETCONTENT, (WPARAM)0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETCURSEL, (WPARAM)-1, 0);
		for (size_t x = 0; x != numberOfGenres; x ++)
		{
			y = SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_ADDSTRING, 0, (LPARAM)genres[x]);
			SendMessage(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETITEMDATA, y, x);
		}
		y = SendMessage(GetDlgItem(hwndDlg, IDC_GENRE), CB_ADDSTRING, 0, (LPARAM)L"");
		SendMessage(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETITEMDATA, y, 255);

		info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);

		LPWSTR pszBuffer = (LPWSTR)malloc(sizeof(WCHAR) * TEXTBUFFER_MAX);


		for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
		{
			pszBuffer[0]=0;
			int result = in_get_extended_fileinfoW(p->filename,strfields[i],pszBuffer,TEXTBUFFER_MAX);
			SetDlgItemTextW(hwndDlg, strfieldctrls[i], pszBuffer);
			EnableWindow(GetDlgItem(hwndDlg, strfieldctrls[i]), result?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, staticstrfieldctrls[i]), result?TRUE:FALSE);
		}

		pszBuffer[0]=0;
		in_get_extended_fileinfoW(p->filename,L"formatinformation",pszBuffer, TEXTBUFFER_MAX);
		// due to quirks with the more common resource editors, is easier to just store the string
		// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
		// on new lines as is wanted (silly multiline edit controls)
		SetDlgItemTextW(hwndDlg, IDC_FORMATINFO, RepairMutlilineString(pszBuffer, TEXTBUFFER_MAX));

		wchar_t tg[64]=L"", ag[64]=L"";
		in_get_extended_fileinfoW(p->filename,L"replaygain_track_gain",tg,64);
		in_get_extended_fileinfoW(p->filename,L"replaygain_album_gain",ag,64);

		if (!tg[0]) getStringW(IDS_NOTPRESENT,tg,64);
		else
		{
			// this isn't nice but it localises the RG values
			// for display as they're saved in the "C" locale
			double value = _wtof_l(tg,langManager->Get_C_NumericLocale());
			StringCchPrintfW(tg,64,L"%-+.2f dB", value);
		}
		if (!ag[0]) getStringW(IDS_NOTPRESENT,ag,64);
		else
		{
			// this isn't nice but it localises the RG values
			// for display as they're saved in the "C" locale
			double value = _wtof_l(ag,langManager->Get_C_NumericLocale());
			StringCchPrintfW(ag,64,L"%-+.2f dB", value);
		}
		wchar_t tgagstr[128] = {0};
		StringCbPrintfW(pszBuffer, TEXTBUFFER_MAX, getStringW(IDS_TRACK_GAIN_AND_ALBUM_GAIN,tgagstr,128),tg,ag);
		SetDlgItemTextW(hwndDlg, IDC_REPLAYGAIN, RepairMutlilineString(pszBuffer, TEXTBUFFER_MAX));
		my_change=0;
		free(pszBuffer);

		// test for the musicid feature not being available and remove
		// the autotag button as required (useful for lite installs)
		ICDDBMusicIDManager3 *musicid = NULL;
		waServiceFactory *factory = WASABI_API_SVC?WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID):0;
		api_gracenote* gn = NULL;
		int remove = FALSE;
		if(factory){
			gn = (api_gracenote *)factory->getInterface();
			if(gn){
				musicid = gn->GetMusicID();
				if(musicid){
					musicid->Shutdown();
					musicid->Release();
				}
				else{
					remove = TRUE;
				}
				factory->releaseInterface(gn);
			}
			else{
				remove = TRUE;
			}
		}
		if(remove){
			DestroyWindow(GetDlgItem(hwndDlg,IDC_AUTOTAG));
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
#ifndef _WIN64
		case IDC_AUTOTAG:
		{
			ICDDBMusicIDManager3 *musicid = NULL;
			waServiceFactory *factory = WASABI_API_SVC?WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID):0;
			api_gracenote* gn = NULL;
			if (factory)
			{
				gn = (api_gracenote *)factory->getInterface();
				if (gn)
				{
					musicid = gn->GetMusicID();
				}
			}
			if (!musicid)
			{
				wchar_t title[32];
				if (gn) factory->releaseInterface(gn);
				MessageBoxW(hwndDlg, getStringW(IDS_GRACENOTE_TOOLS_NOT_INSTALLED, NULL, 0),
				            getStringW(IDS_ERROR, title, 32),0);
				break;
			}
			// we have the musicid pointer, so lets try and tag this mother.
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			
			if (NULL == p  || NULL == p->filename || L'\0' == p->filename)
				break;

			// first, see if there's a pre-existing gracenote file id
			wchar_t fileid[1024]=L"";
			extendedFileInfoStructW gracenote_info;
			gracenote_info.filename = p->filename;
			gracenote_info.metadata = L"GracenoteFileID";
			gracenote_info.ret = fileid;
			gracenote_info.retlen = 1024;
			if (0 == SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&gracenote_info, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
				fileid[0] = L'\0';

			ICddbFileInfoPtr info = 0;
			info.CreateInstance(CLSID_CddbFileInfo);
			info->put_Filename((wchar_t*)p->filename);
			ICddbFileInfoList *dummy=0;
			long match_code=666;
			IConnectionPoint *icp = GetConnectionPoint(musicid, DIID__ICDDBMusicIDManagerEvents);

			DWORD m_dwCookie = 0;
			autoTagListen listen(gn,musicid, fileid);
			listen.h = hwndDlg;
			if (icp) icp->Advise(static_cast<IDispatch *>(&listen), &m_dwCookie);

			musicid->TrackID(info, MUSICID_LOOKUP_ASYNC|MUSICID_RETURN_SINGLE|MUSICID_GET_TAG_FROM_APP|MUSICID_GET_FP_FROM_APP|MUSICID_PREFER_WF_MATCHES, &match_code, &dummy);
			if (dummy)
				dummy->Release();

			LPDialogBoxParamW(IDD_AUTOTAGGING,hwndDlg,FileInfo_Autotagging,(LPARAM)&listen);

			musicid->Shutdown();
			musicid->Release();
			factory->releaseInterface(gn);
		}
		break;
#endif
		case IDOK:
			if (!GetPropW(GetParent(hwndDlg),L"INBUILT_NOWRITEINFO"))
			{
				info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);

				LPWSTR pszBuffer = (LPWSTR)malloc(sizeof(WCHAR) * TEXTBUFFER_MAX);

				for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
				{
					if (!GetDlgItemTextW(hwndDlg,strfieldctrls[i],pszBuffer, TEXTBUFFER_MAX))
						pszBuffer[0] = L'\0';
					in_set_extended_fileinfoW(p->filename,strfields[i], pszBuffer);
				}
				if (NULL != pszBuffer)
					free(pszBuffer);

				if (in_write_extended_fileinfo() == 0)
				{
					wchar_t title[256];
					MessageBoxW(hwndDlg,
						getStringW(IDS_METADATA_ERROR, NULL, 0),
						getStringW(IDS_METADATA_ERROR_TITLE, title, 256),						
						MB_OK | MB_ICONWARNING);
				}
			}
			break;
		default:
			if (!my_change && (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_EDITUPDATE))
			{
				my_change=1;
				for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
				{
					if (LOWORD(wParam) == strfieldctrls[i])
					{
						if (HIWORD(wParam) != CBN_SELCHANGE)
						{
							LPWSTR pszBuffer = (LPWSTR)malloc(sizeof(WCHAR) * TEXTBUFFER_MAX);

							GetDlgItemTextW(hwndDlg,strfieldctrls[i], pszBuffer, TEXTBUFFER_MAX);
							SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)pszBuffer);
							if (NULL != pszBuffer)
								free(pszBuffer);
						}
						else
						{
							int n = SendMessage(GetDlgItem(hwndDlg, strfieldctrls[i]), CB_GETCURSEL, 0, 0);
							int m = SendMessage(GetDlgItem(hwndDlg, strfieldctrls[i]), CB_GETITEMDATA, n, 0);
							if (m>=0 && m<numberOfGenres)
							{
								SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)genres[m]);
							}
							// handles case where genre is cleared
							else if (!n && m == 255)
							{
								SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)L"");
							}
							else if(n == CB_ERR)
							{
								// if we got here then it is likely to be from a genre not in the built in list
								LPWSTR pszBuffer = (LPWSTR)malloc(sizeof(WCHAR) * TEXTBUFFER_MAX);
								if(GetDlgItemTextW(hwndDlg,strfieldctrls[i], pszBuffer, TEXTBUFFER_MAX)){
									SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)pszBuffer);
								}
								if (NULL != pszBuffer)
									free(pszBuffer);
							}
						}
					}
				}
				my_change=0;
			}
		}
		break;
	case WM_USER:
		if (wParam && lParam && !my_change)
		{
			for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
				if (_wcsicmp((wchar_t*)wParam,strfields[i])==0)
					SetDlgItemTextW(hwndDlg,strfieldctrls[i],(wchar_t*)lParam);
		}
		break;
	}
	return 0;
}

static void Adjust(int bmpw, int bmph, int &x, int &y, int &w, int &h)
{
	// maintain 'square' stretching
	double aspX = (double)(w)/(double)bmpw;
	double aspY = (double)(h)/(double)bmph;
	double asp = min(aspX, aspY);
	int newW = (int)(bmpw*asp);
	int newH = (int)(bmph*asp);
	x = (w - newW)/2;
	y = (h - newH)/2;
	w = newW;
	h = newH;
}

static HBITMAP getBitmap(ARGB32 * data, int w, int h, HWND parent)
{
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = w;
	info.bmiHeader.biHeight = -h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = GetDC(parent);
	HBITMAP bm = CreateCompatibleBitmap(dc,w,h);
	SetDIBits(dc,bm,0,h,data,&info,DIB_RGB_COLORS);
	ReleaseDC(parent,dc);
	return bm;
}

// these two are also used by AlbumArtRetrival.cpp
ARGB32 * decompressImage(const void *data, int datalen, int * dataW, int * dataH)
{
	ARGB32* ret=NULL;
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->testData(data,datalen))
				{
					ret = l->loadImage(data,datalen,dataW,dataH);
					sf->releaseInterface(l);
					break;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return ret;
}

HBITMAP getBitmap(ARGB32 * data, int dw, int dh, int targetW, int targetH, HWND parent)
{
	HQSkinBitmap bm(data,dw,dh);
	int x=0,y=0,w=targetW,h=targetH;
	Adjust(dw,dh,x,y,w,h);
	BltCanvas canv(targetW,targetH);
	bm.stretch(&canv,x,y,w,h);
	return getBitmap((ARGB32*)canv.getBits(),targetW,targetH,parent);
}

void EnableArtFrame(HWND hwnd, BOOL enable)
{
	const int artFrameElements[] =
	{
		IDC_STATIC_FRAME,
		IDC_ARTHOLDER,
		IDC_BUTTON_CHANGE,
		IDC_BUTTON_SAVEAS,
		IDC_BUTTON_COPY,
		IDC_BUTTON_PASTE,
		IDC_BUTTON_DELETE,
	};
	for (int i=0; i<sizeof(artFrameElements)/sizeof(int); i++)
		EnableWindow(GetDlgItem(hwnd,artFrameElements[i]),enable);
	SetDlgItemTextW(hwnd,IDC_STATIC_FRAME,getStringW(IDS_NO_IMAGE,0,0));
}

class EditArtItem
{
public:
	ARGB32 * bits;
	int w;
	int h;
	void *data;
	size_t datalen;
	wchar_t *mimetype;
	bool dirty;
	EditArtItem(ARGB32 *bits, int w, int h, bool dirty=false) : bits(bits), w(w), h(h), dirty(dirty), data(0), datalen(0), mimetype(0) {}
	~EditArtItem()
	{
		if (bits) WASABI_API_MEMMGR->sysFree(bits);
		free(mimetype);
		if (data) WASABI_API_MEMMGR->sysFree(data);
	}
};

static EditArtItem * getCurrentArtItem(HWND hwndDlg)
{
	int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
	if (sel == -1)
		return NULL;
	EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,sel,0);
	if (e != (EditArtItem *)-1)
		return e;
	return NULL;
}

static void GetSize(HBITMAP bm, int &w, int &h, HWND hwndDlg)
{
	HDC dc = GetDC(hwndDlg);
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(dc,bm,0,0,NULL,&info,DIB_RGB_COLORS);
	w = abs(info.bmiHeader.biWidth);
	h = abs(info.bmiHeader.biHeight);
	ReleaseDC(hwndDlg,dc);
}

static bool AddNewArtItem(HWND hwndDlg)
{
	wchar_t buf[100]=L"";
	GetDlgItemTextW(hwndDlg,IDC_COMBO_ARTTYPE,buf,100);
	if (!buf[0]) return false;
	int s=SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_FINDSTRINGEXACT, (WPARAM)-1,(LPARAM)buf);

	if (s != LB_ERR && SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0))
	{
		// user has selected something already in our list
		SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,s,0);
		SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
		return true;
	}
	// let's add this new item
	s=SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)buf);
	if (s == LB_ERR) return false;
	EditArtItem *e = new EditArtItem(0,0,0);
	SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETITEMDATA,s,(LPARAM)e);
	SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,s,0);
	EnableArtFrame(hwndDlg,TRUE);
	return true;
}

static svc_imageLoader *FindImageLoader(const wchar_t *filespec, waServiceFactory **factory)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{	
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->isMine(filespec))
				{
					*factory = sf;
					return l;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}


static ARGB32 *loadImgFromFile(const wchar_t *file, int *len, int *w, int *h, wchar_t ** mime, ARGB32** imageData)
{
	waServiceFactory *sf;
	svc_imageLoader *loader = FindImageLoader(file, &sf);
	if (loader)
	{
		HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hf != INVALID_HANDLE_VALUE)
		{
			*len = GetFileSize(hf, 0);
			HANDLE hmap = CreateFileMapping(hf, 0, PAGE_READONLY, 0, 0, 0);
			if (hmap)
			{
				void *data = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
				if (data)
				{
					if (loader->testData(data,*len))
					{
						ARGB32* im = loader->loadImage(data,*len,w,h);
						if (im && !_wcsicmp(loader->mimeType(), L"image/jpeg"))
						{
							*mime = _wcsdup(L"jpg");
							*imageData = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(*len);
							memcpy(*imageData, data, *len);
						}
						UnmapViewOfFile(data);
						CloseHandle(hmap);
						CloseHandle(hf);
						sf->releaseInterface(loader);
						return im;
					}
					UnmapViewOfFile(data);
				}
				CloseHandle(hmap);
			}
			CloseHandle(hf);
		}
		sf->releaseInterface(loader);
	}
	return 0;
}

static void * writeImg(const ARGB32 *data, int w, int h, int *length, const wchar_t *ext)
{
	if (!ext && !*ext) return NULL;
	if (*ext == L'.') ext++;
	FOURCC imgwrite = svc_imageWriter::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgwrite);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgwrite,i);
		if (sf)
		{
			svc_imageWriter * l = (svc_imageWriter*)sf->getInterface();
			if (l)
			{
				if (wcsstr(l->getExtensions(),ext))
				{
					void* ret = l->convert(data,32,w,h,length);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static int writeFile(const wchar_t *file, const void * data, int length)
{
	FILE *f=_wfopen(file,L"wb");
	if (!f) return ALBUMART_FAILURE;
	if (fwrite(data,length,1,f) != 1)
	{
		fclose(f);
		return ALBUMART_FAILURE;
	}
	fclose(f);
	return ALBUMART_SUCCESS;
}

static void writeImageToFile(ARGB32 * img, int w, int h, const wchar_t *file)
{
	int length=0;
	void * data = writeImg(img,w,h,&length,wcsrchr(file,L'.'));
	if (data)
	{
		writeFile(file,data,length);
		WASABI_API_MEMMGR->sysFree(data);
	}
}

static INT_PTR CALLBACK FileInfo_Artwork(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		EnableArtFrame(hwndDlg,FALSE);

		// added 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
		// as doing this means we keep the placeholder incase of a change at a
		// later time and without doing anything to require an translation updates
		HWND wnd = GetDlgItem(hwndDlg, IDC_BUTTON_DOWNLOAD);
		if (IsWindow(wnd)) DestroyWindow(wnd);

		info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
		if (AGAVE_API_ALBUMART)
		{
		int w = 0, h = 0;
		ARGB32 *bits = NULL;
		if (AGAVE_API_ALBUMART->GetAlbumArt(p->filename, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS)
		{
			WASABI_API_MEMMGR->sysFree(bits);
			int i = SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)getStringW(IDS_COVER,NULL,0));
			SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,i,0);
			PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
		}

		wchar_t *types = NULL;
#if 0 // benski> TODO:
		if (AGAVE_API_ALBUMART->GetAlbumArtTypes(p->filename,&types) == ALBUMART_SUCCESS)
		{
			wchar_t *p = types;
			int sel = 0;
			while (*p)
			{
				int is_cover = (!_wcsicmp(p,L"cover") || !_wcsicmp(p,getStringW(IDS_COVER,NULL,0)));
				int i = SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)(is_cover?getStringW(IDS_COVER,NULL,0):p));
				if (!_wcsicmp(p,L"cover"))
					sel = i;
				p += wcslen(p) + 1;
			}
			WASABI_API_MEMMGR->sysFree(types);
		
			SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,sel,0);
			PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
		}
#endif
		

		if (AGAVE_API_ALBUMART->GetValidAlbumArtTypes(p->filename,&types) == ALBUMART_SUCCESS)
		{
			wchar_t *p = types;
			int sel = 0;
			while (*p)
			{
				int is_cover = (!_wcsicmp(p,L"cover") || !_wcsicmp(p,getStringW(IDS_COVER,NULL,0)));
				int i = SendDlgItemMessageW(hwndDlg,IDC_COMBO_ARTTYPE,CB_ADDSTRING,0,(LPARAM)(is_cover?getStringW(IDS_COVER,NULL,0):p));
				if (is_cover)
					sel = i;
				p += wcslen(p) + 1;
			}
			WASABI_API_MEMMGR->sysFree(types);
			SendDlgItemMessage(hwndDlg,IDC_COMBO_ARTTYPE,CB_SETCURSEL,sel,0);
		}
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ARTLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
				wchar_t type[100]=L"";
				int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);

				if (sel == -1)
				{
					HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)0);
					if (bmold) DeleteObject(bmold);
					EnableArtFrame(hwndDlg,FALSE);
					return 0;
				}

				EnableArtFrame(hwndDlg,TRUE);
				SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,sel,(LPARAM)type);

				int w = 0, h = 0;
				ARGB32 *bits = NULL;
				if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(p->filename,(!_wcsicmp(type,getStringW(IDS_COVER,NULL,0))?L"cover":type),&w,&h,&bits) == ALBUMART_SUCCESS)
				{
					HBITMAP bm = getBitmap(bits,w,h,228,228,hwndDlg);
					HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bm);
					if (bmold) DeleteObject(bmold);

					wchar_t caption[128], *mimeType = 0, *uiType = 0;
					int origin[] = {IDS_ORIGIN_NONE, IDS_ORIGIN_EMBEDDED, IDS_ORIGIN_ALBUM_MATCH, IDS_ORIGIN_NFO,
									IDS_ORIGIN_COVER_MATCH, IDS_ORIGIN_FOLDER_MATCH, IDS_ORIGIN_FRONT_MATCH, IDS_ORIGIN_ARTWORK_MATCH};
					int ret = AGAVE_API_ALBUMART->GetAlbumArtOrigin(p->filename,(!_wcsicmp(type,getStringW(IDS_COVER,NULL,0))?L"cover":type), &mimeType);
					if (mimeType)
					{
						uiType = wcschr(mimeType, L'/');
						if (uiType && *uiType)
						{
							uiType++;
						}
					}
					wchar_t buf[100] = {0};
					StringCchPrintfW(caption,128,getStringW(IDS_ARTWORK_DETAILS, NULL, 0), type, w, h,
									 getStringW(origin[ret], buf, sizeof(buf)), (uiType && *uiType ? uiType : mimeType));

					SetDlgItemTextW(hwndDlg,IDC_STATIC_FRAME, caption);
					WASABI_API_MEMMGR->sysFree(mimeType);

					EditArtItem *e = new EditArtItem(bits,w,h);

					size_t len = 0;
					ARGB32 *data = NULL;
					if (AGAVE_API_ALBUMART->GetAlbumArtData(p->filename, L"cover", (void**)&data, &len, &mimeType) == ALBUMART_SUCCESS)
					{
						e->data = data;
						e->datalen = len;
					}
					WASABI_API_MEMMGR->sysFree(mimeType);

					EditArtItem *eold = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,sel,0);
					if (eold && eold != (EditArtItem *)-1) delete eold;
					SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETITEMDATA,sel,(LPARAM)e);
				}
			}
			break;
		// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
		#if 0
		case IDC_BUTTON_DOWNLOAD:
		{
			wchar_t artist[256]=L"";
			wchar_t album[256]=L"";
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			GetDlgItemTextW(p->tabs[0],IDC_ALBUM_ARTIST,artist,256);
			if (!artist[0]) GetDlgItemTextW(p->tabs[0],IDC_ARTIST,artist,256);
			GetDlgItemTextW(p->tabs[0],IDC_ALBUM,album,256);

			artFetchData d = {sizeof(d),hwndDlg,artist,album,0};
			int r = (int)SendMessage(hMainWindow,WM_WA_IPC,(LPARAM)&d,IPC_FETCH_ALBUMART);
			if (r == -2) break; // cancel all was pressed
			if (r == 0 && d.imgData && d.imgDataLen) // success
			{
				if (AddNewArtItem(hwndDlg))
				{
					bool success=false;
					EditArtItem *e = getCurrentArtItem(hwndDlg);
					if (e)
					{
						int w=0,h=0;
						ARGB32* data = decompressImage(d.imgData,d.imgDataLen,&w,&h);
						if (data)
						{
							if (e->bits) WASABI_API_MEMMGR->sysFree(e->bits);
							e->bits = data;
							e->w = w;
							e->h = h;
							if (e->data) WASABI_API_MEMMGR->sysFree(e->data);
							e->data = d.imgData;
							d.imgData = 0;
							e->datalen = d.imgDataLen;
							free(e->mimetype);
							e->mimetype = _wcsdup(L"jpg");
							e->dirty = true;
							HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
							HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
							if (bmold) DeleteObject(bmold);
							success=true;
						}
					}
					if (!success)
					{
						// fail, remove :(
						int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
						if (sel == -1)
							return 0;
						if (e) delete e;
						SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
						EnableArtFrame(hwndDlg,0);
					}
				}
				if (d.imgData)
					WASABI_API_MEMMGR->sysFree(d.imgData);
			}
		}
		break;
		#endif
		case IDC_BUTTON_LOAD:
			if (AddNewArtItem(hwndDlg))
			{
				if (!FileInfo_Artwork(hwndDlg,WM_COMMAND,IDC_BUTTON_CHANGE,0))
				{
					// fail, remove :(
					int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
					if (sel == -1)
						return 0;
					EditArtItem * e = getCurrentArtItem(hwndDlg);
					if (e) delete e;
					SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
					EnableArtFrame(hwndDlg,0);
				}
			}
			break;
		case IDC_BUTTON_CHANGE:
		{
			EditArtItem *e = getCurrentArtItem(hwndDlg);
			if (!e) break;
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			wchar_t file[1024]=L"", folder[MAX_PATH]=L"";
			OPENFILENAMEW fn = {sizeof(OPENFILENAMEW),0};
			// set the ofd to the current file's folder
			lstrcpynW(folder,p->filename,MAX_PATH);
			PathRemoveFileSpecW(folder);
			PathAddBackslashW(folder);
			fn.lpstrInitialDir = folder;
			fn.hwndOwner = hwndDlg;
			fn.lpstrFile = file;
			fn.nMaxFile = 1024;

			static wchar_t fileExtensionsString[MAX_PATH] = {0};
			if(!fileExtensionsString[0])
			{
				getStringW(IDS_IMAGE_FILES,fileExtensionsString,MAX_PATH);
				wchar_t *temp=fileExtensionsString+lstrlenW(fileExtensionsString) + 1;

				// query the available image loaders and build it against the supported formats
				FOURCC imgload = svc_imageLoader::getServiceType();
				int n = WASABI_API_SVC->service_getNumServices(imgload);
				for (int i=0; i<n; i++)
				{
					waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
					if (sf)
					{
						svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
						if (l)
						{
							wchar_t *tests[] = {L"*.jpg",L"*.png",L"*.gif",L"*.bmp"};
							for(int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
							{
								if (l->isMine(tests[i]))
								{
									StringCchCatW(temp,MAX_PATH,tests[i]);
									StringCchCatW(temp,MAX_PATH,L";");
								}
							}
							sf->releaseInterface(l);
						}
					}
				}
				*(temp = temp + lstrlenW(temp) + 1) = 0;
			}
			fn.lpstrFilter = fileExtensionsString;

			fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOVALIDATE;
			if (GetOpenFileNameW(&fn))
			{
				int len = 0, w = 0, h = 0;
				wchar_t * mime = 0;
				// TODO: benski> save original bits and mime type
				// UPDATE: will only grab the raw data for jpeg files at the moment
				ARGB32 * data = 0, * bits = loadImgFromFile(file,&len,&w,&h,&mime,&data);
				if (bits)
				{
					if (e->bits) WASABI_API_MEMMGR->sysFree(e->bits);
					if (e->data) WASABI_API_MEMMGR->sysFree(e->data);
					e->bits = bits;
					e->w = w;
					e->h = h;
					if (data)
					{
						e->data = data;
						e->datalen = len;
					}
					e->mimetype = mime;
					e->dirty = true;
					HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
					HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
					if (bmold) DeleteObject(bmold);
					return 1;
				}
			}
			break;
		}
		case IDC_BUTTON_SAVEAS:
		{
			EditArtItem *e = getCurrentArtItem(hwndDlg);
			if (!e) break;
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			wchar_t file[1024]=L"", folder[MAX_PATH]=L"";
			OPENFILENAMEW fn = {sizeof(OPENFILENAMEW),0};
			// set the ofd to the current file's folder
			lstrcpynW(folder,p->filename,MAX_PATH);
			PathRemoveFileSpecW(folder);
			PathAddBackslashW(folder);
			fn.lpstrInitialDir = folder;
			fn.hwndOwner = hwndDlg;
			fn.lpstrFile = file;
			fn.nMaxFile = 1020;

			wchar_t *tests[] = {L"*.jpg",L"*.png",L"*.gif",L"*.bmp"};
			static int tests_idx[4] = {0,1,2,3}, tests_run = 0, last_filter = 0;
			int j = 0, tests_str[] = {IDS_JPEG_FILE,IDS_PNG_FILE,IDS_GIF_FILE,IDS_BMP_FILE};
			size_t size = 1024;
			static wchar_t filter[1024] = {0}, *sff = filter;

			if(!tests_run)
			{
				tests_run = 1;
				FOURCC imgload = svc_imageLoader::getServiceType();
				int n = WASABI_API_SVC->service_getNumServices(imgload);
				for (int i=0; i<n; i++)
				{
					waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
					if (sf)
					{
						svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
						if (l)
						{
							for(int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
							{
								if (l->isMine(tests[i]))
								{
									tests_idx[j] = i;
									j++;
									int len = 0;
									getStringW(tests_str[i],sff,size);
									size-=(len = lstrlenW(sff)+1);
									sff+=len;
									lstrcpynW(sff,tests[i],size);
									size-=(len = lstrlenW(sff)+1);
									sff+=len;
								}
							}
							sf->releaseInterface(l);
						}
					}
				}
				last_filter = tests_idx[0];
			}

			fn.lpstrFilter = filter;
			// TODO: save between sessions??
			fn.nFilterIndex = last_filter;	// default to *.jpg / last filter
			fn.Flags = OFN_OVERWRITEPROMPT;
			if (GetSaveFileNameW(&fn))
			{
				last_filter = fn.nFilterIndex;
				int l = wcslen(file);
				if (l>4 && file[l-4]==L'.'); // we have an extention
				else StringCchCatW(file,1024,tests[tests_idx[fn.nFilterIndex-1]]+1); // map to the extension to use

				// TODO: benski> if mime types are the same, we can use e->data and e->datalen
				// UPDATE: enable writeFile(..) when means to check mimetype is implemented...
				//writeFile(file,e->data,e->datalen);
				writeImageToFile(e->bits,e->w,e->h,file);
			}
			break;
		}
		case IDC_BUTTON_COPY:
		{
			EditArtItem *e = getCurrentArtItem(hwndDlg);
			if (!e) break;
			if (!OpenClipboard(hwndDlg)) break;
			EmptyClipboard();
			HBITMAP bm = getBitmap(e->bits,e->w,e->h,hwndDlg);
			SetClipboardData(CF_BITMAP,bm);
			CloseClipboard();
			DeleteObject(bm);
			break;
		}
		case IDC_BUTTON_PASTENEW:
			if (AddNewArtItem(hwndDlg))
			{
				if (!FileInfo_Artwork(hwndDlg,WM_COMMAND,IDC_BUTTON_PASTE,0))
				{
					// fail, remove :(
					int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
					if (sel == -1)
						return 0;
					EditArtItem * e = getCurrentArtItem(hwndDlg);
					if (e) delete e;
					SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
					EnableArtFrame(hwndDlg,0);
				}
			}
			break;
		case IDC_BUTTON_PASTE:
		{
			EditArtItem *e = getCurrentArtItem(hwndDlg);
			if (!e) break;
			if (!OpenClipboard(hwndDlg)) break;
			HBITMAP bm = (HBITMAP)GetClipboardData(CF_BITMAP);
			if (bm)
			{
				GetSize(bm,e->w,e->h,hwndDlg);
				BITMAPINFO info={0};
				info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				info.bmiHeader.biWidth = e->w;
				info.bmiHeader.biHeight = -e->h;
				info.bmiHeader.biPlanes = 1;
				info.bmiHeader.biBitCount = 32;
				info.bmiHeader.biCompression = BI_RGB;
				HDC dc = GetDC(hwndDlg);
				if (e->bits) WASABI_API_MEMMGR->sysFree(e->bits);
				e->bits = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(e->w*e->h*sizeof(ARGB32));
				GetDIBits(dc,bm,0,e->h,e->bits,&info,DIB_RGB_COLORS);
				ReleaseDC(hwndDlg,dc);
				e->dirty=true;
				HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
				HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
				if (bmold) DeleteObject(bmold);
				CloseClipboard();
				return 1;
			}
			CloseClipboard();
			break;
		}
		case IDC_BUTTON_DELETE:
		{
			wchar_t buf[1024];
			getStringW(IDS_ARTDELETE,buf,1024);
			if (MessageBoxW(hwndDlg,buf,getStringW(IDS_AREYOUSURE,0,0),MB_YESNO) != IDYES) break;
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			EditArtItem *e = getCurrentArtItem(hwndDlg);
			if (!e) break;
			int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
			buf[0]=0;
			SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,sel,(LPARAM)buf);
			AGAVE_API_ALBUMART->DeleteAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf));
			HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)0);
			if (bmold) DeleteObject(bmold);
			delete e;
			SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
			EnableArtFrame(hwndDlg,0);
			break;
		}
		case IDOK:
		{
			info_params * p = (info_params *)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			int l = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0);
			for (int i=0; i<l; i++)
			{
				EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,i,0);
				if (e->dirty)
				{
					wchar_t buf[1024];
					SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,i,(LPARAM)buf);
					if (e->data)
					{
						AGAVE_API_ALBUMART->SetAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf),
							0,0,e->data,e->datalen,e->mimetype);
					}
					else
					{
						AGAVE_API_ALBUMART->SetAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf),
							e->w,e->h,e->bits,0,0);
					}
				}
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
	{
		HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_GETIMAGE,IMAGE_BITMAP,0);
		if (bmold) DeleteObject(bmold);
		int l = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0);
		for (int i=0; i<l; i++)
		{
			EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,i,0);
			if (e && e != (EditArtItem *)-1) delete e;
		}
	}
	break;
	}
	return 0;
}