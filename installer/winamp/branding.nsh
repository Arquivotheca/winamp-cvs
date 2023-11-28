!ifndef NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER
!define NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER

!include ".\sections\openCandy.nsh"


!ifdef NOKIA
	!define WINAMP				"Winamp Nokia Edition"
	!define WINAMPFOLDER  		"${WINAMP}"
	!define WINAMPEXE 			"winamp.exe"
	!define WINAMPLINK			"${WINAMP}.lnk"
	!define MODERNSKINNAME 		"Nokia Edition"

	!ifdef BETA
		!define InstallType 	"Nokia Edition Beta"
	!else
		!define InstallType 	"Nokia Edition"
	!endif
!else
	!define WINAMP				"Winamp"

	!ifdef WINAMP64
		!define WINAMPFOLDER  	"${WINAMP}64"
	!else
		!define WINAMPFOLDER 	"${WINAMP}"
	!endif
	!define WINAMPEXE 			"winamp.exe"

	!define WINAMPLINK			"${WINAMP}.lnk"
	!define MODERNSKINNAME 		"Winamp Modern"
!endif

; Header image path
!ifdef NOKIA
	!define HEADER_IMAGE_PATH 	".\res\wabanner256_nokia.bmp"
!else ifdef BETA
	!define HEADER_IMAGE_PATH 	".\res\wabetabanner256.bmp"
!else ifdef NIGHT
	!define HEADER_IMAGE_PATH 	".\res\wanightbanner256.bmp"
!else
	!define HEADER_IMAGE_PATH 	".\res\wabanner55.bmp"
!endif

; WelcomeFinish image path
!ifndef BETA
	!define WELCOMEFINISH_IMAGE_PATH 				".\res\welcome55.bmp"
	!define UNINSTALLER_WELCOMEFINISH_IMAGE_PATH 	"${WELCOMEFINISH_IMAGE_PATH}"
!else
	!define WELCOMEFINISH_IMAGE_PATH 				"${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
	!define UNINSTALLER_WELCOMEFINISH_IMAGE_PATH 	"${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"
!endif ; BETA

; License path
!ifdef NOKIA
	!define LICENSE_PATH  "..\..\resources\license\license_nokia.txt"
!else ifdef BETA
	!define LICENSE_PATH  "..\..\resources\license\licenseBeta.txt"
!else ifdef NIGHT
	!define LICENSE_PATH  "..\..\resources\license\licenseNight.txt"
!else
	${OPENCANDY_SETLICENSE}
	!ifndef LICENSE_PATH 
		!define LICENSE_PATH "..\..\resources\license\license.txt"
	!endif
!endif

!ifdef BUNDLE
	!define INSTALLER_TYPE_DESCRIPTION "$(installerContainsBundle)"
!else ifdef PRO
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsPro)"
!else ifdef FULL
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsFull)"
!else ifdef STD
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsStandard)"
!else ifdef LITE
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsLite)"
!else
	!define INSTALLER_TYPE_DESCRIPTION	""
!endif


!endif ;NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER