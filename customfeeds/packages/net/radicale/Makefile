#
# Copyright (C) 2008-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=radicale
PKG_VERSION:=1.1.6
PKG_RELEASE:=5

PYPI_NAME:=Radicale
PKG_HASH:=c007198ea45ef797344672c681d4c13f8b4aa85c15c41a1156225767a405c92b

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:radicale:radicale

include ../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

define Package/radicale
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=Radicale CalDAV/CardDAV server
  URL:=https://radicale.org/
  DEPENDS:= \
      +python3-light \
      +python3-codecs \
      +python3-logging \
      +python3-openssl \
      +python3-urllib \
      +python3-xml
  USERID:=radicale=5232:radicale=5232
  PROVIDES:=radicale-py2 radicale-py3
endef

define Package/radicale/description
  The Radicale Project is a CalDAV (calendar) and CardDAV (contact) server.  It
  aims to be a light solution, easy to use, easy to install, easy to configure.
  As a consequence, it requires few software dependances and is pre-configured to
  work out-of-the-box.
endef

define Package/radicale/conffiles
/etc/config/radicale
/etc/radicale/users
/etc/radicale/rights
endef

define Py3Package/radicale/install
	$(INSTALL_DIR)	$(1)/etc/init.d
	$(INSTALL_BIN)  ./files/radicale.init    $(1)/etc/init.d/radicale

	$(INSTALL_DIR)	$(1)/etc/hotplug.d/iface
	$(INSTALL_BIN)  ./files/radicale.hotplug $(1)/etc/hotplug.d/iface/80-radicale

	$(INSTALL_DIR)	$(1)/etc/config
	$(INSTALL_CONF) ./files/radicale.config  $(1)/etc/config/radicale

	$(INSTALL_DIR)	$(1)/etc/radicale/ssl
	$(INSTALL_DATA) ./files/config.template  $(1)/etc/radicale/config.template
	$(INSTALL_DATA) ./files/logging.template $(1)/etc/radicale/logging.template
	$(INSTALL_DATA) ./files/radicale.users   $(1)/etc/radicale/users
	$(INSTALL_DATA) ./files/radicale.rights  $(1)/etc/radicale/rights

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/radicale $(1)/usr/bin/
endef

$(eval $(call Py3Package,radicale))
$(eval $(call BuildPackage,radicale))
$(eval $(call BuildPackage,radicale-src))
