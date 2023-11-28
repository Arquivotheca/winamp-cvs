#ifndef ML_LOCAL_HEADER
#define ML_LOCAL_HEADER

#include <windows.h>
#include <commctrl.h>

#include "gaystring.h"
#include "config.h"

#include "../nu/Map.h"
#include "../nde/nde_c.h"

struct ExtendedFields
{
	const wchar_t *ispodcast;
	const wchar_t *podcastchannel;
	const wchar_t *podcastpubdate;
	const wchar_t *GracenoteFileID;
	const wchar_t *GracenoteExtData;
	const wchar_t *lossless;
	const wchar_t *codec;
	const wchar_t *director;
	const wchar_t *producer;
	const wchar_t *width;
	const wchar_t *height;
	const wchar_t *mimetype;
	const wchar_t *realsize;
	const wchar_t *dateadded;
	const wchar_t *cloud;
};
extern const ExtendedFields extended_fields;

extern const int NUM_EXTRA_COLSW;
extern const unsigned char extra_idsW[]; // defined in view_media.cpp
extern const wchar_t *extra_strsW[];

extern HCURSOR hDragNDropCursor;
wchar_t *getRecordExtendedItem_fast(const itemRecordW *item, const wchar_t *name);
void setRecordExtendedItem_fast(itemRecordW *item, const wchar_t *name, const wchar_t *value);

int OnLocalMediaItemClick(int action, int item, HWND parent);
int OnLocalMediaClick(int action, HWND parent);
BOOL IPC_HookExtInfo(INT_PTR param);
BOOL IPC_HookExtInfoW(INT_PTR param);
BOOL IPC_HookTitleInfo(INT_PTR param);

#include "../winamp/wa_ipc.h"


#define INT_ENTRY_MAX_NUM       20
#define INT_ENTRY_MAX_PATHSIZE  512
#define INT_ENTRY_MAX_TEXTSIZE  128

#define TREE_IMAGE_LOCAL_AUDIO				101
#define TREE_IMAGE_LOCAL_VIDEO				102
#define TREE_IMAGE_LOCAL_MOSTPLAYED			103
#define TREE_IMAGE_LOCAL_RECENTLYADDED		104
#define TREE_IMAGE_LOCAL_RECENTLYPLAYED		105
#define TREE_IMAGE_LOCAL_NEVERPLAYED		106
#define TREE_IMAGE_LOCAL_TOPRATED			107
#define TREE_IMAGE_LOCAL_PODCASTS			108
#define TREE_IMAGE_LOCAL_RECENTLYMODIFIED	109


#define MAINTABLE_ID_FILENAME			0
#define MAINTABLE_ID_TITLE				1
#define MAINTABLE_ID_ARTIST				2
#define MAINTABLE_ID_ALBUM				3
#define MAINTABLE_ID_YEAR				4
#define MAINTABLE_ID_GENRE				5
#define MAINTABLE_ID_COMMENT			6
#define MAINTABLE_ID_TRACKNB			7
#define MAINTABLE_ID_LENGTH				8  // in seconds
#define MAINTABLE_ID_TYPE				9  // 0=audio, 1=video
#define MAINTABLE_ID_LASTUPDTIME		10 // last time (seconds since 1970) of db update of this item
#define MAINTABLE_ID_LASTPLAY			11 // last time (seconds since 1970) of last play
#define MAINTABLE_ID_RATING				12
#define MAINTABLE_ID_GRACENOTE_ID		14 // OLD Gracenote ID's.  Don't use this anymore!!!
#define MAINTABLE_ID_PLAYCOUNT			15 // play count
#define MAINTABLE_ID_FILETIME			16 // file time
#define MAINTABLE_ID_FILESIZE			17 // file size, bytes (was kilobytes until 5.7)
#define MAINTABLE_ID_BITRATE			18 // file bitratea, kbps
#define MAINTABLE_ID_DISC				19 // disc #
#define MAINTABLE_ID_ALBUMARTIST		20 // album artist
#define MAINTABLE_ID_ALBUMGAIN			21 // album gain (replaygain)
#define MAINTABLE_ID_TRACKGAIN			22 // track gain (replaygain)
#define MAINTABLE_ID_PUBLISHER			23 // publisher (record label)
#define MAINTABLE_ID_COMPOSER			24 // composer
#define MAINTABLE_ID_BPM				25 // beats per minute (tempo)
#define MAINTABLE_ID_DISCS				26 // number of discs total
#define MAINTABLE_ID_TRACKS				27 // number of tracks total
#define MAINTABLE_ID_ISPODCAST			28
#define MAINTABLE_ID_PODCASTCHANNEL		29
#define MAINTABLE_ID_PODCASTPUBDATE		30
#define MAINTABLE_ID_GRACENOTEFILEID	31
#define MAINTABLE_ID_GRACENOTEEXTDATA	32
#define MAINTABLE_ID_LOSSLESS			33
#define MAINTABLE_ID_CATEGORY			34
#define MAINTABLE_ID_CODEC				35
#define MAINTABLE_ID_DIRECTOR			36
#define MAINTABLE_ID_PRODUCER			37
#define MAINTABLE_ID_WIDTH				38
#define MAINTABLE_ID_HEIGHT				39
#define MAINTABLE_ID_MIMETYPE			40
#define MAINTABLE_ID_DATEADDED			41 // time file was added to the db

