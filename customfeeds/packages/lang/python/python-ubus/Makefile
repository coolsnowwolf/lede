#
# Copyright (C) 2018-2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-ubus
PKG_VERSION:=0.1.1
PKG_RELEASE:=$(AUTORELEASE)

PYPI_NAME:=ubus
PKG_HASH:=7e57bda989bc35b48c7075d03ec2818226e722bbf1bde138d7e7ea26d462682a

PKG_MAINTAINER:=Erik Larsson <who+openwrt@cnackers.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-ubus
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python3 ubus
  URL:=https://gitlab.nic.cz/turris/python-ubus/
  DEPENDS:=+libubus +libblobmsg-json +python3-light
endef

define Package/python3-ubus/description
  Python bindings for ubus.
endef

$(eval $(call Py3Package,python3-ubus))
$(eval $(call BuildPackage,python3-ubus))
$(eval $(call BuildPackage,python3-ubus-src))
