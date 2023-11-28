; WA_SDK.nsi

; This script will collect the files in Winamp SDK and create an installer for them

;------------------------

!ifndef VERSION
  !define VERSION '5.55'
!endif

; This is where all projects live.  Ensure this is the correct relative path.  
!ifndef PROJECTS
  !define PROJECTS '..\..'
!endif

;------------------------

Name "Winamp ${VERSION} SDK"
OutFile "WA${VERSION}_SDK.exe"
InstallDir "$PROGRAMFILES\Winamp SDK"
Page directory
Page instfiles
Section ""

; APIs
!include "winamp_api.nsh"
!include "ml_api.nsh"
!include "wasabi.nsh"
!include "bfc.nsh"
!include "xml.nsh"
!include "playlist.nsh"
!include "nu.nsh"
!include "Agave.nsh"
!include "nsv.nsh"
!include "burner.nsh"

; examples
!include "ml_http.nsh"
!include "ml_xmlex.nsh"
!include "plLoadEx.nsh"
!include "dsp_test.nsh"
!include "gen_tray.nsh"
!include "in_raw.nsh"
!include "in_tone.nsh"
!include "coverdirectory.nsh"
!include "irctell.nsh"
!include "enc_flac.nsh"
!include "ml_iso.nsh"
!include "in_chain.nsh"
!include "xspf.nsh"
!include "mlExplorer.nsh"
!include "out_null.nsh"
!include "gen_classicart.nsh"

; skinning
!include "maki.nsh"

; open source
!include "ReplayGainAnalysis.nsh"
!include "nde.nsh"

; TODO
; example using api_tagz
; example using hotkeys
; example using api_decodefile
; vis_avs
; jnetlib
; file reader API
; example using api_random (maybe by adding noise generator to dsp_test)

!ifdef old_stuff_for_reference

SetOutPath $INSTDIR\gen_ml
File ${PROJECTS}\gen_ml\gaystring.h
File ${PROJECTS}\gen_ml\gaystring.cpp
File ${PROJECTS}\gen_ml\itemlist.cpp
File ${PROJECTS}\gen_ml\itemlist.h
File ${PROJECTS}\gen_ml\listview.cpp
File ${PROJECTS}\gen_ml\listview.h
File ${PROJECTS}\gen_ml\ml_ipc.h
File ${PROJECTS}\gen_ml\ml_lib.cpp

SetOutPath $INSTDIR\vis
File .\readme\wa5vis.txt
SetOutPath $INSTDIR\vis\vis_avs\apesdk
File /x CVS ${PROJECTS}\vis_avs\apesdk\*.*
SetOutPath $INSTDIR\vis\vis_avs\ns-eel
File /x CVS ${PROJECTS}\ns-eel\*.*
SetOutPath $INSTDIR\vis\vis_test
File ${PROJECTS}\vis_milkdrop\svis.mak

SetOutPath $INSTDIR\Winamp
File ${PROJECTS}\gen_hotkeys\wa_hotkeys.h
File ${PROJECTS}\Winamp\api_random.h
File ${PROJECTS}\Winamp\api_decodefile.h
File ${PROJECTS}\Winamp\api_audiostream.h
File ${PROJECTS}\Winamp\api_albumart.h

SetOutPath $INSTDIR\playlist
File ${PROJECTS}\playlist\api_playlist.h
File ${PROJECTS}\playlist\api_playlistdirectorycallback.h
File ${PROJECTS}\playlist\api_playlistentry.h

SetOutPath $INSTDIR\tagz
File ${PROJECTS}\tagz\api_tagz.h
File ${PROJECTS}\tagz\ifc_tagprovider.h
File ${PROJECTS}\tagz\ifc_tagparams.h

File /oname=$INSTDIR\readme.txt ${PROJECTS}\Resources\SDK\sdkreadme.txt
!endif

SectionEnd