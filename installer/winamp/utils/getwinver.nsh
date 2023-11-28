; GetWindowsVersion
 ;
 ; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
 ; Updated by Joost Verburg
 ;
 ; Returns on top of stack
 ;
 ; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003)
 ; or
 ; '' (Unknown Windows Version)
 ;
 ; Usage:
 ;   Call GetWindowsVersion
 ;   Pop $R0
 ;   Pop $R1
 ;   ; at this point $R0 is "NT 4.0" or whatnot
 ;   if $R0 95, 98 - $R1 will contain Release specs

!macro GetWindowsVersionInternal

   Push $R0
   Push $R1

   ClearErrors

   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

   IfErrors 0 lbl_winnt

   ; we are not NT
   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber

   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error

   StrCpy $R1 $R0 2 2


   StrCmp $R1 '00' lbl_win32_95
   StrCmp $R1 '10' lbl_win32_98
   StrCmp $R1 '90' lbl_win32_ME 
   StrCpy $R0 ''
   StrCpy $R1 ''
   Goto lbl_done
   
   
   lbl_win32_95:
     StrCpy $R1 $R0 4 5
     StrCpy $R0 '95'
     StrCmp $R1 '950' 0 +3
       StrCpy $R1 'Original'
       goto lbl_done
     StrCmp $R1 '950a' 0 +3
       StrCpy $R1 'SP1'
       goto lbl_done
     StrCmp $R1 '1111' 0 +3
       StrCpy $R1 'SR2'
       goto lbl_done
     StrCpy $R1 ''
     goto lbl_done
   lbl_win32_98:
     StrCpy $R1 $R0 4 5
     
     StrCpy $R0 '98'
     StrCmp $R1 '1980' 0 +3
       StrCpy $R1 'Original'
       goto lbl_done
     StrCmp $R1 '2222' 0 +3
       StrCpy $R1 'SE'
       goto lbl_done
     StrCpy $R1 ''
     goto lbl_done
   lbl_win32_ME:
     StrCpy $R0 'ME'
     StrCpy $R1 'Original'
   Goto lbl_done

   lbl_winnt:

   StrCpy $R1 $R0 1

   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x

   StrCpy $R1 $R0 3

   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003
   StrCmp $R1 '6.0' lbl_winnt_vista 
   StrCmp $R1 '6.1' lbl_winnt_7 ; lbl_error - TODO special one for win7 (not!)
   StrCmp $R1 '6.2' lbl_winnt_8

   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
     Strcpy $R1 ''
   Goto lbl_done

   lbl_winnt_2000:
     Strcpy $R0 '2000'
     Strcpy $R1 ''
   Goto lbl_done

   lbl_winnt_XP:
     Strcpy $R0 'XP'
     Strcpy $R1 ''
   Goto lbl_done

   lbl_winnt_2003:
     Strcpy $R0 '2003'
     Strcpy $R1 ''
    Goto lbl_done

   lbl_winnt_vista:
    Strcpy $R0 'VISTA'
    Strcpy $R1 ''
   Goto lbl_done
   
   lbl_winnt_7:
    Strcpy $R0 '7'
    Strcpy $R1 ''
   Goto lbl_done

   lbl_winnt_8:
    Strcpy $R0 '8'
    Strcpy $R1 ''
   Goto lbl_done 

   lbl_error:
     Strcpy $R0 ''
     Strcpy $R1 ''
   lbl_done:

   Exch $R1
   Exch 1
   Exch $R0
!macroend
 
!macro GetWindowsVersion
	${CallArtificialFunction} GetWindowsVersionInternal
!macroend
!define GetWindowsVersion "!insertmacro 'GetWindowsVersion'"

 Function GetWinampFolder

   Push $R1
   Push $R2
   Push $R3

   StrCpy $R0 $INSTDIR
   StrCpy $R1 0
   StrLen $R2 $R0

   loop:
     IntOp $R1 $R1 + 1
     IntCmp $R1 $R2 get 0 get
     StrCpy $R3 $R0 1 -$R1
     StrCmp $R3 "\" get
     Goto loop

   get:
     IntCmp $R1 0 +2
     IntOp $R1 $R1 - 1
     StrCpy $R0 $R0 $R1 -$R1

     Pop $R3
     Pop $R2
     Exch $R0

 FunctionEnd
