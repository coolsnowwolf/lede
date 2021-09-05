#
# Copyright (C) 2018-2020 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(origin GO_INCLUDE_DIR),undefined)
  GO_INCLUDE_DIR:=$(dir $(lastword $(MAKEFILE_LIST)))
endif

include $(GO_INCLUDE_DIR)/golang-values.mk


# Variables (all optional, except GO_PKG) to be set in package
# Makefiles:
#
# GO_PKG (required) - name of Go package
#
#   Go name of the package.
#
#   e.g. GO_PKG:=golang.org/x/text
#
#
# GO_PKG_INSTALL_EXTRA - list of regular expressions, default empty
#
#   Additional files/directories to install. By default, only these
#   files are installed:
#
#   * Files with one of these extensions:
#     .go, .c, .cc, .cpp, .h, .hh, .hpp, .proto, .s
#
#   * Files in any 'testdata' directory
#
#   * go.mod and go.sum, in any directory
#
#   e.g. GO_PKG_INSTALL_EXTRA:=example.toml marshal_test.toml
#
#
# GO_PKG_INSTALL_ALL - boolean (0 or 1), default false
#
#   If true, install all files regardless of extension or directory.
#
#   e.g. GO_PKG_INSTALL_ALL:=1
#
#
# GO_PKG_SOURCE_ONLY - boolean (0 or 1), default false
#
#   If true, 'go install' will not be called. If the package does not
#   (or should not) build any binaries, then specifying this option will
#   save build time.
#
#   e.g. GO_PKG_SOURCE_ONLY:=1
#
#
# GO_PKG_BUILD_PKG - list of build targets, default GO_PKG/...
#
#   Build targets for compiling this Go package, i.e. arguments passed
#   to 'go install'.
#
#   e.g. GO_PKG_BUILD_PKG:=github.com/debian/ratt/cmd/...
#
#
# GO_PKG_EXCLUDES - list of regular expressions, default empty
#
#   Patterns to exclude from the build targets expanded from
#   GO_PKG_BUILD_PKG.
#
#   e.g. GO_PKG_EXCLUDES:=examples/
#
#
# GO_PKG_GO_GENERATE - boolean (0 or 1), default false
#
#   If true, 'go generate' will be called on all build targets (as
#   determined by GO_PKG_BUILD_PKG and GO_PKG_EXCLUDES). This is usually
#   not necessary.
#
#   e.g. GO_PKG_GO_GENERATE:=1
#
#
# GO_PKG_GCFLAGS - list of options, default empty
#
#   Additional go tool compile options to use when building targets.
#
#   e.g. GO_PKG_GCFLAGS:=-N -l
#
#
# GO_PKG_LDFLAGS - list of options, default empty
#
#   Additional go tool link options to use when building targets.
#
#   Note that the OpenWrt build system has an option to strip binaries
#   (enabled by default), so -s (Omit the symbol table and debug
#   information) and -w (Omit the DWARF symbol table) flags are not
#   necessary.
#
#   e.g. GO_PKG_LDFLAGS:=-r dir1:dir2 -u
#
#
# GO_PKG_LDFLAGS_X - list of string variable definitions, default empty
#
#   Each definition will be passed as the parameter to the -X go tool
#   link option, i.e. -ldflags "-X importpath.name=value".
#
#   e.g. GO_PKG_LDFLAGS_X:=main.Version=$(PKG_VERSION) main.BuildStamp=$(SOURCE_DATE_EPOCH)
#
#
# GO_PKG_TAGS - list of build tags, default empty
#
#   Build tags to consider satisfied during the build, passed as the
#   parameter to the -tags option for 'go install'.
#
#   e.g. GO_PKG_TAGS:=release,noupgrade
#
#
# GO_PKG_INSTALL_BIN_PATH - target directory path, default /usr/bin
#
#   Directory path under "dest_dir" where binaries will be installed by
#   '$(call GoPackage/Package/Install/Bin,dest_dir)'.
#
#   e.g. GO_PKG_INSTALL_BIN_PATH:=/sbin

# Credit for this package build process (GoPackage/Build/Configure and
# GoPackage/Build/Compile) belong to Debian's dh-golang completely.
# https://salsa.debian.org/go-team/packages/dh-golang


GO_PKG_BUILD_PKG?=$(strip $(GO_PKG))/...
GO_PKG_INSTALL_BIN_PATH?=/usr/bin

GO_PKG_WORK_DIR_NAME:=.go_work
GO_PKG_BUILD_DIR=$(PKG_BUILD_DIR)/$(GO_PKG_WORK_DIR_NAME)/build
GO_PKG_BUILD_BIN_DIR=$(GO_PKG_BUILD_DIR)/bin$(if $(GO_HOST_TARGET_DIFFERENT),/$(GO_OS_ARCH))

