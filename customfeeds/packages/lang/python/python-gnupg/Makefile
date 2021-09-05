# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-gnupg
PKG_VERSION:=0.4.7
PKG_RELEASE:=$(AUTORELEASE)

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=2061f56b1942c29b92727bf9aecbd3cea3893acc9cccbdc7eb4604285efe4ac7

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.txt
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_CPE_ID:=cpe:/a:python:python-gnupg

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-gnupg
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=A wrapper for GnuPG
  URL:=https://docs.red-dove.com/python-gnupg/
  DEPENDS:=+gnupg +python3-light +python3-logging
endef

define Package/python3-gnupg/description
  The gnupg module allows Python programs to make use of the
  functionality provided by the GNU Privacy Guard (abbreviated GPG or
  GnuPG). Using this module, Python programs can encrypt and decrypt
  data, digitally sign documents and verify digital signatures, manage
  (generate, list and delete) encryption keys, using Public Key
  Infrastructure (PKI) encryption technology based on OpenPGP.
endef

$(eval $(call Py3Package,python3-gnupg))
$(eval $(call BuildPackage,python3-gnupg))
$(eval $(call BuildPackage,python3-gnupg-src))
