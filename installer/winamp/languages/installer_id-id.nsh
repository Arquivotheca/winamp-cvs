﻿; Language-Country:		ID-ID
; LangId:				1057
; CodePage:				1252
; Revision:			    1
; Last udpdated:		22.04.2012
; Author:				Smankusors (Antony Kurniawan)	
; Email:				antonykurniawan95@yahoo.com.au	
; Comments: This is the Indonesian Winamp Installer

!insertmacro LANGFILE_EXT "Indonesian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Bahasa Pemasang"
${LangFileString} LANGUAGE_DLL_INFO "Mohon pilih bahasa."
 
${LangFileString} installFull "Penuh"
${LangFileString} installStandart "Standar"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "Minimal"
${LangFileString} installPrevious "Pemasangan Sebelumnya"

; BrandingText
${LangFileString} BuiltOn "dibangun pada"
${LangFileString} at "di"

${LangFileString} installWinampTop "Ini akan memasang Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsBundle "Pemasang ini berisi instalasi bungkusan penuh."
${LangFileString} installerContainsPro "Pemasang ini berisi instalasi pro."
${LangFileString} installerContainsFull "Pemasang ini berisi instalasi penuh."
${LangFileString} installerContainsStandard "Pemasang ini berisi instalasi standar."
${LangFileString} installerContainsLite "Pemasang ini berisi instalasi lite."
${LangFileString} licenseTop "Mohon baca dan menyetujui persyaratan lisensi di bawah ini sebelum memasang."
${LangFileString} directoryTop "Pemasang telah menentukan lokasi yang optimal untuk $(^NameDA). Jika Anda ingin mengubah folder, lakukan sekarang."
${LangFileString} verHistHeader "Riwayat Versi"
${LangFileString} verHistHeaderSub "Riwayat versi-versi Winamp"
${LangFileString} verHistTop "Harap tinjau perubahan versi sebelum melanjutkan:"
${LangFileString} verHistBottom "Pembaruan yang ditandai dengan merah memerlukan perhatian khusus Anda."
${LangFileString} verHistButton "Selesai"

${LangFileString} uninstallPrompt "Ini akan menghapus Winamp. Lanjutkan?"

${LangFileString} msgCancelInstall "Batal pasang?"
${LangFileString} msgReboot "Mulai ulang dibutuhkan untuk menyelesaikan pemasangan.$\r$\nMulai ulang sekarang? (Jika anda ingin mulai ulang nanti, pilih Tidak)"
${LangFileString} msgCloseWinamp "Anda harus menutup Winamp sebelum Anda bisa melanjutkan.$\r$\n$\r$\n	Setelah Anda telah menutup Winamp, pilih Coba lagi.$\r$\n$\r$\n	Jika Anda ingin mencoba untuk memasang bagaimanapun, pilih Abaikan.$\r$\n$\r$\n	Jika Anda ingin membatalkan pemasangan, pilih Batalkan."
${LangFileString} msgInstallAborted "Pemasangan dibatalkan oleh pengguna"

${LangFileString} secWinamp "Winamp (Wajib)"
${LangFileString} compAgent "Agen Winamp"
${LangFileString} compModernSkin "Dukungan Kulit Modern"
${LangFileString} whatNew "Apa yang Baru"
${LangFileString} uninstallWinamp "Hapus Winamp"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Unduh dan Pasang Format Media Windows"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Pemutaran OGG Vorbis"
${LangFileString} secOGGEnc "Pengkodean OGG Vorbis"
${LangFileString} secAACE "Pengkodean HE-AAC"
${LangFileString} secMP3E "Pengkodean MP3"
${LangFileString} secMP4E "Dukungan MP4"
${LangFileString} secWMAE "Pengkodean WMA"
${LangFileString} msgWMAError "Ada masalah memasang komponen. Pengkodean WMA tidak akan dipasang. Mohon kunjungi http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , unduh pengkodean dan coba lagi."
${LangFileString} secCDDA "Pemutaran dan extraksi CD"
${LangFileString} secSonicBurning "Dukungan Perobekan/Pembakaran Sonic"
${LangFileString} msgCDError "Ada masalah memasang komponen. Perobekan/Pembakaran CD mungkin tidak berfungsi dengan baik."
${LangFileString} secCDDB "CDDB untuk mengenali CD"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"
${LangFileString} secDVD "Pemutaran DVD (Pembaca Kode DVD pihak ke-3 diperlukan)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "Ikon eMusic dan penawaran untuk 50 MP3 gratis!"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "Ikon AOL Music Now dan penawaran 30 hari percobaan gratis!"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Dapatkan perlindungan AV Gratis dengan Active Virus Shield!"
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Dapatkan Active Security Monitor - GRATIS!"
!endif

