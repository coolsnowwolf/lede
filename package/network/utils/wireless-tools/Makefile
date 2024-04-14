#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wireless-tools
PKG_VERSION:=29
PKG_MINOR:=
PKG_RELEASE:=7

PKG_SOURCE:=wireless_tools.$(PKG_VERSION)$(PKG_MINOR).tar.gz
PKG_SOURCE_URL:=https://hewlettpackard.github.io/wireless-tools
PKG_HASH:=6fb80935fe208538131ce2c4178221bab1078a1656306bce8909c19887e2e5a1
TAR_OPTIONS += || true

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0

PKG_BUILD_DIR:=$(BUILD_DIR)/wireless_tools.$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/wireless-tools/Default
  URL:=http://hplabs.hp.com/personal/Jean_Tourrilhes/Linux/Tools.html
endef

define Package/wireless-tools
$(call Package/wireless-tools/Default)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Tools for manipulating Linux Wireless Extensions
endef

define Package/wireless-tools/description
 This package contains a collection of tools for configuring wireless
 adapters implementing the "Linux Wireless Extensions".
endef

define Package/libiw
$(call Package/wireless-tools/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for manipulating Linux Wireless Extensions
  ABI_VERSION:=29
endef

define Package/libiw/description
 This package contains a library for manipulating
 "Linux Wireless Extensions".
endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I." \
		BUILD_WE_ESSENTIAL=y \
		LIBS="-lm -Wl,--gc-sections" \
		libiw.so.$(PKG_VERSION) iwmulticall
	$(MAKE) -C $(PKG_BUILD_DIR) \
		PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_DIR="$(PKG_INSTALL_DIR)/usr/sbin" \
		INSTALL_LIB="$(PKG_INSTALL_DIR)/usr/lib" \
		install-iwmulticall
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/{iwlib,wireless}.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libiw.so* $(1)/usr/lib/
	$(LN) libiw.so.$(PKG_VERSION) $(1)/usr/lib/libiw.so
endef

define Package/wireless-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/iwconfig $(1)/usr/sbin/
	$(LN) iwconfig $(1)/usr/sbin/iwlist
	$(LN) iwconfig $(1)/usr/sbin/iwpriv
endef

define Package/libiw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libiw.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libiw))
$(eval $(call BuildPackage,wireless-tools))
