#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

ifeq ($(ARCH),x86_64)
	PKG_ARCH_KCPTUN:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_KCPTUN:=mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_KCPTUN:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_KCPTUN:=386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_KCPTUN:=arm7
endif
ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_KCPTUN:=arm6
endif
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_KCPTUN:=arm5
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_KCPTUN:=arm64
endif

PKG_NAME:=kcptun-client
PKG_VERSION:=20200103
PKG_RELEASE:=2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=kcptun-linux-$(PKG_ARCH_KCPTUN)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/xtaci/kcptun/releases/download/v$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Kcptun is a cross-platform proxy software
	DEPENDS:=
	URL:=https://github.com/xtaci/kcptun
endef

define Package/$(PKG_NAME)/description
  Kcptun is a Fast and Reliable ARQ Protocol tunnel
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client_linux_$(PKG_ARCH_KCPTUN) $(1)/usr/bin/kcptun-client
endef

$(eval $(call BuildPackage,$(PKG_NAME)))