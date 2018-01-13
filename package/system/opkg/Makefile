#
# Copyright (C) 2006-2015 OpenWrt.org
# Copyright (C) 2016-2017 LEDE Project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=opkg
PKG_RELEASE:=2
PKG_FLAGS:=essential

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.openwrt.org/project/opkg-lede.git
PKG_SOURCE_DATE:=2017-12-07
PKG_SOURCE_VERSION:=3b417b9f41b4ceb5912d82f867dd5534e5675b5c
PKG_MIRROR_HASH:=1c9c21e56186345d0034d15051c1d9c5ebb5aa6658d6ae8886d8403207a914a3

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

# Extend depends from version.mk
PKG_CONFIG_DEPENDS += \
	CONFIG_SIGNED_PACKAGES \
	CONFIG_TARGET_INIT_PATH

PKG_BUILD_PARALLEL:=1
HOST_BUILD_PARALLEL:=1
PKG_INSTALL:=1

HOST_BUILD_DEPENDS:=libubox/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/opkg
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=opkg package manager
  DEPENDS:=+uclient-fetch +libpthread +libubox
  URL:=$(PKG_SOURCE_URL)
  MENU:=1
endef

define Package/opkg/description
  Lightweight package management system
  opkg is the opkg Package Management System, for handling
  installation and removal of packages on a system. It can
  recursively follow dependencies and download all packages
  necessary to install a particular package.

  opkg knows how to install both .ipk and .deb packages.
endef

define Package/opkg/conffiles
/etc/opkg.conf
/etc/opkg/keys/
/etc/opkg/customfeeds.conf
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections
EXTRA_CFLAGS += $(TARGET_CPPFLAGS)

CMAKE_OPTIONS += \
	-DBUILD_TESTS=OFF \
	-DHOST_CPU=$(PKGARCH) \
	-DPATH_SPEC="$(TARGET_INIT_PATH)" \
	-DVERSION="$(PKG_SOURCE_VERSION) ($(PKG_SOURCE_DATE))"

CMAKE_HOST_OPTIONS += \
	-DSTATIC_UBOX=ON \
	-DBUILD_TESTS=OFF \
	-DHOST_CPU=$(PKGARCH) \
	-DLOCK_FILE=/tmp/opkg.lock \
	-DVERSION="$(PKG_SOURCE_VERSION) ($(PKG_SOURCE_DATE))"

define Package/opkg/install
	$(INSTALL_DIR) $(1)/usr/lib/opkg
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/opkg
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/customfeeds.conf $(1)/etc/opkg/customfeeds.conf
	$(INSTALL_DATA) ./files/opkg$(2).conf $(1)/etc/opkg.conf
	$(INSTALL_BIN) ./files/20_migrate-feeds $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/opkg-cl $(1)/bin/opkg
  ifneq ($(CONFIG_SIGNED_PACKAGES),)
	echo "option check_signature 1" >> $(1)/etc/opkg.conf
  endif
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/opkg-key $(1)/usr/sbin/
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/opkg-cl $(STAGING_DIR_HOST)/bin/opkg
endef

$(eval $(call BuildPackage,opkg))
$(eval $(call HostBuild))
