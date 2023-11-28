﻿; Language-Country:	TR-TR
; LangId:			1055
; CodePage:			1252
; Last udpdated:	16.03.2013
; Author:			Ali Sarıoğlu
; Email:			alsau@mynet.com

!insertmacro LANGFILE_EXT "Türkçe"
 
; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Kurulum dili"
${LangFileString} LANGUAGE_DLL_INFO "Lütfen bir dil seçin."

${LangFileString} installFull "Tam"
${LangFileString} installStandart "Standart"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "En Az"
${LangFileString} installPrevious "Önceki Kurulum"

; BrandingText
${LangFileString} BuiltOn " "
${LangFileString} at "-"

${LangFileString} installWinampTop "Bu, Winamp'ın ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType} sürümünü kuracak."
${LangFileString} installerContainsBundle " Bu kurulum Winamp'ın toplu sürümünü içerir."
${LangFileString} installerContainsPro " Bu kurulum Winamp'ın pro sürümünü içerir."
${LangFileString} installerContainsFull " Bu kurulum Winamp'ın tam sürümünü içerir."
${LangFileString} installerContainsStandard " Bu kurulum Winamp'ın standart sürümünü içerir."
${LangFileString} installerContainsLite " Bu kurulum Winamp'ın lite sürümünü içerir."
${LangFileString} licenseTop "Kuruluma başlamadan önce aşağıdaki lisans anlaşmasını okuyun ve kabul edin."
${LangFileString} directoryTop "Kur, Winamp'ın kurulacağı en uygun konumu belirledi. Eğer başka bir konuma kurmak isterseniz Gözat'a tıklatın ve başka bir konum seçin."
${LangFileString} verHistHeader "Sürüm Geçmişi"
${LangFileString} verHistHeaderSub "Winamp sürümleri geçmişi"
${LangFileString} verHistTop "Devam etmeden önce, lütfen sürüm değişikliklerini gözden geçirin:"
${LangFileString} verHistBottom "Güncellemeler kırmızı renkte ve koyu olarak belirtilmiştir."
${LangFileString} verHistButton "Bitti"

${LangFileString} uninstallPrompt "Winamp kaldırılacak. Devam etmek istiyor musunuz?"

${LangFileString} msgCancelInstall "Kurulumdan çık?"
${LangFileString} msgReboot "Kurulumun tamamlanması için bilgisayarınızın yeniden başlatılması gerekir.$\r$\nBilgisayarınız yeniden başlatılsın mı? (başlatma işlemini daha sonra yapmak isterseniz, Hayır'ı tıklatın)"
${LangFileString} msgCloseWinamp "Devam edebilmeniz için Winamp'ı kapatmalısınız.$\r$\n$\r$\n	Winamp'ı kapatın ve Yeniden Dene'yi tıklatın.$\r$\n$\r$\n	Bunu atlamak ve devam etmek için Yoksay'ı tıklatın.$\r$\n$\r$\n	Kurulumu iptal etmek için Durdur'u tıklatın."
${LangFileString} msgInstallAborted "Kurma işlemi kullanıcı tarafından iptal edildi"

${LangFileString} secWinamp "Winamp (gerekli)"
${LangFileString} compAgent "Winamp Ajanı"
${LangFileString} compModernSkin "Modern Dış Görünüm Desteği"
${LangFileString} whatNew "Neler Yeni"
${LangFileString} uninstallWinamp "Winamp Kaldır"


