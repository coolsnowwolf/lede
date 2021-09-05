#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=avrdude
PKG_VERSION:=6.3
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/$(PKG_NAME)
PKG_HASH:=0f9f731b6394ca7795b88359689a7fa1fba818c6e1d962513eb28da670e0a196

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/avrdude
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Microcontroller programming
  TITLE:=AVR Downloader/UploaDEr
  URL:=https://www.nongnu.org/avrdude/
  DEPENDS:=+libncurses +libreadline +libusb-compat +libftdi1 +libelf
endef

define Package/avrdude/description
 AVRDUDE is a full featured program for programming Atmel's AVR CPU's.
endef

CONFIGURE_ARGS+= \
	--disable-doc \
	--disable-parport \
	--enable-linuxgpio \

TARGET_CFLAGS+= \
	-D_GNU_SOURCE \

define Package/avrdude/conffiles
/etc/avrdude.conf
endef

define Package/avrdude/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/avrdude.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/avrdude $(1)/usr/bin/
endef

$(eval $(call BuildPackage,avrdude))
