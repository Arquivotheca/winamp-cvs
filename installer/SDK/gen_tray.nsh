; System Tray plugin
SetOutPath $INSTDIR\gen_tray

; project files
File ${PROJECTS}\gen_tray\GEN_TRAY.sln
File ${PROJECTS}\gen_tray\GEN_TRAY.vcproj

; source
File ${PROJECTS}\gen_tray\TRAYCTL.C
File ${PROJECTS}\gen_tray\WINAMPCMD.H
File ${PROJECTS}\gen_tray\api.h

; icons
File ${PROJECTS}\gen_tray\ICON1.ICO
File ${PROJECTS}\gen_tray\ICON2.ICO
File ${PROJECTS}\gen_tray\ICON3.ICO
File ${PROJECTS}\gen_tray\ICON4.ICO
File ${PROJECTS}\gen_tray\ICON5.ICO
SetOutPath $INSTDIR\gen_tray\icons
File ${PROJECTS}\gen_tray\icons\compact.bmp
File ${PROJECTS}\gen_tray\icons\icon1.ico
File ${PROJECTS}\gen_tray\icons\icon2.ico
File ${PROJECTS}\gen_tray\icons\icon3.ico
File ${PROJECTS}\gen_tray\icons\icon4.ico
File ${PROJECTS}\gen_tray\icons\icon5.ico
File ${PROJECTS}\gen_tray\icons\icon7.ico
File ${PROJECTS}\gen_tray\icons\icon8.ico
File ${PROJECTS}\gen_tray\icons\icon9.ico
SetOutPath $INSTDIR\gen_tray

; resources
File ${PROJECTS}\gen_tray\RESOURCE.H
File ${PROJECTS}\gen_tray\SCRIPT1.RC
