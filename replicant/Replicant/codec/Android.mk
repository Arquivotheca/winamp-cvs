LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := replicant-codec
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_ARM_MODE := arm
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif


LOCAL_SRC_FILES :=  codec.cpp DecodeAPI.cpp
LOCAL_SRC_FILES += adapters/audio-decoder/callback_to_callback.cpp adapters/audio-decoder/callback_to_callback_interleave.cpp adapters/audio-decoder/pcmutils.c adapters/audio-decoder/callback_to_callback_convert.cpp adapters/audio-decoder/callback_to_callback_interleave_decimate.cpp adapters/audio-decoder/callback_to_callback_interleave_gain.cpp
LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_STATIC_LIBRARY)


