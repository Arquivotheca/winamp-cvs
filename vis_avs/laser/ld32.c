#ifdef LASER
#include <windows.h>

#include "ld32.h"

static HINSTANCE hDll;

LONG (WINAPI *_LoadPalette)(LPSTR ColorFilename, LPLONG RETURN_LDStatus);
LONG (WINAPI *_WritePoint)(LONG PointNumber, PTSTRUCT *SUPPLY_PtStruct);
LONG (WINAPI *_InitialQMCheck)(LPLONG RETURN_LDStatus);
LONG (WINAPI *_DisplayUpdate)(void);
LONG (WINAPI *_DisplayBufferStatus)(LONG *RETURN_BufferIsFree, LONG *RETURN_CurrentOutputPoints);
LONG (WINAPI *_DisplayFrame)(LONG Frame);
LONG (WINAPI *_WriteFrameEx)(FRAMESTRUCTEX *SUPPLY_FrameStruct, PTSTRUCT *SUPPLY_PointArray);
LONG (WINAPI *_SetWorkingScanners)(LONG Scanner);
LONG (WINAPI *_SetWorkingTracks)(LONG Track);
LONG (WINAPI *_SetWorkingFrame)(LONG FrameNumber);
LONG (WINAPI *_BeginSessionEx)(LPLONG RETURN_Version, LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus);
LONG (WINAPI *_DisplayFlags)(LONG Flags);
LONG (WINAPI *_EndSession)(void);
LONG (WINAPI *_GetLDDLLModuleUsage)(LPLONG ModuleUsage);
LONG (WINAPI *_ResetLD)(void);			//Updates all track variables 
LONG (WINAPI *_ReadProjectionZone)(LONG ZoneNumber, PROJECTIONZONE *RETURN_PZ);
LONG (WINAPI *_OpenLDCommWindow)(void);
LONG (WINAPI *_DisplayProjectionZones)(long ProjectionZoneCode);

LONG WINAPI InitialQMCheck(LPLONG RETURN_LDStatus)
{
  hDll=LoadLibrary("ld2000.dll");
  if (!hDll)
  {
//    printf("LD2000: error loading DLL\n");
    *RETURN_LDStatus=1;
    return 1;
  }
#define RETR(x) *((void**)&_##x)=(void*)GetProcAddress(hDll,#x); if (!_##x) { FreeLibrary(hDll); hDll=0; *RETURN_LDStatus=1; return 1; }
  
 // \
//             if (!_##x) printf("LD2000: error loading DLL: " #x "\n");

  RETR(ReadProjectionZone);
  RETR(InitialQMCheck);
  RETR(DisplayUpdate);
  RETR(DisplayBufferStatus);
  RETR(DisplayFrame);
  RETR(WriteFrameEx);
  RETR(WritePoint);
  RETR(SetWorkingScanners);
  RETR(LoadPalette);
  RETR(SetWorkingTracks);
  RETR(SetWorkingFrame);
  RETR(EndSession);
  RETR(BeginSessionEx);
  RETR(DisplayFlags);
  RETR(ResetLD);
  RETR(OpenLDCommWindow);
  RETR(GetLDDLLModuleUsage);
  RETR(DisplayProjectionZones);
  return _InitialQMCheck(RETURN_LDStatus);
}

LONG WINAPI DisplayProjectionZones(long ProjectionZoneCode)
{
  return _DisplayProjectionZones(ProjectionZoneCode);
}



LONG WINAPI ReadProjectionZone(LONG ZoneNumber, PROJECTIONZONE *RETURN_PZ)
{
  if (!hDll||!_ReadProjectionZone) return 1;
  return _ReadProjectionZone(ZoneNumber,RETURN_PZ);
}

LONG WINAPI DisplayBufferStatus(LONG *RETURN_BufferIsFree, LONG *RETURN_CurrentOutputPoints)
{
  return _DisplayBufferStatus(RETURN_BufferIsFree,RETURN_CurrentOutputPoints);
}

LONG WINAPI DisplayUpdate(void)
{
  return _DisplayUpdate();
}
LONG WINAPI DisplayFrame(LONG Frame)
{
  return _DisplayFrame(Frame);
}

LONG WINAPI WriteFrameEx(FRAMESTRUCTEX *SUPPLY_FrameStruct, PTSTRUCT *SUPPLY_PointArray)
{
  return _WriteFrameEx(SUPPLY_FrameStruct,SUPPLY_PointArray);
}

LONG WINAPI SetWorkingScanners(LONG Scanner)
{
  return _SetWorkingScanners(Scanner);
}

LONG WINAPI SetWorkingTracks(LONG Track)
{
  return _SetWorkingTracks(Track);
}

LONG WINAPI SetWorkingFrame(LONG FrameNumber)
{
  return _SetWorkingFrame(FrameNumber);
}

LONG WINAPI OpenLDCommWindow(void)
{
  return _OpenLDCommWindow();
}

LONG WINAPI EndSession(void)
{
  return _EndSession();
}

LONG WINAPI LoadPalette(LPSTR ColorFilename, LPLONG RETURN_LDStatus)
{
  return _LoadPalette(ColorFilename,RETURN_LDStatus);
}

LONG WINAPI WritePoint(LONG PointNumber, PTSTRUCT *SUPPLY_PtStruct)
{
  return _WritePoint(PointNumber,SUPPLY_PtStruct);
}

LONG WINAPI GetLDDLLModuleUsage(LPLONG ModuleUsage)
{
  return _GetLDDLLModuleUsage(ModuleUsage);
}

LONG WINAPI ResetLD()
{
  return _ResetLD();
}

LONG WINAPI BeginSessionEx(LPLONG RETURN_Version, LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus)
{
  return _BeginSessionEx(
    RETURN_Version, RETURN_MaxFrames, 
    RETURN_MaxPoints, RETURN_MaxBuffer, 
    RETURN_UndoFrames,RETURN_LDStatus);
}
LONG WINAPI DisplayFlags(LONG Flags)
{
  return _DisplayFlags(Flags);
}

void LaserQuit()
{
  if (hDll) FreeLibrary(hDll);
  hDll=0;
  _InitialQMCheck=0;
}

#endif