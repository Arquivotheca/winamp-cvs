; media library API
SetOutPath $INSTDIR\gen_ml
File ${PROJECTS}\gen_ml\ml.h
File ${PROJECTS}\gen_ml\ml_ipc_0313.h
File ${PROJECTS}\gen_ml\childwnd.h

; local media API
SetOutPath $INSTDIR\ml_local
File ${PROJECTS}\ml_local\api_mldb.h

; Replay Gain API (this should probably be moved out of ml_rg eventually)
SetOutPath $INSTDIR\ml_rg
File ${PROJECTS}\ml_rg\obj_replaygain.h

; Podcast API
SetOutPath $INSTDIR\ml_wire
File ${PROJECTS}\ml_wire\api_podcasts.h
File ${PROJECTS}\ml_wire\ifc_podcast.h
File ${PROJECTS}\ml_wire\ifc_article.h