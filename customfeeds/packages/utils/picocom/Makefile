#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=picocom
PKG_VERSION:=3.1
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/npat-efault/picocom/tar.gz/$(PKG_VERSION)?
PKG_HASH:=e6761ca932ffc6d09bd6b11ff018bdaf70b287ce518b3282d29e0270e88420bb

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE:=LICENSE.txt

include $(INCLUDE_DIR)/package.mk

define Package/picocom
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A minimal dumb-terminal emulation program
  URL:=https://github.com/npat-efault/picocom
  SUBMENU:=Terminal
endef

define Package/picocom/description
 minimal dumb-terminal emulation program
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" \
		picocom
endef

define Package/picocom/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/picocom $(1)/usr/bin/
endef

$(eval $(call BuildPackage,picocom))
