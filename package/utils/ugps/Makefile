#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ugps
PKG_RELEASE:=3

PKG_SOURCE_URL=$(PROJECT_GIT)/project/ugps.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2017-02-11
PKG_SOURCE_VERSION:=c30055a685bca1e81ccc8f4bd3bb08d780a7d762
PKG_MIRROR_HASH:=a618f9a15d3a585453550190eed2bb0a308ddba0ab37f2f6b76dd3c87fd80e8b

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ugps
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OpenWrt GPS Daemon
  DEPENDS:=+libubox +libubus
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

ifneq ($(CONFIG_USE_GLIBC),)
  TARGET_CFLAGS += -D_DEFAULT_SOURCE
endif

define Package/ugps/conffiles
/etc/config/gps
endef

define Package/ugps/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ugps $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/ugps.init $(1)/etc/init.d/ugps
	$(INSTALL_CONF) ./files/gps.config $(1)/etc/config/gps
endef

$(eval $(call BuildPackage,ugps))
