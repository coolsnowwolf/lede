# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nftables
PKG_VERSION:=1.1.6
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://netfilter.org/projects/$(PKG_NAME)/files
PKG_HASH:=372931bda8556b310636a2f9020adc710f9bab66f47efe0ce90bff800ac2530c

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_BUILD_FLAGS:=lto

include $(INCLUDE_DIR)/package.mk

DISABLE_NLS:=

CONFIGURE_ARGS += \
        --disable-debug \
        --disable-man-doc \
        --with-mini-gmp \
        --without-cli \
        --disable-python

define Package/nftables/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=nftables userspace utility
  DEPENDS:=+kmod-nft-core +libnftnl +kmod-nft-fullcone
  URL:=http://netfilter.org/projects/nftables/
  PROVIDES:=nftables
endef

define Package/nftables-nojson
  $(Package/nftables/Default)
  TITLE+= no JSON support
  VARIANT:=nojson
  DEFAULT_VARIANT:=1
  CONFLICTS:=nftables-json
endef

define Package/nftables-json
  $(Package/nftables/Default)
  TITLE+= with JSON support
  VARIANT:=json
  DEPENDS+=+jansson
endef

ifeq ($(BUILD_VARIANT),json)
  CONFIGURE_ARGS += --with-json
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/nftables $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnftables.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/nftables/install/Default
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/nft $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

Package/nftables-nojson/install = $(Package/nftables/install/Default)
Package/nftables-json/install = $(Package/nftables/install/Default)

$(eval $(call BuildPackage,nftables-nojson))
$(eval $(call BuildPackage,nftables-json))
