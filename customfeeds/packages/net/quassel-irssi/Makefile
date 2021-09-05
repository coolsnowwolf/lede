#
# Copyright (C) 2016 Ben Rosser <rosser.bjr@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quassel-irssi
PKG_SOURCE_DATE:=2017-11-30
PKG_SOURCE_VERSION:=079be662dde374a383646256108a4974c2bc7796
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/phhusson/quassel-irssi/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=c276a92a47f8edf5ae1d9db0e72a69d078f2f3f80e055853fc6d06099d898966
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=Ben Rosser <rosser.bjr@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=core/COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

MAKE_PATH := core
MAKE_VARS += SYSTEM_QUASSELC=1 IRSSI_CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" IRSSI_INCLUDE=$(STAGING_DIR)/usr/include/irssi

define Package/quassel-irssi
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+irssi +quasselc
    SUBMENU:=Instant Messaging
    URL:=https://github.com/phhusson/quassel-irssi
    TITLE:=An irssi plugin to connect to quassel core
endef

define Package/quassel-irssi/description
  An irssi plugin that supports connecting to a quassel core.
endef

define Package/quassel-irssi/install
	$(INSTALL_DIR) $(1)/usr/lib/irssi/modules/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/irssi/modules/libquassel_core.so* $(1)/usr/lib/irssi/modules/
endef

$(eval $(call BuildPackage,quassel-irssi))
