#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ap51-flash
PKG_VERSION:=2019.0.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ap51-flash/ap51-flash/releases/download/v$(PKG_VERSION)
PKG_HASH:=7146a22576a23bfe36673980bc3795a97417692eaddb2f90b527074a7d7e42d5

PKG_MAINTAINER:=Russell Senior <russell@personaltelco.net>
PKG_LICENSE:=GPL-3.0-or-later CC0-1.0
PKG_LICENSE_FILES:=LICENSES/GPL-3.0-or-later.txt LICENSES/CC0-1.0.txt

include $(INCLUDE_DIR)/package.mk

define Package/ap51-flash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A tool for flashing (nearly) all ap51/ap61 based routers
  URL:=https://ap51-flash.readthedocs.io/en/latest/
endef

# pass optimization flags
MAKE_FLAGS += \
	REVISION="$(PKG_VERSION)" \
	ap51-flash

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/ap51-flash/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d $(1)/usr/sbin \
		$(1)/usr/lib/ap51-flash
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ap51-flash $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/ap51-flash.init $(1)/etc/init.d/ap51-flash
	$(INSTALL_BIN) ./files/ap51-flash.sh $(1)/usr/lib/ap51-flash/ap51-flash.sh
	$(INSTALL_DATA) ./files/ap51-flash.config $(1)/etc/config/ap51-flash
endef

define Package/ap51-flash/conffiles
/etc/config/ap51-flash
endef

$(eval $(call BuildPackage,ap51-flash))
