#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libyang
PKG_VERSION:=1.0.225
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/CESNET/libyang/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=1b736443d2c69b5d7a71ac412655e6edab0647b18f35f7bf504b0a24e06cb046

PKG_MAINTAINER:=Jakov Smolic <jakov.smolic@sartura.hr>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1
CMAKE_BINARY_SUBDIR:=build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libyang
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=YANG data modeling language library
  URL:=https://github.com/CESNET/libyang
  DEPENDS:=+libpcre +libpthread
endef

define Package/libyang-cpp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=YANG data modeling C++ language library
  URL:=https://github.com/CESNET/libyang
  DEPENDS:=+libyang +libstdcpp
endef

define Package/yanglint
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=YANG data modeling language utility
  URL:=https://github.com/CESNET/libyang
  DEPENDS:=+libyang
endef

define Package/libyang/description
 libyang is YANG data modelling language parser and toolkit written (and providing API) in C.
 The library is used e.g. in libnetconf2, Netopeer2 or sysrepo projects.
endef

CMAKE_OPTIONS += \
	-DENABLE_LYD_PRIV:BOOL=ON \
	-DCMAKE_BUILD_TYPE:String="Release" \
	-DGEN_LANGUAGE_BINDINGS=ON \
	-DGEN_PYTHON_BINDINGS=OFF

define Package/libyang/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libyang.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/libyang1
	$(INSTALL_DIR) $(1)/usr/lib/libyang1/extensions
	$(INSTALL_DIR) $(1)/usr/lib/libyang1/user_types
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libyang1/extensions/* $(1)/usr/lib/libyang1/extensions
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libyang1/user_types/* $(1)/usr/lib/libyang1/user_types
endef

define Package/libyang-cpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libyang-cpp.so* $(1)/usr/lib/
endef

define Package/yanglint/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/yanglint $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libyang))
$(eval $(call BuildPackage,libyang-cpp))
$(eval $(call BuildPackage,yanglint))