// menu command id
#define IDM_DOSHITMENU_ADDNEWVIEW	40030
#define IDM_RESCANFOLDERSNOW		4066
#define IDM_ADD_DIRS				4067
#define IDM_REMOVE_UNUSED_FILES		4068
#define IDM_ADD_PLEDIT              4069

extern BOOL myMenu;

int init(void);
void config(void);
void quit(void);

int treeGetParam(HTREEITEM h);

class Table;
class C_Config;
extern CRITICAL_SECTION g_db_cs;
extern nde_database_t g_db;
extern nde_table_t g_table;
extern int g_table_dirty;
extern const char *WINAMP_INI;

extern HWND m_curview_hwnd;

extern char g_path[];
extern char g_tableDir[], g_viewsDir[];
extern C_Config *g_config;

extern HMENU g_context_menus;

typedef struct
{
	wchar_t *name;
	wchar_t *query;
	char *metafn; //filename, without path, of meta file
	int mode;
	int imgIndex;
	int index;
} queryItem;

typedef nu::Map <int, queryItem*> QueryList;
extern QueryList  m_query_list;
extern C_Config *g_view_metaconf;
extern int g_guessifany;
extern int g_querydelay;
extern int g_viewnotplay;

void loadQueryTree();
extern int m_query_tree;
extern int m_query_mode;
static char *m_query_metafile;
HWND onTreeViewSelectChange(HWND hwnd);

void db_setFieldStringW(nde_scanner_t s, unsigned char id, const wchar_t *data);
void db_setFieldInt(nde_scanner_t s, unsigned char id, int data);
void db_setFieldInt64(nde_scanner_t s, unsigned char id, __int64 data);
int db_getFieldInt(nde_scanner_t s, unsigned char id, int defaultVal);
void db_removeField(nde_scanner_t s, unsigned char id);

void main_playQuery(C_Config *metaconf, const wchar_t *query, int enqueue, int startplaying=1); // enqueue =-1 sends it to the playlist
void main_playItemRecordList (itemRecordListW *obj, int enqueue, int startplaying=1);
int addQueryItem(const wchar_t *name, const wchar_t *val, int mode, int select, const char *metafn, int imageIndex, int num=-1);
void replaceQueryItem(int n, const wchar_t *name, const wchar_t *val, int mode, int imageIndex);
void saveQueryTree();

int pluginHandleIpcMessage(int msg, int param);

int openDb();
void closeDb();

void nukeLibrary(HWND hwndDlg);

//add.cpp
int addFileToDb(const wchar_t *filename, int onlyupdate, int use_metadata, int guess_mode, int playcnt=0, int lastplay=0, bool force=false); // adds a file to the db, gets info, etc.
int RemoveFileFromDB(/*const Table *table, */const wchar_t *filename);	// removes a file from the DB
void makeFilename2(const char *filename, char *filename2, int filename2_len);
void makeFilename2W(const wchar_t *filename, wchar_t *filename2, int filename2_len);

//gracenote.cpp
void gracenoteInit();
int gracenoteQueryFile(const wchar_t *filename);
void gracenoteCancelRequest();
int gracenoteDoTimerStuff();
void gracenoteSetValues(const wchar_t *artist, const wchar_t *album, const wchar_t *title);
const wchar_t *gracenoteGetTuid();
int gracenoteIsWorking();

