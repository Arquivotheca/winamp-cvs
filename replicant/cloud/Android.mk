ifneq ($(REPLICANT_NO_CLOUD), 1)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(ROOT_REPLICANT)/PlatformName.mk

LOCAL_MODULE := cloud
LOCAL_MODULE_FILENAME := lib$(LOCAL_MODULE).$(PLATFORM_NAME).w6c

LOCAL_C_INCLUDES := $(ROOT_REPLICANT)
LOCAL_CFLAGS := -fvisibility=hidden -DSQLITE_THREADSAFE=2

LOCAL_STATIC_LIBRARIES := nu foundation yajl nswasabi
LOCAL_SHARED_LIBRARIES := libjnet nx

LOCAL_SRC_FILES :=  main.cpp Logger.cpp CloudAPI.cpp CloudSocket.cpp CloudThread.cpp Config.cpp DevicesList.cpp post.cpp Pull-Announce.cpp Pull-Update.cpp sha1.c CloudClient-Upload.cpp CloudClient-Download.cpp UserProfile.cpp CloudClient-Playlists.cpp ItemMetadata.cpp
LOCAL_SRC_FILES +=  CloudDB.cpp CloudDB-IDMap.cpp CloudDB-Media.cpp CloudDB-Step.cpp CloudDB-Info.cpp CloudDB-Devices.cpp CloudDB-Hash.cpp CloudDB-Artwork.cpp CloudDB-Playlists.cpp CloudDB-MetahashMap.cpp
LOCAL_SRC_FILES +=  Transaction-Announce.cpp TransactionQueue.cpp Transaction-Reset.cpp Transaction-Delete.cpp Transaction-Update.cpp JSONMetadata.cpp Transaction-Played.cpp

# sqlite
#LOCAL_SRC_FILES +=  sqlite/sqlite3.c
LOCAL_SHARED_LIBRARIES += android-sqlite

# JSON-Tree
LOCAL_SRC_FILES +=  JSON-Builder.cpp JSON-KeyValue.cpp JSON-Tree.cpp JSON-Value.cpp

LOCAL_LDFLAGS = -llog
include $(BUILD_SHARED_LIBRARY)

endif
