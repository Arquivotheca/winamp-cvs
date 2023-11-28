; Sample DSP Plugin
SetOutPath $INSTDIR\dsp_test

; project files
File ${PROJECTS}\dsp_test\dsp_test.sln
File ${PROJECTS}\dsp_test\dsp_test.vcproj

; source
File ${PROJECTS}\dsp_test\dsp_test.c


; documentation
File ${PROJECTS}\dsp_test\dsp_ns.txt

; resources
File ${PROJECTS}\dsp_test\RESOURCE.H
File ${PROJECTS}\dsp_test\SCRIPT1.RC
