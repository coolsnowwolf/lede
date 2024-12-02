#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
PKG_VERSION:=3.30.5
PKG_VERSION_MAJOR:=$(word 1,$(subst ., ,$(PKG_VERSION))).$(word 2,$(subst ., ,$(PKG_VERSION)))
PKG_RELEASE:=1
PKG_CPE_ID:=cpe:/a:kitware:cmake

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/Kitware/CMake/releases/download/v$(PKG_VERSION)/ \
		https://cmake.org/files/v$(PKG_VERSION_MAJOR)/
PKG_HASH:=9f55e1a40508f2f29b7e065fa08c29f82c402fa0402da839fffe64a25755a86d

HOST_BUILD_PARALLEL:=1
HOST_CONFIGURE_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	CC="$(HOSTCC_NOCACHE)" \
	CXX="$(HOSTCXX_NOCACHE)" \
	MAKEFLAGS="$(HOST_JOBS)" \
	CXXFLAGS="$(HOST_CFLAGS)" \
	MAKE="$(STAGING_DIR_HOST)/bin/ninja"

HOST_CONFIGURE_ARGS := \
	--no-debugger \
	$(if $(MAKE_JOBSERVER),--parallel="$(MAKE_JOBSERVER)") \
	--prefix="$(STAGING_DIR_HOST)" \
	--system-expat \
	--system-liblzma \
	--system-zlib \
	--system-zstd \
	--generator=Ninja

define Host/Compile/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) $(1)
endef

define Host/Install/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) install
endef

define Host/Uninstall/Default
	+$(NINJA) -C $(HOST_BUILD_DIR) uninstall
endef

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  HOST_MAKE_FLAGS += VERBOSE=1
endif

$(eval $(call HostBuild))
