#ifndef NULLSOFT_FLVMETADATA_H
#define NULLSOFT_FLVMETADATA_H

#include "AMFObject.h"
#include "../nu/PtrList.h"

class FLVMetadata
{
public:
	FLVMetadata();
	~FLVMetadata();
	bool Read(uint8_t *data, size_t size);
	struct Tag
	{
		Tag(); 
		~Tag();
		
		AMFString name;
		AMFMixedArray *parameters; // needs to be pointer so we can refcount
	};
	nu::PtrList<Tag> tags;
	
};
#endif