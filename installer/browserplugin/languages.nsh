!ifndef WACHK_LANGUAGES_HEADER
!define WACHK_LANGUAGES_HEADER

!macro WACHK_LANG_INCLUDE __languageId __languageNsisId
	!ifndef WACHK_EMBEDDED_INSTALLER

		!ifdef "LANG_USE_${__languageId}" | LANG_USE_ALL
			!define WACHK_LANG_INCLUDE_OKTOINCLUDE
		!endif

		!ifdef WACHK_LANG_INCLUDE_OKTOINCLUDE
			!echo "browserplugin: including language support for ${__languageId}"
	   
			!verbose push
			!verbose 2
		
			!ifndef MUI_LANGDLL_ALLLANGUAGES
				!define MUI_LANGDLL_ALLLANGUAGES
			!endif 
		
			!ifndef MUI_LANGDLL_ALWAYSSHOW
				!ifdef WACHK_LANG_ATLEASTONE
					!define MUI_LANGDLL_ALWAYSSHOW
				!endif
			!endif

			!ifndef WACHK_LANG_ATLEASTONE
				!define WACHK_LANG_ATLEASTONE
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
			
			!undef WACHK_LANG_INCLUDE_OKTOINCLUDE
			!verbose pop
		!endif
	!else ; embedded version
		!ifdef "NSIS_NLF_${__languageNsisId}_LOADED"
			!insertmacro LANGFILE_INCLUDE_WITHDEFAULT ".\strings\strings_${__languageId}.nsh" ".\strings\strings_en-us.nsh"
		!endif
	!endif

!macroend

!define WACHK_LANG_INCLUDE	"!insertmacro 'WACHK_LANG_INCLUDE'"

!ifndef WACHK_EMBEDDED_INSTALLER
	!ifndef LANG_USE_EN-US
		!define LANG_USE_EN-US 
	!endif
!endif

${WACHK_LANG_INCLUDE} "EN-US" "English"
${WACHK_LANG_INCLUDE} "DE-DE" "German"
${WACHK_LANG_INCLUDE} "ES-US" "SpanishInternational"
${WACHK_LANG_INCLUDE} "PL-PL" "Polish"
${WACHK_LANG_INCLUDE} "TR-TR" "Turkish"
${WACHK_LANG_INCLUDE} "PT-BR" "PortugueseBR"
${WACHK_LANG_INCLUDE} "JA-JP" "Japanese"
${WACHK_LANG_INCLUDE} "NL-NL" "Dutch"
${WACHK_LANG_INCLUDE} "FR-FR" "French"
${WACHK_LANG_INCLUDE} "RU-RU" "Russian"
${WACHK_LANG_INCLUDE} "RO-RO" "Romanian"
${WACHK_LANG_INCLUDE} "KO-KR" "Korean"
${WACHK_LANG_INCLUDE} "ZH-TW" "TradChinese"
${WACHK_LANG_INCLUDE} "ZH-CN" "SimpChinese"
${WACHK_LANG_INCLUDE} "SV-SE" "Swedish"
${WACHK_LANG_INCLUDE} "IT-IT" "Italian"
!endif
