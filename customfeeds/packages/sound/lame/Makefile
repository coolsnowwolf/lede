#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=lame
PKG_VERSION:=3.100
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/lame
PKG_HASH:=ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e
PKG_FIXUP:=autoreconf

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=LGPL-2.0
PKG_LICENSE_FILES:=COPYING LICENSE
PKG_CPE_ID:=cpe:/a:lame_project:lame

PKG_CONFIG_DEPENDS:= CONFIG_LAME-LIB_OPTIMIZE_SPEED

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL=1

define Package/lame/Default
  SECTION:=sound
  CATEGORY:=Sound
  URL:=http://sourceforge.net/projects/lame
endef

define Package/lame
$(call Package/lame/Default)
  TITLE:=lame
  MENU:=1
  DEPENDS += +libncurses +lame-lib
endef

define Package/lame/description
lame mp3 encoder
endef

define Package/lame-lib/config
	source "$(SOURCE)/Config.in"
endef

define Package/lame-lib
$(call Package/lame/Default)
  TITLE:=lame-lib
endef

define Package/lame-lib/description
lame mp3 encoder libs
endef

ifeq ($(ARCH),i386)
TARGET_CFLAGS+=-msse
endif

ifeq ($(CONFIG_LAME-LIB_OPTIMIZE_SPEED),y)
	TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3 -ffast-math
endif

CONFIGURE_ARGS += --disable-gtktest --disable-static

define Package/lame/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lame $(1)/usr/bin/
endef

define Package/lame-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/lame
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/lame/*.h \
		$(1)/usr/include/lame/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{la,so*} \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,lame-lib))
$(eval $(call BuildPackage,lame))