${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Windows Media Biçimini İndir ve Kur"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis Kayıttan Yürütme"
${LangFileString} secOGGEnc "OGG Vorbis dosyası oluşturma desteği"
${LangFileString} secAACE "AAC/aacPlus biçiminde dosya oluşturma desteği"
${LangFileString} secMP3E "MP3 biçiminde dosya oluşturma desteği"
${LangFileString} secMP4E "MP4 desteği"
${LangFileString} secWMAE "WMA biçiminde dosya oluşturma desteği"
${LangFileString} msgWMAError "Bileşenler kurulurken bir sorun oluştu. WMA Kodlayıcı kurulamadı. Lütfen http://www.microsoft.com/windows/windowsmedia/9series/encoder/ web sitesinden kodlayıcıyı indirin ve tekrar deneyin."
${LangFileString} secCDDA "CD yürütme ve çıkarma"
${LangFileString} secSonicBurning "Sonic CD Kopyalama/Yazma"
${LangFileString} msgCDError "Bileşenler yüklenirken bir sorun oluştu. CD Kopyalama/Yazma tam anlamıyla çalışmadı."
${LangFileString} secCDDB "CD'ler için CDDB desteği"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"
${LangFileString} secDVD "DVD Oynatma (3. parti DVD kod çözücü gerekir)"

!ifdef eMusic-7plus
  ${LangFileString} secEMusic "50 ücretsiz MP3 müzik dosyası için eMusic simgesi!"
!endif
!ifdef musicNow
  ${LangFileString} secMusicNow "AOL Music Now simgesi ve 30 günlük ücretsiz deneme için öneri!"
!endif
!ifdef BUNDLE_AVS
  ${LangFileString} secBundleAVS "Etkin Virüs Kalkan'ı ile ücretsiz AV korunmasını alın! "
!endif
!ifdef BUNDLE_ASM
  ${LangFileString} secBundleASM "Etkin Güvenlik İzleyicisi - ÜCRETSİZDİR!"
!endif

${LangFileString} secDSP "Signal Processor Studio Eklentisi"
${LangFileString} secWriteWAV "Eski tarz, WAV dosyası oluşturucu"
${LangFileString} secLineInput "Line Input Desteği"
${LangFileString} secDirectSound "DirectSound çıktı desteği"

${LangFileString} secHotKey "Genel Kısayollar Desteği"
${LangFileString} secJmp "Dosyaya Atla Eklentisi Desteği"
${LangFileString} secTray "Nullsoft Görev Çubuğu Denetim Alanı"

${LangFileString} msgRemoveMJUICE "MJuice desteği bilgisayarınızdan kaldırılsın mı?$\r$\n$\r$\nMJF dosyaları Winamp'tan başka programlarda kullanılmayacaksa EVET'i seçin"
${LangFileString} msgNotAllFiles "Tüm dosyalar kaldırılamadı.$\r$\nDosyaları kendiniz kaldırmak $\r$\nisterseniz, onları silebilirsiniz.$\r$\n(Muhtemelen Winamp'ın kurulumdan sonra $\r$\n3. parti eklentiler yüklediniz)"


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "DirectShow biçimleri (MPG, M2V)"
${LangFileString} secAVI "AVI Video"
${LangFileString} secFLV "Flash Video (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF "Flash Media Protokolü (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Mini Tam Ekran"
${LangFileString} secAVS "Gelişmiş Görsel Öğe Stüdyosu (AVS)"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp Ortam Dosyası"
${LangFileString} secOM "Çevrimiçi Ortam"
${LangFileString} secOrb "Uzak Ortam"
${LangFileString} secWire "Podcast Dizini"
${LangFileString} secMagic "MusicIP Mix"
${LangFileString} secPmp "Taşınabilir Ortam Yürütücüleri"
${LangFileString} secPmpIpod "iPod® desteği"
${LangFileString} secPmpCreative "Creative® yürütücüleri için destek"
${LangFileString} secPmpP4S "Microsoft® PlaysForSure® için destek"
${LangFileString} secPmpUSB "USB Aygıtları Desteği"
${LangFileString} secPmpActiveSync "Microsoft® ActiveSync® için destek"
${LangFileString} secPmpAndroid "Android aygıt deesteği"
${LangFileString} secPmpWifi "Android Wifi desteği"

${LangFileString} sec_ML_LOCAL "Yerel Ortam"
${LangFileString} sec_ML_PLAYLISTS "Çalma Listeleri"
${LangFileString} sec_ML_DISC "CD Kopyala ve Yaz"
${LangFileString} sec_ML_BOOKMARKS "Yer İmleri"
${LangFileString} sec_ML_HISTORY "Geçmiş"
${LangFileString} sec_ML_NOWPLAYING "Şimdi Yürütülüyor"
${LangFileString} sec_ML_RG "Replay Gain Analiz Aracı"
${LangFileString} sec_ML_DASH "Winamp Gösterge"
${LangFileString} sec_ML_TRANSCODE "Dosya Dönüştürme Aracı"
${LangFileString} sec_ML_PLG "Çalma Listesi Oluşturucu"
${LangFileString} sec_ML_IMPEX "Veritabanı Al/Ver Aracı"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) Kur"
${LangFileString} IDS_SELECT_LANGUAGE  "Lütfen kurulum dilini seçin"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Çoklu Ortam Motoru"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Çıktı"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Ses Kayıttan Yürütme"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Ses Dosyası Oluşturma Bileşenleri"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Video Kayıttan Yürütme"
${LangFileString} IDS_GRP_VISUALIZATION		"Görsel Öğe"
${LangFileString} IDS_GRP_UIEXTENSION		"Kullanıcı Arayüzü Bileşenleri"
${LangFileString} IDS_GRP_WALIB				"Winamp Kitaplığı"
${LangFileString} IDS_GRP_WALIB_CORE		"Ortam Kitaplığı Temel Bileşenleri"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Taşınabilir Ortam Yürütücüsü Desteği"
${LangFileString} IDS_GRP_LANGUAGES 	    "Diller"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME Çıktı"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC dosya oluşturma desteği"
${LangFileString} IDS_SEC_MILKDROP2     "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Otomatik Etiketleyici"
${LangFileString} IDS_SEC_GEN_DROPBOX	"Winamp DropBox (alfa)"
;${LangFileString} IDS_SEC_ML_ORGLER		"Winamp Orgler™"
${LangFileString} IDS_SEC_ML_ADDONS		"Winamp Eklenti Tarayıcısı"
${LangFileString} IDS_SEC_MUSIC_BUNDLE	"Winamp Ücretsiz Müzik Demeti"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "French Radio Eklentisi"
${LangFileString} IDS_SEC_CLOUD       "Winamp Cloud"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Çevrimiçi Servisler yapılandırılıyor..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Çalışan başka bir Winamp kopyası veya bir bileşeni olup olmadığı denetleniyor..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Internet bağlantısı için açılıyor..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Internet bağlantısı denetleniyor..."
${LangFileString} IDS_RUN_NOINET				"Internet bağlantısı yok"
${LangFileString} IDS_RUN_EXTRACT				"Ayıklanan: "
${LangFileString} IDS_RUN_DOWNLOAD				"İndirilen: "
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"İndirme tamamlandı."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"İndirme hatası. Neden:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"İndirme iptal edildi."
${LangFileString} IDS_RUN_INSTALL				"Yüklenen: "
${LangFileString} IDS_RUN_INSTALLFIALED			"İndirme başarısız."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Dosya bulunamadı."
${LangFileString} IDS_RUN_DONE					"Bitti."
${LangFileString} IDS_RUN_OPTIMIZING			"En iyi duruma getiriliyor..."

${LangFileString} IDS_DSP_PRESETS 	"SPS için hazır düzen dosyaları"
${LangFileString} IDS_DEFAULT_SKIN	"Varsayılan dış görünümler"
${LangFileString} IDS_AVS_PRESETS	"AVS için görsel öğe dosyaları"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop için görsel öğe dosyaları"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 için görsel öğe dosyaları"

${LangFileString} IDS_CLEANUP_PLUGINS	"Eklentiler siliyor..."
${LangFileString} IDS_REMOVE_SKINS		"Varsayılan dış görünümler kaldırılıyor..."

; download
${LangFileString} IDS_DOWNLOADING	"İndiriliyor"
${LangFileString} IDS_CONNECTING	"Bağlanılıyor ..."
${LangFileString} IDS_SECOND		" (1 saniye kaldı)"
${LangFileString} IDS_MINUTE		" (1 dakika kaldı)"
${LangFileString} IDS_HOUR			" (1 saat kaldı)"
${LangFileString} IDS_SECONDS		" (%u saniye kaldı)"
${LangFileString} IDS_MINUTES		" (%u dakika kaldı)"
${LangFileString} IDS_HOURS			" (%u saat kaldı)"
${LangFileString} IDS_PROGRESS		"%skB (%%%d) / %skB @ %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler	"Yürüt"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Kurulum Tamamlandı"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) bilgisayarınıza yüklendi.$\r$\n$\r$\n\
													Bu sihirbazı kapatmak için Bitir düğmesine tıklatın."
