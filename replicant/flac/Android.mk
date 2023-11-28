ifneq ($(REPLICANT_NO_FLAC), 1)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := flac
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  FLACPlayback.cpp main.cpp FLACPlaybackService.cpp FLACMetadata.cpp FLACDecoder.cpp FLACDecoderCallback.cpp FLACMetadataService.cpp FLACFileCallbacks.cpp FLACMetadataEditor.cpp FLACRawReader.cpp FLACHTTP.cpp
LOCAL_SRC_FILES += crt/FLACMetadataCallbacks.cpp

LOCAL_STATIC_LIBRARIES := libFLAC nswasabi nu foundation
LOCAL_SHARED_LIBRARIES := nx jnet
LOCAL_LDFLAGS := -llog

include $(BUILD_SHARED_LIBRARY)
endif
