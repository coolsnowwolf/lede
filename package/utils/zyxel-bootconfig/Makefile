#
# Copyright (C) 2022 David Bauer <mail@david-bauer.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zyxel-bootconfig
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/zyxel-bootconfig
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for handling ZyXEL Bootconfig settings
  MAINTAINER:=David Bauer <mail@david-bauer.net>
endef

define Package/zyxel-bootconfig/description
 This package contains an utility that allows handling ZyXEL Bootconfig settings.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/zyxel-bootconfig/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/lib/preinit
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zyxel-bootconfig $(1)/usr/bin/
	$(CP) ./files/95_apply_bootconfig $(1)/lib/preinit/95_apply_bootconfig
endef

$(eval $(call BuildPackage,zyxel-bootconfig))
