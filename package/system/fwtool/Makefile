#
# Copyright (C) Felix Fietkau <nbd@nbd.name>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fwtool
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/fwtool.git
PKG_SOURCE_DATE:=2025-10-03
PKG_SOURCE_VERSION:=04cd252e4e9394ffacd51f56f1f124abc534f715
PKG_MIRROR_HASH:=d0c72a6326aa5a5ee8360b0727075643aec63febc4849ab8f13aed315115d7ab
CMAKE_INSTALL:=1

PKG_FLAGS:=nonshared

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER := Felix Fietkau <nbd@nbd.name>
PKG_BUILD_DEPENDS := fwtool/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

define Package/fwtool
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for appending and extracting firmware metadata and signatures
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/fwtool $(1)/bin/
endef

define Package/fwtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fwtool $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,fwtool))
