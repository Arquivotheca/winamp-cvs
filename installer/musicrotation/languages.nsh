!ifndef ROTATION_LANGUAGES_HEADER
!define ROTATION_LANGUAGES_HEADER

!macro ROTATION_LANG_INCLUDE __languageId __languageNsisId

	!ifdef "LANG_USE_${__languageId}" | LANG_USE_ALL
		!define ROTATION_LANG_INCLUDE_OKTOINCLUDE
	!endif

	!ifdef ROTATION_LANG_INCLUDE_OKTOINCLUDE
		!echo "Including language support for: ${__languageId}"
   
		!verbose push
		!verbose 2
    
		!ifndef MUI_LANGDLL_ALLLANGUAGES
			!define MUI_LANGDLL_ALLLANGUAGES
		!endif 
    
		!ifndef MUI_LANGDLL_ALWAYSSHOW
			!ifdef ROTATION_LANG_ATLEASTONE
				!define MUI_LANGDLL_ALWAYSSHOW
			!endif
		!endif

		!ifndef ROTATION_LANG_ATLEASTONE
			!define ROTATION_LANG_ATLEASTONE
		!endif 

		!insertmacro MUI_INSERT

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

		!ifndef MUI_LANGDLL_LANGUAGES
			!ifdef MUI_LANGDLL_ALLLANGUAGES
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${__languageNsisId}_NAME}' '${LANG_${__languageNsisId}}' "
			!else
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${__languageNsisId}_NAME}' '${LANG_${__languageNsisId}}' '${LANG_${__languageNsisId}_CP}' "
			!endif
		!else
			!ifdef MUI_LANGDLL_LANGUAGES_TEMP
				!undef MUI_LANGDLL_LANGUAGES_TEMP
			!endif
			!define MUI_LANGDLL_LANGUAGES_TEMP "${MUI_LANGDLL_LANGUAGES}"
			!undef MUI_LANGDLL_LANGUAGES

			!ifdef MUI_LANGDLL_ALLLANGUAGES
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${__languageNsisId}_NAME}' '${LANG_${__languageNsisId}}' ${MUI_LANGDLL_LANGUAGES_TEMP}"
			!else
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${__languageNsisId}_NAME}' '${LANG_${__languageNsisId}}' '${LANG_${__languageNsisId}_CP}' ${MUI_LANGDLL_LANGUAGES_TEMP}"
			!endif
		!endif
		
		!undef ROTATION_LANG_INCLUDE_OKTOINCLUDE
		!verbose pop
	!endif
!macroend
!define ROTATION_LANG_INCLUDE	"!insertmacro 'ROTATION_LANG_INCLUDE'"

!ifndef LANG_USE_EN-US
	!define LANG_USE_EN-US 
!endif

${ROTATION_LANG_INCLUDE} "EN-US" "English"
${ROTATION_LANG_INCLUDE} "DE-DE" "German"
${ROTATION_LANG_INCLUDE} "PL-PL" "Polish"
${ROTATION_LANG_INCLUDE} "PT-BR" "PortugueseBR"
${ROTATION_LANG_INCLUDE} "ES-US" "SpanishInternational"
${ROTATION_LANG_INCLUDE} "JA-JP" "Japanese"
${ROTATION_LANG_INCLUDE} "NL-NL" "Dutch"
${ROTATION_LANG_INCLUDE} "RU-RU" "Russian"
${ROTATION_LANG_INCLUDE} "FR-FR" "French"
${ROTATION_LANG_INCLUDE} "IT-IT" "Italian"

!endif
