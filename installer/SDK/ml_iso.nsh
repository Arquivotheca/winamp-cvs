; XML Parser / Media Library example
SetOutPath $INSTDIR\ml_iso

; project files
File ${PROJECTS}\ml_iso\ml_iso.sln
File ${PROJECTS}\ml_iso\ml_iso.vcproj

; source
File ${PROJECTS}\ml_iso\main.cpp
File ${PROJECTS}\ml_iso\main.h
File ${PROJECTS}\ml_iso\ToISO.cpp
File ${PROJECTS}\ml_iso\api.h
