#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=seafile-ccnet
PKG_VERSION:=7.1.5
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/haiwen/ccnet-server/tar.gz/v$(PKG_VERSION)-server?
PKG_HASH:=11e5281c239474756ed606bffe8f24c1089a3a9eb2502cfac7815d25ebc4585a

PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=AGPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.txt

PKG_BUILD_DIR:=$(BUILD_DIR)/ccnet-server-$(PKG_VERSION)-server
PKG_BUILD_DEPENDS:=vala/host

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../lang/python/python3-package.mk

define Package/seafile-ccnet
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Seafile server - ccnet component
  URL:=http://seafile.com/
  DEPENDS:= \
    +glib2 \
    +jansson \
    +libevent2 \
    +libmariadb \
    +libopenssl \
    +libpthread \
    +libsearpc \
    +libsqlite3 \
    +libuuid \
    $(ICONV_DEPENDS)
endef

define Package/seafile-ccnet/description
  Ccnet is a framework for writing networked applications in C.
endef

define Package/python3-seafile-ccnet
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python bindings for Seafile Ccnet
  DEPENDS:=+seafile-ccnet +python3-light +python3-searpc
endef

define Package/python3-seafile-ccnet/description
$(call Package/seafile-ccnet/description)

  This package contains Python bindings for Ccnet.
endef

CONFIGURE_ARGS += \
	--disable-console \
	--disable-ldap \
	--enable-python \
	--with-mysql="$(STAGING_DIR)/usr/bin/mysql_config"

CONFIGURE_VARS += \
	PYTHON="$(HOST_PYTHON3_BIN)"

define Package/seafile-ccnet/install
	$(INSTALL_DIR) $(1)/usr/{lib,libexec}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/libexec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libccnet.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ccnet
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/ccnet/*.h $(1)/usr/include/ccnet/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libccnet.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libccnet.{a,la,so*} $(1)/usr/lib/
endef

Py3Package/python3-seafile-ccnet/install:=:

$(eval $(call BuildPackage,seafile-ccnet))

$(eval $(call Py3Package,python3-seafile-ccnet))
$(eval $(call BuildPackage,python3-seafile-ccnet))
$(eval $(call BuildPackage,python3-seafile-ccnet-src))
