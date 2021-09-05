#
# Copyright (C) 2020 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(origin GO_INCLUDE_DIR),undefined)
  GO_INCLUDE_DIR:=$(dir $(lastword $(MAKEFILE_LIST)))
endif

include $(GO_INCLUDE_DIR)/golang-values.mk


# these variables have the same meanings as in golang-package.mk
GO_HOST_INSTALL_EXTRA?=$(GO_PKG_INSTALL_EXTRA)
GO_HOST_INSTALL_ALL?=$(GO_PKG_INSTALL_ALL)
GO_HOST_SOURCE_ONLY?=$(GO_PKG_SOURCE_ONLY)
GO_HOST_BUILD_PKG?=$(GO_PKG_BUILD_PKG)
GO_HOST_EXCLUDES?=$(GO_PKG_EXCLUDES)
GO_HOST_GO_GENERATE?=$(GO_PKG_GO_GENERATE)
GO_HOST_GCFLAGS?=$(GO_PKG_GCFLAGS)
GO_HOST_LDFLAGS?=$(GO_PKG_LDFLAGS)
GO_HOST_LDFLAGS_X?=$(GO_PKG_LDFLAGS_X)
GO_HOST_TAGS?=$(GO_PKG_TAGS)
GO_HOST_INSTALL_BIN_PATH?=/bin


# need to repeat this here in case golang-package.mk is not included
GO_PKG_BUILD_PKG?=$(strip $(GO_PKG))/...

GO_HOST_WORK_DIR_NAME:=.go_work
GO_HOST_BUILD_DIR=$(HOST_BUILD_DIR)/$(GO_HOST_WORK_DIR_NAME)/build
GO_HOST_BUILD_BIN_DIR=$(GO_HOST_BUILD_DIR)/bin

GO_HOST_BUILD_DEPENDS_PATH:=/share/gocode
GO_HOST_BUILD_DEPENDS_SRC=$(STAGING_DIR_HOSTPKG)$(GO_HOST_BUILD_DEPENDS_PATH)/src

GO_HOST_DIR_NAME:=$(lastword $(subst /,$(space),$(CURDIR)))
GO_HOST_STAGING_DIR:=$(TMP_DIR)/host-stage-$(GO_HOST_DIR_NAME)
GO_HOST_STAGING_FILES_LIST_DIR:=$(HOST_BUILD_PREFIX)/stamp
GO_HOST_BIN_STAGING_FILES_LIST:=$(GO_HOST_STAGING_FILES_LIST_DIR)/$(GO_HOST_DIR_NAME)-bin.list
GO_HOST_SRC_STAGING_FILES_LIST:=$(GO_HOST_STAGING_FILES_LIST_DIR)/$(GO_HOST_DIR_NAME)-src.list

ifeq ($(GO_HOST_PIE_SUPPORTED),1)
  GO_HOST_ENABLE_PIE:=1
endif

GO_HOST_BUILD_CONFIG_VARS= \
	GO_PKG="$(strip $(GO_PKG))" \
	GO_INSTALL_EXTRA="$(strip $(GO_HOST_INSTALL_EXTRA))" \
	GO_INSTALL_ALL="$(strip $(GO_HOST_INSTALL_ALL))" \
	GO_SOURCE_ONLY="$(strip $(GO_HOST_SOURCE_ONLY))" \
	GO_BUILD_PKG="$(strip $(GO_HOST_BUILD_PKG))" \
	GO_EXCLUDES="$(strip $(GO_HOST_EXCLUDES))" \
	GO_GO_GENERATE="$(strip $(GO_HOST_GO_GENERATE))" \
	GO_INSTALL_BIN_PATH="$(strip $(GO_HOST_INSTALL_BIN_PATH))" \
	BUILD_DIR="$(HOST_BUILD_DIR)" \
	GO_BUILD_DIR="$(GO_HOST_BUILD_DIR)" \
	GO_BUILD_BIN_DIR="$(GO_HOST_BUILD_BIN_DIR)" \
	GO_BUILD_DEPENDS_PATH="$(GO_HOST_BUILD_DEPENDS_PATH)" \
	GO_BUILD_DEPENDS_SRC="$(GO_HOST_BUILD_DEPENDS_SRC)"

