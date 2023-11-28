#include "UsbDevice.h"
#include "../nu/AutoWide.h"
#include <shlwapi.h>
#include <strsafe.h>

// from main.cpp
extern PMPDevicePlugin plugin;
extern C_ItemList devices;

// from replaceVars.cpp
extern wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song);
extern BOOL RecursiveCreateDirectory(wchar_t* buf);

// from eject.cpp
extern BOOL EjectVolume(TCHAR cDriveLetter);

// from filecopy.cpp
extern int CopyFile(wchar_t * infile, wchar_t * outfile, void * callbackContext, void (*callback)(void * callbackContext, wchar_t * status), int * killswitch);

static int song_sortfunc(const void *elem1, const void *elem2);
static UsbSong *BinaryChopFind(const wchar_t *songfn,Playlist * mpl);

static DeviceType detectDeviceType(wchar_t drive);
static int getMetaItemInt(wchar_t * item, wchar_t * filename);

LRESULT CALLBACK UsbMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if(uMsg == WM_TIMER && wParam == 1) for(int i=0; i<devices.GetSize(); i++) if(((UsbDevice*)devices.Get(i))->messageWnd == hwnd) {
    KillTimer(hwnd,1);
    UsbDevice * dev = (UsbDevice*)devices.Get(i);
    dev->cacheUpToDate=false;
    int n = dev->metadataWait.GetSize() - 1;
    if(n >= 0) {
      UsbSong * s = (UsbSong*)dev->metadataWait.Get(n);
      dev->metadataWait.Del(n);
      dev->fillMetaData(s,s->filename);
      if(n % 100 == 0) dev->refreshCache();
      if(n) SetTimer(hwnd,1,100,NULL);
      else {
        DestroyWindow(hwnd);
        dev->messageWnd=NULL;
      }
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static bool classRegistered = 0;
HWND CreateDummyWindow()
{
	if (!classRegistered)
	{
		WNDCLASSW wc = {0, };

		wc.style = 0;
		wc.lpfnWndProc = UsbMsgProc;
		wc.hInstance = plugin.hDllInstance;
		wc.hIcon = 0;
		wc.hCursor = NULL;
		wc.lpszClassName = L"pmp_usb_window";

		if (!RegisterClassW(&wc))
			return 0;

		classRegistered = true;
	}
	HWND dummy = CreateWindowW(L"pmp_usb_window", L"pmp_usb_window", 0, 0, 0, 0, 0, NULL, NULL, plugin.hDllInstance, NULL);

	return dummy;
}

void UsbDevice::setupTranscoder() {
  if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
  transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
  if(!transcoder) return;
  
  wchar_t * p = supportedFormats;
	while(*p) {
		wchar_t * np = wcschr(p,L';');
		if(np) *np = 0;
		transcoder->AddAcceptableFormat(p);
		if(np) { *np = L';'; p=np+1; } 
		else return;
	}
}

void deleteOldTagCache(wchar_t drive) {
  wchar_t a[] = {drive,L":\\tag_cache.xml"};
  SetFileAttributesW(a,FILE_ATTRIBUTE_NORMAL);
  _wunlink(a);
}

UsbDevice::UsbDevice(wchar_t drive, pmpDeviceLoading * load) : transcoder(NULL) {
	loadedUpToDate = true;
	messageWnd=NULL;
	StringCchPrintf(xmlFile, 100, L"%c:\\" TAG_CACHE, drive);
	// Start-NDE
	StringCchPrintf(ndeDataFile, 100, L"%c:\\winamp_metadata.dat", drive);
	StringCchPrintf(ndeIndexFile, 100, L"%c:\\winamp_metadata.idx", drive);
	// End-NDE
	load->dev = this;
	load->UpdateCaption = NULL;
	//pass load to ml_pmp, ml updates load->UpdateCaption and context
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)load,PMP_IPC_DEVICELOADING);
	if(load->UpdateCaption) {
		wchar_t buf[100] = L"";
		WASABI_API_LNGSTRINGW_BUF(IDS_LOADING_DRIVE_X,buf,100);
		wchar_t * x = wcsrchr(buf,L'X');
		if(x) *x = drive;
		load->UpdateCaption(buf,load->context);
	}
	deleteOldTagCache(drive);
	devType = detectDeviceType(drive);
	// load settings
	StringCchCopy(ini_file,MAX_PATH,L"x:\\pmp_usb.ini");
	ini_file[0]=drive;

	wchar_t customName[fieldlen];
	GetPrivateProfileString(L"pmp_usb",L"pldir",devType==TYPE_PSP?L"X:\\PSP\\MUSIC":L"X:",pldir,sizeof(pldir)/sizeof(wchar_t),ini_file);
	GetPrivateProfileString(L"pmp_usb",L"songFormat",devType==TYPE_PSP?L"X:\\PSP\\MUSIC\\<Artist> - <Album>\\## - <Title>":L"X:\\<Artist>\\<Album>\\## - <Title>",songFormat,sizeof(songFormat)/sizeof(wchar_t),ini_file);
	GetPrivateProfileString(L"pmp_usb",L"supportedFormats",L"mp3;wav;wma",supportedFormats,sizeof(supportedFormats)/sizeof(wchar_t),ini_file);
	GetPrivateProfileString(L"pmp_usb",L"purgeFolders",L"1",purgeFolders,sizeof(purgeFolders)/sizeof(wchar_t),ini_file);
	GetPrivateProfileString(L"pmp_usb",L"customName",devType==TYPE_PSP?L"Sony PSP":L"",customName,sizeof(customName)/sizeof(wchar_t),ini_file);
	pl_write_mode = GetPrivateProfileInt(L"pmp_usb",L"pl_write_mode",0,ini_file);
	pldir[0] = drive;
	songFormat[0] = drive;

	transferQueueLength = 0;
	this->drive = drive;
	Playlist * mpl = new Playlist;
	playlists.Add(mpl);
	lstrcpyn(mpl->filename,customName,MAX_PATH);
	wchar_t * pl = _wcsdup(pldir);
	pl[0] = drive;
	RecursiveCreateDirectory(pl);
	wchar_t root[3] = L"X:";
	root[0] = drive;
	fileProbe(root);

	// sort the master playlist by filename, so we can find stuff later using binary chop
	qsort(mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(void*),song_sortfunc);

	// sort out and read playlists....
	for(int i=1; i < playlists.GetSize(); i++) {
		wchar_t thisline0[MAX_PATH+3]={drive,L":"};
		wchar_t *thisline = &thisline0[2];
		Playlist * pls = (Playlist *)playlists.Get(i);
		FILE * f = _wfopen(pls->filename,L"rt");
		if(!f) continue;
		while(fgetws(thisline,(MAX_PATH-1),f)) {
			if(thisline[0] != 0 && thisline[0] != L'#') {//set up playlists
				wchar_t * e = wcsrchr(thisline,L'\n');
				if(e) *e = 0; //end string at new line -- thisline = file name
				UsbSong *t;
				if(thisline[0] == L'\\') {
					t = BinaryChopFind(thisline0,mpl);
				}
				else if(thisline[0] == L'.' && thisline[1] == L'\\') {
					// if a .\ then remove the . as applicable
					lstrcpynW(&thisline0[2],&thisline0[3],MAX_PATH+3);
					t = BinaryChopFind(thisline0,mpl);
				}
				else {
					// if now \\ then attempt to add a \ on the front
					if(thisline[0] != L'\\') {
						wchar_t thislinetmp[MAX_PATH+3] = {drive,L":\\"};
						StringCchCat(thislinetmp,MAX_PATH+3,&thisline0[2]);
						t = BinaryChopFind(thislinetmp,mpl);
					}
					else {
						t = BinaryChopFind(thisline,mpl);
						pls->dirty=true;
					}
				}
				if(t) pls->songs.Add(t);
			}
		}
		fclose(f);
	}
	tag();
	if(metadataWait.GetSize()) {
		messageWnd=CreateDummyWindow();
		SetTimer(messageWnd,1,1000,NULL);
	}

	devices.Add(this);
	extern HWND config;
	if(config) PostMessage(config,WM_USER,0,0);

	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);
	setupTranscoder();
}

UsbDevice::~UsbDevice() {
	for(int i=0; i < playlists.GetSize(); i++) {
		Playlist * pl = (Playlist *)playlists.Get(i);
		if(i == 0) { // mpl
			for(int j=0; j < pl->songs.GetSize(); j++)
				delete (UsbSong*)pl->songs.Get(j);
		}
		delete pl;
	}
  if(messageWnd) { DestroyWindow(messageWnd); messageWnd = NULL; }
  if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
}

DeviceType detectDeviceType(wchar_t drive) {
  // Check for device being a PSP
  {
    wchar_t memstickind[] = {drive, L":\\MEMSTICK.IND"};
    FILE * f = NULL;
    f = _wfopen(memstickind,L"rb");
    if(f) {
      fclose(f);
      WIN32_FIND_DATA FindFileData;
	  HANDLE hFind = INVALID_HANDLE_VALUE;
      wchar_t pspdir[] = {drive,L":\\PSP"};
	  hFind = FindFirstFile(pspdir, &FindFileData);
      if(hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          return TYPE_PSP;
      }
    }
  }
  // Cannot recognise device type
  return TYPE_OTHER;
}

