# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nftables
PKG_VERSION:=0.7
PKG_RELEASE:=1

PKG_SOURCE_URL:=git://git.netfilter.org/nftables
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=79cbd19e7437680561b26109bbf4f48cb2e8e0a7
PKG_MIRROR_HASH:=6b06a114c5a1b397b0ae616d5d292859611b0eb05b647182b3c00421909338b8
PKG_MAINTAINER:=Steven Barth <steven@midlink.org>
PKG_LICENSE:=GPL-2.0

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS += \
        --with-mini-gmp \
        --without-cli \

define Package/nftables
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=nftables packet filtering userspace utility
  DEPENDS:=+kmod-nft-core +kmod-nft-nat +libnftnl
  URL:=http://netfilter.org/projects/nftables/
endef

define Package/nftables/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/nft $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,nftables))
