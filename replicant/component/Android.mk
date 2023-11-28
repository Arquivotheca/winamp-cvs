LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := component

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
LOCAL_SRC_FILES := ComponentManagerBase.cpp android/ComponentManager.cpp

LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := nx

LOCAL_LDLIBS = -llog -ldl

include $(BUILD_STATIC_LIBRARY)