GO_HOST_MORE_CFLAGS?= \
	-Wformat -Werror=format-security \
	-fstack-protector-strong \
	-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
	-Wl,-z,now -Wl,-z,relro \
	$(if $(GO_HOST_ENABLE_PIE),$(FPIC))

GO_HOST_MORE_LDFLAGS?= \
	-znow -zrelro \
	$(if $(GO_HOST_ENABLE_PIE),$(FPIC) -specs=$(INCLUDE_DIR)/hardened-ld-pie.specs)

GO_HOST_TARGET_VARS= \
	CGO_ENABLED=1 \
	CC=gcc \
	CXX=g++ \
	PKG_CONFIG=pkg-config \
	CGO_CFLAGS="$(HOST_CFLAGS) $(GO_HOST_MORE_CFLAGS)" \
	CGO_CPPFLAGS="$(HOST_CPPFLAGS) $(GO_HOST_MORE_CPPFLAGS)" \
	CGO_CXXFLAGS="$(HOST_CFLAGS) $(GO_HOST_MORE_CFLAGS)" \
	CGO_LDFLAGS="$(HOST_LDFLAGS) $(GO_HOST_MORE_LDFLAGS)" \
	GO_GCC_HELPER_CC="$(HOSTCC)" \
	GO_GCC_HELPER_CXX="$(HOSTCXX)" \
	GO_GCC_HELPER_PATH="$$$$PATH" \
	PATH="$(STAGING_DIR_HOSTPKG)/lib/go-cross/openwrt:$$$$PATH"

GO_HOST_BUILD_VARS= \
	GOPATH="$(GO_HOST_BUILD_DIR)" \
	GOCACHE="$(GO_BUILD_CACHE_DIR)" \
	GOMODCACHE="$(GO_MOD_CACHE_DIR)" \
	GOENV=off

GO_HOST_VARS= \
	$(GO_HOST_TARGET_VARS) \
	$(GO_HOST_BUILD_VARS)

GO_HOST_DEFAULT_LDFLAGS= \
	-linkmode external \
	-extldflags '$(patsubst -z%,-Wl$(comma)-z$(comma)%,$(HOST_LDFLAGS) $(GO_HOST_MORE_LDFLAGS))'

GO_HOST_CUSTOM_LDFLAGS= \
	$(GO_HOST_LDFLAGS) \
	$(patsubst %,-X %,$(GO_HOST_LDFLAGS_X))

GO_HOST_INSTALL_ARGS= \
	-v \
	-ldflags "all=$(GO_HOST_DEFAULT_LDFLAGS)" \
	$(if $(filter $(GO_HOST_ENABLE_PIE),1),-buildmode pie) \
	$(if $(GO_HOST_GCFLAGS),-gcflags "$(GO_HOST_GCFLAGS)") \
	$(if $(GO_HOST_CUSTOM_LDFLAGS),-ldflags "$(GO_HOST_CUSTOM_LDFLAGS) $(GO_HOST_DEFAULT_LDFLAGS)") \
	$(if $(GO_HOST_TAGS),-tags "$(GO_HOST_TAGS)")

define GoHost/Host/Configure
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_HOST_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh configure
endef

