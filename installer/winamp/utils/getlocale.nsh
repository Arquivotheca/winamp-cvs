;; Returns 1 if Os is in USA

Function IsUSLocated

   Push $1
   Push $0
   
   StrCpy $1 "0"

   StrCmp $WinVer "XP"  useGeo
   StrCmp $WinVer "ME"  useGeo

   System::Call "Kernel32::GetUserDefaultLCID(v)i .r0"
   StrCmp $0 "1033" 0 +2
     StrCpy $1 "1"
     goto f_end
useGeo:
   System::Call "Kernel32::GetUserGeoID(i)i(16)i.r0"
   StrCmp $0 "244" 0 +2
     StrCpy $1 "1"
     goto f_end
f_end:
  Pop $0
  Exch $1
FunctionEnd