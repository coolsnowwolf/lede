# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-cryptodome
PKG_VERSION:=3.9.7
PKG_RELEASE:=3

PYPI_NAME:=pycryptodome
PKG_HASH:=f1add21b6d179179b3c177c33d18a2186a09cc0d3af41ff5ed3f377360b869f2

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE.rst
PKG_MAINTAINER:=Richard Yu <yurichard3839@gmail.com>
PKG_CPE_ID:=cpe:/a:pycryptodome:pycryptodome

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=
PYTHON3_PKG_SETUP_VARS:= \
  CONFIG_BIG_ENDIAN="$(CONFIG_BIG_ENDIAN)"

define Package/python3-cryptodome
  SECTION:=lang-python
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=A self-contained cryptographic library for Python
  URL:=https://www.pycryptodome.org/
  DEPENDS:=+libgmp +python3
  PROVIDES:=python3-crypto
endef

define Package/python3-cryptodome/description
PyCryptodome is a self-contained Python package of low-level
cryptographic primitives.
endef

$(eval $(call Py3Package,python3-cryptodome))
$(eval $(call BuildPackage,python3-cryptodome))
$(eval $(call BuildPackage,python3-cryptodome-src))
