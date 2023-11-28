//
//  precomp.h
//  gracenote
//


#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus 
#include <new>
#endif

#include "foundation/error.h"
#include "foundation/types.h"
#ifdef __cplusplus 
#include "foundation/dispatch.h"
#endif

#ifdef __cplusplus
#include "nu/ByteWriter.h"
#include "nu/ByteReader.h"
#include "nu/PtrList.h"
#endif

#include "nx/nxuri.h"
#include "nx/nxonce.h"
#include "nx/nxthread.h"

#ifdef __cplusplus
#include "nswasabi/ReferenceCounted.h"
#include "api.h"
#include "metadata/metadata.h"
#endif

#include "gnsdk.h"