static int song_sortfunc(const void *elem1, const void *elem2) {
  UsbSong *a=(UsbSong *)*(void **)elem1;
  UsbSong *b=(UsbSong *)*(void **)elem2;
  return _wcsicmp(a->filename,b->filename);
}

static UsbSong *BinaryChopFind(const wchar_t *songfn,Playlist * mpl) {
  UsbSong s;
  lstrcpyn(s.filename,songfn,MAX_PATH);
  UsbSong * d = &s;
  UsbSong ** ret = (UsbSong **)bsearch(&d,mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(void*),song_sortfunc);
  return ret?*ret:NULL;
}

bool supportedFormat(wchar_t * file, wchar_t * supportedFormats) {
	wchar_t * ext = wcsrchr(file,'.');
	if(!ext) return false;
	ext++;
	wchar_t * p = supportedFormats;
	while(*p) {
	  bool ret=false;
	  wchar_t * np = wcschr(p,L';');
	  if(np) *np = 0;
	  if(!_wcsicmp(ext,p)) ret=true;
	  if(np) { *np = L';'; p=np+1; } 
	  else return ret;
	  if(ret) return true; 
	}
	return false;
}
//read files from device's folder 'indir'
void UsbDevice::fileProbe(wchar_t * indir) {
	wchar_t dir[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	StringCchPrintf(dir,MAX_PATH,L"%s\\*",indir);
	hFind = FindFirstFile(dir, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;
	do {
		if(wcscmp(FindFileData.cFileName,L".") && wcscmp(FindFileData.cFileName,L"..")) {
			wchar_t fullfile[MAX_PATH];
			StringCchPrintf(fullfile,MAX_PATH,L"%s\\%s",indir,FindFileData.cFileName);
			//PathCombineW(fullfile,indir,FindFileData.cAlternateFileName);
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { //file is directory
				fileProbe(fullfile);	//call until we have found a file
			} else { // found a file!
				wchar_t * ext = wcsrchr(FindFileData.cFileName,'.'); 
				if(!ext) continue; //no files with extensions in the directory
				ext++;
				if(!_wcsicmp(ext,L"m3u")) { // its a playlist
					Playlist * pl = new Playlist;
					lstrcpyn(pl->filename,fullfile,MAX_PATH-1);
					playlists.Add(pl);
					continue;
				}	//its a file
				if(supportedFormat(fullfile,supportedFormats)) //check extension
				{
					UsbSong *s = new UsbSong(); //(UsbSong*)malloc(sizeof(UsbSong));
					//fillMetaData(s, fullfile);
					//s->Init();					
					lstrcpynW(s->filename, fullfile, MAX_PATH);
					((Playlist*)playlists.Get(0))->songs.Add(s);//add track to alltrack list (playlist 0)
				}
			}
		}
	} while(FindNextFile(hFind, &FindFileData) != 0);
	FindClose(hFind);
}

__int64 UsbDevice::getDeviceCapacityAvailable() {
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return freeb.QuadPart;
}

__int64 UsbDevice::getDeviceCapacityTotal() {
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return total.QuadPart;
}

static __int64 fileSize(wchar_t * filename)
{
	WIN32_FIND_DATA f={0};
	HANDLE h = FindFirstFileW(filename,&f);
	if(h == INVALID_HANDLE_VALUE) return -1;
	FindClose(h);
	ULARGE_INTEGER i;
	i.HighPart = f.nFileSizeHigh;
	i.LowPart = f.nFileSizeLow;
	return i.QuadPart;
}

void UsbDevice::Close() {
	for(int i=0; i < devices.GetSize(); i++) {
		if(((UsbDevice*)devices.Get(i)) == this) {
			devices.Del(i); 
			break;
		}
	}
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	//plugin.deviceDisconnected(this);
	delete this;
}
void UsbDevice::Eject() {
	for(int i=0; i < devices.GetSize(); i++) {
		if(((UsbDevice*)devices.Get(i)) == this) {
			devices.Del(i); 
			break;
		}
	}
	if(EjectVolume(drive)) {
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
		//plugin.deviceDisconnected(this);
		delete this;
	} else {
		wchar_t titleStr[32];
		MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_FAILED_TO_EJECT_DRIVE),
				   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32),0);
		devices.Add(this);
	}
}

int UsbDevice::getPlaylistCount() { return playlists.GetSize(); }

void UsbDevice::getPlaylistName(int playlistnumber, wchar_t * buf, int len) {
	wchar_t * fn = ((Playlist *)playlists.Get(playlistnumber))->filename;
	if(playlistnumber != 0) {
		if(fn[0]) {
			wchar_t * fn2 = wcsrchr(fn,L'\\');
			if(fn2) fn = fn2 + 1;
			lstrcpyn(buf,fn,len);
			fn2 = wcsrchr(buf,L'.');
			if(fn2) *fn2 = 0;
		}
	}
	else //playlist number = 0 -> this is the device
	{
		if(fn[0]) { //if we have a custom device name
			lstrcpyn(buf,fn,len);
		}
		else {
			WASABI_API_LNGSTRINGW_BUF(IDS_USB_DRIVE_X,buf,len);
			wchar_t * x = wcsrchr(buf,L'X');
			if(x) *x = drive;
		}
	}
}
int UsbDevice::getPlaylistLength(int playlistnumber) {
	return ((Playlist *)playlists.Get(playlistnumber))->songs.GetSize();
}
songid_t UsbDevice::getPlaylistTrack(int playlistnumber,int songnum) {
	return (songid_t)((Playlist *)playlists.Get(playlistnumber))->songs.Get(songnum);
}

static void removebadchars(wchar_t *s) {
	while (*s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s=L'_';
		s++;
	}
}

void UsbDevice::setPlaylistName(int playlistnumber, const wchar_t * buf) {
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	if(playlistnumber==0)
	{
		WritePrivateProfileString(L"pmp_usb",L"customName",buf,ini_file);
    lstrcpyn(pl->filename,buf,sizeof(pl->filename)/sizeof(wchar_t));
	}
	else {
		_wunlink(pl->filename);
		wchar_t * p = const_cast<wchar_t *>(buf);
		if(wcslen(buf) >= MAX_PATH-1) p[MAX_PATH-1]=0;
		while(*p == L'.') p++;
		removebadchars(p);
		StringCchPrintf(pl->filename,MAX_PATH,L"%s\\%s.m3u",pldir,p);
		pl->filename[0]=drive;
		pl->dirty=true;
	}
}

void UsbDevice::playlistSwapItems(int playlistnumber, int posA, int posB) {
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	void * a = pl->songs.Get(posA);
	void * b = pl->songs.Get(posB);
	pl->songs.Set(posA,b);
	pl->songs.Set(posB,a);
	pl->dirty = true;
}

void UsbDevice::addTrackToPlaylist(int playlistnumber, songid_t songid) { 
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	pl->songs.Add((void*)songid);
	pl->dirty = true;
}

void UsbDevice::removeTrackFromPlaylist(int playlistnumber, int songnum) {
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	pl->songs.Del(songnum);
	pl->dirty = true;
}

void UsbDevice::deletePlaylist(int playlistnumber) {
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	_wunlink(pl->filename);
	playlists.Del(playlistnumber);
	delete pl;
}
int UsbDevice::newPlaylist(const wchar_t * name) {
	Playlist * pl = new Playlist;
	wchar_t * p = const_cast<wchar_t *>(name);
	if(wcslen(name) >= (MAX_PATH-1)) p[MAX_PATH-1]=0;	
	while(*p == L'.') p++;
	removebadchars(p);
	StringCchPrintf(pl->filename, MAX_PATH, L"%s\\%s.m3u",pldir,p);
	pl->filename[0]=drive;
	playlists.Add(pl);
	return playlists.GetSize()-1;
}

void UsbDevice::deleteTrack(songid_t songid) {
	UsbSong * s = (UsbSong*)songid;
	//errno == 2 is ENOENT
  if(!_wunlink(s->filename) || errno == 2){ //will continue delete if file was deleted successfully or file path does not exist in the first place (errno==2)
		for(int i=0; i < playlists.GetSize(); i++) {
			Playlist * pl = (Playlist *)playlists.Get(i);
			int l = pl->songs.GetSize();
			for(int j = l-1; j >= 0; j--) {
				if(((UsbSong*)pl->songs.Get(j)) == s) {
					pl->songs.Del(j);
					pl->dirty = true;
				}
			}
      l = metadataWait.GetSize();
      for(int j = l-1; j >= 0; j--)
				if(((UsbSong*)metadataWait.Get(j)) == s)
					metadataWait.Del(j);
		}
    if(purgeFolders[0]=='1') deleteEmptyDir(s->filename);
    delete (UsbSong*)songid;
	}
	else
	{
		wchar_t titleStr[32];
		MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_TRACK_IN_USE),
				   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32),0);
	}
}
void UsbDevice::deleteEmptyDir(wchar_t *indir0) { //the directory passed into this will be deleted one folder down
	wchar_t indir[MAX_PATH];
	lstrcpyn(indir,indir0,MAX_PATH);
	do { wchar_t * s = wcsrchr(indir,L'\\'); if(s) *s=0; else return; } while(RemoveDirectory(indir));
	return;
}
__int64 UsbDevice::getTrackSizeOnDevice(const itemRecordW * track) {
  if(transcoder) {
    if(transcoder->ShouldTranscode(track->filename)) {
      int k = transcoder->CanTranscode(track->filename);
      if(k != -1 && k != 0) return k;
      return 0;
    } else return fileSize(track->filename);
  } else {
    if(!supportedFormat(track->filename,supportedFormats)) return 0;
	  return fileSize(track->filename);
  }
}

