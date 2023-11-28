LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := nsid3v2
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES :=  frameheader.cpp frames.c header.cpp tag.cpp util.cpp values.cpp extendedheader.cpp frame.cpp nsid3v2_common.cpp
LOCAL_SRC_FILES += frame_text.cpp frame_usertext.cpp frame_private.cpp frame_object.cpp frame_popm.cpp frame_apic.cpp frame_utils.cpp frame_comments.cpp frame_id.cpp
LOCAL_SRC_FILES +=  android/nsid3v2.cpp 
LOCAL_STATIC_LIBRARIES := nu
include $(BUILD_STATIC_LIBRARY)