//guess.cpp
wchar_t *guessTitles(const wchar_t *filename, 
					 int *tracknum,
					 wchar_t **artist,
					 wchar_t **album,
					 wchar_t **title); // should free the result of this function after using artist/album/title

//prefs.cpp
INT_PTR CALLBACK PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

int autoscan_add_directory(const wchar_t *path, int *guess, int *meta, int *recurse, int noaddjustcheck);// if we return 1, guess and meta will be filled in
void refreshPrefs(int screen);

//util.cpp
extern "C" {
	char *scanstr_back(char *str, char *toscan, char *defval);
	wchar_t *scanstr_backW(wchar_t *str, wchar_t *toscan, wchar_t *defval);
	char *extension(char *fn);
	wchar_t *extensionW(wchar_t *fn);
	void process_substantives(wchar_t* dest);
	void ConvertRatingMenuStar(HMENU menu, UINT menu_id);
};

//view_audio.cpp
INT_PTR CALLBACK view_audioDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

//view_miniinfo.cpp
INT_PTR CALLBACK view_miniinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

//view_errorinfo.cpp
INT_PTR CALLBACK view_errorinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

/* bgscan.cpp */
void Scan_ScanFolderBackground(const wchar_t *path, int guess, int meta, int recurse);
void Scan_ScanFolder(HWND parent, const wchar_t *path, int guess, int meta, int recurse);
// When you call Scan_ScanFolders, it will own the memory and release it with free()
void Scan_ScanFolders(HWND parent, size_t count, wchar_t **paths, int *guess, int *meta, int *recurse);
void Scan_BackgroundScan();
void Scan_Cancel();
void Scan_Kill();
// remove missing files
void Scan_RemoveFiles(HWND parent);

//view_media.cpp
void makeQueryStringFromText(GayStringW *query, wchar_t *text, int nf=8);
inline BOOL WINAPI IsCharSpaceA(char c) { return (c == ' ' || c == '\t'); }
inline BOOL WINAPI IsCharSpaceW(wchar_t c) { return (c == L' ' || c == L'\t'); }
inline bool IsThe(const char *str) { if (str && (str[0] == 't' || str[0] == 'T') && (str[1] == 'h' || str[1] == 'H') && (str[2] == 'e' || str[2] == 'E') && (str[3] == ' ')) return true; else return false; }
__forceinline static bool IsTheW(const wchar_t *str) 
{
	if ((str[0] & ~0x20) == L'T'
		&& (str[1] & ~0x20) == L'H'
		&& (str[2] & ~0x20) == L'E'
		&& str[3] == L' ')
		return true;
	else
		return false;
}
#define SKIP_THE_AND_WHITESPACE(x) { char *save##x=(char*)x; while (IsCharSpaceA(*x) && *x) x++; if (IsThe(x)) x+=4; while (IsCharSpaceA(*x)) x++; if (!*x) x=save##x; }
#define SKIP_THE_AND_WHITESPACEW(x) { wchar_t *save##x=(wchar_t*)x; while ((*x == L' ' || *x == L'\t') && *x) x++; if (IsTheW(x)) x+=4; while ((*x == L' ' || *x == L'\t')) x++; if (!*x) x=save##x; }
//wherever this goes is fine

#define UPDATE_QUERY_TIMER_ID 505
#define UPDATE_RESULT_LIST_TIMER_ID 506
INT_PTR CALLBACK view_mediaDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
int WCSCMP_NULLOK(const wchar_t *pa, const wchar_t *pb);

typedef void (*resultsniff_funcW)(itemRecordW *items, int numitems, int user32, int *killswitch);

void bgQuery_Stop();
extern nde_scanner_t m_media_scanner;
typedef nu::PtrList<wchar_t> CloudFiles;

// returns length in seconds (high bit on means could be much more), or -1 if killed
int saveQueryToListW(C_Config *viewconf, nde_scanner_t s, itemRecordListW *obj,
					 CloudFiles *uploaded, CloudFiles *uploading,
					 resultsniff_funcW cb=0, int user32=0, int *killswitch=0, __int64 *total_bytes=0);

// queries.cpp
void view_queryContextMenu(INT_PTR param1, HWND hHost, POINTS pts, int item);
void queriesContextMenu(INT_PTR param1, HWND hHost, POINTS pts);
void queryEditItem(int n);
void addNewQuery(HWND parent);
void queryDeleteItem(HWND parent, int n);
BOOL windowOffScreen(HWND hwnd, POINT pt);

