#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tree
PKG_RELEASE:=1
PKG_VERSION:=1.8.0
PKG_SOURCE_URL:=http://mama.indstate.edu/users/ice/tree/src
PKG_HASH:=715d5d4b434321ce74706d0dd067505bb60c5ea83b5f0b3655dae40aa6f9b7c2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_MAINTAINER:=Banglang Huang <banglang.huang@foxmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/tree
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=List contents of directories in a tree-like format
  DEPENDS:=+libc +libgcc
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Package/tree/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tree $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,tree))
