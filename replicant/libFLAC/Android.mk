LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libFLAC
LOCAL_C_INCLUDES := $(LOCAL_PATH)/android $(LOCAL_PATH)/src/libFLAC/include  $(LOCAL_PATH)/include
LOCAL_CFLAGS := -DHAVE_CONFIG_H
LOCAL_SRC_FILES := src/libFLAC/bitmath.c\
	src/libFLAC/bitreader.c \
	src/libFLAC/bitwriter.c \
	src/libFLAC/cpu.c \
	src/libFLAC/crc.c \
	src/libFLAC/fixed.c \
	src/libFLAC/float.c \
	src/libFLAC/format.c \
	src/libFLAC/lpc.c \
	src/libFLAC/md5.c \
	src/libFLAC/memory.c \
	src/libFLAC/metadata_iterators.c \
	src/libFLAC/metadata_object.c \
	src/libFLAC/stream_decoder.c \
	src/libFLAC/stream_encoder.c \
	src/libFLAC/stream_encoder_framing.c \
	src/libFLAC/window.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
