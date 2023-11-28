//------------------------------------------------------------------------------
// File: AMVideo.cpp
//
// Desc: DirectShow base classes - implements helper functions for
//       bitmap formats.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <streams.h>
#include <limits.h>

// These are bit field masks for true colour devices

const DWORD bits555[] = {0x007C00,0x0003E0,0x00001F};
const DWORD bits565[] = {0x00F800,0x0007E0,0x00001F};
const DWORD bits888[] = {0xFF0000,0x00FF00,0x0000FF};

// This maps bitmap subtypes into a bits per pixel value and also a
// name. unicode and ansi versions are stored because we have to
// return a pointer to a static string.
const struct {
    const GUID *pSubtype;
    WORD BitCount;
    CHAR *pName;
    WCHAR *wszName;
} BitCountMap[] =  { &MEDIASUBTYPE_RGB1,        1,   "RGB Monochrome",     L"RGB Monochrome",   
                     &MEDIASUBTYPE_RGB4,        4,   "RGB VGA",            L"RGB VGA",          
                     &MEDIASUBTYPE_RGB8,        8,   "RGB 8",              L"RGB 8",            
                     &MEDIASUBTYPE_RGB565,      16,  "RGB 565 (16 bit)",   L"RGB 565 (16 bit)", 
                     &MEDIASUBTYPE_RGB555,      16,  "RGB 555 (16 bit)",   L"RGB 555 (16 bit)", 
                     &MEDIASUBTYPE_RGB24,       24,  "RGB 24",             L"RGB 24",           
                     &MEDIASUBTYPE_RGB32,       32,  "RGB 32",             L"RGB 32",
                     &MEDIASUBTYPE_ARGB32,    32,  "ARGB 32",             L"ARGB 32",
                     &MEDIASUBTYPE_Overlay,     0,   "Overlay",            L"Overlay",          
                     &GUID_NULL,                0,   "UNKNOWN",            L"UNKNOWN"           
};


// Given a bitmap subtype we return a description name that can be used for
// debug purposes. In a retail build this function still returns the names
// If the subtype isn't found in the lookup table we return string UNKNOWN

int LocateSubtype(const GUID *pSubtype)
{
    ASSERT(pSubtype);
    const GUID *pMediaSubtype;
    INT iPosition = 0;

    // Scan the mapping list seeing if the source GUID matches any known
    // bitmap subtypes, the list is terminated by a GUID_NULL entry

    while (TRUE) {
        pMediaSubtype = BitCountMap[iPosition].pSubtype;
        if (IsEqualGUID(*pMediaSubtype,*pSubtype) ||
            IsEqualGUID(*pMediaSubtype,GUID_NULL)
            )
        {
            break;
        }
        
        iPosition++;
    }

    return iPosition;
}

