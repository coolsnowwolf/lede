#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=brook
PKG_VERSION:=1.0
PKG_RELEASE:=20180227

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Brook is a cross-platform proxy software
	DEPENDS:=
	URL:=https://github.com/txthinking/brook
endef

define Package/$(PKG_NAME)/description
Brook is a cross-platform proxy software
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin

ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ./files/7620n/brook $(1)/usr/bin/
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ./files/ar71xx/brook $(1)/usr/bin/
endif
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/x86/brook $(1)/usr/bin/
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/x86_64/brook $(1)/usr/bin/
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/arm/brook $(1)/usr/bin/
endif
ifeq ($(ARCH),aarch64)
	$(INSTALL_BIN) ./files/armv7/brook $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
