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
PKG_BUILD_DIR:=$(LINUX_DIR)/tools/spi-$(TARGET_DIR_NAME)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/spidev-test
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+kmod-spi-dev
  TITLE:=SPI testing utility
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
endef

define Package/spidev-test/description
  SPI testing utility.
endef

define Build/Prepare
	# For SDK: Sources are copied by target/sdk/Makefile's
	# USERSPACE_UTILS(_FILES)
	$(CP) $(LINUX_DIR)/tools/spi/* $(PKG_BUILD_DIR)/
endef

MAKE_FLAGS = \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CC="$(TARGET_CC)" \
	LD="$(TARGET_CROSS)ld" \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	WERROR=0 \
	prefix=/usr

define Build/Compile
	+$(MAKE_FLAGS) $(MAKE) $(PKG_JOBS) \
		-C $(PKG_BUILD_DIR) \
		-f Makefile \
		--no-print-directory
endef

define Package/spidev-test/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spidev_test $(1)/sbin/
endef

$(eval $(call BuildPackage,spidev-test))