${LangFileString} IDS_PAGE_FINISH_RUN		"Kur kapandıktan sonra $(^NameDA)'ı başlat"
${LangFileString} IDS_PAGE_FINISH_LINK		"Winamp.com'u ziyaret etmek için buraya tıklatın"
${LangFileString} IDS_PAGE_FINISH_SETMUSICBUNDLE	"Winamp çalma listesine Ek MP3 ekle"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"$(^NameDA) Kur'a hoş geldiniz"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA), ortam kitaplığınızı yönetmenizi ve internet radyolarını dinleminize olanak sağlar.  \
											Winamp Cloud ile evde, işte, arabada veya bulunduğunuz her yerde müzik dinleyebilirsiniz.$\r$\n$\r$\n\
											Özelliklerden Bazıları:$\r$\n$\r$\n  \
												•  Winamp Cloud müzik kitaplığınızı rahatlıkla birleştirir$\r$\n$\r$\n  \
												•  Müzik kitaplığınızı aygıtlar üzerinden yönetin$\r$\n$\r$\n  \
												•  Andorid için Winamp ile kablosuz masaüstü eşitleme$\r$\n$\r$\n  \
												•  Otomatik Çalma Listesi Oluşturucu ile listeler oluşturun$\r$\n$\r$\n  \
												•  30,000'nin üzerinde podcast'i dinleyin ve abone olun"
					
