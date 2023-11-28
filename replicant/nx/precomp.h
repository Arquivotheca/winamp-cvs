//
//  precomp.h
//  nx
//

#include "foundation/atomics.h"
#include "foundation/error.h"
#include "foundation/guid.h"
#include "foundation/types.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#ifdef WIN32
#include <windows.h>
#else

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/resource.h>

#ifdef __APPLE__ 
#include <CoreFoundation/CoreFoundation.h>
#endif // __APPLE__	

#endif //WIN32

