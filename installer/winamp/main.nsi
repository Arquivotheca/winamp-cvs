!ifdef _DEBUG
  SetCompress off
  FileBufSize 64
  SetDatablockOptimize off
!else
  SetDatablockOptimize on
  !ifdef lzma
    SetCompressor /SOLID lzma
    SetCompressorDictSize 16
    FileBufSize 64
  !else
    SetCompressor /SOLID bzip2 ; actually doesnt seem to help :(
  !endif
!endif

ReserveFile "${NSISDIR}\Plugins\LangDLL.dll"
ReserveFile "${NSISDIR}\Plugins\nsDialogs.dll"
ReserveFile "${NSISDIR}\Plugins\System.dll"

!define PRIMOSDK_PATH	"..\..\SDKs\Rovi PrimoSDK Plus\4_28_06_0"
!define PRIMOSDK_REDIST_PATH "${PRIMOSDK_PATH}\Redist"

!ifndef FILES_PATH
 !define FILES_PATH ..\..\output\winamp
!endif

!define MUI_LANGDLL_REGISTRY_ROOT HKLM
!define MUI_LANGDLL_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "LangId"


!define MUI_LANGDLL_WINDOWTITLE $(LANGUAGE_DLL_TITLE)
!define MUI_LANGDLL_INFO $(LANGUAGE_DLL_INFO)

!define MUI_CUSTOMFUNCTION_ABORT	OnInstallAbort

;!include "nsDialogs.nsh"
;!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "WinMessages.nsh"

!include "LogicLib.nsh"
!include ".\parameters.nsh"
!include ".\branding.nsh"

!include "verInfo.nsh"

Var SETTINGSDIR
Var INSTINI
Var WINAMPINI
Var WINAMPM3U
Var M3UBASEDIR
Var PREVINSTINI
Var RESTARTAGENT
Var FIRSTINSTALL
Var needplaystart
Var IsNT     ; 1 - if NT based,  otherwise - 0
Var WinVer   ; OS code, check utils\getWinVer.nsh for all values
Var WinVerRelease ; if WinVer = 95 or 98 - will contain Release specs
Var IsUS
Var IDX_INSTTYPE_STANDART
Var IDX_INSTTYPE_PREVIUOS
Var IDX_INSTTYPE_FULL
Var PROMOTIONCONFIG


!ifndef _DEBUG
 BrandingText "Nullsoft Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType} -- $(BuiltOn) ${__DATE__} $(at) ${__TIME__}"
!else
 BrandingText "Nullsoft Winamp Debug -- internal use only"
!endif

RequestExecutionLevel admin
Caption $(IDS_CAPTION)
Name "${WINAMP}"
InstallDir "$PROGRAMFILES\${WINAMPFOLDER}"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "UninstallString"

XPStyle on


!ifdef netscape
 !define OVERWRITEMODE try
!else
 !define OVERWRITEMODE on
!endif

!ifdef _DEBUG
 ShowInstDetails show
 AutoCloseWindow false
!else
 ShowInstDetails nevershow
 AutoCloseWindow true
!endif

SetDateSave on

!define SECTIONIN_FULL "SectionIn 1"

!ifdef FULL
 !define SECTIONIN_STD "SectionIn 1 2"
 !define SECTIONIN_LITE "SectionIn 1 2 3"
!else ifdef STD
 !define SECTIONIN_STD "SectionIn 1"
 !define SECTIONIN_LITE "SectionIn 1 2"
!else
 !define SECTIONIN_STD "SectionIn 1"
 !define SECTIONIN_LITE "SectionIn 1"
!endif


!include ".\utils\wafuncs.nsh"
!include ".\utils\getWinVer.nsh"
!include ".\utils\getlocale.nsh"

!ifdef _DEBUG
 OutFile "wasetup_dbg${LANG_FILESPEC}.exe"
!else
 !ifdef LITE
  OutFile "$%INSTALL_LITE%${LANG_FILESPEC}.exe"
 !else ifdef full
  !ifdef PRO
   OutFile "$%INSTALL_PRO%${LANG_FILESPEC}.exe"
  !else ifdef eMusic-7plus
   !ifdef BUNDLE
    OutFile "$%INSTALL_BUNDLE%${LANG_FILESPEC}.exe"
   !else
    OutFile "$%INSTALL_EMUSIC%${LANG_FILESPEC}.exe"
   !endif
  !else
   !ifdef BUNDLE
    OutFile "$%INSTALL_BUNDLE%${LANG_FILESPEC}.exe"
   !else
    OutFile "$%INSTALL_FULL%${LANG_FILESPEC}.exe"
   !endif
  !endif
 !else ifdef STD
  OutFile "$%INSTALL_STD%${LANG_FILESPEC}.exe"
 !else
  OutFile 'winamp_unknown${LANG_FILESPEC}.exe'
 !endif
!endif

;!include ".\sections\opencandy.nsh"

!include ".\ui.nsh"

!include ".\sectionsHelper.nsh"
!include ".\wasections.nsh"


!include ".\uninstall\uninstall.nsh"

!include ".\languages.nsh"

Section -LastSection IDX_SECTION_LAST ; keep last section after languages
SectionEnd

!include ".\uiEvents.nsh"
!include ".\descriptionTable.nsh"

Function .onInit
	!ifdef WINAMP64
	ReadEnvStr $0 PROGRAMW6432
	StrCpy $INSTDIR "$0\${WINAMPFOLDER}"
	!endif

	StrCpy $PROMOTIONCONFIG	""
	${GetParameters} $0
	ClearErrors
	${GetOptions} "$0" "/promotion" $1
	${IfNot} ${Errors}
		${StrFilter} "$1" "" "" " =$\"" $PROMOTIONCONFIG
	${EndIf}
	
	InitPluginsDir
  
	StrCpy $PREVINSTINI  ""
	StrCpy $INSTINI  "$PLUGINSDIR\install.ini"
  
	!ifdef LANGID
		WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME}" ${LANGID}
	!endif ; LANGID

	!insertmacro MUI_LANGDLL_DISPLAY
	
	; check windows version
	${GetWindowsVersion}
	Pop $WinVer
	Pop $WinVerRelease

	${If} $WinVer == "95"
	${OrIf} $WinVer == "98"
	${OrIf} $WinVer == "ME"
	${OrIf} $WinVer == "2000"
		MessageBox MB_OK|MB_ICONEXCLAMATION "$(IDS_MSG_WINDOWS_TOO_OLD)"
		Quit
	${EndIf}
	StrCpy $IsNT  1

	Call IsUSLocated
	Pop $IsUS
  
	${MUSIC_BUNDLE_INIT}
	${WINAMPDETECT_INIT}
	${XBundle_InitForWinamp}
	${OPENCANDY_INIT}
	${FrenchRadio_OnInit}
  
