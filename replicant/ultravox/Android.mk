LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk
LOCAL_MODULE := ultravox
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  main.cpp UltravoxDemuxer.cpp UltravoxDemuxerService.cpp UltravoxHeader.c SHOUTcast2Metadata.cpp

LOCAL_STATIC_LIBRARIES := nswasabi nu foundation
LOCAL_SHARED_LIBRARIES := jnet nx
include $(BUILD_SHARED_LIBRARY)
