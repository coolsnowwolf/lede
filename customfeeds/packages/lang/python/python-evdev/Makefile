#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=python-evdev
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PKG_LICENSE:=BSD-3-Clause
PKG_MAINTAINER:=Paulo Costa <me@paulo.costa.nom.br>, Alexandru Ardelean <ardeleanalex@gmail.com>

PYPI_NAME:=evdev
PKG_HASH:=8782740eb1a86b187334c07feb5127d3faa0b236e113206dfe3ae8f77fb1aaf1

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-evdev
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python3-evdev
  URL:=https://github.com/gvalkov/python-evdev
  DEPENDS:= \
      +python3-light \
      +python3-ctypes
endef

define Package/python3-evdev/description
	Bindings to the Linux input handling subsystem
endef

LINUX_EVDEV_HEADERS="$(LINUX_DIR)/include/uapi/linux/input.h:$(LINUX_DIR)/include/uapi/linux/input-event-codes.h"

PYTHON3_PKG_SETUP_GLOBAL_ARGS:= \
	build build_ecodes \
	--evdev-headers="$(LINUX_EVDEV_HEADERS)" \
	build_ext

$(eval $(call Py3Package,python3-evdev))
$(eval $(call BuildPackage,python3-evdev))
$(eval $(call BuildPackage,python3-evdev-src))