; components					
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOT: Bento (önerilen) dış görünümünün tasarım ve \
															yeni özelliklerinden en iyi verimi almak için \
															tüm bileşenler seçilmiş olmalı."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Kısayol Oluşturma Seçenekleri Seçimi"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Oluşturmak istediğiniz kısayolları seçin."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Winamp için oluşturmak istediğiniz kısayolları seçin."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Başlat menüsüne yerleştir"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Hızlı Başlat'a yerleştir"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Masaüstüne simge yerleştir"

; bundle page
${LangFileString} IDS_PAGE_BUNDLE_TITLE		"$(^NameDA)'dan En İyi Sonucu Alın"
${LangFileString} IDS_PAGE_BUNDLE_SUBTITLE	"$(^NameDA)'dan en iyi sonucu almak için aşağıdan ek özellikler seçin."
${LangFileString} IDS_PAGE_BUNDLE_CAPTION	"Bu ek özellikler ile en iyi $(^NameDA) deneyimi edinin."
${LangFileString} IDS_PAGE_BUNDLE_DLSIZE	"indirme boyutu:"
${LangFileString} IDS_PAGE_BUNDLE_INSTALLED	"zaten yüklü"
${LangFileString} IDS_PAGE_BUNDLE_BADOS		"sadece Windows XP veya daha üstü"
${LangFileString} IDS_PAGE_BUNDLE_MB		"MB"
${LangFileString} IDS_PAGE_BUNDLE_KB		"KB"

