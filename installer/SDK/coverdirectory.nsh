; Cover Directory example component
SetOutPath $INSTDIR\coverdirectory

; project files
File ${PROJECTS}\coverdirectory\coverdirectory.sln
File ${PROJECTS}\coverdirectory\cover_directory.vcproj

; source
File ${PROJECTS}\coverdirectory\CoverDirectory.cpp
File ${PROJECTS}\coverdirectory\CoverDirectory.h
File ${PROJECTS}\coverdirectory\main.cpp
File ${PROJECTS}\coverdirectory\api.h

