#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

ifeq ($(ARCH),x86_64)
	PKG_ARCH_SYNCTHING:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_SYNCTHING:=mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_SYNCTHING:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_SYNCTHING:=386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_SYNCTHING:=arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_SYNCTHING:=arm64
endif

PKG_NAME:=syncthing
PKG_VERSION:=v1.3.3
PKG_RELEASE:=2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-linux-$(PKG_ARCH_SYNCTHING)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/syncthing/syncthing/releases/download/$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A continuous file synchronization program
	DEPENDS:=
	URL:=https://github.com/syncthing/syncthing
endef

define Package/$(PKG_NAME)/description
  Syncthing is a continuous file synchronization program.
endef

define Build/Prepare
	tar -xzvf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME)-linux-$(PKG_ARCH_SYNCTHING)-$(PKG_VERSION)/syncthing $(1)/usr/bin/syncthing
endef

$(eval $(call BuildPackage,$(PKG_NAME)))