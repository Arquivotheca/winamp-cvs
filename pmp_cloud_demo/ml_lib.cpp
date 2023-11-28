/*
** Copyright (C) 2003-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#include <windows.h>
#include <../gen_ml/ml.h>

/*
---------------------------------- 
wide version starts here
---------------------------------- 
*/
 
 void freeRecord(itemRecordW *p)
 {
	free(p->title);
	free(p->artist);
	free(p->comment);
	free(p->album);
	free(p->genre);
	free(p->filename);
	free(p->albumartist); 
	free(p->replaygain_album_gain); 
	free(p->replaygain_track_gain); 
	free(p->publisher);
	free(p->composer);
	if (p->extended_info)
	{
		int x=0;
		for (x = 0; p->extended_info[x].key; x ++)
		{
			free(p->extended_info[x].key);
			free(p->extended_info[x].value);
		}
		free(p->extended_info);
	}
	memset(p,0,sizeof(itemRecordW));
}

void freeRecordList(itemRecordListW *obj)
{
	emptyRecordList(obj);
	free(obj->Items);
	obj->Items=0;
	obj->Alloc=obj->Size=0;
}

void emptyRecordList(itemRecordListW *obj)
{
	itemRecordW *p=obj->Items;
	while (obj->Size-->0)
	{
		freeRecord(p);
		p++;
	}
	obj->Size=0;
}

void allocRecordList(itemRecordListW *obj, int newsize, int granularity)
{
	if (newsize < obj->Alloc || newsize < obj->Size) return;

	obj->Alloc=newsize+granularity;
	obj->Items=(itemRecordW*)realloc(obj->Items,sizeof(itemRecordW)*obj->Alloc);
	if (!obj->Items) obj->Alloc=0;
}

void copyRecord(itemRecordW *out, const itemRecordW *in)
{
	int y;
#define COPYSTR(FOO) out->FOO = in->FOO ? _wcsdup(in->FOO) : 0;
#define COPY(FOO) out->FOO = in->FOO;
	COPYSTR(filename)
	COPYSTR(title)
	COPYSTR(album)
	COPYSTR(artist)
	COPYSTR(comment)
	COPYSTR(genre)
	COPYSTR(albumartist); 
	COPYSTR(replaygain_album_gain); 
	COPYSTR(replaygain_track_gain); 
	COPYSTR(publisher);
	COPYSTR(composer);
	COPY(year);
	COPY(track);
	COPY(tracks);
	COPY(length);
	COPY(rating);
	COPY(playcount); 
	COPY(lastplay); 
	COPY(lastupd); 
	COPY(filetime);
	COPY(filesize);
	COPY(bitrate); 
	COPY(type); 
	COPY(disc); 
	COPY(bpm);
	COPY(discs);  
#undef COPYSTR
	out->extended_info=0;

	if (in->extended_info) for (y = 0; in->extended_info[y].key; y ++)
	{
		setRecordExtendedItem(out,in->extended_info[y].key,in->extended_info[y].value);
	}
}

void copyRecordList(itemRecordListW *out, const itemRecordListW *in)
{
	int x;
	allocRecordList(out,out->Size+in->Size,0);
	if (!out->Items) return;
	for (x = 0; x < in->Size; x ++)
	{
		copyRecord(&out->Items[out->Size++],&in->Items[x]);
	}
}

wchar_t *getRecordExtendedItem(const itemRecordW *item, const wchar_t *name)
{
	size_t x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x].key; x ++)
	{
		if (!_wcsicmp(item->extended_info[x].key,name))
			return item->extended_info[x].value;
	}
	return NULL;
}

void setRecordExtendedItem(itemRecordW *item, const wchar_t *name, const wchar_t *value)
{
	size_t x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x].key; x ++)
	{
		if (!_wcsicmp(item->extended_info[x].key,name))
		{
			free(item->extended_info[x].value);
			item->extended_info[x].value = _wcsdup(value);
			return;
		}
	}
	// x=number of valid items.
	item->extended_info=(extendedInfoW *)realloc(item->extended_info,sizeof(extendedInfoW) * (x+2));
	if (item->extended_info)
	{
		item->extended_info[x].key = _wcsdup(name);
		item->extended_info[x].value = _wcsdup(value);

		item->extended_info[x+1].key=0;
		item->extended_info[x+1].value=0;
	}
}

void setRecordExtendedItemUTF8(itemRecordW *item, const wchar_t *name, const unsigned char *value, size_t bytes)
{
	size_t x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x].key; x ++)
	{
		if (!_wcsicmp(item->extended_info[x].key,name))
		{
			free(item->extended_info[x].value);
			item->extended_info[x].value = 0;
			int size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, 0, 0);
			if (size)
			{
					item->extended_info[x].value = (wchar_t *)malloc((size+1)*sizeof(wchar_t));
					if (MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, item->extended_info[x].value, size))
					{
						item->extended_info[x].value[size]=0; // null terminate
					}
					else
					{
						free(item->extended_info[x].value);
						item->extended_info[x].value=0;
					}
			}

			return;
		}
	}
	// x=number of valid items.
	item->extended_info=(extendedInfoW *)realloc(item->extended_info,sizeof(extendedInfoW) * (x+2));
	if (item->extended_info)
	{
		item->extended_info[x].key = _wcsdup(name);
		item->extended_info[x].value = 0;
		int size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, 0, 0);
		if (size)
		{
			item->extended_info[x].value = (wchar_t *)malloc((size+1)*sizeof(wchar_t));
			if (MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, item->extended_info[x].value, size))
			{
				item->extended_info[x].value[size]=0; // null terminate
			}
			else
			{
				free(item->extended_info[x].value);
				item->extended_info[x].value=0;
			}
		}

		item->extended_info[x+1].key=0;
		item->extended_info[x+1].value=0;
	}
}
