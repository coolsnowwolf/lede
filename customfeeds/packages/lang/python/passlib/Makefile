# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=passlib
PKG_VERSION:=1.7.2
PKG_RELEASE:=2
PKG_LICENSE:=BSD-3-Clause

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=8d666cef936198bc2ab47ee9b0410c94adf2ba798e5a84bf220be079ae7ab6a8

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk

include ../python3-package.mk

define Package/python3-passlib
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Comprehensive password hashing framework
  URL:=https://bitbucket.org/ecollins/passlib
  DEPENDS:=+python3 +python3-dateutil
endef

define Package/python3-passlib/description
Passlib is a password hashing library for Python 2 & 3, which provides
cross-platform implementations of over 30 password hashing algorithms,
as well as a framework for managing existing password hashes. It’s
designed to be useful for a wide range of tasks, from verifying a hash
found in /etc/shadow, to providing full-strength password hashing for
multi-user applications.
endef

$(eval $(call Py3Package,python3-passlib))

$(eval $(call BuildPackage,python3-passlib))
$(eval $(call BuildPackage,python3-passlib-src))
