LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libexpat
LOCAL_CFLAGS := -include android/config.h
LOCAL_SRC_FILES := lib/xmltok.c lib/xmlparse.c lib/xmlrole.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/lib
include $(BUILD_STATIC_LIBRARY)
