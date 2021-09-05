#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-schedule
PKG_VERSION:=0.6.0
PKG_RELEASE:=2

PYPI_NAME:=schedule
PKG_HASH:=f9fb5181283de4db6e701d476dd01b6a3dd81c38462a54991ddbb9d26db857c9

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-schedule
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Job scheduling for humans
  URL:=https://github.com/dbader/schedule
  DEPENDS:=+python3-light +python3-logging
endef

define Package/python3-schedule/description
  An in-process scheduler for periodic jobs that uses the builder pattern for configuration.
  The Schedule lets you run Python functions (or any other callable)
  periodically at predetermined intervals using a simple, human-friendly syntax.
endef

$(eval $(call Py3Package,python3-schedule))
$(eval $(call BuildPackage,python3-schedule))
$(eval $(call BuildPackage,python3-schedule-src))
