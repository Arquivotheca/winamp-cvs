LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_ARM_MODE:=arm
LOCAL_MODULE := mp3
LOCAL_MODULE_FILENAME := libmp3.$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp 
endif
LOCAL_SRC_FILES :=  main.cpp GioReplicant.cpp

# local file
ifneq ($(REPLICANT_NO_LOCAL), 1)
LOCAL_SRC_FILES +=  giofile_crt.cpp MP3Playback.cpp MP3PlaybackService.cpp MP3RawReader.cpp MP3Decoder.cpp MP3DecoderCallback.cpp MP3MetadataService.cpp MP4MP3Decoder.cpp MP4DecoderService.cpp
endif

# shoutcast
ifneq ($(REPLICANT_NO_ICY), 1)
LOCAL_SRC_FILES +=  gioicy.cpp ICYMP3.cpp ICYMP3Service.cpp
endif

# shoutcast 2.0
ifneq ($(REPLICANT_NO_ULTRAVOX), 1)
LOCAL_SRC_FILES +=  gioultravox.cpp UltravoxMP3.cpp UltravoxMP3Service.cpp
endif

# HTTP
ifneq ($(REPLICANT_NO_HTTP), 1)
LOCAL_SRC_FILES +=  giojnet.cpp MP3HTTP.cpp MP3HTTPService.cpp
endif

# utilities
LOCAL_SRC_FILES +=  CVbriHeader.cpp LAMEInfo.cpp MPEGHeader.cpp OFL.cpp 

LOCAL_LDLIBS := -lz -llog
LOCAL_STATIC_LIBRARIES := nswasabi nsmp3 nsid3v2 nu foundation 
LOCAL_SHARED_LIBRARIES := nx jnet

include $(BUILD_SHARED_LIBRARY)

# --- NEON build ---
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_ARM_MODE:=arm
LOCAL_MODULE := mp3-neon
LOCAL_ARM_NEON  := true
LOCAL_MODULE_FILENAME := libmp3.$(PLATFORM_NAME)-neon.w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -mfpu=neon
LOCAL_SRC_FILES :=  main.cpp GioReplicant.cpp

# local file
ifneq ($(REPLICANT_NO_LOCAL), 1)
LOCAL_SRC_FILES +=  giofile_crt.cpp MP3Playback.cpp MP3PlaybackService.cpp MP3RawReader.cpp MP3Decoder.cpp MP3DecoderCallback.cpp MP3MetadataService.cpp MP4MP3Decoder.cpp MP4DecoderService.cpp
endif

# shoutcast
ifneq ($(REPLICANT_NO_ICY), 1)
LOCAL_SRC_FILES +=  gioicy.cpp ICYMP3.cpp ICYMP3Service.cpp
endif

# shoutcast 2.0
ifneq ($(REPLICANT_NO_ULTRAVOX), 1)
LOCAL_SRC_FILES +=  gioultravox.cpp UltravoxMP3.cpp UltravoxMP3Service.cpp
endif

# HTTP
ifneq ($(REPLICANT_NO_HTTP), 1)
LOCAL_SRC_FILES +=  giojnet.cpp MP3HTTP.cpp MP3HTTPService.cpp
endif

# utilities
LOCAL_SRC_FILES +=  CVbriHeader.cpp LAMEInfo.cpp MPEGHeader.cpp OFL.cpp 

LOCAL_LDLIBS := -lz -llog
LOCAL_STATIC_LIBRARIES := nswasabi nsmp3-neon nsid3v2 nu foundation 
LOCAL_SHARED_LIBRARIES := nx jnet

include $(BUILD_SHARED_LIBRARY)
endif