# $(1) additional arguments for go command line (optional)
define GoHost/Host/Compile
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_HOST_BUILD_CONFIG_VARS) \
	$(GO_HOST_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh build $(GO_HOST_INSTALL_ARGS) $(1)
endef

define GoHost/Host/Install/Bin
	rm -rf "$(GO_HOST_STAGING_DIR)"
	mkdir -p "$(GO_HOST_STAGING_DIR)" "$(GO_HOST_STAGING_FILES_LIST_DIR)"

	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_HOST_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh install_bin "$(GO_HOST_STAGING_DIR)"

	if [ -f "$(GO_HOST_BIN_STAGING_FILES_LIST)" ]; then \
		"$(SCRIPT_DIR)/clean-package.sh" \
			"$(GO_HOST_BIN_STAGING_FILES_LIST)" \
			"$(1)" ; \
	fi

	cd "$(GO_HOST_STAGING_DIR)" && find ./ > "$(GO_HOST_STAGING_DIR).files"

	$(call locked, \
		mv "$(GO_HOST_STAGING_DIR).files" "$(GO_HOST_BIN_STAGING_FILES_LIST)" && \
		$(CP) "$(GO_HOST_STAGING_DIR)"/* "$(1)/", \
		host-staging-dir \
	)

	rm -rf "$(GO_HOST_STAGING_DIR)"
endef

define GoHost/Host/Install/Src
	rm -rf "$(GO_HOST_STAGING_DIR)"
	mkdir -p "$(GO_HOST_STAGING_DIR)" "$(GO_HOST_STAGING_FILES_LIST_DIR)"

	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_HOST_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh install_src "$(GO_HOST_STAGING_DIR)"

	if [ -f "$(GO_HOST_SRC_STAGING_FILES_LIST)" ]; then \
		"$(SCRIPT_DIR)/clean-package.sh" \
			"$(GO_HOST_SRC_STAGING_FILES_LIST)" \
			"$(1)" ; \
	fi

	cd "$(GO_HOST_STAGING_DIR)" && find ./ > "$(GO_HOST_STAGING_DIR).files"

	$(call locked, \
		mv "$(GO_HOST_STAGING_DIR).files" "$(GO_HOST_SRC_STAGING_FILES_LIST)" && \
		$(CP) "$(GO_HOST_STAGING_DIR)"/* "$(1)/", \
		host-staging-dir \
	)

	rm -rf "$(GO_HOST_STAGING_DIR)"
endef

define GoHost/Host/Install
	$(if $(filter $(GO_HOST_SOURCE_ONLY),1),, \
		$(call GoHost/Host/Install/Bin,$(1)) \
	)
	$(call GoHost/Host/Install/Src,$(1))
endef

define GoHost/Host/Uninstall
	if [ -f "$(GO_HOST_BIN_STAGING_FILES_LIST)" ]; then \
		"$(SCRIPT_DIR)/clean-package.sh" \
			"$(GO_HOST_BIN_STAGING_FILES_LIST)" \
			"$(HOST_BUILD_PREFIX)" ; \
		rm -f "$(GO_HOST_BIN_STAGING_FILES_LIST)" ; \
	fi

	if [ -f "$(GO_HOST_SRC_STAGING_FILES_LIST)" ]; then \
		"$(SCRIPT_DIR)/clean-package.sh" \
			"$(GO_HOST_SRC_STAGING_FILES_LIST)" \
			"$(HOST_BUILD_PREFIX)" ; \
		rm -f "$(GO_HOST_SRC_STAGING_FILES_LIST)" ; \
	fi
endef


ifneq ($(strip $(GO_PKG)),)
  Host/Configure=$(call GoHost/Host/Configure)
  Host/Compile=$(call GoHost/Host/Compile)
  Hooks/HostCompile/Post+=Go/CacheCleanup
  Host/Uninstall=$(call GoHost/Host/Uninstall,$(1))
endif

define GoHostBuild
  Host/Install=$$(call GoHost/Host/Install,$$(1))
endef

define GoBinHostBuild
  Host/Install=$$(call GoHost/Host/Install/Bin,$$(1))
endef

define GoSrcHostBuild
  Host/Install=$$(call GoHost/Host/Install/Src,$$(1))
endef
