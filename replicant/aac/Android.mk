ifneq ($(REPLICANT_NO_AAC), 1)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := aac
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT) $(ROOT_REPLICANT)/Android/include
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_STATIC_LIBRARIES :=  nsaac nu foundation
LOCAL_SHARED_LIBRARIES := libjnet nx

LOCAL_SRC_FILES :=  main.cpp

ifneq ($(REPLICANT_NO_ICY), 1)
# shoutcast
LOCAL_SRC_FILES += android/ICYAAC.cpp ICYAACService.cpp
endif

# MP4
ifneq ($(REPLICANT_NO_MP4), 1)
LOCAL_SRC_FILES += android/MP4AACDecoder.cpp MP4DecoderService.cpp
endif

# Ultravox
ifneq ($(REPLICANT_NO_ULTRAVOX), 1)
LOCAL_SRC_FILES += UltravoxAAC.cpp UltravoxAACService.cpp
endif

LOCAL_SRC_FILES += android/PVMP4.cpp android/IAACDecoder.cpp android/FDKAACDecoder.cpp

LOCAL_LDFLAGS = -llog
include $(BUILD_SHARED_LIBRARY)

endif