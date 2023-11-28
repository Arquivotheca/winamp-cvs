# ARMv7 is significanly faster due to the use of the hardware FPU

APP_ABI := armeabi armeabi-v7a
#APP_CFLAGS := -O2
APP_OPTIM := release

include $(call my-dir)/BuildConfig.mk

ifneq ($(REPLICANT_NO_MP4), 1)
APP_STL := gnustl_static	 
endif

ifndef REPLICANT_APP_PLATFORM
APP_PLATFORM := android-9
else
APP_PLATFORM := android-$(REPLICANT_APP_PLATFORM)
endif

REPLICANT_DEBUG := 1