int UsbDevice::trackAddedToTransferQueue(const itemRecordW * track) {
	__int64 k = getTrackSizeOnDevice(track);
	if(!k) return -2;
	__int64 l = (__int64)k;
	__int64 avail = getDeviceCapacityAvailable();
	__int64 cmp = transferQueueLength;
	cmp += l;
	if(cmp > avail) return -1;
	else {
		transferQueueLength += l;
		return 0;
	}
}

void UsbDevice::trackRemovedFromTransferQueue(const itemRecordW * track) {
	transferQueueLength -= (__int64)getTrackSizeOnDevice(track);
}

int UsbDevice::transferTrackToDevice(const itemRecordW * track,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),songid_t * songid,int * killswitch) {
	wchar_t fn[MAX_PATH] = L"X:\\";
	lstrcpyn(fn,songFormat,MAX_PATH);
  fn[0] = drive;
	wchar_t * src = _wcsdup(track->filename);
  wchar_t ext[10]=L"";
  {wchar_t *e = wcsrchr(src,L'.'); if(e) lstrcpyn(ext,e,10);}
  bool transcodefile = false;
  if(transcoder && transcoder->ShouldTranscode(src)) {
    int r = transcoder->CanTranscode(src,ext);
    if(r != 0 && r != -1) transcodefile = true;
  }
	UsbSong *s = new UsbSong();
  lstrcpyn(s->filename,src,MAX_PATH);  //this will get written over, but for now we have this so that the user can keep the old filename
	fillMetaData(s, src);
	FixReplacementVars(fn, MAX_PATH,this,(songid_t)s);
	StringCchCat(fn,MAX_PATH,ext); //place extension
	StringCchPrintf(s->filename, MAX_PATH, L"%s", fn);
	wchar_t * dir = wcsrchr(fn,L'\\');
  wchar_t * dir2 = wcsrchr(fn,L'/');
  wchar_t slash;
  if(dir2 > dir) { dir = dir2; slash=L'/';} else slash = L'\\';
	*dir = 0;
	RecursiveCreateDirectory(fn);
	*dir = slash;
	int r;
  if(transcodefile)
    r = transcoder->TranscodeFile(src,fn,killswitch,callback,callbackContext);
  else 
    r = CopyFile(src,fn,callbackContext,callback,killswitch);

	if(!r) {
    s->bitrate = getMetaItemInt(L"bitrate", fn);
    s->size = fileSize(fn);
    *songid = (songid_t)s;
	} else delete s;
	free(src);
	return r;
}

wchar_t * lastEdit=NULL;

static void globalCommitChanges() {
	if(lastEdit) {
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)0,IPC_WRITE_EXTENDED_FILE_INFO);
		free(lastEdit);
		lastEdit=NULL;
	}
}

void UsbDevice::commitChanges() {
	// write dirty playlists
	for(int i=1; i < playlists.GetSize(); i++) {
		Playlist * pl = (Playlist *)playlists.Get(i);
		if(pl->dirty) {
			FILE * f = _wfopen(pl->filename,L"wt");
			if(!f) continue;
			fwprintf(f,L"#EXTM3U\n");
			for(int j=0; j < pl->songs.GetSize(); j++) {
				// original mode as \path\to\file.mp3
				if(!pl_write_mode) {
					fwprintf(f,L"%s\n",((UsbSong*)pl->songs.Get(j))->filename+2);
				}
				// dot mode as .\path\to\file.mp3
				else if(pl_write_mode == 1) {
					fwprintf(f,L".%s\n",((UsbSong*)pl->songs.Get(j))->filename+2);
				}
				// slash removed mode as path\to\file.mp3
				else if(pl_write_mode == 2) {
					fwprintf(f,L"%s\n",((UsbSong*)pl->songs.Get(j))->filename+3);
				}
			}
			fclose(f);
			pl->dirty=false;
		}
	}

	//update cache
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
	outputXML();
	cacheUpToDate = true;
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);

	globalCommitChanges();
}

#if 0
// Start-NDE
void UsbDevice::commitChanges() {
	// write dirty playlists
	for(int i=1; i < playlists.GetSize(); i++) {
		Playlist * pl = (Playlist *)playlists.Get(i);
		if(pl->dirty) {
			FILE * f = _wfopen(pl->filename,L"wt");
			if(!f) continue;
			fwprintf(f,L"#EXTM3U\n");
			for(int j=0; j < pl->songs.GetSize(); j++) {
				// original mode as \path\to\file.mp3
				if(!pl_write_mode) {
					fwprintf(f,L"%s\n",((UsbSong*)pl->songs.Get(j))->filename+2);
				}
				// dot mode as .\path\to\file.mp3
				else if(pl_write_mode == 1) {
					fwprintf(f,L".%s\n",((UsbSong*)pl->songs.Get(j))->filename+2);
				}
				// slash removed mode as path\to\file.mp3
				else if(pl_write_mode == 2) {
					fwprintf(f,L"%s\n",((UsbSong*)pl->songs.Get(j))->filename+3);
				}
			}
			fclose(f);
			pl->dirty=false;
		}
	}

	//update cache
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
	outputNDE();
	cacheUpToDate = true;
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);

	globalCommitChanges();
}
// End-NDE
#endif

#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb) {
	if (!pa) pa=L"";
	else SKIP_THE_AND_WHITESPACE(pa);
  if (!pb) pb=L"";
	else SKIP_THE_AND_WHITESPACE(pb);
	return lstrcmpi(pa,pb);
}
#undef SKIP_THE_AND_WHITESPACE

int sortby;
Device * sortDev;

void UsbDevice::sortPlaylist(int playlistnumber, int sortBy) {
	sortby = sortBy;
	sortDev = this;
	Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
	qsort(pl->songs.GetAll(),pl->songs.GetSize(),sizeof(void*),sortFunc);
	pl->dirty = true;
}
static int sortFunc(const void *elem1, const void *elem2)
{
	int use_by = sortby;
	songid_t a=(songid_t)*(songid_t *)elem1;
	songid_t b=(songid_t)*(songid_t *)elem2;

#define RETIFNZ(v) if ((v)!=0) return v;

	// this might be too slow, but it'd be nice
	int x;
	for (x = 0; x < 5; x ++)
	{
		if (use_by == SORTBY_TITLE) // title -> artist -> album -> disc -> track
		{
			wchar_t bufa[2048];
			wchar_t bufb[2048];
			sortDev->getTrackTitle(a,bufa,2048);
			sortDev->getTrackTitle(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
				use_by=SORTBY_ARTIST;
		}
		else if (use_by == SORTBY_ARTIST) // artist -> album -> disc -> track -> title
		{
			wchar_t bufa[2048];
			wchar_t bufb[2048];
			sortDev->getTrackArtist(a,bufa,2048);
			sortDev->getTrackArtist(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
				use_by=SORTBY_ALBUM;
		}
		else if (use_by == SORTBY_ALBUM) // album -> disc -> track -> title -> artist
		{
			wchar_t bufa[2048];
			wchar_t bufb[2048];
			sortDev->getTrackAlbum(a,bufa,2048);
			sortDev->getTrackAlbum(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
				use_by=SORTBY_DISCNUM;
		}
		else if (use_by == SORTBY_DISCNUM) // disc -> track -> title -> artist -> album
		{
			int v1=sortDev->getTrackDiscNum(a);
			int v2=sortDev->getTrackDiscNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
				use_by=SORTBY_TRACKNUM;
		}
		else if (use_by == SORTBY_TRACKNUM) // track -> title -> artist -> album -> disc
		{
			int v1=sortDev->getTrackTrackNum(a);
			int v2=sortDev->getTrackTrackNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
				use_by=SORTBY_TITLE;     
		}
		else if (use_by == SORTBY_GENRE) // genre -> artist -> album -> disc -> track
		{
			wchar_t bufa[2048];
			wchar_t bufb[2048];
			sortDev->getTrackGenre(a,bufa,2048);
			sortDev->getTrackGenre(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
				use_by=SORTBY_ARTIST;
		}
		else break; // no sort order?
	} 

	return 0;
}

static void getMetaItem(wchar_t * item, wchar_t * filename, wchar_t * out, int len) {
	wchar_t buf[1024]=L"";
	extendedFileInfoStructW efs={filename,item,buf,1024};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	lstrcpyn(out,efs.ret,len);
}

static int getMetaItemInt(wchar_t * item, wchar_t * filename) {
	wchar_t buf[1024]=L"";
	extendedFileInfoStructW efs={filename,item,buf,1024};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	return _wtoi(buf);
}

static void setMetaItemInt(wchar_t * item, const wchar_t * filename, int value) {
	if(wcscmp(lastEdit?lastEdit:L"",filename))globalCommitChanges();
	if(!lastEdit) lastEdit = _wcsdup(filename);
	wchar_t buf[1024]=L"";
	StringCchPrintf(buf,1024,L"%d",value);
	extendedFileInfoStructW efs={filename,item,buf,1024};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_SET_EXTENDED_FILE_INFOW);
}

static void setMetaItem(wchar_t * item, const wchar_t * filename, const wchar_t * value) {
	if(wcscmp(lastEdit?lastEdit:L"",filename)) globalCommitChanges();
	if(!lastEdit) lastEdit = _wcsdup(filename);
	extendedFileInfoStructW efs={filename,item,const_cast<wchar_t *>(value),1024};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_SET_EXTENDED_FILE_INFOW);
}
// get metadata items
void UsbDevice::getTrackArtist(songid_t songid, wchar_t * buf, int len) {
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->artist, len);
	else
		getMetaItem(L"artist",((UsbSong*)songid)->filename,buf,len);
}

void UsbDevice::getTrackAlbum(songid_t songid, wchar_t * buf, int len) { 
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->album, len);
	else
		getMetaItem(L"album",((UsbSong*)songid)->filename,buf,len);
}

