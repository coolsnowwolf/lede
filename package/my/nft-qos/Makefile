#
# Copyright (C) 2018 rosysong@rosinson.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nft-qos
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Rosy Song <rosysong@rosinson.com>

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/nft-qos
  SECTION:=utils
  CATEGORY:=Base system
  DEPENDS:=+nftables
  TITLE:=QoS scripts over nftables
  PKGARCH:=all
endef

define Package/nft-qos/description
 This package provides implementation for qos over nftables.
 Currently, it is heavily devices-oriented.
endef

define Package/nft-qos/conffiles
/etc/config/nft-qos
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/nft-qos/install
	$(INSTALL_DIR) $(1)/lib/nft-qos
	$(INSTALL_DATA) ./files/nft-qos.sh $(1)/lib/nft-qos/nft-qos.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/nft-qos.config $(1)/etc/config/nft-qos
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nft-qos.init $(1)/etc/init.d/nft-qos
endef

$(eval $(call BuildPackage,nft-qos))
