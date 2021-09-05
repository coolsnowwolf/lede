#
# Copyright (C) 2010 Gianluigi Tiesi <sherpya@netfarm.it>
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=debootstrap
PKG_VERSION:=1.0.123
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-udeb_$(PKG_VERSION)_all.udeb
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/d/debootstrap
PKG_HASH:=df537297f212a7ffab3bb033065a466887e552a579d4f7115f2d042a86ba811f

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=Unique
PKG_LICENSE_FILES:=debian/copyright

UNPACK_CMD=ar -p "$(DL_DIR)/$(PKG_SOURCE)" data.tar.xz | xzcat | tar -C $(1) -xf -

include $(INCLUDE_DIR)/package.mk

define Package/debootstrap
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Bootstrap a basic Debian system
  URL:=https://wiki.debian.org/Debootstrap
  DEPENDS:= +coreutils +coreutils-chroot +coreutils-sha1sum +ar +xz-utils +xz
endef

define Package/debootstrap/description
 debootstrap is used to create a Debian base system from scratch, without
 requiring the availability of dpkg or apt. It does this by downloading .deb
 files from a mirror site, and carefully unpacking them into a directory which
 can eventually be chrooted into.
endef

define Build/Compile
# file pkgdetails.c was imported from debian package base-installer version 1.130
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) ./files/pkgdetails.c -o $(PKG_BUILD_DIR)/usr/share/debootstrap/pkgdetails
endef

define Package/debootstrap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usr/sbin/$(PKG_NAME) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/share/debootstrap
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usr/share/debootstrap/pkgdetails $(1)/usr/share/debootstrap
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usr/share/debootstrap/functions $(1)/usr/share/debootstrap
	$(INSTALL_DIR) $(1)/usr/share/debootstrap/scripts
	$(CP) $(PKG_BUILD_DIR)/usr/share/debootstrap/scripts/* $(1)/usr/share/debootstrap/scripts
endef

$(eval $(call BuildPackage,debootstrap))
