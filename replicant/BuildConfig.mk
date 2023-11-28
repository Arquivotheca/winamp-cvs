# the build configuration stuff assumes that undefined values are "0" (meaning to build them).  I'm just putting the := 0 so we know that the configuration option is available.

# TODO: If I can figure out how to make functions in this thing, we might be able to combine the APP_CFLAGS and the define in one line

# settings that effect more than one plugin
REPLICANT_NO_MP4 := 0
REPLICANT_NO_ULTRAVOX := 1
REPLICANT_NO_ICY := 0
REPLICANT_NO_HTTP := 0
REPLICANT_NO_OGG := 1
REPLICANT_NO_FILE := 0

# individual plugins
REPLICANT_NO_AAC := 0
REPLICANT_NO_FLAC := 0

# set preprocessor flags according to the settings
ifeq ($(REPLICANT_NO_MP4), 1)
APP_CFLAGS += -DREPLICANT_NO_MP4
endif

ifeq ($(REPLICANT_NO_ULTRAVOX), 1)
APP_CFLAGS += -DREPLICANT_NO_ULTRAVOX
endif

ifeq ($(REPLICANT_NO_ICY), 1)
APP_CFLAGS += -DREPLICANT_NO_ICY
endif

ifeq ($(REPLICANT_NO_HTTP), 1)
APP_CFLAGS += -DREPLICANT_NO_HTTP
endif

ifeq ($(REPLICANT_NO_OGG), 1)
APP_CFLAGS += -DREPLICANT_NO_OGG
endif

ifeq ($(REPLICANT_NO_AAC), 1)
APP_CFLAGS += -DREPLICANT_NO_AAC
endif

ifeq ($(REPLICANT_NO_FLAC), 1)
APP_CFLAGS += -DREPLICANT_NO_FLAC
endif

ifeq ($(REPLICANT_NO_FILE), 1)
APP_CFLAGS += -DREPLICANT_NO_FILE
endif
