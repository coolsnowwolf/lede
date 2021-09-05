#
# Copyright (C) 2017 Stijn Tintel <stijn@linux-ipv6.be>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vallumd
PKG_VERSION:=0.1.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/stintel/vallumd/tar.gz/vallumd-$(PKG_VERSION)?
PKG_HASH:=d6119f84840ff36f05c273f0a6f3e0f9eacf3a07c1583271c4fef62a7b199428

PKG_MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>
PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/vallumd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Centralized or distributed blacklist
  DEPENDS:=+ipset +libmosquitto
endef

define Package/vallumd/description
  Centralized or distributed blacklist
endef

define Package/vallumd/conffiles
/etc/config/vallumd
endef

define Package/vallumd/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d $(1)/usr/sbin
	$(INSTALL_CONF) ./files/vallumd.conf $(1)/etc/config/vallumd
	$(INSTALL_BIN) ./files/vallumd.init $(1)/etc/init.d/vallumd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/vallumd $(1)/usr/sbin/vallumd
endef


$(eval $(call BuildPackage,vallumd))
