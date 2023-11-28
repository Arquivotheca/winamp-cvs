ifneq ($(REPLICANT_NO_ALAC), 1)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := alac
LOCAL_MODULE_FILENAME := libalac.w6c
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_STATIC_LIBRARIES := nsalac nu foundation
LOCAL_SHARED_LIBRARIES := nx

LOCAL_SRC_FILES :=  main.cpp

# MP4
ifneq ($(REPLICANT_NO_MP4), 1)
LOCAL_SRC_FILES += MP4ALACDecoder.cpp MP4DecoderService.cpp
endif

include $(BUILD_SHARED_LIBRARY)

endif