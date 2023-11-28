!include "MUI2.nsh"
!include ".\fileFunc.nsh"
!include ".\wordFunc.nsh"
!include ".\logicLib.nsh"

;!define XBUNDLE_DISABLED

!define XBUNDLE_SOURCE_PATH_CD "..\xbundle"
;!define XBUNDLE_HOST_PATH_CD	"..\winamp"
!include ".\xbundle.nsh"

SetCompress off
ShowInstDetails show
AutoCloseWindow false

RequestExecutionLevel user ; admin
InstProgressFlags smooth
XPStyle on

Name 	"XBundle Test"
Caption "$(^NameDA)"
OutFile "XBundleTest.exe"
InstallDir "$TEMP\XBundle"

ShowInstDetails show

!define MUI_LANGDLL_REGISTRY_ROOT 					HKCU
!define MUI_LANGDLL_REGISTRY_KEY 					"Software\XBundle Test"
!define MUI_LANGDLL_REGISTRY_VALUENAME 				"LangId"

!define MUI_ICON 									"..\shared\icons\install.ico"
!define MUI_UNICON 									"..\shared\icons\uninstall.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP 						"..\shared\bitmaps\header.bmp"


!define MUI_WELCOMEPAGE_TITLE		 				"XBundle Test"
!define MUI_WELCOMEFINISHPAGE_BITMAP 				"..\shared\bitmaps\welcome.bmp"
!define MUI_PAGE_CUSTOMFUNCTION_PRE					XBundle_OnWelcomePagePre
!insertmacro MUI_PAGE_WELCOME


!define MUI_PAGE_HEADER_TEXT						"Choose Destination Folder"
!define MUI_PAGE_CUSTOMFUNCTION_PRE					XBundle_OnDirectoryPagePre
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE				XBundle_OnDirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

;!define XBUNDLE_PAGE_TEXT_TOP ""
;!define XBUNDLE_STRING_DOWNLOAD_PROGRESS_TEMPLATE	"<<< downloading >>>"
!define XBUNDLE_PAGE_CUSTOMFUNCTION_PRE				XBundle_OnBundlePagePre
${XBundle_InsertPage}

!insertmacro MUI_PAGE_INSTFILES


!define MUI_LANGDLL_WINDOWTITLE 					"Installer Language"
!define MUI_LANGDLL_INFO 							"Please select a language."
!define MUI_LANGDLL_ALWAYSSHOW
!define MUI_LANGDLL_ALLLANGUAGES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"

Function .onInit
	InitPluginsDir
	ClearErrors
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function XBundle_OnWelcomePagePre
	${XBundle_Init} "http://10.180.151.224/promotion/promotion.catalog"
FunctionEnd

Function XBundle_OnDirectoryPagePre
	${XBundle_BeginCatalogDownload}
FunctionEnd

Function XBundle_OnDirectoryPageLeave
	${XBundle_FinishCatalogDownload}
FunctionEnd


Function XBundle_OnBundlePagePre
	;MessageBox MB_OK "Hello, from XBundle Pre"
	;Abort
FunctionEnd

Section -Main
	SetDetailsView show
SectionEnd

${XBundle_InsertSection}