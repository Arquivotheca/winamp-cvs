

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_MODULE := file
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c
LOCAL_SRC_FILES :=  main.cpp
# local playback
LOCAL_SRC_FILES +=  FilePlayback.cpp
# decoder 
LOCAL_SRC_FILES +=  FileDecoder.cpp 
# metadata
LOCAL_SRC_FILES +=  FileMetadata.cpp FileMetadataRead.cpp FileMetadataService.cpp FileMetadataWrite.cpp
# raw reader
LOCAL_SRC_FILES +=  FileRawReader.cpp

LOCAL_STATIC_LIBRARIES := nswasabi nsid3v2 nsid3v1 nsapev2 nu foundation 
LOCAL_SHARED_LIBRARIES := nx 
LOCAL_LDFLAGS := -llog -lz

include $(BUILD_SHARED_LIBRARY)
