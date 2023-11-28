#include "nxsleep.h"
#include "foundation/error.h"
#include <unistd.h>

int NXSleep(unsigned int milliseconds)
{
	usleep(1000*milliseconds);
	return NErr_Success;
}

int NXSleepYield(void)
{
	sched_yield();
	return NErr_Success;
}
