LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := gnsdk-dsp
LOCAL_SRC_FILES := android-arm/libgnsdk_dsp.2.1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gnsdk-link
LOCAL_SRC_FILES := android-arm/libgnsdk_link.2.1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gnsdk-manager
LOCAL_SRC_FILES := android-arm/libgnsdk_manager.2.1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gnsdk-musicid_file
LOCAL_SRC_FILES := android-arm/libgnsdk_musicid_file.2.1.0.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk
LOCAL_MODULE := gracenote
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(ROOT_REPLICANT) $(LOCAL_PATH)/include $(LOCAL_PATH)/include/android_arm-32
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_SRC_FILES :=  AutoTagTrack.cpp AutoTagAlbum.cpp Gracenote.cpp main.cpp MetadataGDO.cpp MusicID_File_Populate.cpp
LOCAL_LDLIBS = -llog

LOCAL_STATIC_LIBRARIES := nu foundation
LOCAL_SHARED_LIBRARIES := gnsdk-dsp gnsdk-link gnsdk-manager gnsdk-musicid_file nx
include $(BUILD_SHARED_LIBRARY)