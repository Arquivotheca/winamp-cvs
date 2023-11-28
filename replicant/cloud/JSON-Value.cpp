#include "JSON-KeyValue.h"
#include "JSON-Value.h"
#include "foundation/error.h"
#include "nswasabi/AutoCharNX.h"
#include <stdio.h>

JSON::Value::Value(DataType data_type) : data_type(data_type)
{
}

/* --------------------- */ 
JSON::Value_Null::Value_Null() : Value(DATA_NULL)
{
}

void JSON::Value_Null::Dump(nx_file_t out, int indent) const
{
	NXFileWrite(out, "null", 4);
}

/* --------------------- */ 
JSON::Value_Map::Value_Map() : Value(DATA_MAP)
{
}

int JSON::Value_Map::FindNextKey(size_t start, const char *key, const JSON::Value **found, size_t *out_iterator) const
{
	for (size_t i=start;i<map_data.size();i++)
	{
		nx_string_t str;
		int ret = map_data[i]->GetKey_NoRetain(&str);
		if (ret == NErr_Success)
		{
			if (NXStringKeywordCompareWithCString(str, key) == 0)
			{
				ret = map_data[i]->GetValue(found);
				//if (ret != NErr_Success)
				{
					if (out_iterator)
						*out_iterator = i+1;
				}
				return ret;
			}
		}
	}
	return NErr_EndOfEnumeration;
}

int JSON::Value_Map::EnumerateValues(size_t index, const Value **found) const
{
	if (map_data.size() <= index)
		return NErr_EndOfEnumeration;

	*found = map_data.at(index);
	return NErr_Success;
}

void JSON::Value_Map::Dump(nx_file_t out, int indent) const
{
	NXFileWrite(out, "{\n", 2);
	for (size_t i=0;i<map_data.size();i++)
	{
		assert(map_data[i]);
		((Value *)(map_data[i]))->Dump(out, indent+1);
		if ((i+1) != map_data.size())
			NXFileWrite(out, ",\n", 2);
		else
			NXFileWrite(out, "\n", 1);
	}
	char temp[16];
	int len = sprintf(temp, "%*s}", indent, "");
	NXFileWrite(out, temp, len);
}

/* --------------------- */ 
// valid if this is an Array
int JSON::Value_Array::EnumerateValues(size_t index, const Value **found) const
{
	if (array_data.size() <= index)
		return NErr_EndOfEnumeration;

	*found = array_data.at(index);
	return NErr_Success;
}

void JSON::Value_Array::Dump(nx_file_t out, int indent) const
{
	NXFileWrite(out, "[\n", 2);
	for (size_t i=0;i<array_data.size();i++)
	{
		assert(array_data[i]);
		char temp[16];
		int len = sprintf(temp, "%*s", indent+1, "");
		NXFileWrite(out, temp, len);
		array_data[i]->Dump(out, indent+1);
		if ((i+1) != array_data.size())
			NXFileWrite(out, ",\n", 2);
		else
			NXFileWrite(out, "\n", 1);
	}
	char temp[16];
	int len = sprintf(temp, "%*s]", indent, "");
	NXFileWrite(out, temp, len);
}

/* --------------------- */ 
JSON::Value_String::Value_String() : Value(DATA_STRING)
{
	string_data=0;
}

JSON::Value_String::~Value_String()
{
	NXStringRelease(string_data);
}

int JSON::Value_String::GetString(nx_string_t *found) const
{
	if (string_data == 0)
		return NErr_Empty;

	*found = NXStringRetain(string_data);
	return NErr_Success;	
}

void JSON::Value_String::Dump(nx_file_t out, int indent) const
{
	assert(string_data);
	NXFileWrite(out, "\"", 1);
	AutoCharUTF8 temp(string_data);
	NXFileWrite(out, temp.GetValidString(), temp.size());
	NXFileWrite(out, "\"", 1);
}

/* --------------------- */ 
JSON::Value_Integer::Value_Integer() : Value(DATA_INTEGER)
{
	integer_data=0;
}

int JSON::Value_Integer::GetInteger(int64_t *found) const
{
	*found = integer_data;
	return NErr_Success;		
}

int JSON::Value_Integer::GetString(nx_string_t *found) const
{
	return NXStringCreateWithInt64(found, integer_data);
}

void JSON::Value_Integer::Dump(nx_file_t out, int indent) const
{
	char temp[64];
	int len = sprintf(temp, "%lld", integer_data);
	NXFileWrite(out, temp, len);
}

/* --------------------- */ 
JSON::Value_Double::Value_Double() : Value(DATA_DOUBLE)
{
	double_data=0;
}

int JSON::Value_Double::GetDouble(double *found) const
{
	*found = double_data;
	return NErr_Success;
}

void JSON::Value_Double::Dump(nx_file_t out, int indent) const
{
	char temp[64];
	int len = sprintf(temp, "%g", double_data);
	NXFileWrite(out, temp, len);
}

/* --------------------- */ 
JSON::Value_Boolean::Value_Boolean() : Value(DATA_BOOLEAN)
{
	boolean_data=false;
}

void JSON::Value_Boolean::Dump(nx_file_t out, int indent) const
{
	char temp[8];
	int len = sprintf(temp, "%s", boolean_data ? "true" : "false");
	NXFileWrite(out, temp, len);
}

/* --------------------- */ 
JSON::Value_Array::Value_Array() : Value(DATA_ARRAY)
{
}

/* --------------------- */ 
JSON::Value_Key::Value_Key(nx_string_t key) : Value(DATA_KEY), key(key)
{
	value=0;
}

JSON::Value_Key::~Value_Key()
{
	NXStringRelease(key);
	delete value;
}

int JSON::Value_Key::GetValue(const Value **found) const
{
	if (!value)
		return NErr_Empty;

	*found = value;
	return NErr_Success;
}

int JSON::Value_Key::GetKey_NoRetain(nx_string_t *out_key) const
{
	if (!key)
		return NErr_Empty;

	*out_key = key;
	return NErr_Success;
}

void JSON::Value_Key::Dump(nx_file_t out, int indent) const
{
#ifdef DEBUG
	assert(key);
	char temp[1024] = {0};
	int len = sprintf(temp, "%*s\"%s\" : ", indent, "", AutoCharPrintfUTF8(key));
	NXFileWrite(out, temp, len);
	assert(value);
	value->Dump(out, indent+1);
#endif
}
