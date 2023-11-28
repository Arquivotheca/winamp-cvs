LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := http
LOCAL_ARM_MODE := arm
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  HTTPPlayback.cpp HTTPPlaybackService.cpp main.cpp

LOCAL_STATIC_LIBRARIES := nswasabi nu foundation 
LOCAL_SHARED_LIBRARIES := jnet nx
LOCAL_LDFLAGS := -llog

include $(BUILD_SHARED_LIBRARY)