FunctionEnd

Function Promotion_Complete
	${If} $PROMOTIONCONFIG != ""
		ReadINIStr $0 "$PROMOTIONCONFIG" "Installer" "invokeComplete"
		${If} $0 != ""
			Exec "$0"
		${EndIf}
	${EndIf}
FunctionEnd

Function .onInstSuccess
	Delete $PREVINSTINI
	Call SaveSelection
	DeleteINISec "$INSTINI" "Bundle"
	CopyFiles /SILENT "$INSTINI" "$INSTDIR\install.ini"
	Call Promotion_Complete
	${XBundle_Finish}
	${OPENCANDY_FINISH_SUCCESS}
FunctionEnd

Function .onInstFailed
	Call Promotion_Complete
	${XBundle_Finish}
	${OPENCANDY_FINISH_FAILED}
FunctionEnd


Function OnInstallAbort
	Call Promotion_Complete
	${XBundle_Finish}
	${OPENCANDY_USER_ABORT}
FunctionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
  
  ; check windows version
	${GetWindowsVersion}
	Pop $WinVer
	Pop $WinVerRelease
	
	StrCpy $winamp.uninstall.checkFolder "false"
FunctionEnd

${InitializeGetSectionName}

Function GetLastInstTypeIndex
  Push $R0
  Push $R1
  StrCpy $R0 0
loop:
  InstTypeGetText $R0 $R1
  StrCmp $R1 '' +3
    IntOp $R0 $R0 + 1
    goto loop
  Pop $R1
  IntOp $R0 $R0 - 1
  Exch $R0
FunctionEnd

Function ReadSections
  Push $R0
  Push $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  Push $R6
  Push $R7
  Push $R8
  StrCpy $R1 ${IDX_SECTION_LAST}
  Call GetLastInstTypeIndex
  Pop $R5
  StrCpy $R0 1
  IntOp $R6 $R0 << $R5
  IntOp $R6 $R6 ~

  StrCpy $R0 0
  
  ReadINIStr $R8 "$INSTINI" "installer" "sectionsVer"
  
  loop:
  
	!ifdef IDX_SEC_ML
		StrCmp $R0 ${IDX_SEC_ML} skipRead
	!endif
	
	!ifdef IDX_SEC_ML_PMP
		StrCmp $R0 ${IDX_SEC_ML_PMP} skipRead
	!endif	
		   
	${If} $R8 >= 2
		${GetSectionName} $R0 $R2
	${Else}
		SectionGetText $R0 $R2
	${EndIf}
	
    StrCmp $R2 "-" skipRead
    StrCmp $R2 "" skipRead

    SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x00000017
    IntCmp $R3 1 gotoRead gotoRead
    IntOp $R7 $R3 & 0x00000002  ; check if it is section
    IntCmp $R7 0 +2
    StrCpy $R7 $R0  ;remember section index (we can use it later to make it bold)
    goto skipRead ; skip sections, section end and readonly
