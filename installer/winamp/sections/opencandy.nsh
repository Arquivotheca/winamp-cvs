!ifndef NULLSOFT_WINAMP_INSALLER_OPENCANDY_HEADER
!define NULLSOFT_WINAMP_INSALLER_OPENCANDY_HEADER
!ifdef OPENCANDY | OPENCANDY_FINAL

!addplugindir "${NSISDIR}\OpenCandy"

Var opencandy.alreadyStarted
Var opencandy.userAbort

!define OC_STR_MY_PRODUCT_NAME "$(^NameDA)"
!define OC_OCSETUPHLP_FILE_PATH "${NSISDIR}\OpenCandy\OCSetupHlp.dll"

!ifdef OPENCANDY_FINAL
	!define OC_STR_KEY		"0a34e3033295eabaeca241ad8c276442"
	!define OC_STR_SECRET	"7837f8313d85d6cf33963d36604015dd"
!else
	; Beta 
	!define OC_STR_KEY		"af1bfeb4134cfd0e3d781b7224c79cf1"
	!define OC_STR_SECRET	"1f3a527f47d95270f2b0423c98957d5a"
	; Sample
	;!define OC_STR_KEY "748ad6d80864338c9c03b664839d8161"
	;!define OC_STR_SECRET "dfb3a60d6bfdb55c50e1ef53249f1198"
!endif	

!include "OCSetupHlp.nsh"
!insertmacro OpenCandyReserveFile

!macro OPENCANDY_START
	${If} 0 == $opencandy.alreadyStarted
		StrCpy $opencandy.alreadyStarted 1
		!insertmacro OpenCandyAsyncInit "${OC_STR_MY_PRODUCT_NAME}" "${OC_STR_KEY}" "${OC_STR_SECRET}" ${OC_INIT_MODE_NORMAL}
	${EndIf}
!macroend
!define OPENCANDY_START "!insertmacro 'OPENCANDY_START'"

!macro OPENCANDY_INIT
	StrCpy $opencandy.alreadyStarted 0
	StrCpy $opencandy.userAbort 0
	${If} ${Silent}
		${OPENCANDY_START}
	${EndIf}	
!macroend
!define OPENCANDY_INIT "!insertmacro 'OPENCANDY_INIT'"

!define OPENCANDY_PAGE "!insertmacro 'OpenCandyOfferPage'"

!macro OPENCANDY_SECTION
	Section -OpenCandyInstall
		!insertmacro 'OpenCandyInstallEmbedded'
	SectionEnd
!macroend
!define OPENCANDY_SECTION "!insertmacro 'OPENCANDY_SECTION'"

!macro OPENCANDY_FINISH_SUCCESS
	${If} 0 != $opencandy.alreadyStarted
		!insertmacro 'OpenCandyOnInstSuccess'
	${EndIf}
!macroend
!define OPENCANDY_FINISH_SUCCESS	"!insertmacro 'OPENCANDY_FINISH_SUCCESS'"

!define OPENCANDY_FINISH_FAILED

!macro OPENCANDY_GUIEND
	${If} 0 != $opencandy.alreadyStarted
		!insertmacro 'OpenCandyOnGuiEnd'
	${EndIf}
!macroend
!define OPENCANDY_GUIEND	"!insertmacro 'OPENCANDY_GUIEND'"

!macro OPENCANDY_SETLICENSE
	!ifndef LICENSE_PATH
		!define LICENSE_PATH "..\..\resources\license\license_with_opencandy.txt"
	!endif
!macroend
!define OPENCANDY_SETLICENSE "!insertmacro 'OPENCANDY_SETLICENSE'"

!macro OPENCANDY_USER_ABORT
	StrCpy $opencandy.userAbort 1
!macroend
!define OPENCANDY_USER_ABORT "!insertmacro 'OPENCANDY_USER_ABORT'"

!macro OPENCANDY_GETSHOWPAGE	__varOut
	${If} 0 != $opencandy.alreadyStarted
	
		!insertmacro GetOCOfferStatus ${OC_NSIS_FALSE}
		Pop ${__varOut}
		${If} ${OC_OFFER_STATUS_CANOFFER_READY} == ${__varOut}
			StrCpy ${__varOut} "true"
		${Else}	
			StrCpy ${__varOut} "false"
		${EndIf}
	${Else}	
		StrCpy ${__varOut} "false"
	${EndIf}
!macroend
!define OPENCANDY_GETSHOWPAGE "!insertmacro 'OPENCANDY_GETSHOWPAGE'"


!define OPENCANDY_PERFORM_API_CHECK "!insertmacro 'OpenCandyAPIDoChecks'"

!else ;OPENCANDY | OPENCANDY_FINAL

!define OPENCANDY_INIT
!define OPENCANDY_START
!define OPENCANDY_PAGE
!define OPENCANDY_SECTION
!define OPENCANDY_FINISH_SUCCESS
!define OPENCANDY_FINISH_FAILED
!define OPENCANDY_GUIEND
!define OPENCANDY_SETLICENSE
!define OPENCANDY_SEND_TRACKING
!define OPENCANDY_USER_ABORT
!define OPENCANDY_PERFORM_API_CHECK

!macro OPENCANDY_GETSHOWPAGE	__varOut
	StrCpy ${__varOut} "false"
!macroend
!define OPENCANDY_GETSHOWPAGE	"!insertmacro 'OPENCANDY_GETSHOWPAGE'"

!endif ;OPENCANDY | OPENCANDY_FINAL
!endif ;NULLSOFT_WINAMP_INSALLER_OPENCANDY_HEADER