${LangFileString} secDSP "Plug-in Studio Prosesor Sinyal"
${LangFileString} secWriteWAV "Penulis sekolah tua WAV"
${LangFileString} secLineInput "Dukungan Baris Masukkan"
${LangFileString} secDirectSound "Dukungan Keluaran DirectSound"

${LangFileString} secHotKey "Dukungan tombol pintas umum"
${LangFileString} secJmp "Dukungan Lompat ke Berkas Diperlebar"
${LangFileString} secTray "Kontrol Baki Nullsoft"

${LangFileString} msgRemoveMJUICE "Hapus dukungan MJuice dari sistem Anda?$\r$\n$\r$\nPilih YA kecuali jika Anda menggunakan berkas MJF pada program lainnya selain Winamp."
${LangFileString} msgNotAllFiles "Tidak semua berkas dihapus.$\r$\nJika Anda ingin menghapus berkas oleh Anda sendiri, silakan melakukannya."


${LangFileString} secNSV "Video Nullsoft (NSV)"
${LangFileString} secDSHOW "Format-format DirectShow (MPG, M2V)"
${LangFileString} secAVI "Video AVI"
${LangFileString} secFLV "Video Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "Video MPEG-4 (MP4, M4V)"

${LangFileString} secSWF "Protokol Media Flash (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Layar Penuh Kecil"
${LangFileString} secAVS "Studio Visualisasi Lanjutan"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Perpustakaan Media Winamp"
${LangFileString} secOM "Media Online"
${LangFileString} secOrb "Media Jarak Jauh"
${LangFileString} secWire "Direktori Podcast"
${LangFileString} secMagic "MusicIP Mix"
${LangFileString} secPmp "Pemutar Media Portabel"
${LangFileString} secPmpIpod "Dukungan iPod®"
${LangFileString} secPmpCreative "Dukungan untuk pemutar Creative®"
${LangFileString} secPmpP4S "Dukungan untuk Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Dukungan Perangkat USB"
${LangFileString} secPmpActiveSync "Dukungan untuk Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Dukungan perangkat Android"
${LangFileString} secPmpWifi "Dukungan WiFi Android"