; bundles
${LangFileString} IDS_BUNDLE1_NAME			"Winamp® Remote"
;${LangFileString} IDS_BUNDLE1_DESCRIPTION	"Hareket halindeyken müzik ve videolarınızın keyfini çıkarın. Winamp.com, \
;                            uyumlu mobil aygıtlar, Nintendo® Wii™, Sony® PlayStation® 3 ve Microsoft® Xbox™ 360 ile evinizdeki müziklere erişin."
${LangFileString} IDS_BUNDLE2_NAME			"Winamp® Araç Çubuğu"
${LangFileString} IDS_BUNDLE2_DESCRIPTION	"Web tarayıcısından Winamp'ı doğrudan denetlemenizi ve SHOUTcast'a anlık \
											erişim sağlar."
${LangFileString} IDS_BUNDLE21_NAME			"Winamp® Search'ü varsayılan arama motorum olarak ayarla"
${LangFileString} IDS_BUNDLE21_DESCRIPTION	"Winamp, Winamp Search ile web araması yapmanıza izin verir."
${LangFileString} IDS_WINAMP_SEARCH			"Winamp Search"

${LangFileString} IDS_BUNDLE3_NAME			"50 ücretsiz MP3 + EMusic'den 1 adet ücretsiz Ses Kitabı" 
${LangFileString} IDS_BUNDLE3_DESCRIPTION	"Başlangıç için, EMusic'e 2 haftalık ücretsiz deneme üyeliği ve 50 adet ücretsiz müzik indirme, \
                                             bir adet ücretsiz ses kitabı. Üyeliğiniz iptal olsa bile indirdiğiniz müzikleri ve ses kitabını saklayabilirsiniz."
											 
; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Winamp Ajanı kapatılamadı.$\r$\n\
                                                   Başka bir kullanıcı için yapılandırılmadığından emin olun.\
                                                   $\r$\n$\r$\n	Winamp Ajanı'nı kapatın ve Yeniden Dene'yi tıklatın.\
                                                   $\r$\n$\r$\n	Winamp Ajanı'nı yoksayıp devam etmek için Yoksay'ı tıklatın. \
                                                   $\r$\n$\r$\n	Kurulumu iptal etmek için Durdur'u tıklatın."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Windows'un bu sürümü artık desteklenmiyor.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} en az Windows 2000 veya daha yüksek sürümünü gerektirir."
												 
; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Uyumsuz 3. parti gen_msn7.dll eklentisi algılandı!$\r$\n$\r$\nBu eklenti Winamp 5.57 ve daha sonraki sürümlerde çökmeye neden olur.$\r$\nEklenti şimdi devre dışı bırakılacak. Devam etmek için Tamam'ı tıklatın."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Uyumsuz 3. parti gen_lyrics.dll eklentisi algılandı!$\r$\n$\r$\nBu eklenti Winamp 5.57 ve daha sonraki sürümlerde çökmeye neden olur.$\r$\nEklenti şimdi devre dışı bırakılacak. Devam etmek için Tamam'ı tıklatın."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "3. parti gen_lyrics eklentisi algılandı!$!$\r$\n$\r$\nBu eklentinin eski sürümleri Winamp 5.6 ve daha yeni sürümleri için uyumsuzdur. Devam etmeden önce http://lyricsplugin.com adresinden eklentinin son sürümüne sahip olduğunuzdan emin olun."
												 
