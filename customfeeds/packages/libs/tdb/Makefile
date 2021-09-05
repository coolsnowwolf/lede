#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tdb
PKG_VERSION:=1.4.3
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.samba.org/ftp/tdb/
PKG_HASH:=c8058393dfa15f47e11ebd2f1d132693f0b3b3b8bf22d0201bfb305026f88a1b

PKG_BUILD_DEPENDS:=python3/host
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
# for $(LINUX_VERSION)
include $(INCLUDE_DIR)/kernel.mk
# for $(VERSION_DIST)
include $(INCLUDE_DIR)/version.mk

define Package/tdb
  SUBMENU:=Database
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Trivial Database
  URL:=http://sourceforge.net/projects/tdb/
  MAINTAINER:=Dmitry V. Zimin <pfzim@mail.ru>
endef

define Package/tdb/description
  TDB is a Trivial Database. In concept, it is very much like GDBM,
  and BSD's DB except that it allows multiple simultaneous writers
  and uses locking internally to keep writers from trampling on
  each other. TDB is also extremely small.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/tdb.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

CONFIGURE_ARGS:= \
	--cross-compile \
	--cross-answers="$(PKG_BUILD_DIR)/cache.txt" \
	--prefix=$(CONFIGURE_PREFIX) \
	--exec-prefix=$(CONFIGURE_PREFIX) \
	--bindir=$(CONFIGURE_PREFIX)/bin \
	--sbindir=$(CONFIGURE_PREFIX)/sbin \
	--libexecdir=$(CONFIGURE_PREFIX)/lib \
	--sysconfdir=/etc \
	--datadir=$(CONFIGURE_PREFIX)/share \
	--localstatedir=/var \
	--mandir=$(CONFIGURE_PREFIX)/man \
	--infodir=$(CONFIGURE_PREFIX)/info \
	--disable-python \
	--disable-rpath \
	--disable-rpath-install \
	--disable-rpath-private-install

define Build/Configure
	$(CP) ./files/tdb.cache.txt $(PKG_BUILD_DIR)/cache.txt
	echo -e "\nChecking uname sysname type: \"$(VERSION_DIST)\" \
		\nChecking uname release type: \"$(LINUX_VERSION)-$(GNU_TARGET_NAME)\" \
		\nChecking uname machine type: \"$(ARCH)\" \
		\nChecking uname version type: \"$(VERSION_DIST) Linux-$(LINUX_VERSION) $(shell date +%Y-%m-%d)\"\n" >> $(PKG_BUILD_DIR)/cache.txt;
	$(call Build/Configure/Default)
endef

define Package/tdb/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tdb))

