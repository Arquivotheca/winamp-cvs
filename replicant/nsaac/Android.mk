LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := nsaac

LOCAL_C_INCLUDES := $(ROOT_REPLICANT) $(LOCAL_PATH)/include
LOCAL_CFLAGS := -fvisibility=hidden

# utilities
LOCAL_SRC_FILES := ADTSHeader.c

#LOCAL_SRC_FILES :=  nsaac.c bitbuffer.c
# base AAC decoder
#LOCAL_SRC_FILES += aacdec/aacdecoder.c aacdec/aac_ram.c aacdec/aac_rom.c aacdec/bitstream.c aacdec/block.c aacdec/channel.c aacdec/channelinfo.c aacdec/conceal.c aacdec/dse.c aacdec/imdct.c aacdec/longblock.c aacdec/pce.c aacdec/pns.c aacdec/pulsedata.c aacdec/shortblock.c aacdec/stereo.c aacdec/streaminfo.c aacdec/tns.c
# SBR/PS decoder
#LOCAL_SRC_FILES += sbrdec/aacPLUScheck.c sbrdec/env_calc.c sbrdec/env_dec.c sbrdec/env_extr.c sbrdec/freq_sca.c sbrdec/huff_dec.c sbrdec/hybrid.c sbrdec/lpp_tran.c sbrdec/ps_bitdec.c sbrdec/ps_dec.c sbrdec/qmf_dec.c sbrdec/sbrdecoder.c sbrdec/sbr_bitb.c sbrdec/sbr_crc.c sbrdec/sbr_dec.c sbrdec/sbr_fft.c sbrdec/sbr_ram.c sbrdec/sbr_rom.c
# utilities
#LOCAL_SRC_FILES += math/cfftn.c math/transcendent.c

#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
