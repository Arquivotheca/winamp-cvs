LOCAL_PATH:= $(call my-dir)

# ----------- ARM BUILD -----------
include $(CLEAR_VARS)
LOCAL_MODULE := nsmp3
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif

LOCAL_SRC_FILES := mp3decode.cpp mp3quant.cpp mp3read.cpp mp3ssc.cpp \
mp3tools.cpp mpegbitstream.cpp mpegheader.cpp mpgadecoder.cpp polyphase.cpp sequencedetector.cpp \
bitstream.cpp conceal.cpp crc16.cpp huffdec.cpp huffmanbitobj.cpp huffmandecoder.cpp huffmantable.cpp \
l3table.cpp mdct.cpp meanvalue.cpp mp2decode.cpp mp3ancofl.cpp 

include $(BUILD_STATIC_LIBRARY)

# ----------- NEON BUILD -----------
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
include $(CLEAR_VARS)
LOCAL_MODULE := nsmp3-neon
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON  := true
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -mfpu=neon

LOCAL_SRC_FILES := mp3decode.cpp mp3quant.cpp mp3read.cpp mp3ssc.cpp \
mp3tools.cpp mpegbitstream.cpp mpegheader.cpp mpgadecoder.cpp sequencedetector.cpp \
bitstream.cpp conceal.cpp crc16.cpp huffdec.cpp huffmanbitobj.cpp huffmandecoder.cpp huffmantable.cpp \
l3table.cpp meanvalue.cpp mp2decode.cpp mp3ancofl.cpp 	

LOCAL_SRC_FILES += arm-neon/polyphase.cpp arm-neon/polyphase-neon.S arm-neon/mdct.cpp
include $(BUILD_STATIC_LIBRARY)
endif