// handleMessage.cpp
INT_PTR HandleIpcMessage(INT_PTR msg, INT_PTR param);
extern "C" extern int (*warand)(void);

extern WNDPROC wa_oldWndProc;

wchar_t *itemrecordWTagFunc(wchar_t * tag, void * p);
wchar_t *fieldTagFunc(wchar_t * tag, void * p); //return 0 if not found
void ndeTagFuncFree(wchar_t * tag, void * p); // for NDE strings
DWORD doGuessProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
void TAG_FMT_EXT(const wchar_t *filename, void *f, void *ff, void *p, wchar_t *out, int out_len, int extended);
extern int asked_for_playcount;
LRESULT APIENTRY wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int m_calling_getfileinfo;
extern HMENU wa_play_menu ;
void add_pledit_to_library();
void add_to_library(HWND wndparent);
extern int g_bgscan_scanning, g_bgrescan_force, g_bgrescan_do, g_bgrescan_int;
extern WNDPROC ml_oldWndProc;
extern time_t g_bgscan_last_rescan;
int runBGscan(int ms);
void compactRecordList(itemRecordListW *obj);

LRESULT APIENTRY ml_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int FLOATCMP_NULLOK(const char *pa, const char *pb);
int FLOATWCMP_NULLOK(const wchar_t *pa, const wchar_t *pb);
void ClearTitleHookCache();

int FindFileInDatabase(nde_scanner_t s, int fieldId, const wchar_t *filename, wchar_t alternate[MAX_PATH]);

__int64 ScannerRefToObjCacheNFNW(nde_scanner_t s, itemRecordListW *obj, bool compat);
__int64 ScannerRefToObjCacheNFNW(nde_scanner_t s, itemRecordW *obj, bool compat);
int updateFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *data);
void sortResults(C_Config *viewconf, itemRecordListW *obj);
void queryStrEscape(const char *raw, GayString &str);
void queryStrEscape(const wchar_t *raw, GayStringW &str);
void ParseIntSlashInt(wchar_t *string, int *part, int *parts);

HWND updateCurrentView(HWND hwndDlg);

#define MEDIAVIEW_COL_ARTIST			0
#define MEDIAVIEW_COL_TITLE				1
#define MEDIAVIEW_COL_ALBUM				2
#define MEDIAVIEW_COL_LENGTH			3
#define MEDIAVIEW_COL_TRACK				4
#define MEDIAVIEW_COL_GENRE				5
#define MEDIAVIEW_COL_YEAR				6
#define MEDIAVIEW_COL_FILENAME			7
#define MEDIAVIEW_COL_RATING			8
#define MEDIAVIEW_COL_PLAYCOUNT			9
#define MEDIAVIEW_COL_LASTPLAY			10
#define MEDIAVIEW_COL_LASTUPD			11
#define MEDIAVIEW_COL_FILETIME			12
#define MEDIAVIEW_COL_COMMENT			13
#define MEDIAVIEW_COL_FILESIZE			14
#define MEDIAVIEW_COL_BITRATE			15
#define MEDIAVIEW_COL_TYPE				16
#define MEDIAVIEW_COL_DISC				17
#define MEDIAVIEW_COL_ALBUMARTIST		18
#define MEDIAVIEW_COL_FULLPATH			19
#define MEDIAVIEW_COL_ALBUMGAIN			20
#define MEDIAVIEW_COL_TRACKGAIN			21
#define MEDIAVIEW_COL_PUBLISHER			22
#define MEDIAVIEW_COL_COMPOSER			23
#define MEDIAVIEW_COL_EXTENSION			24
#define MEDIAVIEW_COL_ISPODCAST			25
#define MEDIAVIEW_COL_PODCASTCHANNEL	26
#define MEDIAVIEW_COL_PODCASTPUBDATE	27
#define MEDIAVIEW_COL_BPM				28
#define MEDIAVIEW_COL_CATEGORY			29
#define MEDIAVIEW_COL_DIRECTOR			30
#define MEDIAVIEW_COL_PRODUCER			31
#define MEDIAVIEW_COL_DIMENSION			32
#define MEDIAVIEW_COL_DATEADDED			33
#define MEDIAVIEW_COL_CLOUD				34

#define MEDIAVIEW_COL_NUMS 35			// number of columns
#endif // ML_LOCAL_HEADER