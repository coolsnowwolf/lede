#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-sentry-sdk
PKG_VERSION:=0.19.2
PKG_RELEASE:=1

PYPI_NAME:=sentry-sdk
PKG_HASH:=17b725df2258354ccb39618ae4ead29651aa92c01a92acf72f98efe06ee2e45a

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-sentry-sdk
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python Client for Sentry
  URL:=https://github.com/getsentry/sentry-python
  DEPENDS:= \
	+python3-certifi \
	+python3-light \
	+python3-logging \
	+python3-urllib3
endef

define Package/python3-sentry-sdk/description
  Python Sentry-Python is an SDK for Sentry.
endef

$(eval $(call Py3Package,python3-sentry-sdk))
$(eval $(call BuildPackage,python3-sentry-sdk))
$(eval $(call BuildPackage,python3-sentry-sdk-src))
