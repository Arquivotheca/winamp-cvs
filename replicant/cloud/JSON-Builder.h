#pragma once
#include "JSON-Types.h"
#include "JSON-Value.h"
#include "JSON-Container.h"

namespace JSON
{
	class Value_Null_Builder : public Value_Null
	{
	public:
		Value_Null_Builder();
	};

	class Value_String_Builder : public Value_String
	{
	public:
		Value_String_Builder(nx_string_t value);
	};

	class Value_Integer_Builder : public Value_Integer
	{
	public:
		Value_Integer_Builder(int64_t value);
	};

	class Value_Boolean_Builder : public Value_Boolean
	{
	public:
		Value_Boolean_Builder(bool value);
	};

	class Value_Double_Builder : public Value_Double
	{
	public:
		Value_Double_Builder(double value);
	};

	class Value_Array_Builder : public Value_Array, public Container
	{
	public:
		Value_Array_Builder();
		int Add(Value *value, JSON::Tree *);
	};

	class Value_Map_Builder : public Value_Map, public Container
	{
	public:
		Value_Map_Builder();
		int Add(Value *key, JSON::Tree *);
	};

	class Value_Key_Builder : public Value_Key, public Container
	{
	public:
		Value_Key_Builder(nx_string_t key);
		int Add(Value *key, JSON::Tree *parent);
	};
};