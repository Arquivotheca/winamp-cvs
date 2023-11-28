LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := icy
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)

LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  main.cpp ICYDemuxer.cpp ICYDemuxerService.cpp ICYMetadata.cpp ICYReader.cpp 

LOCAL_STATIC_LIBRARIES := foundation nu
LOCAL_SHARED_LIBRARIES := jnet nx
LOCAL_LDFLAGS := -llog
include $(BUILD_SHARED_LIBRARY)
