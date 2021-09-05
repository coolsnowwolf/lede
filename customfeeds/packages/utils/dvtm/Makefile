#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dvtm
PKG_VERSION:=0.15
PKG_RELEASE:=1

PKG_SOURCE:=dvtm-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.brain-dump.org/projects/dvtm/
PKG_HASH:=8f2015c05e2ad82f12ae4cf12b363d34f527a4bbc8c369667f239e4542e1e510
PKG_MAINTAINER:=Yunfan Jiang <jyf1987@gmail.com>

PKG_LICENSE:=MIT/X
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/dvtm
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Terminal
  DEPENDS:=+libncursesw
  TITLE:=dynamic virtual terminal manager
  URL:=http://www.brain-dump.org/projects/dvtm/
endef

define Package/dvtm/description
    dvtm brings the concept of tiling window management, popularized by X11-window managers like dwm to the console. As a console window manager it tries to make it easy to work with multiple console based programs.
endef

define Package/dvtm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dvtm $(1)/usr/bin/
endef

$(eval $(call BuildPackage,dvtm))
