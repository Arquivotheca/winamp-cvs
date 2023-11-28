
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk
LOCAL_MODULE := audiotrack-pro
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif
LOCAL_SRC_FILES :=  AudioSession.cpp AudioTrack/AudioThread.cpp main.cpp Equalizer.cpp
LOCAL_SRC_FILES +=  pcmutils.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES +=  pcmutils-armv7-vfp.S
endif

LOCAL_STATIC_LIBRARIES := nu foundation nseq
LOCAL_SHARED_LIBRARIES := android-utils android-media nx
LOCAL_LDFLAGS := -llog
include $(BUILD_SHARED_LIBRARY)

# android 2.3-4.0
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk
LOCAL_MODULE := audiotrack9-pro
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -DREPLICANT_AUDIOTRACK9
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif
LOCAL_SRC_FILES :=  AudioSession.cpp AudioTrack/AudioThread.cpp main.cpp Equalizer.cpp
LOCAL_SRC_FILES +=  pcmutils.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES +=  pcmutils-armv7-vfp.S
endif

LOCAL_STATIC_LIBRARIES := nu foundation nseq
LOCAL_SHARED_LIBRARIES := android-utils9 android-media9 nx
LOCAL_LDFLAGS := -llog
include $(BUILD_SHARED_LIBRARY)


# android 4.1+
ifeq ($(TARGET_PLATFORM),android-9)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk
LOCAL_MODULE := OpenSL-out
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -DREPLICANT_OPENSL
ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
endif

LOCAL_SRC_FILES :=  AudioSession.cpp OpenSL/AudioThread.cpp OpenSL/BufferManager.cpp main.cpp Equalizer.cpp
LOCAL_SRC_FILES +=  pcmutils.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES +=  pcmutils-armv7-vfp.S
endif

LOCAL_STATIC_LIBRARIES := nu foundation nseq
LOCAL_SHARED_LIBRARIES := nx
LOCAL_LDLIBS  += -lOpenSLES -llog

include $(BUILD_SHARED_LIBRARY)
endif