${LangFileString} sec_ML_LOCAL "Media Lokal"
${LangFileString} sec_ML_PLAYLISTS "Daftar putar"
${LangFileString} sec_ML_DISC "Robek & Bakar CD"
${LangFileString} sec_ML_BOOKMARKS "Penanda"
${LangFileString} sec_ML_HISTORY "Riwayat"
${LangFileString} sec_ML_NOWPLAYING "Sekarang Bermain"
${LangFileString} sec_ML_RG "Alat Analisis Laba Pemutaran"
${LangFileString} sec_ML_DASH "Panel Winamp"
${LangFileString} sec_ML_TRANSCODE "Alat Transkoding"
${LangFileString} sec_ML_PLG "Pembuat Daftar Putar"
${LangFileString} sec_ML_IMPEX "Alat Impor/Ekspor Basis Data"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Pemasang $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Mohon memilih bahasa pemasang"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Mesin Multimedia"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Keluaran"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Pemutaran Audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Pengkodean Audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Pemutaran Video"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualisasi"
${LangFileString} IDS_GRP_UIEXTENSION		"Ekstensi Antarmuka Pengguna"
${LangFileString} IDS_GRP_WALIB				"Perpustakaan Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Komponen Inti Perpustakaan Media"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Dukungan Pemutar Media Portabel"
${LangFileString} IDS_GRP_LANGUAGES 	    "Bahasa"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Keluaran WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Pengkodean FLAC"
${LangFileString} IDS_SEC_MILKDROP2 	"Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Penanda Otomatis"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (alpha)"
;${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Add-on Winamp"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Unduh Bundel MP3"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Mengkonfigurasi layanan online..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Memeriksa apakah instansi lain dari Winamp berjalan..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Membuka koneksi internet..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Mengecek apakah internet tersedia..."
${LangFileString} IDS_RUN_NOINET				"Tidak ada koneksi internet"
${LangFileString} IDS_RUN_EXTRACT				"Mengekstrak"
${LangFileString} IDS_RUN_DOWNLOAD				"Mengunduh"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Selesai mengunduh."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Gagal mengunduh. Alasan :"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Pengunduhan dibatalkan."
${LangFileString} IDS_RUN_INSTALL				"Memasang"
${LangFileString} IDS_RUN_INSTALLFIALED			"Pemasangan gagal."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Berkas tidak ditemukan. Menjadwalkan unduhan."
${LangFileString} IDS_RUN_DONE					"Selesai."
${LangFileString} IDS_RUN_OPTIMIZING			"Mengoptimalkan..."

${LangFileString} IDS_DSP_PRESETS 	"Preset SPS"
${LangFileString} IDS_DEFAULT_SKIN	"kulit-kulit bawaan"
${LangFileString} IDS_AVS_PRESETS	"Preset AVS"
${LangFileString} IDS_MILK_PRESETS	"Preset MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"Preset MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Membersihkan plugin..."
${LangFileString} IDS_REMOVE_SKINS		"Menghapus kulit bawaan..."


; download
${LangFileString} IDS_DOWNLOADING	"Mengunduh..."
${LangFileString} IDS_CONNECTING	"Menghubungi..."
${LangFileString} IDS_SECOND		" (1 detik tersisa)"
${LangFileString} IDS_MINUTE		" (1 menit tersisa)"
${LangFileString} IDS_HOUR			" (1 jam tersisa)"
${LangFileString} IDS_SECONDS		" (%u detik tersisa)"
${LangFileString} IDS_MINUTES		" (%u menit tersisa)"
${LangFileString} IDS_HOURS			" (%u jam tersisa)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) dari %skB @ %u.%01ukB/detik"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Putar"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Pemasangan Selesai"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) telah terpasang di komputer Anda.$\r$\n$\r$\n\
													Klik Selesai untuk menutup panduan ini."
${LangFileString} IDS_PAGE_FINISH_RUN		"Jalankan $(^NameDA) setelah pemasang ditutup"
${LangFileString} IDS_PAGE_FINISH_LINK		"Klik disini untuk mengunjungi Winamp.com"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Tambah bundel MP3 ke daftar putar Winamp"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Selamat datang di pemasang $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) memungkinkan Anda untuk mendengarkan, melihat dan mengelola musik, video, podcast, dan radio internet.$\r$\n$\r$\n$\r$\nFitur termasuk :$\r$\n$\r$\n  \
													•  Sikronisasi media dengan nirkabel ke $(^NameDA) untuk$\r$\naplikasi Android$\r$\n$\r$\n  \
													•  Kontrol pemutaran di penjelajah dengan Toolbar Winamp$\r$\n$\r$\n  \
													•  Membersihkan metadata media dengan fitur Penanda$\r$\nOtomatis$\r$\n$\r$\n  \
													•  Membuat daftar putar menggunakan Pembuat Daftar$\r$\nPutar Otomatis$\r$\n$\r$\n  \
													•  Dengarkan dan berlangganan lebih dari 30.000 podcast"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"CATATAN : Untuk menikmati fitur baru dan \
															desain dari kulit Bento (direkomendasikan), semua \
															komponen harus dicentang."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Pilih Opsi Mulai"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Pilih dari opsi mulai berikut."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Pilih dari opsi berikut untuk mengkonfigurasi opsi mulai Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Buat entri Menu mulai"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Buat ikon Mulai Cepat"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Buat ikon Desktop"

; bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"Dapatkan hasil maksimal dari $(^NameDA)"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"Pilih firur tambahan dibawah ini untuk mendapatkan hasil maksimal dari $(^NameDA)."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Dapatkan pengalaman $(^NameDA) terbaik dengan fitur tambahan ini."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"ukuran unduhan:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"sudah terpasang"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"hanya Windows XP atau yang lebih baru"
${LangFileString} IDS_PAGE_BUNDLE_MB		"MB"
${LangFileString} IDS_PAGE_BUNDLE_KB		"KB"

; bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote" ; keep for uninstaller
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Nikmati musik dan video di mana saja. Mengalirkan musik rumah Anda dari Winamp.com, \
;                                            kompatibel dengan perangkat selular, Nintendo® Wii™, Sony® PlayStation® 3, dan Microsoft® Xbox™ 360."
${LangFileString} IDS_BUNDLE2_NAME			"Toolbar Winamp®"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Kontrol Winamp langsung dari penjelajah web Anda dan dapatkan akses cepat \
													ke SHOUTcast."
