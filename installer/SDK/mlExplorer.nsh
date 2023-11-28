; Sample HTTP Plugin
SetOutPath $INSTDIR\mlExplorer

; project files
File ${PROJECTS}\mlExplorer\mlExplorer.sln
File ${PROJECTS}\mlExplorer\mlExplorer.vcproj

; source
File ${PROJECTS}\mlExplorer\mlExplorer.cpp

; docs
File ${PROJECTS}\mlExplorer\readme.txt
