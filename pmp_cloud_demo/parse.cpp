#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yajl/yajl_parse.h>
#include <../gen_ml/ml.h>
#include <malloc.h>

void setRecordExtendedItemUTF8(itemRecordW *item, const wchar_t *name, const unsigned char *value, size_t bytes);

static void FillUTF8(wchar_t **ptr, const unsigned char *value, size_t bytes)
{
	wchar_t *str=0;
	int size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, 0, 0);
	if (size)
	{
		str = (wchar_t *)malloc((size+1)*sizeof(wchar_t));
		if (MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, bytes, str, size))
		{
			str[size]=0; // null terminate
		}
		else
		{
			free(str);
			str=0;
		}
	}
	*ptr = str;
}

ItemParser::~ItemParser()
{
	yajl_free(parser);
}

struct ParseState : public ItemParser
{
	ParseState();
	itemRecordW *active_item;

	/* these will either be valid or NULL depending on the last-received map key */
	wchar_t **active_field_string;
	int *active_field_int;
	__time64_t *active_field_time;
	const wchar_t *extended_key;

	template <class _t>
	void SetActiveField(_t *ptr);	

	template <>
	void SetActiveField<wchar_t *>(wchar_t **ptr)
	{
		active_field_string=ptr;
	}

	template <>
	void SetActiveField<int>(int *ptr)
	{
		active_field_int=ptr;
	}

	template <>
	void SetActiveField<__time64_t>(__time64_t *ptr)
	{
		active_field_time=ptr;
	}

	template <>
	void SetActiveField<const wchar_t >(const wchar_t *ptr)
	{
		extended_key=ptr;
	}

	int state;
};

ParseState::ParseState()
{
	memset(&record_list, 0, sizeof(record_list));
	active_item=0;
	active_field_string=0;
	active_field_int=0;
	active_field_time=0;
	extended_key=0;

	state = 0;
}

static int indent=0;
static void print_indent()
{
	for (int i=0;i<indent;i++)
		printf(" ");
}
static int on_null(void * ctx)
{
	ParseState *parse_state = (ParseState *)ctx;
	print_indent();
	printf("on null\n");

	parse_state->active_field_string=0;
	parse_state->active_field_int=0;
	parse_state->active_field_time=0;
	parse_state->extended_key=0;
	return 1;    
}

static int on_boolean(void * ctx, int boolean)
{
	ParseState *parse_state = (ParseState *)ctx;
	print_indent();
	printf("on boolean: %d\n", boolean);

	parse_state->active_field_string=0;
	parse_state->active_field_int=0;
	parse_state->active_field_time=0;
	parse_state->extended_key=0;
	return 1;
}

static int on_integer(void * ctx, long long integerVal)
{
	ParseState *parse_state = (ParseState *)ctx;
	print_indent();
	printf("on integer: %llu\n", integerVal);
	return 1;
}

static int on_double(void * ctx, double doubleVal)
{
	ParseState *parse_state = (ParseState *)ctx;
	print_indent();
	printf("on double: %f\n", doubleVal);
	return 1;
}

static int myatoi(const char *p, size_t len) 
{
	char *w = (char *)alloca((len+1)*sizeof(char));
	strncpy(w, p, len);
	w[len] = 0;
	int a = strtol(w,0, 10);
	return a;
}

static __time64_t myatotime(const char *p, size_t len) 
{
	char *w = (char *)alloca((len+1)*sizeof(char));
	strncpy(w, p, len);
	w[len] = 0;
	return _atoi64(w);
}

static int on_number(void * ctx, const char * s, size_t l)
{
	ParseState *parse_state = (ParseState *)ctx;
	if (parse_state->active_field_int)
		*parse_state->active_field_int = myatoi(s, l);
	else if (parse_state->active_field_time)
		*parse_state->active_field_time = myatotime(s, l);
	else if (parse_state->extended_key)
		setRecordExtendedItemUTF8(parse_state->active_item, parse_state->extended_key, (const unsigned char *)s, l);
	else if (parse_state->active_field_string)
		FillUTF8(parse_state->active_field_string, (const unsigned char *)s, l);

	print_indent();
	printf("on number: ");
	fwrite(s, l, 1, stdout);
	printf("\n");

	parse_state->active_field_string=0;
	parse_state->active_field_int=0;
	parse_state->active_field_time=0;
	parse_state->extended_key=0;
	return 1;
}

