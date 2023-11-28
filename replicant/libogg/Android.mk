LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libogg

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/include
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  src/bitwise.c src/framing.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
