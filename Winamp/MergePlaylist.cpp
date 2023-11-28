#include "main.h"
#include "MergePlaylist.h"
#include <strsafe.h>

merge_pl_entry::merge_pl_entry(const wchar_t *fn, const wchar_t *ft, int len)
: cached(false), filename(0), filetitle(0)
{
	SetFilename(fn);
	SetTitle(ft);
	SetLengthMilliseconds(len);
}
merge_pl_entry::~merge_pl_entry()
{
	plstring_release(filename);
	plstring_release(filetitle);
}

size_t merge_pl_entry::GetFilename(wchar_t *filename, size_t filenameCch)
{
	if (!this->filename)
		return 0;

	if (!filename)
		return lstrlenW(this->filename);

	StringCchCopyW(filename, filenameCch, this->filename);
	return 1;
}

size_t merge_pl_entry::GetTitle(wchar_t *title, size_t titleCch)
{
	if (!filetitle)
		return 0;

	if (!title)
		return lstrlenW(filetitle);

	StringCchCopyW(title, titleCch, filetitle);
	return 1;

}

int merge_pl_entry::GetLengthInMilliseconds()
{
	return length;
}

size_t merge_pl_entry::GetExtendedInfo(const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	return 0;
}


void merge_pl_entry::SetFilename(const wchar_t *filename)
{
	plstring_release(this->filename);

	if (filename && filename[0])
		this->filename = plstring_wcsdup(filename);
	else
		this->filename=0;
}

void merge_pl_entry::SetTitle(const wchar_t *title)
{
	plstring_release(filetitle);
	if (title && title[0])
	{
		filetitle = plstring_wcsdup(title);
		cached=true;
	}
	else
		filetitle=0;
}

void merge_pl_entry::SetLengthMilliseconds(int length)
{
	if (length<=0)
		this->length=-1000;
	else
		this->length=length;
}



MergePlaylist::MergePlaylist()
{
	total_time=0;
}
void MergePlaylist::Clear()
{
	entries.deleteAll();
}

void MergePlaylist::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	if (lengthInMS > 0)
		total_time += lengthInMS;
	entries.push_back(new merge_pl_entry(filename, title, lengthInMS));
}

void MergePlaylist::AppendWithInfo(const wchar_t *filename, const wchar_t *title, int lengthInMS)
{
	if (lengthInMS > 0)
		total_time += lengthInMS;

	entries.push_back(new merge_pl_entry(filename, title, lengthInMS));
}

MergePlaylist::~MergePlaylist()
{
	entries.deleteAll();
}

size_t MergePlaylist::GetNumItems()
{
	return entries.size();
}

size_t MergePlaylist::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->GetFilename(filename, filenameCch);
}

size_t MergePlaylist::GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->GetTitle(title, titleCch);
}

const wchar_t *MergePlaylist::ItemTitle(size_t item)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->filetitle;
}

const wchar_t *MergePlaylist::ItemName(size_t item)
{
	if (item >= entries.size())
		return 0;

	return entries[item]->filename;
}

int MergePlaylist::GetItemLengthMilliseconds(size_t item)
{
	if (item >= entries.size())
		return -1;
	return entries[item]->GetLengthInMilliseconds();
}

size_t MergePlaylist::GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	if (item >= entries.size())
		return 0;
	return entries[item]->GetExtendedInfo(metadata, info, infoCch);
}


void MergePlaylist::SetItemFilename(size_t item, const wchar_t *filename)
{
	if (item < entries.size())
	{
		entries[item]->SetFilename(filename);
	}
}

void MergePlaylist::SetItemTitle(size_t item, const wchar_t *title)
{
	if (item < entries.size())
	{
		entries[item]->SetTitle(title);
	}
}

void MergePlaylist::SetItemLengthMilliseconds(size_t item, int length)
{
	if (item < entries.size())
	{
		entries[item]->SetLengthMilliseconds(length);
	}
}

void MergePlaylist::AppendPlaylist(MergePlaylist &copy)
{
	size_t cnt = copy.GetNumItems();
	for (size_t i = 0;i != cnt;i++)
	{
		if (copy.entries[i]->filename && !HasFilename(copy.entries[i]->filename))
		{
			if (copy.entries[i]->length > 0)
				total_time += copy.entries[i]->length;

			entries.push_back(copy.entries[i]);
		}
	}
	copy.entries.clear();
}

bool MergePlaylist::HasFilename(const wchar_t *filename)
{
	for (size_t i = 0;i != entries.size();i++)
	{
		if (entries[i]->filename && !_wcsicmp(filename, entries[i]->filename))
			return true;
	}
	return false;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MergePlaylist

START_MULTIPATCH;
START_PATCH(patch_playlist)
M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_CLEAR, Clear)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPENDWITHINFO, AppendWithInfo)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPEND, Append)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEM, GetItem)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
M_CB(patch_playlist, ifc_playlist, IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
NEXT_PATCH(patch_playlistloadercallback)
M_VCB(patch_playlistloadercallback, ifc_playlistloadercallback, IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile);
END_PATCH
END_MULTIPATCH;