void UsbDevice::getTrackTitle(songid_t songid, wchar_t * buf, int len) {
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->title, len);
	else
		getMetaItem(L"title",((UsbSong*)songid)->filename,buf,len); 
}

int UsbDevice::getTrackTrackNum(songid_t songid) { 
	if(loadedUpToDate) 
		return ((UsbSong*)songid)->track;
	else
		return getMetaItemInt(L"track",((UsbSong*)songid)->filename); 
}

int UsbDevice::getTrackDiscNum(songid_t songid) { 
	if(loadedUpToDate) 
		return ((UsbSong*)songid)->discnum;
	else
		return getMetaItemInt(L"disc",((UsbSong*)songid)->filename); 
}

void UsbDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len) { 
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->genre, len);
	else
		getMetaItem(L"genre",((UsbSong*)songid)->filename,buf,len);
}

int UsbDevice::getTrackYear(songid_t songid) { 
	if(loadedUpToDate) 
		return ((UsbSong*)songid)->year;
	else
		return getMetaItemInt(L"year",((UsbSong*)songid)->filename);
}

__int64 UsbDevice::getTrackSize(songid_t songid) { 
	UsbSong* song = (UsbSong*)songid;
	if(loadedUpToDate) {
		return song->size;
	}
	else {
		return fileSize(song->filename);
	}
}

int UsbDevice::getTrackLength(songid_t songid) { 
	if(loadedUpToDate)
		return ((UsbSong*)songid)->length;
	else {
		basicFileInfoStructW b={0};
		b.filename=((UsbSong*)songid)->filename;
		b.quickCheck=0;
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&b,IPC_GET_BASIC_FILE_INFOW);
		return b.length * 1000;
	}
}

int UsbDevice::getTrackBitrate(songid_t songid) { 
	if(loadedUpToDate) 
		return ((UsbSong*)songid)->bitrate;
	else
		return getMetaItemInt(L"bitrate",((UsbSong*)songid)->filename);
}

// the following are not supported
int UsbDevice::getTrackPlayCount(songid_t songid) { return -1; }
int UsbDevice::getTrackRating(songid_t songid) { return -1; }
__time64_t UsbDevice::getTrackLastPlayed(songid_t songid) { return -1; }
__time64_t UsbDevice::getTrackLastUpdated(songid_t songid) { return -1; }

void UsbDevice::getTrackAlbumArtist(songid_t songid, wchar_t * buf, int len) {
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->albumartist, len);
	else
		getMetaItem(L"albumartist",((UsbSong*)songid)->filename,buf,len);
}

void UsbDevice::getTrackPublisher(songid_t songid, wchar_t * buf, int len) {
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->publisher, len);
	else
		getMetaItem(L"publisher",((UsbSong*)songid)->filename,buf,len);
}

void UsbDevice::getTrackComposer(songid_t songid, wchar_t * buf, int len) {
	if(loadedUpToDate) 
		wcsncpy(buf, ((UsbSong*)songid)->composer, len);
	else
		getMetaItem(L"composer",((UsbSong*)songid)->filename,buf,len);
}

