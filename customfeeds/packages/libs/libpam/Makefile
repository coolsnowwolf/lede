#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpam
PKG_VERSION:=1.5.1
PKG_RELEASE:=1

PKG_SOURCE:=Linux-PAM-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/linux-pam/linux-pam/releases/download/v$(PKG_VERSION)
PKG_HASH:=201d40730b1135b1b3cdea09f2c28ac634d73181ccd0172ceddee3649c5792fc
PKG_BUILD_DIR:=$(BUILD_DIR)/Linux-PAM-$(PKG_VERSION)

PKG_MAINTAINER:=Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
PKG_LICENSE:=BSD-3c GPL
PKG_LICENSE_FILES:=COPYING Copyright
PKG_CPE_ID:=cpe:/a:kernel:linux-pam

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpam
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=the Linux-PAM libraries and modules.
  URL:=http://www.kernel.org/pub/linux/libs/pam
endef

define Package/libpam/description
	The Linux-PAM Pluggable Authentication Modules.
endef

CONFIGURE_ARGS += \
	--enable-pamlocking \
	--enable-shared \
	--enable-static \
	--disable-audit \
	--disable-cracklib \
	--disable-db \
	--disable-debug \
	--disable-doc \
	--disable-econf \
	--disable-lckpwdf \
	--disable-nis \
	--disable-prelude \
	--disable-regenerate-docu \
	--disable-rpath \
	--disable-selinux \
	--disable-Werror \
	--with-gnu-ld \
	--without-mailspool \
	--without-xauth

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libpam/install
	$(INSTALL_DIR) $(1)/etc $(1)/etc/pam.d
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/lib/security $(1)/usr/lib/security/pam_filter
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/etc/
	$(CP) ./files/* $(1)/etc/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/*.so* $(1)/usr/lib/security/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/pam_filter/* $(1)/usr/lib/security/pam_filter/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,libpam))