${LangFileString} IDS_BUNDLE21_NAME			"Atur Pencarian AOL® sebagai mesin pencari bawaan"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Ijinkan Winamp bantu Anda mencari web dengan Pencarian Winamp."
${LangFileString} IDS_WINAMP_SEARCH			"Pencarian Winamp"

${LangFileString} IDS_BUNDLE3_NAME			"Gratis 50 Unduhan MP3 +1 Buku audio gratis dari eMusic"
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Mulai percobaan 2 minggu eMusic gratis dan nikmati 50 unduhan musik MP3 secara gratis \ ditambah 1 buku audio gratis. Unduhan musik dan buku audio adalah milik Anda, bahkan jika Anda membatalkan."

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Tidak dapat menutup Agen Winamp.$\r$\n\
                                                   Pastikan bahwa pengguna lain tidak masuk ke Windows.\
                                                   $\r$\n$\r$\n	Setelah Anda menutup Agen Winamp, pilih Coba lagi.\
                                                   $\r$\n$\r$\n	Jika Anda ingin mencoba untuk menginstal bagaimanapun, pilih Abaikan.\
                                                   $\r$\n$\r$\n	Jika Anda ingin membatalkan pemasangan, pilih Batalkan."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Versi Windows ini tidak lagi didukung.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} membutuhkan minimal Windows 2000 atau yang lebih baru."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Plugin gen_msn7.dll pihak ke-3 yang tidak kompatibel terdeteksi!$\r$\n$\r$\nPlugin ini menyebabkan Winamp 5.23 dan selanjutnya crash saat memuat.$\r$\nPlugin sekarang akan dinonaktifkan. Klik OK untuk diproseskan."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Plugin gen_lyrics.dll pihak ke-3 yang tidak kompatibel terdeteksi!$\r$\n$\r$\nPlugin ini menyebabkan Winamp 5.59 dan selanjutnya crash saat memuat.$\r$\nPlugin sekarang akan dinonaktifkan. Klik OK untuk diproseskan."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Plugin gen_lyrics pihak ke-3 terdeteksi!$\r$\n$\r$\nVersi tua dari plugin ini tidak kompatibel dengan Winamp 5.6 dan yang lebih baru. Pastikan Anda punya versi terbaru dari http://lyricsplugin.com sebelum melanjutkan."

