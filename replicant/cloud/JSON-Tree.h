#pragma once

#include "JSON-Value.h"
#include "../libyajl/include/yajl/yajl_parse.h"

typedef struct nsjson_tree_s *nsjson_tree_t;
int JSON_Tree_CreateParser(nsjson_tree_t *out_tree);
int JSON_Tree_GetHandle(nsjson_tree_t tree, yajl_handle *handle);
int JSON_Tree_Finish(nsjson_tree_t tree, const JSON::Value **value);
void JSON_Tree_Destroy(nsjson_tree_t tree);