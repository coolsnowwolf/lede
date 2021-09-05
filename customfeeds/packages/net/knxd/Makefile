#
# Copyright (C) 2008-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

### Remarks
### 'python pascal ruby lua' need to be deleted in src/clients/Makefile.am

include $(TOPDIR)/rules.mk

PKG_NAME:=knxd
PKG_VERSION:=0.14.51
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/knxd/knxd/tar.gz/$(PKG_VERSION)?
PKG_HASH:=c8378bc6f671a5ab75edb51b23e839ee1adcdd00b372314ca9d2bdcd37fb70fb

PKG_MAINTAINER:=Othmar Truniger <github@truniger.ch>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone

include $(INCLUDE_DIR)/package.mk

define Package/knxd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=EIB KNX daemon
  DEPENDS:=+libusb-1.0 +libev +libfmt
endef

define Package/knxd/description
EIB KNX Daemon
endef

define Package/knxd/conffiles
/etc/knxd.ini
/etc/config/knxd
endef

CONFIGURE_ARGS+= \
	--disable-systemd

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lm)

define Package/knxd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libeibclient.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/knxd_args $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/knxd/eibwrite-cgi $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/knxd/eibread-cgi $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/knxd.init $(1)/etc/init.d/knxd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/knxd.ini $(1)/etc/knxd.ini
	$(INSTALL_DATA) ./files/knxd.config $(1)/etc/config/knxd
endef

$(eval $(call BuildPackage,knxd))
