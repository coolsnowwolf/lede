#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=seafile-seahub
PKG_VERSION:=7.1.5
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/haiwen/seahub/tar.gz/v$(PKG_VERSION)-server?
PKG_HASH:=cc7f5a1642d203b2390ae3c30c8a5546d1e829d9d1a5ddf686e558292746ce5c

PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.txt

PKG_BUILD_DIR:=$(BUILD_DIR)/seahub-$(PKG_VERSION)-server

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=Django-1.11

PKG_BUILD_PARALLEL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

define Package/seafile-seahub
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Seafile server - seahub component
  URL:=https://seafile.com/
  DEPENDS:= \
    +gunicorn3 \
    +python3 \
    +python3-chardet \
    +python3-cryptodome \
    +python3-dateutil \
    +python3-django1 \
    +python3-django-formtools \
    +python3-django-picklefield \
    +python3-django-postoffice \
    +python3-django-restframework39 \
    +python3-django-simple-captcha \
    +python3-django-statici18n \
    +python3-django-webpack-loader \
    +python3-openpyxl \
    +python3-pillow \
    +python3-pyjwt \
    +python3-pymysql \
    +python3-pytz \
    +python3-qrcode \
    +python3-requests \
    +python3-requests-oauthlib \
    +python3-seafile-ccnet \
    +python3-searpc
endef

define Package/seafile-seahub/description
  The web end of seafile server.

  Note: Localization support is turned off by default for performance
  reasons. Set 'USE_I18N = True' in /etc/seafile/conf/seahub_settings.py
  to use languages other than English.
endef

define Build/Configure
endef

MAKE_VARS += \
	PYTHON="$(HOST_PYTHON3_BIN)" \
	DJANGO_ADMIN_PY="$(STAGING_DIR_HOSTPKG)/bin/django-admin"

define Build/Compile
	$(call HostPython3/PipInstall,$(PYTHON3_PKG_HOST_PIP_INSTALL_ARGS))
	$(call Build/Compile/Default,locale)
endef

define Py3Package/seafile-seahub/install
	$(INSTALL_DIR) $(1)/usr/share/seafile/seafile-server/seahub/tools
	$(CP) $(PKG_BUILD_DIR)/{frontend,locale,media,seahub,sql,static,thirdpart} $(1)/usr/share/seafile/seafile-server/seahub/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/manage.py $(1)/usr/share/seafile/seafile-server/seahub/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/tools/secret_key_generator.py $(1)/usr/share/seafile/seafile-server/seahub/tools/
	$(SED) "s/\(SEAFILE_VERSION\s*=\s*\)'\([0-9]\.[0-9]\.[0-9]\)'/\1'$(PKG_VERSION)'/g" $(1)/usr/share/seafile/seafile-server/seahub/seahub/settings.py
	mv $(1)/usr/share/seafile/seafile-server/seahub/media/avatars $(1)/usr/share/seafile/seafile-server/seahub/media/avatars_default
endef

Py3Package/seafile-seahub/filespec:=

$(eval $(call Py3Package,seafile-seahub))
$(eval $(call BuildPackage,seafile-seahub))
$(eval $(call BuildPackage,seafile-seahub-src))
