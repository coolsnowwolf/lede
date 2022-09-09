#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ethtool
PKG_VERSION:=5.19
PKG_RELEASE:=1

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/network/ethtool
PKG_HASH:=3b752a3329827907ac3812f2831dfecf51c8c41c55d2d69cfb9c53ca06449fc6

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ethtool
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Display or change ethernet card settings
  URL:=http://www.kernel.org/pub/software/network/ethtool/
  VARIANT:=tiny
  CONFLICTS:=ethtool-full
endef

define Package/ethtool-full
  $(Package/ethtool)
  TITLE += (full)
  VARIANT:=full
  PROVIDES:=ethtool
  DEPENDS:=+libmnl
  CONFLICTS:=
endef

define Package/ethtool/description
 ethtool is a small utility for examining and tuning your ethernet-based
 network interface
endef

Package/ethtool-full/description:=$(Package/ethtool/description)

ifeq ($(BUILD_VARIANT),full)
CONFIGURE_ARGS += --enable-netlink --enable-pretty-dump
else
CONFIGURE_ARGS += --disable-netlink --disable-pretty-dump
endif

define Package/ethtool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ethtool $(1)/usr/sbin
endef

Package/ethtool-full/install=$(Package/ethtool/install)

$(eval $(call BuildPackage,ethtool))
$(eval $(call BuildPackage,ethtool-full))
