LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := nu
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -fno-rtti

# platform independent files
LOCAL_SRC_FILES := nodelist.c LockFreeRingBuffer.cpp RingBuffer.cpp utf.c  PtrList.cpp BitReader.cpp ByteWriter.c ProgressTracker.cpp

# ARMv7 build
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES += android-armv7/ThreadLoop.cpp android-armv7/LockFreeLIFO.c android-armv7/LockFreeLIFO-armv6.S android-armv7/lfmpscq.c  android-armv7/lfmpscq-armv6.S android-armv7/ByteReader-armv7.S android-armv7/lfitem-armv6.S android-armv7/ByteReader.c android-armv7/MessageLoop.cpp
endif

# ARMv5 build
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES += android-arm/ThreadLoop.cpp android-arm/lfmpscq.c ByteReader.c android-arm/lfitem.c android-arm/MessageLoop.cpp
endif

# x86 build
ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_SRC_FILES += android-x86/ThreadLoop.cpp android-x86/lfmpscq.c ByteReader.c android-x86/lfitem.c android-x86/LockFreeLIFO.c android-x86/LockFreeLIFO-android-x86.S
endif

include $(BUILD_STATIC_LIBRARY)

