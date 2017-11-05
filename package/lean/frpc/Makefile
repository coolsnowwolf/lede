#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=frpc
PKG_VERSION:=0.13
PKG_RELEASE:=20171022

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=FRPC Client
	DEPENDS:=
	URL:=https://github.com/fatedier/frp/releases
endef

define Package/$(PKG_NAME)/description
frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet
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
	$(INSTALL_BIN) ./files/7620n/frpc $(1)/usr/bin/
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ./files/ar71xx/frpc $(1)/usr/bin/
endif
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/x86/frpc $(1)/usr/bin/
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/x86_64/frpc $(1)/usr/bin/
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/arm/frpc $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
