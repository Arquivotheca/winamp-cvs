;!define BETA

; Do not define BETA - will be defined by the verctrl if necessary....
; all versions number will be also modified

!ifndef _DEBUG

!ifndef VERSION_MAJOR
  !define VERSION_MAJOR   "0"
!endif

!ifndef VERSION_MINOR
  !define VERSION_MINOR   "0"
!endif

!ifndef VERSION_MINOR_SECOND
  !define VERSION_MINOR_SECOND   "0"
!endif

!ifndef VERSION_MINOR_SECOND_SHORT
  !define VERSION_MINOR_SECOND_SHORT   "0"
!endif

!ifndef BUILD_NUM
  !define BUILD_NUM   "000"
!endif

VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND_SHORT}.${BUILD_NUM}"

  VIAddVersionKey "ProductName" "${WINAMP} Installer"
  VIAddVersionKey "Comments" "Visit http://www.winamp.com/ for updates."
  VIAddVersionKey "CompanyName" "Nullsoft, Inc."
  VIAddVersionKey "LegalTrademarks" "Nullsoft and Winamp are trademarks of Nullsoft, Inc."
  VIAddVersionKey "LegalCopyright" "Copyright © 1997-2013, Nullsoft, Inc."
  VIAddVersionKey "FileDescription" "${WINAMP} Installer"


VIAddVersionKey "FileVersion" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND_SHORT}.${BUILD_NUM}"
VIAddVersionKey "ProductVersion" "${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND_SHORT} Build ${BUILD_NUM}"
!ifdef BETA
  VIAddVersionKey "SpecialBuild" "${VERSION_ADDITIONALINFO} Beta"
!else
  VIAddVersionKey "SpecialBuild" "${VERSION_ADDITIONALINFO}"
!endif

!ifndef InstallType
  !ifdef BETA
    !define InstallType "Beta"
  !else ifdef NIGHT
    !define InstallType "Nightly"
  !else
    !define InstallType ""
  !endif
!endif

!else ; _DEBUG

!define VERSION_MAJOR                "Debug"
!define VERSION_MINOR                ""
!define VERSION_MINOR_SECOND         ""
!define VERSION_MINOR_SECOND_SHORT   ""
!define BUILD_NUM                    ""

!define InstallType                  "Internal"

!endif ; _DEBUG













































