#pragma once
#include "foundation/types.h"
#include "JSON-Types.h"
#include "JSON-Value.h"
#include "JSON-Builder.h"
#include "nx/nxstring.h"
#include "nu/vector.h"
#include "nu/PtrList.h"
#include "nu/PtrDeque.h"

namespace JSON
{

	class Tree : public Container
	{
	public:
		Tree();
		~Tree();
		int Add(Value *value, JSON::Tree *);
		JSON::Value *root;

		nu::PtrDeque<JSON::Container> container_stack;

		int AddValue(JSON::Value *value);
	};
}