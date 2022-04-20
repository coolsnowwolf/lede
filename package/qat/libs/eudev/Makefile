#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=eudev
PKG_VERSION:=3.2.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dev.gentoo.org/~blueness/eudev/
PKG_HASH:=89618619084a19e1451d373c43f141b469c9fd09767973d73dd268b92074d4fc
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=gperf/host
PKG_FIXUP:=autoreconf
PKG_INSTALL=1

define Package/libeudev
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Dynamic device management subsystem
  URL:=https://wiki.gentoo.org/wiki/Project:Eudev
  MAINTAINER:=Ian Cooper <iancooper@hotmail.com>
  PROVIDES:=libudev
  CONFLICTS:=libudev-zero udev
endef

define Package/eudev
  SECTION:=Libs
  CATEGORY:=Libraries
  TITLE:=Dynamic device management subsystem
  URL:=https://wiki.gentoo.org/wiki/Project:Eudev
  MAINTAINER:=Ian Cooper <iancooper@hotmail.com>
  MENU:=1
  DEPENDS:=+EUDEV_EXTRA_blkid:libblkid +EUDEV_EXTRA_kmod:libkmod +librt +libeudev
endef

define Package/eudev/description
Eudev is the Gentoo Linux port of udev. This implementation installs only the libraries
and a udevmonitor binary. This allows packages dependent on libudev to be compiled and
run on OpenWrt. The udevmonitor binary can be used to see what events are tiggered so
that an appropriate hotplug handler script can be installed.
endef

define Package/eudev/config
	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += \
	--prefix=/usr --exec-prefix= --sysconfdir=/etc \
	--libexecdir=/lib/udev --sbindir=/sbin \
	--disable-hwdb --disable-introspection --disable-manpages \
	--disable-selinux \
	$(if $(CONFIG_EUDEV_EXTRA_blkid),--enable-blkid,--disable-blkid) \
	$(if $(CONFIG_EUDEV_EXTRA_kmod),--enable-kmod,--disable-kmod)


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libudev.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/share/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pkgconfig/udev.pc $(1)/usr/share/pkgconfig
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libudev.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libudev.a $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/lib/pkgconfig/libudev.pc $(1)/usr/lib/pkgconfig
endef

define Package/libeudev/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libudev.so* $(1)/usr/lib
endef

define Package/eudev/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/udevadm $(1)/sbin/udevadm
endef

$(eval $(call BuildPackage,eudev))
$(eval $(call BuildPackage,libeudev))
