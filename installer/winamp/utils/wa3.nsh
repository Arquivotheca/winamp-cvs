Function GetWinamp3InstPath

  Push $0
  Push $1
  Push $2
  ReadRegStr $0 HKLM \
     "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp3" \
     "UninstallString"
  StrCmp $0 "" fin

    StrCpy $1 $0 1 0 ; get firstchar
    StrCmp $1 '"' "" getparent
      ; if first char is ", let's remove "'s first.
      StrCpy $0 $0 "" 1
      StrCpy $1 0
      rqloop:
        StrCpy $2 $0 1 $1
        StrCmp $2 '"' rqdone
        StrCmp $2 "" rqdone
        IntOp $1 $1 + 1
        Goto rqloop
      rqdone:
      StrCpy $0 $0 $1
    getparent:
    ; the uninstall string goes to an EXE, let's get the directory.
    StrCpy $1 -1
    gploop:
      StrCpy $2 $0 1 $1
      StrCmp $2 "" gpexit
      StrCmp $2 "\" gpexit
      IntOp $1 $1 - 1
      Goto gploop
    gpexit:
    StrCpy $0 $0 $1

    StrCmp $0 "" fin
    IfFileExists $0\studio.exe fin
    IfFileExists $0\winamp3.exe fin
      StrCpy $0 ""
  fin:
  Pop $2
  Pop $1
  Exch $0

FunctionEnd

Function winamp3upgrade
  Push $0
  Push $1
  Push $2

  Call GetWinamp3InstPath
  Pop $0
  StrLen $1 $0
  IntCmp $1 4 noupgrade noupgrade ; make sure our wa3 path > 4 chars long

    DetailPrint $(msgWA3)

    MessageBox MB_YESNO|MB_ICONQUESTION $(msgWA3_UPGRADE) IDNO noupgrade

  StrCpy $2 ""

  StrCmp $0 $INSTDIR dirupgrade   ; if we're installing to the winamp3 dir, dont prompt for upgrade

  StrCmp $0\ $INSTDIR dirupgrade
  StrCmp $0 $INSTDIR\ dirupgrade

  SetDetailsPrint none
  DetailPrint $(msgWA3_MIGRATE)

  Delete $0\skins\default.wal
  Delete $0\skins\classic.wsz

  Push $R0
  Call GetSkinDir
  Pop $R0
  CreateDirectory $R0

    ; copy $0\skins\*.wal to $R0\*.wal
    Push $R1
      FindFirst $R1 $1 $0\skins\*.wal
      StrCmp $1 "" noWALs
        WALloop:
          StrCmp $1 "${MODERNSKINNAME}.wal" skip_wal
            Rename $0\skins\$1 $R0\$1
          skip_wal:
          FindNext $R1 $1
          StrCmp $1 "" "" WALloop
        FindClose $R1
      noWALs:

      FindFirst $R1 $1 $0\skins\*
      StrCmp $1 "" noDirs
        dirloop:
          StrCmp $1 "${MODERNSKINNAME}" skip_dir
            StrCmp $1 . skip_dir
            StrCmp $1 .. skip_dir
              IfFileExists $0\skins\$1\skin.xml "" skip_dir
                Rename $0\skins\$1 $R0\$1
          skip_dir:
          FindNext $R1 $1
          StrCmp $1 "" "" dirloop
        FindClose $R1
      noDirs:

    Pop $R1
  Pop $R0
  SetDetailsPrint lastused

  DetailPrint $(msgWA3_REMOVE)
  SetDetailsPrint none

  ExecWait '"$INSTDIR\Winamp.exe" /UNREG'
  ExecWait '"$0\uninst-wa3.exe" /S _=$0'
  Sleep 2000
  StrCpy $2 "foo"
  SetDetailsPrint lastused

dirupgrade:
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Winamp3"
  DeleteRegKey HKLM "SOFTWARE\Nullsoft\Winamp3"
  DeleteRegKey HKEY_CLASSES_ROOT .wal
  DeleteRegKey HKCR .b4s
  DeleteRegKey HKCR Winamp3.SkinZip
  DeleteRegKey HKCR Winamp3.File
  DeleteRegKey HKCR Winamp3.PlayList

  Delete $0\winamp3.exe
  Delete $0\studio.exe


  RMdir $0\skins
  RMdir /r $0\wacs
  RMdir /r $0\locales
  Delete $0\uninst-wa3.exe
  RMdir $0

  SetDetailsPrint textonly

  StrCmp $2 "" noupgrade
  IfFileExists $0 0 noupgrade
    MessageBox MB_OK $(msgWA3_REMOVE2)
  
noupgrade:
  SetDetailsPrint lastused
  Pop $2
  Pop $1
  Pop $0
FunctionEnd
