ifneq ($(REPLICANT_NO_MP4), 1)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := faad
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/libfaad
LOCAL_CFLAGS := -fvisibility=hidden -DHAVE_CONFIG_H
LOCAL_SRC_FILES := libfaad/bits.c libfaad/error.c libfaad/lt_predict.c	libfaad/ps_dec.c libfaad/sbr_e_nf.c libfaad/sbr_syntax.c libfaad/syntax.c \
libfaad/cfft.c		libfaad/filtbank.c	libfaad/mdct.c		libfaad/ps_syntax.c	libfaad/sbr_fbt.c	libfaad/sbr_tf_grid.c	libfaad/tns.c \
libfaad/common.c	libfaad/hcr.c		libfaad/mp4.c		libfaad/pulse.c		libfaad/sbr_hfadj.c	libfaad/specrec.c \
libfaad/decoder.c	libfaad/huffman.c	libfaad/ms.c		libfaad/rvlc.c		libfaad/sbr_hfgen.c	libfaad/ssr.c \
libfaad/drc.c		libfaad/ic_predict.c	libfaad/output.c	libfaad/sbr_dct.c	libfaad/sbr_huff.c	libfaad/ssr_fb.c \
libfaad/drm_dec.c	libfaad/is.c		libfaad/pns.c		libfaad/sbr_dec.c	libfaad/sbr_qmf.c	libfaad/ssr_ipqf.c 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
endif