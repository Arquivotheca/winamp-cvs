/*
** Copyright (C) 2003-2013 Nullsoft, Inc.
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
#include "ml.h"

void freeRecord(itemRecord *p)
{
	free(p->title);
	free(p->artist);
	free(p->comment);
	free(p->album);
	free(p->genre);
	free(p->filename);
	if (p->extended_info)
	{
		int x=0;
		for (x = 0; p->extended_info[x]; x ++)
		free(p->extended_info[x]);
		free(p->extended_info);
	}
	memset(p,0,sizeof(itemRecord));
}

void freeRecordList(itemRecordList *obj)
{
	emptyRecordList(obj);
	free(obj->Items);
	obj->Items=0;
	obj->Alloc=obj->Size=0;
}

void emptyRecordList(itemRecordList *obj)
{
	itemRecord *p=obj->Items;
	while (obj->Size-->0)
	{
		freeRecord(p);
		p++;
	}
	obj->Size=0;
}

void allocRecordList(itemRecordList *obj, int newsize, int granularity)
{
	if (newsize < obj->Alloc || newsize < obj->Size) return;

	obj->Alloc=newsize+granularity;
	obj->Items=(itemRecord*)realloc(obj->Items,sizeof(itemRecord)*obj->Alloc);
	if (!obj->Items) obj->Alloc=0;
}

void copyRecord(itemRecord *out, const itemRecord *in)
{
	int y;
#define COPYSTR(FOO) out->FOO = in->FOO ? _strdup(in->FOO) : 0;
	COPYSTR(filename)
	COPYSTR(title)
	COPYSTR(album)
	COPYSTR(artist)
	COPYSTR(comment)
	COPYSTR(genre)
	out->year=in->year;
	out->track=in->track;
	out->length=in->length;
#undef COPYSTR
	out->extended_info=0;

	if (in->extended_info)
	{
		for (y = 0; in->extended_info[y]; y ++)
		{
			char *p=in->extended_info[y];
			if (*p) setRecordExtendedItem(out,p,p+strlen(p)+1);
		}
	}
}

void copyRecordList(itemRecordList *out, const itemRecordList *in)
{
	int x;
	allocRecordList(out,out->Size+in->Size,0);
	if (!out->Items) return;
	for (x = 0; x < in->Size; x ++)
	{
		copyRecord(&out->Items[out->Size++],&in->Items[x]);
	}
}

char *getRecordExtendedItem(const itemRecord *item, const char *name)
{
	int x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x]; x ++)
	{
		if (!_stricmp(item->extended_info[x],name))
		return item->extended_info[x]+strlen(name)+1;
	}
	return NULL;
}

void setRecordExtendedItem(itemRecord *item, const char *name, char *value)
{
	int x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x]; x ++)
	{
		if (!_stricmp(item->extended_info[x],name))
		{
			if (strlen(value)>strlen(item->extended_info[x]+strlen(name)+1))
			{
				free(item->extended_info[x]);
				item->extended_info[x]=(char*)malloc(strlen(name)+strlen(value)+2);
			}
			strcpy(item->extended_info[x],name);
			strcpy(item->extended_info[x]+strlen(name)+1,value);
			return;
		}
	}
	// x=number of valid items.
	item->extended_info=(char**)realloc(item->extended_info,sizeof(char*) * (x+2));
	if (item->extended_info)
	{
		item->extended_info[x]=(char*)malloc(strlen(name)+strlen(value)+2);
		strcpy(item->extended_info[x],name);
		strcpy(item->extended_info[x]+strlen(name)+1,value);

		item->extended_info[x+1]=0;
	}
}

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
	COPYSTR(filename);
	COPYSTR(title);
	COPYSTR(album);
	COPYSTR(artist);
	COPYSTR(comment);
	COPYSTR(genre);
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
	COPY(discs);
	COPY(bpm);
	COPYSTR(category);
#undef COPYSTR
	out->extended_info=0;

	if (in->extended_info)
	{
		for (y = 0; in->extended_info[y].key; y ++)
		{
			setRecordExtendedItem(out,in->extended_info[y].key,in->extended_info[y].value);
		}
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

// TODO: redo this without AutoChar
#include "../nu/AutoChar.h"
#define COPY_EXTENDED_STR(field) if (input-> ## field && input-> ## field ## [0]) setRecordExtendedItem(output, #field, AutoChar(input-> ## field));
#define COPY_EXTENDED_INT(field) if (input->##field > 0) { char temp[64]; _itoa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
#define COPY_EXTENDED_INT64(field) if (input->##field > 0) { char temp[64]; _i64toa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
#define COPY_EXTENDED_INT0(field) if (input->##field >= 0) { char temp[64]; _itoa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
void convertRecord(itemRecord *output, const itemRecordW *input)
{
	int y;
	output->filename=AutoCharDup(input->filename);
	output->title=AutoCharDup(input->title);
	output->album=AutoCharDup(input->album);
	output->artist=AutoCharDup(input->artist);
	output->comment=AutoCharDup(input->comment);
	output->genre=AutoCharDup(input->genre);
	output->year=input->year;
	output->track=input->track;
	output->length=input->length;
	output->extended_info=0;
	COPY_EXTENDED_STR(albumartist);
	COPY_EXTENDED_STR(replaygain_album_gain);
	COPY_EXTENDED_STR(replaygain_track_gain);
	COPY_EXTENDED_STR(publisher);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_INT(tracks);
	COPY_EXTENDED_INT(rating);
	COPY_EXTENDED_INT(playcount);
	COPY_EXTENDED_INT64(lastplay);
	COPY_EXTENDED_INT64(lastupd);
	COPY_EXTENDED_INT64(filetime);
	COPY_EXTENDED_INT(filesize);
	COPY_EXTENDED_INT(bitrate);
	COPY_EXTENDED_INT0(type);
	COPY_EXTENDED_INT(disc);
	COPY_EXTENDED_INT(discs);
	COPY_EXTENDED_INT(bpm);
	COPY_EXTENDED_STR(category);

	if (input->extended_info)
	{
		for (y = 0; input->extended_info[y].key; y ++)
		{
			setRecordExtendedItem(output, AutoChar(input->extended_info[y].key), AutoChar(input->extended_info[y].value));
		}
	}
}
#undef COPY_EXTENDED_STR
#undef COPY_EXTENDED_INT
#undef COPY_EXTENDED_INT0

#include "../nu/AutoWide.h"
#define COPY_EXTENDED_STR(field) output->##field = AutoWideDup(getRecordExtendedItem(input, #field));
#define COPY_EXTENDED_INT(field) { char *x = getRecordExtendedItem(input, #field); output->##field=x?atoi(x):0; }

void convertRecord(itemRecordW *output, const itemRecord *input)
{
	output->filename=AutoWideDup(input->filename);
	output->title=AutoWideDup(input->title);
	output->album=AutoWideDup(input->album);
	output->artist=AutoWideDup(input->artist);
	output->comment=AutoWideDup(input->comment);
	output->genre=AutoWideDup(input->genre);
	output->year=input->year;
	output->track=input->track;
	output->length=input->length;
	output->extended_info=0;
	COPY_EXTENDED_STR(albumartist);
	COPY_EXTENDED_STR(replaygain_album_gain);
	COPY_EXTENDED_STR(replaygain_track_gain);
	COPY_EXTENDED_STR(publisher);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_INT(tracks);
	COPY_EXTENDED_INT(rating);
	COPY_EXTENDED_INT(playcount);
	COPY_EXTENDED_INT(lastplay);
	COPY_EXTENDED_INT(lastupd);
	COPY_EXTENDED_INT(filetime);
	COPY_EXTENDED_INT(filesize);
	COPY_EXTENDED_INT(type);
	COPY_EXTENDED_INT(disc);
	COPY_EXTENDED_INT(discs);
	COPY_EXTENDED_INT(bpm);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_STR(category);
	// TODO: copy input's extended fields
}
#undef COPY_EXTENDED_STR
#undef COPY_EXTENDED_INT

void convertRecordList(itemRecordList *output, const itemRecordListW *input)
{
	output->Alloc = output->Size = input->Size;
	output->Items = (itemRecord*)calloc(sizeof(itemRecord),input->Size);
	for(int i=0; i < input->Size; i++)
		convertRecord(&output->Items[i],&input->Items[i]);
}

void convertRecordList(itemRecordListW *output, const itemRecordList *input)
{
	output->Alloc = output->Size = input->Size;
	output->Items = (itemRecordW*)calloc(sizeof(itemRecordW),input->Size);
	for(int i=0; i < input->Size; i++)
		convertRecord(&output->Items[i],&input->Items[i]);
}