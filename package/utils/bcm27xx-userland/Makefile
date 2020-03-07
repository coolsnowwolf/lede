#
# Copyright (C) 2019-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bcm27xx-userland
PKG_VERSION:=06bc6daa02137ca72b7a2104afad81e82a44de17
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/raspberrypi/userland/tar.gz/$(PKG_VERSION)?
PKG_HASH:=cf4dff40a2f4ec38bc7e42a80b989df1bf88e12d714f7e681f83c4e5a588f5c9

PKG_FLAGS:=nonshared

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

CMAKE_INSTALL:=1
CMAKE_OPTIONS+=-DVMCS_INSTALL_PREFIX=/usr

ifeq ($(ARCH),aarch64)
  CMAKE_OPTIONS+=-DARM64=ON
else
  CMAKE_OPTIONS+=-DARM64=OFF
endif

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TAR_OPTIONS:=--strip-components 1 $(TAR_OPTIONS)
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define Package/bcm27xx-userland
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@TARGET_bcm27xx
  TITLE:=BCM27xx userland tools
  DEFAULT:=y if TARGET_bcm27xx
endef

define Package/bcm27xx-userland/description
  BCM27xx userland tools including vcgencmd and tvservice.
endef

define Package/bcm27xx-userland/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtmerge $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtparam $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtoverlay $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspistill $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspivid $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspividyuv $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspiyuv $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tvservice $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcgencmd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcmailbox $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ $(1)/usr/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ $(1)/usr/
endef

$(eval $(call BuildPackage,bcm27xx-userland))
