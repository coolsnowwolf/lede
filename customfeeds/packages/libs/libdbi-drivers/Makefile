#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdbi-drivers
PKG_VERSION:=0.9.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/libdbi-drivers
PKG_HASH:=43d2eacd573a4faff296fa925dd97fbf2aedbf1ae35c6263478210c61004c854

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Nicolas Thill <nico@openwrt.org>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdbi-drivers/default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://libdbi-drivers.sourceforge.net/
endef

define Package/libdbi-drivers
$(call Package/libdbi-drivers/default)
  DEPENDS:=libdbi +libdbd-mysql +libdbd-pgsql +libdbd-sqlite3
  TITLE:=Database drivers for libdbi
endef

define Package/libdbd-mysql
$(call Package/libdbi-drivers/default)
  DEPENDS:=libdbi +libmysqlclient
  TITLE:=MySQL database server driver for libdbi
endef

define Package/libdbd-pgsql
$(call Package/libdbi-drivers/default)
  DEPENDS:=libdbi +libpq
  TITLE:=PostgreSQL database server driver for libdbi
endef

define Package/libdbd-sqlite3
$(call Package/libdbi-drivers/default)
  DEPENDS:=libdbi +libsqlite3
  TITLE:=SQLite3 database driver for libdbi
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static \
	--disable-rpath \
	--disable-docs \
	--with-dbi-incdir=$(STAGING_DIR)/usr/include \
	--with-dbi-libdir=$(STAGING_DIR)/usr/lib

ifneq ($(CONFIG_PACKAGE_libdbd-mysql),)
	CONFIGURE_ARGS += \
		--with-mysql \
		--with-mysql-incdir=$(STAGING_DIR)/usr/include/mysql \
		--with-mysql-libdir=$(STAGING_DIR)/usr/lib/mysql
	TARGET_LDFLAGS += \
		-L$(STAGING_DIR)/usr/lib/mysql/
else
	CONFIGURE_ARGS += --without-mysql
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_libdbd-pgsql),)
	CONFIGURE_ARGS += \
		--with-pgsql \
		--with-pgsql-incdir=$(STAGING_DIR)/usr/include \
		--with-pgsql-libdir=$(STAGING_DIR)/usr/lib
else
	CONFIGURE_ARGS += --without-pgsql
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_libdbd-sqlite3),)
	CONFIGURE_ARGS += \
		--with-sqlite3 \
		--with-sqlite3-incdir=$(STAGING_DIR)/usr/include \
		--with-sqlite3-libdir=$(STAGING_DIR)/usr/lib
else
	CONFIGURE_ARGS += --without-sqlite3
endif

define BuildPlugin
  define Package/libdbd-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/dbd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dbd/libdbd$(1).so $$(1)/usr/lib/dbd/
  endef
  $$(eval $$(call BuildPackage,libdbd-$(1)))
endef

#$(eval $(call BuildPackage,libdbi-drivers))
$(eval $(call BuildPlugin,mysql))
$(eval $(call BuildPlugin,pgsql))
$(eval $(call BuildPlugin,sqlite3))
