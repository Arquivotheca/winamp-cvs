LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := nsid3v1
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES :=  tag.cpp nsid3v1.cpp
LOCAL_STATIC_LIBRARIES := nu
include $(BUILD_STATIC_LIBRARY)

