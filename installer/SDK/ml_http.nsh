; Sample HTTP Plugin
SetOutPath $INSTDIR\ml_http

; project files
File ${PROJECTS}\ml_http\ml_http.sln
File ${PROJECTS}\ml_http\ml_http.vcproj

; source
File ${PROJECTS}\ml_http\main.cpp
File ${PROJECTS}\ml_http\main.h
File ${PROJECTS}\ml_http\HTMLControl.cpp
File ${PROJECTS}\ml_http\HTMLControl.h
File ${PROJECTS}\ml_http\SampleHttp.cpp

; resources
File ${PROJECTS}\ml_http\resource.h
File ${PROJECTS}\ml_http\ml_http.rc

SetOutPath $INSTDIR\ml_http\resources
File ${PROJECTS}\ml_http\resources\ti_now_playing_16x16x16.bmp