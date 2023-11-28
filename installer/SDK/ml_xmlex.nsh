; XML Parser / Media Library example
SetOutPath $INSTDIR\ml_xmlex

; project files
File ${PROJECTS}\ml_xmlex\ml_xmlex.sln
File ${PROJECTS}\ml_xmlex\ml_xmlex.vcproj

; source
File ${PROJECTS}\ml_xmlex\main.cpp
File ${PROJECTS}\ml_xmlex\main.h
File ${PROJECTS}\ml_xmlex\xmlview.cpp

; resources
File ${PROJECTS}\ml_xmlex\resource.h
File ${PROJECTS}\ml_xmlex\ml_xmlex.rc

; documents and sample files
File ${PROJECTS}\ml_xmlex\readme.txt
File ${PROJECTS}\ml_xmlex\xmltest.xml
File ${PROJECTS}\ml_xmlex\xmltest2.xml

