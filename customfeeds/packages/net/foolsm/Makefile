#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=foolsm
PKG_VERSION:=1.0.13
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://lsm.foobar.fi/download
PKG_HASH:=4eeda0e666e8ee93aab9b9c6709e9695e042dc391fb0999280874c8a73bce476

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/foolsm
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+msmtp
  TITLE:=A link state monitor
  URL:=http://lsm.foobar.fi/
endef

define Package/foolsm/description
  foolsm is a link state monitor for carrying out actions when a link
  transistions from the up to down state or vice versa.
endef

define Package/foolsm/conffiles
/etc/foolsm/foolsm.conf
endef

MAKE_FLAGS += \
	PREFIX="$(CONFIGURE_PREFIX)"

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/foolsm/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/foolsm $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/foolsm/script.d
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) ./files/connections.conf $(1)/etc/foolsm/connections.conf
	$(INSTALL_DATA) ./files/foolsm.conf $(1)/etc/foolsm/foolsm.conf
	$(INSTALL_BIN) ./files/foolsm_script $(1)/etc/foolsm/script
	$(INSTALL_BIN) ./files/foolsm.init $(1)/etc/init.d/foolsm
endef

define Package/foolsm/conffiles
/etc/foolsm/connections.conf
/etc/foolsm/foolsm.conf
endef

$(eval $(call BuildPackage,foolsm))