gotoRead:
    ReadINIStr $R4 "$INSTINI" "sections" $R2
  ;  MessageBox MB_OK "$R2 - $R4"
    StrCmp $R4 "" 0 normalRead ;new feature or not?
    IntOp $R4 0x00000000 | 0x0000009 ; new feature  make it selected and bold
    SectionGetFlags $R7 $0
    IntOp $0 $0 | 0x0000008
    SectionSetFlags $R7 $0
normalRead:
	!ifdef IDX_SEC_ML_ONLINE
	${If} $R0 == ${IDX_SEC_ML_ONLINE}
		ReadINIStr $R7 "$PROMOTIONCONFIG" "Installer" "onlineMedia"
		${If} $R7 == "enabled"
			IntOp $R4 $R4 | ${SF_SELECTED}
		${EndIf}
	${EndIf}
	!endif
	
    IntOp $R3 $R3 & 0xfffffffe
    IntOp $R3 $R3 | $R4
    SectionSetFlags $R0 $R3
	
    SectionGetInstTypes $R0 $R3
    IntOp $R3 $R3 & $R6
    IntOp $R4 $R4 << $R5
    IntOp $R3 $R3 | $R4
    SectionSetInstTypes $R0 $R3
skipRead:
    IntOp $R0 $R0 + 1
    IntCmp $R0 $R1 "" loop
  Pop $R8
  Pop $R7
  Pop $R6
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

Function SaveSelection
  Push $R0
  Push $R1
  Push $R2
  Push $R3
  StrCpy $R1 ${IDX_SECTION_LAST}
  StrCpy $R0 0
  
  WriteINIStr "$INSTINI" "installer" "sectionsVer" "2"
  DeleteINISec "$INSTINI" "sections"

	loop:
    !ifdef STD | FULL
     	StrCmp $R0 ${IDX_SEC_ML} 0 ignore_1
    	StrCmp $R0 ${IDX_SEC_ML_PMP} 0 ignore_1
    !endif ; FULL
  	${GetSectionName} $R0 $R2
   	SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x0001
    goto force_write
!ifdef STD | FULL
 ignore_1:
!endif ; FULL
    ${GetSectionName} $R0 $R2
    StrCmp $R2 "-" skipWrite
    StrCmp $R2 "" skipWrite
   
    SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x00000017
    IntCmp $R3 1 0 0 skipWrite ; skip sections, section end and readonly
 force_write:
    WriteINIStr "$INSTINI" "sections" $R2 $R3
skipWrite:
    IntOp $R0 $R0 + 1
    IntCmp $R0 $R1 "" loop
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

!ifdef STD | FULL
Function UpdateAutoSelectSection

	Exch $1    ; section id
	Exch
	Exch $0    ; group id
	Exch
	Push $2
	Push $3
	Push $4
	Push $5

	StrCpy $2 "1" ; - nesting counter
	StrCpy $3 "0" ; - sel counter
	IntOp $0  $0 + 1

	${Do}
		SectionGetFlags $0 $4
		IntOp $5 $4 & 0x0002  ; test if group start
		${If} $5 <> 0
			IntOp $2  $2 + 1
		${EndIf}
    
		IntOp $5 $4 & 0x0004  ; test if group end
		${If} $5 <> 0
			IntOp $2  $2 - 1
			${If} $2 == 0
				${ExitDo}
			${EndIf}
		${Else}
			IntOp $5 $4 & 0x0041  ; test if selected or partial selected
			${If} $5 <> 0
			${AndIf} $0 <> $1
				StrCpy $3  "1"
				${ExitDo}
			${EndIf}
		${EndIf}

		IntOp $0  $0 + 1
	${Loop}
  
	SectionGetFlags $1 $4
	IntOp $4 $4 & 0xFFEF
	${If} $3 <> 0
		IntOp $4 $4 | 0x0011
	${EndIf}
	SectionSetFlags $1 $4

	Pop $5
	Pop $4
	Pop $3
	Exch $2
	Exch
	Exch $1
	Exch 2
	Exch $0

FunctionEnd

!macro UpdateAutoSelectSection __groupId __sectionId
	Push ${__groupId}
	Push ${__sectionId}
	Call UpdateAutoSelectSection
!macroend
!define UpdateAutoSelectSection "!insertmacro 'UpdateAutoSelectSection'"
!endif

Function .onSelChange
	!ifdef STD | FULL
	${UpdateAutoSelectSection} ${IDX_GRP_WALIB_PORTABLE} ${IDX_SEC_ML_PMP}
	${UpdateAutoSelectSection} ${IDX_GRP_WALIB} ${IDX_SEC_ML}
	!endif
FunctionEnd

${OPENCANDY_PERFORM_API_CHECK}
