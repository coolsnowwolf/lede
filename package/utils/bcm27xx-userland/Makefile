#
# Copyright (C) 2019-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bcm27xx-userland
PKG_VERSION:=97bc8180ad682b004ea224d1db7b8e108eda4397
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/raspberrypi/userland/tar.gz/$(PKG_VERSION)?
PKG_HASH:=d67def03931215f41b741aed5a3a1bc2bd62fa33f5cc14692e9a4d65f2e0ea27

PKG_FLAGS:=nonshared

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENCE

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

define Package/bcm27xx-userland-dev
  SECTION:=devel
  CATEGORY:=Development
  SUBMENU:=Libraries
  DEPENDS:=@TARGET_bcm27xx +bcm27xx-userland
  TITLE:=Development files of BCM27xx userland tools
endef

define Package/bcm27xx-userland-dev/description
  This package contains the header and static libraries of
  the BCM27xx userland tools.
endef

define Package/bcm27xx-userland/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtmerge $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtparam $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dtoverlay $(1)/usr/bin
ifneq ($(ARCH),aarch64)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspistill $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspivid $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspividyuv $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/raspiyuv $(1)/usr/bin
endif
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tvservice $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcgencmd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vcmailbox $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
ifneq ($(ARCH),aarch64)
	$(INSTALL_DIR) $(1)/usr/lib/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/plugins/ $(1)/usr/lib/
endif
endef

define Package/bcm27xx-userland-dev/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ $(1)/usr/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,bcm27xx-userland))
$(eval $(call BuildPackage,bcm27xx-userland-dev))