// set metadata items
void UsbDevice::setTrackArtist(songid_t songid, const wchar_t * value) {
	wcsncpy(((UsbSong*)songid)->artist, value, fieldlen);
	setMetaItem(L"artist",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackAlbum(songid_t songid, const wchar_t * value) { 
	wcsncpy(((UsbSong*)songid)->album, value, fieldlen);
	setMetaItem(L"album",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackTitle(songid_t songid, const wchar_t * value) { 
	wcsncpy(((UsbSong*)songid)->title, value, fieldlen);
	setMetaItem(L"title",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackTrackNum(songid_t songid, int value) { 
	((UsbSong*)songid)->track = value;
	setMetaItemInt(L"track",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackDiscNum(songid_t songid, int value) { 
	((UsbSong*)songid)->discnum = value;
	setMetaItemInt(L"disc",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackGenre(songid_t songid, const wchar_t * value) { 
	wcsncpy(((UsbSong*)songid)->genre, value, fieldlen);
	setMetaItem(L"genre",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackYear(songid_t songid, int value) { 
	((UsbSong*)songid)->year = value;
	setMetaItemInt(L"year",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackAlbumArtist(songid_t songid, const wchar_t * value) {
	wcsncpy(((UsbSong*)songid)->albumartist, value, fieldlen);
	setMetaItem(L"albumartist",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackPublisher(songid_t songid, const wchar_t * value) {
	wcsncpy(((UsbSong*)songid)->publisher, value, fieldlen);
	setMetaItem(L"publisher",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

void UsbDevice::setTrackComposer(songid_t songid, const wchar_t * value) {
	wcsncpy(((UsbSong*)songid)->composer, value, fieldlen);
	setMetaItem(L"composer",((UsbSong*)songid)->filename,value);
	cacheUpToDate = false;
}

//bool UsbDevice::fillTrack(int index) {
//	Playlist *mpl = (Playlist*)playlists.Get(0);
//	UsbSong *cur = (UsbSong*)mpl->songs.Get(index);
//	cur->loadedInML = true;
//	//this function will be implemented when the new ML is released.  we dont want to load track data on initialization,
//	//we want to load the filename and get the track metadata from the cache or ID3 afterward, one track loaded each 1ms
//	//through a dummy window (see example in ml_pmp)
//	//this will make winamp still interactive while loading track metadata for the device
//	getTrackArtist((songid_t)cur,cur->artist,fieldlen);
//	getTrackTitle((songid_t)cur,cur->title,fieldlen);
//	return true;
//}
bool UsbDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue) {
	wchar_t buf[2048]=L"";
	wchar_t title[2048]=L"";

	if(!enqueue) { //clear playlist
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
		/*int l=SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_PE_GETINDEXTOTAL);
		while(l>=0) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,--l,IPC_PE_DELETEINDEX);*/
	}

	for(int i=0; i<listLength; i++) {
		UsbSong *curSong = (UsbSong*)songidList[i];    

		enqueueFileWithMetaStructW s={0};
		s.filename = _wcsdup(curSong->filename);
		TAG_FMT_EXT(buf, playTagFunc, fieldTagFuncFree, curSong, title, 2047, 0);
		s.title = _wcsdup(title);
		s.length = curSong->length/1000;

		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
	}

	if(!enqueue) { //play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	return true;
}
void TAG_FMT_EXT(const wchar_t *filename, void *f, void *ff, void *p, wchar_t *out, int out_len, int extended)
{
	waFormatTitleExtended fmt; 
	fmt.filename=filename;
	fmt.useExtendedInfo=extended;
	fmt.out = out;
	fmt.out_len = out_len;
	fmt.p = p;
	fmt.spec = 0;
	*(void **)&fmt.TAGFUNC = f;
	*(void **)&fmt.TAGFREEFUNC = ff;
	*out = 0;

	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}
wchar_t *playTagFunc(wchar_t *tag, void * p) //return 0 if not found
{
	UsbSong *t = (UsbSong*)p;
	wchar_t buf[128];
	wchar_t *value = NULL;

	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename")) value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"albumartist"))	value = t->albumartist;
	else if (!_wcsicmp(tag, L"publisher"))	value = t->publisher;
	else if (!_wcsicmp(tag, L"composer"))	value = t->composer;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintf(buf, 128, L"%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"tracknumber"))
	{
		if (t->track > 0)
		{
			StringCchPrintf(buf, 128, L"%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bitrate")) {
		if(t->bitrate > 0) {
			StringCchPrintf(buf, 128, L"%d", t->bitrate);
			value = buf;
		}
	}
	else if(!_wcsicmp(tag, L"discnum")) {
		if(t->discnum > 0) {
			StringCchPrintf(buf, 128, L"%02d", t->discnum);
			value = buf;
		}
	}
	else
		return 0;

	if (!value) return 0;
	else return _wcsdup(value); // TODO: not the most efficient way
}
void fieldTagFuncFree(wchar_t * tag, void * p) { //return 0 if not found
	if(tag) free(tag);
}

wchar_t pldir[MAX_PATH];
int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)pldir);
	return 0;
}

static INT_PTR CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
  static UsbDevice * dev;
  switch(uMsg) {
    case WM_INITDIALOG:
      {
        prefsParam* p = (prefsParam*)lParam;
        dev = (UsbDevice*)p->dev;
        p->config_tab_init(hwndDlg,p->parent);
        SetDlgItemTextW(hwndDlg,IDC_NAMEFORMAT,dev->songFormat);
        SetDlgItemTextW(hwndDlg,IDC_PLDIR,dev->pldir);
        SetDlgItemTextW(hwndDlg,IDC_SUPPORTEDFORMATS,dev->supportedFormats);
        if(dev->purgeFolders[0]=='1') CheckDlgButton(hwndDlg,IDC_PURGEFOLDERS,BST_CHECKED);
        else CheckDlgButton(hwndDlg,IDC_PURGEFOLDERS,BST_UNCHECKED);

		SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_SLASH_AT_START));
		SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_DOT_AT_START));
		SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_NO_SLASH_OR_DOT));
		SendDlgItemMessage(hwndDlg,IDC_PL_WRITE_COMBO,CB_SETCURSEL,dev->pl_write_mode,0);
		SetDlgItemTextW(hwndDlg,IDC_PL_WRITE_EG,WASABI_API_LNGSTRINGW(IDS_EG_SLASH+dev->pl_write_mode));
      }
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
      case IDC_NAMEFORMAT:
        if(HIWORD(wParam)==EN_CHANGE) {
          GetDlgItemTextW(hwndDlg,IDC_NAMEFORMAT,dev->songFormat,sizeof(dev->songFormat)/sizeof(wchar_t));
          WritePrivateProfileStringW(L"pmp_usb",L"songFormat",dev->songFormat,dev->ini_file);
        }
        break;
      case IDC_PLDIR:
        if(HIWORD(wParam)==EN_CHANGE) {
          GetDlgItemTextW(hwndDlg,IDC_PLDIR,dev->pldir,sizeof(dev->pldir)/sizeof(wchar_t));
          WritePrivateProfileStringW(L"pmp_usb",L"pldir",dev->pldir,dev->ini_file);
        }
        break;
      case IDC_SUPPORTEDFORMATS:
        if(HIWORD(wParam)==EN_CHANGE) {
          GetDlgItemTextW(hwndDlg,IDC_SUPPORTEDFORMATS,dev->supportedFormats,sizeof(dev->supportedFormats)/sizeof(wchar_t));
          WritePrivateProfileStringW(L"pmp_usb",L"supportedFormats",dev->supportedFormats,dev->ini_file);
        }
        break;
      case IDC_REFRESHCACHE:
        {
          wchar_t titleStr[32];
          dev->refreshCache();
		  MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_CACHE_UPDATED),
					 WASABI_API_LNGSTRINGW_BUF(IDS_SUCCESS,titleStr,32),MB_OK);
          break;
        }
	  case IDC_PL_WRITE_COMBO:
		{
		  dev->pl_write_mode = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
		  SetDlgItemTextW(hwndDlg,IDC_PL_WRITE_EG,WASABI_API_LNGSTRINGW(IDS_EG_SLASH+dev->pl_write_mode));
		  wchar_t tmp[16];
		  StringCchPrintf(tmp, 16, L"%d", dev->pl_write_mode);
		  WritePrivateProfileStringW(L"pmp_usb",L"pl_write_mode",tmp,dev->ini_file);
		  break;
		}

      case IDC_FILENAMEHELP:
        {
			wchar_t titleStr[64];
			MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_FILENAME_FORMATTING_INFO),
					   WASABI_API_LNGSTRINGW_BUF(IDS_FILENAME_FORMAT_HELP,titleStr,64),MB_OK);
        }
        break;
      case IDC_PLBROWSE:
        {	
          wchar_t *tempWS;
          BROWSEINFO bi;
          LPITEMIDLIST iil;
          LPMALLOC lpm;
          wchar_t bffFileName[MAX_PATH];
          ZeroMemory(&bi,sizeof(bi));
          bffFileName[0] = '\0';
          bi.hwndOwner = hwndDlg;
          bi.pszDisplayName = bffFileName;
		  bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_SELECT_FOLDER_TO_LOAD_PLAYLISTS);
          bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
		  bi.lpfn = BrowseCallbackProc;
		  lstrcpynW(pldir, dev->pldir, MAX_PATH);

          if((iil = SHBrowseForFolder(&bi)) != NULL)
          {
            SHGetPathFromIDListW(iil,bffFileName);
            SHGetMalloc(&lpm);
            // path is now in bffFileName
          }

          tempWS = _wcsdup(bffFileName);
          if(tempWS[0] == dev->drive) {
            lstrcpynW(dev->pldir, tempWS, MAX_PATH);
            SetDlgItemText(hwndDlg,IDC_PLDIR,tempWS);
          }
          else 
            if(bffFileName[0] != 0) //dont print error if the user selected 'cancel'
			{
				wchar_t titleStr[32];
				MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_ERR_SELECTED_PATH_NOT_ON_DEVICE),
						   WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32), MB_OK);
			}
			free(tempWS);
        }
        break;
      case IDC_FORMATSHELP:
        {
			wchar_t titleStr[64];
			MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_SUPPORTED_FORMAT_INFO),
					   WASABI_API_LNGSTRINGW_BUF(IDS_SUPPORTED_FORMAT_HELP,titleStr,64),MB_OK);
        }
        break;
      case IDC_PURGEFOLDERS:
        {
          if(IsDlgButtonChecked(hwndDlg,IDC_PURGEFOLDERS) == BST_CHECKED)
            wcsncpy(dev->purgeFolders,L"1",2);
          else
            wcsncpy(dev->purgeFolders,L"0",2);
          WritePrivateProfileStringW(L"pmp_usb",L"purgeFolders",dev->purgeFolders,dev->ini_file);
        }
        break;
      case IDC_RESCAN:
        {
        //update changes
        SetFileAttributesW(dev->ini_file,FILE_ATTRIBUTE_HIDDEN);

        wchar_t driveletter = dev->drive; //hold on to driveletter before it goes away
        //disconnect
        dev->Close();

        //connect
        pmpDeviceLoading load;
        dev = new UsbDevice(driveletter,&load);
		wchar_t titleStr[64];
		MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_RESCAN_COMPLETE_SAVED),
					WASABI_API_LNGSTRINGW_BUF(IDS_RESCAN_COMPLETE,titleStr,64),MB_OK);
        }
        break;
      }
  }
  return 0;
}

intptr_t UsbDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4) {
  switch(param1) {
  case DEVICE_SET_ICON:
    {
      MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
      i->hinst = plugin.hDllInstance;
      switch(devType) {
        case TYPE_PSP: i->resourceId = IDR_PSP_ICON; break;
        default:       i->resourceId = IDR_USB_ICON; break;
      }
    }
    break;
  case DEVICE_SUPPORTED_METADATA:
    {
      intptr_t supported = SUPPORTS_ARTIST | SUPPORTS_ALBUM | SUPPORTS_TITLE | SUPPORTS_TRACKNUM | SUPPORTS_DISCNUM | SUPPORTS_GENRE | 
        SUPPORTS_YEAR | SUPPORTS_SIZE | SUPPORTS_LENGTH | SUPPORTS_BITRATE | SUPPORTS_LASTUPDATED | SUPPORTS_ALBUMARTIST | 
				SUPPORTS_COMPOSER | SUPPORTS_PUBLISHER;
      return supported;
    }
    break;
  case DEVICE_CAN_RENAME_DEVICE:
    return 1;
  case DEVICE_GET_INI_FILE:
    wcsncpy((wchar_t*)param2,ini_file,MAX_PATH);
    break;
  case DEVICE_GET_PREFS_DIALOG:
    if(param3 == 0) {
      pref_tab * p = (pref_tab *)param2;
      p->hinst = WASABI_API_LNG_HINST;
      p->dlg_proc = config_dialogProc;
      p->res_id = IDD_CONFIG;
	  WASABI_API_LNGSTRINGW_BUF(IDS_ADVANCED,p->title,100);
    }
    break;
  }
  return 0;
}
__int64 UsbDevice::songSizeOnHardDrive(songid_t song) {
	return fileSize(((UsbSong*)song)->filename);
}

int UsbDevice::copyToHardDrive(songid_t song, // the song to copy
															 wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
															 void * callbackContext, //pass this to the callback
															 void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
															 int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
															 ) // -1 for failed/not supported. 0 for success.
{
	wchar_t * ext = wcsrchr(((UsbSong*)song)->filename,L'.'); // the file extention (eg ".mp3")
	if(ext && wcslen(ext) < 10) StringCchCat(path,MAX_PATH, ext); // append correct extention
	return CopyFile(((UsbSong*)song)->filename,path,callbackContext,callback,killswitch); // copy file
}

