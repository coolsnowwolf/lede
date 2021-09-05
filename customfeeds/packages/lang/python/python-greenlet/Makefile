#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-greenlet
PKG_VERSION:=1.0.0
PKG_RELEASE:=2

PYPI_NAME:=greenlet
PKG_HASH:=719e169c79255816cdcf6dccd9ed2d089a72a9f6c42273aae12d55e8d35bdcf8

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
# FIXME: remove when GCC10 is the oldest supported compiler, or the issue goes away
PKG_USE_MIPS16:=0

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-greenlet
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lightweight coroutines for in-process concurrent programming
  URL:=https://github.com/python-greenlet/greenlet
  DEPENDS:= \
	+python3-light \
	+libstdcpp \
	@!arc
endef

define Package/python3-greenlet/description
  The greenlet package is a spin-off of Stackless
  a version of CPython that supports micro-threads called tasklets.
endef

# FIXME: remove when GCC10 is the oldest supported compiler, or the issue goes away
# This is required in addition to PKG_USE_MIPS16:=0 because otherwise MIPS16
# flags are inherited from the Python base package (via sysconfig module)
ifdef CONFIG_USE_MIPS16
TARGET_CFLAGS += -mno-mips16 -mno-interlink-mips16
endif

$(eval $(call Py3Package,python3-greenlet))
$(eval $(call BuildPackage,python3-greenlet))
$(eval $(call BuildPackage,python3-greenlet-src))
