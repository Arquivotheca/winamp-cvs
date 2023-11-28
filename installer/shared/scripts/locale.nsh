!ifndef NSIS_EXTENSION_LIBRARY_LANGUAGE_HEADER
!define NSIS_EXTENSION_LIBRARY_LANGUAGE_HEADER

!define LOCALE_SISO639LANGNAME      0x00000059
!define LOCALE_SISO3166CTRYNAME     0x0000005A

!macro Locale_MakeLocaleID __languageID __sortID __outputVar
	Push $0
	Push $1
	IntOp $0 ${__languageID} & 0xFFFF
	IntOp $1 ${__sortID} & 0xFFFF
	IntOp $1 $1 << 16
	IntOp $0 $1 | $0
	Pop $1
	Exch $0
	Pop ${__outputVar}
!macroend
!define Locale_MakeLocaleID "!insertmacro 'Locale_MakeLocaleID'"

Function Locale_GetInfo
	Exch $0
	Exch
	Exch $1
	Push $2
	System::Call "kernel32::GetLocaleInfoW(i r0,  i r1, w .r0, i ${NSIS_MAX_STRLEN}) i.r2"
	${If} $2 == 0
		StrCpy $0 ""
	${EndIf}
	Pop $2
	Pop $1
	Exch $0
FunctionEnd
!macro Locale_GetInfo __localeID __infoID __outputVar
	Push "${__infoID}"
	Push "${__localeID}"
	Call Locale_GetInfo
	Pop "${__outputVar}"
!macroend
!define Locale_GetInfo "!insertmacro 'Locale_GetInfo'"

!macro Locale_GetCountryName __localeID __outputVar
	Push "${LOCALE_SISO3166CTRYNAME}"
	Push "${__localeID}"
	Call Locale_GetInfo
	Pop "${__outputVar}"
!macroend
!define Locale_GetCountryName "!insertmacro 'Locale_GetCountryName'"

!macro Locale_GetLanguageName __localeID __outputVar
	Push "${LOCALE_SISO639LANGNAME}"
	Push "${__localeID}"
	Call Locale_GetInfo
	Pop "${__outputVar}"
!macroend
!define Locale_GetLanguageName "!insertmacro 'Locale_GetLanguageName'"

!endif ;NSIS_EXTENSION_LIBRARY_LANGUAGE_HEADER