;Winamp3
${LangFileString} msgWA3			"Winamp3 algılandı..."
${LangFileString} msgWA3_UPGRADE		"Winamp3 algılandı. Winamp3 kurulumu Winamp ${VERSION_MAJOR}. ${VERSION_MINOR}${VERSION_MINOR_SECOND}sürümüne yükseltilip ve dış görünümler Winamp'ın ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} sürümüne aktarılsın mı?$\r$\n(Yükseltmek için Evet'i, Winamp3 ve Winamp 5.x kurulumunu ayrı ayrı saklamak için ise Hayır'ı tıklatın)"
${LangFileString} msgWA3_MIGRATE		"Dış Görünümler Aktarılıyor..."
${LangFileString} msgWA3_REMOVE			"Winamp3 kur kaldırılıyor..."
${LangFileString} msgWA3_REMOVE2		"'$0' konumundaki tüm dosyalar kaldırılamadı.$\r$\nEğer dosyaları bu dizinden kaldırmak istiyorsanız bunu el ile yapmalısınız."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"${DIRECTXINSTAL_WINVER_LO} veya daha düşük bir sürümü bulundu"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "${DIRECTXINSTAL_WINVER_HI} veya daha yüksek bir sürümü bulundu"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} sürümü denetleniyor"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"En az ${DIRECTXINSTAL_DIRECTXNAME} sürümü gereklidir"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"${DIRECTXINSTAL_DIRECTXNAME} sürümü algılanamadı"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} sürümü algılandı"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Desteklenmeyen ${DIRECTXINSTAL_DIRECTXNAME} sürümü"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"$0 için denetleniyor"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"İndirme Gerekli"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"İnternet bağlantısı denetleniyor"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"${DIRECTXINSTAL_DIRECTXNAME} son sürümünü indirin:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"${DIRECTXINSTAL_DIRECTXNAME} kur indiriliyor"
${LangFileString} IDS_DIRECTX_FOUND						"Bulunan"
${LangFileString} IDS_DIRECTX_MISSING					"Hata"
${LangFileString} IDS_DIRECTX_SUCCESS					"Başarılı"
${LangFileString} IDS_DIRECTX_ABORTED					"İptal Edildi"
${LangFileString} IDS_DIRECTX_FAILED					"Başarısız"
${LangFileString} IDS_DIRECTX_DONE						"Bitti"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"${DIRECTXINSTAL_DIRECTXNAME} kur başlatılıyor"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} en az ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} sürümüne ihtiyaç duyar.$\r$\nŞimdi indirmek istiyor musunuz?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} en az ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} sürümüne ihtiyaç duyar"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"${DIRECTXINSTAL_DIRECTXNAME} indirilemedi"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"${DIRECTXINSTAL_DIRECTXNAME} kurulamadı"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Bilgisayarınızda ${DIRECTXINSTAL_WINAMPNAME} için gerekli ${DIRECTXINSTAL_DIRECTXNAME} bileşeni eksik"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Eksik ${DIRECTXINSTAL_DIRECTXNAME} bileşeni indirilemiyor"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Eksik ${DIRECTXINSTAL_DIRECTXNAME} bileşeni indirilemez"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"$(IDS_SEC_GEN_FRENCHRADIO) yükleniyor..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp (gerekli)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp Ajanı, hızlı erişim için görev çubuğunda simge gösterir ve Winamp ile ilişkilendirilmiş dosya türlerini korur"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Çoklu Ortam Motoru (Girdi/Çıktı sistemi)"
${LangFileString} IDS_SEC_CDDB_DESC				"CD başlıklarını çevrimiçi Gracenote veritabanından otomatik olarak alma desteği sağlar"
${LangFileString} IDS_SEC_SONIC_DESC			"Ses CD'leri Kopyalama ve Yazma için gerekli Sonic Kitaplığı"
${LangFileString} IDS_SEC_DSP_DESC				"Koro, flanj, tempo ve perdeleme denetimi gibi ekstra efektler uygulamak için DSP eklentisi"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Ses Kayıttan Yürütme Desteği (Girdi Eklentileri: Ses Çözücüleri)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"MP3, MP2, MP1, AAC biçimleri için kayıttan yürütme desteği sağlar (gerekli)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"WMA biçimi için kayıttan yürütme desteği (DRM desteği içerir)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"MIDI biçimleri (MID, RMI, KAR, MUS, CMF ve diğerleri) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Module biçimleri (MOD, XM, IT, S3M, ULT ve diğerleri) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Ogg Vorbis biçimi (OGG) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"MPEG-4 Audio biçimleri (MP4, M4A) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"FLAC için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Ses CD'leri için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Waveform biçimleri (WAV, VOC, AU, AIFF ve diğerleri) için kayıttan yürütme desteği"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Video Kayıttan Yürütme Desteği (Girdi Eklentileri: Video Çözücüleri)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Windows Media video biçimleri (WMV, ASF) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Nullsoft Video biçimi (NSV, NSA) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"MPEG-1/2 ve diğer video biçimleri için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"AVI ve MPEG video biçimleri için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Flash Video (FLV) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Matroska Video (MKV) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"MPEG-4 Video (MP4, M4V) için kayıttan yürütme desteği"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Adobe Flash akış biçimi (SWF, RTMP) için kayıttan yürütme desteği"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Kodlama ve Dönüştürme Desteği (Bir dosyayı başka biçimdeki dosyaya dönüştürmek ve CD kopyalamak için)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"WMA biçiminde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"WAV biçiminde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_SEC_MP3_ENC_DESC			"MP3 biçiminde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"M4A ve AAC biçimlerinde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"FLAC biçiminde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Ogg Vorbis biçiminde dosya oluşturma ve dönüştürme desteği"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Çıktı Eklentileri (ses nasıl işlendiği ve ses kartınıza nasıl gönderildiği denetimi için)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Eski tarz WAV/MME Yazıcısı (bazı kullanıcılar Kodlama eklentileri yerine bunu kullanmayı tercih ediyorlar)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound Çıktı (gerekli / varsayılan Çıktı eklentisi)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Eski tarz WaveOut Çıktısı (isteğe bağlı, artık önerilmiyor veya gerekli değil)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Kullanıcı Arayüzü Bileşenleri"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Genel Kısayollar eklentisi, Winamp'ı kısayollar aracılığıyla yönetmenizi sağlar"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Dosyaya Atla eklentisi, çalma listesindeki dosyaları belirli bir şekilde sıralamak ve daha fazlası için destek sağlar"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft Görev Çubuğu Denetim Alanı eklentisi, yürütme denetimi simgelerini görev çubuğunda gösterir"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Winamp Modern ve Bento dış görünümler gibi serbest formlar kullanan dış görünümler için gereklidir"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Görsel Öğe Eklentileri"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Tam Ekran görsel öğe eklentisi"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio eklentisi"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop görsel öğe eklentisi"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 görsel öğe eklentisi (varsayılan g.ö. eklentisi)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Line Input Desteği, linein:// komutunu kullanma desteği sağlar (mikr/hat girişi için)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp Kitaplığı"
${LangFileString} IDS_SEC_ML_DESC				"Winamp Ortam Kitaplığı (gerekli)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Dönüştürme Aracı, bir dosyayı başka biçimdeki dosyaya dönüştürür (ör. wav dosyasını mp3 dosyasına)"
${LangFileString} IDS_SEC_ML_RG_DESC			"Replay Gain Analiz Aracı, ses yüksekliği dengeleme için destek sağlar"
${LangFileString} IDS_SEC_ML_DASH_DESC			"Gösterge, Ortam Kitaplığı için isteğe bağlı web sayfası portalını gösterir"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Otomatik Etiketleyici (Gracenote destekli), eksik meta verileri doldurma desteği sağlar"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Podcast Dizini, podcast yayınlarına abone olma ve indirme desteği sağlar"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Çevrimiçi Servisler, çevrimiçi SHOUTcast Radyo ve TV, In2TV, AOL Video ve XM Radyo akışlarını içerir"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Gracenote tarafından sağlanan akustik olarak dinamik çalma listeleri oluşturma desteği sağlar"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Ortam Kitaplığı Temel Bileşenleri"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Yerel Ortam veritabanı, Güçlü sorgulama-arama işlevi ve akıllı görünümler oluşturmak için destek sağlar"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Çalma Listeleri Yöneticisi, çalma listesi oluşturma, düzenleme ve saklamak için destek sağlar"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD Kopyala ve Yaz, CD kopyalama ve CD'ye yazma için ortam kitaplığı arayüzü desteği sağlar"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Yer İmleri Yöneticisi, sık kullandığınız akış, dosya ve klasörleri yönetmenizi sağlar"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Geçmiş, yerel veya uzak ortamdaki son yürütülen tüm dosya ve akışları gösterir"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Şimdi Yürütülüyor, geçerli parça hakkında çevrimiçi bilgiler edinmek için destek sağlar"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Taşınabilir Ortam Yürütücüsü Desteği"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Taşınabilir Ortam Yürütücüsü Desteği için temel eklenti (gerekli)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod® desteği"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Taşınabilir Creative® aygıt desteği (Nomad™, Zen™ ve MuVo™ yürütücülerini yönetmek için)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Microsoft® PlaysForSure® için destek (tüm P4S uyumlu yürütücüleri yönetmek için)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB Aygıtları desteği (genel usb aygıtları ve yürütücüleri için)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Microsoft® ActiveSync® için destek (Windows Mobile®, Akıllı Telefon ve Cep Bilgisayarı aygıtlarını yönetmek için)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Android aygıtları için destek sağlar"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Android Wifi desteği"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Yakında gelecek dropbox eklentisinin alfa sürümü. Etkinleştirmek için Ctrl-ÜstKrk-d tuşlarını kullanın"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Ortam Kitaplığı için iTunes uyumlu veritabanı al/ver eklentisi"
;${LangFileString} IDS_SEC_ML_ORGLER_DESC		"Winamp Orgler™, dinlediğiniz parçaların geçmişini izlemenizi ve tablo halinde çevrimi paylaşmanızı sağlar"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"$(^NameDA) ile 300'den fazla Fransız radyo istasyonunu canlı dinleyin (Virgin radio, NRJ, RTL, Skyrock, RMC...)"
${LangFileString} IDS_SEC_CLOUD_DESC			"Winamp Cloud her yerden müziklerinizi yönetin ve  dinleyin"


