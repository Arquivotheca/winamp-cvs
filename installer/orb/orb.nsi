!ifndef FILES_PATH
  !define FILES_PATH	"..\..\output\winamp"
!endif ;FILES_PATH

!ifndef WINAMP_NAME
  !define WINAMP_NAME	"Winamp"
!endif ;WINAMP_NAME

!ifndef WINAMP_PATH
  !define WINAMP_PATH "$PROGRAMFILES\${WINAMP_NAME}"
!endif

!ifndef WINAMPREMOTE_NAME
  !define WINAMPREMOTE_NAME '${WINAMP_NAME} Remote'
!endif

;------------------------

SetCompressor /SOLID /FINAL lzma 

RequestExecutionLevel admin
InstProgressFlags smooth
XPStyle on

Name "${WINAMPREMOTE_NAME}"
OutFile "orbembed.exe"
InstallDir "${WINAMP_PATH}"

!include ".\fileFunc.nsh"
!include ".\logicLib.nsh"

Var orb.installer.errorcode


!macro Orb_GetDllNeedReplace  __destinationDll __localDll __outputResult
	Push $0
	Push $1
	Push $2
	Push $3
	Push $4
	Push $5
	
	!ifndef orb.dllreplace.label.counter
		!define orb.dllreplace.label.counter 0
	!else
		!define /math orb.dllreplace.label.counter.tmp ${orb.dllreplace.label.counter} + 1
		!undef orb.dllreplace.label.counter
		!define orb.dllreplace.label.counter ${orb.dllreplace.label.counter.tmp}
		!undef orb.dllreplace.label.counter.tmp
	!endif
	
	!ifdef orb.dllreplace.label
		!undef orb.dllreplace.label
	!endif
	!define orb.dllreplace.label	orb.dllreplace.label.${orb.dllreplace.label.counter}
	
	${If} ${FileExists} "${__destinationDll}"
		ClearErrors
		GetDLLVersion "${__destinationDll}" $0 $1
		${IfNot} ${Errors}
			GetDLLVersionLocal "${__localDll}" $2 $3
			IntOp $4 $0 / 0x00010000
			IntOp $5 $2 / 0x00010000
			IntCmp $4 $5 0 ${orb.dllreplace.label}.replace ${orb.dllreplace.label}.skip
			IntOp $4 $0 & 0x0000FFFF
			IntOp $5 $2 & 0x0000FFFF
			IntCmp $4 $5 0 ${orb.dllreplace.label}.replace ${orb.dllreplace.label}.skip
			IntOp $4 $1 / 0x00010000
			IntOp $5 $3 / 0x00010000
			IntCmp $4 $5 0 ${orb.dllreplace.label}.replace ${orb.dllreplace.label}.skip
			IntOp $4 $1 & 0x0000FFFF
			IntOp $5 $3 & 0x0000FFFF
			IntCmp $4 $5 ${orb.dllreplace.label}.skip ${orb.dllreplace.label}.replace ${orb.dllreplace.label}.skip
		${EndIf}
	${EndIf}

${orb.dllreplace.label}.replace:
	StrCpy ${__outputResult} "true"
	Goto ${orb.dllreplace.label}.end
	
${orb.dllreplace.label}.skip:
	StrCpy ${__outputResult} ""
	Goto ${orb.dllreplace.label}.end
		
${orb.dllreplace.label}.end:	
	Pop $R5
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0
!macroend

!define GetDllNeedReplace "!insertmacro 'Orb_GetDllNeedReplace'"

Section .OnInit
	StrCpy $orb.installer.errorcode	 0
	
SectionEnd

Section -MainModule
	${GetParameters} $0
	ClearErrors
	${GetOptions} "$0" "/WINAMPONLY"  $1
	${If} ${Errors}
		File "/oname=$TEMP\orb.exe" "..\..\resources\bundles\winampremote\winampremote.exe"
		ExecWait '$TEMP\orb.exe /S -z"-nostart"' $orb.installer.errorcode
		Delete "$TEMP\orb.exe"
	${EndIf}
	
SectionEnd

Section -OmBrowser
	IntCmp $orb.installer.errorcode 0 0 orb.ombrowser.end orb.ombrowser.end
	
	SetOutPath "$INSTDIR\System"
	
	${GetDllNeedReplace} "$OUTDIR\omBrowser.w5s" "${FILES_PATH}\System\omBrowser.w5s" $0
	${If} $0 == "true"
		File "${FILES_PATH}\System\omBrowser.w5s"
	${EndIf}
		
orb.ombrowser.end:
SectionEnd

Section -MediaLibraryPlugin
	IntCmp $orb.installer.errorcode 0 0 orb.mlplugin.end orb.mlplugin.end
	
	SetOutPath "$INSTDIR\Plugins"
	
	${GetDllNeedReplace} "$OUTDIR\ml_orb.dll" "${FILES_PATH}\Plugins\ml_orb.dll" $0
	${If} $0 == "true"
		File "${FILES_PATH}\Plugins\ml_orb.dll"
	${EndIf}
		
orb.mlplugin.end:		
SectionEnd

Section -Last
	SetErrorLevel $orb.installer.errorcode
SectionEnd