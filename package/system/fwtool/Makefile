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
PKG_SOURCE_DATE:=2019-11-12
PKG_SOURCE_VERSION:=8f7fe925ca205c8e8e2d0d1b16218c1e148d5173
PKG_MIRROR_HASH:=ff68e77397a7ba8f497aae9a6d1f89e196c89391a8d5ed0b81c4eafb889ba744
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
