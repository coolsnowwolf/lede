#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=frpc
PKG_VERSION:=0.27.0
PKG_RELEASE:=1

ifeq ($(ARCH),mipsel)
	FRPC_ARCH:=mipsle
endif
ifeq ($(ARCH),mips)
	FRPC_ARCH:=mips
endif
ifeq ($(ARCH),i386)
	FRPC_ARCH:=386
endif
ifeq ($(ARCH),x86_64)
	FRPC_ARCH:=amd64
endif
ifeq ($(ARCH),arm)
	FRPC_ARCH:=arm
endif
ifeq ($(ARCH),aarch64)
	FRPC_ARCH:=arm64
endif

PKG_LICENSE:=Apache-2.0

PKG_SOURCE_URL:=https://github.com/fatedier/frp/releases/download/v$(PKG_VERSION)
PKG_SOURCE:=frp_$(PKG_VERSION)_linux_$(FRPC_ARCH).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/frp_$(PKG_VERSION)_linux_$(FRPC_ARCH)
PKG_HASH:=skip

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
	$(PKG_UNPACK)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/frpc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))