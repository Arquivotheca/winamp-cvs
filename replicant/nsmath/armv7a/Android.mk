NSMATH_ARMV7A_LOCAL_PATH := $(call my-dir)
include $(NSMATH_ARMV7A_LOCAL_PATH)/pcm/Android.mk

LOCAL_PATH:= $(NSMATH_ARMV7A_LOCAL_PATH)

# ----------- ARM BUILD -----------
include $(CLEAR_VARS)
LOCAL_MODULE := nsmath-armv7a
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../..
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES := pcm.cpp
LOCAL_STATIC_LIBRARIES := nsmath-armv7a-pcm-vfp
include $(BUILD_STATIC_LIBRARY)