void UsbDevice::outputXML(void) {
	int top = ((Playlist*)playlists.Get(0))->songs.GetSize();
	/*if(top>0)*/ { //dont make a file if theres nothing to print!
		FILE *f;
		f = _wfopen(xmlFile, L"wb");
		UsbSong * songToPrint;
		int i;
		int top = ((Playlist*)playlists.Get(0))->songs.GetSize();
		//XML will prodruce error in parser if part of tag is &, <, or >
		wchar_t fn[MAX_PATH];
		wchar_t artist[fieldlen], album[fieldlen], title[fieldlen], genre[fieldlen], albumartist[fieldlen], publisher[fieldlen], composer[fieldlen];
		if(f) {
			fputws(L"\xFEFF",f);
			fputws(L"<?xml version=\"1.0\" encoding=\"UTF-16\" ?>\r\n",f);
			fputws(L"<!-- Warning: These elements are used by Winamp to load song metadata. Editing this file will cause Winamp to display incorrect track information.  -->\r\n",f);
			fputws(L"<USBDeviceLibrary>\r\n",f);
			for(i=0;i<top;i++)
			{
				//replace &, <, and > for XML output
				songToPrint=(UsbSong*)((Playlist*)playlists.Get(0))->songs.Get(i);
				//if(!songToPrint->filled) continue;
				fixFilenameForXML(fn, songToPrint->filename);
				fixTagsForXML(artist, songToPrint->artist);
				fixTagsForXML(albumartist, songToPrint->albumartist);
				fixTagsForXML(publisher, songToPrint->publisher);
				fixTagsForXML(composer, songToPrint->composer);
				fixTagsForXML(album, songToPrint->album);
				fixTagsForXML(title, songToPrint->title);
				fixTagsForXML(genre, songToPrint->genre);

				//output

				fputws(L"\t<Song ",f);
				fputws(L"Filename=\"",f); fputws(fn,f); fputws(L"\" ",f);
				fputws(L"Artist=\"",f); fputws(artist,f); fputws(L"\" ",f);
				fputws(L"Album=\"",f); fputws(album,f); fputws(L"\" ",f);
				fputws(L"Title=\"",f); fputws(title,f); fputws(L"\" ",f);
				fputws(L"Genre=\"",f); fputws(genre,f); fputws(L"\" ",f);
				fputws(L"AlbumArtist=\"",f); fputws(albumartist,f); fputws(L"\" ",f);
				fputws(L"Publisher=\"",f); fputws(publisher,f); fputws(L"\" ",f);
				fputws(L"Composer=\"",f); fputws(composer,f); fputws(L"\" ",f);
				fputws(L"Year=\"",f); fwprintf(f,L"%d",songToPrint->year); fputws(L"\" ",f);
				fputws(L"Track=\"",f); fwprintf(f,L"%d",songToPrint->track); fputws(L"\" ",f);
				fputws(L"Bitrate=\"",f); fwprintf(f,L"%d",songToPrint->bitrate); fputws(L"\" ",f);
				fputws(L"Discnum=\"",f); fwprintf(f,L"%d",songToPrint->discnum); fputws(L"\" ",f);
				fputws(L"Length=\"",f); fwprintf(f,L"%d",songToPrint->length); fputws(L"\" ",f);
				fputws(L"Size=\"",f); fwprintf(f,L"%I64d",songToPrint->size); fputws(L"\" ",f);
				fputws(L"/>\r\n",f);
			}
			fputws(L"</USBDeviceLibrary>\r\n",f);
		}
		if(f) fclose(f);
	}
}

void fixFilenameForXML(wchar_t *dest, const wchar_t *cstr)
{
	int tindex = 0;
	wchar_t filename[MAX_PATH];
	for(int i=0;i<fieldlen && tindex<fieldlen;i++)
	{
		if(cstr[i]==L'&')
		{
			if(tindex < MAX_PATH-5)
			{
				filename[tindex++] = '&';
				filename[tindex++] = 'a';
				filename[tindex++] = 'm';
				filename[tindex++] = 'p';
				filename[tindex] = ';';
			}
			else
				filename[tindex] = ' ';
		}else if(tindex < MAX_PATH)
			filename[tindex] = cstr[i];
		if(cstr[i] == 0) break;
		tindex++;
	}
	wcsncpy(dest, filename, fieldlen);
}
void fixTagsForXML(wchar_t* dest, const wchar_t *cstr)
{
	int tindex = 0;
	wchar_t temp[fieldlen];
	for(int i=0;i<fieldlen && tindex<fieldlen;i++)
	{
		switch(cstr[i]) {
		case(L'&'):
			if(tindex < fieldlen-5) {
				temp[tindex++] = '&';
				temp[tindex++] = 'a';
				temp[tindex++] = 'm';
				temp[tindex++] = 'p';
				temp[tindex] = ';';
			}
			else  temp[tindex] = ' '; //no room
			break;
		case(L'<'):
			{
				if(tindex < fieldlen-4) {
					temp[tindex++] = '&';
					temp[tindex++] = 'l';
					temp[tindex++] = 't';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			}
		case(L'>'): {
			if(tindex < fieldlen-4) {
				temp[tindex++] = '&';
				temp[tindex++] = 'g';
				temp[tindex++] = 't';
				temp[tindex] = ';';
			}
			else  temp[tindex] = ' '; //no room
			break;
								}
  case(L'\"'): {
      if(tindex < fieldlen-4) {
        temp[tindex++] = '&';
        temp[tindex++] = 'q';
        temp[tindex++] = 'u';
        temp[tindex++] = 'o';
        temp[tindex++] = 't';
        temp[tindex] = ';';
      }
      else  temp[tindex] = ' '; //no room
      break;
                }
   case(L'\''): {
      if(tindex < fieldlen-4) {
        temp[tindex++] = '&';
        temp[tindex++] = 'a';
        temp[tindex++] = 'p';
        temp[tindex++] = 'o';
        temp[tindex++] = 's';
        temp[tindex] = ';';
      }
      else  temp[tindex] = ' '; //no room
      break;
                }
		default: {
			temp[tindex] = cstr[i];
			break;
						 }
		}
		if(cstr[i] == 0) break;
		tindex++;
	}
	wcsncpy(dest, temp, fieldlen);
}

extern wchar_t *guessTitles(const wchar_t *filename, 
                 int *tracknum,
                 wchar_t **artist, 
                 wchar_t **album,
                 wchar_t **title);

static void guessMetadata(UsbSong *t) {
  wchar_t *artist=NULL,*album=NULL,*title=NULL;
  wchar_t * f = guessTitles(t->filename,&t->track,&artist,&album,&title);
  if(artist) lstrcpyn(t->artist,artist,fieldlen);
  if(album) lstrcpyn(t->album,album,fieldlen);
  if(title) lstrcpyn(t->title,title,fieldlen);
  t->size = fileSize(t->filename);
  free(f);
}

void UsbDevice::fillMetaData(UsbSong *s, wchar_t *fileName)
{
	if(!s->filled) {
    wchar_t *g_artist, *g_album, *g_title, *g_free;
    int g_track;
    g_free = guessTitles(fileName,&g_track,&g_artist,&g_album,&g_title);

    getMetaItem(L"title", fileName, s->title, fieldlen);
    if(s->title[0]==0 && g_title && *g_title) lstrcpyn(s->title,g_title,fieldlen);
    else { g_track=-1; g_album=L""; g_artist=L""; }
		getMetaItem(L"artist", fileName, s->artist, fieldlen);
    if(s->artist[0]==0 && g_artist && *g_artist) lstrcpyn(s->artist,g_artist,fieldlen);
		getMetaItem(L"album", fileName, s->album, fieldlen);
		if(s->album[0]==0 && g_album && *g_album) lstrcpyn(s->album,g_album,fieldlen);
		getMetaItem(L"genre", fileName, s->genre, fieldlen);
		getMetaItem(L"albumartist", fileName, s->albumartist, fieldlen);
		getMetaItem(L"publisher", fileName, s->publisher, fieldlen);
		getMetaItem(L"composer", fileName, s->composer, fieldlen);
		s->year = getMetaItemInt(L"year", fileName);
		s->track = getMetaItemInt(L"track", fileName);
    if(!s->track) s->track = g_track;
		s->bitrate=getMetaItemInt(L"bitrate", fileName);
		s->size = fileSize(fileName);
		s->discnum = getMetaItemInt(L"disc",fileName);	

		//get track length
		basicFileInfoStructW b={0};
		b.filename=fileName;
		b.quickCheck=0;
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&b,IPC_GET_BASIC_FILE_INFOW);
		s->length = b.length * 1000;
		s->filled = true;
    free(g_free);
	}
}

