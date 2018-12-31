#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=frp
PKG_VERSION:=0.22.0
PKG_RELEASE:=1

PKG_LICENSE:=Apache-2.0

PKG_SOURCE_URL:=https://codeload.github.com/fatedier/frp/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=frp-$(PKG_VERSION).tar.gz
PKG_HASH:=f20dde034d3c6a3eaf9a5d1a9c62b47c2ae5d27c59c5549cf3c01d8b7099330d

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1

GO_PKG:=github.com/fatedier/frp
GO_PKG_BUILD_PKG:=github.com/fatedier/frp/cmd/frpc

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/frpc
	SECTION:=net
	CATEGORY:=Network
	TITLE:=FRPC Client
	DEPENDS:=$(GO_ARCH_DEPENDS)
	URL:=https://github.com/fatedier/frp
endef

define Package/frpc/description
frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet
endef


define Package/frpc/install
	$(call GoPackage/Package/Install/Bin,$(1))
endef

$(eval $(call GoBinPackage,frpc))
$(eval $(call BuildPackage,frpc))
