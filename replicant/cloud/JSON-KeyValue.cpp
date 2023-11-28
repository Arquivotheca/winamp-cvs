#include "JSON-KeyValue.h"
#include "foundation/error.h"
#include "../libyajl/include/yajl/yajl_parse.h"
#include <new>


JSON::Tree::Tree()
{
	root=0;
	container_stack.push_front(this);
}


JSON::Tree::~Tree()
{
	delete root;
}

int JSON::Tree::Add(Value *value, JSON::Tree *)
{
	root = value;
	return NErr_Success;
}

int JSON::Tree::AddValue(JSON::Value *value)
{
	if (!value)
		return NErr_OutOfMemory;

	int ret = container_stack.front()->Add(value, this);
	if (ret != NErr_Success)
	{
		delete value;
		return ret;
	}

	if (value->data_type == JSON::DATA_ARRAY)
		container_stack.push_front((JSON::Value_Array_Builder *)value);
	else if (value->data_type == JSON::DATA_MAP)
		container_stack.push_front((JSON::Value_Map_Builder *)value);
	else if (value->data_type == JSON::DATA_KEY)
		container_stack.push_front((JSON::Value_Key_Builder *)value);

	return NErr_Success;
}

static int on_null(void * ctx)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Null_Builder());
	if (ret != NErr_Success)
		return 0;

	return 1;
}

static int on_boolean(void * ctx, int boolean)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Boolean_Builder(!!boolean));
	if (ret != NErr_Success)
		return 0;
	
	return 1;
}

static int on_integer(void * ctx, long long integerVal)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Integer_Builder(integerVal));
	if (ret != NErr_Success)
		return 0;
	return 1;
}


static int on_double(void * ctx, double doubleVal)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Double_Builder(doubleVal));
	if (ret != NErr_Success)
		return 0;
	return 1;
}

static int on_string(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	nx_string_t str = 0;
	int ret = NXStringCreateWithBytes(&str, stringVal, stringLen, nx_charset_utf8);
	if (ret != NErr_Success)
		return 0;

	ret = parse_state->AddValue(new (std::nothrow) JSON::Value_String_Builder(str));
	if (ret != NErr_Success)
	{
		NXStringRelease(str);
		return 0;
	}

	return 1;
}


static int on_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	nx_string_t str = 0;
	int ret = NXStringCreateWithBytes(&str, stringVal, stringLen, nx_charset_utf8);
	if (ret != NErr_Success)
		return 0;

	ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Key_Builder(str));
	if (ret != NErr_Success)
	{
		NXStringRelease(str);
		return 0;
	}
	
	return 1;
}


static int on_start_map(void * ctx)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Map_Builder);
	if (ret != NErr_Success)
		return 0;

	return 1;
}


static int on_end_map(void * ctx)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	parse_state->container_stack.pop_front();
	return 1;
}

static int on_start_array(void * ctx)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	int ret = parse_state->AddValue(new (std::nothrow) JSON::Value_Array_Builder);
	if (ret != NErr_Success)
		return 0;

	return 1;
}


static int on_end_array(void * ctx)
{
	JSON::Tree *parse_state = (JSON::Tree *)ctx;

	parse_state->container_stack.pop_front();	
	return 1;
}

yajl_callbacks nsjson_tree_callbacks = {
	on_null,
	on_boolean,
	on_integer,
	on_double,
	0,//on_number,
	on_string,
	on_start_map,
	on_map_key,
	on_end_map,
	on_start_array,
	on_end_array
};
