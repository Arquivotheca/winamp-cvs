# TODO eventually maybe use ARMv6
ifeq ($(TARGET_ARCH_ABI),armeabi)
PLATFORM_NAME := ARMv5
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
PLATFORM_NAME := ARMv7
endif

ifeq ($(TARGET_ARCH_ABI),x86)
PLATFORM_NAME := x86
endif
