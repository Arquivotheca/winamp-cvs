!include "sectionLogic.nsh"

; Rotation config file
;
; To add section set parameters and then call !insertmacro ROTATION_SECTION_ADD
; Section parameters:
; 	ROTATION_ARTIST		artist_name			[required]
; 	ROTATION_ALBUM		album_name			[required]
; 	ROTATION_TITLE		title				[required]
; 	ROTATION_LENGTH_SEC	length_in_seconds	[required]			total song legth in seconds.
; 	ROTATION_COMMENT	comments			[optional]			comments will be visible in the components page description area 
;																when mouse hovered over section.
; 	ROTATION_FILE		source_file_path	[required]			path to the source file (can be local file or url).
; 	ROTATION_FILESIZEKB	file_size_kb		[optional]			if source file is not local specify size in kilobytes to help 
;																instaler show correct required space.
; 	ROTATION_ART		source_alumart_jpg_path		[optional]	path to the album art file on local machine or url (must be jpg).
; 	ROTATION_ARTSIZEKB	album_art_jpg_size_kb		[optional]	if album art is not local specify size in kilobytes to help
;																instaler show correct required space.
; Example:
;	!define ROTATION_ARTIST 		"Ночные Снайперы"
;	!define ROTATION_ALBUM 			"Рубеж"
;	!define ROTATION_TITLE 			"31-я весна"
;	!define ROTATION_LENGTH_SEC		241
;	!define ROTATION_COMMENT		"Non latin alpahbet test (downloadable section)"
;	!define ROTATION_FILE 			"http://dl.dropbox.com/u/1994752/Rotation%20Test/song3.mp3"
;	!define ROTATION_FILESIZEKB		3768
;	!define ROTATION_ART			"http://dl.dropbox.com/u/1994752/Rotation%20Test/song3.jpg"
;	!define ROTATION_ARTSIZEKB		68
;	!insertmacro ROTATION_SECTION_ADD
;

!define ROTATION_ARTIST 	"Blind Pilot"
!define ROTATION_ALBUM 		"We Are the Tide"
!define ROTATION_TITLE 		"Keep You Right"
!define ROTATION_LENGTH_SEC	215
!define ROTATION_COMMENT	"Free MP3"
!define ROTATION_FILE 		"http://download.nullsoft.com/winamp/client/bundles/blind-pilot-keep-you-right_2011-09-12-131753-4137-0-0-0.128.mp3"
!define ROTATION_FILESIZEKB	3386
!insertmacro ROTATION_SECTION_ADD

!define ROTATION_ARTIST 	"Real Estate"
!define ROTATION_ALBUM 		"Days"
!define ROTATION_TITLE 		"It's Real"
!define ROTATION_LENGTH_SEC 	168
!define ROTATION_COMMENT	"Free MP3"
!define ROTATION_FILE 		"http://download.nullsoft.com/winamp/client/bundles/real-estate-its-real_2011-08-03-132907-4137-0-0-0.128.mp3"
!define ROTATION_FILESIZEKB	2654
!insertmacro ROTATION_SECTION_ADD

!define ROTATION_ARTIST 	"Zee Avi"
!define ROTATION_ALBUM 		"Ghostbird"
!define ROTATION_TITLE 		"Swell Window"
!define ROTATION_LENGTH_SEC	231
!define ROTATION_COMMENT	"Free MP3"
!define ROTATION_FILE 		"http://download.nullsoft.com/winamp/client/bundles/zee-avi-swell-window_2011-08-03-155149-4137-0-0-0.128.mp3"
!define ROTATION_FILESIZEKB	3642	
!insertmacro ROTATION_SECTION_ADD

!define ROTATION_ARTIST 	"Future Islands"
!define ROTATION_ALBUM 		"On the Water"
!define ROTATION_TITLE 		"Balance"
!define ROTATION_LENGTH_SEC	246
!define ROTATION_COMMENT	"Free MP3"
!define ROTATION_FILE 		"http://download.nullsoft.com/winamp/client/bundles/future-islands-balance_2011-09-07-174111-4137-0-0-0.128.mp3"
!define ROTATION_FILESIZEKB	3872	
!insertmacro ROTATION_SECTION_ADD

!define ROTATION_ARTIST 	"Regina"
!define ROTATION_ALBUM 		"Soite Mulle"
!define ROTATION_TITLE 		"Unessa"
!define ROTATION_LENGTH_SEC	238
!define ROTATION_COMMENT	"Free MP3"
!define ROTATION_FILE 		"http://download.nullsoft.com/winamp/client/bundles/regina-unessa_2011-09-27-111321-4137-0-0-0.128.mp3"
!define ROTATION_FILESIZEKB	3740
!insertmacro ROTATION_SECTION_ADD
