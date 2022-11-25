#
# Copyright (C) 2015 Rafał Miłecki <zajec5@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=otrx
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firmware-utils.git
PKG_SOURCE_DATE:=2021-12-02
PKG_SOURCE_VERSION:=56e8e19151743c923f48604c457850cf8eb52076
PKG_MIRROR_HASH:=2a40ac73e8eab0a7a4474cb331b8e2fc972635314b0b5e02a9f2b9a32c5d5f3b

include $(INCLUDE_DIR)/package.mk

define Package/otrx
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for opening (analyzing) TRX firmware images
  MAINTAINER:=Rafał Miłecki <zajec5@gmail.com>
  DEPENDS:=@(TARGET_bcm47xx||TARGET_bcm53xx)
endef

define Package/otrx/description
 This package contains an utility that allows validating TRX images.
endef

TARGET_CFLAGS += -Wall

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-o $(PKG_BUILD_DIR)/otrx \
		$(PKG_BUILD_DIR)/src/otrx.c
endef

define Package/otrx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/otrx $(1)/usr/bin/
endef

$(eval $(call BuildPackage,otrx))
