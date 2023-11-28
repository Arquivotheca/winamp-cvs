LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := foundation
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES :=  android-armv7/atomics_armv6.S
else
LOCAL_SRC_FILES :=  android-arm/atomics.c android-arm/atomics_armv5.S
endif
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../Android/include
#LOCAL_CFLAGS := -fvisibility=hidden
#LOCAL_LDLIBS := -fvisibility=hidden
include $(BUILD_STATIC_LIBRARY)
