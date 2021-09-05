#
# Copyright (C) 2012 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the Apache 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=owipcalc
PKG_RELEASE:=7

PKG_MAINTAINER:=Nick Hainke <vincent@systemli.org>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/owipcalc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple IPv4/IPv6 address calculator
  DEPENDS:=+libowipcalc
endef

define Package/owipcalc/description
  The owipcalc utility supports a number of calculations and tests to work
  with ip-address ranges, this is useful for scripts that e.g. need to
  partition ipv6-prefixes into small subnets or to calculate address ranges
  for dhcp pools.
endef

define Package/libowipcalc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Owipcalc Library
endef

define Package/libowipcalc/description
  The owipcalc library supports a number of calculations and tests to work
  with ip-address ranges, this is useful for programms that e.g. need to
  partition ipv6-prefixes into small subnets or to calculate address ranges
  for dhcp pools.
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-shared -fPIC -o $(PKG_BUILD_DIR)/libowipcalc.so $(PKG_BUILD_DIR)/owipcalc.c
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/owipcalc $(PKG_BUILD_DIR)/main.c -L$(PKG_BUILD_DIR)/ -lowipcalc
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/owipcalc.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libowipcalc.so $(1)/usr/lib/libowipcalc.so
endef

define Package/owipcalc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/owipcalc $(1)/usr/bin/owipcalc
endef

define Package/libowipcalc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libowipcalc.so $(1)/usr/lib/libowipcalc.so
endef

$(eval $(call BuildPackage,libowipcalc))
$(eval $(call BuildPackage,owipcalc))
