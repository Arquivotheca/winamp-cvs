#pragma once
/* ifc_gracenote_results is the same thing as ifc_metadata.  
we make a derived class just for type-safety
e.g. to prevent someone from passing an unrelated ifc_metadata object to Save() */

#include "metadata/ifc_metadata.h"

class ifc_gracenote_results : public ifc_metadata { };