${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Eklenti Tarayıcısı ile eklentiler keşfedin ve Winamp'a ekleyin"
${LangFileString} IDS_SEC_MUSIC_BUNDLE_DESC		"Winamp Müzikten ücretsiz müzikler indirin ve dinleyin"

${LangFileString} IDS_UNINSTALL_BUNDLES_GROUP_DESC		"Kaldırılacak ilişkili programları seçin."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"$(^NameDA) programını bilgisayardan kaldır."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Kaldırma Yolu:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Ortam Yürütücüsü"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"3. parti eklentiler dâhil tüm $(^NameDA) eklentilerini kaldır."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Kullanıcı Tercihleri"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Tüm $(^NameDA) eklentileri ve tercih ayarlarını kaldır."

${LangFileString} IDS_UNINSTALL_BUNDLES_HEADER			"Winamp ile ilişkili kaldırılacak programlar:"
${LangFileString} IDS_UNINSTALL_WINAMP_TOOLBAR_DESC		"Web tarayıcısından Winamp'ı doğrudan denetlemenizi ve SHOUTcast'a anlık erişim sağlar."
${LangFileString} IDS_UNINSTALL_WINAMP_REMOTE_DESC		"Hareken halindeyken video ve müziklerin keyfini çıkarın."
${LangFileString} IDS_UNINSTALL_BROWSER_PLUGIN_DESC		"Firefox ve Internet Explorer tarayıcılar için Winamp Algılayıcı eklentileri."
${LangFileString} IDS_UNINSTALL_EMUSIC_DESC				"eMusic.com'dan indirilmiş müzik, MP3 ve şarkılar."
${LangFileString} IDS_UNINSTALL_BUNDLE_TEMPLATE			"$2 kaldırılıyor..."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"$(^NameDA) için geri bildirim gönder"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"$(^NameDA) klasörünü aç"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNot: Tüm dosyalar kaldırıldı. İçeriği görmek için Winamp klasörünü açın."

