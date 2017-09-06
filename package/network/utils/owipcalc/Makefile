#
# Copyright (C) 2012 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the Apache 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=owipcalc
PKG_RELEASE:=3
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk


define Package/owipcalc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple IPv4/IPv6 address calculator
  MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
endef

define Package/owipcalc/description
  The owipcalc utility supports a number of calculations and tests to work
  with ip-address ranges, this is useful for scripts that e.g. need to
  partition ipv6-prefixes into small subnets or to calculate address ranges
  for dhcp pools.
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/owipcalc $(PKG_BUILD_DIR)/owipcalc.c
endef


define Package/owipcalc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/owipcalc $(1)/usr/bin/owipcalc
endef

$(eval $(call BuildPackage,owipcalc))
