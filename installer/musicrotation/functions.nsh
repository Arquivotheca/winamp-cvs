!ifndef STRINGREPLACE_HELPER
!define STRINGREPLACE_HELPER

Function NormalizePathPart
	Exch $0
	Push $1
	Push $2
	Push $3
	
	StrCpy $1 ""
	StrLen $3 $0
	
	
normalize_path_loop:
	IntCmp $3 0 normalize_path_finish 
	StrCpy $2 $0 1 -$3
	StrCmp $2 "?" normalize_path_filter1
	StrCmp $2 "*" normalize_path_filter1
	StrCmp $2 "|" normalize_path_filter1
	StrCmp $2 "/" normalize_path_filter2
	StrCmp $2 "\" normalize_path_filter2
	StrCmp $2 ":" normalize_path_filter2
	StrCmp $2 "$\"" normalize_path_filter3
	StrCmp $2 "<" normalize_path_filter4
	StrCmp $2 ">" normalize_path_filter5
	goto normalize_path_copychar
normalize_path_filter1:
	StrCpy $2 "_"
	goto normalize_path_copychar
normalize_path_filter2:
	StrCpy $2 "-"
	goto normalize_path_copychar
normalize_path_filter3:
	StrCpy $2 "'"
	goto normalize_path_copychar
normalize_path_filter4:
	StrCpy $2 "("
	goto normalize_path_copychar
normalize_path_filter5:
	StrCpy $2 ")"
	goto normalize_path_copychar
	
normalize_path_copychar:	
	StrCpy $1 "$1$2"
	IntOp $3 $3 - 1
	goto normalize_path_loop

normalize_path_finish:
	Pop $3
	Pop $2
	Exch $1
	Exch 1
	Pop $0
FunctionEnd
   
!macro NormalizePathPartMacro stringOut stringIn
	Push "${stringIn}"
	Call NormalizePathPart
	Pop "${stringOut}"
!macroend

!define NormalizePathPart '!insertmacro "NormalizePathPartMacro"'


Function GetFileExtension
	Exch $0
	Push $1
	Push $2
	
	StrLen $2 $0
	IntOp $2 $2 - 1
	
file_extension_loop:
	IntCmp $2 0 0 file_extension_notfound 0
	StrCpy $1 $0 1 $2
	StrCmp $1 "." file_extension_found
	IntOp $2 $2 - 1
	goto file_extension_loop

file_extension_notfound:
	StrCpy $1 "mp3"
	goto file_extension_finish
	
file_extension_found:
	IntOp $2 $2 + 1
	StrCpy $1 $0 "" $2
	StrCmp $1 "" 0 file_extension_finish
		StrCpy $1 "mp3"
	goto file_extension_finish
	
file_extension_finish:
	Pop $2
	Exch $1
	Exch 1
	Pop $0
FunctionEnd

!macro GetFileExtensionMacro stringOut stringIn
	Push "${stringIn}"
	Call GetFileExtension
	Pop "${stringOut}"
!macroend

!define GetFileExtension '!insertmacro "GetFileExtensionMacro"'

!endif ;STRINGREPLACE_HELPER