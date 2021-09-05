#
# Copyright (C) 2007-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gkrellmd
PKG_VERSION:=2.3.11
PKG_RELEASE:=1

PKG_SOURCE:=gkrellm-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://gkrellm.srcbox.net/releases
PKG_HASH:=1ee0643ed9ed99f88c1504c89d9ccb20780cf29319c904b68e80a8e7c8678c06
PKG_BUILD_DIR:=$(BUILD_DIR)/gkrellm-$(PKG_VERSION)

PKG_MAINTAINER:=Peter Denison <openwrt@marshadder.org>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/gkrellmd
  SECTION:=admin
  CATEGORY:=Administration
  DEPENDS:=+glib2
  TITLE:=The GNU Krell Monitors Server
  URL:=http://gkrellm.net/
endef

define Package/gkrellmd/description
	Gkrellmd listens for connections from gkrellm clients. When
	a gkrellm client connects to a gkrellmd server all builtin
	monitors collect their data from the server.
endef

define Package/gkrellmd/conffiles
/etc/$(PKG_NAME).conf
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/server \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) " \
		without-libsensors="yes"
endef

define Package/gkrellmd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/server/$(PKG_NAME) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/server/$(PKG_NAME).conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,gkrellmd))