bool UsbDevice::cacheIsCurrent() {
	//For fLastAccess/LastWrite information, use GetFileAttributesEx
	wchar_t cur[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA cacheFileInfo, tempInfo;
	GetFileAttributesExW(xmlFile, GetFileExInfoStandard, (LPVOID)&cacheFileInfo);
	Playlist *mpl = (Playlist*)playlists.Get(0);
	for(int i = 0; i < mpl->songs.GetSize(); i++) //cycle through all songs
	{
		StringCchPrintf(cur, MAX_PATH, L"%s", ((UsbSong*)mpl->songs.Get(i))->filename);
		if(cur)
			GetFileAttributesExW(cur, GetFileExInfoStandard, (LPVOID)&tempInfo);

		//cachetime - song time
		if(CompareFileTime(&cacheFileInfo.ftLastWriteTime, &tempInfo.ftLastWriteTime) < 0) 
			return false;
	}
	return true;
}

// Start-NDE
bool UsbDevice::isNDECacheCurrent() {
	//For fLastAccess/LastWrite information, use GetFileAttributesEx
	wchar_t cur[MAX_PATH];
	WIN32_FILE_ATTRIBUTE_DATA cacheFileInfo, tempInfo;
	GetFileAttributesExW(ndeDataFile, GetFileExInfoStandard, (LPVOID)&cacheFileInfo);
	GetFileAttributesExW(ndeIndexFile, GetFileExInfoStandard, (LPVOID)&cacheFileInfo);
	Playlist *mpl = (Playlist*)playlists.Get(0);
	for(int i = 0; i < mpl->songs.GetSize(); i++) //cycle through all songs
	{
		StringCchPrintf(cur, MAX_PATH, L"%s", ((UsbSong*)mpl->songs.Get(i))->filename);
		if(cur)
			GetFileAttributesExW(cur, GetFileExInfoStandard, (LPVOID)&tempInfo);

		//cachetime - song time
		if(CompareFileTime(&cacheFileInfo.ftLastWriteTime, &tempInfo.ftLastWriteTime) < 0) 
			return false;
	}
	return true;
}

void UsbDevice::refreshNDECache(void) {
	SetFileAttributesW(ndeDataFile,FILE_ATTRIBUTE_NORMAL);
	SetFileAttributesW(ndeIndexFile,FILE_ATTRIBUTE_NORMAL);
	outputNDE();
	SetFileAttributesW(ndeDataFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
	SetFileAttributesW(ndeIndexFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
}
// End-NDE


void UsbDevice::refreshCache(void) {
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
	outputXML();
	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
}

#if 0
void UsbDevice::tag(void) {
	FILE *fRead; //fread is solely to see if a tag_cache.xml exists
	//SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_HIDDEN);
	cacheUpToDate = true;  //loadSongFromCache will set to false if song cannot be found
	fRead = _wfopen(xmlFile, L"r");
	Playlist *mpl = ((Playlist*)playlists.Get(0));
	if(fRead && cacheIsCurrent()) { //cacheIsCurrent checks fLastWrite time and compares to see if cache is stale
		fclose(fRead);
		//fRead = _wfopen(ini_file, L"r"); //check path of ini file... do NOT run xmlreader for a bad device... it will fail
		//initialize and load using Winamp's XML reader
		CacheLoader loader;
		if(loader.Load(xmlFile, this)) { //if API_CACHELOADER_FAILURE, Cache corrupt
			//output xml file after reading tags
			SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
			OutputDebugString(L"Corrupt XML File!\n");	
			int i;

			int top = mpl->songs.GetSize();
			//first load in all songs data from ID3 - this is what we were trying to avoid
      for(i = 0; i < top; i++) {
        UsbSong *t = (UsbSong*)mpl->songs.Get(i);
        guessMetadata(t);
        metadataWait.Add(t);
      }
				//fillMetaData((UsbSong*)mpl->songs.Get(i),((UsbSong*)mpl->songs.Get(i))->filename);
			//fill cache after reading tags
			//outputXML();
		}
		else { /*cache existed OR no ini file... see if there are other songs on the device we missed*/
			UsbSong *t;
			for(int sIndex = 0; sIndex < mpl->songs.GetSize(); sIndex++)	{			
				t = (UsbSong*)mpl->songs.Get(sIndex);
				if(!((UsbSong*)mpl->songs.Get(sIndex))->filled) { //tag songs not included in cache
					cacheUpToDate = false;
					//fillMetaData(t, t->filename);
          guessMetadata(t);
          metadataWait.Add(t);
				}
			}
			SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
			outputXML();
		}
	}
	else { //xml file does not exist or is not current; make xml file
		if(fRead) fclose(fRead);
		//output to xml format
		int i;
		int top = mpl->songs.GetSize();
		//first load in all songs data from ID3 - this is what we were trying to avoid
    for(i = 0; i < top; i++) {
      UsbSong *t = (UsbSong *)mpl->songs.Get(i);
      guessMetadata(t);
      metadataWait.Add(t);
			//fillMetaData(t,t->filename);  //could actually put this in cacheiscurrent() and save a loop
    }
		//fill cache
		SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_NORMAL);
		outputXML();
	}

	SetFileAttributesW(xmlFile,FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
	cacheUpToDate = true; 
	loadedUpToDate = true;
}
#endif

// Start-NDE
void UsbDevice::tag(void) {
	Nullsoft::Utility::AutoLock lock(dbcs);
	Playlist *mpl = ((Playlist*)playlists.Get(0));

	if((isNDECachePopulated() == NDE_USB_SUCCESS) && (isNDECacheCurrent())) { 
		// isNDECacheCurrent checks fLastWrite time and compares to see if cache is stale
		// initialize and load the NDE table
		// Read from NDE table
		nde_scanner_t scanner = NDE_Table_CreateScanner(deviceTable);
		if (scanner) {
			NDE_Scanner_First(scanner);
			while (!NDE_Scanner_EOF(scanner)) {
				UsbSong song;
				
				nde_field_t filename = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_FILENAME);
				wchar_t* filenameString = NDE_StringField_GetString(filename);
				lstrcpyn(song.filename, filenameString, fieldlen);

				nde_field_t artist = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ARTIST);
				wchar_t* artistString = NDE_StringField_GetString(artist);
				lstrcpyn(song.artist, artistString, fieldlen);

				nde_field_t album = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ALBUM);
				wchar_t* albumString = NDE_StringField_GetString(album);
				lstrcpyn(song.album, albumString, fieldlen);

				nde_field_t albumArtist = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ALBUM_ARTIST);
				wchar_t* albumArtistString = NDE_StringField_GetString(albumArtist);
				lstrcpyn(song.albumartist, albumArtistString, fieldlen);

				nde_field_t publisher = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_PUBLISHER);
				wchar_t* publisherString = NDE_StringField_GetString(publisher);
				lstrcpyn(song.publisher, publisherString, fieldlen);

				nde_field_t composer = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_COMPOSER);
				wchar_t* composerString = NDE_StringField_GetString(composer);
				lstrcpyn(song.composer, composerString, fieldlen);

				nde_field_t title = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_TITLE);
				wchar_t* titleString = NDE_StringField_GetString(title);
				lstrcpyn(song.title, titleString, fieldlen);

				nde_field_t genre = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_GENRE);
				wchar_t* genreString = NDE_StringField_GetString(genre);
				lstrcpyn(song.genre, genreString, fieldlen);

				nde_field_t track = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_TRACK);
				int trackInt= NDE_IntegerField_GetValue(track);
				song.track = trackInt;

				nde_field_t year = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_YEAR);
				int yearInt= NDE_IntegerField_GetValue(year);
				song.year = yearInt;

				nde_field_t discNumber = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_DISC_NUMBER);
				int discNumberInt= NDE_IntegerField_GetValue(discNumber);
				song.discnum = discNumberInt;

				nde_field_t length = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_LENGTH);
				int lengthInt= NDE_IntegerField_GetValue(length);
				song.length = lengthInt;

				nde_field_t bitrate = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_BITRATE);
				int bitrateInt= NDE_IntegerField_GetValue(bitrate);
				song.bitrate = bitrateInt;

				nde_field_t size = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_SIZE);
				int sizeInt= NDE_IntegerField_GetValue(size);
				song.size = sizeInt;

				loadSongFromCache(&song);
				NDE_Scanner_Next(scanner);
			}
		} else {
			// No scanner, something went wrong, brute force erase the db and recreate
			NDE_DestroyDatabase(discDB);
			outputNDE();
		}
		// End read from NDE table
	}
	else { 
		// NDE database/table does not exist or is not current; create it
		int top = mpl->songs.GetSize();
		
		//first load in all songs data from ID3 - this is what we were trying to avoid
		for(int i = 0; i < top; i++) {
			UsbSong *t = (UsbSong *)mpl->songs.Get(i);
			//guessMetadata(t);
			wchar_t tmp[1024]={0,};

			if (getFileInfoW(t->filename,L"artist",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					StringCchCopyW(t->artist, fieldlen, tmp);
				}
			}
			
			if (getFileInfoW(t->filename,L"title",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) { 
					StringCchCopyW(t->title, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename,L"album",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) { 
					StringCchCopyW(t->album, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename,L"composer",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) { 
					StringCchCopyW(t->composer, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename,L"publisher",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) { 
					StringCchCopyW(t->publisher, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename,L"albumartist",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) { 
					StringCchCopyW(t->albumartist, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename, L"length",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					t->length = _wtoi(tmp);
				}
			}

			if (getFileInfoW(t->filename, L"track",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					t->track = _wtoi(tmp);
				}
			}

			if (getFileInfoW(t->filename, L"disc",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					t->discnum = _wtoi(tmp);
				}
			}

			if (getFileInfoW(t->filename, L"genre",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					StringCchCopyW(t->genre, fieldlen, tmp);
				}
			}

			if (getFileInfoW(t->filename, L"year",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0] && !wcsstr(tmp,L"__") && !wcsstr(tmp,L"/") && !wcsstr(tmp,L"\\") && !wcsstr(tmp,L".")) { 
					wchar_t *p=tmp;
					while (*p) {
						if (*p == L'_') *p=L'0';
						p++;
					}
					int y=_wtoi(tmp);
					t->year = _wtoi(tmp);
				}
			}

			if (getFileInfoW(t->filename, L"bitrate",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					t->bitrate = _wtoi(tmp);
				}
			}

			if (getFileInfoW(t->filename, L"size",tmp,sizeof(tmp)/sizeof(wchar_t))) {
				if(tmp[0]) {
					t->size = _wtoi(tmp);
				} 
			} else {
				t->size = fileSize(t->filename);
			}
			metadataWait.Add(t);
		}
		//fill cache
		SetFileAttributesW(ndeDataFile, FILE_ATTRIBUTE_NORMAL);
		SetFileAttributesW(ndeIndexFile, FILE_ATTRIBUTE_NORMAL);
		outputNDE();
	}

	SetFileAttributesW(ndeDataFile, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
	SetFileAttributesW(ndeIndexFile, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
	cacheUpToDate = true; 
	loadedUpToDate = true;
}
// End-NDE

void UsbDevice::loadSongFromCache(UsbSong * s) {

  UsbSong *cur = BinaryChopFind(s->filename,(Playlist*)playlists.Get(0));
	if(!cur) return; // song not found. This isn't a problem

	//enough with the parsing... lets actually set our values
	lstrcpyn(cur->artist,s->artist,fieldlen);
	lstrcpyn(cur->albumartist,s->albumartist,fieldlen);
	lstrcpyn(cur->publisher,s->publisher,fieldlen);
	lstrcpyn(cur->composer,s->composer,fieldlen);
	lstrcpyn(cur->album,s->album,fieldlen);
	cur->bitrate = s->bitrate;
	cur->discnum = s->discnum;
	lstrcpyn(cur->genre,s->genre,fieldlen);
	cur->length = s->length;
	cur->size = s->size;
	lstrcpyn(cur->title,s->title,fieldlen);
	cur->track = s->track;
	cur->year = s->year;
	cur->filled = true;
} 

// START - OutputNDE
// Method to write into a NDE database table instead of being output in XML
// NDE is the mechanism of choice for all storage and retrieval in winamp
// XML is overkill and must be avoided if possible
void UsbDevice::outputNDE(void) {
	Nullsoft::Utility::AutoLock lock(dbcs);
	openDeviceTable();
	nde_scanner_t s = NDE_Table_CreateScanner(deviceTable);
	SeekToBegininngOfDevice(s);

	int top = ((Playlist*)playlists.Get(0))->songs.GetSize();

	UsbSong * songToPrint;
	for(int i=0;i<top;i++)
	{
		NDE_Scanner_New(s);
		songToPrint=(UsbSong*)((Playlist*)playlists.Get(0))->songs.Get(i);
		if (songToPrint->filename) {
			db_setFieldString(s, DEVICEVIEW_COL_FILENAME, songToPrint->filename);
		}

		if (songToPrint->artist) {
			db_setFieldString(s, DEVICEVIEW_COL_ARTIST, songToPrint->artist);
		}

		if (songToPrint->albumartist) {
			db_setFieldString(s, DEVICEVIEW_COL_ALBUM_ARTIST, songToPrint->albumartist);
		}

		if (songToPrint->publisher) {
			db_setFieldString(s, DEVICEVIEW_COL_PUBLISHER, songToPrint->publisher);
		}

		if (songToPrint->composer) {
			db_setFieldString(s, DEVICEVIEW_COL_COMPOSER, songToPrint->composer);
		}

		if (songToPrint->album) {
			db_setFieldString(s, DEVICEVIEW_COL_ALBUM, songToPrint->album);
		}

		if (songToPrint->title) {
			db_setFieldString(s, DEVICEVIEW_COL_TITLE, songToPrint->title);
		}

		if (songToPrint->genre) {
			db_setFieldString(s, DEVICEVIEW_COL_GENRE, songToPrint->genre);
		}

		if (songToPrint->year) {
			db_setFieldInt(s, DEVICEVIEW_COL_YEAR, songToPrint->year);
		}

		if (songToPrint->track) {
			db_setFieldInt(s, DEVICEVIEW_COL_TRACK, songToPrint->track);
		}

		if (songToPrint->bitrate) {
			db_setFieldInt(s, DEVICEVIEW_COL_BITRATE, songToPrint->bitrate);
		}

		if (songToPrint->discnum) {
			db_setFieldInt(s, DEVICEVIEW_COL_DISC_NUMBER, songToPrint->discnum);
		}

		if (songToPrint->length) {
			db_setFieldInt(s, DEVICEVIEW_COL_LENGTH, songToPrint->length);
		}

		if (songToPrint->size) {
			db_setFieldInt(s, DEVICEVIEW_COL_SIZE, songToPrint->size);
		}
		NDE_Scanner_Post(s);
	}
	NDE_Table_DestroyScanner(deviceTable, s);
	NDE_Table_Sync(deviceTable);
	closeDeviceTable();
}


nde_database_t UsbDevice::discDB = 0;
nde_table_t UsbDevice::deviceTable = 0;
Nullsoft::Utility::LockGuard UsbDevice::dbcs;
wchar_t UsbDevice::ndeDataFile[100] = {0};
wchar_t UsbDevice::ndeIndexFile[100] = {0};

void UsbDevice::createDeviceFields() 
{
	// create defaults
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_FILENAME,"filename",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_ARTIST,"artist",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_ALBUM,"album",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_TITLE,"title",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_GENRE,"genre",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_ALBUM_ARTIST,"albumartist",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_PUBLISHER,"publisher",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_COMPOSER,"composer",FIELD_STRING);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_YEAR,"year",FIELD_INTEGER);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_TRACK,"track",FIELD_INTEGER);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_BITRATE,"bitrate",FIELD_INTEGER);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_DISC_NUMBER,"discnumber",FIELD_INTEGER);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_LENGTH,"length",FIELD_INTEGER);
	NDE_Table_NewColumn(deviceTable, DEVICEVIEW_COL_SIZE,"size",FIELD_INTEGER);
	NDE_Table_PostColumns(deviceTable);
	NDE_Table_AddIndexByID(deviceTable, 0, "filename");
}

