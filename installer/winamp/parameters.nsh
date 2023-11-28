!ifdef NOKIA
  !define full
  !define VERSION_SUFFIX  nokia_edition
  !define VERSION_ADDITIONALINFO "Nokia Edition"
!else ifdef lite
  !define VERSION_SUFFIX lite
  !define VERSION_ADDITIONALINFO "lite"
!else ifdef std
  !define VERSION_SUFFIX std
  !define VERSION_ADDITIONALINFO "standard"
!else ifdef pro
  !define VERSION_SUFFIX pro
  !define VERSION_ADDITIONALINFO "Pro"
  !define noaod
!else ifdef full
  !ifdef eMusic-7plus
    !ifdef bundle
      !define VERSION_SUFFIX  full_bundle_emusic-7plus
      !define VERSION_ADDITIONALINFO "full bundle"
    !else
       !define VERSION_ADDITIONALINFO "full"
       !define VERSION_SUFFIX full_emusic-7plus
    !endif
    !define eMusicBundleLetter "-7plus"
    !define eMusic
  !else ; no emusic :)
    !ifdef bundle
      !define VERSION_SUFFIX full_bundle
      !define VERSION_ADDITIONALINFO "full bundle"
    !else
      !define VERSION_SUFFIX full
      !define VERSION_ADDITIONALINFO "full"
    !endif
  !endif
!else
  ;!define GayError
  !error "CONFIGURATION ERROR: you gotta use /Dlite, /Dstd, or /Dfull"
!endif

!ifndef noaod
!define noaod ;haha
!endif

!ifndef eMusicBundleLetter
 !define eMusicBundleLetter "-7plus"
!endif

!ifdef LANGID
 !undef LANGID
!endif

!ifndef LANG_USE_EN-US
  !define LANG_USE_EN-US ; always
!endif

!ifdef LANG

  !macro WALANG_TESTLANGID LANGISO LANGDEC
 
   !if "${LANG}" == "${LANGISO}"
     !define WALANG_TESTLANGID_FOUND
   !else if "${LANG}" == "${LANGISO}-CMTY"
     !define WALANG_TESTLANGID_FOUND
   !endif
   !ifdef WALANG_TESTLANGID_FOUND
     !ifdef "LANG_USE_${LANG}" | LANG_USE_ALL
       !echo "default lang: ${LANG} (${LANGDEC})"
       !define LANGID ${LANGDEC}
     !endif
     !undef WALANG_TESTLANGID_FOUND
   !endif

  !macroend
  
  !insertmacro WALANG_TESTLANGID "EN-US" 1033
  !insertmacro WALANG_TESTLANGID "DE-DE" 1031
  !insertmacro WALANG_TESTLANGID "ES-US" 3082
  !insertmacro WALANG_TESTLANGID "FR-FR" 1036
  !insertmacro WALANG_TESTLANGID "IT-IT" 1040
  !insertmacro WALANG_TESTLANGID "NL-NL" 1043
  !insertmacro WALANG_TESTLANGID "PL-PL" 1045
  !insertmacro WALANG_TESTLANGID "SV-SE" 1053
  !insertmacro WALANG_TESTLANGID "RU-RU" 1049
  !insertmacro WALANG_TESTLANGID "ZH-TW" 1028
  !insertmacro WALANG_TESTLANGID "ZH-CN" 2052
  !insertmacro WALANG_TESTLANGID "JA-JP" 1041
  !insertmacro WALANG_TESTLANGID "KO-KR" 1042
  !insertmacro WALANG_TESTLANGID "TR-TR" 1055
  !insertmacro WALANG_TESTLANGID "PT-BR" 1046
  !insertmacro WALANG_TESTLANGID "RO-RO" 1048
  !insertmacro WALANG_TESTLANGID "HU-HU" 1038
  !insertmacro WALANG_TESTLANGID "ID-ID" 1057

!endif

!ifdef LANG_USE_ALL
  !define LANG_FILESPEC "_all"
  !define WALANG_SHOWSECTIONS
!else
  !ifdef LANG
    !define LANG_FILESPEC "_${LANG}"
  !else
    !define LANG_FILESPEC ""
  !endif
!endif
