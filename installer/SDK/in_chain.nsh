; Chained input plugin example
SetOutPath $INSTDIR\in_chain

; project files
File ${PROJECTS}\in_chain\in_chain.sln
File ${PROJECTS}\in_chain\in_chain.vcproj

; source
File ${PROJECTS}\in_chain\main.cpp