static int on_string(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	ParseState *parse_state = (ParseState *)ctx;

	if (parse_state->active_field_int)
		*parse_state->active_field_int = myatoi((const char *)stringVal, stringLen);
	else if (parse_state->active_field_time)
		*parse_state->active_field_time = myatotime((const char *)stringVal, stringLen);
	else if (parse_state->extended_key)
		setRecordExtendedItemUTF8(parse_state->active_item, parse_state->extended_key, (const unsigned char *)stringVal, stringLen);
	else if (parse_state->active_field_string)
		FillUTF8(parse_state->active_field_string, (const unsigned char *)stringVal, stringLen);

	print_indent();
	printf("on string: ");
	fwrite(stringVal, stringLen, 1, stdout);
	printf("\n");

	parse_state->active_field_string=0;
	parse_state->active_field_int=0;
	parse_state->active_field_time=0;
	parse_state->extended_key=0;
	return 1;
}

static void InitItemRecord(itemRecordW *p)
{
	p->title=0;
	p->album=0;
	p->artist=0;
	p->comment=0;
	p->genre=0;
	p->albumartist=0; 
	p->replaygain_album_gain=0;
	p->replaygain_track_gain=0;
	p->publisher=0;
	p->composer=0;
	p->year=-1;
	p->track=-1;
	p->tracks=-1;
	p->length=-1;
	p->rating=-1;
	p->playcount=-1;
	p->lastplay=-1;
	p->lastupd=-1;
	p->filetime=-1;
	p->filesize=-1;
	p->bitrate=-1;
	p->type=-1;
	p->disc=-1;
	p->discs=-1;
	p->bpm=-1;
	p->extended_info=0; 
	p->category=0;
}

static int on_start_map(void * ctx)
{
	ParseState *parse_state = (ParseState *)ctx;
	switch(parse_state->state)
	{
	case 0:
		parse_state->state=1;
		break;
	case 1:
		allocRecordList(&parse_state->record_list, parse_state->record_list.Size+1);
		parse_state->active_item = &parse_state->record_list.Items[parse_state->record_list.Size];
		InitItemRecord(parse_state->active_item);
		parse_state->state = 2;
		break;
	default:
		printf("error\n");
		return 0;
	}

	print_indent();
	printf("on start map\n");
	indent++;
	return 1;
}

#define JSON_MATCH_STRING(stringVal, stringLen, string_literal) (stringLen == (sizeof(string_literal)-1) && !memcmp(stringVal, string_literal, sizeof(string_literal)-1))

