#
# Copyright (C) 2021 Jo-Philipp Wich <jo@mein.io>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=firewall4
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firewall4.git
PKG_SOURCE_DATE:=2021-03-31
PKG_SOURCE_VERSION:=29fba840201287b9265888adba6298779b750af5
PKG_MIRROR_HASH:=1d26a611aeecdf37f09e4cdee6b192e5da087abf6e0fc7a9ca97a80e58d14222
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

include $(INCLUDE_DIR)/package.mk

define Package/firewall4
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=OpenWrt 4th gen firewall
  DEPENDS:=+ucode +ucode-mod-fs +ucode-mod-uci +ucode-mod-ubus +kmod-nft-core +kmod-nft-fib +kmod-nft-nat +kmod-nft-nat6 +nftables-json
  CONFLICTS:=firewall kmod-ipt-nat
endef

define Package/firewall4/description
 This package provides an nftables-based implementation of the UCI firewall
 sharing the same configuration format.
endef

define Package/firewall4/conffiles
/etc/config/firewall
/etc/nftables.d/
endef

define Package/firewall4/install
	$(CP) -a $(PKG_BUILD_DIR)/root/* $(1)/
endef

define Build/Compile
endef

$(eval $(call BuildPackage,firewall4))