int UsbDevice::getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, size_t len)
{
	dest[0]=0;
	return AGAVE_API_METADATA->GetExtendedFileInfo(filename, metadata, dest, len);
}

int UsbDevice::openDeviceDatabase() 
{
	Nullsoft::Utility::AutoLock lock(dbcs);
	if (!discDB) 
	{
		discDB = NDE_CreateDatabase(plugin.hDllInstance);
	}
	return NDE_USB_SUCCESS;
}

int UsbDevice::isNDECachePopulated() 
{
	if (openDeviceTable() == NDE_USB_SUCCESS) {
		int rows = NDE_Table_GetRecordsCount(deviceTable);
		if (rows) return NDE_USB_SUCCESS;
	}
	return NDE_USB_FAILURE;
}

int UsbDevice::openDeviceTable() {
	Nullsoft::Utility::AutoLock lock(dbcs);
	int ret = openDeviceDatabase();
	if (ret != NDE_USB_SUCCESS)
		return ret;

	if (!deviceTable)
	{
		deviceTable = NDE_Database_OpenTable(discDB, ndeDataFile, ndeIndexFile,NDE_OPEN_ALWAYS,NDE_CACHE); 
		if (deviceTable) {
			createDeviceFields();
		}
	}
	return deviceTable?NDE_USB_SUCCESS:NDE_USB_FAILURE;
}

void UsbDevice::closeDeviceTable()
{
	if (deviceTable)
	{
		NDE_Table_Sync(deviceTable);
		NDE_Database_CloseTable(discDB, deviceTable);
		deviceTable=0;
	}

	NDE_DestroyDatabase(discDB);
	discDB=0;
}

void UsbDevice::db_setFieldInt(nde_scanner_t s, unsigned char id, int data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_IntegerField_SetValue(f, data);
}

void UsbDevice::db_setFieldString(nde_scanner_t s, unsigned char id, const wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_StringField_SetString(f, data);
}

void UsbDevice::db_removeField(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		NDE_Scanner_DeleteField(s, f);
	}
}

int UsbDevice::db_getFieldInt(nde_scanner_t s, unsigned char id, int defaultVal)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_IntegerField_GetValue(f);
	else
		return defaultVal;
}

wchar_t* UsbDevice::db_getFieldString(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
		return NDE_StringField_GetString(f);
	else
		return 0;
}

void UsbDevice::SeekToBegininngOfDevice(nde_scanner_t s)
{
	NDE_Scanner_First(s, 0);
}

// END - OutputNDE

UsbSong::UsbSong() {
  filename[0]=artist[0]=album[0]=title[0]=genre[0]=albumartist[0]=publisher[0]=composer[0]=0;
  filled=year=track=length=discnum=bitrate=(int)(size=0);
}

