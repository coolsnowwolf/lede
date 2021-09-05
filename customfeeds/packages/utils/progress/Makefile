#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=progress
PKG_VERSION:=0.15
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/Xfennec/progress/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=1ed0ac65a912ef1aa605d524eaddaacae92079cf71182096a7c65cbc61687d1b

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/progress
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Coreutils Progress Viewer (formerly known as 'cv')
  URL:=https://github.com/Xfennec/progress
  DEPENDS:=+libncursesw
endef

define Package/progress/description
	This tool can be described as a Tiny, Dirty, Linux-and-OSX-Only
	C command that looks for coreutils basic commands (cp, mv, dd, tar,
	gzip/gunzip, cat, etc.) currently running on your system and
	displays the percentage of copied data. It can also show estimated
	time and throughput, and provide a "top-like" mode (monitoring).
	.
	It simply scans `/proc` for interesting commands, and then looks at
	directories `fd` and `fdinfo` to find opened files and seek positions,
	and reports status for the largest file.
	.
	It's very light, and compatible with virtually any command.
	This program was formerly known as 'cv' on github.
endef

define Package/progress/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/progress $(1)/usr/bin/
endef

$(eval $(call BuildPackage,progress))
