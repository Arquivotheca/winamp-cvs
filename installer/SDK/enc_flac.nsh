; FLAC encoder plugin
SetOutPath $INSTDIR\enc_flac

; project files
File ${PROJECTS}\enc_flac\enc_flac.sln
File ${PROJECTS}\enc_flac\enc_flac.vcproj

; source
File ${PROJECTS}\enc_flac\AudioCoderFlake.cpp
File ${PROJECTS}\enc_flac\AudioCoderFlake.h
File ${PROJECTS}\enc_flac\main.cpp

; resources
File ${PROJECTS}\enc_flac\enc_flac.rc
File ${PROJECTS}\enc_flac\resource.h

; flake
SetOutPath $INSTDIR\enc_flac\flake
File ${PROJECTS}\enc_flac\flake\bswap.h
File ${PROJECTS}\enc_flac\flake\common.h
File ${PROJECTS}\enc_flac\flake\Changelog
File ${PROJECTS}\enc_flac\flake\config.h
File ${PROJECTS}\enc_flac\flake\COPYING
File ${PROJECTS}\enc_flac\flake\README
SetOutPath $INSTDIR\enc_flac\flake\win32
File ${PROJECTS}\enc_flac\flake\win32\libflake.vcproj
SetOutPath $INSTDIR\enc_flac\flake\libflake
File ${PROJECTS}\enc_flac\flake\libflake\bitio.h
File ${PROJECTS}\enc_flac\flake\libflake\crc.c
File ${PROJECTS}\enc_flac\flake\libflake\crc.h
File ${PROJECTS}\enc_flac\flake\libflake\encode.c
File ${PROJECTS}\enc_flac\flake\libflake\encode.h
File ${PROJECTS}\enc_flac\flake\libflake\flake.h
File ${PROJECTS}\enc_flac\flake\libflake\lpc.c
File ${PROJECTS}\enc_flac\flake\libflake\lpc.h
File ${PROJECTS}\enc_flac\flake\libflake\md5.c
File ${PROJECTS}\enc_flac\flake\libflake\md5.h
File ${PROJECTS}\enc_flac\flake\libflake\optimize.c
File ${PROJECTS}\enc_flac\flake\libflake\optimize.h
File ${PROJECTS}\enc_flac\flake\libflake\rice.c
File ${PROJECTS}\enc_flac\flake\libflake\rice.h
File ${PROJECTS}\enc_flac\flake\libflake\vbs.c
File ${PROJECTS}\enc_flac\flake\libflake\vbs.h

