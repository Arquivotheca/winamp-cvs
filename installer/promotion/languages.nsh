!ifndef PROMOTION_LANGUAGES_HEADER
!define PROMOTION_LANGUAGES_HEADER

!include "langFile.nsh"

!macro PROMOTION_LANG_INCLUDE __languageId __languageNsisId

	!ifdef "LANG_USE_${__languageId}" | LANG_USE_ALL
		!define PROMOTION_LANG_INCLUDE_OKTOINCLUDE
	!endif

	!ifdef PROMOTION_LANG_INCLUDE_OKTOINCLUDE
		!echo "Including language support for: ${__languageId}"
   
		!verbose push
		!verbose 2
    	
		!ifndef PROMOTION_LANG_ATLEASTONE
			!define PROMOTION_LANG_ATLEASTONE
		!endif 
	
		!ifndef "NSIS_NLF_${__languageNsisId}_LOADED"
			LoadLanguageFile "${NSISDIR}\Contrib\Language files\${__languageNsisId}.nlf"
			!define "NSIS_NLF_${__languageNsisId}_LOADED"
		!endif
    
		!ifndef LANGFILE_DEFAULT
			!define LANGFILE_DEFAULT "${NSISDIR}\Contrib\Language files\English.nsh"
		!endif
		
		!insertmacro LANGFILE_INCLUDE "${NSISDIR}\Contrib\Language files\${__languageNsisId}.nsh"

		!ifdef LANGFILE_DEFAULT
			!undef LANGFILE_DEFAULT
		!endif
	
		!insertmacro LANGFILE_INCLUDE_WITHDEFAULT ".\strings\strings_${__languageId}.nsh" ".\strings\strings_en-us.nsh"

		
		!define LANGFILE_DEFAULT "${NSISDIR}\Contrib\Language files\English.nsh"
		
		!undef PROMOTION_LANG_INCLUDE_OKTOINCLUDE
		!verbose pop
	!endif
!macroend
!define PROMOTION_LANG_INCLUDE	"!insertmacro 'PROMOTION_LANG_INCLUDE'"

!ifndef LANG_USE_EN-US
	!define LANG_USE_EN-US 
!endif

${PROMOTION_LANG_INCLUDE} "EN-US" "English"

!endif
