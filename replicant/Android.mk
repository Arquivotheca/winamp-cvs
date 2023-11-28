OLD_LOCAL_PATH := $(call my-dir)
ROOT_REPLICANT := $(call my-dir)
include $(call all-subdir-makefiles)

$(shell ${NDK_REPLICANT_PATH}/generate-replicant-githash.sh)
