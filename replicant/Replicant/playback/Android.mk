LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := replicant-playback
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES := playback.cpp Player.cpp Player_APC.cpp Player_Internal.cpp Player_Playback.cpp

LOCAL_STATIC_LIBRARIES := nu foundation nseq
LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_STATIC_LIBRARY)


