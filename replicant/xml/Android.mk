LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := xml
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  main.cpp 

LOCAL_SRC_FILES +=  android/XMLAttributes.cpp android/XMLParser.cpp android/Encodings.cpp android/regexp.cpp

LOCAL_STATIC_LIBRARIES := foundation libexpat nx nu
#LOCAL_SHARED_LIBRARIES := nx

include $(BUILD_SHARED_LIBRARY)
