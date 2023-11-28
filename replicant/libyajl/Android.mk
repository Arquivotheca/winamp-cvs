LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libyajl

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_SRC_FILES :=  src/yajl.c src/yajl_alloc.c src/yajl_buf.c src/yajl_encode.c src/yajl_gen.c src/yajl_lex.c src/yajl_parser.c src/yajl_tree.c src/yajl_version.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
