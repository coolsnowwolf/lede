#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mbtools
PKG_SOURCE_DATE:=2014-10-29
PKG_SOURCE_VERSION:=149e9c69cec180f18cf8781cf5285b97352bf719
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/webstack/mbtools
PKG_MIRROR_HASH:=3f6a452d59a62b4f582020ebf1e371f0f0c94d99b1b056af3c4e124426a7a772

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/mbtools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Modbus tools
  DEPENDS:=+glib2 +libmodbus $(INTL_DEPENDS)
endef

define Package/mbtools/description
  Modbus tools based on libmodbus to log data fetched by a master/client or
  received by a slave/server (writing of registers). mbcollect is able to act
  as client or server (in TCP or RTU)
endef

ifneq ($(INTL_FULL),)
TARGET_LDFLAGS += \
	-L$(INTL_PREFIX)/lib \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

endif

define Package/mbtools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mbrecorder $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mbcollect $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/src/mbcollect.ini $(1)/etc
endef

$(eval $(call BuildPackage,mbtools))
