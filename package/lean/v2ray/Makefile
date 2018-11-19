#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=4.5.0
PKG_RELEASE:=2

ifeq ($(ARCH),x86_64)
	PKG_ARCH_V2RAY:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_V2RAY:=mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_V2RAY:=mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_V2RAY:=386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_V2RAY:=arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_V2RAY:=arm64
endif

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=V2Ray is a cross-platform proxy software
	DEPENDS:=+ca-certificates
	URL:=https://github.com/v2ray/v2ray-core
endef

define Package/$(PKG_NAME)/description
V2Ray is a cross-platform proxy software
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/v2ray
	$(INSTALL_BIN) ./files/$(PKG_ARCH_V2RAY)/v2ray $(1)/usr/bin/v2ray/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
