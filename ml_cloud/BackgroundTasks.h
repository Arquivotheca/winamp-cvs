#pragma once
#include "api.h"
#include "../replicant/cloud/ifc_clouddb.h"
#include "../nu/threadpool/TimerHandle.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxuri.h"
#include "../ml_local/MLDBCallback.h"
#include "PlaylistsCB.h"
#include "../replicant/cloud/Attributes.h"
#include "../replicant/cloud/CloudCallback.h"

class Cloud_Background : public MLDBCallback//, public CloudCallback
{
public:
	Cloud_Background();
	~Cloud_Background();

	int Initialize();
	size_t Release();

	void Kill();
	void Rescan(int missing_only);
	void CredentialsChanged(int force=0);

	int IsKilled() { return killswitch; }
	void OnReset();
	void OnFirstPull(int clean, bool forced);

	void OnFileAdded(const wchar_t *filename);
	void OnFileUpdated(const wchar_t *filename, bool from_library);
	void OnFileRemove_Post(const wchar_t *filename);
	void OnCleared(const wchar_t **filenames, int count);
	void OnFilePlayed(const wchar_t *filename, time_t played, int count);
	void OnGetCloudStatus(const wchar_t *filename, HMENU *menu);
	void OnProcessCloudStatus(int menu_item, int *result);

	static int Background_Run(HANDLE handle, void *user_data, intptr_t id);
	static int Background_Add(HANDLE handle, void *user_data, intptr_t id);
	static int Background_Update(HANDLE handle, void *user_data, intptr_t id);
	static int Background_AddFiles(HANDLE handle, void *user_data, intptr_t id);
	
	static int Background_MediaHash(HANDLE handle, void *user_data, intptr_t id);
	static int Background_AlbumArt(HANDLE handle, void *user_data, intptr_t id);
	static int Background_CheckLogin(HANDLE handle, void *user_data, intptr_t id);
	static int Background_RefreshDevices(HANDLE handle, void *user_data, intptr_t id);
	static int Background_Remove(HANDLE handle, void *user_data, intptr_t id);
	static int Background_FilePlayed(HANDLE handle, void *user_data, intptr_t id);
	
	ifc_clouddb *Get_DB_Connection() { return external_db_connection; }
	ifc_clouddb *Get_UI_DB_Connection() { return ui_db_connection; 	}

	Attributes *Get_Attributes() { return &attributes; }
private:
	bool Internal_Initialize();

	int *mediahash_ids;
	size_t num_mediahash_ids;
	size_t mediahash_itr;

	int *albumart_ids;
	size_t num_albumart_ids;
	size_t albumart_itr;

	ThreadID *background_thread;
	ifc_clouddb *db_connection;
	ifc_clouddb *external_db_connection;
	ifc_clouddb *ui_db_connection;
	
	volatile int login_attempts;
	volatile int load_attempts;
	volatile int killswitch;
	volatile int first_pull;
	TimerHandle login_timer, add_timer, media_timer;
	Attributes attributes;
	wchar_t last_menu_filepath[MAX_PATH];
	int last_menu_playlist;
};