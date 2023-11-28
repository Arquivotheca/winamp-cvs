#include "IpodDevice2.h"
#include "ipodplaylist.h"

#include <shlwapi.h>
#include <strsafe.h>

// dtor
// cleanup the memory allocated for the vector of songs
IpodPlaylist::~IpodPlaylist() 
{
}

// default ctor
IpodPlaylist::IpodPlaylist() 
: pid(0), master(false), dateCreated(0), dateModified(0), numItems(0), name(NULL)
{
}

// this is the constructor that gets called
IpodPlaylist::IpodPlaylist(BOOL m, int id) 
	: master(m), pid(id), dateCreated(0), dateModified(0), numItems(0), name(NULL)
{
}
