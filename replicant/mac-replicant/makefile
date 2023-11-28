#
# CM Build
#

TRACE_ENABLED = 1

CMPRODUCT ?= replicant_mac
CMMAJOR ?= 1
CMMINOR ?= 0
CMRELEASE ?= 0
CMSPIN ?= 0
CMPRODVER ?= $(CMMAJOR).$(CMMINOR).$(CMRELEASE)

ifndef TOP
	TOP_DIR:=$(shell pwd)/../..
else
	TOP_DIR:=$(TOP)
endif



BUILD_VERSION:=$(CMMAJOR)
ifneq ($(CMMINOR),0)
	BUILD_VERSION:="$(BUILD_VERSION).$(CMMINOR)"
endif
ifneq ($(CMRELEASE),0)
	BUILD_VERSION:="$(BUILD_VERSION).$(CMRELEASE)"
endif

BUILD_NUMBER_HEX := $(shell printf "0x%02X%02X%02X%02X" $(CMMAJOR) $(CMMINOR) $(CMRELEASE) $(CMSPIN))
BUILD_DATE = $(shell date -u "+%s")

export NVG_PRODUCT_BUILD_NUMBER=$(BUILD_NUMBER_HEX)
export NVG_PRODUCT_BUILD_DATE=$(BUILD_DATE)

PRODUCT_VERSION:=$(CMPRODVER)
PRODUCT_NAME:=$(CMPRODUCT)

BUILD_DIR=$(TOP_DIR)/build
PRODUCT_BUILD_DIR = $(BUILD_DIR)/$(PRODUCT_NAME)

ifndef DEVELOPER_DIR
	DEVELOPER_DIR:=$(shell xcode-select -print-path)
else
	ifeq "$(wildcard $(DEVELOPER_DIR) )" ""
		DEVELOPER_DIR:=$(DEVELOPER_DIR).app/Contents/Developer
	endif
endif

 
XCODEBUILD := "$(DEVELOPER_DIR)/usr/bin/xcodebuild"
SCANBUILD := "scan-build"
PRODUCTBUILD = "/usr/bin/productbuild"
TAR = "/usr/bin/tar"
PLISTBUDDY = "/usr/libexec/PlistBuddy"
NSBUILD_TOOLS_DIR=$(TOP_DIR)/build-tools
NSBUILD_TOOLS_BIN_DIR=$(NSBUILD_TOOLS_DIR)/bin
NVGTOOL_BIN=$(NSBUILD_TOOLS_BIN_DIR)/nvgtool

XCODE_BUILD_ACTIONS= 
# ifdef CMBUILD_REQUEST_ID
  	XCODE_BUILD_ACTIONS += clean 
# endif
XCODE_BUILD_ACTIONS += install
	
ifdef CMBUILD_REQUEST_ID
	PERFORM_CODESIGN = "yes"
else
	PERFORM_CODESIGN = "no"
endif

SYMROOT:=$(PRODUCT_BUILD_DIR)
OBJROOT:=$(PRODUCT_BUILD_DIR)/intermediate
DSTROOT:=$(PRODUCT_BUILD_DIR)/install
REPORTROOT:=$(PRODUCT_BUILD_DIR)/reports
PKGROOT:=$(BUILD_DIR)/packages

INSTALL_PATH_PREFIX=/.

PACKAGE_NAME=$(PRODUCT_NAME)-$(PRODUCT_VERSION)

PRODUCTBUILD_SIGN_PARAM=
ifeq ($(PERFORM_CODESIGN), "yes")				
	RRODUCTBUILD_SIGN_PARAM += --sign "3rd Party Mac Developer Installer: AOL, Inc."
endif
   	
define Trace
	if [ "$(TRACE_ENABLED)" == "1" ] ; then \
   		echo "-------------------  $(1)  -------------------";\
	fi
endef

#########################################################################################
.PHONY : build

build:
	
	rm -rf "$(PRODUCT_BUILD_DIR)"
	mkdir -p "$(PRODUCT_BUILD_DIR)"
	mkdir -p "$(SYMROOT)"
	mkdir -p "$(OBJROOT)"	
	mkdir -p "$(DSTROOT)"
	mkdir -p "$(PKGROOT)"
		

	@$(call Trace,"Building targets")
	$(XCODEBUILD) -project ./mac-replicant.xcodeproj \
				  -scheme mac-replicant \
				  -parallelizeTargets \
				  -configuration Release \
				  $(XCODE_BUILD_ACTIONS) \
				  SYMROOT=$(SYMROOT) \
				  OBJROOT=$(OBJROOT) \
				  DSTROOT=$(DSTROOT) \
				  INSTALL_PATH_PREFIX=$(INSTALL_PATH_PREFIX) \
				 
	
	@$(call Trace,"Creating product package [with symbols]")
	cd "$(DSTROOT)/$(INSTALL_PATH)"; \
	$(TAR) -czf $(PKGROOT)/$(PACKAGE_NAME).tar.gz ./*
	

#########################################################################################
.PHONY : analyze

analyze:

	@$(call Trace,"Analyzing targets")
	$(SCANBUILD) -o "$(REPORTROOT)" $(XCODEBUILD) -project ./mac-replicant.xcodeproj \
                                                  -scheme mac-replicant \
                                                  -parallelizeTargets \
                                                  -configuration Debug \
                                                  clean build \
												  CLANG_STATIC_ANALYZER_MODE=YES


#########################################################################################	
.PHONY : clean

clean:
	@echo Cleaning...
	rm -rf $(PRODUCT_DIR)
