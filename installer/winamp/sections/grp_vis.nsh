SectionGroup $(IDS_GRP_VISUALIZATION) IDX_GRP_VISUALIZATION ;  Visualization

  !ifndef WINAMP64
   ${WinampSection} "visTiny" $(secTiny) IDX_SEC_NSFS        ; >>> [Nullsoft Tiny Fullscreen]
     ${SECTIONIN_LITE}
     Call SetVisPluginDir
     File ${FILES_PATH}\plugins\vis_nsfs.dll
   ${WinampSectionEnd}                                       ; <<< [Nullsoft Tiny Fullscreen]
  !endif ; WINAMP64


  !ifdef full
   !ifndef WINAMP64
    ${WinampSection} "visAVS" $(secAVS) IDX_SEC_AVS          ; >>> [Advanced Visualization Studio]
      ${SECTIONIN_FULL}
      Call SetVisPluginDir

      ${If} $WinVer == "2000"
	  ${OrIf} $WinVer == "XP"
	  ${OrIf} $WinVer == "2003"
      File ..\..\resources\plugins\vis_avs.dll
	  ${Else}
	  File /oname=vis_avs.dll "..\..\resources\plugins\vis_avs2.dll"
	  ${EndIf}
;     File ${FILES_PATH}\plugins\vis_avs.dll
      SetOverwrite off
      File ..\..\resources\data\vis_avs.dat
      SetOverwrite ${OVERWRITEMODE}

      DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_AVS_PRESETS)..."
      SetDetailsPrint none
     
      SetOutPath $OUTDIR\avs
      File ..\..\resources\data\avs\*.ape
      File ..\..\resources\data\avs\*.bmp

      SetOutPath "$OUTDIR\Winamp 5 Picks"
      Delete "$OUTDIR\fçk - checkers with metaballs (skupers remix).avs" ;fix for fucked up char
      File "..\..\resources\data\avs\Winamp 5 Picks\*.avs"

      SetOutPath "$OUTDIR\..\Community Picks"
      File "..\..\resources\data\avs\Community Picks\*.avs"

      SetDetailsPrint lastused

      SetOutPath $INSTDIR\Plugins
    ${WinampSectionEnd}                                   ; <<< [Advanced Visualization Studio]
   !endif ; WINAMP64
  !endif ; full

  !ifdef std | full
   !ifndef WINAMP64
    ${WinampSection} "secMilk2" $(IDS_SEC_MILKDROP2) IDX_SEC_MILKDROP2      ; >>> [Milkdrop2]
      ${SECTIONIN_STD}
      Call SetVisPluginDir
      File "${FILES_PATH}\plugins\vis_milk2.dll"
;     File ..\..\resources\plugins\vis_milk2.dll
      SetOutPath $SETTINGSDIR\Plugins\Milkdrop2
      File ..\..\resources\data\milk2_img.ini
      File ..\..\resources\data\milk2_msg.ini

      DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_MILK2_PRESETS)..."
      SetDetailsPrint none

      SetOutPath $INSTDIR\Plugins\Milkdrop2
      File /a /r /x CVS "..\..\resources\data\Milkdrop2\*.*"

      SetDetailsPrint lastused

	  ClearErrors
      ReadINIStr $0 "$WINAMPINI" "Winamp" "visplugin_name"
      IfErrors 0 +3
        WriteINIStr "$WINAMPINI" "Winamp" "visplugin_name" vis_milk2.dll
        WriteINIStr "$WINAMPINI" "Winamp" "visplugin_num" 0
      SetOutPath $INSTDIR\Plugins
    ${WinampSectionEnd}                                                  ; <<< [Milkdrop2]
   !endif ; WINAMP64
  !endif ; full

  !ifdef full
   !ifndef WINAMP64
    ${WinampSection} "visLine" $(secLineInput) IDX_SEL_LINEIN           ; >>> [Line Input Support]
      ${SECTIONIN_FULL}
      SetOutPath $INSTDIR\Plugins
      File "${FILES_PATH}\plugins\in_linein.dll"
    ${WinampSectionEnd}                                                  ; <<< [Line Input Support]
   !endif ; WINAMP64
  !endif ; std | full

SectionGroupEnd ;  Visualization