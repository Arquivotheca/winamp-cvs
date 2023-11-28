#pragma once
#include "JSON-Types.h"
#include "foundation/types.h"
#include "foundation/error.h"
#include "nx/nxstring.h"
#include "nx/nxfile.h"

namespace JSON
{
	class KeyValue;

	class Value
	{
	public:
		const DataType data_type;
		virtual ~Value() {}
		virtual void Dump(nx_file_t out, int indent) const=0;
		virtual int EnumerateValues(size_t index, const Value **found) const { return NErr_WrongFormat; }
		virtual int FindNextKey(size_t start, const char *key, const JSON::Value **found, size_t *out_iterator=0) const { return NErr_WrongFormat; }
		virtual int GetInteger(int64_t *found) const { return NErr_WrongFormat; }
		virtual int GetString(nx_string_t *found) const { return NErr_WrongFormat; }
		virtual int GetDouble(double *found) const { return NErr_WrongFormat; }
	protected:
		Value(DataType data_type);
	};

	class Value_Null : public Value
	{
	protected:
		Value_Null();
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_String : public Value
	{
	public:
		~Value_String();
		int GetString(nx_string_t *found) const;
	protected:
		Value_String();

		nx_string_t string_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Integer : public Value
	{
	public:
		int GetInteger(int64_t *found) const;
		int GetString(nx_string_t *found) const;
	protected:
		Value_Integer();
		int64_t integer_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Double : public Value
	{
	protected:
		int GetDouble(double *found) const;
	protected:
		Value_Double();
		double double_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Boolean : public Value
	{
	protected:
		Value_Boolean();
		bool boolean_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Array : public Value
	{
	public:
		int EnumerateValues(size_t index, const Value **found) const;
	protected:
		Value_Array();
		Array array_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Map : public Value
	{
	public:
		int FindNextKey(size_t start, const char *key, const JSON::Value **found, size_t *out_iterator=0) const;
		int EnumerateValues(size_t index, const Value **found) const;
	protected:
		Value_Map();
		JSON::Map map_data;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

	class Value_Key : public Value
	{
	public:
		~Value_Key();
		int GetValue(const Value **found) const;
		int GetKey_NoRetain(nx_string_t *out_key) const;
	protected:
		Value_Key(nx_string_t key);
		nx_string_t key;
		Value *value;
	private:
		void Dump(nx_file_t out, int indent) const;
	};

}
