#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iucode-tool
PKG_VERSION:=2.3.1
PKG_RELEASE:=2

PKG_SOURCE:=iucode-tool_$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://gitlab.com/iucode-tool/releases/raw/latest
PKG_HASH:=12b88efa4d0d95af08db05a50b3dcb217c0eb2bfc67b483779e33d498ddb2f95

PKG_BUILD_DEPENDS:=USE_UCLIBC:argp-standalone USE_MUSL:argp-standalone
HOST_BUILD_DEPENDS:=HOST_OS_MACOS:argp-standalone/host

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
PKG_LICENSE:=GPL-2.0

PKG_FLAGS:=nonshared

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/iucode-tool
  SECTION:=utils
  CATEGORY:=Base system
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=@TARGET_x86
  TITLE:=Intel microcode loader
endef

define Package/iucode-tool/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iucode_tool $(1)/usr/bin/
endef

# List of build hosts with working cpuid.h
IUT_NATIVE_HOST_OS_ARCH := \
	linux/x86_64 linux/amd64 linux/i386 linux/i686

IUT_HOST_OS_ARCH := $(call tolower,$(HOST_OS))/$(HOST_ARCH)

# Use cpuid.h compat header if build host does not have working cpuid.h
ifeq ($(filter $(IUT_HOST_OS_ARCH),$(IUT_NATIVE_HOST_OS_ARCH)),)
HOST_CFLAGS += \
	-I$(HOST_BUILD_DIR)/cpuid-compat
endif

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/iucode_tool $(STAGING_DIR_HOST)/bin/iucode_tool
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,iucode-tool))
