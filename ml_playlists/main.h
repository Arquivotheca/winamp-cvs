#ifndef NULLSOFT_ML_PLAYLISTS_MAIN_H
#define NULLSOFT_ML_PLAYLISTS_MAIN_H

#include <windows.h>
#include <shlwapi.h>
#include "resource.h"

#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../Winamp/wa_ipc.h"

#include "config.h"
#include "PlaylistInfo.h"
#include "../nu/MediaLibraryInterface.h"
#include "./menu.h"

#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"

#include "../nu/ChildSizer.h"
#include "../nu/DialogSkinner.h"
#include "api.h"
#include "../nu/AutoLock.h"
#include "../nu/Map.h"

#define WINAMP_MANAGEPLAYLISTS          40385
#define ID_DOSHITMENU_ADDNEWPLAYLIST    40031

INT_PTR pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

extern INT_PTR playlistsTreeId, playlistsCloudTreeId;
extern HNAVITEM playlistItem;
extern int imgPL, imgCloudPL;
INT_PTR LoadPlaylist(INT_PTR treeId);

extern HMENU wa_playlists_cmdmenu;
void HookPlaylistEditor();
void UnhookPlaylistEditor();

INT_PTR CALLBACK view_playlistsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK AddPlaylistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK view_playlistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);


extern winampMediaLibraryPlugin plugin;
extern HMENU g_context_menus;
extern wchar_t g_path[MAX_PATH];
static const bool ADD_TO_TREE = true;
extern HINSTANCE cloud_hinst;
extern int IPC_GET_CLOUD_HINST, IPC_GET_CLOUD_ACTIVE;
extern int cloud_avail, normalimage, cloudImage;
extern HWND currentView;

void AddPlaylist(int callback, const wchar_t *title, const wchar_t *filename,
				 bool makeTree, int cloud, size_t numItems = 0, uint64_t length=0);
void LoadPlaylists();
void UpdatePlaylists();
int AddToCloud();
void playlists_Add(HWND parent, bool callback = true);
void playlists_AddToCloudPrompt(HWND hwndDlg);

enum { SORT_TITLE_ASCENDING, SORT_TITLE_DESCENDING, SORT_NUMBER_ASCENDING, SORT_NUMBER_DESCENDING };
void playlists_Sort(size_t sort_type);

wchar_t *createPlayListDBFileName(wchar_t *filename); // filename is ignored but used for temp space, make sure it's 1024+256 chars =)
void Playlist_importFromWinamp();
void Playlist_importFromFile(HWND dlgparent);
void Playlist_importFromFolders(HWND dlgparent);
bool FindTreeItem(INT_PTR treeId);

void RenamePlaylist(GUID _guid, HWND parent);
void DeletePlaylist(GUID _guid, HWND parent, bool confirm);

void RefreshPlaylistsList();

void HookMediaLibrary();
void UnhookMediaLibrary();
void playlist_ContextMenu(HWND hwndDlg, POINT p);

extern C_Config *g_config;

extern int (*warand)(void);

void MakeTree(PlaylistInfo &playlist);
void UpdateTree(PlaylistInfo &playlist, int tree_id);

LRESULT pluginHandleIpcMessage(int msg, WPARAM param);
extern HCURSOR hDragNDropCursor;
void FormatLength(wchar_t *str, int length, int buf_len);

extern int IPC_LIBRARY_SENDTOMENU;

void Hook(HWND winamp);

extern INT_PTR sendToIgnoreID;
extern INT_PTR lastActiveID;
void playlist_Reload(bool forced = false);
void playlist_Save(HWND hwndDlg);
void playlist_SaveGUID(GUID _guid);
void playlist_ReloadGUID(GUID _guid);
extern HWND activeHWND; // active playlist view

int CALLBACK WINAPI _bcp( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
INT_PTR PlayPlaylist(INT_PTR treeId);

INT_PTR playlists_OnClick(INT_PTR treeId, int clickType, HWND wnd);
int playlists_OnKeyDown(int treeId, NMTVKEYDOWN *p, HWND hwndDlg);
int playlists_OnDrag(int treeId, POINT *pt, int *type);
int playlists_OnDrop(int treeId, POINT *pt, int destTreeId);

int playlists_CloudAvailable();

/* SendTo.cpp */
int playlists_BuildSendTo(int sourceType, INT_PTR context);
void AddPlaylistFromFilenames(const char *filenames, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename=0);
void AddPlaylistFromFilenamesW(const wchar_t *filenames, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename=0);
void AddPlaylistFromItemRecordList(itemRecordList *obj, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename=0);
void AddPlaylistFromItemRecordListW(itemRecordListW *obj, const wchar_t *playlistTitle, int makeTree, const wchar_t *filename=0);
int playlists_OnDropTarget(int id, int type, INT_PTR data);
int playlists_OnSendTo(int sourceType, INT_PTR data, int id);

void SwapPlayEnqueueInMenu(HMENU listMenu);
void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu);

extern int uniqueAddress;

typedef nu::Map<INT_PTR, GUID> TREE_TO_GUID_MAP;
extern TREE_TO_GUID_MAP tree_to_guid_map;

#define DEFAULT_PL_SEND_TO 1
#endif