#pragma once

#include "../nu/vector.h"
#include <bfc/platform/types.h>

class IpodSong;

class IpodPlaylist
{
public:
	IpodPlaylist();
	IpodPlaylist(BOOL master, int pid);
	~IpodPlaylist();
	
public:
	// utility
	BOOL isMaster() { return master; }
	wchar_t* getName() { return name; }

public:
	int64_t pid;
	LPTSTR name;
	Vector<IpodSong*> songs;
	int dateCreated;
	int dateModified;
	int numItems;
	BOOL master;
};


