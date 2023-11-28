LOCAL_PATH:= $(call my-dir)

# ----------- ARM BUILD -----------
include $(CLEAR_VARS)
LOCAL_MODULE := nsmath-armv7a-pcm-vfp
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../..
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES := pcm_convert_F32_S16.c pcm_convert_F32_S24.c pcm_interleave_S32_F32.c pcm_interleave_S32_S16.c pcm_interleave_S32_S16_gain.c pcm_interleave_S32_S16_shift.c pcm_interleave_S32_S16_shift8.c pcm_interleave_S32_S24.c pcm_interleave_S32_S24_gain.c pcm_interleave_S32_S24_pad.c pcm_interleave_S32_S24_shift.c

include $(BUILD_STATIC_LIBRARY)