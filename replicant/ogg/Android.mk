ifneq ($(REPLICANT_NO_OGG), 1)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ogg
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  android/OggPlayback.cpp main.cpp OggPlaybackService.cpp

LOCAL_STATIC_LIBRARIES := libogg nu foundation nswasabi
LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_SHARED_LIBRARY)
endif