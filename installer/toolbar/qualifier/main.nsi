!include "config.nsh"
!include "fileFunc.nsh"
!include "wordFunc.nsh"
!include "logicLib.nsh"

SetCompressor /SOLID zlib
SetCompress force

SetDatablockOptimize on
AutoCloseWindow true
ShowInstDetails nevershow
RequestExecutionLevel user
InstProgressFlags smooth
;XPStyle on

;Icon "${TOOLBAR_QUALIFIER_ICON_PATH}"
ChangeUI all "${TOOLBAR_QUALIFIER_UI_PATH}"

OutFile "${TOOLBAR_QUALIFIER_OUTPUT_FILE}"
Name ""


Function Toolbar_IsSupportedSystem
	Push $0
	Push $1
	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" "CurrentVersion"
	StrCpy $1 $0 1
	StrCpy $0 $0 1 2
	IntOp $0 $0 + $1
	${If} $1 >= 5
	${AndIf} $0 > 5  ; Win2000 > ...
		StrCpy $0 "yes"
	${Else}
		StrCpy $0 "no"
	${EndIf}
	Pop $1
	Exch $0
FunctionEnd
!macro Toolbar_IsSupportedSystem __outputVar
	Call Toolbar_IsSupportedSystem
	Pop "${__outputVar}"
!macroend
!define Toolbar_IsSupportedSystem "!insertmacro 'Toolbar_IsSupportedSystem'"	
	
Function Toolbar_IsInternetExplorerInstallRequired
	Exch $0
	Push $1
	ReadRegStr $1 HKLM "Software\Microsoft\Internet Explorer" "Version"
	${If} $1 != ""
		ClearErrors
		ReadRegStr $1 HKLM "Software\Winamp Toolbar\ieToolbar\CurrentVersion" "Upgrade"
		${If} ${Errors}
			ReadRegStr $1 HKLM "Software\Winamp Toolbar\ieToolbar\OriginalVersion" "Build"
		${EndIf}
				
		${If} $1 != ""
			Push $2
			Push $3
			StrLen $2 $1
			IntOp $2 $2 - 1
			StrCpy $3 $1 1 $2
			${If} $3 == ";"
				StrCpy $1 $1 $2
			${EndIf}
			Pop $3
			${VersionCompare} $1 $0  $2
			${If} $2 == 2
				StrCpy $0 "yes"
			${Else}
				StrCpy $0 "no"
			${EndIf}
			Pop $2
		${Else}
			StrCpy $0 "yes"
		${EndIf}
	${Else}
		StrCpy $0 "no"
	${EndIf}
	
	Pop $1
	Exch $0
FunctionEnd
!macro Toolbar_IsInternetExplorerInstallRequired __toolbarVersion __outputVar
	Push "${__toolbarVersion}"
	Call Toolbar_IsInternetExplorerInstallRequired
	Pop "${__outputVar}"
!macroend
!define Toolbar_IsInternetExplorerInstallRequired "!insertmacro 'Toolbar_IsInternetExplorerInstallRequired'"		
	
Function Toolbar_IsFirefoxInstallRequired
	Exch $0
	Push $1
	ClearErrors
	ReadRegStr $1 HKLM "Software\Mozilla\Mozilla Firefox" "CurrentVersion"
	${If} ${Errors}
		ReadRegStr $1 HKCU "Software\Mozilla\Mozilla Firefox" "CurrentVersion"
	${EndIf}	
	
	${If} $1 != ""
		Push $2
		Push $3
		Push $4
		
		FindFirst $2 $1 "$APPDATA\Mozilla\Firefox\Profiles\*"
		loop:
		${If} $1 != ""
			FindFirst $3 $4 "$APPDATA\Mozilla\Firefox\Profiles\$1\extensions\{0b38152b-1b20-484d-a11f-5e04a9b0661f}\install.rdf"
			FindClose $3
			${If} $4 == ""
				FindNext $2 $1
				Goto loop
			${Else}
				StrCpy $1 "$APPDATA\Mozilla\Firefox\Profiles\$1\extensions\{0b38152b-1b20-484d-a11f-5e04a9b0661f}\install.rdf"
			${EndIf}
		${EndIf}		
		FindClose $2
		
		${If} $1 != ""
			ClearErrors
			FileOpen $2 $1 r
			${IfNot} ${Errors} 
				${Do}
					FileRead $2 $1
					${If} $1 != ""
						${WordFind2X} "$1" "<em:version>" "</em:version>" "+1" $3
						${If} $3 != $1
							StrCpy $1 $3
							${ExitDo}
						${EndIf}
					${Else}
						${ExitDo}
					${EndIf}
				${Loop}
				FileClose $2
			${EndIf}	
		${EndIf}
		
		${If} $1 != ""
			${VersionCompare} $1 $0 $2
			${If} $2 == 2
				StrCpy $0 "yes"
			${Else}
				StrCpy $0 "no"
			${EndIf}
		${Else}
			StrCpy $0 "yes"	
		${EndIf}
		
		Pop $4
		Pop $3
		Pop $2
	${Else}
		StrCpy $0 "no"
	${EndIf}	
		
	Pop $1
	Exch $0
FunctionEnd
!macro Toolbar_IsFireFoxInstallRequired __toolbarVersion __outputVar
	Push "${__toolbarVersion}"
	Call Toolbar_IsFireFoxInstallRequired
	Pop "${__outputVar}"
!macroend
!define Toolbar_IsFireFoxInstallRequired "!insertmacro 'Toolbar_IsFireFoxInstallRequired'"

Function .onInit
	
	${Toolbar_IsSupportedSystem} $0
	${If} $0 != "yes"
		SetErrorLevel 4
		Quit
	${EndIf}
	
	${GetParameters} $1
	
	${GetOptions} "$1" "--version-ie=" $2
	${If} $2 != ""	
		${Toolbar_IsInternetExplorerInstallRequired} $2 $0
		${If} $0 == "yes"
			SetErrorLevel 0
			Quit
		${EndIf}
	${EndIf}
	
	${GetOptions} "$1" "--version-firefox=" $2
	${If} $2 != ""
		${Toolbar_IsFireFoxInstallRequired} $2 $0
		${If} $0 == "yes"
			SetErrorLevel 0
			Quit
		${EndIf}
	${EndIf}
	
	SetErrorLevel 3
	Quit
FunctionEnd

Section -Main
SectionEnd