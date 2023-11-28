#pragma once
#include "../nde/nde_c.h"
#include "../nde/ndestring.h"
#include "../nu/PtrDeque.h"
#include "../nu/AutoLock.h"
#include <bfc/platform/types.h>

class CollectedData
{
public:
	CollectedData();
	CollectedData(const CollectedData &copy);
	~CollectedData();
	// init from filename
	bool Populate(const wchar_t *filename);
	// init from database scanner (method lives in db.cpp)
	void Populate(nde_scanner_t scanner);

	void Reset();
public:
	// note: all strings are nde strings (reference counted)
	int32_t id;
	wchar_t *filename;
	time_t timestamp;
	wchar_t *artist;
	wchar_t *album;
	wchar_t *title;
	int32_t track;
	wchar_t *genre;
	wchar_t *mimetype;
	int32_t playLength;

	// populated flag indicate whether metadata has already been populated
	bool populated;

private:
	void Init();
};

/* db.cpp */
bool AddCollectedData(const CollectedData &data);
 /* retrieves and removes an entry in the database
 if you fail to submit it, be sure to put it back using AddCollectedData
 returns false if there's nothing in the DB
 */
bool GetCollectedData(CollectedData &data);

/* queue.cpp */
void AddCollectedDataToAvQueue(const CollectedData &data);

/* submit.cpp */
enum
{
	SUBMIT_OK,
	SUBMIT_FAILED, // submission failed, blow away from database
	SUBMIT_TRY_AGAIN, // submission failed, but safe to try again
};

// submits to MUD.  returns SUBMIT_OK if successful
int Submit(const CollectedData &data);

// submits to AVTrack. returns SUBMIT_OK if successful
int SubmitAvTrack(CollectedData *data);