#
# Copyright (C) 2012-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecdsautils
PKG_VERSION:=0.3.2.20160630
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/freifunk-gluon/ecdsautils
PKG_SOURCE_VERSION:=07538893fb6c2a9539678c45f9dbbf1e4f222b46
PKG_MIRROR_HASH:=397395a471c0b5af1a173666ba21a5bedb4c3423a6e37c545c3627bed73dcb76

PKG_MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>
PKG_LICENSE_FILES:=COPYRIGHT

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libecdsautil
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libuecc
  TITLE:=ECDSA library
  URL:=https://github.com/tcatm/ecdsautils
  LICENSE:=MIT
endef

define Package/ecdsautils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libecdsautil +libuecc
  TITLE:=ECDSA Utilities
  URL:=https://github.com/tcatm/ecdsautils
  LICENSE:=BSD-2-Clause
endef

define Package/libecdsautil/description
 Library to sign and verify checksums using ECDSA.
endef

define Package/ecdsautils/description
 Utilities to sign and verify checksums using ECDSA.
endef

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="MINSIZEREL"

define Package/libecdsautil/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libecdsautil.so* $(1)/usr/lib/
endef

define Package/ecdsautils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ecdsautil $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ecdsakeygen $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ecdsasign $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ecdsaverify $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libecdsautil))
$(eval $(call BuildPackage,ecdsautils))
