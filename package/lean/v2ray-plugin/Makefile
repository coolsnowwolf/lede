#
# Copyright (C) 2020 SharerMax
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray-plugin
PKG_VERSION:=1.3.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/v2ray-plugin-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/shadowsocks/v2ray-plugin/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=dfb86cd8d9be86e665c4b86b68cd7037e4310de001656eef01ec9aeea71edd10

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=madeye <max.c.lv@gmail.com>

GO_PKG:=github.com/shadowsocks/v2ray-plugin
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk


define Package/v2ray-plugin
	SECTION:=net
	CATEGORY:=Network
	TITLE:=SIP003 plugin for shadowsocks, based on v2ray
	URL:=https://github.com/shadowsocks/v2ray-plugin
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/v2ray-plugin/description
	Yet another SIP003 plugin for shadowsocks, based on v2ray
endef

define Build/Prepare
	$(call Build/Prepare/Default)
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
endef

define Package/v2ray-plugin/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ray-plugin $(1)/usr/bin/v2ray-plugin
endef
$(eval $(call GoBinPackage,v2ray-plugin))
$(eval $(call BuildPackage,v2ray-plugin))
