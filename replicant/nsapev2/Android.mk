LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := nsapev2
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES :=  header.cpp item.cpp tag.cpp nsapev2_common.cpp
LOCAL_SRC_FILES +=  android/nsapev2.cpp
LOCAL_STATIC_LIBRARIES := nu
LOCAL_SHARED_LIBRARIES := nx
include $(BUILD_STATIC_LIBRARY)

