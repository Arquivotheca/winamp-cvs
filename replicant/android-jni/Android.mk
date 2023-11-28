LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := replicant
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME)

LOCAL_C_INCLUDES := $(ROOT_REPLICANT) $(ROOT_REPLICANT)/Android/include
LOCAL_SRC_FILES := Application.cpp JNIReplicant.cpp JNIMediaPlayer.cpp JNIMetadata.cpp JNIMetadataEditor.cpp JNI.cpp JNIEqualizer.cpp JNIIPlaylistLoader.cpp JNIPlaylistManager.cpp JNIPlaybackParameters.cpp PlaybackParameters.cpp JNIAutoTagAlbum.cpp JNIAutoTagTrack.cpp JNIData.cpp JNIArtwork.cpp JNICloudManager.cpp JNIMediaServer.cpp MD5.c

LOCAL_STATIC_LIBRARIES := Wasabi component replicant-common replicant-playback replicant-metadata replicant-codec nu foundation nseq nswasabi
LOCAL_SHARED_LIBRARIES := nx

LOCAL_LDLIBS = -llog -ldl

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := replicant-loader

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_SRC_FILES := Loader.cpp

LOCAL_STATIC_LIBRARIES := nullsoft-cpufeatures

LOCAL_LDLIBS = -llog

include $(BUILD_SHARED_LIBRARY)

