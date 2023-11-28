ifneq ($(REPLICANT_NO_MP4), 1)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := nsmp4
LOCAL_C_INCLUDES := $(ROOT_REPLICANT) $(LOCAL_PATH)/src $(LOCAL_PATH)/include $(LOCAL_PATH)/include/android
LOCAL_CFLAGS := -fexceptions
LOCAL_SRC_FILES := boxes/atom_amr.cpp   boxes/atom_free.cpp  boxes/atom_sound.cpp     boxes/atom_tkhd.cpp\
boxes/atom_avc1.cpp  boxes/atom_ftyp.cpp  boxes/atom_mp4s.cpp  boxes/atom_standard.cpp  boxes/atom_treftype.cpp\
boxes/atom_avcC.cpp  boxes/atom_gmin.cpp  boxes/atom_mp4v.cpp  boxes/atom_stbl.cpp      boxes/atom_trun.cpp\
boxes/atom_chpl.cpp  boxes/atom_hdlr.cpp  boxes/atom_mvhd.cpp  boxes/atom_stdp.cpp\
boxes/atom_d263.cpp  boxes/atom_hinf.cpp  boxes/atom_ohdr.cpp  boxes/atom_stsc.cpp      boxes/atom_udta.cpp\
boxes/atom_damr.cpp  boxes/atom_hnti.cpp  boxes/atom_root.cpp  boxes/atom_stsd.cpp      boxes/atom_url.cpp\
boxes/atom_dref.cpp  boxes/atom_href.cpp  boxes/atom_rtp.cpp   boxes/atom_stsz.cpp      boxes/atom_urn.cpp\
boxes/atom_elst.cpp  boxes/atom_mdat.cpp  boxes/atom_s263.cpp  boxes/atom_stz2.cpp      boxes/atom_video.cpp\
boxes/atom_enca.cpp  boxes/atom_mdhd.cpp  boxes/atom_sdp.cpp   boxes/atom_text.cpp      boxes/atom_vmhd.cpp\
boxes/atom_encv.cpp  boxes/atom_meta.cpp  boxes/atom_smi.cpp   boxes/atom_tfhd.cpp\
boxes/atom_alac.cpp boxes/atom_wave.cpp\
src/3gp.cpp          src/mp4.cpp            src/mp4file.cpp     src/mp4property.cpp     src/odcommands.cpp\
   src/mp4atom.cpp        src/mp4file_io.cpp  src/mp4track.cpp        src/qosqualifiers.cpp\
src/descriptors.cpp  src/mp4container.cpp   src/mp4info.cpp     src/mp4util.cpp         src/rtphint.cpp\
src/isma.cpp         src/mp4descriptor.cpp  src/mp4meta.cpp     src/ocidescriptors.cpp  \
src/nsmp4.cpp
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_CFLAGS := -fexceptions
LOCAL_SHARED_LIBRARIES := nx
include $(BUILD_STATIC_LIBRARY)
endif
