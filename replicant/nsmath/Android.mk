NSMATH_LOCAL_PATH := $(call my-dir)
include $(NSMATH_LOCAL_PATH)/armv5/Android.mk
include $(NSMATH_LOCAL_PATH)/armv7a/Android.mk

LOCAL_PATH:= $(NSMATH_LOCAL_PATH)

# ----------- ARM BUILD -----------
ifeq ($(TARGET_ARCH_ABI),armeabi)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := nsmath
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME)
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp

LOCAL_STATIC_LIBRARIES := nsmath-armv5

include $(BUILD_SHARED_LIBRARY)
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk


LOCAL_MODULE := nsmath
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME)
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_STATIC_LIBRARIES := nsmath-armv7a

include $(BUILD_SHARED_LIBRARY)
endif