;Winamp3
${LangFileString} msgWA3			"Winamp3 terdeteksi..."
${LangFileString} msgWA3_UPGRADE		"Winamp3 terdeteksi. Meningkatkan pemasangan Winamp3 untuk Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} dan pindahkan kulit untuk Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}?$\r$\n(Memilih Ya akan meningkatkan dan dianjurkan, memilih Tidak akan membiarkan Anda tetap terpisah Winamp3 dan pemasangan Winamp 5.x)"
${LangFileString} msgWA3_MIGRATE		"Memindahkan Kulit..."
${LangFileString} msgWA3_REMOVE			"Menghapus pemasangan Winamp3..."
${LangFileString} msgWA3_REMOVE2		"Berkas-berkas di '$0' tidak semuanya dihapus.$\r$\nJika Anda ingin menghapus berkas-berkas direktori ini, Anda dapat secara manual menghapusnya."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Terdeteksi ${DIRECTXINSTAL_WINVER_LO} atau yang lebih rendah."
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Terdeteksi ${DIRECTXINSTAL_WINVER_HI} atau yang lebih tinggi"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Mengecek versi ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Dibutuhkan minimal versi ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Tidak dapat mendeteksi versi ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Terdeteksi versi ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Versi ${DIRECTXINSTAL_DIRECTXNAME} tidak mendukung"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Mengecek apakah $0 ada"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Unduhan diperlukan"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Mengecek koneksi internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Versi terakhir dari ${DIRECTXINSTAL_DIRECTXNAME} tersedia di:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Mengunduh pemasang ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Ditemukan"
${LangFileString} IDS_DIRECTX_MISSING					"Hilang"
${LangFileString} IDS_DIRECTX_SUCCESS					"Sukses"
${LangFileString} IDS_DIRECTX_ABORTED					"Dibatalkan"
${LangFileString} IDS_DIRECTX_FAILED					"Gagal"
${LangFileString} IDS_DIRECTX_DONE						"Selesai"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Menjalankan pemasang ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} membutuhkan setidaknya ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} untuk beroperasi dengan baik.$\r$\nPasang itu sekarang?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} membutuhkan setidaknya ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} untuk berfungsi dengan baik"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Tidak dapat mengunduh ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Tidak dapat memasang ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Komputer Anda kehilangan komponen ${DIRECTXINSTAL_DIRECTXNAME} yang diperlukan oleh ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Tidak dapat mengunduh komponen ${DIRECTXINSTAL_DIRECTXNAME} yang hilang"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Tidak dapat memasang komponen ${DIRECTXINSTAL_DIRECTXNAME} yang hilang"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Inti Winamp (wajib)"
${LangFileString} IDS_SEC_AGENT_DESC			"Agen Winamp, untuk akses cepat baki sistem dan mempertahakan asosiasi jenis berkas"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Mesin Multimedia (Sistem masukkan/keluaran)"
${LangFileString} IDS_SEC_CDDB_DESC				"Dukungan CDDB, untuk secara otomatis mengambil judul CD dari basis data Gracenote secara online"
${LangFileString} IDS_SEC_SONIC_DESC			"Perpustakaan Sonic, dibutuhkan untuk merobek && Pembakaran CD Audio"
${LangFileString} IDS_SEC_DSP_DESC				"Plugin DSP, untuk menerapkan efek tambahan seperti kontrol chorus, flanger, tempo, dan pitch"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Dukungan Pemutaran Audio (Plugin Masukkan : Pembaca Kode Audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Dukungan pemutaran untuk format MP3, MP2, MP1, AAC (wajib)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Dukungan pemutaran untuk format WMA (termasuk dukungan DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Dukungan pemutaran untuk format MIDI (MID, RMI, KAR, MUS, CMF, dan banyak lagi)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Dukungan pemutaran untuk format Modul (MOD, XM, IT, S3M, ULT, dan banyak lagi)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Dukungan pemuatan untuk format Ogg Vorbis (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Dukungan pemutaran untuk format Audio MPEG-4 (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Dukungan pemutaran untuk format FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Dukungan pemutaran untuk CD Audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Pemutaran dukungan untuk format Bentuk gelombang (WAV, VOC, AU, AIFF, dan banyak lagi)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Dukungan Pemutaran Video (Plugin Masukkan : Pembaca Kode Video)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Dukungan pemutaran untuk format video Media Windows (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Dukungan pemutaran untuk format video Nullsoft (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Dukungan pemutaran untuk format video MPEG-1/2 dan lainnya"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Dukungan pemutaran untuk Video AVI"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Dukungan pemutaran untuk Video Flash (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Dukungan pemutaran untuk Video Matroska (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Dukungan pemutaran untuk Video MPEG-4 (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Dukungan pemutaran untuk format Adobe Flash Streaming (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Dukungan Pengkodean dan Transkoding (dibutuhkan untuk merobek CD dan mengkonversi format berkas)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format WAV"
${LangFileString} IDS_SEC_MP3_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format MP3"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format M4A dan AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Dukungan untuk merobek dan transkoding ke format Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Plugin Keluaran (yang mengontrol bagaimana audio diproses dan dikirim ke kartu audio Anda)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Penulis Sekolah tua WAV/MME (usang, namun beberapa pengguna masih lebih suka untuk menggunakannya)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Keluaran DirectSound (plugin keluaran wajib / bawaan)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Keluaran Sekolah tua WaveOut (opsional, dan tidak lagi direkomendasikan atau diperlukan)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Ekstensi Antarmuka Pengguna"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Plugin Tombol Pintas Umum, untuk mengendalikan Winamp dengan keyboard ketika aplikasi lain dalam fokus"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Dukungan Lompat ke Berkas Diperluas, untuk mengantri lagu dalam daftar putar, dan masih banyak lagi"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin Kontrol Baki Nullsoft, untuk menambahkan ikon kontrol Putar dalam baki sistem"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Dukungan Kulit Modern, diperlukan untuk menggunakan kulit bentuk yang unik seperti Winamp Modern dan Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Plugin Visualisasi"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin visualisasi layar penuh kecil Nullsoft"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin Studio Visualisasi Lanjutan"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin visualisasi Milkdrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Plugin visualisasi Milkdrop2 (plugin visualisasi bawaan)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Dukungan Baris Masukkan menggunakan perintah linein:// (berlaku visualisator untuk mic/baris masukkan)"
${LangFileString} IDS_GRP_WALIB_DESC			"Perpustakaan Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Perpustakaan Media Winamp (wajib)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Alat Transkoding, digunakan untuk mengkonversi dari satu format berkas menjadi lainnya"
${LangFileString} IDS_SEC_ML_RG_DESC			"Alat Analisis Laba Putar Ulang, digunakan untuk meratakan volume"
${LangFileString} IDS_SEC_ML_DASH_DESC			"Panel, halaman rumah portal opsional untuk Perpustakaan Media"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Otomatis Penanda Winamp (Didukung oleh Gracenote), untuk mengisi metadata yang hilang"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Direktori Podcast, untuk berlangganan dan mengunduh podcast"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Layanan Online, termasuk SHOUTcast Radio && TV, AOL Radio feat. CBS Radio, Winamp Charts, dan banyak lagi"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Pembuat Daftar Putar Winamp (didukung oleh Gracenote), untuk membuat daftar putar secara akustik"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Komponen Inti Perpustakaan Media"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Basis data Media Lokal, dengan sistem kueri yang kuat dan pandangan cerdas kustom"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Pengelola Daftar Putar, untuk membuat, mengedit dan menyimpan semua daftar putar Anda"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Robek && Bakar CD, antarmuka perpustakaan media untuk merobek && membakar CD Audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Pengelola Penanda, untuk menandai aliran, berkas, atau folder favorit Anda"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Riwayat, untuk akses cepat ke semua berkas lokal atau jauh dan aliran yang terakhir diputar"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Sekarang Bermain, untuk melihat informasi tentang trek yang sedang bermain"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Dukungan Pemutar Media Portabel"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plugin Dukungan Inti Pemutar Media Portabel (wajib)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Dukungan iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Dukungan untuk portabel Creative® (untuk mengelola pemutar Nomad™, Zen™ dan MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Dukungan untuk Microsoft® PlaysForSure® (untuk mengelola semua pemutar P4S yang kompatibel)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Dukungan Perangkat USB (untuk mengelola jempol kandar usb generik dan pemutar)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Dukungan Microsoft® ActiveSync® (untuk mengelola perangkat Windows Mobile®, Smartphone && PC Saku)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Dukungan untuk perangkat Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Dukungan WiFi Android"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Versi alpha dari plugin Dropbox akan segera datang. Gunakan ctrl-shift-d untuk mengaktifkan"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Plugin iTunes yang kompatibel dengan basis data impor/ekspor Perpustakaan Media"
;${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Winamp Orgler™ memungkinkan Anda melacak, memetakan & berbagi riwayat mendengarkan Anda"
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Temukan dan tambahkan ekstensi untuk pemutar media Winamp Anda dengan Pengaya"
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Unduh dan pasang musik gratis dari Musik Winamp"

${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Pilih program terkait untuk menghapus."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Hapus $(^NameDA) dari komputer Anda."

;${LangFileString} IDS_UNINSTALL_COMPONENTS_HEADER		"Program tersebut yang terkait dengan $(^NameDA) berikut akan dihapus. Untuk mengubah, silakan hapus centang pada kotak yang sesuai:"
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Jalur hapus:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Pemutar Media"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Hpus semua komponen Pemutar Media $(^NameDA) termasuk paket plug-in pihak ketiga."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Preferensi Pengguna"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Hapus semua preferensi dan plug-in $(^NameDA)."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"Program tersebut yang terkait dengan Winamp dalam penghapusan ini:"
; ${LangFileString} IDS_UNINSTALL_BUNDLES_FOOTER		"Catatan : Anda dapat menghapus program tersebut setiap saat melalui Panel Kontrol Windows."
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Kontrol $(^NameDA) langsung dari penjelajah web Anda dan dapatkan akses cepat ke SHOUTcast."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Nikmati musik dan video di mana saja."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Plug-in Detektor Winamp untuk Mozilla Firefox dan Internet Explorer."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"Unduhan Music, Unduhan MP3, lagu MP3 dari eMusic.com"
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"Menghapus $2..."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Bantu $(^NameDA) dengan mengirimkan umpan balik"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Buka folder $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nCatatan : Tidak semua berkas telah dihapus dari penghapusan ini. Untuk melihat, buka folder Winamp."