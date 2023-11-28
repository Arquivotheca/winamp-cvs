//
//  precomp.h
//  mp3
//

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
#include "new"
#endif

#include "foundation/error.h"
#include "foundation/types.h"

#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "nu/strsafe.h"

#include "nx/nx.h"

#ifdef __cplusplus 
#include "nu/PtrDeque.h"
#include "jnetlib/jnetlib.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/ServiceName.h"
#include "decode/svc_decode.h"
#include "http/svc_http_demuxer.h"
#include "replaygain/ifc_replaygain_settings.h"
#include "service/ifc_servicefactory.h"
#endif

