ifneq ($(REPLICANT_NO_MP4), 1)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_MODULE := mp4
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_SRC_FILES :=  main.cpp
# local playback
LOCAL_SRC_FILES +=  MP4Playback.cpp MP4PlaybackService.cpp MP4MetadataBase.cpp MP4FileObject.cpp MP4Metadata.cpp MP4MetadataService.cpp MP4MetadataEditor.cpp MP4Decoder.cpp MP4DecoderCallback.cpp MP4RawReader.cpp MP4MetadataFile.cpp MP4HTTP.cpp

LOCAL_STATIC_LIBRARIES := nsmp4 nswasabi nu foundation 
LOCAL_SHARED_LIBRARIES := nx jnet
LOCAL_LDFLAGS := -llog
include $(BUILD_SHARED_LIBRARY)
endif
