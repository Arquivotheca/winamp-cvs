!include "FileFunc.nsh"
!include "logicLib.nsh"


OutFile "fileSizeLocal.exe"
SilentInstall silent
RequestExecutionLevel user
 
Section
	${GetParameters} $0
	ClearErrors
	${GetOptions} $0 "/output="	$1
	${If} ${Errors}
		StrCpy $1 "$EXEDIR\fileSizeLocal.nsh"
	${EndIf}
	
	FileOpen $2 "$1" w
	${If} ${Errors}
		goto file_size_local_end
	${EndIf}
		
	${GetOptions} $0 "/source=" $3
	${If} ${Errors}
		FileWrite $2 '!error $\"FileSizeLocal unable to open file $\'$3$\'$\"'
		goto file_size_local_close
	${Else}
	
		SearchPath $5 "$3"
		${If} ${Errors}
			FileWrite $2 '!error $\"FileSizeLocal unable to locate file $\'$3$\'$\"'
			goto file_size_local_close
		${EndIf}
		${GetParent} "$5" $3
		${GetFileName} "$5" $4
		${GetSize} "$3" "/M=$4 /S=0B /G=0" $6 $7 $8
		${If} ${Errors}
			FileWrite $2 '!error $\"FileSizeLocal unable to get file size for $\'$5$\'$\"'
			goto file_size_local_close
		${Else}
			FileWrite $2 '!define FILE_SIZE_LOCAL $6'
		${EndIf}		
	${EndIf}

file_size_local_close:
	FileClose $2
 file_size_local_end:
SectionEnd