GO_PKG_BUILD_DEPENDS_PATH:=/usr/share/gocode
GO_PKG_BUILD_DEPENDS_SRC=$(STAGING_DIR)$(GO_PKG_BUILD_DEPENDS_PATH)/src

ifdef CONFIG_PKG_ASLR_PIE_ALL
  ifeq ($(strip $(PKG_ASLR_PIE)),1)
    ifeq ($(GO_TARGET_PIE_SUPPORTED),1)
      GO_PKG_ENABLE_PIE:=1
    else
      $(warning PIE buildmode is not supported for $(GO_OS)/$(GO_ARCH))
    endif
  endif
endif

ifdef CONFIG_PKG_ASLR_PIE_REGULAR
  ifeq ($(strip $(PKG_ASLR_PIE_REGULAR)),1)
    ifeq ($(GO_TARGET_PIE_SUPPORTED),1)
      GO_PKG_ENABLE_PIE:=1
    else
      $(warning PIE buildmode is not supported for $(GO_OS)/$(GO_ARCH))
    endif
  endif
endif

ifdef CONFIG_GOLANG_SPECTRE
  ifeq ($(GO_TARGET_SPECTRE_SUPPORTED),1)
    GO_PKG_ENABLE_SPECTRE:=1
  else
    $(warning Spectre mitigations are not supported for $(GO_ARCH))
  endif
endif

# sstrip causes corrupted section header size
ifneq ($(CONFIG_USE_SSTRIP),)
  ifneq ($(CONFIG_DEBUG),)
    GO_PKG_STRIP_ARGS:=--strip-unneeded --remove-section=.comment --remove-section=.note
  else
    GO_PKG_STRIP_ARGS:=--strip-all
  endif
  STRIP:=$(TARGET_CROSS)strip $(GO_PKG_STRIP_ARGS)
endif

define GoPackage/GoSubMenu
  SUBMENU:=Go
  SECTION:=lang
  CATEGORY:=Languages
endef

GO_PKG_BUILD_CONFIG_VARS= \
	GO_PKG="$(strip $(GO_PKG))" \
	GO_INSTALL_EXTRA="$(strip $(GO_PKG_INSTALL_EXTRA))" \
	GO_INSTALL_ALL="$(strip $(GO_PKG_INSTALL_ALL))" \
	GO_SOURCE_ONLY="$(strip $(GO_PKG_SOURCE_ONLY))" \
	GO_BUILD_PKG="$(strip $(GO_PKG_BUILD_PKG))" \
	GO_EXCLUDES="$(strip $(GO_PKG_EXCLUDES))" \
	GO_GO_GENERATE="$(strip $(GO_PKG_GO_GENERATE))" \
	GO_INSTALL_BIN_PATH="$(strip $(GO_PKG_INSTALL_BIN_PATH))" \
	BUILD_DIR="$(PKG_BUILD_DIR)" \
	GO_BUILD_DIR="$(GO_PKG_BUILD_DIR)" \
	GO_BUILD_BIN_DIR="$(GO_PKG_BUILD_BIN_DIR)" \
	GO_BUILD_DEPENDS_PATH="$(GO_PKG_BUILD_DEPENDS_PATH)" \
	GO_BUILD_DEPENDS_SRC="$(GO_PKG_BUILD_DEPENDS_SRC)"

GO_PKG_TARGET_VARS= \
	GOOS="$(GO_OS)" \
	GOARCH="$(GO_ARCH)" \
	GO386="$(GO_386)" \
	GOARM="$(GO_ARM)" \
	GOMIPS="$(GO_MIPS)" \
	GOMIPS64="$(GO_MIPS64)" \
	CGO_ENABLED=1 \
	CC="$(TARGET_CC)" \
	CXX="$(TARGET_CXX)" \
	CGO_CFLAGS="$(filter-out $(GO_CFLAGS_TO_REMOVE),$(TARGET_CFLAGS))" \
	CGO_CPPFLAGS="$(TARGET_CPPFLAGS)" \
	CGO_CXXFLAGS="$(filter-out $(GO_CFLAGS_TO_REMOVE),$(TARGET_CXXFLAGS))" \
	CGO_LDFLAGS="$(TARGET_LDFLAGS)"

GO_PKG_BUILD_VARS= \
	GOPATH="$(GO_PKG_BUILD_DIR)" \
	GOCACHE="$(GO_BUILD_CACHE_DIR)" \
	GOMODCACHE="$(GO_MOD_CACHE_DIR)" \
	GOENV=off

GO_PKG_VARS= \
	$(GO_PKG_TARGET_VARS) \
	$(GO_PKG_BUILD_VARS)

GO_PKG_DEFAULT_GCFLAGS= \
	$(if $(GO_PKG_ENABLE_SPECTRE),-spectre all)