static int on_map_key(void * ctx, const unsigned char * stringVal,
	size_t stringLen)
{
	ParseState *parse_state = (ParseState *)ctx;
	if (parse_state->state == 1)
	{

	}
	else if (parse_state->state == 2)
	{
		parse_state->active_field_string=0;
		parse_state->active_field_int=0;
		parse_state->active_field_time=0;
		parse_state->extended_key=0;

		if (JSON_MATCH_STRING(stringVal, stringLen, "idhash"))
			parse_state->SetActiveField(L"cloud_id");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "deleted"))
			parse_state->SetActiveField(L"cloud_deleted");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "mediahash"))
			parse_state->SetActiveField(L"cloud_mediahash");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "revision"))
			parse_state->SetActiveField(L"cloud_revision");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "filename"))
			parse_state->SetActiveField(&parse_state->active_item->filename);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "filesize"))
			parse_state->SetActiveField(&parse_state->active_item->filesize);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "type"))
			parse_state->SetActiveField(&parse_state->active_item->type);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "mimetype"))
			parse_state->SetActiveField(L"cloud_mime");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "title"))
			parse_state->SetActiveField(&parse_state->active_item->title);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "artist"))
			parse_state->SetActiveField(&parse_state->active_item->artist);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "album"))
			parse_state->SetActiveField(&parse_state->active_item->album);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "year"))
			parse_state->SetActiveField(&parse_state->active_item->year);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "genre"))
			parse_state->SetActiveField(&parse_state->active_item->genre);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "comment"))
			parse_state->SetActiveField(&parse_state->active_item->comment);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "trackno"))
			parse_state->SetActiveField(&parse_state->active_item->track);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "length"))
			parse_state->SetActiveField(&parse_state->active_item->length);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "rating"))
			parse_state->SetActiveField(&parse_state->active_item->rating);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "bitrate"))
			parse_state->SetActiveField(&parse_state->active_item->bitrate);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "disc"))
			parse_state->SetActiveField(&parse_state->active_item->disc);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "albumartist"))
			parse_state->SetActiveField(&parse_state->active_item->albumartist);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "replaygain_album_gain"))
			parse_state->SetActiveField(&parse_state->active_item->replaygain_album_gain);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "replaygain_track_gain"))
			parse_state->SetActiveField(&parse_state->active_item->replaygain_track_gain);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "publisher"))
			parse_state->SetActiveField(&parse_state->active_item->publisher);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "composer"))
			parse_state->SetActiveField(&parse_state->active_item->composer);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "discs"))
			parse_state->SetActiveField(&parse_state->active_item->discs);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "tracks"))
			parse_state->SetActiveField(&parse_state->active_item->tracks);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "bpm"))
			parse_state->SetActiveField(&parse_state->active_item->bpm);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "ispodcast"))
			parse_state->SetActiveField(L"ispodcast");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "podcastchannel"))
			parse_state->SetActiveField(L"podcastchannel");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "podcastpubdate"))
			parse_state->SetActiveField(L"podcastpubdate");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "GracenoteFileID"))
			parse_state->SetActiveField(L"GracenoteFileID");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "GracenoteExtData"))
			parse_state->SetActiveField(L"GracenoteExtData");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "lossless"))
			parse_state->SetActiveField(L"lossless");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "category"))
			parse_state->SetActiveField(&parse_state->active_item->category);
		else if (JSON_MATCH_STRING(stringVal, stringLen, "codec"))
			parse_state->SetActiveField(L"codec");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "director"))
			parse_state->SetActiveField(L"director");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "producer"))
			parse_state->SetActiveField(L"producer");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "width"))
			parse_state->SetActiveField(L"width");
		else if (JSON_MATCH_STRING(stringVal, stringLen, "height"))
			parse_state->SetActiveField(L"height");		
	}
	else
	{
		printf("error\n");
		return 0;
	}
	print_indent();
	printf("on map key: ");
	fwrite(stringVal, stringLen, 1, stdout);
	printf("\n");
	return 1;
}

static int on_end_map(void * ctx)
{
	ParseState *parse_state = (ParseState *)ctx;
	if (parse_state->state == 1)
	{
		parse_state->state=0;
	}
	else if (parse_state->state == 2)
	{
		parse_state->active_item=0;
		parse_state->record_list.Size++;
		parse_state->state=1;
	}
	indent--;
	print_indent();
	printf("on end map\n");

	return 1;
}

static int on_start_array(void * ctx)
{
	ParseState *parse_state = (ParseState *)ctx;
	print_indent();
	printf("on start array\n");
	indent++;
	return 1;
}

static int on_end_array(void * ctx)
{
	ParseState *parse_state = (ParseState *)ctx;
	indent--;
	print_indent();
	printf("on end array\n");
	return 1;
}

static yajl_callbacks callbacks = {
	on_null,
	on_boolean,
	on_integer,
	on_double,
	on_number,
	on_string,on_start_map,
	on_map_key,
	on_end_map,
	on_start_array,
	on_end_array
};

ItemParser *Parse()
{
	ParseState *parse_state = new ParseState;
	if (!parse_state)
		return 0;

	parse_state->parser = yajl_alloc(&callbacks, NULL, parse_state);

	return parse_state;
}