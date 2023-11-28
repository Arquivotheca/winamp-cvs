#include "JSON-Tree.h"
#include "JSON-KeyValue.h"
#include "foundation/error.h"
#include <new>

extern yajl_callbacks nsjson_tree_callbacks;
typedef struct nsjson_tree_s 
{
	nsjson_tree_s();
	~nsjson_tree_s();
	JSON::Tree tree;
	yajl_handle hand;
} *nsjson_tree_t;

nsjson_tree_s::nsjson_tree_s()
{
	hand=0;
}

nsjson_tree_s::~nsjson_tree_s()
{
	if (hand)
		yajl_free(hand);
}

int JSON_Tree_CreateParser(nsjson_tree_t *out_tree)
{
	nsjson_tree_t tree = new (std::nothrow) nsjson_tree_s;
	if (!tree)
		return NErr_OutOfMemory;

	yajl_handle hand = yajl_alloc(&nsjson_tree_callbacks, 0, &tree->tree);
	if (!hand)
	{
		delete tree;
		return NErr_FailedCreate;
	}

	tree->hand = hand;
	*out_tree = tree;
	return NErr_Success;
}

int JSON_Tree_GetHandle(nsjson_tree_t tree, yajl_handle *handle)
{
	if (!tree)
		return NErr_BadParameter;

	*handle = tree->hand;
	return NErr_Success;
}

int JSON_Tree_Finish(nsjson_tree_t tree, const JSON::Value **value)
{
	if (!tree)
		return NErr_BadParameter;

	/* TODO: 
	if (tree->tree.parse_error != NErr_Success)
		return tree->tree.parse_error;
		*/

	if (!tree->tree.root)
		return NErr_Empty;

	*value = tree->tree.root;
	return NErr_Success;
}

void JSON_Tree_Destroy(nsjson_tree_t tree)
{
	delete tree;
}