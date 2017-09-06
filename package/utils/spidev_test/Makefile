#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=spidev-test
PKG_RELEASE:=$(LINUX_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/spidev-test
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+kmod-spi-dev
  TITLE:=SPI testing utility
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
endef

define Package/spidev-test/description
  SPI testing utility.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/spidev_test \
		$(PKG_BUILD_DIR)/spidev_test.c
endef

define Package/spidev-test/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spidev_test $(1)/sbin/
endef

$(eval $(call BuildPackage,spidev-test))
