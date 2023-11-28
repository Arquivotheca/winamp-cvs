#pragma once
#include "JSON-Types.h"
#include "nu/PtrDeque.h"
namespace JSON
{
	class Tree;	
	class Container : public nu::PtrDequeNode
	{
	public:
		virtual int Add(Value *value, JSON::Tree *parse_state)=0;
	};
}