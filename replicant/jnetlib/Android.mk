LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := jnet
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME)
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -DUSE_SSL
LOCAL_SRC_FILES := asyncdns.cpp connection.cpp headers.cpp httpget.cpp httpserv.cpp httpuserv.cpp jnetlib.cpp listen.cpp multicastlisten.cpp sslconnection.cpp udpconnection.cpp util.cpp
LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := ssl
LOCAL_LDLIBS = -lz 
#LOCAL_LDFLAGS := -Wl,-soname,/data/lib$(LOCAL_MODULE).$(PLATFORM_NAME)
include $(BUILD_SHARED_LIBRARY)
