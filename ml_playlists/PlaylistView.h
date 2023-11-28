#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLISTVIEW_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLISTVIEW_H

#include "../nu/listview.h"
#define WM_PLAYLIST	WM_USER + 1980

#define WM_PLAYLIST_RELOAD	WM_PLAYLIST + 1
#define WM_PLAYLIST_UNLOAD	WM_PLAYLIST + 2

class PlayListView : public W_ListView
{
public:
		
};
// TODO: make a nice pretty class to wrap the playlist view

BOOL playlist_Notify(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
void PlaySelection(int enqueue, int is_all);
extern int we_are_drag_and_dropping;
void UpdatePlaylistTime(HWND hwndDlg);
void TagEditor(HWND hwnd);
extern W_ListView playlist_list;
 void SyncPlaylist();
 void Playlist_DeleteSelected(int selected);
 void Playlist_ResetSelected();
 void EditEntry(HWND);
#endif