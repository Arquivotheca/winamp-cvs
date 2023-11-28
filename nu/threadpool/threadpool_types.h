#pragma once
#include "../ValueDeque.h"
#include <windows.h>
namespace ThreadPoolTypes
{
	typedef nu::ValueDeque<HANDLE> HandleList;
	const int MAX_SEMAPHORE_VALUE = 1024; //some arbitrarily high amount*
}