GO_PKG_DEFAULT_ASMFLAGS= \
	$(if $(GO_PKG_ENABLE_SPECTRE),-spectre all)

GO_PKG_DEFAULT_LDFLAGS= \
	-buildid '$(SOURCE_DATE_EPOCH)' \
	-linkmode external \
	-extldflags '$(patsubst -z%,-Wl$(comma)-z$(comma)%,$(TARGET_LDFLAGS))'

GO_PKG_CUSTOM_LDFLAGS= \
	$(GO_PKG_LDFLAGS) \
	$(patsubst %,-X %,$(GO_PKG_LDFLAGS_X))

GO_PKG_INSTALL_ARGS= \
	-v \
	-trimpath \
	-ldflags "all=$(GO_PKG_DEFAULT_LDFLAGS)" \
	$(if $(GO_PKG_DEFAULT_GCFLAGS),-gcflags "all=$(GO_PKG_DEFAULT_GCFLAGS)") \
	$(if $(GO_PKG_DEFAULT_ASMFLAGS),-asmflags "all=$(GO_PKG_DEFAULT_ASMFLAGS)") \
	$(if $(filter $(GO_PKG_ENABLE_PIE),1),-buildmode pie) \
	$(if $(filter $(GO_ARCH),arm),-installsuffix "v$(GO_ARM)") \
	$(if $(filter $(GO_ARCH),mips mipsle),-installsuffix "$(GO_MIPS)") \
	$(if $(filter $(GO_ARCH),mips64 mips64le),-installsuffix "$(GO_MIPS64)") \
	$(if $(GO_PKG_GCFLAGS),-gcflags "$(GO_PKG_GCFLAGS) $(GO_PKG_DEFAULT_GCFLAGS)") \
	$(if $(GO_PKG_CUSTOM_LDFLAGS),-ldflags "$(GO_PKG_CUSTOM_LDFLAGS) $(GO_PKG_DEFAULT_LDFLAGS)") \
	$(if $(GO_PKG_TAGS),-tags "$(GO_PKG_TAGS)")

define GoPackage/Build/Configure
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_PKG_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh configure
endef

# $(1) additional arguments for go command line (optional)
define GoPackage/Build/Compile
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_PKG_BUILD_CONFIG_VARS) \
	$(GO_PKG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh build $(GO_PKG_INSTALL_ARGS) $(1)
endef

define GoPackage/Build/InstallDev
	$(call GoPackage/Package/Install/Src,$(1))
endef

define GoPackage/Package/Install/Bin
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_PKG_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh install_bin "$(1)"
endef

define GoPackage/Package/Install/Src
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(GO_PKG_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh install_src "$(1)"
endef

define GoPackage/Package/Install
	$(if $(filter $(GO_PKG_SOURCE_ONLY),1),, \
		$(call GoPackage/Package/Install/Bin,$(1)) \
	)
	$(call GoPackage/Package/Install/Src,$(1))
endef


ifneq ($(strip $(GO_PKG)),)
  ifeq ($(GO_TARGET_SPECTRE_SUPPORTED),1)
    PKG_CONFIG_DEPENDS+=CONFIG_GOLANG_SPECTRE
  endif

  Build/Configure=$(call GoPackage/Build/Configure)
  Build/Compile=$(call GoPackage/Build/Compile)
  Hooks/Compile/Post+=Go/CacheCleanup
  Build/InstallDev=$(call GoPackage/Build/InstallDev,$(1))
endif

define GoPackage
  ifndef Package/$(1)/install
    Package/$(1)/install=$$(call GoPackage/Package/Install,$$(1))
  endif
endef

define GoBinPackage
  ifndef Package/$(1)/install
    Package/$(1)/install=$$(call GoPackage/Package/Install/Bin,$$(1))
  endif
endef

define GoSrcPackage
  ifndef Package/$(1)/install
    Package/$(1)/install=$$(call GoPackage/Package/Install/Src,$$(1))
  endif
endef


# Deprecated variables - these will be removed after the next OpenWrt release
GO_PKG_PATH=$(GO_PKG_BUILD_DEPENDS_PATH)
GO_PKG_WORK_DIR=$(PKG_BUILD_DIR)/$(GO_PKG_WORK_DIR_NAME)
GO_PKG_CACHE_DIR=$(GO_BUILD_CACHE_DIR)
GO_PKG_DEFAULT_VARS=$(GO_PKG_VARS)
GoPackage/Environment=$(GO_PKG_VARS)
GoPackage/is_dir_not_empty=$$$$($(FIND) "$(1)" -maxdepth 0 -type d \! -empty 2>/dev/null)
GoPackage/has_binaries=$(call GoPackage/is_dir_not_empty,$(GO_PKG_BUILD_BIN_DIR))
# End of deprecated variables
