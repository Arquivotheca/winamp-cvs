//-----------------------------------------------------------------------------
// init.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Initialization Function
//
//-----------------------------------------------------------------------------
// If you are a licensed user of PrimoSDK you are authorized to
// copy part or entirely the code of this example into your
// application.

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <tchar.h>

#include "primosdk.h"
#include "pxsample_c.h"

DWORD InitializeSDK(PDWORD pdwRel, LPTSTR szTitle)
{
	return PrimoSDK_Init(pdwRel);
}

VOID UninitializeSDK(VOID)
{
}

