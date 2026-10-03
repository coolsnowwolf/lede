#
# Copyright (C) 2021 Jo-Philipp Wich <jo@mein.io>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=firewall4
PKG_RELEASE:=5

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firewall4.git
PKG_SOURCE_DATE:=2024-12-18
PKG_SOURCE_VERSION:=18fc0ead19faf06b8ce7ec5be84957278e942dfa
PKG_MIRROR_HASH:=790c846317645e2325ca41c9e1d8bec6f1a4af2959cd33ac710de30b2b86f8cd
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

include $(INCLUDE_DIR)/package.mk

define Package/firewall4
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=OpenWrt 4th gen firewall
  DEPENDS:= \
	+kmod-nft-core +kmod-nft-fib +kmod-tcp-bbr +kmod-nft-offload \
	+kmod-nft-nat +kmod-nft-nat6 +kmod-nft-fullcone \
	+nftables-json \
	+ucode +ucode-mod-fs +ucode-mod-ubus +ucode-mod-uci
  EXTRA_DEPENDS:=ucode (>=2022.03.22)
  PROVIDES:=uci-firewall
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
