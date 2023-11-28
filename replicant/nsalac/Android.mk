LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := nsalac
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_SRC_FILES := alac_decode.c
LOCAL_STATIC_LIBRARIES := nu
include $(BUILD_STATIC_LIBRARY)
