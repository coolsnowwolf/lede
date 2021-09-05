#
# Copyright (C) 2019 Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mstpd
PKG_VERSION:=0.0.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mstpd/mstpd/tar.gz/$(PKG_VERSION)?
PKG_HASH:=91a1862548b5b509caa2e96e5fb9912bc98d4d58cc98e99a577824735756c14d

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/mstpd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Multiple Spanning Tree Protocol daemon
  URL:=https://github.com/mstpd/mstpd
endef

define Package/mstpd/description
  Multiple Spanning Tree Protocol daemon.
  Implements MSTP which is not implemented yet in the Linux kernel.
endef

define Package/mstpd/config
  source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += \
	-DRTNL_RCV_BUFSIZE=$(CONFIG_MSTPD_RTNL_RCV_BUFSIZE) \
	-DRTNL_SND_BUFSIZE=$(CONFIG_MSTPD_RTNL_SND_BUFSIZE)

MAKE_VARS+=MODE=prod

define Package/mstpd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mstpd $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mstpctl $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/etc/init.d/mstpd.init $(1)/etc/init.d/mstpd
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/sbin/bridge-stp $(1)/sbin
endef

$(eval $(call BuildPackage,mstpd))
