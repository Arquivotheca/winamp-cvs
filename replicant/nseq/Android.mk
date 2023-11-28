LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := nseq
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden 
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif
LOCAL_SRC_FILES :=  arm/eq_4front.c
include $(BUILD_STATIC_LIBRARY)

