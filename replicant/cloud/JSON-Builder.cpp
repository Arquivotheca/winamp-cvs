#include "JSON-Builder.h"
#include "foundation/error.h"
#include "JSON-KeyValue.h"

JSON::Value_Null_Builder::Value_Null_Builder()
{
}

JSON::Value_String_Builder::Value_String_Builder(nx_string_t str)
{
	string_data = NXStringRetain(str);
}

JSON::Value_Integer_Builder::Value_Integer_Builder(int64_t value)
{
	integer_data = value;
}

JSON::Value_Boolean_Builder::Value_Boolean_Builder(bool value)
{
	boolean_data = value;
}

JSON::Value_Double_Builder::Value_Double_Builder(double value)
{
	double_data= value;
}

JSON::Value_Array_Builder::Value_Array_Builder()
{
}

int JSON::Value_Array_Builder::Add(Value *value, JSON::Tree *)
{
	array_data.push_back(value);
	return NErr_Success;
}

JSON::Value_Map_Builder::Value_Map_Builder()
{
}

int JSON::Value_Map_Builder::Add(JSON::Value *key, JSON::Tree *)
{
	map_data.push_back((JSON::Value_Key *)key);
	return NErr_Success;
}

JSON::Value_Key_Builder::Value_Key_Builder(nx_string_t key) : Value_Key(key)
{
}

int JSON::Value_Key_Builder::Add(Value *value, JSON::Tree *parse_state)
{
	this->value = value;
	// take ourselves off the stack 
	parse_state->container_stack.pop_front();

	
	return NErr_Success;
}