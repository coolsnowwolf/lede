# This Makefile is free software, SPDX codes: GPL-2.0-or-later OR MIT

include $(TOPDIR)/rules.mk

PKG_NAME:=cJSON
PKG_VERSION:=1.7.14
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/DaveGamble/cJSON/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=fb50a663eefdc76bafa80c82bc045af13b1363e8f45cec8b442007aef6a41343

PKG_MAINTAINER:=Karl Palsson <karlp@etactica.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:cjson_project:cjson

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cJSON
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Ultralightweight JSON parser in ANSI C
  URL:=https://github.com/DaveGamble/cJSON
endef

define Package/cJSON/description
 Ultralightweight JSON parser in ANSI C.
cJSON aims to be the dumbest possible parser that you can get your
job done with. It's a single file of C, and a single header file.
endef

CMAKE_OPTIONS += -DENABLE_CJSON_TEST=OFF

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cjson
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cjson/cJSON.h $(1)/usr/include/cjson
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjson.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libcjson.pc $(1)/usr/lib/pkgconfig
	$(SED) 's,/usr,$(STAGING_DIR)/usr,g' $(1)/usr/lib/pkgconfig/libcjson.pc
endef

define Package/cJSON/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcjson.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,cJSON))
