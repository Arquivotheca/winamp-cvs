OLD_LOCAL_PATH := $(call my-dir)
include $(OLD_LOCAL_PATH)/pcm/Android.mk

LOCAL_PATH:= $(OLD_LOCAL_PATH)

# ----------- ARM BUILD -----------
include $(CLEAR_VARS)
LOCAL_MODULE := nsmath-armv5
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../..
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp

LOCAL_SRC_FILES := pcm.cpp
LOCAL_STATIC_LIBRARIES := nsmath-armv5-pcm
include $(BUILD_STATIC_LIBRARY)





