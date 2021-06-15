#
# Copyright (C) Felix Fietkau <nbd@nbd.name>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fwtool
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

PKG_MAINTAINER := Felix Fietkau <nbd@nbd.name>
PKG_BUILD_DEPENDS := fwtool/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

define Package/fwtool
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for appending and extracting firmware metadata and signatures
endef

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $(HOST_BUILD_DIR)/fwtool ./src/fwtool.c
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/fwtool $(1)/bin/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/fwtool ./src/fwtool.c
endef

define Package/fwtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fwtool $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,fwtool))
