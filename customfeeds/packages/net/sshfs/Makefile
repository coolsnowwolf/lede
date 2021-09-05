#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sshfs
PKG_VERSION:=3.7.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/libfuse/sshfs/releases/download/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=fe5d3436d61b46974889e0c4515899c21a9d67851e3793c209989f72353d7750

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/sshfs
  TITLE:=SSHFS
  DEPENDS:=+fuse3-utils +glib2 +libpthread
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  URL:=https://github.com/libfuse/sshfs
endef

define Package/sshfs/description
	Mount remote system over sftp.
endef

define Package/sshfs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sshfs $(1)/usr/bin/
endef

$(eval $(call BuildPackage,sshfs))
